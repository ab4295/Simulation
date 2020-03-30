//
//  main.cpp
//  CG_Lab_Simul_1
//
//  Created by 송창용 on 2020/02/28.
//  Copyright © 2020 송창용. All rights reserved.
//

#include "glSetup.hpp"

#include <Eigen/Dense>
using namespace Eigen;

#include<iostream>
using namespace std;

void init();
void quit();
void initializeMeshSpringSystem();
void update();
void solveODE();
void collisionHandling();
void keyboard(GLFWwindow* window, int key, int code, int action, int mode);
void render(GLFWwindow* window);
void setupLight();
void setupMaterial();
void drawShpere(float radius, const Vector3f& color, int N);

bool pause = true;
int frame = 0;

Vector4f light(0.0,0.0,5.0,1);

float AXIS_LENGTH = 3;
float AXIS_LINE_WIDTH = 2;

GLfloat bgColor[4] = {1,1,1,1};

GLUquadricObj* sphere = NULL;

const int nParticles = 4;
const int meshVertex = 36;

Vector3f position[meshVertex];
Vector3f velocity[meshVertex];

const int nEdges = 110;
float h;
int e1[nEdges];
int e2[nEdges];
float l[nEdges];
float k[nEdges];
float k0 = 1.0;
float absolute_value[nEdges];
bool useConst = true;
bool constrained[meshVertex];

float radius = 0.02;
float m = 0.1;

int N_SUBSTEPS = 1;
float useGravity = true;

float k_r = 0.75;
float epsilon = 1.0E-4;

bool contact[meshVertex];
Vector3f contactN[meshVertex];

const int nWalls = 4;
Vector3f wallP[nWalls];
Vector3f wallN[nWalls];

enum IntergrationMethod
{
    EULER = 1,
    MODIFIED_EULER,
} intMethod = MODIFIED_EULER;

int main(int argc, char* argv[])
{
    // Orthographics viewing
    perspective = false;
    // Initialize the OpenGL system
    GLFWwindow* window = initializeOpenGL(argc, argv, bgColor);
    if (window == NULL)
        return -1;
    // Vertical sync for 60fps
    glfwSwapInterval(1);
    // Callbacks
    glfwSetKeyCallback(window, keyboard);
    // Depth test
    glEnable(GL_DEPTH_TEST);
    // Normal vectors are normalized after transformation.
    glEnable(GL_NORMALIZE);
    // Back face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    // Viewport and perspective setting
    reshape(window, windowW, windowH);
    // Initialization - Main loop - Finalization //
    init();
    initializeMeshSpringSystem();
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        if (!pause)
            update();
        
        render(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    quit();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
void init()
{
    sphere = gluNewQuadric();
    gluQuadricDrawStyle(sphere, GLU_FILL);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    gluQuadricOrientation(sphere, GLU_OUTSIDE);
    gluQuadricTexture(sphere, GL_FALSE);
    // Keyboard and mouse
    cout << "Keyboard Input: space for play on/off" << endl;
    cout << "Keyboard Input: g for gravity on/off" << endl;
    cout << "Keyboard Input: i for the system initialization" << endl;
    cout << "Keyboard Input: c for constraint specification" << endl;
    cout << "Keyboard Input: e for the Euler integration" << endl;
    cout << "Keyboard Input: m for the modified Euler integration" << endl;
    cout << "Keyboard Input: [1:9] for the # of sub-time steps" << endl;
    cout << "Keyboard Input: [1:4] for constraint specification" << endl;
    cout << "Keyboard Input: up to increase the spring constant" << endl;
    cout << "Keyboard Input: down to decrease the spring constant" << endl;
}
void quit()
{
    gluDeleteQuadric(sphere);
}
void rebuildSpringK()
{
    cout << "Spring constant = " << k0 << " N/m" << endl;
    
    for(int i = 0;i<nEdges;i++)
    {
        k[i] = k0 / l[i];
        cout << "k[" << i << "]" << k[i] << endl;
    }
}
void initializeMeshSpringSystem()
{
    float init_x = -0.6;
    float init_y = 0.6;
    int k = 0;
    int cal1 = 1, cal2 = 1;
    int out = 0;
    /* Initialize join e1, e2 */
    
    /* width */
    for(int i = 0;i< 30;i++)
    {
        if((cal1 * 5)+(cal2 * out) != i)
        {
            e1[i] = i;
            e2[i] = i+1;
        }
        else
        {
            e1[i] = 30 + out;
            e2[i] = 31 + out;
            cal1++;
            out++;
        }
    }
    cal1 = 1;
    cal2 = 1;
    out = 1;
    
    /* height */
    for(int i = 30;i< 60;i++)
    {
        if((cal1 * 30) + (cal2 * out) != (i-30))
        {
            e1[i] = i-30;
            e2[i] = i-24;
        }
        else
        {
            out++;
        }
    }
    cal1 = 1;
    cal2 = 1;
    out = 0;
    
    /* diagonalize_1 */
    for(int i = 60;i< 85;i++)
    {
        if((cal1 * 5)+(cal2 * out) != (i-60))
        {
            e1[i] = i - 60;
            e2[i] = (i - 60) + 7;
        }
        else
        {
            e1[i] = i - (40 + (out * 5));
            e2[i] = i - (40 + (out * 5)) + 7;
            cal1++;
            out++;
        }
    }
    out = 0;
    
    /* diagonalize_2 */
    for(int i = 85;i<110;i++)
    {
        if((i - 85) % 6 != 0)
        {
            e1[i] = i - 85;
            e2[i] = (i - 85) + 5;
        }
        else
        {
            e1[i] = i - (60 + (5 * out));
            e2[i] = i - (60 + (5 * out)) + 5;
            out++;
        }
    }
    /* Initialize position */
    for(int i = 0;i<meshVertex;i++)
    {
        if(i % 6 == 0 && i > 1)
        {
            init_x = -0.6;
            init_y -= 0.15;
            k = 0;
        }
        position[i] = Vector3f(init_x + (0.15 * k),init_y, 0);
        k++;
    }
    
    for(int i = 0;i<meshVertex;i++)
    {
        velocity[i].setZero();
        contact[i] = false;
        constrained[i] = false;
    }
    
    for(int i = 0;i<nEdges;i++)
    {
        l[i] = (position[e1[i]] - position[e2[i]]).norm();
    }
    
    rebuildSpringK();
    
    wallN[0] = Vector3f( 1.0, 0, 0);
    wallN[1] = Vector3f(-1.0, 0, 0);
    wallN[2] = Vector3f(0, 1.0, 0);
    wallN[3] = Vector3f(0, -1.0, 0);
    
    for (int i = 0; i < nWalls; i++)
        wallN[i].normalize();
    
    collisionHandling();
}
void update()
{
    for(int i = 0;i<N_SUBSTEPS;i++)
    {
        solveODE();
    }
    frame++;
}
void solveODE()
{
    Vector3f f[meshVertex];
    Vector3f gravity(0,-9.8,0);
    Vector3f f_i;
    float L_i;
    h = 1.0/60.0/N_SUBSTEPS;
    for(int i  = 0;i<meshVertex;i++)
    {
        f[i].setZero();
        
        if(useGravity)
        {
            f[i] += m * gravity;
        }
    }
    
    for(int i = 0; i< nEdges;i++)
    {
        Vector3f v_i = (position[e1[i]] - position[e2[i]]);
        L_i = v_i.norm();
        f_i = k[i] * (L_i - l[i]) * v_i / L_i;
        f[e2[i]] += f_i;
        f[e1[i]] -= f_i;
    }
    
    for(int i = 0;i<meshVertex;i++)
    {
        if(constrained[i])
            continue;
        
        if(contact[i])
        {
            f[i] -= contactN[i].dot(f[i]) * contactN[i];
        }
        
        float a = k[i] * -1.0f;
        switch (intMethod)
        {
            case EULER:
                position[i] += h * velocity[i];
                velocity[i] += h * f[i] / m;
                break;
            case MODIFIED_EULER:
                velocity[i] += h * f[i] / m;
                position[i] += h * velocity[i];
                break;
        }
    }
    collisionHandling();
}
void collisionHandling()
{
    wallP[0] = Vector3f(-1.0 * aspect, 0, 0);
    wallP[1] = Vector3f( 1.0 * aspect, 0, 0);
    wallP[2] = Vector3f(0, -1.0, 0);
    wallP[3] = Vector3f(0, 1.0, 0);
    
    for(int i = 0;i<meshVertex;i++)
    {
        contact[i] = false;
        for(int j = 0;j< nWalls;j++)
        {
            float d_N = wallN[j].dot(position[i] - wallP[j]);
            if(d_N < radius)
            {
                position[i] += (radius - d_N) * wallN[j];
                
                float v_N = wallN[j].dot(velocity[i]);
                
                if(fabs(v_N) < epsilon)
                {
                    contact[i] = true;
                    contactN[i] = wallN[j];
                }
                else if(v_N < 0)
                {
                    velocity[i] -= (1+ k_r) * v_N * wallN[j];
                }
            }
        }
    }
}
void keyboard(GLFWwindow* window, int key, int code, int action, int mode)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch(key)
        {
            case GLFW_KEY_Q: /* close window. */
            case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE); break;
            case GLFW_KEY_SPACE: pause = !pause;                      break;
            case GLFW_KEY_G: useGravity = !useGravity;                break;
            case GLFW_KEY_I: initializeMeshSpringSystem();            break;
            case GLFW_KEY_E: intMethod = EULER;                       break;
            case GLFW_KEY_M: intMethod = MODIFIED_EULER;              break;
                
            case GLFW_KEY_C: useConst = !useConst;                    break;
                
            case GLFW_KEY_UP: k0 = min(k0 + 0.1, 10.0); rebuildSpringK(); break;
            case GLFW_KEY_DOWN: k0 = max(k0 - 0.1, 0.1);rebuildSpringK(); break;
                
            case GLFW_KEY_PERIOD: break;
            case GLFW_KEY_LEFT_BRACKET:  break;
            case GLFW_KEY_RIGHT_BRACKET: break;
        }
        if(useConst)
        {
            switch(key)
            {
                case GLFW_KEY_1: constrained[0] = !constrained[0]; break;
                case GLFW_KEY_2: constrained[1] = !constrained[1]; break;
                case GLFW_KEY_3: constrained[2] = !constrained[2]; break;
                case GLFW_KEY_4: constrained[3] = !constrained[3]; break;
                case GLFW_KEY_5: constrained[4] = !constrained[4]; break;
                case GLFW_KEY_6: constrained[5] = !constrained[5]; break;
            }
        }
        else
        {
            switch(key)
            {
                case GLFW_KEY_1: N_SUBSTEPS = 1; h = 1.0/6000.0/N_SUBSTEPS; cout << "h:" << h << "FPS" << endl; break;
                case GLFW_KEY_2: N_SUBSTEPS = 2; h = 1.0/6000.0/N_SUBSTEPS; cout << "h:" << h  << "FPS" << endl; break;
                case GLFW_KEY_3: N_SUBSTEPS = 3; h = 1.0/6000.0/N_SUBSTEPS; cout << "h:" << h<< "FPS" << endl; break;
                case GLFW_KEY_4: N_SUBSTEPS = 4; h = 1.0/6000.0/N_SUBSTEPS; cout << "h:" << h << "FPS" << endl; break;
                case GLFW_KEY_5: N_SUBSTEPS = 5; h = 1.0/6000.0/N_SUBSTEPS; cout << "h:" << h << "FPS" << endl; break;
                case GLFW_KEY_6: N_SUBSTEPS = 6; h = 1.0/6000.0/N_SUBSTEPS; cout << "h:" << h << "FPS" << endl; break;
                case GLFW_KEY_7: N_SUBSTEPS = 7; h = 1.0/6000.0/N_SUBSTEPS; cout << "h:" << h << "FPS" << endl; break;
                case GLFW_KEY_8: N_SUBSTEPS = 8; h = 1.0/6000.0/N_SUBSTEPS; cout << "h:" << h << "FPS" << endl; break;
                case GLFW_KEY_9: N_SUBSTEPS = 9; h = 1.0/6000.0/N_SUBSTEPS; cout << "h:" << h << "FPS" << endl; break;
            }
        }
    }
}
void render(GLFWwindow* window)
{
    glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (0)
    {
        glDisable(GL_LIGHTING);
        drawAxes(AXIS_LENGTH, AXIS_LINE_WIDTH);
    }
    setupLight();
    
    setupMaterial();
    
    for (int i = 0; i < meshVertex; i++) {
        glPushMatrix();
        glTranslatef(position[i][0], position[i][1], position[i][2]);
        if (constrained[i])
            drawShpere(radius, Vector3f(1, 1, 0), 20);
        else
            drawShpere(radius, Vector3f(0, 1, 0), 20);
        glPopMatrix();
    }
    
    glLineWidth(7);
    glColor3f(0, 0, 1);
    glBegin(GL_LINES);
    for (int i = 0; i < nEdges; i++)
    {
        glVertex3fv(position[e1[i]].data());
        glVertex3fv(position[e2[i]].data());
        
    }
    glEnd();
}
void setupLight()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat ambient[4] = { 0.1, 0.1, 0.1, 1 };
    GLfloat diffuse[4] = { 1.0, 1.0, 1.0, 1 };
    GLfloat specular[4] = { 1.0, 1.0, 1.0, 1 };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light.data());
}
void setupMaterial()
{
    GLfloat mat_ambient[4] = { 0.1, 0.1, 0.1, 1 };
    GLfloat mat_specular[4] = { 0.5, 0.5, 0.5, 1 };
    GLfloat mat_shininess = 128;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
}
void setDiffuseColor(const Vector3f& color)
{
    GLfloat mat_diffuse[4] = { color[0], color[1], color[2], 1 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    
}
void drawShpere(float radius, const Vector3f& color, int N)
{
    setDiffuseColor(color);
    gluSphere(sphere, radius, N, N);
}

