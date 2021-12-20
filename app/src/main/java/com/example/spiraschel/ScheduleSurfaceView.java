package com.example.spiraschel;

import android.content.Context;
import android.opengl.GLSurfaceView;

class ScheduleSurfaceView extends GLSurfaceView {

    private final ScheduleRenderer renderer;

    public ScheduleSurfaceView(Context context){
        super(context);

        // Create an OpenGL ES 2.0 context
        setEGLContextClientVersion(2);

        renderer = new ScheduleRenderer();

        // Set the Renderer for drawing on the GLSurfaceView
        setRenderer(renderer);
    }

    public void setUserOffset(double x, double y){
        renderer.setUserOffset(x, y);
    }
}
