package com.mapbox.mapboxsdk.style.sources;

import com.google.gson.Gson;
import com.mapbox.services.commons.geojson.FeatureCollection;

import java.net.URL;
import java.util.HashMap;

/**
 * A GeoJson source.
 *
 * @see <a href="https://www.mapbox.com/mapbox-gl-style-spec/#sources-geojson">the style specification</a>
 */
public class GeoJsonSource extends Source {

    /**
     * Create an empty GeoJsonSource
     *
     * @param id the source id
     */
    public GeoJsonSource(String id) {
        initialize(id);
    }

    /**
     * Create a GeoJsonSource from a raw json string
     *
     * @param id      the source id
     * @param geoJson raw Json body
     */
    public GeoJsonSource(String id, String geoJson) {
        if (geoJson == null || geoJson.startsWith("http")) {
            throw new IllegalArgumentException("Expected a raw json body");
        }
        initialize(id);
        nativeSetGeoJson(geoJson);
    }

    /**
     * Create a GeoJsonSource from a remote geo json file
     *
     * @param id  the source id
     * @param url remote json file
     */
    public GeoJsonSource(String id, URL url) {
        initialize(id);
        nativeSetUrl(url.toExternalForm());
    }

    /**
     * Create a GeoJsonSource from a FeatureCollection
     *
     * @param id       the source id
     * @param features the features
     */
    public GeoJsonSource(String id, FeatureCollection features) {
        initialize(id);
        nativeSetGeoJson(features.toJson());
    }

    public void setGeoJson(FeatureCollection features) {
        nativeSetGeoJson(features.toJson());
    }

    public void setGeoJson(String json) {
        nativeSetGeoJson(json);
    }

    public void setUrl(URL url) {
        nativeSetUrl(url.toExternalForm());
    }

    public void setUrl(String url) {
        nativeSetUrl(url);
    }

    public GeoJsonSource withCluster(boolean cluster) {
        //TODO this.put("cluster", cluster);
        return this;
    }

    public GeoJsonSource withClusterMaxZoom(float zoom) {
        //TODO this.put("clusterMaxZoom", zoom);
        return this;
    }

    public GeoJsonSource withClusterRadius(float radius) {
        //TODO this.put("clusterRadius", radius);
        return this;
    }

    protected native void initialize(String layerId);

    protected native void nativeSetUrl(String url);

    protected native void nativeSetGeoJson(String geoJson);

    @Override
    protected native void finalize() throws Throwable;

}
