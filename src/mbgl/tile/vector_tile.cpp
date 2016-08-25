#include <mbgl/tile/vector_tile.hpp>
#include <mbgl/tile/tile_loader_impl.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/style/update_parameters.hpp>
#include <mapbox/vector_tile.hpp>
#include <protozero/types.hpp>

#include <map>
#include <unordered_map>
#include <functional>
#include <utility>

namespace mbgl {

class VectorTileLayer;

class VectorTileFeature : public GeometryTileFeature {
public:
    using feature_type = std::unique_ptr<const mapbox::vector_tile::feature>;
    VectorTileFeature(feature_type feature);

    FeatureType getType() const override { return static_cast<FeatureType>(tile_feature->getType()); }
    optional<Value> getValue(const std::string&) const override;
    std::unordered_map<std::string,Value> getProperties() const override;
    optional<FeatureIdentifier> getID() const override;
    GeometryCollection getGeometries() const override;

private:
    feature_type tile_feature;
};

class VectorTileLayer : public GeometryTileLayer {
public:
    VectorTileLayer(const protozero::data_view &);

    std::size_t featureCount() const override { return tile_layer.featureCount(); }
    util::ptr<const GeometryTileFeature> getFeature(std::size_t) const override;
    std::string getName() const override;

private:
    friend class VectorTileData;
    friend class VectorTileFeature;
    mapbox::vector_tile::layer tile_layer;
};

class VectorTileData : public GeometryTileData {
public:
    VectorTileData(std::shared_ptr<const std::string> data);

    util::ptr<GeometryTileLayer> getLayer(const std::string&) const override;

private:
    std::shared_ptr<const std::string> data;
    mutable bool parsed = false;
    mutable std::map<std::string, util::ptr<GeometryTileLayer>> layers;
};

VectorTile::VectorTile(const OverscaledTileID& id_,
                       std::string sourceID_,
                       const style::UpdateParameters& parameters,
                       const Tileset& tileset)
    : GeometryTile(id_, sourceID_, parameters.style, parameters.mode),
      loader(*this, id_, parameters, tileset) {
}

void VectorTile::setNecessity(Necessity necessity) {
    loader.setNecessity(necessity);
}

void VectorTile::setData(std::shared_ptr<const std::string> data_,
                         optional<Timestamp> modified_,
                         optional<Timestamp> expires_) {
    modified = modified_;
    expires = expires_;

    GeometryTile::setData(data_ ? std::make_unique<VectorTileData>(data_) : nullptr);
}

VectorTileFeature::VectorTileFeature(feature_type feature)
    : tile_feature(std::move(feature)) { }

optional<Value> VectorTileFeature::getValue(const std::string& key) const {
    return tile_feature->getValue(key);
}

std::unordered_map<std::string,Value> VectorTileFeature::getProperties() const {
    return tile_feature->getProperties();
}

optional<FeatureIdentifier> VectorTileFeature::getID() const {
    return tile_feature->getID();
}

GeometryCollection VectorTileFeature::getGeometries() const {
    const float scale = float(util::EXTENT) / tile_feature->getExtent();
    GeometryCollection lines = tile_feature->getGeometries<GeometryCollection>(scale);
    if (tile_feature->getVersion() >= 2 || tile_feature->getType() != mapbox::vector_tile::GeomType::POLYGON) {
        return lines;
    }
    return fixupPolygons(lines);
}

VectorTileData::VectorTileData(std::shared_ptr<const std::string> data_)
    : data(std::move(data_)) {
}

util::ptr<GeometryTileLayer> VectorTileData::getLayer(const std::string& name) const {
    if (!parsed) {
        parsed = true;
        mapbox::vector_tile::buffer tile_buffer(*data);
        for (auto const& tile_layer : tile_buffer.getLayers()) {
            layers.emplace(tile_layer.first, std::make_shared<VectorTileLayer>(tile_layer.second));
        }
    }

    auto layer_it = layers.find(name);
    if (layer_it != layers.end()) {
        return layer_it->second;
    }

    return nullptr;
}

VectorTileLayer::VectorTileLayer(const protozero::data_view & layer_view) :
    tile_layer(layer_view) { }

util::ptr<const GeometryTileFeature> VectorTileLayer::getFeature(std::size_t i) const {
    return std::make_shared<VectorTileFeature>(tile_layer.getFeature(i));
}

std::string VectorTileLayer::getName() const {
    return tile_layer.getName();
}

} // namespace mbgl
