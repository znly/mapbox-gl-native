package com.mapbox.mapboxsdk.simpletestapp;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;

import com.mapbox.mapboxsdk.Mapbox;
import com.mapbox.mapboxsdk.camera.CameraUpdateFactory;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.geometry.LatLngBounds;
import com.mapbox.mapboxsdk.maps.MapView;
import com.mapbox.mapboxsdk.maps.MapboxMap;
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback;

public class MainActivity extends Activity implements View.OnClickListener {
    private static final LatLng LAT_LNG_CENTER = new LatLng(48.866800199999986, 2.3536729199999993);

    private MapView mMapView;
    private MapboxMap mMapboxMap;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Mapbox.getInstance(getApplicationContext(), getString(R.string.mapbox_access_token));
        setContentView(R.layout.activity_main);

        mMapView = (MapView) findViewById(R.id.mapview);
        findViewById(R.id.btnZoomIn).setOnClickListener(this);
        findViewById(R.id.btnZoomOut).setOnClickListener(this);

        mMapView.onCreate(savedInstanceState);
        mMapView.getMapAsync(new OnMapReadyCallback() {
            @Override
            public void onMapReady(MapboxMap map) {
                mMapboxMap = map;
            }
        });
    }

    @Override
    protected void onStart() {
        super.onStart();
        mMapView.onStart();
    }

    @Override
    public void onResume() {
        super.onResume();
        mMapView.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
        mMapView.onPause();
    }

    @Override
    public void onLowMemory() {
        super.onLowMemory();
        mMapView.onLowMemory();
    }

    @Override
    protected void onStop() {
        super.onStop();
        mMapView.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mMapView.onDestroy();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        mMapView.onSaveInstanceState(outState);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btnZoomIn : zoomIn(); break;
            case R.id.btnZoomOut : zoomOut(); break;
        }
    }

    private void zoomIn() {
        if (mMapboxMap == null) {
            return;
        }
        double distance = 0.001;
        LatLngBounds bounds = new LatLngBounds.Builder()
                .include(new LatLng(LAT_LNG_CENTER.getLatitude() - distance, LAT_LNG_CENTER.getLongitude() - distance))
                .include(new LatLng(LAT_LNG_CENTER.getLatitude() + distance, LAT_LNG_CENTER.getLongitude() + distance))
                .build();
        mMapboxMap.moveCamera(CameraUpdateFactory.newLatLngBounds(bounds, 10));
    }

    private void zoomOut() {
        if (mMapboxMap == null) {
            return;
        }
        mMapboxMap.animateCamera(CameraUpdateFactory.newLatLngZoom(LAT_LNG_CENTER, 0), 100);
    }
}