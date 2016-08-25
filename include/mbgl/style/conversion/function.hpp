#pragma once

#include <mbgl/style/function.hpp>
#include <mbgl/style/conversion.hpp>
#include <mbgl/style/conversion/constant.hpp>

namespace mbgl {
namespace style {
namespace conversion {

template <class T>
struct Converter<Function<T>> {
    template <class V>
    Result<Function<T>> operator()(const V& value) const {
        if (!isObject(value)) {
            return Error { "function must be an object" };
        }

        auto stopsValue = objectMember(value, "stops");
        if (!stopsValue) {
            return Error { "function value must specify stops" };
        }

        if (!isArray(*stopsValue)) {
            return Error { "function stops must be an array" };
        }

        optional<std::string> property;
        const auto propertyValue = objectMember(value, "property");
        if (propertyValue) {
            property = toString(*propertyValue);
            if (!property) {
                return Error { "function property must be a string" };
            }
        }

        using Type = typename Function<T>::Type;

        Type type = Type::Exponential;
        const auto typeValue = objectMember(value, "type");
        if (typeValue) {
            optional<std::string> typeString = toString(*typeValue);
            if (!typeString) {
                return Error { "function type must be a string" };
            } else if (*typeString == "interval") {
                type = Type::Interval;
            } else if (*typeString == "categorical") {
                type = Type::Categorical;
            } else if (*typeString != "exponential") {
                return Error { "function type is invalid" };
            }
        }

        std::vector<std::pair<float, T>> stops;
        const std::size_t length = arrayLength(*stopsValue);
        for (std::size_t i = 0; i < length; ++i) {
            const auto& stopValue = arrayMember(*stopsValue, i);

            if (!isArray(stopValue)) {
                return Error { "function stop must be an array" };
            }

            if (arrayLength(stopValue) != 2) {
                return Error { "function stop must have two elements" };
            }

            optional<float> z = toNumber(arrayMember(stopValue, 0));
            if (!z) {
                return Error { "function stop zoom level must be a number" };
            }

            Result<T> v = convert<T>(arrayMember(stopValue, 1));
            if (!v) {
                return v.error();
            }

            stops.emplace_back(*z, *v);
        }

        auto baseValue = objectMember(value, "base");
        if (!baseValue) {
            return Function<T>(stops, 1.0f, type);
        }

        optional<float> base = toNumber(*baseValue);
        if (!base) {
            return Error { "function base must be a number"};
        }

        return Function<T>(stops, *base, type);
    }
};

} // namespace conversion
} // namespace style
} // namespace mbgl
