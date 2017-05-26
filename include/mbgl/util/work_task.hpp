#pragma once

#include <mbgl/util/noncopyable.hpp>

#include <memory>

namespace mbgl {

class WorkTask : private util::noncopyable {
public:
    virtual ~WorkTask() = default;

    virtual void operator()() = 0;
    virtual void cancel() = 0;

    template <class Fn, class... Args>
    static std::shared_ptr<WorkTask> make(Fn&&, Args&&...);

    template <class Fn, class... Args>
    static std::shared_ptr<WorkTask> makeWithCallback(Fn&&, Args&&...);
};

} // namespace mbgl
