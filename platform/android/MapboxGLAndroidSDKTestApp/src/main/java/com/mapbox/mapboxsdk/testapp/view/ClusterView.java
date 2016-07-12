package com.mapbox.mapboxsdk.testapp.view;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.content.Context;
import android.graphics.Color;
import android.graphics.PointF;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.TextView;

import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.maps.Projection;

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

    public boolean isCollapsed() {
        return collapsed;
    }

    public boolean isAnimating() {
        return isAnimating;
    }

    public void setCollapsed(boolean collapsed) {
        this.collapsed = collapsed;
    }

    public void setAnimating(boolean animating) {
        isAnimating = animating;
    }

    public void expand(Projection projection) {
        isAnimating = true;
        PointF point = projection.toScreenLocation(location);
        animate().x(point.x).y(point.y).setDuration(300).setListener(new AnimatorListenerAdapter() {
            @Override
            public void onAnimationEnd(Animator animation) {
                super.onAnimationEnd(animation);
                isAnimating = false;
            }

            @Override
            public void onAnimationStart(Animator animation) {
                super.onAnimationStart(animation);
                setVisibility(View.VISIBLE);
            }
        }).start();
    }

    public void collapse(PointF point) {
        isAnimating = true;
        animate().x(point.x).y(point.y).setDuration(300).setListener(new AnimatorListenerAdapter() {
            @Override
            public void onAnimationEnd(Animator animation) {
                super.onAnimationEnd(animation);
                isAnimating = false;
                setVisibility(View.INVISIBLE);
            }
        }).start();
    }
}
