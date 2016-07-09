#include <mbgl/style/layers/custom_layer_impl.hpp>
#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/map/transform_state.hpp>

namespace mbgl {
namespace style {

CustomLayer::Impl::Impl(const std::string& id_,
                         CustomLayerInitializeFunction initializeFn_,
                         CustomLayerRenderFunction renderFn_,
                         CustomLayerDeinitializeFunction deinitializeFn_,
                         void* context_) {
    id = id_;
    initializeFn = initializeFn_;
    renderFn = renderFn_;
    deinitializeFn = deinitializeFn_;
    context = context_;
}

CustomLayer::Impl::Impl(const CustomLayer::Impl& other)
    : Layer::Impl(other) {
    id = other.id;
    // Don't copy anything else.
}

CustomLayer::Impl::~Impl() {
    if (deinitializeFn) {
        deinitializeFn(context);
    }
}

std::unique_ptr<Layer> CustomLayer::Impl::clone() const {
    return std::make_unique<CustomLayer>(*this);
}

std::unique_ptr<Layer> CustomLayer::Impl::cloneRef(const std::string&) const {
    assert(false);
    return std::make_unique<CustomLayer>(*this);
}

void CustomLayer::Impl::initialize() {
    assert(initializeFn);
    initializeFn(context);
}

void CustomLayer::Impl::render(PaintParameters& parameters_) const {
    assert(renderFn);

    CustomLayerRenderParameters parameters;

    parameters.width = parameters_.state.getWidth();
    parameters.height = parameters_.state.getHeight();
    parameters.latitude = parameters_.state.getLatLng().latitude;
    parameters.longitude = parameters_.state.getLatLng().longitude;
    parameters.zoom = parameters_.state.getZoom();
    parameters.bearing = -parameters_.state.getAngle() * util::RAD2DEG;
    parameters.pitch = parameters_.state.getPitch();
    parameters.altitude = parameters_.state.getAltitude();

    renderFn(context, parameters);
}

bool CustomLayer::Impl::recalculate(const CalculationParameters&) {
    passes = RenderPass::Translucent;
    return false;
}

std::unique_ptr<Bucket> CustomLayer::Impl::createBucket(BucketParameters&) const {
    return nullptr;
}

} // namespace style
} // namespace mbgl
