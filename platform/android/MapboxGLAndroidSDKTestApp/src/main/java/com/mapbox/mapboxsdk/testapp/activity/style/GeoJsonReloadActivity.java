package com.mapbox.mapboxsdk.testapp.activity.style;

import android.animation.TypeEvaluator;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.MenuItem;

import com.mapbox.mapboxsdk.constants.MapboxConstants;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.maps.MapView;
import com.mapbox.mapboxsdk.maps.MapboxMap;
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback;
import com.mapbox.mapboxsdk.style.layers.SymbolLayer;
import com.mapbox.mapboxsdk.style.sources.GeoJsonSource;
import com.mapbox.mapboxsdk.style.sources.Source;
import com.mapbox.mapboxsdk.testapp.R;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;

import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.iconImage;

public class GeoJsonReloadActivity extends AppCompatActivity {

    private MapView mapView;
    private MapboxMap mapboxMap;
    private Handler handler = new Handler();
    private URL url;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_animated_marker);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(true);
            actionBar.setDisplayShowHomeEnabled(true);
        }

        try {
            url = new URL("https://wanderdrone.appspot.com/");
        } catch (MalformedURLException e) {

        }

        mapView = (MapView) findViewById(R.id.mapView);
        mapView.onCreate(savedInstanceState);
        mapView.getMapAsync(new OnMapReadyCallback() {

            @Override
            public void onMapReady(@NonNull final MapboxMap mapboxMap) {
                GeoJsonReloadActivity.this.mapboxMap = mapboxMap;
                mapboxMap.addSource(new GeoJsonSource("drone", url));
                SymbolLayer layer = new SymbolLayer("point", "drone");
                layer.setProperties(iconImage("marker-15"));
                mapboxMap.addLayer(layer);
                reloadSourceRecursivly();
            }
        });
    }

    private void reloadSourceRecursivly() {
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (mapboxMap != null) {
                    Source source = mapboxMap.getSource("drone");
                    if (source != null && source instanceof GeoJsonSource) {
                        new LoadLocationPointTask(source).execute();
                    }
                }
            }
        }, 2000);
    }

    private class LoadLocationPointTask extends AsyncTask<Void, Void, String> {

        private Source source;

        public LoadLocationPointTask(Source source) {
            this.source = source;
        }

        @Override
        protected String doInBackground(Void... params) {
            try {
                OkHttpClient client = new OkHttpClient();
                Request request = new Request.Builder()
                        .url(url)
                        .build();
                Response responses = null;

                try {
                    responses = client.newCall(request).execute();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                String jsonData = responses.body().string();
                Log.e(MapboxConstants.TAG, jsonData);
                Log.v(MapboxConstants.TAG, "Reloading");
                return jsonData;
            } catch (Exception e) {
                Log.e(MapboxConstants.TAG, "Error", e);
                return null;
            }
        }

        @Override
        protected void onPostExecute(String s) {
            super.onPostExecute(s);
            if (s != null) {
                ((GeoJsonSource) source).setGeoJson(s);
                reloadSourceRecursivly();
            }
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                onBackPressed();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        mapView.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
        mapView.onPause();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        mapView.onSaveInstanceState(outState);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mapView.onDestroy();
    }

    @Override
    public void onLowMemory() {
        super.onLowMemory();
        mapView.onLowMemory();
    }

    /**
     * Evaluator for LatLng pairs
     */
    private static class LatLngEvaluator implements TypeEvaluator<LatLng> {

        private LatLng latLng = new LatLng();

        @Override
        public LatLng evaluate(float fraction, LatLng startValue, LatLng endValue) {
            latLng.setLatitude(startValue.getLatitude()
                    + ((endValue.getLatitude() - startValue.getLatitude()) * fraction));
            latLng.setLongitude(startValue.getLongitude()
                    + ((endValue.getLongitude() - startValue.getLongitude()) * fraction));
            return latLng;
        }
    }

    private double getBearing(LatLng from, LatLng to) {
        double degrees2radians = Math.PI / 180;
        double radians2degrees = 180 / Math.PI;

        double lon1 = degrees2radians * from.getLongitude();
        double lon2 = degrees2radians * to.getLongitude();
        double lat1 = degrees2radians * from.getLatitude();
        double lat2 = degrees2radians * to.getLatitude();
        double a = Math.sin(lon2 - lon1) * Math.cos(lat2);
        double b = Math.cos(lat1) * Math.sin(lat2)
                - Math.sin(lat1) * Math.cos(lat2) * Math.cos(lon2 - lon1);

        return radians2degrees * Math.atan2(a, b);
    }
}
