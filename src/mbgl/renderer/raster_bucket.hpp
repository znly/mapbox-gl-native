#pragma once

#include <mbgl/renderer/bucket.hpp>
#include <mbgl/util/raster.hpp>

namespace mbgl {

namespace style {
class RasterLayer;
} // namespace style

class RasterBucket : public Bucket {
public:
    void upload(gl::ObjectStore&, gl::Config&) override;
    bool hasData() const override;

    void setImage(PremultipliedImage);

    void render(PaintParameters&,
                RenderPass,
                const RenderTile&,
                const style::RasterLayer&);

    Raster raster;
};

} // namespace mbgl
