#include <mbgl/style/layers/background_layer_impl.hpp>
#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/geometry/static_vertex_buffer.hpp>
#include <mbgl/shader/shaders.hpp>
#include <mbgl/sprite/sprite_atlas.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/util/tile_cover.hpp>

namespace mbgl {
namespace style {

void BackgroundLayer::Impl::cascade(const CascadeParameters& parameters) {
    paint.cascade(parameters);
}

bool BackgroundLayer::Impl::recalculate(const CalculationParameters& parameters) {
    bool hasTransitions = paint.recalculate(parameters);

    passes = paint.backgroundOpacity > 0 ? RenderPass::Translucent : RenderPass::None;

    return hasTransitions;
}

std::unique_ptr<Bucket> BackgroundLayer::Impl::createBucket(BucketParameters&) const {
    return nullptr;
}

void BackgroundLayer::Impl::render(PaintParameters& parameters) {
    // Note that for bottommost layers without a pattern, the background color is drawn with
    // glClear rather than this method.

    bool isPatterned = !paint.backgroundPattern.value.to.empty();
    optional<SpriteAtlasPosition> imagePosA;
    optional<SpriteAtlasPosition> imagePosB;

    auto& patternShader = parameters.shaders.pattern;
    auto& plainShader = parameters.shaders.plain;
    auto& arrayBackgroundPattern = parameters.shaders.backgroundPatternArray;
    auto& arrayBackground = parameters.shaders.backgroundArray;

    mat4 identity;
    matrix::identity(identity);

    if (isPatterned) {
        imagePosA = parameters.spriteAtlas.getPosition(paint.backgroundPattern.value.from, true);
        imagePosB = parameters.spriteAtlas.getPosition(paint.backgroundPattern.value.to, true);

        if (!imagePosA || !imagePosB)
            return;

        parameters.config.program = patternShader.getID();
        patternShader.u_matrix = identity;
        patternShader.u_pattern_tl_a = imagePosA->tl;
        patternShader.u_pattern_br_a = imagePosA->br;
        patternShader.u_pattern_tl_b = imagePosB->tl;
        patternShader.u_pattern_br_b = imagePosB->br;
        patternShader.u_mix = paint.backgroundPattern.value.t;
        patternShader.u_opacity = paint.backgroundOpacity;

        parameters.spriteAtlas.bind(true, parameters.store, parameters.config, 0);
        arrayBackgroundPattern.bind(patternShader, parameters.tileStencilBuffer, BUFFER_OFFSET(0), parameters.store);

    } else {
        parameters.config.program = plainShader.getID();
        plainShader.u_color = paint.backgroundColor;
        plainShader.u_opacity = paint.backgroundOpacity;

        arrayBackground.bind(plainShader, parameters.tileStencilBuffer, BUFFER_OFFSET(0), parameters.store);
    }

    parameters.config.stencilTest = GL_FALSE;
    parameters.config.depthFunc.reset();
    parameters.config.depthTest = GL_TRUE;
    parameters.config.depthMask = GL_FALSE;
    parameters.setDepthSublayer(0);

    for (const auto& tileID : util::tileCover(parameters.state, parameters.state.getIntegerZoom())) {
        mat4 vertexMatrix;
        parameters.state.matrixFor(vertexMatrix, tileID);
        matrix::multiply(vertexMatrix, projMatrix, vertexMatrix);

        if (isPatterned) {
            patternShader.u_matrix = vertexMatrix;
            patternShader.u_pattern_size_a = imagePosA->size;
            patternShader.u_pattern_size_b = imagePosB->size;
            patternShader.u_scale_a = paint.backgroundPattern.value.fromScale;
            patternShader.u_scale_b = paint.backgroundPattern.value.toScale;
            patternShader.u_tile_units_to_pixels = 1.0f / tileID.pixelsToTileUnits(1.0f, parameters.state.getIntegerZoom());

            GLint tileSizeAtNearestZoom = util::tileSize * parameters.state.zoomScale(parameters.state.getIntegerZoom() - tileID.canonical.z);
            GLint pixelX = tileSizeAtNearestZoom * (tileID.canonical.x + tileID.wrap * parameters.state.zoomScale(tileID.canonical.z));
            GLint pixelY = tileSizeAtNearestZoom * tileID.canonical.y;
            patternShader.u_pixel_coord_upper = {{ float(pixelX >> 16), float(pixelY >> 16) }};
            patternShader.u_pixel_coord_lower = {{ float(pixelX & 0xFFFF), float(pixelY & 0xFFFF) }};
        } else {
            plainShader.u_matrix = vertexMatrix;
        }

        MBGL_CHECK_ERROR(glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)parameters.tileStencilBuffer.index()));
    }
}

} // namespace style
} // namespace mbgl
