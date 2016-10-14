#include <mbgl/renderer/terrain_bucket.hpp>
#include <mbgl/style/layers/terrain_layer.hpp>
#include <mbgl/shader/terrain_shader.hpp>
#include <mbgl/shader/terrain_prepare_shader.hpp>
#include <mbgl/renderer/painter.hpp>
#include <mbgl/gl/gl.hpp>
#include <mbgl/gl/context.hpp>

namespace mbgl {

using namespace style;

TerrainBucket::TerrainBucket(PremultipliedImage&&) {
    // TODO
}

TerrainBucket::TerrainBucket(DEMPyramid&& pyramid_) : pyramid(std::move(pyramid_)) {
}

void TerrainBucket::upload(gl::Context& context) {
    if (!pyramid.levels.empty()) {
        texture = context.createTexture(pyramid.levels.front().image);
        for (size_t l = 1; l < pyramid.levels.size(); l++) {
            auto& image = pyramid.levels[l].image;
            MBGL_CHECK_ERROR(glTexImage2D(GL_TEXTURE_2D, l, GL_RGBA, image.width, image.height, 0,
                                          GL_RGBA, GL_UNSIGNED_BYTE, image.data.get()));
        }
    }


//    if (!pyramid.levels.empty()) {
//        texture = context.createTexture(pyramid.levels[4].image);
//    }
//    prepared = true;

    uploaded = true;
}

bool TerrainBucket::needsPrepare() const {
    return !prepared;
}

void TerrainBucket::setPreparedTexture(gl::Texture&& t) {
    texture = std::move(t);
    prepared = true;
}

void TerrainBucket::render(Painter& painter,
                           PaintParameters& parameters,
                           const Layer& layer,
                           const RenderTile& tile) {
    painter.renderTerrain(parameters, *this, *layer.as<TerrainLayer>(), tile);
}

void TerrainBucket::drawTerrainPrepare(TerrainPrepareShader& shader,
                                gl::VertexBuffer<RasterVertex>& vertices,
                                gl::VertexArrayObject& array,
                                gl::Context& context) {
    assert(texture);
    context.bindTexture(*texture, 0, gl::TextureFilter::Linear, gl::TextureMipMap::Yes);
    array.bind(shader, vertices, BUFFER_OFFSET_0, context);
    MBGL_CHECK_ERROR(
        glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(vertices.vertexCount)));
}

void TerrainBucket::drawTerrain(TerrainShader& shader,
                                gl::VertexBuffer<RasterVertex>& vertices,
                                gl::VertexArrayObject& array,
                                gl::Context& context) {
    assert(texture);
    context.bindTexture(*texture, 0, gl::TextureFilter::Linear);
    array.bind(shader, vertices, BUFFER_OFFSET_0, context);
    MBGL_CHECK_ERROR(
        glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(vertices.vertexCount)));
}

bool TerrainBucket::hasData() const {
    return true;
}

bool TerrainBucket::needsClipping() const {
    return true;
}

} // namespace mbgl
