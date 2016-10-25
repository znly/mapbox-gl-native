#pragma once

#include <mbgl/util/optional.hpp>

#include <mapbox/geometry/feature.hpp>

#include <unordered_map>

namespace mbgl {

using Value = mapbox::geometry::value;
using NullValue = mapbox::geometry::null_value_t;
using PropertyMap = mapbox::geometry::property_map;
using FeatureIdentifier = mapbox::geometry::identifier;
using Feature = mapbox::geometry::feature<double>;
using LayerFeaturesHash = std::unordered_map<std::string, std::vector<Feature>>;

} // namespace mbgl
