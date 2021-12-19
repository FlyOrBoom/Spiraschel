/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gles3jni.h"
#include <EGL/egl.h>

#define STR(s) #s
#define STRV(s) STR(s)

#define TIME_ATTRIB 0
#define INDEX_ATTRIB 1
#define START_ATTRIB 2
#define OFFSET_ATTRIB 3

#define MULTILINE(...) #__VA_ARGS__

static const char* VERTEX_SHADER =
    "#version 300 es\n"
    "layout(location = " STRV(TIME_ATTRIB) ") in int t;\n"
    "layout(location = " STRV(INDEX_ATTRIB) ") in int i;\n"
    "layout(location = " STRV(START_ATTRIB) ") in int s;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "    float time = float(t);\n"
    "    float index = float(i);\n"
    "    float start = float(s);\n"
    "    float early_index = floor(index*0.5);\n"
"        float angle = time * 0.0017453292519943;\n"
"        float radius = time * 0.000017 + ( mod(time, 2.0) - 0.5 ) * ( mod(index, 2.0) * (1. + (start - time) * 0.02) * 0.0004 + 0.02 );\n"
    "    gl_Position = vec4(\n"
    "        radius * sin(angle),\n"
    "        radius * cos(angle),\n"
    "        0.0, 1.0\n"
    "    );\n"
    "    float value = early_index;\n"
    "    vec3 color = vec3( cos(value), cos(value+1.), cos(value+2.));\n"
    "    vec3 blank = mix(vec3(1), color*color, 0.2);\n"
    "    vColor = vec4( mix(blank, color*color, mod(index, 2.0)), 1.);\n"
    "}\n";

static const char FRAGMENT_SHADER[] =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec4 vColor;\n"
    "out vec4 outColor;\n"
    "void main() {\n"
    "    outColor = vColor;\n"
    "}\n";

class RendererES3: public Renderer {
public:
    RendererES3();
    virtual ~RendererES3();
    bool init();

private:
    enum {VB_INSTANCE, VB_SCALEROT, VB_OFFSET, VB_COUNT};

    virtual float* mapOffsetBuf();
    virtual void unmapOffsetBuf();
    virtual float* mapTransformBuf();
    virtual void unmapTransformBuf();
    virtual void draw(unsigned int numInstances);

    const EGLContext mEglContext;
    GLuint mProgram;
    GLuint mVB[VB_COUNT];
    GLuint mVBState;
};

Renderer* createES3Renderer() {
    RendererES3* renderer = new RendererES3;
    if (!renderer->init()) {
        delete renderer;
        return NULL;
    }
    return renderer;
}

RendererES3::RendererES3()
:   mEglContext(eglGetCurrentContext()),
    mProgram(0),
    mVBState(0)
{
    for (int i = 0; i < VB_COUNT; i++)
        mVB[i] = 0;
}

bool RendererES3::init() {
    mProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    if (!mProgram)
        return false;

    int bounds[]{510,567,574,631,638,695,702,761,809,866,873,930,MINUTES_PER_DAY};
    int start = bounds[0];
    int start_index = 0;

    Point spiral[SECONDS_PER_DAY];

    for(int time = 0; time < SECONDS_PER_DAY; time++){

        if (time > start*60){
            start_index++;
            start = bounds[start_index];
        }

        Point p = { time, start_index, start*60 };
        spiral[time] = p;
    }

    glGenBuffers(VB_COUNT, mVB);
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_INSTANCE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(spiral), &spiral[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_OFFSET]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float), NULL, GL_STATIC_DRAW);

    glGenVertexArrays(1, &mVBState);
    glBindVertexArray(mVBState);

    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_INSTANCE]);
    glVertexAttribIPointer(TIME_ATTRIB, 1, GL_INT, sizeof(Point), (const GLvoid*)offsetof(Point, time));
    glVertexAttribIPointer(INDEX_ATTRIB, 1, GL_INT, sizeof(Point), (const GLvoid*)offsetof(Point, index));
    glVertexAttribIPointer(START_ATTRIB, 1, GL_INT, sizeof(Point), (const GLvoid*)offsetof(Point, start));
    glEnableVertexAttribArray(TIME_ATTRIB);
    glEnableVertexAttribArray(INDEX_ATTRIB);
    glEnableVertexAttribArray(START_ATTRIB);

    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_OFFSET]);
    glVertexAttribPointer(OFFSET_ATTRIB, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);
    glEnableVertexAttribArray(OFFSET_ATTRIB);
    glVertexAttribDivisor(OFFSET_ATTRIB, 1);

    ALOGV("Using OpenGL ES 3.0 renderer");
    return true;
}

RendererES3::~RendererES3() {
    /* The destructor may be called after the context has already been
     * destroyed, in which case our objects have already been destroyed.
     *
     * If the context exists, it must be current. This only happens when we're
     * cleaning up after a failed init().
     */
    if (eglGetCurrentContext() != mEglContext)
        return;
    glDeleteVertexArrays(1, &mVBState);
    glDeleteBuffers(VB_COUNT, mVB);
    glDeleteProgram(mProgram);
}

float* RendererES3::mapOffsetBuf() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_OFFSET]);
    return (float*)glMapBufferRange(GL_ARRAY_BUFFER,
            0, sizeof(float),
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void RendererES3::unmapOffsetBuf() {
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

float* RendererES3::mapTransformBuf() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_SCALEROT]);
    return (float*)glMapBufferRange(GL_ARRAY_BUFFER,
            0, 4*sizeof(float),
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void RendererES3::unmapTransformBuf() {
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void RendererES3::draw(unsigned int numInstances) {
    glUseProgram(mProgram);
    glBindVertexArray(mVBState);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, SECONDS_PER_DAY, 1);
}
