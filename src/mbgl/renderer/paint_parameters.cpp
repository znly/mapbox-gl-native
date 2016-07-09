#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/clip_id.hpp>

namespace mbgl {

void PaintParameters::setClipping(const ClipID& clip) {
    const GLint ref = (GLint)clip.reference.to_ulong();
    const GLuint mask = (GLuint)clip.mask.to_ulong();
    config.stencilFunc = { GL_EQUAL, ref, mask };
}

void PaintParameters::setDepthSublayer(int n) {
    float nearDepth = ((1 + currentLayer) * numSublayers + n) * depthEpsilon;
    float farDepth = nearDepth + depthRangeSize;
    config.depthRange = { nearDepth, farDepth };
}

} // namespace mbgl
