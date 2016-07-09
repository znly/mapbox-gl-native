#pragma once

#include <mbgl/gl/gl_config.hpp>
#include <mbgl/gl/object_store.hpp>

#include <mbgl/map/mode.hpp>

#include <array>

namespace mbgl {

class TransformState;
class Shaders;
class StaticVertexBuffer;
class StaticRasterVertexBuffer;
class SpriteAtlas;
class GlyphAtlas;
class LineAtlas;
class FrameHistory;
class ClipID;

class PaintParameters {
public:
    gl::Config& config;
    gl::ObjectStore& store;
    Shaders& shaders;

    StaticVertexBuffer& tileStencilBuffer;
    StaticRasterVertexBuffer& rasterBoundsBuffer;

    bool overdraw;
    std::array<uint16_t, 2> framebufferSize;
    float pixelRatio;
    MapMode mapMode;

    const TransformState& state;
    const std::array<float, 2>& pixelsToGLUnits;

    SpriteAtlas& spriteAtlas;
    GlyphAtlas& glyphAtlas;
    LineAtlas& lineAtlas;
    FrameHistory& frameHistory;

    void setClipping(const ClipID&);
    void setDepthSublayer(int n);

    GLsizei currentLayer = 0;
    float depthRangeSize = 0;

    constexpr static int numSublayers = 3;
    constexpr static float depthEpsilon = 1.0f / (1 << 16);
};

} // namespace mbgl
