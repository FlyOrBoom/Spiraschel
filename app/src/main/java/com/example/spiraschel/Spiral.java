package com.example.spiraschel;

import android.opengl.GLES20;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.time.LocalTime;
import java.util.Calendar;
import java.util.TimeZone;

public class Spiral {

    private FloatBuffer vertexBuffer;
    static final int SECONDS = 24*60*60;
    static final int DIMENSIONS = 4;

    float vertices[] = new float[SECONDS * DIMENSIONS];

    private final String vertexShaderCode = "" +
            "attribute vec4 vertex;" +
            "uniform float offset;" +
            "varying vec3 color;" +
            "float cap(float x, float a){" +
            "   return sqrt(1.0 - (a-x)*(a-x));" +
            "}" +
            "float left_cap(float x, float a){" +
            "   return cap(min(x,a), a);" +
            "}" +
            "float right_cap(float x, float a){" +
            "   return cap(max(x,a), a);" +
            "}" +
            "void main() {" +
            "" +
            "  float time = vertex.x;" +
            "  float index = vertex.y;" +
            "  float start = vertex.z;" +
            "  float end = vertex.w;" +
            "" +
            "  float angle = time * 0.0017453292519943;" +
            "  float elapsed = time - start;" +
            "  float total = end - start;" +
            "  float progress = elapsed/total;" +
            "  float nearness = time - offset;" +
            "" +
            "  float slope = 0.00002;" +
            "  float taper = 0.05;" +
            "  float proximity = nearness * 0.003;" +
            "  float thickness = 0.08;" +
            "  thickness *= mix(0.5, 1.0, progress);" +
            "  thickness *= left_cap(taper*elapsed, 1.2);" +
            "  thickness *= right_cap(taper*elapsed, taper*total - 1.2);" +
            "  thickness *= mix(0.5, 1.0, smoothstep(1.0, 0.0, proximity*proximity));" +
            "  float flipflop = mod(time, 2.0) - 0.5;" +
            "  " +
            "  float radius = 0.8 * exp( slope*(offset-time) + thickness*flipflop );" +
            "  gl_Position = vec4( radius*sin(angle), radius*cos(angle), 0, 1 );" +
            "" +
            "  float early_index = floor(index * 0.5);" +
            "  vec3 color_start = cos( early_index + vec3(0,1,2) );" +
            "  vec3 color_end = cos( early_index + vec3(0.5,1.5,2.5) );" +
            "  color = mix(color_start, color_end, progress);" +
            "  color *= color;" +
            "  color = mix(vec3(0.9), color, mod(index, 2.0));" +
            "}";

    private final String fragmentShaderCode =
            "precision mediump float;" +
            "varying vec3 color;" +
            "void main() {" +
            "  gl_FragColor = vec4(color, 1);" +
            "}";

    // Set color with red, green, blue and alpha (opacity) values
    float color[] = { 0.63671875f, 0.76953125f, 0.22265625f, 1.0f };

    private final int mProgram;

    public Spiral(int[] bounds) {

        int bounds_start = bounds[0] * 60;
        int bounds_end = bounds[bounds.length - 1] * 60;

        int interval_start = bounds[0] * 60;
        int interval_end = bounds[1] * 60;
        int interval_index = 1;

        for(int i = 0; i < bounds_start * DIMENSIONS; i++)
            vertices[i] = 0f;

        for(int t = bounds_start; t < bounds_end; t++){

            if (t >= interval_end){
                interval_start = bounds[interval_index] * 60;
                interval_index++;
                interval_end = bounds[interval_index] * 60;
            }

            int i = t * DIMENSIONS;
            vertices[i+0] = (float) t;
            vertices[i+1] = (float) interval_index;
            vertices[i+2] = (float) interval_start;
            vertices[i+3] = (float) interval_end;
        }

        for(int i = bounds_end * DIMENSIONS; i < SECONDS * DIMENSIONS; i++)
            vertices[i] = 0f;

        // initialize vertex byte buffer for shape coordinates
        ByteBuffer bb = ByteBuffer.allocateDirect(
                // (number of coordinate values * 4 bytes per float)
                vertices.length * 4);
        // use the device hardware's native byte order
        bb.order(ByteOrder.nativeOrder());

        // create a floating point buffer from the ByteBuffer
        vertexBuffer = bb.asFloatBuffer();
        // add the coordinates to the FloatBuffer
        vertexBuffer.put(vertices);
        // set the buffer to read the first coordinate
        vertexBuffer.position(0);

        int vertexShader = ScheduleRenderer.loadShader(GLES20.GL_VERTEX_SHADER,
                vertexShaderCode);
        int fragmentShader = ScheduleRenderer.loadShader(GLES20.GL_FRAGMENT_SHADER,
                fragmentShaderCode);

        // create empty OpenGL ES Program
        mProgram = GLES20.glCreateProgram();

        // add the vertex shader to program
        GLES20.glAttachShader(mProgram, vertexShader);

        // add the fragment shader to program
        GLES20.glAttachShader(mProgram, fragmentShader);

        // creates OpenGL ES program executables
        GLES20.glLinkProgram(mProgram);

    }

    private int vertexHandle;
    private int offsetHandle;

    public void draw(float offset) {
        // Add program to OpenGL ES environment
        GLES20.glUseProgram(mProgram);

        // get handle to vertex shader's vPosition member
        vertexHandle = GLES20.glGetAttribLocation(mProgram, "vertex");

        // Enable a handle to the triangle vertices
        GLES20.glEnableVertexAttribArray(vertexHandle);

        // Prepare the triangle coordinate data
        GLES20.glVertexAttribPointer(vertexHandle, DIMENSIONS,
                GLES20.GL_FLOAT, false,
                DIMENSIONS * 4, vertexBuffer);

        // get handle to fragment shader's vColor member
        offsetHandle = GLES20.glGetUniformLocation(mProgram, "offset");

        // Set color for drawing the triangle
        GLES20.glUniform1f(offsetHandle, offset);

        // Draw the triangle
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, SECONDS);

        // Disable vertex array
        GLES20.glDisableVertexAttribArray(vertexHandle);
    }

}
