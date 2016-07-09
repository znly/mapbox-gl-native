#include <mbgl/renderer/fill_bucket.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/style/layers/fill_layer.hpp>
#include <mbgl/style/layers/fill_layer_impl.hpp>
#include <mbgl/shader/shaders.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/sprite/sprite_atlas.hpp>
#include <mbgl/gl/gl.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/platform/log.hpp>

#include <mapbox/earcut.hpp>

#include <cassert>

namespace mapbox {
namespace util {
template <> struct nth<0, mbgl::GeometryCoordinate> {
    static int64_t get(const mbgl::GeometryCoordinate& t) { return t.x; };
};

template <> struct nth<1, mbgl::GeometryCoordinate> {
    static int64_t get(const mbgl::GeometryCoordinate& t) { return t.y; };
};
} // namespace util
} // namespace mapbox

namespace mbgl {

using namespace style;

struct GeometryTooLongException : std::exception {};

FillBucket::FillBucket() {
}

FillBucket::~FillBucket() = default;

void FillBucket::addGeometry(const GeometryCollection& geometry) {
    for (auto& polygon : classifyRings(geometry)) {
        // Optimize polygons with many interior rings for earcut tesselation.
        limitHoles(polygon, 500);

        std::size_t totalVertices = 0;

        for (const auto& ring : polygon) {
            totalVertices += ring.size();
            if (totalVertices > 65535)
                throw GeometryTooLongException();
        }

        for (const auto& ring : polygon) {
            std::size_t nVertices = ring.size();

            if (nVertices == 0)
                continue;

            if (lineGroups.empty() || lineGroups.back()->vertex_length + nVertices > 65535)
                lineGroups.emplace_back(std::make_unique<LineGroup>());

            LineGroup& lineGroup = *lineGroups.back();
            GLsizei lineIndex = lineGroup.vertex_length;

            vertexBuffer.add(ring[0].x, ring[0].y);
            lineElementsBuffer.add(lineIndex + nVertices - 1, lineIndex);

            for (uint32_t i = 1; i < nVertices; i++) {
                vertexBuffer.add(ring[i].x, ring[i].y);
                lineElementsBuffer.add(lineIndex + i - 1, lineIndex + i);
            }

            lineGroup.vertex_length += nVertices;
            lineGroup.elements_length += nVertices;
        }

        std::vector<uint32_t> indices = mapbox::earcut(polygon);

        std::size_t nIndicies = indices.size();
        assert(nIndicies % 3 == 0);

        if (triangleGroups.empty() || triangleGroups.back()->vertex_length + totalVertices > 65535) {
            triangleGroups.emplace_back(std::make_unique<TriangleGroup>());
        }

        TriangleGroup& triangleGroup = *triangleGroups.back();
        GLsizei triangleIndex = triangleGroup.vertex_length;

        for (uint32_t i = 0; i < nIndicies; i += 3) {
            triangleElementsBuffer.add(triangleIndex + indices[i],
                                       triangleIndex + indices[i + 1],
                                       triangleIndex + indices[i + 2]);
        }

        triangleGroup.vertex_length += totalVertices;
        triangleGroup.elements_length += nIndicies / 3;
    }
}

void FillBucket::upload(gl::ObjectStore& store, gl::Config&) {
    vertexBuffer.upload(store);
    triangleElementsBuffer.upload(store);
    lineElementsBuffer.upload(store);

    // From now on, we're going to render during the opaque and translucent pass.
    uploaded = true;
}

bool FillBucket::hasData() const {
    return !triangleGroups.empty() || !lineGroups.empty();
}

void FillBucket::render(PaintParameters& parameters,
                        RenderPass pass,
                        const RenderTile& tile,
                        const FillLayer& layer) {
    const FillPaintProperties& properties = layer.impl->paint;
    mat4 vertexMatrix = tile.translatedMatrix(properties.fillTranslate,
                                              properties.fillTranslateAnchor,
                                              parameters.state);

    Color fillColor = properties.fillColor;
    float opacity = properties.fillOpacity;

    const bool isOutlineColorDefined = !properties.fillOutlineColor.isUndefined();
    Color strokeColor = isOutlineColorDefined? properties.fillOutlineColor : fillColor;

    auto worldSize = util::convert<GLfloat>(parameters.framebufferSize);

    bool pattern = !properties.fillPattern.value.from.empty();
    bool outline = properties.fillAntialias && !pattern && isOutlineColorDefined;
    bool fringeline = properties.fillAntialias && !pattern && !isOutlineColorDefined;

    parameters.config.stencilOp.reset();
    parameters.config.stencilTest = GL_TRUE;
    parameters.config.depthFunc.reset();
    parameters.config.depthTest = GL_TRUE;
    parameters.config.depthMask = GL_TRUE;
    parameters.config.lineWidth = 2.0f; // This is always fixed and does not depend on the pixelRatio!
    parameters.setClipping(tile.clip);

    auto& outlineShader = parameters.shaders.outline;
    auto& patternShader = parameters.shaders.pattern;
    auto& outlinePatternShader = parameters.shaders.outlinePattern;
    auto& plainShader = parameters.shaders.plain;

    // Because we're drawing top-to-bottom, and we update the stencil mask
    // befrom, we have to draw the outline first (!)
    if (outline && pass == RenderPass::Translucent) {
        parameters.config.program = outlineShader.getID();
        outlineShader.u_matrix = vertexMatrix;

        outlineShader.u_outline_color = strokeColor;
        outlineShader.u_opacity = opacity;

        // Draw the entire line
        outlineShader.u_world = worldSize;
        if (isOutlineColorDefined) {
            // If we defined a different color for the fill outline, we are
            // going to ignore the bits in 0x07 and just care about the global
            // clipping mask.
            parameters.setDepthSublayer(2); // OK
        } else {
            // Otherwise, we only want to drawFill the antialiased parts that are
            // *outside* the current shape. This is important in case the fill
            // or stroke color is translucent. If we wouldn't clip to outside
            // the current shape, some pixels from the outline stroke overlapped
            // the (non-antialiased) fill.
            parameters.setDepthSublayer(0); // OK
        }
        drawVertices(outlineShader, parameters.store, parameters.overdraw);
    }

    if (pattern) {
        optional<SpriteAtlasPosition> imagePosA = parameters.spriteAtlas.getPosition(properties.fillPattern.value.from, true);
        optional<SpriteAtlasPosition> imagePosB = parameters.spriteAtlas.getPosition(properties.fillPattern.value.to, true);

        // Image fill.
        if (pass == RenderPass::Translucent && imagePosA && imagePosB) {
            parameters.config.program = patternShader.getID();
            patternShader.u_matrix = vertexMatrix;
            patternShader.u_pattern_tl_a = imagePosA->tl;
            patternShader.u_pattern_br_a = imagePosA->br;
            patternShader.u_pattern_tl_b = imagePosB->tl;
            patternShader.u_pattern_br_b = imagePosB->br;
            patternShader.u_opacity = properties.fillOpacity;
            patternShader.u_image = 0;
            patternShader.u_mix = properties.fillPattern.value.t;
            patternShader.u_pattern_size_a = imagePosA->size;
            patternShader.u_pattern_size_b = imagePosB->size;
            patternShader.u_scale_a = properties.fillPattern.value.fromScale;
            patternShader.u_scale_b = properties.fillPattern.value.toScale;
            patternShader.u_tile_units_to_pixels = 1.0f / tile.id.pixelsToTileUnits(1.0f, parameters.state.getIntegerZoom());

            GLint tileSizeAtNearestZoom = util::tileSize * parameters.state.zoomScale(parameters.state.getIntegerZoom() - tile.id.canonical.z);
            GLint pixelX = tileSizeAtNearestZoom * (tile.id.canonical.x + tile.id.wrap * parameters.state.zoomScale(tile.id.canonical.z));
            GLint pixelY = tileSizeAtNearestZoom * tile.id.canonical.y;
            patternShader.u_pixel_coord_upper = {{ float(pixelX >> 16), float(pixelY >> 16) }};
            patternShader.u_pixel_coord_lower = {{ float(pixelX & 0xFFFF), float(pixelY & 0xFFFF) }};

            parameters.spriteAtlas.bind(true, parameters.store, parameters.config, 0);

            // Draw the actual triangles into the color & stencil buffer.
            parameters.setDepthSublayer(0);
            drawElements(patternShader, parameters.store, parameters.overdraw);

            if (properties.fillAntialias && !isOutlineColorDefined) {
                parameters.config.program = outlinePatternShader.getID();
                outlinePatternShader.u_matrix = vertexMatrix;

                outlinePatternShader.u_pattern_tl_a = imagePosA->tl;
                outlinePatternShader.u_pattern_br_a = imagePosA->br;
                outlinePatternShader.u_pattern_tl_b = imagePosB->tl;
                outlinePatternShader.u_pattern_br_b = imagePosB->br;
                outlinePatternShader.u_opacity = properties.fillOpacity;
                outlinePatternShader.u_image = 0;
                outlinePatternShader.u_mix = properties.fillPattern.value.t;
                outlinePatternShader.u_pattern_size_a = imagePosA->size;
                outlinePatternShader.u_pattern_size_b = imagePosB->size;
                outlinePatternShader.u_scale_a = properties.fillPattern.value.fromScale;
                outlinePatternShader.u_scale_b = properties.fillPattern.value.toScale;
                outlinePatternShader.u_tile_units_to_pixels = 1.0f / tile.id.pixelsToTileUnits(1.0f, parameters.state.getIntegerZoom());
                outlinePatternShader.u_pixel_coord_upper = {{ float(pixelX >> 16), float(pixelY >> 16) }};
                outlinePatternShader.u_pixel_coord_lower = {{ float(pixelX & 0xFFFF), float(pixelY & 0xFFFF) }};

                // Draw the entire line
                outlinePatternShader.u_world = worldSize;

                parameters.spriteAtlas.bind(true, parameters.store, parameters.config, 0);

                parameters.setDepthSublayer(2);
                drawVertices(outlinePatternShader, parameters.store, parameters.overdraw);
            }
        }
    } else {
        // No image fill.
        if ((fillColor.a >= 1.0f && opacity >= 1.0f) == (pass == RenderPass::Opaque)) {
            // Only draw the fill when it's either opaque and we're drawing opaque
            // fragments or when it's translucent and we're drawing translucent
            // fragments
            // Draw filling rectangle.
            parameters.config.program = plainShader.getID();
            plainShader.u_matrix = vertexMatrix;
            plainShader.u_color = fillColor;
            plainShader.u_opacity = opacity;

            // Draw the actual triangles into the color & stencil buffer.
            parameters.setDepthSublayer(1);
            drawElements(plainShader, parameters.store, parameters.overdraw);
        }
    }

    // Because we're drawing top-to-bottom, and we update the stencil mask
    // below, we have to draw the outline first (!)
    if (fringeline && pass == RenderPass::Translucent) {
        parameters.config.program = outlineShader.getID();
        outlineShader.u_matrix = vertexMatrix;

        outlineShader.u_outline_color = fillColor;
        outlineShader.u_opacity = opacity;

        // Draw the entire line
        outlineShader.u_world = worldSize;

        parameters.setDepthSublayer(2);
        drawVertices(outlineShader, parameters.store, parameters.overdraw);
    }
}

void FillBucket::drawElements(PlainShader& shader, gl::ObjectStore& store, bool overdraw) {
    GLbyte* vertex_index = BUFFER_OFFSET(0);
    GLbyte* elements_index = BUFFER_OFFSET(0);
    for (auto& group : triangleGroups) {
        assert(group);
        group->array[overdraw ? 1 : 0].bind(shader, vertexBuffer, triangleElementsBuffer, vertex_index, store);
        MBGL_CHECK_ERROR(glDrawElements(GL_TRIANGLES, group->elements_length * 3, GL_UNSIGNED_SHORT, elements_index));
        vertex_index += group->vertex_length * vertexBuffer.itemSize;
        elements_index += group->elements_length * triangleElementsBuffer.itemSize;
    }
}

void FillBucket::drawElements(PatternShader& shader, gl::ObjectStore& store, bool overdraw) {
    GLbyte* vertex_index = BUFFER_OFFSET(0);
    GLbyte* elements_index = BUFFER_OFFSET(0);
    for (auto& group : triangleGroups) {
        assert(group);
        group->array[overdraw ? 3 : 2].bind(shader, vertexBuffer, triangleElementsBuffer, vertex_index, store);
        MBGL_CHECK_ERROR(glDrawElements(GL_TRIANGLES, group->elements_length * 3, GL_UNSIGNED_SHORT, elements_index));
        vertex_index += group->vertex_length * vertexBuffer.itemSize;
        elements_index += group->elements_length * triangleElementsBuffer.itemSize;
    }
}

void FillBucket::drawVertices(OutlineShader& shader, gl::ObjectStore& store, bool overdraw) {
    GLbyte* vertex_index = BUFFER_OFFSET(0);
    GLbyte* elements_index = BUFFER_OFFSET(0);
    for (auto& group : lineGroups) {
        assert(group);
        group->array[overdraw ? 1 : 0].bind(shader, vertexBuffer, lineElementsBuffer, vertex_index, store);
        MBGL_CHECK_ERROR(glDrawElements(GL_LINES, group->elements_length * 2, GL_UNSIGNED_SHORT, elements_index));
        vertex_index += group->vertex_length * vertexBuffer.itemSize;
        elements_index += group->elements_length * lineElementsBuffer.itemSize;
    }
}

void FillBucket::drawVertices(OutlinePatternShader& shader, gl::ObjectStore& store, bool overdraw) {
    GLbyte* vertex_index = BUFFER_OFFSET(0);
    GLbyte* elements_index = BUFFER_OFFSET(0);
    for (auto& group : lineGroups) {
        assert(group);
        group->array[overdraw? 3 : 2].bind(shader, vertexBuffer, lineElementsBuffer, vertex_index, store);
        MBGL_CHECK_ERROR(glDrawElements(GL_LINES, group->elements_length * 2, GL_UNSIGNED_SHORT, elements_index));
        vertex_index += group->vertex_length * vertexBuffer.itemSize;
        elements_index += group->elements_length * lineElementsBuffer.itemSize;
    }
}

} // namespace mbgl
