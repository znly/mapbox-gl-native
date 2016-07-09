#pragma once

#include <mbgl/renderer/bucket.hpp>
#include <mbgl/map/mode.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/geometry/elements_buffer.hpp>
#include <mbgl/geometry/circle_buffer.hpp>

namespace mbgl {

class PaintParameters;

namespace style {
class CircleLayer;
} // namespace style

class CircleBucket : public Bucket {
    using TriangleGroup = ElementGroup<3>;

public:
    CircleBucket(const MapMode);
    ~CircleBucket() override;

    void upload(gl::ObjectStore&, gl::Config&) override;

    bool hasData() const override;
    void addGeometry(const GeometryCollection&);

    void render(PaintParameters&,
                RenderPass,
                const RenderTile&,
                const style::CircleLayer&);

private:
    CircleVertexBuffer vertexBuffer_;
    TriangleElementsBuffer elementsBuffer_;

    std::vector<std::unique_ptr<TriangleGroup>> triangleGroups_;

    const MapMode mode;
};

} // namespace mbgl
