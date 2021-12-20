package com.example.spiraschel;

import android.content.Context;
import android.opengl.EGLConfig;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLDisplay;

class ScheduleSurfaceView extends GLSurfaceView {

    private final ScheduleRenderer renderer;

    public ScheduleSurfaceView(Context context){
        super(context);

        // Create an OpenGL ES 2.0 context
        setEGLContextClientVersion(2);
        setEGLConfigChooser(new MultisampleConfigChooser());

        renderer = new ScheduleRenderer();

        // Set the Renderer for drawing on the GLSurfaceView
        setRenderer(renderer);
    }

    public void setUserOffset(double x, double y){
        renderer.setUserOffset(x, y);
    }
}
