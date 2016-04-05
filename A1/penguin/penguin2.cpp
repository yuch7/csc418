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

//////////////////////////////////////////////////////
// TODO: Add additional joint parameters here
//////////////////////////////////////////////////////

// Head Joint parameters
const float HEAD_JOINT_MIN = -8.0f;
const float HEAD_JOINT_MAX =  8.0f;
// Head rotation angle
float head_rot = 0.0f;

// Left leg rotation angle
float joint_rot_inv = 0.0f;

// Right leg rotation angle
float hand_rot = 0.0f;

// Lower beak's Y position
float beak_y_pos = 0.0f;

// Penguin X-axis value
float peng_x = 400.0f;

// Penguin Y-axis value
float peng_y = 0.0f;

// Feet Joint parameters
const float FOOT_JOINT_MIN = -10.0f;
const float FOOT_JOINT_MAX =  10.0f;
// Left foot rotation angle
float left_foot_rot = 0.0f;
// Right foot rotation angle
float right_foot_rot = 0.0f;


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
void drawTrapezoid(float width1, float width2, float height);
void drawBody(float width);
void drawCircle(float r, bool fill);
void drawHead(float width);

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
        printf("Using 1000x400 window by default...\n");
        Win[0] = 1000;
        Win[1] = 400;
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

    // Create a control to specify the rotation of the joint
    GLUI_Spinner *joint_spinner
        = glui->add_spinner("Hand Joint", GLUI_SPINNER_FLOAT, &hand_rot);
    joint_spinner->set_speed(0.1);
    joint_spinner->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);

    ///////////////////////////////////////////////////////////
    // TODO: 
    //   Add controls for additional joints here
    ///////////////////////////////////////////////////////////

    // Control for the Left Leg joint
    GLUI_Spinner *left_leg_spinner
        = glui->add_spinner("Left Leg", GLUI_SPINNER_FLOAT, &joint_rot_inv);
    left_leg_spinner->set_speed(0.1);
    left_leg_spinner->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Control for the Right Leg Joing
    GLUI_Spinner *right_leg_spinner
        = glui->add_spinner("Right Leg", GLUI_SPINNER_FLOAT, &joint_rot);
    right_leg_spinner->set_speed(0.1);
    right_leg_spinner->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Control for the Left Foot joint
    GLUI_Spinner *left_foot_spinner
        = glui->add_spinner("Left Foot", GLUI_SPINNER_FLOAT, &left_foot_rot);
    left_foot_spinner->set_speed(0.1);
    left_foot_spinner->set_float_limits(FOOT_JOINT_MIN, FOOT_JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Control for the Right Foot joint
    GLUI_Spinner *right_foot_spinner
        = glui->add_spinner("Right Foot", GLUI_SPINNER_FLOAT, &right_foot_rot);
    right_foot_spinner->set_speed(0.1);
    right_foot_spinner->set_float_limits(FOOT_JOINT_MIN, FOOT_JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Control for the Head joint
    GLUI_Spinner *head_rot_spinner
        = glui->add_spinner("Head Rotation", GLUI_SPINNER_FLOAT, &head_rot);
    head_rot_spinner->set_speed(0.1);
    head_rot_spinner->set_float_limits(HEAD_JOINT_MIN, HEAD_JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Control for the Beak Y-axis
    GLUI_Spinner *beak_rot_spinner
        = glui->add_spinner("Beak-Y", GLUI_SPINNER_FLOAT, &beak_y_pos);
    beak_rot_spinner->set_speed(0.1);
    beak_rot_spinner->set_float_limits(HEAD_JOINT_MIN+HEAD_JOINT_MIN, HEAD_JOINT_MAX+HEAD_JOINT_MIN, GLUI_LIMIT_CLAMP);

    // Control for the Penguin X-axis
    GLUI_Spinner *peng_x_spinner
        = glui->add_spinner("X", GLUI_SPINNER_FLOAT, &peng_x);
    peng_x_spinner->set_speed(0.1);
    peng_x_spinner->set_float_limits(-400, 400, GLUI_LIMIT_CLAMP);

    // Control for the Penguin Y-axis
    GLUI_Spinner *peng_y_spinner
        = glui->add_spinner("Y", GLUI_SPINNER_FLOAT, &peng_y);
    peng_y_spinner->set_speed(0.1);
    peng_y_spinner->set_float_limits(0, HEAD_JOINT_MAX/2, GLUI_LIMIT_CLAMP);

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

    // Reset the animation frame to 0 if X-axis is out of bounds
    if (400 - animation_frame*2 < -400) {
        animation_frame = 0;
    }

    // Update geometry
    const double joint_rot_speed = 0.1;
    double joint_rot_t = (sin(animation_frame*joint_rot_speed) + 1.0) / 2.0;
    joint_rot = joint_rot_t * JOINT_MIN + (1 - joint_rot_t) * JOINT_MAX;

    ///////////////////////////////////////////////////////////
    // TODO:
    //   Modify this function animate the character's joints
    //   Note: Nothing should be drawn in this function!  OpenGL drawing
    //   should only happen in the display() callback.
    ///////////////////////////////////////////////////////////

    // Hand rotation is same as right leg rotation
    hand_rot = joint_rot;

    // Left leg rotation is basically inverse of the right leg rotation
    joint_rot_inv = -joint_rot;

    // Head rotates between -8 to 8 degrees and it is derived from sin function as well
    head_rot = joint_rot_t * HEAD_JOINT_MIN + (1 - joint_rot_t) * HEAD_JOINT_MAX;

    // Beak can go down on Y-axis by upto -16 degrees
    beak_y_pos = head_rot+HEAD_JOINT_MIN;

    // The X axis of penguin; Moves at same speed between the range [400, -400] starting from -400
    peng_x = 400 - animation_frame*2;

    // The Y axis of Penguin; It goes up by upto 4 degrees at each step
    peng_y = abs(head_rot/2);

    // Right foot rotation with range [-10, 10]
    right_foot_rot = joint_rot_t * FOOT_JOINT_MIN + (1 - joint_rot_t) * FOOT_JOINT_MAX;

    // Left foot rotation is basically inverse of the right foot rotation
    left_foot_rot = - right_foot_rot;

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
    // const float BODY_WIDTH = 30.0f;
    // const float BODY_LENGTH = 50.0f;
    const float ARM_LENGTH = 50.0f;
    const float ARM_WIDTH = 10.0f;

    const float TORSO_WIDTH = 130.0f;
    const float TORSO_LENGTH = 200.0f;

    const float LEG_WIDTH = 8.0f;
    const float LEG_LENGTH = 45.0f;

    const float HEAD_SIZE = 80.0f;

    const float BEAK_LENGTH = 50.0f;
    const float UPPER_BEAK_HEIGHT = 7.0f;
    const float LOWER_BEAK_HEIGHT = 5.0f;

    const float EYE_SIZE = 5.5f;
    const float EYEBALL_SIZE = 2.0f;



    // ************** Start TORSO **************
    glPushMatrix();

        // Draw basic road (It makes it easier to visualize transformations)
        glBegin(GL_POLYGON);
            glColor3f(0.5f, 0.5f, 0.7f);
            glEnable( GL_LINE_SMOOTH );
            glLineWidth( 1.5 );
            glVertex3f(-1000, -140, 0);
            glVertex3f(1000, -140, 0);
            glVertex3f(1000, -180, 0);
            glVertex3f(-1000, -180, 0);
        glEnd();

        // Transformation: translation of Penguin/Torso on X and Y
        glTranslatef(peng_x,peng_y,0.0);

        // Scale the body to size of torso
        glScalef(TORSO_WIDTH, TORSO_LENGTH, 1.0);

        // Set the colour to green
        glColor3f(0.0, 0.0, 0.0);

        // Draw the body as a box dimensions
        drawBody(1.0);


        // NOTE: I have commented and explained code in the Arm part; Lot of code repeats for the rest of the drawings
        //       so I have only put necessary comments for the rest of the code
        // ************** Start Arm **************
        glPushMatrix();
            // Move the arm a bit to the right and up
            glTranslatef(0.1, 0.3, 0.0);

            // Scale it back to have the Arm dimensions with respect to Torso dimension
            glScalef(1/TORSO_WIDTH, 1/TORSO_LENGTH, 1.0);

            // Draw circle for joint
            drawCircle(3,false);

            // Transformation: Hand joint rotation
            glRotatef(hand_rot, 0.0, 0.0, 1.0);

            // Scale the trapezoid by the fixed arm width and length
            glScalef(ARM_WIDTH, ARM_LENGTH, 1.0);

            // Translate trapezoid to align center
            glTranslatef(0.0, -0.8, 0.0);
            glColor3f(0.0, 1.0, 0.0);
            drawTrapezoid(3.0,2.0,2.0);

        glPopMatrix();
        // ************** End Arm **************


        // ************** Start Left Leg **************
        glPushMatrix();
            glTranslatef(0.15, -0.4, 0.0);
            glScalef(1/TORSO_WIDTH, 1/TORSO_LENGTH, 1.0);

            // Draw joint
            drawCircle(3,false);

            glRotatef(25, 0.0, 0.0, 1.0);

            // Transformation: Left lef rotation joint
            glRotatef(joint_rot, 0.0, 0.0, 1.0);
            glScalef(LEG_WIDTH, LEG_LENGTH, 1.0);
            glTranslatef(0.0, -0.5, 0.0);
            glColor3f(0.0, 1.0, 0.0);
            drawTrapezoid(3.0,2.0,1.5);

            // ************** Start Left Foot **************
            glPushMatrix();
                glTranslatef(0, -0.4, 0.0);

                // Scale the Foot to fix the dimensions with respect to left leg
                glScalef(1/LEG_WIDTH, 1/LEG_LENGTH, 1.0);

                // Draw Joint
                drawCircle(3,false);

                glRotatef(-75, 0.0, 0.0, 1.0);

                // Left foot rotation
                glRotatef(left_foot_rot, 0.0, 0.0, 1.0);
                glScalef(LEG_WIDTH, LEG_LENGTH, 1.0);
                glTranslatef(0.0, -0.5, 0.0);
                glColor3f(0.0, 1.0, 0.0);
                drawTrapezoid(3.0,2.0,1.5);
            glPopMatrix();
            // ************** End Left Foot **************
        glPopMatrix();
        // ************** End Left Leg **************


        // ************** Start Right Leg **************
        glPushMatrix();
            glTranslatef(-0.15, -0.4, 0.0);
            glScalef(1/TORSO_WIDTH, 1/TORSO_LENGTH, 1.0);

            drawCircle(3,false);

            glRotatef(15, 0.0, 0.0, 1.0);
            glRotatef(joint_rot_inv, 0.0, 0.0, 1.0);
            glScalef(LEG_WIDTH, LEG_LENGTH, 1.0);
            glTranslatef(0.0, -0.5, 0.0);
            glColor3f(0.0, 1.0, 0.0);
            drawTrapezoid(3.0,2.0,1.5);

            // ************** Start Right Foot **************
            glPushMatrix();
                glTranslatef(0, -0.4, 0.0);
                glScalef(1/LEG_WIDTH, 1/LEG_LENGTH, 1.0);

                drawCircle(3,false);

                glRotatef(-75, 0.0, 0.0, 1.0);
                glRotatef(right_foot_rot, 0.0, 0.0, 1.0);
                glScalef(LEG_WIDTH, LEG_LENGTH, 1.0);
                glTranslatef(0.0, -0.5, 0.0);
                glColor3f(0.0, 1.0, 0.0);
                drawTrapezoid(3.0,2.0,1.5);

            glPopMatrix();
            // ************** End Right Foot **************
        glPopMatrix();
        // ************** End Right Leg **************


        // ************** Start Head **************
        glPushMatrix();
            glTranslatef(0, 0.48, 0.0);
            glScalef(1/TORSO_WIDTH, 1/TORSO_LENGTH, 1.0);

            drawCircle(3,false);
            glRotatef(head_rot, 0.0, 0.0, 1.0);
            glScalef(HEAD_SIZE, 5*HEAD_SIZE/6, 1.0);
            glTranslatef(-0.5, -0.1, 0.0);
            glColor3f(0.0, 1.0, 0.0);
            drawHead(1.0);

            // ************** Start Lower Beak **************
            glPushMatrix();
                glTranslatef(-0.65, 0.3, 0.0);

                glScalef(1/HEAD_SIZE, 1/HEAD_SIZE, 1.0);

                glScalef(BEAK_LENGTH, LOWER_BEAK_HEIGHT, 1.0);
                glRotatef(90, 0.0, 0.0, 1.0);

                // Translate X-axis since we will be rotating by 90 degrees
                glTranslatef(-0.5+(beak_y_pos/10.0), -0.7, 0.0);
                glColor3f(0.0, 0.0, 0.0);
                drawTrapezoid(0.5,1.5,1.0);
            glPopMatrix();
            // ************** End Lower Beak **************

            // ************** Start Upper Beak **************
            glPushMatrix();
                glTranslatef(-0.65, 0.4, 0.0);

                // Scale it back so the beak dimensions are calculated with respect to Head dimensions
                glScalef(1/HEAD_SIZE, 1/HEAD_SIZE, 1.0);

                glScalef(BEAK_LENGTH, UPPER_BEAK_HEIGHT, 1.0);
                glRotatef(90, 0.0, 0.0, 1.0);
                glTranslatef(-0.5, -0.7, 0.0);
                glColor3f(0.0, 0.0, 0.0);
                drawTrapezoid(0.5,1.5,1.0);
            glPopMatrix();
            // ************** End Upper Beak **************

            // ************** Start Eye **************
            glPushMatrix();
                glTranslatef(0.25, 0.6, 0.0);
                glScalef(1/HEAD_SIZE, 1/HEAD_SIZE, 1.0);

                // Draw Eye
                drawCircle(EYE_SIZE,false);
                
                // ************** Start Eyeball **************
                glPushMatrix();
                    // Draw eyeball with radius EYEBALL_SIZE
                    glTranslatef(-1, -0.5, 0.0);
                    drawCircle(EYEBALL_SIZE,true);
                glPopMatrix();
                // ************** End Eyeball **************
            glPopMatrix();
            // ************** End Eye **************
        glPopMatrix();
        // ************** End Head **************

    // Retrieve the previous state of the transformation stack
    glPopMatrix();
    // ************** End Penguin/Torso **************


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

// Draw trapezoid with 2 different widths and a same height
void drawTrapezoid(float width1, float width2, float height) {
    glBegin(GL_LINE_LOOP);
    glVertex2d(-width1/2, height/2);
    glVertex2d(-width2/2, -height/2);
    glVertex2d(width2/2, -height/2);
    glVertex2d(width1/2, height/2);
    glEnd();
}

// Draw the penguin body/torso with a box dimension
void drawBody(float width)
{
    glBegin(GL_LINE_LOOP);
    glVertex2d(-width/2, -width/4);
    glVertex2d(-width/4, -width/2);
    glVertex2d(width/4, -width/2);
    glVertex2d(width/2, -width/4);
    glVertex2d(width/4, width/2);
    glVertex2d(-width/4, width/2);
    glEnd();
}

// Draw circle for joints; if fill is true, fill with the same color
void drawCircle(float r, bool fill) {
    float deg = 360.0 / 50;
    if (fill) {
        glBegin(GL_POLYGON);
    } else {
        glBegin(GL_LINE_LOOP);
    }
    glColor3f(0.5, 0.5, 1.0);
    for (unsigned int i = 0; i < 50; i++) {
        glVertex2d(r * cos(deg * i), r * sin(deg * i));
    }
    glEnd();
}

// Draw the head for the penguin in a box dimension; scale later
void drawHead(float width) {
    glBegin(GL_LINE_LOOP);
    glVertex2d(0, 0);
    glVertex2d(width/8,7*width/8);
    glVertex2d(width/3,width);
    glVertex2d(13*width/16,13*width/16);
    glVertex2d(width,0);
    glEnd();
}