/***********************************************************
             CSC418, FALL 2009
 
                 penguin.cpp
                 author: Mike Pratscher
                 based on code by: Eron Steger, J. Radulovich

		Main source file for assignment 2
		Uses OpenGL, GLUT and GLUI libraries
  
    Instructions:
        Please read the assignment page to determine 
        exactly what needs to be implemented.  Then read 
        over this file and become acquainted with its 
        design. In particular, see lines marked 'README'.
		
		Be sure to also look over keyframe.h and vector.h.
		While no changes are necessary to these files, looking
		them over will allow you to better understand their
		functionality and capabilites.

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

#include "keyframe.h"
#include "timer.h"
#include "vector.h"


// *************** GLOBAL VARIABLES *************************


const float PI = 3.14159;

const float SPINNER_SPEED = 0.1;

// --------------- USER INTERFACE VARIABLES -----------------

// Window settings
int windowID;				// Glut window ID (for display)
int Win[2];					// window (x,y) size

GLUI* glui_joints;			// Glui window with joint controls
GLUI* glui_keyframe;		// Glui window with keyframe controls
GLUI* glui_render;			// Glui window for render style
GLUI* glui_light;			// Glui window for light control

char msg[256];				// String used for status message
GLUI_StaticText* status;	// Status message ("Status: <msg>")


// ---------------- ANIMATION VARIABLES ---------------------

// Camera settings
bool updateCamZPos = false;
int  lastX = 0;
int  lastY = 0;
const float ZOOM_SCALE = 0.01;

GLdouble camXPos =  0.0;
GLdouble camYPos =  0.0;
GLdouble camZPos = -7.5;

const GLdouble CAMERA_FOVY = 60.0;
const GLdouble NEAR_CLIP   = 0.1;
const GLdouble FAR_CLIP    = 1000.0;

//light position
float lightpos = 0.0;

// Render settings
enum { WIREFRAME, SOLID, OUTLINED, METALLIC, MATTE };	// README: the different render styles
int renderStyle = WIREFRAME;			// README: the selected render style

// Default color
float defaultColor = 1;

//Redraw wireframe
void (* drawFxn) (int lineDrawn);

// Animation settings
int animate_mode = 0;			// 0 = no anim, 1 = animate

// Keyframe settings
const char filenameKF[] = "keyframes.txt";	// file for loading / saving keyframes

Keyframe* keyframes;			// list of keyframes

int maxValidKeyframe   = 0;		// index of max VALID keyframe (in keyframe list)
const int KEYFRAME_MIN = 0;
const int KEYFRAME_MAX = 32;	// README: specifies the max number of keyframes

// Frame settings
char filenameF[128];			// storage for frame filename

int frameNumber = 0;			// current frame being dumped
int frameToFile = 0;			// flag for dumping frames to file

const float DUMP_FRAME_PER_SEC = 24.0;		// frame rate for dumped frames
const float DUMP_SEC_PER_FRAME = 1.0 / DUMP_FRAME_PER_SEC;

// Time settings
Timer* animationTimer;
Timer* frameRateTimer;

const float TIME_MIN = 0.0;
const float TIME_MAX = 10.0;	// README: specifies the max time of the animation
const float SEC_PER_FRAME = 1.0 / 60.0;

// Joint settings

// README: This is the key data structure for
// updating keyframes in the keyframe list and
// for driving the animation.
//   i) When updating a keyframe, use the values
//      in this data structure to update the
//      appropriate keyframe in the keyframe list.
//  ii) When calculating the interpolated pose,
//      the resulting pose vector is placed into
//      this data structure. (This code is already
//      in place - see the animate() function)
// iii) When drawing the scene, use the values in
//      this data structure (which are set in the
//      animate() function as described above) to
//      specify the appropriate transformations.
Keyframe* joint_ui_data;

// README: To change the range of a particular DOF,
// simply change the appropriate min/max values below
const float ROOT_TRANSLATE_X_MIN = -5.0;
const float ROOT_TRANSLATE_X_MAX =  5.0;
const float ROOT_TRANSLATE_Y_MIN = -5.0;
const float ROOT_TRANSLATE_Y_MAX =  5.0;
const float ROOT_TRANSLATE_Z_MIN = -5.0;
const float ROOT_TRANSLATE_Z_MAX =  5.0;
const float ROOT_ROTATE_X_MIN    = -180.0;
const float ROOT_ROTATE_X_MAX    =  180.0;
const float ROOT_ROTATE_Y_MIN    = -180.0;
const float ROOT_ROTATE_Y_MAX    =  180.0;
const float ROOT_ROTATE_Z_MIN    = -180.0;
const float ROOT_ROTATE_Z_MAX    =  180.0;
const float HEAD_MIN             = -180.0;
const float HEAD_MAX             =  180.0;
const float SHOULDER_PITCH_MIN   = -45.0;
const float SHOULDER_PITCH_MAX   =  45.0;
const float SHOULDER_YAW_MIN     = -45.0;
const float SHOULDER_YAW_MAX     =  45.0;
const float SHOULDER_ROLL_MIN    = 	-45.0;
const float SHOULDER_ROLL_MAX    =  0;
const float HIP_PITCH_MIN        = -45.0;
const float HIP_PITCH_MAX        =  45.0;
const float HIP_YAW_MIN          = -45.0;
const float HIP_YAW_MAX          =  45.0;
const float HIP_ROLL_MIN         = -45.0;
const float HIP_ROLL_MAX         =  45.0;
const float BEAK_MIN             =  0.0;
const float BEAK_MAX             =  0.2;
const float ELBOW_MIN            =  0.0;
const float ELBOW_MAX            = 45.0;
const float KNEE_MIN             =  0.0;
const float KNEE_MAX             = 75.0;

//all half of the actual values
const float BODYTOPLEN = 0.5, BODYBOTLEN = 1.0, BODYHEIGHT = 1.5; 
const float TOPARMWID = 0.25, BOTARMWID = 0.2, ARMLEN = 0.75, ARMTHICK = 0.1;
const float FOOTLEN = 0.2, FOOTHEIGHT = 0.1, FOOTDEPTH = 0.1;
const float LEGTOPWID = 0.1, LEGBOTWID = 0.1, LEGLEN = 0.4, LEGDEPTH = 0.1;
const float HEADTOPLEN = BODYTOPLEN, HEADBOTLEN = BODYTOPLEN + 0.1, HEADHEIGHT = 0.4; 
const float EYERAD = 0.05, CORNRAD = 0.03;
const float BEAKLEN = 0.2, BEAKHEIGHT = 0.05, BEAKDEPTH = 0.1;
const float HANDLEN = 0.2, HANDWID = 0.1;

//light max and min
const float LIGHTMAX = 360, LIGHTMIN = 0;


// ***********  FUNCTION HEADER DECLARATIONS ****************


// Initialization functions
void initDS();
void initGlut(int argc, char** argv);
void initGlui();
void initGl();


// Callbacks for handling events in glut
void reshape(int w, int h);
void animate();
void display(void);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);


// Functions to help draw the object
Vector getInterpolatedJointDOFS(float time);
void drawBody(int lineDrawn = 0);
void drawArm(int lineDrawn = 0);
void drawHand(int lineDrawn = 0);
void drawLegs(int lineDrawn = 0);
void drawFoot(int lineDrawn = 0);
void drawHead(int lineDrawn = 0);
void drawEye();
void drawBeak(int lineDrawn = 0);
void drawOUTLINED();


// Image functions
void writeFrame(char* filename, bool pgm, bool frontBuffer);


// ******************** FUNCTIONS ************************


float deg_rad (float deg) {
	return deg * 3.141592653 / 180;
}

// main() function
// Initializes the user interface (and any user variables)
// then hands over control to the event handler, which calls 
// display() whenever the GL window needs to be redrawn.
int main(int argc, char** argv)
{

    // Process program arguments
    if(argc != 3) {
        printf("Usage: demo [width] [height]\n");
        printf("Using 640x480 window by default...\n");
        Win[0] = 640;
        Win[1] = 480;
    } else {
        Win[0] = atoi(argv[1]);
        Win[1] = atoi(argv[2]);
    }


    // Initialize data structs, glut, glui, and opengl
	initDS();
    initGlut(argc, argv);
    initGlui();
    initGl();

    // Invoke the standard GLUT main event loop
    glutMainLoop();

    return 0;         // never reached
}


// Create / initialize global data structures
void initDS()
{
	keyframes = new Keyframe[KEYFRAME_MAX];
	for( int i = 0; i < KEYFRAME_MAX; i++ )
		keyframes[i].setID(i);

	animationTimer = new Timer();
	frameRateTimer = new Timer();
	joint_ui_data  = new Keyframe();
}


// Initialize glut and create a window with the specified caption 
void initGlut(int argc, char** argv)
{
	// Init GLUT
	glutInit(&argc, argv);

    // Set video mode: double-buffered, color, depth-buffered
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Create window
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Win[0],Win[1]);
    windowID = glutCreateWindow(argv[0]);

    // Setup callback functions to handle events
    glutReshapeFunc(reshape);	// Call reshape whenever window resized
    glutDisplayFunc(display);	// Call display whenever new frame needed
	glutMouseFunc(mouse);		// Call mouse whenever mouse button pressed
	glutMotionFunc(motion);		// Call motion whenever mouse moves while button pressed
}


// Load Keyframe button handler. Called when the "load keyframe" button is pressed
void loadKeyframeButton(int)
{
	// Get the keyframe ID from the UI
	int keyframeID = joint_ui_data->getID();

	// Update the 'joint_ui_data' variable with the appropriate
	// entry from the 'keyframes' array (the list of keyframes)
	*joint_ui_data = keyframes[keyframeID];

	// Sync the UI with the 'joint_ui_data' values
	glui_joints->sync_live();
	glui_keyframe->sync_live();

	// Let the user know the values have been loaded
	sprintf(msg, "Status: Keyframe %d loaded successfully", keyframeID);
	status->set_text(msg);
}

// Update Keyframe button handler. Called when the "update keyframe" button is pressed
void updateKeyframeButton(int)
{
	///////////////////////////////////////////////////////////
	// TODO:
	//   Modify this function to save the UI joint values interpolation
	//   the appropriate keyframe entry in the keyframe list
	//   when the user clicks on the 'Update Keyframe' button.
	//   Refer to the 'loadKeyframeButton' function for help.
	///////////////////////////////////////////////////////////

	// Get the keyframe ID from the UI
	int keyframeID = joint_ui_data->getID();

	// Update the 'maxValidKeyframe' index variable
	// (it will be needed when doing the interpolation)
	if(keyframeID > maxValidKeyframe){
		maxValidKeyframe = keyframeID;
	}

	// Update the appropriate entry in the 'keyframes' array
	// with the 'joint_ui_data' data
	keyframes[keyframeID] = *joint_ui_data;	

	// Let the user know the values have been updated
	sprintf(msg, "Status: Keyframe %d updated successfully", keyframeID);
	status->set_text(msg);
}

// Load Keyframes From File button handler. Called when the "load keyframes from file" button is pressed
//
// ASSUMES THAT THE FILE FORMAT IS CORRECT, ie, there is no error checking!
//
void loadKeyframesFromFileButton(int)
{
	// Open file for reading
	FILE* file = fopen(filenameKF, "r");
	if( file == NULL )
	{
		sprintf(msg, "Status: Failed to open file %s", filenameKF);
		status->set_text(msg);
		return;
	}

	// Read in maxValidKeyframe first
	fscanf(file, "%d", &maxValidKeyframe);

	// Now read in all keyframes in the format:
	//    id
	//    time
	//    DOFs
	//
	for( int i = 0; i <= maxValidKeyframe; i++ )
	{
		fscanf(file, "%d", keyframes[i].getIDPtr());
		fscanf(file, "%f", keyframes[i].getTimePtr());

		for( int j = 0; j < Keyframe::NUM_JOINT_ENUM; j++ )
			fscanf(file, "%f", keyframes[i].getDOFPtr(j));
	}

	// Close file
	fclose(file);

	// Let the user know the keyframes have been loaded
	sprintf(msg, "Status: Keyframes loaded successfully");
	status->set_text(msg);
}

// Save Keyframes To File button handler. Called when the "save keyframes to file" button is pressed
void saveKeyframesToFileButton(int)
{
	// Open file for writing
	FILE* file = fopen(filenameKF, "w");
	if( file == NULL )
	{
		sprintf(msg, "Status: Failed to open file %s", filenameKF);
		status->set_text(msg);
		return;
	}

	// Write out maxValidKeyframe first
	fprintf(file, "%d\n", maxValidKeyframe);
	fprintf(file, "\n");

	// Now write out all keyframes in the format:
	//    id
	//    time
	//    DOFs
	//
	for( int i = 0; i <= maxValidKeyframe; i++ )
	{
		fprintf(file, "%d\n", keyframes[i].getID());
		fprintf(file, "%f\n", keyframes[i].getTime());

		for( int j = 0; j < Keyframe::NUM_JOINT_ENUM; j++ )
			fprintf(file, "%f\n", keyframes[i].getDOF(j));

		fprintf(file, "\n");
	}

	// Close file
	fclose(file);

	// Let the user know the keyframes have been saved
	sprintf(msg, "Status: Keyframes saved successfully");
	status->set_text(msg);
}

// Animate button handler.  Called when the "animate" button is pressed.
void animateButton(int)
{
  // synchronize variables that GLUT uses
  glui_keyframe->sync_live();

  // toggle animation mode and set idle function appropriately
  if( animate_mode == 0 )
  {
	// start animation
	frameRateTimer->reset();
	animationTimer->reset();

	animate_mode = 1;
	GLUI_Master.set_glutIdleFunc(animate);

	// Let the user know the animation is running
	sprintf(msg, "Status: Animating...");
	status->set_text(msg);
  }
  else
  {
	// stop animation
	animate_mode = 0;
	GLUI_Master.set_glutIdleFunc(NULL);

	// Let the user know the animation has stopped
	sprintf(msg, "Status: Animation stopped");
	status->set_text(msg);
  }
}

// Render Frames To File button handler. Called when the "Render Frames To File" button is pressed.
void renderFramesToFileButton(int)
{
	// Calculate number of frames to generate based on dump frame rate
	int numFrames = int(keyframes[maxValidKeyframe].getTime() * DUMP_FRAME_PER_SEC) + 1;

	// Generate frames and save to file
	frameToFile = 1;
	for( frameNumber = 0; frameNumber < numFrames; frameNumber++ )
	{
		// Get the interpolated joint DOFs
		joint_ui_data->setDOFVector( getInterpolatedJointDOFS(frameNumber * DUMP_SEC_PER_FRAME) );

		// Let the user know which frame is being rendered
		sprintf(msg, "Status: Rendering frame %d...", frameNumber);
		status->set_text(msg);

		// Render the frame
		display();
	}
	frameToFile = 0;

	// Let the user know how many frames were generated
	sprintf(msg, "Status: %d frame(s) rendered to file", numFrames);
	status->set_text(msg);
}

// Quit button handler.  Called when the "quit" button is pressed.
void quitButton(int)
{
  exit(0);
}

// Initialize GLUI and the user interface
void initGlui()
{
	GLUI_Panel* glui_panel;
	GLUI_Spinner* glui_spinner;
	GLUI_RadioGroup* glui_radio_group;


    GLUI_Master.set_glutIdleFunc(NULL);


	// Create GLUI window (joint controls) ***************
	//
	glui_joints = GLUI_Master.create_glui("Joint Control", 0, Win[0]+12, 0);

    // Create controls to specify root position and orientation
	glui_panel = glui_joints->add_panel("Root");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_X));
	glui_spinner->set_float_limits(ROOT_TRANSLATE_X_MIN, ROOT_TRANSLATE_X_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_Y));
	glui_spinner->set_float_limits(ROOT_TRANSLATE_Y_MIN, ROOT_TRANSLATE_Y_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_Z));
	glui_spinner->set_float_limits(ROOT_TRANSLATE_Z_MIN, ROOT_TRANSLATE_Z_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_X));
	glui_spinner->set_float_limits(ROOT_ROTATE_X_MIN, ROOT_ROTATE_X_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_Y));
	glui_spinner->set_float_limits(ROOT_ROTATE_Y_MIN, ROOT_ROTATE_Y_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_Z));
	glui_spinner->set_float_limits(ROOT_ROTATE_Z_MIN, ROOT_ROTATE_Z_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify head rotation
	glui_panel = glui_joints->add_panel("Head");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "head:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::HEAD));
	glui_spinner->set_float_limits(HEAD_MIN, HEAD_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify beak
	glui_panel = glui_joints->add_panel("Beak");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "beak:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BEAK));
	glui_spinner->set_float_limits(BEAK_MIN, BEAK_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);


	glui_joints->add_column(false);


	// Create controls to specify right arm
	glui_panel = glui_joints->add_panel("Right arm");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER_PITCH));
	glui_spinner->set_float_limits(SHOULDER_PITCH_MIN, SHOULDER_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER_YAW));
	glui_spinner->set_float_limits(SHOULDER_YAW_MIN, SHOULDER_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER_ROLL));
	glui_spinner->set_float_limits(SHOULDER_ROLL_MAX, -SHOULDER_ROLL_MIN, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "elbow:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_ELBOW));
	glui_spinner->set_float_limits(ELBOW_MIN, ELBOW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify left arm
	glui_panel = glui_joints->add_panel("Left arm");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER_PITCH));
	glui_spinner->set_float_limits(SHOULDER_PITCH_MIN, SHOULDER_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER_YAW));
	glui_spinner->set_float_limits(SHOULDER_YAW_MIN, SHOULDER_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER_ROLL));
	glui_spinner->set_float_limits(SHOULDER_ROLL_MIN, SHOULDER_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "elbow:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_ELBOW));
	glui_spinner->set_float_limits(ELBOW_MIN, ELBOW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);


	glui_joints->add_column(false);


	// Create controls to specify right leg
	glui_panel = glui_joints->add_panel("Right leg");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP_PITCH));
	glui_spinner->set_float_limits(HIP_PITCH_MIN, HIP_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP_YAW));
	glui_spinner->set_float_limits(HIP_YAW_MIN, HIP_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP_ROLL));
	glui_spinner->set_float_limits(HIP_ROLL_MIN, HIP_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "knee:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_KNEE));
	glui_spinner->set_float_limits(KNEE_MIN, KNEE_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify left leg
	glui_panel = glui_joints->add_panel("Left leg");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP_PITCH));
	glui_spinner->set_float_limits(HIP_PITCH_MIN, HIP_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP_YAW));
	glui_spinner->set_float_limits(HIP_YAW_MIN, HIP_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP_ROLL));
	glui_spinner->set_float_limits(HIP_ROLL_MIN, HIP_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "knee:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_KNEE));
	glui_spinner->set_float_limits(KNEE_MIN, KNEE_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);


	///////////////////////////////////////////////////////////
	// TODO (for controlling light source position & additional
	//      rendering styles):
	//   Add more UI spinner elements here. Be sure to also
	//   add the appropriate min/max range values to this
	//   file, and to also add the appropriate enums to the
	//   enumeration in the Keyframe class (keyframe.h).
	///////////////////////////////////////////////////////////

	//create GLUI window for lighting
	glui_light = GLUI_Master.create_glui("Light Control",0,Win[0]+12,350);
	glui_panel = glui_light->add_panel("", GLUI_PANEL_NONE);

	glui_spinner = glui_light->add_spinner_to_panel(glui_panel, "Light Position (circle):", GLUI_SPINNER_FLOAT, &lightpos);
	glui_spinner->set_float_limits(LIGHTMIN, LIGHTMAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);
	//
	// ***************************************************


	// Create GLUI window (keyframe controls) ************
	//
	glui_keyframe = GLUI_Master.create_glui("Keyframe Control", 0, 0, Win[1]+64);

	// Create a control to specify the time (for setting a keyframe)
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_spinner = glui_keyframe->add_spinner_to_panel(glui_panel, "Time:", GLUI_SPINNER_FLOAT, joint_ui_data->getTimePtr());
	glui_spinner->set_float_limits(TIME_MIN, TIME_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create a control to specify a keyframe (for updating / loading a keyframe)
	glui_keyframe->add_column_to_panel(glui_panel, false);
	glui_spinner = glui_keyframe->add_spinner_to_panel(glui_panel, "Keyframe ID:", GLUI_SPINNER_INT, joint_ui_data->getIDPtr());
	glui_spinner->set_int_limits(KEYFRAME_MIN, KEYFRAME_MAX-1, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_keyframe->add_separator();

	// Add buttons to load and update keyframes
	// Add buttons to load and save keyframes from a file
	// Add buttons to start / stop animation and to render frames to file
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_keyframe->add_button_to_panel(glui_panel, "Load Keyframe", 0, loadKeyframeButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Load Keyframes From File", 0, loadKeyframesFromFileButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Start / Stop Animation", 0, animateButton);
	glui_keyframe->add_column_to_panel(glui_panel, false);
	glui_keyframe->add_button_to_panel(glui_panel, "Update Keyframe", 0, updateKeyframeButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Save Keyframes To File", 0, saveKeyframesToFileButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Render Frames To File", 0, renderFramesToFileButton);

	glui_keyframe->add_separator();

	// Add status line
	glui_panel = glui_keyframe->add_panel("");
	status = glui_keyframe->add_statictext_to_panel(glui_panel, "Status: Ready");

	// Add button to quit
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_keyframe->add_button_to_panel(glui_panel, "Quit", 0, quitButton);
	//
	// ***************************************************


	// Create GLUI window (render controls) ************
	//
	glui_render = GLUI_Master.create_glui("Render Control", 0, 367, Win[1]+64);

	// Create control to specify the render style
	glui_panel = glui_render->add_panel("Render Style");
	glui_radio_group = glui_render->add_radiogroup_to_panel(glui_panel, &renderStyle);
	glui_render->add_radiobutton_to_group(glui_radio_group, "Wireframe");
	glui_render->add_radiobutton_to_group(glui_radio_group, "Solid");
	glui_render->add_radiobutton_to_group(glui_radio_group, "Solid w/ outlines");
	glui_render->add_radiobutton_to_group(glui_radio_group, "Metallic");
	glui_render->add_radiobutton_to_group(glui_radio_group, "Matte");
	//
	// ***************************************************


	// Tell GLUI windows which window is main graphics window
	glui_joints->set_main_gfx_window(windowID);
	glui_keyframe->set_main_gfx_window(windowID);
	glui_render->set_main_gfx_window(windowID);
	glui_light->set_main_gfx_window(windowID);
}


// Performs most of the OpenGL intialization
void initGl(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);
}


// Calculates the interpolated joint DOF vector
// using Catmull-Rom interpolation of the keyframes
Vector getInterpolatedJointDOFS(float time)
{
	// Need to find the keyframes bewteen which
	// the supplied time lies.
	// At the end of the loop we have:
	//    keyframes[i-1].getTime() < time <= keyframes[i].getTime()
	//
	int i = 0;
	while( i <= maxValidKeyframe && keyframes[i].getTime() < time )
		i++;

	// If time is before or at first defined keyframe, then
	// just use first keyframe pose
	if( i == 0 )
		return keyframes[0].getDOFVector();

	// If time is beyond last defined keyframe, then just
	// use last keyframe pose
	if( i > maxValidKeyframe )
		return keyframes[maxValidKeyframe].getDOFVector();

	// Need to normalize time to (0, 1]
	float alpha = (time - keyframes[i-1].getTime()) / (keyframes[i].getTime() - keyframes[i-1].getTime());

	// Get appropriate data points
	// for computing the interpolation
	Vector p0 = keyframes[i-1].getDOFVector();
	Vector p1 = keyframes[i].getDOFVector();

	///////////////////////////////////////////////////////////
	// TODO (animation using Catmull-Rom):
	//   Currently the code operates using linear interpolation
    //   Modify this function so it uses Catmull-Rom instead.
	///////////////////////////////////////////////////////////

	
	Vector t0, t1;
	//begining of spline
	if(i == 1)
	{
		t0 = keyframes[i].getDOFVector() - keyframes[i-1].getDOFVector();
		t1 = (keyframes[i+1].getDOFVector() - keyframes[i-1].getDOFVector()) * 0.5;
	}
	//end of spline
	else if(i == maxValidKeyframe)
	{
		t0 = (keyframes[i].getDOFVector() - keyframes[i-2].getDOFVector()) * 0.5;
		t1 = keyframes[i].getDOFVector() - keyframes[i-1].getDOFVector();
	}
	else
	//rest
	{
		t0 = (keyframes[i].getDOFVector()   - keyframes[i-2].getDOFVector()) * 0.5;
		t1 = (keyframes[i+1].getDOFVector() - keyframes[i-1].getDOFVector()) * 0.5;
	}

	// Return the interpolated Vector
	Vector a0 = p0;
	Vector a1 = t0;
	Vector a2 = p0 * (-3) + p1 * 3 + t0 * (-2) + t1 * (-1);
	Vector a3 = p0 * 2 + p1 * (-2) + t0 + t1;

	return (((a3 * alpha + a2) * alpha + a1) * alpha + a0);
}


// Callback idle function for animating the scene
void animate()
{
	// Only update if enough time has passed
	// (This locks the display to a certain frame rate rather
	//  than updating as fast as possible. The effect is that
	//  the animation should run at about the same rate
	//  whether being run on a fast machine or slow machine)
	if( frameRateTimer->elapsed() > SEC_PER_FRAME )
	{
		// Tell glut window to update itself. This will cause the display()
		// callback to be called, which renders the object (once you've written
		// the callback).
		glutSetWindow(windowID);
		glutPostRedisplay();

		// Restart the frame rate timer
		// for the next frame
		frameRateTimer->reset();
	}
}


// Handles the window being resized by updating the viewport
// and projection matrices
void reshape(int w, int h)
{
	// Update internal variables and OpenGL viewport
	Win[0] = w;
	Win[1] = h;
	glViewport(0, 0, (GLsizei)Win[0], (GLsizei)Win[1]);

    // Setup projection matrix for new window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(CAMERA_FOVY, (GLdouble)Win[0]/(GLdouble)Win[1], NEAR_CLIP, FAR_CLIP);
}



// display callback
//
// README: This gets called by the event handler
// to draw the scene, so this is where you need
// to build your scene -- make your changes and
// additions here. All rendering happens in this
// function. For Assignment 2, updates to the
// joint DOFs (joint_ui_data) happen in the
// animate() function.
void display(void)
{
    // Clear the screen with the background colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

    // Setup the model-view transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	// Specify camera transformation
	glTranslatef(camXPos, camYPos, camZPos);


	// Get the time for the current animation step, if necessary
	if( animate_mode )
	{
		float curTime = animationTimer->elapsed();

		if( curTime >= keyframes[maxValidKeyframe].getTime() )
		{
			// Restart the animation
			animationTimer->reset();
			curTime = animationTimer->elapsed();
		}

		///////////////////////////////////////////////////////////
		// README:
		//   This statement loads the interpolated joint DOF vector
		//   into the global 'joint_ui_data' variable. Use the
		//   'joint_ui_data' variable below in your model code to
		//   drive the model for animation.
		///////////////////////////////////////////////////////////
		// Get the interpolated joint DOFs
		joint_ui_data->setDOFVector( getInterpolatedJointDOFS(curTime) );

		// Update user interface
		joint_ui_data->setTime(curTime);
		glui_keyframe->sync_live();
	}


    ///////////////////////////////////////////////////////////
    // TODO:
	//   Modify this function to draw the scene.
	//   This should include function calls that apply
	//   the appropriate transformation matrices and render
	//   the individual body parts.
	//   Use the 'joint_ui_data' data structure to obtain
	//   the joint DOFs to specify your transformations.
	//   Sample code is provided below and demonstrates how
	//   to access the joint DOF values. This sample code
	//   should be replaced with your own.
	//   Use the 'renderStyle' variable and the associated
	//   enumeration to determine how the geometry should be
	//   rendered.
    ///////////////////////////////////////////////////////////

	// SAMPLE CODE **********
	//
	glPushMatrix();

		// setup transformation for body part
		glTranslatef(joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X),
					 joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y),
					 joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Z));
		glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_X), 1,0,0);
		glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_Y), 0,1,0);
		glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_Z), 0,0,1);
		glRotatef(30.0, 0.0, 1.0, 0.0);
		glRotatef(30.0, 1.0, 0.0, 0.0);

		float light[] = { Win[0] * sinf(deg_rad(lightpos)), Win[0] * cosf(deg_rad(lightpos)),1, 0 };

		//googled steel material properties
		float specularMetallic[] = { 1, 1,  1};
		float diffuseMetallic[] = { 0.8, 0.8,  0.8, 1};
        float ambientMetallic[] = { 1, 1,  1, 1};
		float shininessMetallic = 1;

		float specularMatte[] = { 0.5, 0.5,0.5};
		float diffuseMatte[] = { 0.5, 0.5, 0.5, 1.0 };
        float ambientMatte[] = {0.5, 0.5, 0.5, 1.0};
		float shininessMatte = 5;


		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);

		// determine render style and set glPolygonMode appropriately
		switch (renderStyle) {
			case WIREFRAME:
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				break;
			case MATTE:
			case METALLIC:
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_LIGHTING);
				glEnable(GL_LIGHT0);
				glLightfv(GL_LIGHT0, GL_POSITION, light);
				if (renderStyle == METALLIC) {
					glMaterialfv(GL_FRONT, GL_SPECULAR, specularMetallic);
					glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMetallic);
				    glMaterialfv(GL_FRONT, GL_AMBIENT, ambientMetallic);
					glMaterialf(GL_FRONT, GL_SHININESS, shininessMetallic);
				} else if (renderStyle == MATTE){
					glMaterialfv(GL_FRONT, GL_SPECULAR, specularMatte);
					glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMatte);
				    glMaterialfv(GL_FRONT, GL_AMBIENT, ambientMatte);
					glMaterialf(GL_FRONT, GL_SHININESS, shininessMatte);
				}
			case OUTLINED:
				glEnable(GL_DEPTH_TEST);
				glClear(GL_DEPTH_BUFFER_BIT);
			case SOLID:
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				break;
		}

		// draw body part
		glColor3f(defaultColor, defaultColor, defaultColor);
		drawBody();

		//draw head
		glPushMatrix();
			glTranslatef(0, HEADHEIGHT+ BODYHEIGHT, 0);
			glRotatef(joint_ui_data->getDOF(Keyframe::HEAD),0,1,0);
			drawHead();

			//drawEye
			glPushMatrix();
				glTranslatef(-HEADBOTLEN/2, HEADHEIGHT/4, -HEADTOPLEN);
				drawEye();
			glPopMatrix();

			//draw Other Eye
			glPushMatrix();
				glTranslatef(-HEADBOTLEN/2, HEADHEIGHT/4, HEADTOPLEN);
				drawEye();
			glPopMatrix();

			//draw Beak TOP
			glPushMatrix();
				glTranslatef(-BEAKLEN - HEADTOPLEN, BEAKHEIGHT,0);
				drawBeak();
			glPopMatrix();

			//draw Beak BOT
			glPushMatrix();
				glTranslatef(-BEAKLEN - HEADTOPLEN, -BEAKHEIGHT,0);
				glTranslatef(0,-joint_ui_data->getDOF(Keyframe::BEAK),0);
				drawBeak();
			glPopMatrix();

		glPopMatrix();

		//draw left arm
		glPushMatrix();
			glTranslatef(0, 0 ,2*ARMTHICK );
			glRotatef(-9, 1,0,0);
			glTranslatef(0, 0, BODYTOPLEN + ARMTHICK);

			glTranslatef(0,ARMLEN,0);
			glRotatef(joint_ui_data->getDOF(Keyframe::L_SHOULDER_PITCH),0,0,1);
			glRotatef(joint_ui_data->getDOF(Keyframe::L_SHOULDER_YAW),0,1,0);
			glRotatef(joint_ui_data->getDOF(Keyframe::L_SHOULDER_ROLL),1,0,0);
			glTranslatef(0, -ARMLEN,0);

			drawArm();
			glPushMatrix();
				glTranslatef(0,-ARMLEN - HANDLEN,0);
				glRotatef(joint_ui_data->getDOF(Keyframe::L_ELBOW),0,0,-1);
				drawHand();
			glPopMatrix();
		glPopMatrix();

		
		//draw right arm
		glPushMatrix();
			glTranslatef(0,0, -2* ARMTHICK);
			glRotatef(9, 1, 0 ,0);
			glTranslatef(0, 0, -BODYTOPLEN - ARMTHICK);

			glTranslatef(0,ARMLEN,0);
			glRotatef(joint_ui_data->getDOF(Keyframe::R_SHOULDER_PITCH),0,0,1);
			glRotatef(joint_ui_data->getDOF(Keyframe::R_SHOULDER_YAW),0,1,0);
			glRotatef(joint_ui_data->getDOF(Keyframe::R_SHOULDER_ROLL),1,0,0);
			glTranslatef(0, -ARMLEN,0);

			drawArm();
			glPushMatrix();
				glTranslatef(0,-ARMLEN - HANDLEN,0);
				glRotatef(joint_ui_data->getDOF(Keyframe::R_ELBOW),0,0,-1);
				drawHand();
			glPopMatrix();
		glPopMatrix();
		

		//draw left leg
		glPushMatrix();
			glTranslatef(0, -BODYHEIGHT - LEGLEN, 0.3);
			glRotatef(joint_ui_data->getDOF(Keyframe::L_HIP_PITCH),0,0,1);
			glRotatef(joint_ui_data->getDOF(Keyframe::L_HIP_YAW),0,1,0);
			glRotatef(joint_ui_data->getDOF(Keyframe::L_HIP_ROLL),1,0,0);
			drawLegs();
			glPushMatrix();
				glTranslatef(-FOOTLEN, -LEGLEN, 0);
				glRotatef(joint_ui_data->getDOF(Keyframe::L_KNEE),0,0,1);
				drawFoot();
			glPopMatrix();
		glPopMatrix();

		//draw right leg
		glPushMatrix();
			glTranslatef(0, -BODYHEIGHT - LEGLEN, -0.3);
			glRotatef(joint_ui_data->getDOF(Keyframe::R_HIP_PITCH),0,0,1);
			glRotatef(joint_ui_data->getDOF(Keyframe::R_HIP_YAW),0,1,0);
			glRotatef(joint_ui_data->getDOF(Keyframe::R_HIP_ROLL),1,0,0);
			drawLegs();
			glPushMatrix();
				glTranslatef(-FOOTLEN, -LEGLEN, 0);
				glRotatef(joint_ui_data->getDOF(Keyframe::R_KNEE),0,0,1);
				drawFoot();
			glPopMatrix();
		glPopMatrix();

	glPopMatrix();
	//
	// SAMPLE CODE **********

    // Execute any GL functions that are in the queue just to be safe
    glFlush();

	// Dump frame to file, if requested
	if( frameToFile )
	{
		sprintf(filenameF, "frame%03d.ppm", frameNumber);
		writeFrame(filenameF, false, false);
	}

    // Now, show the frame buffer that we just drew into.
    // (this prevents flickering).
    glutSwapBuffers();
}


// Handles mouse button pressed / released events
void mouse(int button, int state, int x, int y)
{
	// If the RMB is pressed and dragged then zoom in / out
	if( button == GLUT_RIGHT_BUTTON )
	{
		if( state == GLUT_DOWN )
		{
			lastX = x;
			lastY = y;
			updateCamZPos = true;
		}
		else
		{
			updateCamZPos = false;
		}
	}
}


// Handles mouse motion events while a button is pressed
void motion(int x, int y)
{
	// If the RMB is pressed and dragged then zoom in / out
	if( updateCamZPos )
	{
		// Update camera z position
		camZPos += (x - lastX) * ZOOM_SCALE;
		lastX = x;

		// Redraw the scene from updated camera position
		glutSetWindow(windowID);
		glutPostRedisplay();
	}
}


// Draw a unit cube, centered at the current location
// README: Helper code for drawing a cube
void drawBody(int lineDrawn)
{

	glBegin(GL_QUADS);

		// draw front face
		glNormal3f(0,0,1);
		glVertex3f(-BODYBOTLEN, -BODYHEIGHT, BODYBOTLEN);
		glVertex3f( BODYBOTLEN, -BODYHEIGHT, BODYBOTLEN);
		glVertex3f( BODYTOPLEN,  BODYHEIGHT, BODYTOPLEN);
		glVertex3f(-BODYTOPLEN,  BODYHEIGHT, BODYTOPLEN);

		// draw back face
		glNormal3f(0,0,-1);
		glVertex3f(-BODYBOTLEN, -BODYHEIGHT, -BODYBOTLEN);
		glVertex3f( BODYBOTLEN, -BODYHEIGHT, -BODYBOTLEN);
		glVertex3f( BODYTOPLEN,  BODYHEIGHT, -BODYTOPLEN);
		glVertex3f(-BODYTOPLEN,  BODYHEIGHT, -BODYTOPLEN);

		// draw left face
		glNormal3f(-1,0,0);
		glVertex3f(-BODYBOTLEN, -BODYHEIGHT, -BODYBOTLEN);
		glVertex3f(-BODYBOTLEN, -BODYHEIGHT,  BODYBOTLEN);
		glVertex3f(-BODYTOPLEN,  BODYHEIGHT,  BODYTOPLEN);
		glVertex3f(-BODYTOPLEN,  BODYHEIGHT, -BODYTOPLEN);

		// draw right face
		glNormal3f(1,0,0);
		glVertex3f( BODYBOTLEN, -BODYHEIGHT,  BODYBOTLEN);
		glVertex3f( BODYBOTLEN, -BODYHEIGHT, -BODYBOTLEN);
		glVertex3f( BODYTOPLEN,  BODYHEIGHT, -BODYTOPLEN);
		glVertex3f( BODYTOPLEN,  BODYHEIGHT,  BODYTOPLEN);

		// draw top
		glNormal3f(0,1,0);
		glVertex3f(-BODYTOPLEN,  BODYHEIGHT,  BODYTOPLEN);
		glVertex3f( BODYTOPLEN,  BODYHEIGHT,  BODYTOPLEN);
		glVertex3f( BODYTOPLEN,  BODYHEIGHT, -BODYTOPLEN);
		glVertex3f(-BODYTOPLEN,  BODYHEIGHT, -BODYTOPLEN);

		// draw bottom
		glNormal3f(0,-1,0);
		glVertex3f(-BODYBOTLEN, -BODYHEIGHT, -BODYBOTLEN);
		glVertex3f( BODYBOTLEN, -BODYHEIGHT, -BODYBOTLEN);
		glVertex3f( BODYBOTLEN, -BODYHEIGHT,  BODYBOTLEN);
		glVertex3f(-BODYBOTLEN, -BODYHEIGHT,  BODYBOTLEN);
	glEnd();

	drawFxn = &drawBody;
	if (renderStyle == OUTLINED && !lineDrawn) drawOUTLINED();


}


void drawHead(int lineDrawn){

	glBegin(GL_QUADS);

		// draw front face
		glNormal3f(0,0,1);
		glVertex3f(-HEADBOTLEN, -HEADHEIGHT, HEADBOTLEN);
		glVertex3f( HEADBOTLEN, -HEADHEIGHT, HEADBOTLEN);
		glVertex3f( HEADTOPLEN,  HEADHEIGHT, HEADTOPLEN);
		glVertex3f(-HEADTOPLEN,  HEADHEIGHT, HEADTOPLEN);

		// draw back face
		glNormal3f(0,0,-1);
		glVertex3f(-HEADBOTLEN, -HEADHEIGHT, -HEADBOTLEN);
		glVertex3f( HEADBOTLEN, -HEADHEIGHT, -HEADBOTLEN);
		glVertex3f( HEADTOPLEN,  HEADHEIGHT, -HEADTOPLEN);
		glVertex3f(-HEADTOPLEN,  HEADHEIGHT, -HEADTOPLEN);

		// draw left face
		glNormal3f(-1,0,0);
		glVertex3f(-HEADBOTLEN, -HEADHEIGHT, -HEADBOTLEN);
		glVertex3f(-HEADBOTLEN, -HEADHEIGHT,  HEADBOTLEN);
		glVertex3f(-HEADTOPLEN,  HEADHEIGHT,  HEADTOPLEN);
		glVertex3f(-HEADTOPLEN,  HEADHEIGHT, -HEADTOPLEN);

		// draw right face
		glNormal3f(1,0,0);
		glVertex3f( HEADBOTLEN, -HEADHEIGHT,  HEADBOTLEN);
		glVertex3f( HEADBOTLEN, -HEADHEIGHT, -HEADBOTLEN);
		glVertex3f( HEADTOPLEN,  HEADHEIGHT, -HEADTOPLEN);
		glVertex3f( HEADTOPLEN,  HEADHEIGHT,  HEADTOPLEN);

		// draw top
		glNormal3f(0,1,0);
		glVertex3f(-HEADTOPLEN,  HEADHEIGHT,  HEADTOPLEN);
		glVertex3f( HEADTOPLEN,  HEADHEIGHT,  HEADTOPLEN);
		glVertex3f( HEADTOPLEN,  HEADHEIGHT, -HEADTOPLEN);
		glVertex3f(-HEADTOPLEN,  HEADHEIGHT, -HEADTOPLEN);

		// draw bottom
		glNormal3f(0,-1,0);
		glVertex3f(-HEADBOTLEN, -HEADHEIGHT, -HEADBOTLEN);
		glVertex3f( HEADBOTLEN, -HEADHEIGHT, -HEADBOTLEN);
		glVertex3f( HEADBOTLEN, -HEADHEIGHT,  HEADBOTLEN);
		glVertex3f(-HEADBOTLEN, -HEADHEIGHT,  HEADBOTLEN);
	glEnd();

	drawFxn = &drawHead;
	if (renderStyle == OUTLINED && !lineDrawn) drawOUTLINED();


}

void drawEye(){
	float deg = 360.0 / 50;
	glBegin(GL_LINE_LOOP);
	glColor3f(0,0,0);
    for (unsigned int i = 0; i < 50; i++) {
        glVertex2d(EYERAD * cos(deg * i), EYERAD * sin(deg * i));
    }
    glEnd();
    glBegin(GL_POLYGON);
    for (unsigned int i = 0; i < 50; i++) {
        glVertex2d(CORNRAD * cos(deg * i), CORNRAD * sin(deg * i));
    }
    glColor3f(defaultColor,defaultColor,defaultColor);
    glEnd();
}

void drawBeak(int lineDrawn){

		glBegin(GL_QUADS);

		// draw front face
		glNormal3f(0,0,1);
		glVertex3f(-BEAKLEN, -BEAKHEIGHT, BEAKDEPTH);
		glVertex3f( BEAKLEN, -BEAKHEIGHT, BEAKDEPTH);
		glVertex3f( BEAKLEN,  BEAKHEIGHT, BEAKDEPTH);
		glVertex3f(-BEAKLEN,  BEAKHEIGHT, BEAKDEPTH);

		// draw back face
		glNormal3f(0,0,-1);
		glVertex3f(-BEAKLEN, -BEAKHEIGHT, -BEAKDEPTH);
		glVertex3f( BEAKLEN, -BEAKHEIGHT, -BEAKDEPTH);
		glVertex3f( BEAKLEN,  BEAKHEIGHT, -BEAKDEPTH);
		glVertex3f(-BEAKLEN,  BEAKHEIGHT, -BEAKDEPTH);

		// draw left face
		glNormal3f(-1,0,0);
		glVertex3f(-BEAKLEN, -BEAKHEIGHT, -BEAKDEPTH);
		glVertex3f(-BEAKLEN, -BEAKHEIGHT,  BEAKDEPTH);
		glVertex3f(-BEAKLEN,  BEAKHEIGHT,  BEAKDEPTH);
		glVertex3f(-BEAKLEN,  BEAKHEIGHT, -BEAKDEPTH);

		// draw right face
		glNormal3f(1,0,0);
		glVertex3f( BEAKLEN, -BEAKHEIGHT,  BEAKDEPTH);
		glVertex3f( BEAKLEN, -BEAKHEIGHT, -BEAKDEPTH);
		glVertex3f( BEAKLEN,  BEAKHEIGHT, -BEAKDEPTH);
		glVertex3f( BEAKLEN,  BEAKHEIGHT,  BEAKDEPTH);

		// draw top
		glNormal3f(0,1,0);
		glVertex3f(-BEAKLEN,  BEAKHEIGHT,  BEAKDEPTH);
		glVertex3f( BEAKLEN,  BEAKHEIGHT,  BEAKDEPTH);
		glVertex3f( BEAKLEN,  BEAKHEIGHT, -BEAKDEPTH);
		glVertex3f(-BEAKLEN,  BEAKHEIGHT, -BEAKDEPTH);

		// draw bot
		glNormal3f(0,-1,0);
		glVertex3f(-BEAKLEN, -BEAKHEIGHT, -BEAKDEPTH);
		glVertex3f( BEAKLEN, -BEAKHEIGHT, -BEAKDEPTH);
		glVertex3f( BEAKLEN, -BEAKHEIGHT,  BEAKDEPTH);
		glVertex3f(-BEAKLEN, -BEAKHEIGHT,  BEAKDEPTH);
	glEnd();

	drawFxn = &drawBeak;
	if (renderStyle == OUTLINED && !lineDrawn) drawOUTLINED();

}

void drawArm(int lineDrawn) {
	glBegin(GL_QUADS);

		// draw front face
		glNormal3f(0,0,1);
		glVertex3f(-BOTARMWID, -ARMLEN, ARMTHICK);
		glVertex3f( BOTARMWID, -ARMLEN, ARMTHICK);
		glVertex3f( TOPARMWID,  ARMLEN, ARMTHICK);
		glVertex3f(-TOPARMWID,  ARMLEN, ARMTHICK);

		// draw back face
		glNormal3f(0,0,-1);
		glVertex3f(-BOTARMWID, -ARMLEN, -ARMTHICK);
		glVertex3f( BOTARMWID, -ARMLEN, -ARMTHICK);
		glVertex3f( TOPARMWID,  ARMLEN, -ARMTHICK);
		glVertex3f(-TOPARMWID,  ARMLEN, -ARMTHICK);

		// draw left face
		glNormal3f(-1,0,0);
		glVertex3f(-BOTARMWID, -ARMLEN, -ARMTHICK);
		glVertex3f(-BOTARMWID, -ARMLEN,  ARMTHICK);
		glVertex3f(-TOPARMWID,  ARMLEN,  ARMTHICK);
		glVertex3f(-TOPARMWID,  ARMLEN, -ARMTHICK);

		// draw right face
		glNormal3f(1,0,0);
		glVertex3f( BOTARMWID, -ARMLEN,  ARMTHICK);
		glVertex3f( BOTARMWID, -ARMLEN, -ARMTHICK);
		glVertex3f( TOPARMWID,  ARMLEN, -ARMTHICK);
		glVertex3f( TOPARMWID,  ARMLEN,  ARMTHICK);

		// draw top
		glNormal3f(0,1,0);
		glVertex3f(-TOPARMWID,  ARMLEN,  ARMTHICK);
		glVertex3f( TOPARMWID,  ARMLEN,  ARMTHICK);
		glVertex3f( TOPARMWID,  ARMLEN, -ARMTHICK);
		glVertex3f(-TOPARMWID,  ARMLEN, -ARMTHICK);

		// draw bottom
		glNormal3f(0,-1,0);
		glVertex3f(-BOTARMWID, -ARMLEN, -ARMTHICK);
		glVertex3f( BOTARMWID, -ARMLEN, -ARMTHICK);
		glVertex3f( BOTARMWID, -ARMLEN,  ARMTHICK);
		glVertex3f(-BOTARMWID, -ARMLEN,  ARMTHICK);

	glEnd();

	drawFxn = &drawArm;
	if (renderStyle == OUTLINED && !lineDrawn) drawOUTLINED();
}

void drawFoot(int lineDrawn) {
	glBegin(GL_QUADS);

		// draw front face
		glNormal3f(0,0,1);
		glVertex3f(-FOOTLEN, FOOTHEIGHT, FOOTDEPTH);
		glVertex3f( -FOOTLEN, -FOOTHEIGHT, FOOTDEPTH);
		glVertex3f( FOOTLEN, -FOOTHEIGHT, 0);
		glVertex3f( FOOTLEN,  FOOTHEIGHT, 0);

		// draw back face
		glNormal3f(0,0,-1);
		glVertex3f(-FOOTLEN, FOOTHEIGHT, -FOOTDEPTH);
		glVertex3f( -FOOTLEN, -FOOTHEIGHT, -FOOTDEPTH);
		glVertex3f( FOOTLEN, -FOOTHEIGHT, 0);
		glVertex3f( FOOTLEN,  FOOTHEIGHT, 0);

		// draw left face
		glNormal3f(-1,0,0);
		glVertex3f(-FOOTLEN, FOOTHEIGHT, -FOOTDEPTH);
		glVertex3f(-FOOTLEN, -FOOTHEIGHT, -FOOTDEPTH);
		glVertex3f(-FOOTLEN, -FOOTHEIGHT, FOOTDEPTH);
		glVertex3f(-FOOTLEN, FOOTHEIGHT, FOOTDEPTH);

		// draw top face
		glNormal3f(0,1,0);
		glVertex3f(-FOOTLEN, FOOTHEIGHT, -FOOTDEPTH);
		glVertex3f(-FOOTLEN, FOOTHEIGHT, FOOTDEPTH);
		glVertex3f(FOOTLEN, FOOTHEIGHT, 0);
		glVertex3f(FOOTLEN, FOOTHEIGHT, 0);

		// draw bot face
		glNormal3f(0,-1,0);
		glVertex3f(-FOOTLEN, -FOOTHEIGHT, -FOOTDEPTH);
		glVertex3f(-FOOTLEN, -FOOTHEIGHT, FOOTDEPTH);
		glVertex3f(FOOTLEN, -FOOTHEIGHT, 0);
		glVertex3f(FOOTLEN, -FOOTHEIGHT, 0);


	glEnd();

	drawFxn = &drawFoot;
	if (renderStyle == OUTLINED && !lineDrawn) drawOUTLINED();
}

void drawLegs(int lineDrawn) {
	glBegin(GL_QUADS);

		// draw front face
		glNormal3f(0,0,1);
		glVertex3f(-LEGBOTWID, -LEGLEN, LEGDEPTH);
		glVertex3f( LEGBOTWID, -LEGLEN, LEGDEPTH);
		glVertex3f( LEGTOPWID,  LEGLEN, LEGDEPTH);
		glVertex3f(-LEGTOPWID,  LEGLEN, LEGDEPTH);

		// draw back face
		glNormal3f(0,0,-1);
		glVertex3f(-LEGBOTWID, -LEGLEN, -LEGDEPTH);
		glVertex3f( LEGBOTWID, -LEGLEN, -LEGDEPTH);
		glVertex3f( LEGTOPWID,  LEGLEN, -LEGDEPTH);
		glVertex3f(-LEGTOPWID,  LEGLEN, -LEGDEPTH);

		// draw left face
		glNormal3f(-1,0,0);
		glVertex3f(-LEGBOTWID, -LEGLEN, -LEGDEPTH);
		glVertex3f(-LEGBOTWID, -LEGLEN,  LEGDEPTH);
		glVertex3f(-LEGTOPWID,  LEGLEN,  LEGDEPTH);
		glVertex3f(-LEGTOPWID,  LEGLEN, -LEGDEPTH);

		// draw right face
		glNormal3f( 1,0,0);
		glVertex3f( LEGBOTWID, -LEGLEN,  LEGDEPTH);
		glVertex3f( LEGBOTWID, -LEGLEN, -LEGDEPTH);
		glVertex3f( LEGTOPWID,  LEGLEN, -LEGDEPTH);
		glVertex3f( LEGTOPWID,  LEGLEN,  LEGDEPTH);

		// draw top
		glNormal3f(0,1,0);
		glVertex3f(-LEGTOPWID,  LEGLEN,  LEGDEPTH);
		glVertex3f( LEGTOPWID,  LEGLEN,  LEGDEPTH);
		glVertex3f( LEGTOPWID,  LEGLEN, -LEGDEPTH);
		glVertex3f(-LEGTOPWID,  LEGLEN, -LEGDEPTH);

		// draw bottom
		glNormal3f(0,-1,0);
		glVertex3f(-LEGBOTWID, -LEGLEN, -LEGDEPTH);
		glVertex3f( LEGBOTWID, -LEGLEN, -LEGDEPTH);
		glVertex3f( LEGBOTWID, -LEGLEN,  LEGDEPTH);
		glVertex3f(-LEGBOTWID, -LEGLEN,  LEGDEPTH);

	glEnd();

	drawFxn = &drawArm;
	if (renderStyle == OUTLINED && !lineDrawn) drawOUTLINED();
}

void drawHand(int lineDrawn) {
	glBegin(GL_QUADS);

		//Front Face
		glNormal3f(0,0,1);
		glVertex3f(-HANDLEN, HANDLEN, HANDWID);
		glVertex3f(-HANDLEN, -HANDLEN, HANDWID);
		glVertex3f(HANDLEN, -HANDLEN, HANDWID);
		glVertex3f(HANDLEN, HANDLEN, HANDWID);

		//Back Face
		glNormal3f(0,0,-1);
		glVertex3f(-HANDLEN, HANDLEN, -HANDWID);
		glVertex3f(-HANDLEN, -HANDLEN, -HANDWID);
		glVertex3f(HANDLEN, -HANDLEN, -HANDWID);
		glVertex3f(HANDLEN, HANDLEN, -HANDWID);

		//Left Face
		glNormal3f(-1,0,0);
		glVertex3f(-HANDLEN, HANDLEN, -HANDWID);
		glVertex3f(-HANDLEN, -HANDLEN, -HANDWID);
		glVertex3f(-HANDLEN, -HANDLEN, HANDWID);
		glVertex3f(-HANDLEN, HANDLEN, HANDWID);

		//Right Face
		glNormal3f(1,0,0);
		glVertex3f(HANDLEN, HANDLEN, -HANDWID);
		glVertex3f(HANDLEN, -HANDLEN, -HANDWID);
		glVertex3f(HANDLEN, -HANDLEN, HANDWID);
		glVertex3f(HANDLEN, HANDLEN, HANDWID);

		//Top Face
		glNormal3f(0,1,0);
		glVertex3f(-HANDLEN, HANDLEN, -HANDWID);
		glVertex3f(-HANDLEN, HANDLEN, HANDWID);
		glVertex3f(HANDLEN, HANDLEN, HANDWID);
		glVertex3f(HANDLEN, HANDLEN, -HANDWID);

		//Bot Face
		glNormal3f(0,-1,0);
		glVertex3f(-HANDLEN, -HANDLEN, -HANDWID);
		glVertex3f(-HANDLEN, -HANDLEN, HANDWID);
		glVertex3f(HANDLEN, -HANDLEN, HANDWID);
		glVertex3f(HANDLEN, -HANDLEN, -HANDWID);
	glEnd();

	drawFxn = &drawHand;
	if (renderStyle == OUTLINED && !lineDrawn) drawOUTLINED();
}

void drawOUTLINED() {
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-0.1, -0.1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(0,0,0);
	(*drawFxn)(1);
	glColor3f(defaultColor, defaultColor, defaultColor);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
};


///////////////////////////////////////////////////////////
//
// BELOW ARE FUNCTIONS FOR GENERATING IMAGE FILES (PPM/PGM)
//
///////////////////////////////////////////////////////////

void writePGM(char* filename, GLubyte* buffer, int width, int height, bool raw=true)
{
	FILE* fp = fopen(filename,"wt");

	if( fp == NULL )
	{
		printf("WARNING: Can't open output file %s\n",filename);
		return;
	}

	if( raw )
	{
		fprintf(fp,"P5\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			fwrite(&buffer[y*width],sizeof(GLubyte),width,fp);
			/*
			for(int x=0;x<width;x++)
			{
				fprintf(fp,"%c",int(buffer[x+y*width];
			}
			*/
		}
	}
	else
	{
		fprintf(fp,"P2\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			for(int x=0;x<width;x++)
			{
				fprintf(fp,"%d ",int(buffer[x+y*width]));
			}
			fprintf(fp,"\n");
		}
	}

	fclose(fp);
}

#define RED_OFFSET   0
#define GREEN_OFFSET 1
#define BLUE_OFFSET  2

void writePPM(char* filename, GLubyte* buffer, int width, int height, bool raw=true)
{
	FILE* fp = fopen(filename,"wt");

	if( fp == NULL )
	{
		printf("WARNING: Can't open output file %s\n",filename);
		return;
	}

	if( raw )
	{
		fprintf(fp,"P6\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			for(int x=0;x<width;x++)
			{
				GLubyte* pix = &buffer[4*(x+y*width)];

				fprintf(fp,"%c%c%c",int(pix[RED_OFFSET]),
									int(pix[GREEN_OFFSET]),
									int(pix[BLUE_OFFSET]));
			}
		}
	}
	else
	{
		fprintf(fp,"P3\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			for(int x=0;x<width;x++)
			{
				GLubyte* pix = &buffer[4*(x+y*width)];

				fprintf(fp,"%d %d %d ",int(pix[RED_OFFSET]),
									   int(pix[GREEN_OFFSET]),
									   int(pix[BLUE_OFFSET]));
			}
			fprintf(fp,"\n");
		}
	}

	fclose(fp);
}

void writeFrame(char* filename, bool pgm, bool frontBuffer)
{
	static GLubyte* frameData = NULL;
	static int currentSize = -1;

	int size = (pgm ? 1 : 4);

	if( frameData == NULL || currentSize != size*Win[0]*Win[1] )
	{
		if (frameData != NULL)
			delete [] frameData;

		currentSize = size*Win[0]*Win[1];

		frameData = new GLubyte[currentSize];
	}

	glReadBuffer(frontBuffer ? GL_FRONT : GL_BACK);

	if( pgm )
	{
		glReadPixels(0, 0, Win[0], Win[1],
					 GL_LUMINANCE, GL_UNSIGNED_BYTE, frameData);
		writePGM(filename, frameData, Win[0], Win[1]);
	}
	else
	{
		glReadPixels(0, 0, Win[0], Win[1],
					 GL_RGBA, GL_UNSIGNED_BYTE, frameData);
		writePPM(filename, frameData, Win[0], Win[1]);
	}
}
