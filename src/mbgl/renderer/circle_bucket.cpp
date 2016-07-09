#include <mbgl/renderer/circle_bucket.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tile.hpp>

#include <mbgl/shader/shaders.hpp>
#include <mbgl/map/transform_state.hpp>

#include <mbgl/style/layers/circle_layer.hpp>
#include <mbgl/style/layers/circle_layer_impl.hpp>

#include <mbgl/util/constants.hpp>

namespace mbgl {

using namespace style;

CircleBucket::CircleBucket(MapMode mode_) : mode(mode_) {
}

CircleBucket::~CircleBucket() {
    // Do not remove. header file only contains forward definitions to unique pointers.
}

void CircleBucket::upload(gl::ObjectStore& store, gl::Config&) {
    vertexBuffer_.upload(store);
    elementsBuffer_.upload(store);
    uploaded = true;
}

bool CircleBucket::hasData() const {
    return !triangleGroups_.empty();
}

void CircleBucket::addGeometry(const GeometryCollection& geometryCollection) {
    for (auto& circle : geometryCollection) {
        for(auto & geometry : circle) {
            auto x = geometry.x;
            auto y = geometry.y;

            // Do not include points that are outside the tile boundaries.
            // Include all points in Still mode. You need to include points from
            // neighbouring tiles so that they are not clipped at tile boundaries.
            if ((mode != MapMode::Still) &&
                (x < 0 || x >= util::EXTENT || y < 0 || y >= util::EXTENT)) continue;

            // this geometry will be of the Point type, and we'll derive
            // two triangles from it.
            //
            // ┌─────────┐
            // │ 4     3 │
            // │         │
            // │ 1     2 │
            // └─────────┘
            //
            vertexBuffer_.add(x, y, -1, -1); // 1
            vertexBuffer_.add(x, y, 1, -1); // 2
            vertexBuffer_.add(x, y, 1, 1); // 3
            vertexBuffer_.add(x, y, -1, 1); // 4

            if (!triangleGroups_.size() || (triangleGroups_.back()->vertex_length + 4 > 65535)) {
                // Move to a new group because the old one can't hold the geometry.
                triangleGroups_.emplace_back(std::make_unique<TriangleGroup>());
            }

            TriangleGroup& group = *triangleGroups_.back();
            auto index = group.vertex_length;

            // 1, 2, 3
            // 1, 4, 3
            elementsBuffer_.add(index, index + 1, index + 2);
            elementsBuffer_.add(index, index + 3, index + 2);

            group.vertex_length += 4;
            group.elements_length += 2;
        }
    }
}

void CircleBucket::render(PaintParameters& parameters,
                          RenderPass pass,
                          const RenderTile& tile,
                          const CircleLayer& layer) {
    if (pass == RenderPass::Opaque) return;

    parameters.config.stencilTest = mode == MapMode::Still ? GL_TRUE : GL_FALSE;
    parameters.config.depthFunc.reset();
    parameters.config.depthTest = GL_TRUE;
    parameters.config.depthMask = GL_FALSE;
    parameters.setDepthSublayer(0);
    parameters.setClipping(tile.clip);

    const CirclePaintProperties& paint = layer.impl->paint;
    auto& shader = parameters.shaders.circle;

    parameters.config.program = shader.getID();

    shader.u_matrix = tile.translatedMatrix(paint.circleTranslate,
                                            paint.circleTranslateAnchor,
                                            parameters.state);

    if (paint.circlePitchScale == CirclePitchScaleType::Map) {
        shader.u_extrude_scale = {{
            parameters.pixelsToGLUnits[0] * parameters.state.getAltitude(),
            parameters.pixelsToGLUnits[1] * parameters.state.getAltitude()
        }};
        shader.u_scale_with_map = true;
    } else {
        shader.u_extrude_scale = parameters.pixelsToGLUnits;
        shader.u_scale_with_map = false;
    }

    shader.u_devicepixelratio = parameters.pixelRatio;
    shader.u_color = paint.circleColor;
    shader.u_radius = paint.circleRadius;
    shader.u_blur = paint.circleBlur;
    shader.u_opacity = paint.circleOpacity;

    GLbyte* vertexIndex = BUFFER_OFFSET(0);
    GLbyte* elementsIndex = BUFFER_OFFSET(0);

    for (auto& group : triangleGroups_) {
        assert(group);

        if (!group->elements_length) continue;

        group->array[0].bind(shader, vertexBuffer_, elementsBuffer_, vertexIndex, parameters.store);

        MBGL_CHECK_ERROR(glDrawElements(GL_TRIANGLES, group->elements_length * 3, GL_UNSIGNED_SHORT, elementsIndex));

        vertexIndex += group->vertex_length * vertexBuffer_.itemSize;
        elementsIndex += group->elements_length * elementsBuffer_.itemSize;
    }
}

} // namespace mbgl
