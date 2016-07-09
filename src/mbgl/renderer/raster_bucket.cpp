#include <mbgl/renderer/raster_bucket.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/style/layers/raster_layer.hpp>
#include <mbgl/style/layers/raster_layer_impl.hpp>
#include <mbgl/shader/shaders.hpp>
#include <mbgl/geometry/static_vertex_buffer.hpp>

namespace mbgl {

using namespace style;

void RasterBucket::upload(gl::ObjectStore& store, gl::Config& config) {
    if (hasData()) {
        raster.upload(store, config, 0);
        uploaded = true;
    }
}

void RasterBucket::setImage(PremultipliedImage image) {
    raster.load(std::move(image));
}

bool RasterBucket::hasData() const {
    return raster.isLoaded();
}

static float saturationFactor(float saturation) {
    if (saturation > 0) {
        return 1 - 1 / (1.001 - saturation);
    } else {
        return -saturation;
    }
}

static float contrastFactor(float contrast) {
    if (contrast > 0) {
        return 1 / (1 - contrast);
    } else {
        return 1 + contrast;
    }
}

static std::array<float, 3> spinWeights(float spin) {
    spin *= util::DEG2RAD;
    float s = std::sin(spin);
    float c = std::cos(spin);
    std::array<float, 3> spin_weights = {{
        (2 * c + 1) / 3,
        (-std::sqrt(3.0f) * s - c + 1) / 3,
        (std::sqrt(3.0f) * s - c + 1) / 3
    }};
    return spin_weights;
}

void RasterBucket::render(PaintParameters& parameters,
                          RenderPass pass,
                          const RenderTile& tile,
                          const RasterLayer& layer) {
    if (pass != RenderPass::Translucent) return;
    if (!hasData()) return;

    const RasterPaintProperties& properties = layer.impl->paint;
    auto& rasterShader = parameters.shaders.raster;

    parameters.config.program = rasterShader.getID();
    parameters.config.stencilTest = GL_FALSE;
    parameters.config.depthFunc.reset();
    parameters.config.depthTest = GL_TRUE;
    parameters.config.depthMask = GL_FALSE;
    parameters.setDepthSublayer(0);

    rasterShader.u_matrix = tile.matrix;
    rasterShader.u_buffer_scale = 1.0f;
    rasterShader.u_opacity0 = properties.rasterOpacity;
    rasterShader.u_opacity1 = 0;

    rasterShader.u_brightness_low = properties.rasterBrightnessMin;
    rasterShader.u_brightness_high = properties.rasterBrightnessMax;
    rasterShader.u_saturation_factor = saturationFactor(properties.rasterSaturation);
    rasterShader.u_contrast_factor = contrastFactor(properties.rasterContrast);
    rasterShader.u_spin_weights = spinWeights(properties.rasterHueRotate);

    rasterShader.u_image0 = 0; // GL_TEXTURE0
    rasterShader.u_image1 = 1; // GL_TEXTURE1
    rasterShader.u_tl_parent = {{ 0.0f, 0.0f }};
    rasterShader.u_scale_parent = 1.0f;

    raster.bind(parameters.store, parameters.config, 0, Raster::Scaling::Linear);
    raster.bind(parameters.store, parameters.config, 1, Raster::Scaling::Linear);

    auto& rasterVAO = parameters.shaders.coveringRasterArray;
    rasterVAO.bind(rasterShader, parameters.rasterBoundsBuffer, BUFFER_OFFSET_0, parameters.store);

    MBGL_CHECK_ERROR(glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)parameters.rasterBoundsBuffer.index()));
}

} // namespace mbgl
