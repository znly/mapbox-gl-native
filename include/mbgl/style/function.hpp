#pragma once

#include <vector>
#include <utility>

namespace mbgl {
namespace style {

template <typename T>
class Function {
public:
    using Stop = std::pair<float, T>;
    using Stops = std::vector<Stop>;

    enum class Type : uint8_t {
        Exponential = 0,
        Interval,
        Categorical,
    };

    explicit Function(Stops stops_, float base_ = 1.0f, Type type_ = Type::Exponential)
        : base(base_), type(type_), stops(std::move(stops_)) {}

    float getBase() const { return base; }
    const Stops& getStops() const { return stops; }

private:
    float base;
    Type type;
    Stops stops;
};

} // namespace style
} // namespace mbgl
