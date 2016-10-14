#include <mbgl/renderer/painter.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/gl/gl.hpp>
#include <mbgl/renderer/terrain_bucket.hpp>
#include <mbgl/style/layers/terrain_layer.hpp>
#include <mbgl/style/layers/terrain_layer_impl.hpp>
#include <mbgl/shader/shaders.hpp>
#include <mbgl/util/offscreen_texture.hpp>

namespace mbgl {

using namespace style;

void Painter::renderTerrain(PaintParameters& parameters,
                           TerrainBucket& bucket,
                           const TerrainLayer& layer,
                           const RenderTile& tile) {
    if (pass != RenderPass::Translucent) return;

    if (bucket.needsPrepare()) {
        OffscreenTexture texture{ context, {{ 256, 256 }} };
        texture.bind();

        auto& terrainPrepareShader = parameters.shaders.terrainPrepare;
        auto& terrainVAO = parameters.shaders.coveringTerrainPrepareArray;

        context.program = terrainPrepareShader.getID();

        mat4 mat;
        matrix::ortho(mat, 0, util::EXTENT, -util::EXTENT, 0, 0, 1);
        matrix::translate(mat, mat, 0, -util::EXTENT, 0);
        terrainPrepareShader.u_matrix = mat;
        terrainPrepareShader.u_image = 0;
        terrainPrepareShader.u_dimension = {{ 512, 512 }};
        terrainPrepareShader.u_zoom = tile.id.canonical.z;

        context.stencilTest = false;
        context.depthTest = false;

        bucket.drawTerrainPrepare(terrainPrepareShader, rasterVertexBuffer, terrainVAO, context);

        bucket.setPreparedTexture(std::move(texture.getTexture()));

        parameters.view.bind();
    }

    const TerrainPaintProperties& properties = layer.impl->paint;
    (void)properties;

    if (bucket.hasData()) {
        auto& terrainShader = parameters.shaders.terrain;
        auto& terrainVAO = parameters.shaders.coveringTerrainArray;

        context.program = terrainShader.getID();
        terrainShader.u_matrix = tile.matrix;

        context.stencilTest = true;

        terrainShader.u_image = 0; // GL_TEXTURE0

        terrainShader.u_mode = 8;
        terrainShader.u_dimension = {{ 256, 256 }};
        terrainShader.u_zoom = tile.id.canonical.z;
        double azimuth = (-properties.terrainIlluminationDirection - 90) * util::DEG2RAD;
        if (properties.terrainIlluminationAlignment == style::AlignmentType::Viewport) {
            azimuth -= state.getAngle();
        }
        terrainShader.u_azimuth = azimuth;
        terrainShader.u_zenith = 60 * util::DEG2RAD;
        terrainShader.u_mipmap = 0;
        terrainShader.u_shadow = properties.terrainShadowColor;
        terrainShader.u_highlight = properties.terrainHighlightColor;
        terrainShader.u_accent = properties.terrainAccentColor;
        terrainShader.u_exaggeration = properties.terrainExaggeration;

        context.depthFunc = gl::DepthTestFunction::LessEqual;
        context.depthTest = true;
        context.depthMask = false;
        setDepthSublayer(0);

        bucket.drawTerrain(terrainShader, rasterVertexBuffer, terrainVAO, context);
    }
}

} // namespace mbgl
