#pragma once

#include <exception>

namespace mbgl {
namespace event {

struct Event {};

struct RegionChangeEvent: public Event {
    bool animated = false;
};
struct RegionWillChangeEvent: public RegionChangeEvent {};
struct RegionIsChangingEvent: public RegionChangeEvent {};
struct RegionDidChangeEvent: public RegionChangeEvent {};

struct MapLoadEvent: public Event {};
struct MapWillStartLoading: public MapLoadEvent {};
struct MapDidFinishLoading: public MapLoadEvent {};
struct MapDidFailLoading: public MapLoadEvent {
    std::exception_ptr error;
};

struct FrameRenderEvent: public Event {};
struct FrameWillStartRendering: public FrameRenderEvent {};
struct FrameDidFinishRendering: public FrameRenderEvent {
    bool fullyRendered = false;
};

struct MapRenderEvent: public Event {};
struct MapWillStartRendering: public MapRenderEvent {};
struct MapDidFinishRendering: public MapRenderEvent {
    bool fullyRendered = false;
};

}
} // namespace mbgl
