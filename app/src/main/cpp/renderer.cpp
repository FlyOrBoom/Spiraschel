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

#include "runner.h"
#include <EGL/egl.h>

#define STR(s) #s
#define STRV(s) STR(s)

#define OFFSET_ATTRIB 0
#define INDEX_ATTRIB 1
#define START_ATTRIB 2
#define END_ATTRIB 3

#define MULTILINE(...) #__VA_ARGS__

static const char* VERTEX_SHADER =
    "#version 300 es\n"
    "layout(location = " STRV(OFFSET_ATTRIB) ") uniform int o;\n"
    "layout(location = " STRV(INDEX_ATTRIB) ") in int i;\n"
    "layout(location = " STRV(START_ATTRIB) ") in int s;\n"
    "layout(location = " STRV(END_ATTRIB) ") in int e;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "    float time = float(gl_VertexID);\n"
    "    float offset = float(o);\n"
    "    float index = float(i);\n"
    "    float index_odd = float(i%2);\n"
    "    float start = float(s*60);\n"
    "    float end = float(e*60);\n"
    "\n"
    "    float early_index = floor(index*0.5);\n"
"        float angle = time * 0.0017453292519943;\n"
"        float radius = exp(1.0-time/offset) * mix(0.98, 0.90, mod(time, 2.0));\n"
    "    gl_Position = vec4(\n"
    "        radius * sin(angle),\n"
    "        radius * cos(angle),\n"
    "        0.0, 1.0\n"
    "    );\n"
    "    vec3 color_start = cos( early_index + vec3(0,1,2) );\n"
    "    vec3 color_end = cos( early_index + vec3(0.5,1.5,2.5) );\n"
    "    vec3 color = mix(color_start, color_end, (time-start)/(end-start));\n"
    "    vec3 blank = mix(vec3(1), color*color, 0.2);\n"
    "    vColor = vec4( mix(blank, color*color, index_odd), 1.);\n"
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
    enum {VB_INSTANCE, VB_COUNT};

    virtual void draw();

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

    short interval_start = 0;
    short interval_end = bounds[0];

    int start = interval_start;
    int end = 930;

    short interval_index = 0;

    Point spiral[SECONDS_PER_DAY];

    for(int time = 0; time < SECONDS_PER_DAY; time++){

        if (time >= interval_end * SECONDS_PER_MINUTE){
            interval_start = bounds[interval_index];
            interval_index++;
            interval_end = bounds[interval_index];
        }

        Point p = { interval_index, interval_start, interval_end };
        spiral[time] = p;
    }

    glGenBuffers(VB_COUNT, mVB);
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_INSTANCE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(spiral), &spiral[0], GL_STATIC_DRAW);

    glGenVertexArrays(1, &mVBState);
    glBindVertexArray(mVBState);

    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_INSTANCE]);
    glVertexAttribIPointer(INDEX_ATTRIB, 1, GL_SHORT, sizeof(Point), (const GLvoid*)offsetof(Point, index));
    glVertexAttribIPointer(START_ATTRIB, 1, GL_SHORT, sizeof(Point), (const GLvoid*)offsetof(Point, start));
    glVertexAttribIPointer(END_ATTRIB, 1, GL_SHORT, sizeof(Point), (const GLvoid*)offsetof(Point, end));
    glEnableVertexAttribArray(INDEX_ATTRIB);
    glEnableVertexAttribArray(START_ATTRIB);
    glEnableVertexAttribArray(END_ATTRIB);

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

void RendererES3::draw() {
    glUseProgram(mProgram);
    glBindVertexArray(mVBState);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, SECONDS_PER_DAY, 1);
    glUniform1i(OFFSET_ATTRIB, mOffset);
}
