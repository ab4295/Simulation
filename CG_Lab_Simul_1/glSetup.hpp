//
//  glSetup.hpp
//  CG_Lab_Simul_1
//
//  Created by 송창용 on 2020/02/28.
//  Copyright © 2020 송창용. All rights reserved.
//

#ifndef glSetup_h
#define glSetup_h

#include <GL/glew.h>
#include <GLFW/glfw3.h>

extern float screenScale;
extern int screenW, screenH;
extern int windowW, windowH;
extern float aspect;
extern bool perspective;
extern float fovy;

GLFWwindow* initializeOpenGL(int argc, char* argv[], GLfloat bg[4], bool modern = false);
void reshape(GLFWwindow* window, int w, int h);
void drawAxes(float l, float w);

#endif /* glSetup_h */
