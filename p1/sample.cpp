#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <vector>


#ifndef F_PI
#define F_PI		((float)(M_PI))
#define F_2_PI		((float)(2.f*F_PI))
#define F_PI_2		((float)(F_PI/2.f))
#endif


#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif


#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "glut.h"


//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joe Graphics

// title of these windows:

const char *WINDOWTITLE = "CS550 P1 -- <Haochuan Zhang>";
const char *GLUITITLE   = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE  = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 1000;

// size of the 3d box to be drawn:

const float BOXSIZE = 2.f;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):

const int LEFT   = 4;
const int MIDDLE = 2;
const int RIGHT  = 1;

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// line width for the axes:

const GLfloat AXES_WIDTH   = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA
};

char * ColorNames[ ] =
{
	(char *)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
};

// fog parameters:

// fog parameters (tuned for a farther camera)
const GLfloat FOGCOLOR[4] = { 0.0f, 0.0f, 0.0f, 1.f }; // keep black if you like space black
const GLenum  FOGMODE     = GL_LINEAR;
const GLfloat FOGDENSITY  = 0.30f;
// old: 1.5f, 4.f  -> too near; everything at distance >4 becomes pure fog (black)
const GLfloat FOGSTART    = 15.0f;   // start fogging much farther away
const GLfloat FOGEND      = 40.0f;   // fully fogged beyond this

// for lighting:

const float	WHITE[ ] = { 1.,1.,1.,1. };

// for animation:

const int MS_PER_CYCLE = 10000;		// 10000 milliseconds = 10 seconds


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
GLuint	BoxList;				// object display list
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
int		NowColor;				// index into Colors[ ]
int		NowProjection;		// ORTHO or PERSP
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
float	Time;					// used for animation, this has a value between 0. and 1.
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees

// >>> BEGIN MY ADD
GLuint MyObjList = 0;
// --- solar system lists ---
GLuint SunList = 0;
GLuint PlanetList[8] = {0};  // Mercury..Neptune
GLuint SunEdge = 0;                 
// --- outlines (wireframe) lists for planets ---
GLuint PlanetEdgeList[8] = {0};

// --- cached random positions on circular orbits (x,y,z) ---
float PlanetPos[8][3];  // y=0, we only use x,z for the planet's position

// small utility: generate a random angle between [0, 2π)
inline float RandAngle()
{
    return ( (float)rand() / (float)RAND_MAX ) * F_2_PI;
}

// --- planet colors ---
float PlanetColor[8][3] = {
    {0.80f, 0.62f, 0.40f}, // Mercury  (warm gray-brown)
    {1.00f, 0.80f, 0.10f}, // Venus    (golden yellow)
    {0.10f, 0.45f, 1.00f}, // Earth    (bright blue)
    {1.00f, 0.30f, 0.10f}, // Mars     (deep orange-red)
    {0.95f, 0.75f, 0.35f}, // Jupiter  (tan/orange)
    {1.00f, 0.90f, 0.55f}, // Saturn   (pale yellow)
    {0.15f, 0.85f, 0.85f}, // Uranus   (teal-cyan)
    {0.15f, 0.25f, 1.00f}  // Neptune  (saturated blue)
};
const float SunColor[3] = {1.00f, 0.80f, 0.20f}; // warm yellow

// --- vertex counter (for self-check) ---
int gVertCount = 0;
inline void V3(float x, float y, float z) {
    glVertex3f(x, y, z);
    gVertCount++;
}
// <<< END MY ADD

// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void			Axes( float );
void			HsvRgb( float[3], float [3] );
void			Cross(float[3], float[3], float[3]);
float			Dot(float [3], float [3]);
float			Unit(float [3], float [3]);
float			Unit(float [3]);


// utility to create an array from 3 separate values:

float *
Array3( float a, float b, float c )
{
	static float array[4];

	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

// utility to create an array from a multiplier and an array:

float *
MulArray3( float factor, float array0[ ] )
{
	static float array[4];

	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}


float *
MulArray3(float factor, float a, float b, float c )
{
	static float array[4];

	float* abc = Array3(a, b, c);
	array[0] = factor * abc[0];
	array[1] = factor * abc[1];
	array[2] = factor * abc[2];
	array[3] = 1.;
	return array;
}


float
Ranf( float low, float high )
{
        float r = (float) rand();               // 0 - RAND_MAX
        float t = r  /  (float) RAND_MAX;       // 0. - 1.

        return   low  +  t * ( high - low );
}

// call this if you want to force your program to use
// a different random number sequence every time you run it:
void
TimeOfDaySeed( )
{
	struct tm y2k;
	y2k.tm_hour = 0;    y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 2000; y2k.tm_mon = 0; y2k.tm_mday = 1;

	time_t  now;
	time( &now );
	double seconds = difftime( now, mktime(&y2k) );
	unsigned int seed = (unsigned int)( 1000.*seconds );    // milliseconds
	srand( seed );
}

// these are here for when you need them -- just uncomment the ones you need:

//#include "setmaterial.cpp"
//#include "setlight.cpp"
//#include "osusphere.cpp"
//#include "osucube.cpp"
//#include "osucylindercone.cpp"
//#include "osutorus.cpp"
//#include "bmptotexture.cpp"
//#include "loadobjmtlfiles.cpp"
//#include "keytime.cpp"
//#include "glslprogram.cpp"
//#include "vertexbufferobject.cpp"


// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since glutInit might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// setup all the graphics stuff:

	InitGraphics( );

	// create the display lists that **will not change**:

	InitLists( );

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );

	// setup all the user interface stuff:

	InitMenus( );

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables for Display( ) to find:

	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;							// makes the value of ms between 0 and MS_PER_CYCLE-1
	Time = (float)ms / (float)MS_PER_CYCLE;		// makes the value of Time between 0. and slightly less than 1.

	// for example, if you wanted to spin an object in Display( ), you might call: glRotatef( 360.f*Time,   0., 1., 0. );

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting Display.\n");

	// set which window we want to do the graphics into:
	glutSetWindow( MainWindow );

	// erase the background:
	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
#ifdef DEMO_DEPTH_BUFFER
	if( DepthBufferOn == 0 )
		glDisable( GL_DEPTH_TEST );
#endif


	// specify shading to be flat:

	// glShadeModel( GL_FLAT );
	glShadeModel( GL_SMOOTH );

	// set the viewport to be a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( NowProjection == ORTHO )
		glOrtho( -2.f, 2.f,     -2.f, 2.f,     0.1f, 1000.f );
	else
		gluPerspective( 70.f, 1.f,	0.1f, 1000.f );

	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	// set the eye position, look-at position, and up-vector:

	// gluLookAt( 0.f, 0.f, 3.f,     0.f, 0.f, 0.f,     0.f, 1.f, 0.f );
	gluLookAt( 0.f, 2.f, 20.f,    0.f, 0.f, 0.f,     0.f, 1.f, 0.f );		// make view further back

	// rotate the scene:

	glRotatef( (GLfloat)Yrot, 0.f, 1.f, 0.f );
	glRotatef( (GLfloat)Xrot, 1.f, 0.f, 0.f );

	// uniformly scale the scene:

	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );

	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	// possibly draw the axes:

	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[NowColor][0] );
		glCallList( AxesList );
	}

	// since we are using glScalef( ), be sure the normals get unitized:

	glEnable( GL_NORMALIZE );


	// draw the box object by calling up its display list:

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	// Sun (color + alpha already set in its list)
	glCallList(SunList);

	// Planets (color + alpha already set in their lists)
	for (int i = 0; i < 8; ++i) {
		glPushMatrix();
			glTranslatef(PlanetPos[i][0], PlanetPos[i][1], PlanetPos[i][2]);
			glCallList(PlanetList[i]);
		glPopMatrix();
	}

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);





#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.f,   0.f, 1.f, 0.f );
			glCallList( BoxList );
		glPopMatrix( );
	}
#endif


	// draw some gratuitous text that just rotates on top of the scene:
	// i commented out the actual text-drawing calls -- put them back in if you have a use for them
	// a good use for thefirst one might be to have your name on the screen
	// a good use for the second one might be to have vertex numbers on the screen alongside each vertex

	glDisable( GL_DEPTH_TEST );
	glColor3f( 0.f, 1.f, 1.f );
	//DoRasterString( 0.f, 1.f, 0.f, (char *)"Text That Moves" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0.f, 100.f,     0.f, 100.f );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1.f, 1.f, 1.f );
	//DoRasterString( 5.f, 5.f, 0.f, (char *)"Text That Doesn't" );

	// swap the double-buffered framebuffers:

	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	NowColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	NowProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 66.66f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}



// initialize the glut and OpenGL libraries:
//	also setup callback functions

void
InitGraphics( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitGraphics.\n");

	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc( Animate );

	// init the glew package (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// all other setups go here, such as GLSLProgram and KeyTime setups:

}

// Global Helper Functions
GLuint MakeUvSphereList(float radius, int slices, int stacks, const float rgb[3]);
GLuint MakeUvSphereEdgeList(float radius, int slices, int stacks, const float rgb[3], float scale=1.0f);
GLuint MakePointSphereList(float R, int SLICES, int STACKS,
                           const float base[3], float alpha,
                           float lighten = 1.20f, float darken = 0.75f,
                           float jitter = 0.015f, float pointSize = 1.6f);

// ---- small helpers (define ONCE, before MakeUvSphereList / MakePointSphereList) ----
static inline float clamp01(float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); }
static inline float mixf(float a, float b, float t) { return a + (b - a) * t; }

// Builds a UV-sphere with per-vertex color gradient and fixed alpha.
// The gradient is a smooth blend between a darker "belt" color (near equator)
// and a lighter "cap" color (near poles). Alpha controls transparency.
GLuint MakeUvSphereList(float R, int SLICES, int STACKS, const float base[3], float alpha,
                        float lighten = 1.20f, float darken = 0.75f)
{
    // derive two endpoints for the gradient from the base color:
    float cap[3]  = { clamp01(base[0]*lighten), clamp01(base[1]*lighten), clamp01(base[2]*lighten) };   // lighter near poles
    float belt[3] = { clamp01(base[0]*darken ), clamp01(base[1]*darken ), clamp01(base[2]*darken ) };   // darker near equator

    GLuint lid = glGenLists(1);
    glNewList(lid, GL_COMPILE);
    {
        for (int i = 0; i < STACKS; ++i)
        {
            float v0  = (float)i / STACKS;
            float v1  = (float)(i + 1) / STACKS;
            float th0 = F_PI * v0;    // 0..PI
            float th1 = F_PI * v1;
            float y0n = cosf(th0), r0 = sinf(th0);  // y normalized in [-1,1]
            float y1n = cosf(th1), r1 = sinf(th1);

            glBegin(GL_TRIANGLES);
            for (int j = 0; j < SLICES; ++j)
            {
                float u0  = (float)j / SLICES;
                float u1  = (float)(j + 1) / SLICES;
                float ph0 = F_2_PI * u0;
                float ph1 = F_2_PI * u1;

                // four vertices on the two latitudes
                float x00 = R * r0 * cosf(ph0), z00 = R * r0 * sinf(ph0), y00 = R * y0n;
                float x10 = R * r0 * cosf(ph1), z10 = R * r0 * sinf(ph1), y10 = R * y0n;
                float x11 = R * r1 * cosf(ph1), z11 = R * r1 * sinf(ph1), y11 = R * y1n;
                float x01 = R * r1 * cosf(ph0), z01 = R * r1 * sinf(ph0), y01 = R * y1n;

                // gradient factor per vertex: map y in [-R,+R] -> t in [0,1]
                float t00 = 0.5f * (y00 / R + 1.f);
                float t10 = 0.5f * (y10 / R + 1.f);
                float t11 = 0.5f * (y11 / R + 1.f);
                float t01 = 0.5f * (y01 / R + 1.f);

                // triangle 1
                glColor4f(mixf(belt[0], cap[0], t00), mixf(belt[1], cap[1], t00), mixf(belt[2], cap[2], t00), alpha);
                V3(x00, y00, z00);
                glColor4f(mixf(belt[0], cap[0], t10), mixf(belt[1], cap[1], t10), mixf(belt[2], cap[2], t10), alpha);
                V3(x10, y10, z10);
                glColor4f(mixf(belt[0], cap[0], t11), mixf(belt[1], cap[1], t11), mixf(belt[2], cap[2], t11), alpha);
                V3(x11, y11, z11);

                // triangle 2
                glColor4f(mixf(belt[0], cap[0], t00), mixf(belt[1], cap[1], t00), mixf(belt[2], cap[2], t00), alpha);
                V3(x00, y00, z00);
                glColor4f(mixf(belt[0], cap[0], t11), mixf(belt[1], cap[1], t11), mixf(belt[2], cap[2], t11), alpha);
                V3(x11, y11, z11);
                glColor4f(mixf(belt[0], cap[0], t01), mixf(belt[1], cap[1], t01), mixf(belt[2], cap[2], t01), alpha);
                V3(x01, y01, z01);
            }
            glEnd();
        }
    }
    glEndList();
    return lid;
}


//  draw only the edges of the sphere: each triangle is drawn with GL_LINE_LOOP to draw the edges;
//  scale>1 can slightly enlarge the sphere to avoid Z-Fighting
GLuint MakeUvSphereEdgeList(float R, int SLICES, int STACKS, const float rgb[3], float scale)
{
    GLuint lid = glGenLists(1);
    glNewList(lid, GL_COMPILE);
    {
        glColor3fv(rgb);
        glLineWidth(1.0f);
        for (int i = 0; i < STACKS; ++i) {
            float v0 = (float)i / STACKS;
            float v1 = (float)(i + 1) / STACKS;
            float th0 = F_PI * v0;
            float th1 = F_PI * v1;
            float y0 = cosf(th0), r0 = sinf(th0);
            float y1 = cosf(th1), r1 = sinf(th1);

            for (int j = 0; j < SLICES; ++j) {
                float u0 = (float)j / SLICES;
                float u1 = (float)(j + 1) / SLICES;
                float ph0 = F_2_PI * u0;
                float ph1 = F_2_PI * u1;

                // four vertices
                float x00 = R * r0 * cosf(ph0), z00 = R * r0 * sinf(ph0), y00 = R * y0;
                float x10 = R * r0 * cosf(ph1), z10 = R * r0 * sinf(ph1), y10 = R * y0;
                float x11 = R * r1 * cosf(ph1), z11 = R * r1 * sinf(ph1), y11 = R * y1;
                float x01 = R * r1 * cosf(ph0), z01 = R * r1 * sinf(ph0), y01 = R * y1;

                // slightly enlarge the sphere to avoid Z-Fighting
                x00*=scale; y00*=scale; z00*=scale;
                x10*=scale; y10*=scale; z10*=scale;
                x11*=scale; y11*=scale; z11*=scale;
                x01*=scale; y01*=scale; z01*=scale;

                glBegin(GL_LINE_LOOP);  //  draw the edges of triangle 1
                    glVertex3f(x00,y00,z00);
                    glVertex3f(x10,y10,z10);
                    glVertex3f(x11,y11,z11);
                glEnd();
                glBegin(GL_LINE_LOOP);  //  draw the edges of triangle 2
                    glVertex3f(x00,y00,z00);
                    glVertex3f(x11,y11,z11);
                    glVertex3f(x01,y01,z01);
                glEnd();
            }
        }
    }
    glEndList();
    return lid;
}

// Builds a "dusty / point-cloud" sphere using GL_POINTS.
// Color per vertex uses a pole-cap (lighter) to equator-belt (darker) gradient.
// Small radial jitter adds a noisy star-dust look.

GLuint MakePointSphereList(float R, int SLICES, int STACKS,
                           const float base[3], float alpha,
                           float lighten, float darken,
                           float jitter, float pointSize)
{
    float cap[3]  = { clamp01(base[0]*lighten), clamp01(base[1]*lighten), clamp01(base[2]*lighten) };
    float belt[3] = { clamp01(base[0]*darken ), clamp01(base[1]*darken ), clamp01(base[2]*darken ) };

    GLuint lid = glGenLists(1);
    glNewList(lid, GL_COMPILE);
    {
        glPointSize(pointSize);
        glBegin(GL_POINTS);
        for (int i = 0; i <= STACKS; ++i)
        {
            float v  = (float)i / STACKS;        // 0..1
            float th = F_PI * v;                 // 0..PI
            float yn = cosf(th), r = sinf(th);   // normalized y, ring radius

            for (int j = 0; j < SLICES; ++j)
            {
                float u  = (float)j / SLICES;    // 0..1
                float ph = F_2_PI * u;

                // base position on the sphere
                float x = r * cosf(ph);
                float z = r * sinf(ph);
                float y = yn;

                // small outward jitter along the normal to get "dust"
                float nrm = sqrtf(x*x + y*y + z*z);
                float jt  = R * jitter * (Ranf(-1.f, 1.f));
                float s   = R + jt;
                float vx  = s * x / nrm;
                float vy  = s * y / nrm;
                float vz  = s * z / nrm;

                // gradient factor t in [0,1] from equator->poles
                float t = 0.5f * (y + 1.f);

                glColor4f(mixf(belt[0],cap[0],t), mixf(belt[1],cap[1],t), mixf(belt[2],cap[2],t), alpha);
                glVertex3f(vx, vy, vz);
            }
        }
        glEnd();
    }
    glEndList();
    return lid;
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitLists.\n");

	float dx = BOXSIZE / 2.f;
	float dy = BOXSIZE / 2.f;
	float dz = BOXSIZE / 2.f;
	glutSetWindow( MainWindow );

	// create the object:

	BoxList = glGenLists( 1 );
	glNewList( BoxList, GL_COMPILE );

		glBegin( GL_QUADS );

			glColor3f( 1., 0., 0. );

				glNormal3f( 1., 0., 0. );
					glVertex3f(  dx, -dy,  dz );
					glVertex3f(  dx, -dy, -dz );
					glVertex3f(  dx,  dy, -dz );
					glVertex3f(  dx,  dy,  dz );

				glNormal3f(-1., 0., 0.);
					glVertex3f( -dx, -dy,  dz);
					glVertex3f( -dx,  dy,  dz );
					glVertex3f( -dx,  dy, -dz );
					glVertex3f( -dx, -dy, -dz );

			glColor3f( 0., 1., 0. );

				glNormal3f(0., 1., 0.);
					glVertex3f( -dx,  dy,  dz );
					glVertex3f(  dx,  dy,  dz );
					glVertex3f(  dx,  dy, -dz );
					glVertex3f( -dx,  dy, -dz );

				glNormal3f(0., -1., 0.);
					glVertex3f( -dx, -dy,  dz);
					glVertex3f( -dx, -dy, -dz );
					glVertex3f(  dx, -dy, -dz );
					glVertex3f(  dx, -dy,  dz );

			glColor3f(0., 0., 1.);

				glNormal3f(0., 0., 1.);
					glVertex3f(-dx, -dy, dz);
					glVertex3f( dx, -dy, dz);
					glVertex3f( dx,  dy, dz);
					glVertex3f(-dx,  dy, dz);

				glNormal3f(0., 0., -1.);
					glVertex3f(-dx, -dy, -dz);
					glVertex3f(-dx,  dy, -dz);
					glVertex3f( dx,  dy, -dz);
					glVertex3f( dx, -dy, -dz);

		glEnd( );
#ifdef NOTDEF
		glColor3f(1., 1., 1.);
		glBegin(GL_TRIANGLES);
		glVertex3f(-dx, -dy, dz);
		glVertex3f(0., -dy, dz + 0.5f);
		glVertex3f(dx, -dy, dz);
		glEnd();
#endif

	glEndList( );


	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );
	
	// // ---------- Sun (gradient + alpha baked into the list) ----------
	// const float SunBase[3] = { 1.0f, 0.8f, 0.2f };     // warm yellow
	// SunList = MakeUvSphereList(
	// 	1.0f,               // radius
	// 	48, 32,             // slices, stacks
	// 	SunBase,            // base color
	// 	0.65f,              // alpha (semi-transparent)
	// 	1.25f,              // lighten factor near poles
	// 	0.85f               // darken factor near equator
	// );

	// Sun: point-cloud style
	const float SunBase[3] = { 1.0f, 0.8f, 0.2f };
	SunList = MakePointSphereList(
		1.0f,     // radius
		160, 120, // more slices/stacks -> denser points
		SunBase,
		0.65f,    // alpha
		1.25f,    // lighten
		0.85f,    // darken
		0.020f,   // jitter (increase a bit for glow)
		1.8f      // point size
	);

	// Outline temporarily disabled:
	// SunEdge = MakeUvSphereEdgeList(1.0f, 48, 32, (const float[3]){0,0,0}, 1.01f);

	// ---------- Planets (distinct bases + gradient + alpha) ----------
	const float Rm  = 0.18f, Rv = 0.28f, Re = 0.30f, Rm2 = 0.24f;
	const float Rj  = 0.60f, Rs = 0.50f, Ru = 0.40f, Rn  = 0.38f;
	const float radii[8] = { Rm, Rv, Re, Rm2, Rj, Rs, Ru, Rn };

	// strongly separated base colors to make differences obvious
	const float PlanetBase[8][3] = {
		{0.80f, 0.62f, 0.40f}, // Mercury  (warm gray-brown)
		{1.00f, 0.80f, 0.10f}, // Venus    (golden yellow)
		{0.10f, 0.45f, 1.00f}, // Earth    (bright blue)
		{1.00f, 0.30f, 0.10f}, // Mars     (deep orange-red)
		{0.95f, 0.75f, 0.35f}, // Jupiter  (tan/orange)
		{1.00f, 0.90f, 0.55f}, // Saturn   (pale yellow)
		{0.15f, 0.85f, 0.85f}, // Uranus   (teal-cyan)
		{0.15f, 0.25f, 1.00f}  // Neptune  (saturated blue)
	};

	// build display lists with per-vertex gradient + alpha
	for (int i = 0; i < 8; ++i) {
		// PlanetList[i] = MakeUvSphereList(
		// 	radii[i],    // radius
		// 	32, 20,      // slices, stacks
		// 	PlanetBase[i],
		// 	0.55f,       // alpha (semi-transparent)
		// 	1.20f,       // lighten factor
		// 	0.80f        // darken factor
		// );
		PlanetList[i] = MakePointSphereList(
			radii[i],
			96, 64,                 // enough density but cheaper than the Sun
			PlanetBase[i],
			0.55f,                  // alpha
			1.20f,                  // lighten
			0.80f,                  // darken
			0.010f,                 // smaller jitter for planets
			1.6f                    // point size
		);
		// Outline temporarily disabled:
		// PlanetEdgeList[i] = MakeUvSphereEdgeList(radii[i], 32, 20, (const float[3]){0,0,0}, 1.01f);
	}

	// ---------- Random placement along circular orbits (x-z plane) ----------
	const float ORBIT_R[8] = { 1.8f, 2.6f, 3.4f, 4.2f, 5.8f, 7.4f, 8.8f, 10.0f };

	// Seed once per run so positions change each execution (remove if you want fixed layout)
	TimeOfDaySeed();

	for (int i = 0; i < 8; ++i) {
		float ang = RandAngle();                  // angle in [0, 2π)
		PlanetPos[i][0] = ORBIT_R[i] * cosf(ang); // x
		PlanetPos[i][1] = 0.0f;                   // y
		PlanetPos[i][2] = ORBIT_R[i] * sinf(ang); // z
	}


}


// initialize the glui window:

void
InitMenus( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitMenus.\n");

	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(float) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Axis Colors",   colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif

	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'o':
		case 'O':
			NowProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			NowProjection = PERSP;
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler
		case '+': case '=':          // US keyboards: '+' shares key with '='
			Scale *= 1.05f;          // scale up a bit
			break;

		case '-': case '_':
			Scale *= 0.95f;          // scale down a bit
			if (Scale < MINSCALE) Scale = MINSCALE;
			break;
		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
    ActiveButton = 0;
    AxesOn = 1;
    DebugOn = 0;
    DepthBufferOn = 1;
    DepthFightingOn = 0;
    DepthCueOn = 0;
    Scale  = 2.5f;   // was 1.0f — increase to enlarge the whole scene
    ShadowsOn = 0;
    NowColor = YELLOW;
    NowProjection = PERSP;
    Xrot = Yrot = 0.;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = { 0.f, 1.f, 0.f, 1.f };

static float xy[ ] = { -.5f, .5f, .5f, -.5f };

static int xorder[ ] = { 1, 2, -3, 4 };

static float yx[ ] = { 0.f, 0.f, -.5f, .5f };

static float yy[ ] = { 0.f, .6f, 1.f, 1.f };

static int yorder[ ] = { 1, 2, 3, -2, 4 };

static float zx[ ] = { 1.f, 0.f, 1.f, 0.f, .25f, .75f };

static float zy[ ] = { .5f, .5f, -.5f, -.5f, 0.f, 0.f };

static int zorder[ ] = { 1, 2, 3, 4, -5, 6 };

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = (float)floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r=0., g=0., b=0.;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}


float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}


float
Unit( float v[3] )
{
	float dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		v[0] /= dist;
		v[1] /= dist;
		v[2] /= dist;
	}
	return dist;
}
