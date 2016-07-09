#pragma once

#include <mbgl/map/transform_state.hpp>

#include <mbgl/tile/tile_id.hpp>

#include <mbgl/renderer/frame_history.hpp>
#include <mbgl/renderer/render_item.hpp>
#include <mbgl/renderer/bucket.hpp>

#include <mbgl/geometry/vao.hpp>
#include <mbgl/geometry/static_vertex_buffer.hpp>

#include <mbgl/gl/gl_config.hpp>
#include <mbgl/gl/gl.hpp>

#include <mbgl/style/style.hpp>

#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/chrono.hpp>
#include <mbgl/util/constants.hpp>

#include <array>
#include <vector>
#include <set>
#include <map>

namespace mbgl {

class RenderTile;
class SpriteAtlas;
class Tile;
class Shaders;
class PaintParameters;
class ClipID;

namespace style {
class Style;
} // namespace style

struct FrameData {
    std::array<uint16_t, 2> framebufferSize;
    TimePoint timePoint;
    float pixelRatio;
    MapMode mapMode;
    GLContextMode contextMode;
    MapDebugOptions debugOptions;
};

class Painter : private util::noncopyable {
public:
    Painter(const TransformState&, gl::ObjectStore&);
    ~Painter();

    void render(const style::Style&,
                const FrameData&,
                SpriteAtlas& annotationSpriteAtlas);

    // Renders debug information for a tile.
    void renderTileDebug(const RenderTile&);

    // Renders the red debug frame around a tile, visualizing its perimeter.
    void renderDebugFrame(const mat4 &matrix);

    void renderClipMasks();

    void renderDebugText(Tile&, const mat4&);
    void drawClippingMasks(PaintParameters&, const std::map<UnwrappedTileID, ClipID>&);

    bool needsAnimation() const;

private:
    std::vector<RenderItem> determineRenderOrder(const style::Style&);

    template <class Iterator>
    void renderPass(PaintParameters&,
                    RenderPass,
                    Iterator it, Iterator end,
                    GLsizei i, int8_t increment);

    bool isOverdraw() const { return frame.debugOptions & MapDebugOptions::Overdraw; }

    mat4 projMatrix;

    std::array<float, 2> pixelsToGLUnits;

    const TransformState& state;
    gl::ObjectStore& store;

    FrameData frame;

    int indent = 0;

    gl::Config config;

    FrameHistory frameHistory;

    std::unique_ptr<Shaders> shaders;
    std::unique_ptr<Shaders> overdrawShaders;

    // Set up the stencil quad we're using to generate the stencil mask.
    StaticVertexBuffer tileStencilBuffer {
        // top left triangle
        {{ 0, 0 }},
        {{ util::EXTENT, 0 }},
        {{ 0, util::EXTENT }},

        // bottom right triangle
        {{ util::EXTENT, 0 }},
        {{ 0, util::EXTENT }},
        {{ util::EXTENT, util::EXTENT }},
    };

    StaticRasterVertexBuffer rasterBoundsBuffer {
        {{ 0, 0, 0, 0 }},
        {{ util::EXTENT, 0, 32767, 0 }},
        {{ 0, util::EXTENT, 0, 32767 }},
        {{ util::EXTENT, util::EXTENT, 32767, 32767 }},
    };

    // Set up the tile boundary lines we're using to draw the tile outlines.
    StaticVertexBuffer tileBorderBuffer {
        {{ 0, 0 }},
        {{ util::EXTENT, 0 }},
        {{ util::EXTENT, util::EXTENT }},
        {{ 0, util::EXTENT }},
        {{ 0, 0 }},
    };

    VertexArrayObject tileBorderArray;
};

} // namespace mbgl
