#include <mbgl/renderer/painter.hpp>
#include <mbgl/shader/shaders.hpp>
#include <mbgl/shader/fill_uniforms.hpp>
#include <mbgl/util/clip_id.hpp>

namespace mbgl {

void Painter::renderClippingMask(const UnwrappedTileID& tileID, const ClipID& clip) {
    context.draw({
        gl::Depth::disabled(),
        gl::Stencil {
            gl::Stencil::Always(),
            static_cast<int32_t>(clip.reference.to_ulong()),
            0b11111111,
            gl::Stencil::Keep,
            gl::Stencil::Keep,
            gl::Stencil::Replace
        },
        gl::Color::disabled(),
        shaders->fill,
        FillColorUniforms::values(
           matrixForTile(tileID),
           0.0f,
           Color {},
           Color {},
           std::array<uint16_t, 2> {{ context.viewport.getCurrentValue().width, context.viewport.getCurrentValue().height }}
        ),
        gl::Unindexed<gl::Triangles>(tileTriangleVertexBuffer)
    });
}

} // namespace mbgl
