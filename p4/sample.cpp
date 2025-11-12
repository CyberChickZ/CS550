// The user interface allows:
// 1. The axes to be turned on and off d
// 2. The color of the axes to be changed
// 3. Debugging to be turned on and off
// 4. Depth cueing to be turned on and off
// 5. The projection to be changed
// 6. The transformations to be reset
// 7. The program to quit
// // Author:        Haochuan Zhang   <zhanhaoc@oregonstate.edu>
// Course:        OSU CS 450/550 — Fall 2025, Project #4: Keytime Animation
// // Notes: Header retained from the original sample; name/email updated per assignment.

#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctype.h>
#include <time.h>

#ifndef F_PI
#define F_PI        ((float)(M_PI))
#define F_2_PI      ((float)(2.f*F_PI))
#define F_PI_2      ((float)(F_PI/2.f))
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
#include <string.h>

//  ---------------------------------------------------------------------------
//  Original window titles kept (can be changed in menus if needed)
const char *WINDOWTITLE = "OpenGL / GLUT Sample - P4 -- Haochuan Zhang";
const char *GLUITITLE   = "User Interface Window";
//  ---------------------------------------------------------------------------

const int GLUITRUE  = true;
const int GLUIFALSE = false;
const int ESCAPE = 0x1b;
const int INIT_WINDOW_SIZE = 1000;
const float BOXSIZE = 2.f;
const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;
const float MINSCALE = 0.05f;
const int SCROLL_WHEEL_UP   = 3;
const int SCROLL_WHEEL_DOWN = 4;
const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;
const int LEFT   = 4;
const int MIDDLE = 2;
const int RIGHT  = 1;

enum Projections { ORTHO, PERSP };
enum ButtonVals { RESET, QUIT };

const GLfloat BACKCOLOR[] = { 0., 0., 0., 1. };
const GLfloat AXES_WIDTH  = 3.;

enum Colors { RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA };
char * ColorNames[] = { (char*)"Red", (char*)"Yellow", (char*)"Green", (char*)"Cyan", (char*)"Blue", (char*)"Magenta" };
const GLfloat Colors[][3] = { {1.,0.,0.},{1.,1.,0.},{0.,1.,0.},{0.,1.,1.},{0.,0.,1.},{1.,0.,1.} };

const GLfloat FOGCOLOR[4] = { .0f, .0f, .0f, 1.f };
const GLenum  FOGMODE     = GL_LINEAR;
const GLfloat FOGDENSITY  = 0.30f;
const GLfloat FOGSTART    = 1.5f;
const GLfloat FOGEND      = 4.f;

const float WHITE[] = {1.,1.,1.,1.};

//  === 10-second cycle ===
const int MS_PER_CYCLE = 10000; //  10000 milliseconds = 10 seconds

//  ----------------------- Non-constant globals -------------------------------
int     ActiveButton;
GLuint  AxesList;
int     AxesOn;
GLuint  BoxList;
int     DebugOn;
int     DepthCueOn;
int     DepthBufferOn;
int     DepthFightingOn;
int     MainWindow;
int     NowColor;
int     NowProjection;
float   Scale;
int     ShadowsOn;
float   Time;
int     Xmouse, Ymouse;
float   Xrot, Yrot;

//  Models and room
float   OBJ_SCALE = 0.5f;
GLuint  FloorDL, WallBackDL, WallRightDL;
GLuint  DinoDL, DogDL, DuckyDL;

//  Lighting + freeze
enum LightMode { POINT_LIGHT = 0, SPOT_LIGHT = 1 };
int     gLightMode = POINT_LIGHT;
bool    Frozen = false;
#define SPEED         3.f
#define LIGHTRADIUS   3.0f
#define LIGHT_Y       1.0f
float gLightColor[4] = {1.f,1.f,1.f,1.f};
static const float COL_WHITE[4]  = {1.f,1.f,1.f,1.f};
static const float COL_RED[4]    = {1.f,0.f,0.f,1.f};
static const float COL_ORANGE[4] = {1.f,0.5f,0.f,1.f};
static const float COL_YELLOW[4] = {1.f,1.f,0.f,1.f};
static const float COL_GREEN[4]  = {0.f,1.f,0.f,1.f};
static const float COL_CYAN[4]   = {0.f,1.f,1.f,1.f};
static const float COL_MAG[4]    = {1.f,0.f,1.f,1.f};

#define XSIDE 8.f
#define ZSIDE 8.f
#define YSIDE 4.f
#define NX 800
#define NZ 800
#define NY 400
#define X0 (-XSIDE/2.f)
#define Z0 (-ZSIDE/2.f)
#define Y0 (-YSIDE/2.f)
#define DX (XSIDE / (float)NX)
#define DZ (ZSIDE / (float)NZ)
#define DY (YSIDE / (float)NY)

//  ------------------- Prototypes from original program -----------------------
void    Animate();
void    Display();
void    DoAxesMenu(int);
void    DoColorMenu(int);
void    DoDepthBufferMenu(int);
void    DoDepthFightingMenu(int);
void    DoDepthMenu(int);
void    DoDebugMenu(int);
void    DoMainMenu(int);
void    DoProjectMenu(int);
void    DoRasterString(float,float,float,char*);
void    DoStrokeString(float,float,float,float,char*);
float   ElapsedSeconds();
void    InitGraphics();
void    InitLists();
void    InitMenus();
void    Keyboard(unsigned char,int,int);
void    MouseButton(int,int,int,int);
void    MouseMotion(int,int);
void    Reset();
void    Resize(int,int);
void    Visibility(int);
void    Axes(float);
void    HsvRgb(float[3],float[3]);
void    Cross(float[3],float[3],float[3]);
float   Dot(float[3],float[3]);
float   Unit(float[3],float[3]);
float   Unit(float[3]);

//  helpers provided by course includes
float * Array3(float a,float b,float c){ static float arr[4]; arr[0]=a;arr[1]=b;arr[2]=c;arr[3]=1.; return arr; }
float * MulArray3(float f,float a[ ]){ static float arr[4]; arr[0]=f*a[0];arr[1]=f*a[1];arr[2]=f*a[2];arr[3]=1.; return arr; }
float * MulArray3(float f,float a,float b,float c){ static float arr[4]; float* abc=Array3(a,b,c); arr[0]=f*abc[0];arr[1]=f*abc[1];arr[2]=f*abc[2];arr[3]=1.; return arr; }
float   Ranf(float low,float high){ float r=(float)rand(); float t=r/(float)RAND_MAX; return low + t*(high-low); }
void    TimeOfDaySeed(){ struct tm y2k={}; y2k.tm_year=2000; y2k.tm_mon=0; y2k.tm_mday=1; time_t now; time(&now); double sec=difftime(now,mktime(&y2k)); unsigned int seed=(unsigned int)(1000.*sec); srand(seed);} 

//  keep course include style EXACTLY as-is (do not touch include method)
#include "setmaterial.cpp"
#include "setlight.cpp"
#include "osusphere.cpp"
#include "osucube.cpp"
#include "osucylindercone.cpp"
#include "osutorus.cpp"
#include "bmptotexture.cpp"
#include "loadobjmtlfiles.cpp"
//  #include "keytime.cpp"   //  keep commented per user instruction; we embed a minimal class below

//  ======================= Embedded Keytimes (Coons/Hermite) ===================
class Keytimes {
public:
    void Init(){ keys.clear(); }
    void AddTimeValue(float t,float v){ keys.emplace_back(t,v); std::sort(keys.begin(),keys.end(),[](auto&a,auto&b){return a.first<b.first;}); }
    float GetValue(float t) const{
        if(keys.empty()) return 0.f;
        if(t<=keys.front().first) return keys.front().second;
        if(t>=keys.back().first)  return keys.back().second;
        int i1=1; while(i1<(int)keys.size() && t>keys[i1].first) ++i1; int i0=i1-1;
        float t0=keys[i0].first, y0=keys[i0].second; float t1=keys[i1].first, y1=keys[i1].second; float dt=t1-t0; if(dt<=0.f) return y0; float s=(t-t0)/dt;
        float m0, m1;
        if(i0>0){ float tm=keys[i0-1].first, ym=keys[i0-1].second; m0=(y1-ym)/(t1-tm); } else { m0=(y1-y0)/dt; }
        if(i1+1<(int)keys.size()){ float tp=keys[i1+1].first, yp=keys[i1+1].second; m1=(yp-y0)/(tp-t0);} else { m1=(y1-y0)/dt; }
        float s2=s*s, s3=s2*s; float h00= 2*s3-3*s2+1; float h10=s3-2*s2+s; float h01=-2*s3+3*s2; float h11=s3-s2;
        return h00*y0 + h10*(dt*m0) + h01*y1 + h11*(dt*m1);
    }
private: std::vector<std::pair<float,float>> keys;
};

//  ------------- 9 animation channels (3 camera + 3 obj1 + 3 obj2) -----------
static Keytimes CamEyeX, CamLookY, CamUpZ;      //  camera
static Keytimes Obj1_Tx, Obj1_Ry, Obj1_S;       //  Dino (Object #1)
static Keytimes Obj2_Tz, Obj2_Rx, Obj2_Sy;      //  Ducky (Object #2)

static void InitKeytimes(){
    //  Each channel: >=6 keys and returns to initial value at t=10.0
    //  Camera
    CamEyeX.Init();  CamEyeX.AddTimeValue(0.0f,  2.5f); CamEyeX.AddTimeValue(2.0f,  1.0f); CamEyeX.AddTimeValue(4.0f,  0.0f); CamEyeX.AddTimeValue(6.0f, -1.0f); CamEyeX.AddTimeValue(8.0f,  0.8f); CamEyeX.AddTimeValue(10.0f, 2.5f);
    CamLookY.Init(); CamLookY.AddTimeValue(0.0f,  0.0f); CamLookY.AddTimeValue(2.0f,  0.8f); CamLookY.AddTimeValue(4.0f,  0.0f); CamLookY.AddTimeValue(6.0f, -0.6f); CamLookY.AddTimeValue(8.0f,  0.3f); CamLookY.AddTimeValue(10.0f, 0.0f);
    CamUpZ.Init();   CamUpZ.AddTimeValue(0.0f,  1.0f); CamUpZ.AddTimeValue(2.0f,  0.9f); CamUpZ.AddTimeValue(4.0f,  1.1f); CamUpZ.AddTimeValue(6.0f,  1.0f); CamUpZ.AddTimeValue(8.0f,  1.05f); CamUpZ.AddTimeValue(10.0f, 1.0f);

    //  Object #1: DINO — tx, ry, uniform scale
    Obj1_Tx.Init();  Obj1_Tx.AddTimeValue(0.0f, -2.2f); Obj1_Tx.AddTimeValue(2.0f, -0.8f); Obj1_Tx.AddTimeValue(4.0f,  0.0f); Obj1_Tx.AddTimeValue(6.0f,  1.0f); Obj1_Tx.AddTimeValue(8.0f,  2.2f); Obj1_Tx.AddTimeValue(10.0f,-2.2f);
    Obj1_Ry.Init();  Obj1_Ry.AddTimeValue(0.0f,   0.f ); Obj1_Ry.AddTimeValue(2.0f,  90.f ); Obj1_Ry.AddTimeValue(4.0f, 180.f ); Obj1_Ry.AddTimeValue(6.0f, 270.f ); Obj1_Ry.AddTimeValue(8.0f, 360.f ); Obj1_Ry.AddTimeValue(10.0f,  0.f );
    Obj1_S.Init();   Obj1_S.AddTimeValue(0.0f, 1.00f); Obj1_S.AddTimeValue(2.0f, 1.20f); Obj1_S.AddTimeValue(4.0f, 0.85f); Obj1_S.AddTimeValue(6.0f, 1.30f); Obj1_S.AddTimeValue(8.0f, 0.95f); Obj1_S.AddTimeValue(10.0f,1.00f);

    //  Object #2: DUCKY — tz, rx, scale.y (non-uniform)
    Obj2_Tz.Init();  Obj2_Tz.AddTimeValue(0.0f,  2.6f); Obj2_Tz.AddTimeValue(2.0f,  1.0f); Obj2_Tz.AddTimeValue(4.0f, -0.6f); Obj2_Tz.AddTimeValue(6.0f, -2.0f); Obj2_Tz.AddTimeValue(8.0f,  0.4f); Obj2_Tz.AddTimeValue(10.0f, 2.6f);
    Obj2_Rx.Init();  Obj2_Rx.AddTimeValue(0.0f,   0.f ); Obj2_Rx.AddTimeValue(2.0f,  60.f ); Obj2_Rx.AddTimeValue(4.0f, 140.f ); Obj2_Rx.AddTimeValue(6.0f, 220.f ); Obj2_Rx.AddTimeValue(8.0f, 320.f ); Obj2_Rx.AddTimeValue(10.0f,  0.f );
    Obj2_Sy.Init();  Obj2_Sy.AddTimeValue(0.0f, 1.00f); Obj2_Sy.AddTimeValue(2.0f, 1.40f); Obj2_Sy.AddTimeValue(4.0f, 0.80f); Obj2_Sy.AddTimeValue(6.0f, 1.25f); Obj2_Sy.AddTimeValue(8.0f, 0.90f); Obj2_Sy.AddTimeValue(10.0f,1.00f);
}

//  ============================ Main (unchanged) ==============================
int main(int argc,char*argv[]){ glutInit(&argc,argv); InitGraphics(); InitLists(); Reset(); InitMenus(); glutSetWindow(MainWindow); glutMainLoop(); return 0; }

//  ============================ Animate (unchanged) ===========================
void Animate(){ if(Frozen) return; int ms=glutGet(GLUT_ELAPSED_TIME); ms%=MS_PER_CYCLE; Time=(float)ms/(float)MS_PER_CYCLE; glutSetWindow(MainWindow); glutPostRedisplay(); }

//  ============================ Display (modified) ============================
void Display(){ glutSetWindow(MainWindow); glDrawBuffer(GL_BACK); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    GLsizei vx=glutGet(GLUT_WINDOW_WIDTH), vy=glutGet(GLUT_WINDOW_HEIGHT); GLsizei v=vx<vy?vx:vy; GLint xl=(vx-v)/2, yb=(vy-v)/2; glViewport(xl,yb,v,v);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); if(NowProjection==ORTHO) glOrtho(-2.f,2.f,-2.f,2.f,0.1f,1000.f); else gluPerspective(70.f,1.f,0.1f,1000.f);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    //  === Keytime clock in seconds (0..10) ===
    float nowSec = Time * 10.0f;

    //  === Animated camera (3 quantities) ===
    float eyeX = CamEyeX.GetValue(nowSec);  float eyeY = 0.0f; float eyeZ = 6.0f;
    float lookX= 0.0f;                      float lookY= CamLookY.GetValue(nowSec); float lookZ= 0.0f;
    float upX  = 0.0f;                      float upY  = 1.0f;                      float upZ  = CamUpZ.GetValue(nowSec);
    gluLookAt(eyeX,eyeY,eyeZ,  lookX,lookY,lookZ,  upX,upY,upZ);

    //  Mouse-controlled scene rotations and scale (kept from original)
    glRotatef((GLfloat)Yrot,0.f,1.f,0.f); glRotatef((GLfloat)Xrot,1.f,0.f,0.f);
    if(Scale<MINSCALE) Scale=MINSCALE; glScalef(Scale,Scale,Scale); glEnable(GL_NORMALIZE);

    //  === Animated light on a circular path ===
    float theta=SPEED*F_2_PI*Time; float xlight=-LIGHTRADIUS*cosf(theta); float zlight=LIGHTRADIUS*sinf(theta); float ylight=LIGHT_Y;
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);
    float ambient[4]={0.f,0.f,0.f,1.f}; glLightfv(GL_LIGHT0,GL_AMBIENT,ambient); glLightfv(GL_LIGHT0,GL_DIFFUSE,gLightColor); glLightfv(GL_LIGHT0,GL_SPECULAR,gLightColor);
    float pos[4]={xlight,ylight,zlight,1.f}; glLightfv(GL_LIGHT0,GL_POSITION,pos);
    glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION,1.f); glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,0.f); glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,0.f);
    if(gLightMode==POINT_LIGHT){ glLightf(GL_LIGHT0,GL_SPOT_CUTOFF,180.f); glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,0.f);} else { float zBack=-ZSIDE/2.f; float dir[3]={0.f,0.f,zBack - zlight}; float len=sqrtf(dir[0]*dir[0]+dir[1]*dir[1]+dir[2]*dir[2]); if(len>0){dir[0]/=len;dir[1]/=len;dir[2]/=len;} glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,dir); glLightf(GL_LIGHT0,GL_SPOT_CUTOFF,18.f); glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,20.f); }
    glDisable(GL_LIGHTING); glPushMatrix(); glTranslatef(xlight,ylight,zlight); glColor3f(gLightColor[0],gLightColor[1],gLightColor[2]); OsuSphere(0.10f,24,16); glPopMatrix();

    //  Fog (kept)
    if(DepthCueOn!=0){ glFogi(GL_FOG_MODE,FOGMODE); glFogfv(GL_FOG_COLOR,FOGCOLOR); glFogf(GL_FOG_DENSITY,FOGDENSITY); glFogf(GL_FOG_START,FOGSTART); glFogf(GL_FOG_END,FOGEND); glEnable(GL_FOG);} else { glDisable(GL_FOG);} 

    if(AxesOn!=0){ glColor3fv(&Colors[NowColor][0]); glCallList(AxesList); }

    //  Room
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glCallList(FloorDL); glCallList(WallBackDL); glCallList(WallRightDL); glDisable(GL_LIGHTING);

    //  ================== Two animated objects (Dino, Ducky) ==================
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);

    //  Object #1: DINO — tx, ry, s
    {
        float tx = Obj1_Tx.GetValue(nowSec);
        float ry = Obj1_Ry.GetValue(nowSec);
        float s  = Obj1_S.GetValue(nowSec);
        glPushMatrix();
            glTranslatef(tx, 0.4f, 0.0f);
            glRotatef(-90.f,0.f,1.f,0.f);            //  keep original facing
            glRotatef(ry,  0.f,1.f,0.f);
            glScalef(OBJ_SCALE*0.25f*s, OBJ_SCALE*0.25f*s, OBJ_SCALE*0.25f*s);
            SetMaterial(1.00f,0.10f,0.10f,15.f);
            GLfloat spec_dull[4]={0.05f,0.05f,0.05f,1.f}; glMaterialfv(GL_FRONT,GL_SPECULAR,spec_dull);
            if(DinoDL) glCallList(DinoDL);
        glPopMatrix();
    }

    //  Object #2: DUCKY — tz, rx, sy (non-uniform)
    {
        float tz = Obj2_Tz.GetValue(nowSec);
        float rx = Obj2_Rx.GetValue(nowSec);
        float sy = Obj2_Sy.GetValue(nowSec);
        glPushMatrix();
            glTranslatef(1.8f, 0.0f, tz);
            glRotatef(rx, 1.f,0.f,0.f);
            glScalef(OBJ_SCALE, OBJ_SCALE*sy, OBJ_SCALE);
            SetMaterial(0.15f,0.45f,1.00f,200.f);
            GLfloat spec_high[4]={1.f,1.f,1.f,1.f}; glMaterialfv(GL_FRONT,GL_SPECULAR,spec_high);
            if(DuckyDL) glCallList(DuckyDL);
        glPopMatrix();
    }

    glDisable(GL_LIGHTING);

    //  2D overlay region (kept empty)
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluOrtho2D(0.f,100.f,0.f,100.f);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity(); glColor3f(1.f,1.f,1.f);

    glutSwapBuffers(); glFlush();
}

//  ============================ Menus/Keyboard/etc. ===========================
void DoAxesMenu(int id){ AxesOn=id; glutSetWindow(MainWindow); glutPostRedisplay(); }
void DoColorMenu(int id){ NowColor=id-RED; glutSetWindow(MainWindow); glutPostRedisplay(); }
void DoDebugMenu(int id){ DebugOn=id; glutSetWindow(MainWindow); glutPostRedisplay(); }
void DoDepthBufferMenu(int id){ DepthBufferOn=id; glutSetWindow(MainWindow); glutPostRedisplay(); }
void DoDepthFightingMenu(int id){ DepthFightingOn=id; glutSetWindow(MainWindow); glutPostRedisplay(); }
void DoDepthMenu(int id){ DepthCueOn=id; glutSetWindow(MainWindow); glutPostRedisplay(); }

void DoMainMenu(int id){ switch(id){ case RESET: Reset(); break; case QUIT: glutSetWindow(MainWindow); glFinish(); glutDestroyWindow(MainWindow); exit(0); default: fprintf(stderr,"Unknown Main Menu ID %d",id);} glutSetWindow(MainWindow); glutPostRedisplay(); }
void DoProjectMenu(int id){ NowProjection=id; glutSetWindow(MainWindow); glutPostRedisplay(); }

void Keyboard(unsigned char c,int,int){ switch(c){
    case 'q': case 'Q': case ESCAPE: DoMainMenu(QUIT); break;
    case '+': case '=': Scale += SCLFACT*SCROLL_WHEEL_CLICK_FACTOR; if(Scale<MINSCALE) Scale=MINSCALE; break;
    case '-': case '_': Scale -= SCLFACT*SCROLL_WHEEL_CLICK_FACTOR; if(Scale<MINSCALE) Scale=MINSCALE; break;
    case 'w': case 'W': memcpy(gLightColor,COL_WHITE,sizeof(gLightColor)); break;
    case 'r': case 'R': memcpy(gLightColor,COL_RED,  sizeof(gLightColor)); break;
    case 'o': case 'O': memcpy(gLightColor,COL_ORANGE,sizeof(gLightColor)); break;
    case 'y': case 'Y': memcpy(gLightColor,COL_YELLOW,sizeof(gLightColor)); break;
    case 'g': case 'G': memcpy(gLightColor,COL_GREEN,sizeof(gLightColor)); break;
    case 'c': case 'C': memcpy(gLightColor,COL_CYAN, sizeof(gLightColor)); break;
    case 'm': case 'M': memcpy(gLightColor,COL_MAG,  sizeof(gLightColor)); break;
    case 'p': case 'P': gLightMode=POINT_LIGHT; break;
    case 's': case 'S': gLightMode=SPOT_LIGHT;  break;
    case 'f': case 'F': Frozen=!Frozen; if(Frozen) glutIdleFunc(NULL); else glutIdleFunc(Animate); break;
    default: fprintf(stderr,"Unknown key: '%c' (0x%02x)",c,c); }
    glutSetWindow(MainWindow); glutPostRedisplay(); }

void MouseButton(int button,int state,int x,int y){ int b=0; switch(button){ case GLUT_LEFT_BUTTON: b=LEFT; break; case GLUT_MIDDLE_BUTTON: b=MIDDLE; break; case GLUT_RIGHT_BUTTON: b=RIGHT; break; case SCROLL_WHEEL_UP: Scale += SCLFACT*SCROLL_WHEEL_CLICK_FACTOR; if(Scale<MINSCALE) Scale=MINSCALE; break; case SCROLL_WHEEL_DOWN: Scale -= SCLFACT*SCROLL_WHEEL_CLICK_FACTOR; if(Scale<MINSCALE) Scale=MINSCALE; break; default: b=0; fprintf(stderr,"Unknown mouse button: %d",button);} if(state==GLUT_DOWN){ Xmouse=x; Ymouse=y; ActiveButton |= b; } else { ActiveButton &= ~b; } glutSetWindow(MainWindow); glutPostRedisplay(); }
void MouseMotion(int x,int y){ int dx=x-Xmouse, dy=y-Ymouse; if((ActiveButton&LEFT)!=0){ Xrot += (ANGFACT*dy); Yrot += (ANGFACT*dx);} if((ActiveButton&MIDDLE)!=0){ Scale += SCLFACT*(float)(dx-dy); if(Scale<MINSCALE) Scale=MINSCALE;} Xmouse=x; Ymouse=y; glutSetWindow(MainWindow); glutPostRedisplay(); }

void Reset(){ ActiveButton=0; AxesOn=1; DebugOn=0; DepthBufferOn=1; DepthFightingOn=0; DepthCueOn=0; Scale=1.0f; ShadowsOn=0; NowColor=YELLOW; NowProjection=PERSP; Xrot=Yrot=0.f; Frozen=false; gLightMode=POINT_LIGHT; memcpy(gLightColor,COL_WHITE,sizeof(gLightColor)); }
void Resize(int,int){ glutSetWindow(MainWindow); glutPostRedisplay(); }
void Visibility(int state){ if(state==GLUT_VISIBLE){ glutSetWindow(MainWindow); glutPostRedisplay(); } }

//  ============================ Graphics Setup ================================
void InitGraphics(){ glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH); glutInitWindowPosition(0,0); glutInitWindowSize(INIT_WINDOW_SIZE,INIT_WINDOW_SIZE); MainWindow=glutCreateWindow(WINDOWTITLE); glutSetWindowTitle(WINDOWTITLE); glClearColor(BACKCOLOR[0],BACKCOLOR[1],BACKCOLOR[2],BACKCOLOR[3]); glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
    glutSetWindow(MainWindow); glutDisplayFunc(Display); glutReshapeFunc(Resize); glutKeyboardFunc(Keyboard); glutMouseFunc(MouseButton); glutMotionFunc(MouseMotion); glutPassiveMotionFunc(MouseMotion); glutVisibilityFunc(Visibility); glutEntryFunc(NULL); glutSpecialFunc(NULL); glutIdleFunc(Animate);
#ifdef WIN32
    GLenum err=glewInit(); if(err!=GLEW_OK){ fprintf(stderr,"glewInit Error"); } else fprintf(stderr,"GLEW initialized OK"); fprintf(stderr,"Status: Using GLEW %s",glewGetString(GLEW_VERSION));
#endif
    InitKeytimes(); //  <-- our keytime channels
}

void InitLists(){ float dx=BOXSIZE/2.f, dy=BOXSIZE/2.f, dz=BOXSIZE/2.f; glutSetWindow(MainWindow);
    //  cube (kept from original)
    BoxList=glGenLists(1); glNewList(BoxList,GL_COMPILE); glBegin(GL_QUADS);
        glColor3f(1.,0.,0.);   glNormal3f( 1.,0.,0.);  glVertex3f( dx,-dy, dz); glVertex3f( dx,-dy,-dz); glVertex3f( dx, dy,-dz); glVertex3f( dx, dy, dz);
        glNormal3f(-1.,0.,0.); glVertex3f(-dx,-dy, dz); glVertex3f(-dx, dy, dz); glVertex3f(-dx, dy,-dz); glVertex3f(-dx,-dy,-dz);
        glColor3f(0.,1.,0.);   glNormal3f( 0.,1.,0.);  glVertex3f(-dx, dy, dz); glVertex3f( dx, dy, dz); glVertex3f( dx, dy,-dz); glVertex3f(-dx, dy,-dz);
        glNormal3f( 0.,-1.,0.);glVertex3f(-dx,-dy, dz); glVertex3f(-dx,-dy,-dz); glVertex3f( dx,-dy,-dz); glVertex3f( dx,-dy, dz);
        glColor3f(0.,0.,1.);   glNormal3f( 0.,0.,1.);  glVertex3f(-dx,-dy, dz); glVertex3f( dx,-dy, dz); glVertex3f( dx, dy, dz); glVertex3f(-dx, dy, dz);
        glNormal3f( 0.,0.,-1.);glVertex3f(-dx,-dy,-dz); glVertex3f(-dx, dy,-dz); glVertex3f( dx, dy,-dz); glVertex3f( dx,-dy,-dz);
    glEnd(); glEndList();

    //  Floor and two walls
    const float ZWALL = -ZSIDE/2.f; const float XWALL = +XSIDE/2.f;
    FloorDL=glGenLists(1); glNewList(FloorDL,GL_COMPILE); SetMaterial(0.60f,0.80f,0.60f,30.f); glNormal3f(0.f,1.f,0.f); for(int i=0;i<NZ;++i){ glBegin(GL_QUAD_STRIP); for(int j=0;j<=NX;++j){ glVertex3f(X0+DX*j, Y0, Z0+DZ*(i+0)); glVertex3f(X0+DX*j, Y0, Z0+DZ*(i+1)); } glEnd(); } glEndList();
    WallBackDL=glGenLists(1); glNewList(WallBackDL,GL_COMPILE); SetMaterial(0.70f,0.70f,0.85f,20.f); glNormal3f(0.f,0.f,1.f); for(int i=0;i<NY;++i){ glBegin(GL_QUAD_STRIP); for(int j=0;j<=NX;++j){ glVertex3f(X0+DX*j, Y0+DY*(i+0), ZWALL); glVertex3f(X0+DX*j, Y0+DY*(i+1), ZWALL);} glEnd(); } glEndList();
    WallRightDL=glGenLists(1); glNewList(WallRightDL,GL_COMPILE); SetMaterial(0.85f,0.70f,0.70f,20.f); glNormal3f(-1.f,0.f,0.f); for(int i=0;i<NY;++i){ glBegin(GL_QUAD_STRIP); for(int j=0;j<=NZ;++j){ glVertex3f(XWALL, Y0+DY*(i+0), Z0+DZ*j); glVertex3f(XWALL, Y0+DY*(i+1), Z0+DZ*j);} glEnd(); } glEndList();

    //  Load OBJ models available to the user
    DinoDL  = LoadObjMtlFiles((char*)"dino.obj");
    DogDL   = LoadObjMtlFiles((char*)"dog.obj");   //  loaded but not used in the 2-object animation
    DuckyDL = LoadObjMtlFiles((char*)"ducky.obj");
}

void InitMenus(){ glutSetWindow(MainWindow); int numColors = sizeof(Colors)/(3*sizeof(float)); int colormenu=glutCreateMenu(DoColorMenu); for(int i=0;i<numColors;i++) glutAddMenuEntry(ColorNames[i],i); int axesmenu=glutCreateMenu(DoAxesMenu); glutAddMenuEntry("Off",0); glutAddMenuEntry("On",1); int depthcuemenu=glutCreateMenu(DoDepthMenu); glutAddMenuEntry("Off",0); glutAddMenuEntry("On",1); int depthbuffermenu=glutCreateMenu(DoDepthBufferMenu); glutAddMenuEntry("Off",0); glutAddMenuEntry("On",1); int depthfightingmenu=glutCreateMenu(DoDepthFightingMenu); glutAddMenuEntry("Off",0); glutAddMenuEntry("On",1); int debugmenu=glutCreateMenu(DoDebugMenu); glutAddMenuEntry("Off",0); glutAddMenuEntry("On",1); int projmenu=glutCreateMenu(DoProjectMenu); glutAddMenuEntry("Orthographic",ORTHO); glutAddMenuEntry("Perspective",PERSP); int mainmenu=glutCreateMenu(DoMainMenu); glutAddSubMenu("Axes",axesmenu); glutAddSubMenu("Axis Colors",colormenu); glutAddSubMenu("Depth Cue",depthcuemenu); glutAddSubMenu("Projection",projmenu); glutAddMenuEntry("Reset",RESET); glutAddSubMenu("Debug",debugmenu); glutAddMenuEntry("Quit",QUIT); glutAttachMenu(GLUT_RIGHT_BUTTON); }

//  ============================ Utilities (kept) ==============================
static float xx[]={0.f,1.f,0.f,1.f}; static float xy[]={-.5f,.5f,.5f,-.5f}; static int xorder[]={1,2,-3,4}; static float yx[]={0.f,0.f,-.5f,.5f}; static float yy[]={0.f,.6f,1.f,1.f}; static int yorder[]={1,2,3,-2,4}; static float zx[]={1.f,0.f,1.f,0.f,.25f,.75f}; static float zy[]={.5f,.5f,-.5f,-.5f,0.f,0.f}; static int zorder[]={1,2,3,4,-5,6}; const float LENFRAC=0.10f; const float BASEFRAC=1.10f;
void Axes(float length){ glBegin(GL_LINE_STRIP); glVertex3f(length,0.,0.); glVertex3f(0.,0.,0.); glVertex3f(0.,length,0.); glEnd(); glBegin(GL_LINE_STRIP); glVertex3f(0.,0.,0.); glVertex3f(0.,0.,length); glEnd(); float fact=LENFRAC*length; float base=BASEFRAC*length; glBegin(GL_LINE_STRIP); for(int i=0;i<4;i++){ int j=xorder[i]; if(j<0){ glEnd(); glBegin(GL_LINE_STRIP); j=-j;} j--; glVertex3f(base+fact*xx[j], fact*xy[j], 0.0);} glEnd(); glBegin(GL_LINE_STRIP); for(int i=0;i<5;i++){ int j=yorder[i]; if(j<0){ glEnd(); glBegin(GL_LINE_STRIP); j=-j;} j--; glVertex3f(fact*yx[j], base+fact*yy[j], 0.0);} glEnd(); glBegin(GL_LINE_STRIP); for(int i=0;i<6;i++){ int j=zorder[i]; if(j<0){ glEnd(); glBegin(GL_LINE_STRIP); j=-j;} j--; glVertex3f(0.0, fact*zy[j], base+fact*zx[j]); } glEnd(); }
void HsvRgb(float hsv[3],float rgb[3]){ float h=hsv[0]/60.f; while(h>=6.) h-=6.; while(h<0.) h+=6.; float s=hsv[1]; s=fmaxf(0.f,fminf(1.f,s)); float v=hsv[2]; v=fmaxf(0.f,fminf(1.f,v)); if(s==0.f){ rgb[0]=rgb[1]=rgb[2]=v; return;} float i=floorf(h), f=h-i; float p=v*(1.f-s), q=v*(1.f-s*f), t=v*(1.f-(s*(1.f-f))); float r=0,g=0,b=0; switch((int)i){ case 0:r=v;g=t;b=p;break; case 1:r=q;g=v;b=p;break; case 2:r=p;g=v;b=t;break; case 3:r=p;g=q;b=v;break; case 4:r=t;g=p;b=v;break; case 5:r=v;g=p;b=q;break;} rgb[0]=r; rgb[1]=g; rgb[2]=b; }
void Cross(float v1[3],float v2[3],float vout[3]){ float tmp[3]; tmp[0]=v1[1]*v2[2]-v2[1]*v1[2]; tmp[1]=v2[0]*v1[2]-v1[0]*v2[2]; tmp[2]=v1[0]*v2[1]-v2[0]*v1[1]; vout[0]=tmp[0]; vout[1]=tmp[1]; vout[2]=tmp[2]; }
float Dot(float v1[3],float v2[3]){ return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2]; }
float Unit(float vin[3],float vout[3]){ float d=vin[0]*vin[0]+vin[1]*vin[1]+vin[2]*vin[2]; if(d>0.0){ d=sqrtf(d); vout[0]=vin[0]/d; vout[1]=vin[1]/d; vout[2]=vin[2]/d; } else { vout[0]=vin[0]; vout[1]=vin[1]; vout[2]=vin[2]; } return d; }
float Unit(float v[3]){ float d=v[0]*v[0]+v[1]*v[1]+v[2]*v[2]; if(d>0.0){ d=sqrtf(d); v[0]/=d; v[1]/=d; v[2]/=d; } return d; }
