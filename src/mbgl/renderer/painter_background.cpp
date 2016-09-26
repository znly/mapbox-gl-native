#include <mbgl/renderer/painter.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/style/layers/background_layer.hpp>
#include <mbgl/style/layers/background_layer_impl.hpp>
#include <mbgl/shader/shaders.hpp>
#include <mbgl/shader/fill_uniforms.hpp>
#include <mbgl/sprite/sprite_atlas.hpp>
#include <mbgl/util/tile_cover.hpp>

namespace mbgl {

using namespace style;

void Painter::renderBackground(PaintParameters& parameters, const BackgroundLayer& layer) {
    // Note that for bottommost layers without a pattern, the background color is drawn with
    // glClear rather than this method.
    const BackgroundPaintProperties& properties = layer.impl->paint;

    if (!properties.backgroundPattern.value.to.empty()) {
        optional<SpriteAtlasPosition> imagePosA = spriteAtlas->getPosition(
            properties.backgroundPattern.value.from, SpritePatternMode::Repeating);
        optional<SpriteAtlasPosition> imagePosB = spriteAtlas->getPosition(
            properties.backgroundPattern.value.to, SpritePatternMode::Repeating);

        if (!imagePosA || !imagePosB)
            return;

        spriteAtlas->bind(true, context, 0);

        for (const auto& tileID : util::tileCover(state, state.getIntegerZoom())) {
            context.draw({
                depthForSublayer(0, gl::Depth::ReadOnly),
                gl::Stencil::disabled(),
                colorForRenderPass(),
                parameters.shaders.fillPattern,
                FillPatternUniforms::values(
                    matrixForTile(tileID),
                    properties.backgroundOpacity.value,
                    std::array<uint16_t, 2> {{ context.viewport.getCurrentValue().width, context.viewport.getCurrentValue().height }},
                    *imagePosA,
                    *imagePosB,
                    properties.backgroundPattern.value,
                    tileID,
                    state
                ),
                gl::Unindexed<gl::TriangleStrip>(tileTriangleVertexBuffer)
            });
        }
    } else {
        for (const auto& tileID : util::tileCover(state, state.getIntegerZoom())) {
            context.draw({
                depthForSublayer(0, gl::Depth::ReadOnly),
                gl::Stencil::disabled(),
                colorForRenderPass(),
                parameters.shaders.fill,
                FillColorUniforms::values(
                    matrixForTile(tileID),
                    properties.backgroundOpacity.value,
                    properties.backgroundColor.value,
                    properties.backgroundColor.value,
                    std::array<uint16_t, 2> {{ context.viewport.getCurrentValue().width, context.viewport.getCurrentValue().height }}
                ),
                gl::Unindexed<gl::TriangleStrip>(tileTriangleVertexBuffer)
            });
        }
    }
}

} // namespace mbgl
