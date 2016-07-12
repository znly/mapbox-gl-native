package com.mapbox.mapboxsdk.testapp.activity.clustering;

import android.graphics.Color;
import android.graphics.PointF;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Gravity;
import android.view.MenuItem;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.TextView;

import com.mapbox.mapboxsdk.camera.CameraPosition;
import com.mapbox.mapboxsdk.constants.MapboxConstants;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.maps.MapView;
import com.mapbox.mapboxsdk.maps.MapboxMap;
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback;
import com.mapbox.mapboxsdk.maps.Projection;
import com.mapbox.mapboxsdk.testapp.R;

public class ClusterActivity extends AppCompatActivity implements MapboxMap.OnCameraChangeListener {

    private MapView mMapView;
    private MapboxMap mMapboxMap;
    private boolean mCollapsedCluster;

    private static final LatLng LAT_LNG = new LatLng(51.502615, 4.972326);
    private TextView parentTextView;

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

        mMapView = (MapView) findViewById(R.id.mapView);
        mMapView.onCreate(savedInstanceState);
        mMapView.getMapAsync(new OnMapReadyCallback() {

            @Override
            public void onMapReady(@NonNull final MapboxMap mapboxMap) {
                mMapboxMap = mapboxMap;
                setupMap();

                final Projection projection = mapboxMap.getProjection();
                mMapView.addOnMapChangedListener(new MapView.OnMapChangedListener() {

                    @Override
                    public void onMapChanged(@MapView.MapChange int change) {
                        if (change == MapView.REGION_IS_CHANGING || change == MapView.REGION_DID_CHANGE || change == MapView.DID_FINISH_LOADING_MAP) {
                            PointF point = projection.toScreenLocation(LAT_LNG);
                            parentTextView.setX(point.x);
                            parentTextView.setY(point.y);
                        }
                    }
                });
            }
        });

        parentTextView = new TextView(this);
        parentTextView.setLayoutParams(new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        parentTextView.setText("3");
        parentTextView.setTextColor(Color.WHITE);
        parentTextView.setGravity(Gravity.CENTER);
        parentTextView.setBackgroundColor(Color.RED);
        parentTextView.setPadding(32, 32, 32, 32);
        mMapView.addView(parentTextView);
    }

    private void setupMap() {
        mMapboxMap.setOnCameraChangeListener(this);
    }

    @Override
    public void onCameraChange(CameraPosition position) {
        if (position.zoom <= 6 && !mCollapsedCluster) {
            collapseCluster();
        } else if (position.zoom > 6 && mCollapsedCluster) {
            expandCluster();
        }
    }

    private void expandCluster() {
        mCollapsedCluster = false;
        Log.e(MapboxConstants.TAG, "Expanding cluster");
    }

    private void collapseCluster() {
        mCollapsedCluster = true;
        Log.e(MapboxConstants.TAG, "Collapsing cluster");
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
        mMapView.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
        mMapView.onPause();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        mMapView.onSaveInstanceState(outState);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mMapView.onDestroy();
    }

    @Override
    public void onLowMemory() {
        super.onLowMemory();
        mMapView.onLowMemory();
    }
}
