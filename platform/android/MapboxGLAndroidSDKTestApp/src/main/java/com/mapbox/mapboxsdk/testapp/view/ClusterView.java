package com.mapbox.mapboxsdk.testapp.view;

import android.content.Context;
import android.graphics.Color;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.TextView;

import com.mapbox.mapboxsdk.geometry.LatLng;

public class ClusterView extends TextView {

    private LatLng location;
    private boolean collapsed;
    private boolean isAnimating;

    public ClusterView(Context context) {
        super(context);
        init(context);
    }

    private void init(Context context) {
        setLayoutParams(new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        setTextColor(Color.WHITE);
        setGravity(Gravity.CENTER);
        setBackgroundColor(Color.GRAY);
        setPadding(32, 32, 32, 32);
        setVisibility(View.INVISIBLE);
    }

    public void setLocation(LatLng latLng) {
        location = latLng;
    }

    public LatLng getLocation() {
        return location;
    }
}
