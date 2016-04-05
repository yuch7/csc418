/***********************************************************
             CSC418/2504, Fall 2009
  
                 penguin.cpp
                 
       Simple demo program using OpenGL and the glut/glui 
       libraries

  
    Instructions:
        Please read the assignment page to determine 
        exactly what needs to be implemented.  Then read 
        over this file and become acquainted with its 
        design.

        Add source code where it appears appropriate. In
        particular, see lines marked 'TODO'.

        You should not need to change the overall structure
        of the program. However it should be clear what
        your changes do, and you should use sufficient comments
        to explain your code.  While the point of the assignment
        is to draw and animate the character, you will
        also be marked based on your design.

***********************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glui.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef _WIN32
#include <unistd.h>
#else
void usleep(unsigned int nanosec)
{
    Sleep(nanosec / 1000);
}
#endif


// *************** GLOBAL VARIABLES *************************


const float PI = 3.14159;

// --------------- USER INTERFACE VARIABLES -----------------

// Window settings
int windowID;               // Glut window ID (for display)
GLUI *glui;                 // Glui window (for controls)
int Win[2];                 // window (x,y) size


// ---------------- ANIMATION VARIABLES ---------------------

// Animation settings
int animate_mode = 0;       // 0 = no anim, 1 = animate
int animation_frame = 0;      // Specify current frame of animation

// Joint parameters
const float JOINT_MIN = -45.0f;
const float JOINT_MAX =  45.0f;
float joint_rot = 0.0f;

//x and y for translation animation
float penguinx = 0.0f;
float penguiny = 0.0f;

// head rot
float head_rot = 0.0f;

//left leg rot
float LL_rot = 0.0f;

//right leg rot
float RL_rot = 0.0f;

//left feet rot
float LF_rot = 0.0f;

//right feet rot
float RF_rot = 0.0f;

//beak y position
float BY_pos = 0.0f;

//arm rotation
float ARM_rot = 0.0f;

//body rotation
float body_rot = 0.0f;

const float HEAD_JOINT_MIN = -8.0f;
const float HEAD_JOINT_MAX = 8.0f;

const float FOOT_JOINT_MIN = -20.0f;
const float FOOT_JOINT_MAX = 20.0f;

const float BODY_MIN = -5.0f;
const float BODY_MAX = 0.0f;
//////////////////////////////////////////////////////
// TODO: Add additional joint parameters here
//////////////////////////////////////////////////////



// ***********  FUNCTION HEADER DECLARATIONS ****************


// Initialization functions
void initGlut(char* winName);
void initGlui();
void initGl();


// Callbacks for handling events in glut
void myReshape(int w, int h);
void animate();
void display(void);

// Callback for handling events in glui
void GLUI_Control(int id);


// Functions to help draw the object
void drawSquare(float size);
void drawCircle(float r, bool filled);
void drawTorso(float width, float height);
void drawRectangle(float width, float height);
void drawArm(float width, float height);
void drawHead(float width, float height);
void drawBeak(float width, float height);

// Return the current system clock (in seconds)
double getTime();


// ******************** FUNCTIONS ************************



// main() function
// Initializes the user interface (and any user variables)
// then hands over control to the event handler, which calls 
// display() whenever the GL window needs to be redrawn.
int main(int argc, char** argv)
{

    // Process program arguments
    if(argc != 3) {
        printf("Usage: demo [width] [height]\n");
        printf("Using 800x600 window by default...\n");
        Win[0] = 800;
        Win[1] = 600;
    } else {
        Win[0] = atoi(argv[1]);
        Win[1] = atoi(argv[2]);
    }


    // Initialize glut, glui, and opengl
    glutInit(&argc, argv);
    initGlut(argv[0]);
    initGlui();
    initGl();

    // Invoke the standard GLUT main event loop
    glutMainLoop();

    return 0;         // never reached
}


// Initialize glut and create a window with the specified caption 
void initGlut(char* winName)
{
    // Set video mode: double-buffered, color, depth-buffered
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Create window
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Win[0],Win[1]);
    windowID = glutCreateWindow(winName);

    // Setup callback functions to handle events
    glutReshapeFunc(myReshape); // Call myReshape whenever window resized
    glutDisplayFunc(display);   // Call display whenever new frame needed 
}


// Quit button handler.  Called when the "quit" button is pressed.
void quitButton(int)
{
  exit(0);
}

// Animate button handler.  Called when the "animate" checkbox is pressed.
void animateButton(int)
{
  // synchronize variables that GLUT uses
  glui->sync_live();

  animation_frame = 0;
  if(animate_mode == 1) {
    // start animation
    GLUI_Master.set_glutIdleFunc(animate);
  } else {
    // stop animation
    GLUI_Master.set_glutIdleFunc(NULL);
  }
}

// Initialize GLUI and the user interface
void initGlui()
{
    GLUI_Master.set_glutIdleFunc(NULL);

    // Create GLUI window
    glui = GLUI_Master.create_glui("Glui Window", 0, Win[0]+10, 0);
    
    ///////////////////////////////////////////////////////////
    // TODO: 
    //   Add controls for additional joints here
    ///////////////////////////////////////////////////////////

     // Control for the Left Leg joint
    GLUI_Spinner *LL_spinner
        = glui->add_spinner("Left Leg", GLUI_SPINNER_FLOAT, &LL_rot);
    LL_spinner->set_speed(0.1);
    LL_spinner->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Control for the Right Leg Joint
    GLUI_Spinner *RL_spinner
        = glui->add_spinner("Right Leg", GLUI_SPINNER_FLOAT, &RL_rot);
    RL_spinner->set_speed(0.1);
    RL_spinner->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Control for the Left Foot joint
    GLUI_Spinner *LF_spinner
        = glui->add_spinner("Left Foot", GLUI_SPINNER_FLOAT, &LF_rot);
    LF_spinner->set_speed(0.1);
    LF_spinner->set_float_limits(FOOT_JOINT_MIN, FOOT_JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Control for the Right Foot joint
    GLUI_Spinner *RF_spinner
        = glui->add_spinner("Right Foot", GLUI_SPINNER_FLOAT, &RF_rot);
    RF_spinner->set_speed(0.1);
    RF_spinner->set_float_limits(FOOT_JOINT_MIN, FOOT_JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Control for the Head joint
    GLUI_Spinner *head_rot_spinner
        = glui->add_spinner("Head Rotation", GLUI_SPINNER_FLOAT, &head_rot);
    head_rot_spinner->set_speed(0.1);
    head_rot_spinner->set_float_limits(HEAD_JOINT_MIN, HEAD_JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Control for the Beak Y-axis
    GLUI_Spinner *BY_spinner
        = glui->add_spinner("Beak-Y", GLUI_SPINNER_FLOAT, &BY_pos);
    BY_spinner->set_speed(0.1);
    BY_spinner->set_float_limits(0, 10, GLUI_LIMIT_CLAMP);

    // Control for the Arm joint
    GLUI_Spinner *arm_spinner
        = glui->add_spinner("Arm", GLUI_SPINNER_FLOAT, &ARM_rot);
    arm_spinner->set_speed(0.1);
    arm_spinner->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Control for the Body rotate
    GLUI_Spinner *body_spinner
        = glui->add_spinner("Body rotation", GLUI_SPINNER_FLOAT, &body_rot);
    body_spinner->set_speed(0.1);
    body_spinner->set_float_limits(BODY_MIN, BODY_MAX, GLUI_LIMIT_CLAMP);


    // Control for the Penguin X-axis
    GLUI_Spinner *penguinx_spinner
        = glui->add_spinner("X", GLUI_SPINNER_FLOAT, &penguinx);
    penguinx_spinner->set_speed(0.1);
    penguinx_spinner->set_float_limits(0, 600, GLUI_LIMIT_CLAMP);

    // Control for the Penguin Y-axis
    GLUI_Spinner *penguiny_spinner
        = glui->add_spinner("Y", GLUI_SPINNER_FLOAT, &penguiny);
    penguiny_spinner->set_speed(0.1);
    penguiny_spinner->set_float_limits(-20, 20, GLUI_LIMIT_CLAMP);

    //display frame number
    glui->add_edittext("Frame:", GLUI_EDITTEXT_FLOAT, &animation_frame);


    // Add button to specify animation mode 
    glui->add_separator();
    glui->add_checkbox("Animate", &animate_mode, 0, animateButton);

    // Add "Quit" button
    glui->add_separator();
    glui->add_button("Quit", 0, quitButton);

    // Set the main window to be the "active" window
    glui->set_main_gfx_window(windowID);
}


// Performs most of the OpenGL intialization
void initGl(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);
}




// Callback idle function for animating the scene
void animate()
{
    
    ///////////////////////////////////////////////////////////
    // TODO:
    //   Modify this function animate the character's joints
    //   Note: Nothing should be drawn in this function!  OpenGL drawing
    //   should only happen in the display() callback.
    ///////////////////////////////////////////////////////////

    //based off this sin animate
    float animationLoop = abs(20 * sin(animation_frame * 0.15));

    if (penguinx == 600) animation_frame = 0;

    penguinx = animation_frame*2;

    penguiny = animationLoop;

    //symetrical when walking
    LL_rot = -RL_rot;

    //based off height of penguin, rotate leg
    RL_rot = 20 * sin(animation_frame * 0.15);

    //keep feet symetrical
    LF_rot = -RF_rot;

    //based off leg rotations
    RF_rot = animationLoop;

    //open and close twice in one loop
    BY_pos = abs(10 * sin(animation_frame * (0.3)));

    //move arms forward first
    ARM_rot = -JOINT_MAX * sin(animation_frame * 0.15);

    //head rotation
    head_rot = HEAD_JOINT_MIN * sin(animation_frame * 0.15);

    //body rotation
    body_rot = -abs(-BODY_MIN * sin(animation_frame * 0.15));

    // Update user interface
    glui->sync_live();

    // Tell glut window to update itself.  This will cause the display()
    // callback to be called, which renders the object (once you've written
    // the callback).
    glutSetWindow(windowID);
    glutPostRedisplay();

    // increment the frame number.
    animation_frame++;

    // Wait 50 ms between frames (20 frames per second)
    usleep(50000);
}


// Handles the window being resized by updating the viewport
// and projection matrices
void myReshape(int w, int h)
{
    // Setup projection matrix for new window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-w/2, w/2, -h/2, h/2);

    // Update OpenGL viewport and internal variables
    glViewport(0,0, w,h);
    Win[0] = w;
    Win[1] = h;
}



// display callback
//
// This gets called by the event handler to draw
// the scene, so this is where you need to build
// your scene -- make your changes and additions here.
// All rendering happens in this function.  For Assignment 1,
// updates to geometry should happen in the "animate" function.
void display(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);

    // OK, now clear the screen with the background colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

    // Setup the model-view transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ///////////////////////////////////////////////////////////
    // TODO:
    //   Modify this function draw the scene
    //   This should include function calls to pieces that
    //   apply the appropriate transformation matrice and
    //   render the individual body parts.
    ///////////////////////////////////////////////////////////

    // Draw our hinged object
    const float BODY_WIDTH = 160.0f;
    const float BODY_LENGTH = 270.0f;
    const float ARM_WIDTH = 40.0f;
    const float ARM_LENGTH =  90.0f;
    const float LEG_WIDTH = 20.0f;
    const float LEG_LENGTH = 60.0f;
    const float JOINT_RAD = 5.0f;
    const float HEAD_WIDTH = 120.0f;
    const float HEAD_LENGTH = 100.0f;
    const float EYE_RAD = 12.0f;
    const float EYE_RET = 7.0f;
    const float BEAK_WIDTH = 80.0f;
    const float BEAK_HEIGHT = 20.0f;

    glLineWidth(2.5);

    // Push the current transformation matrix on the stack
    glPushMatrix();


        //to move entire penguin
        glTranslatef(-penguinx, penguiny, 0);

        //to Translate the penguin to start position
        glTranslatef(Win[0]/2 - BODY_WIDTH/2,0,0);

        glRotatef(body_rot,0,0,1);

        // Draw the torso
        //green color
        glColor3f(0.0, 1.0, 0.0);

        // Draw the body
        drawTorso(BODY_WIDTH, BODY_LENGTH);

        //Draw the head
        glPushMatrix();
            //color red
            glColor3f(1,0,0);

            glTranslatef(0, BODY_LENGTH/2 + HEAD_LENGTH/2 - JOINT_RAD - 10,0);

            glTranslatef(0, HEAD_LENGTH/2 - 10,0);

            glRotatef(head_rot,0,0,1);

            glTranslatef(0,-HEAD_LENGTH/2 + 10,0);

            drawHead(HEAD_WIDTH, HEAD_LENGTH);

            //draw Joint
            glPushMatrix();

                glColor3f(0,1,0);

                glTranslatef(0, -HEAD_LENGTH/2 + 10, 0);

                drawCircle(JOINT_RAD, false);

            glPopMatrix();

            //draw Top Beak
            glPushMatrix();

                glColor3f(1,0,0);

                glTranslatef(-HEAD_WIDTH/2 - BEAK_WIDTH/4,  -3 * BEAK_HEIGHT/2 + BEAK_HEIGHT/2 + BEAK_HEIGHT/8,0);

                glTranslatef(0, BY_pos, 0);

                drawBeak(BEAK_WIDTH, BEAK_HEIGHT);

            glPopMatrix();

            //draw Bottom beak
            glPushMatrix();

                glColor3f(1,0,0);

                glTranslatef(-HEAD_WIDTH/2 - BEAK_WIDTH/4, -3 * BEAK_HEIGHT/2,0);

                drawRectangle(BEAK_WIDTH, BEAK_HEIGHT/4);

            glPopMatrix();

            //draw Eyes
            glPushMatrix();

                glColor3f(0,1,0);

                glTranslatef(-HEAD_WIDTH/5, HEAD_LENGTH/6,0);

                drawCircle(EYE_RAD, false);

                //draw eye retina
                glPushMatrix();

                    glTranslatef(-2, 0, 0);

                    drawCircle(EYE_RET, true);

                glPopMatrix();

            glPopMatrix();


         glPopMatrix();



        // Draw the 'arm'
        glPushMatrix();

            // Draw the square for the arm
            //red arm
            glColor3f(1.0, 0.0, 0.0);
            glTranslatef(BODY_WIDTH/6,0,0);
            glTranslatef(0,ARM_LENGTH/2 - 10,0);
            glRotatef(ARM_rot,0,0,1);
            glTranslatef(0,-ARM_LENGTH/2 + 10,0);
            drawArm(ARM_WIDTH, ARM_LENGTH);

            //draw arm Joint
            glPushMatrix();
                //green color
                glColor3f(0.0, 1.0, 0.0);
                glTranslatef(0, BODY_LENGTH/6 - 10,0);
                drawCircle(JOINT_RAD, false);
            glPopMatrix();

        glPopMatrix();

    //draw left leg joint
        glPushMatrix();

            glTranslatef(-BODY_WIDTH/4 + LEG_WIDTH/2,-BODY_LENGTH/2,0);

            glTranslatef(0, LEG_LENGTH/2 - 10,0);

            glRotatef(LL_rot, 0, 0, 1);

            glTranslatef(0, -LEG_LENGTH/2 + 10,0);

            glColor3f(0,1,0);

            glTranslatef(0, LEG_LENGTH/2 - 10, 0);

            drawCircle(JOINT_RAD, false);


            glTranslatef(0, -LEG_LENGTH/2 + 10, 0);

            //draw leg left
            glPushMatrix();

                glColor3f(1,0,0);
        
                drawRectangle(LEG_WIDTH, LEG_LENGTH);

                //draw left feet
                glPushMatrix(); 

                    glColor3f(1,0,0);

                    glTranslatef(-LEG_WIDTH/2 - 10,-LEG_LENGTH/2 + 10,0);

                    glRotatef(-90,0,0,1);

                    glTranslatef(0, LEG_LENGTH/2 - 10,0);

                    glRotatef(LF_rot, 0,0,1);

                    glTranslatef(0, -LEG_LENGTH/2 + 10,0);

                    drawRectangle(LEG_WIDTH, LEG_LENGTH);

                    //draw left foot Joint
                    glPushMatrix();
                        glColor3f(0,1,0);

                        glTranslatef(0, LEG_LENGTH/2 - 10, 0);

                        drawCircle(JOINT_RAD, false);
                    glPopMatrix();

                glPopMatrix();


            glPopMatrix();

        glPopMatrix();

        //draw right leg joint
        glPushMatrix();

            glTranslatef(BODY_WIDTH/4 - LEG_WIDTH/2,-BODY_LENGTH/2,0);

            glTranslatef(0, LEG_LENGTH/2 - 10,0);

            glRotatef(RL_rot, 0, 0, 1);

            glTranslatef(0, -LEG_LENGTH/2 + 10,0);

            glColor3f(0,1,0);

            glTranslatef(0, LEG_LENGTH/2 - 10, 0);

            drawCircle(JOINT_RAD, false);


            glTranslatef(0, -LEG_LENGTH/2 + 10, 0);

            //draw leg right
            glPushMatrix();

                glColor3f(1,0,0);
        
                drawRectangle(LEG_WIDTH, LEG_LENGTH);

                //draw right feet
                glPushMatrix(); 

                    glColor3f(1,0,0);

                    glTranslatef(-LEG_WIDTH/2 - 10,-LEG_LENGTH/2 + 10,0);

                    glRotatef(-90,0,0,1);

                    glTranslatef(0, LEG_LENGTH/2 - 10,0);

                    glRotatef(RF_rot, 0,0, 1);

                    glTranslatef(0, -LEG_LENGTH/2 + 10,0);

                    drawRectangle(LEG_WIDTH, LEG_LENGTH);

                    //draw right foot Joint
                    glPushMatrix();
                        glColor3f(0,1,0);

                        glTranslatef(0, LEG_LENGTH/2 - 10, 0);

                        drawCircle(JOINT_RAD, false);
                    glPopMatrix();

                glPopMatrix();


            glPopMatrix();

        glPopMatrix();


    // Retrieve the previous state of the transformation stack
    glPopMatrix();


    // Execute any GL functions that are in the queue just to be safe
    glFlush();

    // Now, show the frame buffer that we just drew into.
    // (this prevents flickering).
    glutSwapBuffers();
}


// Draw a square of the specified size, centered at the current location
void drawSquare(float width)
{
    // Draw the square
    glBegin(GL_LINE_LOOP);
    glVertex2d(-width/2, -width/2);
    glVertex2d(width/2, -width/2);
    glVertex2d(width/2, width/2);
    glVertex2d(-width/2, width/2);
    glEnd();
}

//draw rectangle
void drawRectangle(float width, float height)
{
    glBegin(GL_LINE_LOOP);
    glVertex2d(-width/2, height/2);
    glVertex2d(width/2, height/2);
    glVertex2d(width/2, -height/2);
    glVertex2d(-width/2, -height/2);
    glEnd();
}


//draw Joint
void drawCircle(float r, bool filled)
{   
    float deg = 360.0 / 50;
    if (!filled) glBegin(GL_LINE_LOOP);
    else glBegin(GL_POLYGON);
    for (unsigned int i = 0; i < 50; i++) {
        glVertex2d(r * cos(deg * i), r * sin(deg * i));
    }
    glEnd();
}

//draw arm
void drawArm(float width, float height)
{
    glBegin(GL_LINE_LOOP);
    glVertex2d(-width/2, height/2);
    glVertex2d(width/2, height/2);
    glVertex2d(width/4, -height/2);
    glVertex2d(-width/4, -height/2);
    glEnd();
}

//draw the torso
void drawTorso(float width, float height)
{
    glBegin(GL_LINE_LOOP);
    glVertex2d(-width/2, -height/4);
    glVertex2d(-width/4,-height/2);
    glVertex2d(width/4, -height/2);
    glVertex2d(width/2, -height/4);
    glVertex2d(width/4, height/2);
    glVertex2d(-width/4, height/2);
    glEnd();
}

void drawHead(float width, float height)
{

    glBegin(GL_LINE_LOOP);
    glVertex2d(-width/2, -height/2);
    glVertex2d(width/2, -height/2);
    glVertex2d(3 * width/8, height/4);
    glVertex2d(-width/8, height/2);
    glVertex2d(-3 *width/8, height/4);
    glEnd();

}

void drawBeak(float width, float height)
{

    glBegin(GL_LINE_LOOP);
    glVertex2d(-width/2, -height/2);
    glVertex2d(width/2, -height/2);
    glVertex2d(width/2, height/2);
    glVertex2d(-width/2, -height/4);
    glEnd();

}

