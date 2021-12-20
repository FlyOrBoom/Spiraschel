package com.example.spiraschel;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.Log;

import java.util.Calendar;

public class ScheduleRenderer implements GLSurfaceView.Renderer {

    private Spiral spiral;
    private Calendar cal;

    public void onSurfaceCreated(GL10 unused, EGLConfig config) {
        // Set the background frame color
        GLES20.glClearColor(1f, 1f, 1f, 1f);
        int[] bounds = new int[]{510,567,574,631,638,695,702,761,809,866,873,930};

        spiral = new Spiral(bounds);

        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);

    }

    public void onDrawFrame(GL10 unused) {
        // Redraw background color

        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);

        cal = Calendar.getInstance();
        float hour = cal.get(Calendar.HOUR_OF_DAY);
        float minute = cal.get(Calendar.MINUTE);
        float second = cal.get(Calendar.SECOND);

        float offset = 60*(60*hour + minute) + second;

        Log.d("time",String.valueOf(offset));

        spiral.draw(offset);
    }

    public void onSurfaceChanged(GL10 unused, int width, int height) {
        int min = Math.min(width, height);
        GLES20.glViewport(0, 0, min, min);
    }

    public static int loadShader(int type, String shaderCode){

        // create a vertex shader type (GLES20.GL_VERTEX_SHADER)
        // or a fragment shader type (GLES20.GL_FRAGMENT_SHADER)
        int shader = GLES20.glCreateShader(type);

        // add the source code to the shader and compile it
        GLES20.glShaderSource(shader, shaderCode);
        GLES20.glCompileShader(shader);

        return shader;
    }

}
