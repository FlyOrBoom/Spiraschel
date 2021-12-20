package com.example.spiraschel;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;

public class ScheduleActivity extends Activity {

    private ScheduleSurfaceView scheduleView;
    private GestureDetector scheduleGestureDetector;

    String DEBUG_TAG = "H";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Create a GLSurfaceView instance and set it
        // as the ContentView for this Activity.
        scheduleView = new ScheduleSurfaceView(this);
        setContentView(scheduleView);

        scheduleGestureDetector = new GestureDetector(this, new ScheduleGestureListener());
        scheduleView.setOnTouchListener(touchListener);
    }

    View.OnTouchListener touchListener = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            return scheduleGestureDetector.onTouchEvent(event);
        }
    };

    // In the SimpleOnGestureListener subclass you should override
// onDown and any other gesture that you want to detect.
    class ScheduleGestureListener extends GestureDetector.SimpleOnGestureListener {

        @Override
        public boolean onDown(MotionEvent event) {
            Log.d("TAG", "onDown: ");

            // don't return false here or else none of the other
            // gestures will work
            return true;
        }

        @Override
        public boolean onScroll(MotionEvent e1, MotionEvent e2,
                                float distanceX, float distanceY) {
            scheduleView.setUserOffset(distanceX,distanceY);
            return true;
        }
    }

}