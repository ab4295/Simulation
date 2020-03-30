//
//  glSetup.cpp
//  CG_Lab_Simul_1
//
//  Created by 송창용 on 2020/02/28.
//  Copyright © 2020 송창용. All rights reserved.
//

#include "glSetup.hpp"
#include <string.h>
#include <iostream>

using namespace std;
bool perspective = true;
float screenScale = 0.5;
int screenW = 0, screenH = 0; int windowW, windowH;
float aspect;
float fovy = 46.4;

void errorCallback(int error, const char* description)
{
    cerr << "####" << description << endl;
}

void reshape(GLFWwindow* window, int w, int h)
{
    aspect = (float)w/h;
    windowW = w;
    windowH = h; glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    if (perspective) {
        float nearDist = 1.0; float farDist = 20.0;
        gluPerspective(fovy, aspect, nearDist, farDist);
    }
    else {
        glOrtho(-1.0*aspect, 1.0*aspect, -1.0, 1.0, -1.0, 1.0);
    }
    glfwGetWindowSize(window, &screenW, &screenH);
    cerr << "reshape(" << w << ", " << h << ")";
    cerr << " with screeen " << screenW << " x " << screenH << endl;
}

GLFWwindow* initializeOpenGL(int argc, char* argv[], GLfloat bgColor[4], bool modern)
{
    glfwSetErrorCallback(errorCallback);
    if(!glfwInit()) exit(EXIT_FAILURE);
    if (modern)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4); }
    else
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    }
    
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    
    int monitorW, monitorH;
    glfwGetMonitorPhysicalSize(monitor, &monitorW, &monitorH);
    cerr << "Status: Monitor " << monitorW << "mm x " << monitorH << "mm" << endl;
    const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
    if (screenW == 0) screenW = videoMode->width * screenScale; if (screenH == 0) screenH = videoMode->height * screenScale;
    GLFWwindow* window = glfwCreateWindow(screenW, screenH, argv[0], NULL, NULL); if (!window)
    {
        glfwTerminate();
        cerr << "Failed in glfwCreateWindow()" << endl; return NULL;
    }
    // Context
    glfwMakeContextCurrent(window);
    // Clear the background ASAP
    glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
    glfwSwapBuffers(window);
    
    // Check the size of the window
    glfwGetWindowSize(window, &screenW, &screenH);
    
    cerr << "Status: Screeen " << screenW << " x " << screenH << endl;
    glfwGetFramebufferSize(window, &windowW, &windowH);
    cerr << "Status: Framebuffer " << windowW << " x " << windowH << endl;
    // Callbacks
    glfwSetFramebufferSizeCallback(window, reshape);
    // Get the OpenGL version and renderer
    cout << "Status: Renderer " << glGetString(GL_RENDERER) << endl; cout << "Status: Ventor " << glGetString(GL_VENDOR) << endl; cout << "Status: OpenGL " << glGetString(GL_VERSION) << endl;
    // GLSL version for shader loading
    char glslVersion[256];
    strcpy(glslVersion, (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
    cout << "Status: GLSL " << glslVersion << endl;
    glfwSwapInterval(1);
    // GLEW: Supported version
    cerr << "Status: GLEW " << glewGetString(GLEW_VERSION) << endl;
    // Initializing GLEW can be verified in http://glew.sourceforge.net/basic.html
    GLenum error = glewInit(); if (error != GLEW_OK)
    {
        cerr << "ERROR: " << glewGetErrorString(error) << endl;
        return 0;
    }
    return window;
}

void drawAxes(float l, float w)
{
    glLineWidth(w);
    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(l, 0, 0); // x-axis
    glColor3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, l, 0); // y-axis
    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, l); // z-axis
    glEnd();
}
