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
import android.view.View;
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
import com.mapbox.mapboxsdk.testapp.view.ClusterView;
import com.mapbox.mapboxsdk.utils.AnimatorUtils;

import java.util.ArrayList;
import java.util.List;

public class ClusterActivity extends AppCompatActivity implements MapboxMap.OnCameraChangeListener {

    private MapView mMapView;
    private MapboxMap mMapboxMap;

    private boolean mCollapsedCluster;

    private static final LatLng LAT_LNG = new LatLng(51.502615, 4.972326);
    private static final LatLng LAT_LNG_UTRECHT = new LatLng(52.090432, 5.122310);
    private static final LatLng LAT_LNG_MAASTRICHT = new LatLng(50.851274, 5.694722);
    private static final LatLng LAT_LNG_BRUSSEL = new LatLng(50.861592, 4.359965);

    private TextView parentTextView;
    private List<ClusterView> childTextViews;
    private Projection projection;

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

                projection = mapboxMap.getProjection();
                mMapView.addOnMapChangedListener(new MapView.OnMapChangedListener() {

                    @Override
                    public void onMapChanged(@MapView.MapChange int change) {
                        if (change == MapView.REGION_IS_CHANGING || change == MapView.REGION_DID_CHANGE || change == MapView.DID_FINISH_LOADING_MAP) {
                            invalidateCluster();
                        }
                    }
                });
            }
        });

        childTextViews = new ArrayList<>();
        childTextViews.add(createChildView("Brussel", LAT_LNG_BRUSSEL));
        childTextViews.add(createChildView("Utrecht", LAT_LNG_UTRECHT));
        childTextViews.add(createChildView("Maastricht", LAT_LNG_MAASTRICHT));

        parentTextView = new TextView(this);
        parentTextView.setLayoutParams(new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        parentTextView.setText("3");
        parentTextView.setTextColor(Color.WHITE);
        parentTextView.setGravity(Gravity.CENTER);
        parentTextView.setVisibility(View.INVISIBLE);
        parentTextView.setBackgroundResource(R.drawable.round);
        parentTextView.setPadding(32, 32, 32, 32);
        mMapView.addView(parentTextView);
    }

    private ClusterView createChildView(String text, LatLng location) {
        ClusterView textView = new ClusterView(this);
        textView.setText(text);
        textView.setLocation(location);
        mMapView.addView(textView);
        return textView;
    }

    private void setupMap() {
        mMapboxMap.setOnCameraChangeListener(this);
    }

    @Override
    public void onCameraChange(CameraPosition position) {
        if (position.zoom <= 7 && !mCollapsedCluster) {
            collapseCluster();
        } else if (position.zoom > 7 && mCollapsedCluster) {
            expandCluster();
        }
    }

    private void expandCluster() {
        Log.e(MapboxConstants.TAG, "Expanding cluster");
        mCollapsedCluster = false;
        invalidateCluster();
    }

    private void collapseCluster() {
        Log.e(MapboxConstants.TAG, "Collapsing cluster");
        mCollapsedCluster = true;
        invalidateCluster();
    }

    private void invalidateCluster() {
        if (projection != null) {
            PointF point = projection.toScreenLocation(LAT_LNG);
            parentTextView.setX(point.x);
            parentTextView.setY(point.y);

            if (mCollapsedCluster) {
                AnimatorUtils.alpha(parentTextView, 1);
                for (ClusterView textview : childTextViews) {
                    if (textview.getVisibility() == View.VISIBLE) {
                        textview.setVisibility(View.INVISIBLE);
                    }
                    textview.setX(point.x);
                    textview.setY(point.y);
                }
            } else {
                AnimatorUtils.alpha(parentTextView, 0);
                for (ClusterView textview : childTextViews) {
                    if (textview.getVisibility() == View.INVISIBLE) {
                        textview.setVisibility(View.VISIBLE);
                    }
                    PointF childPoint = projection.toScreenLocation(textview.getLocation());
                    textview.setX(childPoint.x);
                    textview.setY(childPoint.y);
                }
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
