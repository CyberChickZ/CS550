/* 
    CS 450/550 -- Fall 2025
    Project #5: Texture Mapping (built on my P4 base)
    Author:        Haochuan Zhang   <zhanhaoc@oregonstate.edu>
    Course:        OSU CS 550 — Fall 2025, Project #5: Texture Mapping
*/
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#ifdef __APPLE__
#   include <OpenGL/gl.h>
#   include <OpenGL/glu.h>
#   include <GLUT/glut.h>
#else
#   include <GL/gl.h>
#   include <GL/glu.h>
#   include <GL/glut.h>
#endif

// ---------------------------------------------------------------------
// Helpers expected by the course support files.
// ---------------------------------------------------------------------
static float gTmp4_[4];
static inline float* Array3(float a, float b, float c){
    gTmp4_[0]=a; gTmp4_[1]=b; gTmp4_[2]=c; gTmp4_[3]=1.f;
    return gTmp4_;
}
static inline float* MulArray3(float s, const float* rgb3){
    gTmp4_[0]=s*rgb3[0]; gTmp4_[1]=s*rgb3[1]; gTmp4_[2]=s*rgb3[2]; gTmp4_[3]=1.f;
    return gTmp4_;
}
static inline float* MulArray3(float s, float r, float g, float b){
    gTmp4_[0]=s*r; gTmp4_[1]=s*g; gTmp4_[2]=s*b; gTmp4_[3]=1.f;
    return gTmp4_;
}
static float WHITE_[3] = {1.f,1.f,1.f};
#define WHITE WHITE_

static inline void Cross(float v1[3], float v2[3], float vout[3]){
    float tmp[3];
    tmp[0] = v1[1]*v2[2] - v2[1]*v1[2];
    tmp[1] = v2[0]*v1[2] - v1[0]*v2[2];
    tmp[2] = v1[0]*v2[1] - v2[0]*v1[1];
    vout[0] = tmp[0];
    vout[1] = tmp[1];
    vout[2] = tmp[2];
}

// osucylindercone.cpp calls Unit(float[3],float[3]); make sure it is declared:
float Unit( float [3], float [3] );

// ---------------------------------------------------------------------
// P4 helper sources — keep include style exactly as-is.
// ---------------------------------------------------------------------
#include "setmaterial.cpp"
#include "setlight.cpp"
#include "osusphere.cpp"
#include "osucube.cpp"
#include "osucylindercone.cpp"
#include "osutorus.cpp"
#include "bmptotexture.cpp"
#include "loadobjmtlfiles.cpp"
#include "keytime.cpp"
// #include "glslprogram.cpp"

// ---------------------------------------------------------------------
#define MS_PER_CYCLE 10000
#define DEG2RAD(d)   ((d)*(M_PI/180.f))
#define RAD2DEG(r)   ((r)*(180.f/M_PI))

static inline float Clamp(float v,float lo,float hi){ return v<lo?lo:(v>hi?hi:v); }

// window
int   MainWindow = 0;
int   WindowWidth = 1024, WindowHeight = 768;

// time / toggles
float TimeFrac = 0.f;
bool  UseTexture = true;     // 't'
bool  ObjMotionOn = true;    // ';'
bool  LightMotionOn = true;  // '''
int   LightType = 0;         // 0=Point, 1=Spot  ('k')
int   MatPaletteIdx = 0;     // 'l'
int   NowObject = 0;         // 0..9, cycled by ',' '.'
int   LookSlot = -1;         // -1=free, 0..9 objects, 10=FarView  ('1')

// free camera
float CamDist = 12.f, CamAzim = 45.f, CamElev = 20.f;
int   ActiveButton = 0, Xmouse=0, Ymouse=0;

// bias while LookAt-locked
float ViewBiasYaw = 0.f, ViewBiasPitch = 0.f;

// geometry types
enum GeomType{ G_SPHERE=0, G_CUBE, G_CYL, G_CONE, G_TORUS, G_OBJ };

struct ObjectInfo{
    const char* name;
    const char* bmpFile;
    GeomType    gtype;
    char        key;          // unused now (kept for Sidebar spirit)
    GLuint      texObject=0;
    GLuint      displayList=0;
    float       orbitR=0.f;
    float       orbitSpd=0.f;
    float       selfSpd=0.f;
    float       scale=1.f;
    float       wx=0.f, wy=0.f, wz=0.f;
    float       camLocal[3] = {0.f,0.f,3.f};
    GLuint      objSourceDL=0;
    const char* objFile=nullptr;
};

static ObjectInfo Objects[] = {
    { "Mercury","mercury.bmp", G_SPHERE,'0' },
    { "Venus",  "venus.bmp",   G_TORUS, '1' },
    { "Earth",  "earth.bmp",   G_CUBE,  '2' },
    { "Mars",   "mars.bmp",    G_CYL,   '3' },
    { "Moon",   "moon.bmp",    G_CONE,  '4' },
    { "Jupiter","jupiter.bmp", G_OBJ,   '5', 0,0, 7.0f,0.25f,0.8f,1.3f,0,0,0,{0,0,5},0,"dog.obj"   },
    { "Saturn", "saturn.bmp",  G_OBJ,   '6', 0,0, 9.0f,0.20f,0.6f,1.3f,0,0,0,{0,0,6},0,"ducky.obj" },
    { "Uranus", "uranus.bmp",  G_OBJ,   '7', 0,0,11.0f,0.16f,0.5f,1.2f,0,0,0,{0,0,6},0,"dino.obj"  },
    { "Neptune","neptune.bmp", G_SPHERE,'8' },
    { "Pluto",  "pluto.bmp",   G_SPHERE,'9' },
};
static const int NUMOBJECTS = (int)(sizeof(Objects)/sizeof(Objects[0]));

// material presets (cycled by 'l'); I use SetMaterial(r,g,b,shininess).
struct MatPreset{ float dif[3]; float shininess; };
static MatPreset gMats[] = {
    {{0.8f,0.8f,0.8f}, 32.f},
    {{0.9f,0.7f,0.5f}, 64.f},
    {{0.55f,0.65f,0.8f},96.f},
    {{0.7f,0.7f,0.7f}, 8.f}
};
static const int NUM_MATS = (int)(sizeof(gMats)/sizeof(gMats[0]));

// prototypes
void InitGraphics();
void InitLists();
void Reset();
void Animate();
void Display();
void Resize(int,int);
void MouseButton(int,int,int,int);
void MouseMotion(int,int);
void Keyboard(unsigned char,int,int);
void Visibility(int);

// utils
static void SphericalToXYZ(float r,float az,float el,float* x,float* y,float* z){
    float ca=cosf(DEG2RAD(az)), sa=sinf(DEG2RAD(az));
    float ce=cosf(DEG2RAD(el)), se=sinf(DEG2RAD(el));
    *x = r*ce*ca; *y = r*se; *z = r*ce*sa;
}
static void LoadTextureBMP(const char* file, GLuint* tex){
    int w=0,h=0;
    unsigned char* ptr = BmpToTexture((char*)file,&w,&h);
    if(!ptr){ fprintf(stderr,"Cannot open texture '%s'\n",file); *tex=0; return; }
    glGenTextures(1,tex);
    glBindTexture(GL_TEXTURE_2D,*tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,w,h,0,GL_RGB,GL_UNSIGNED_BYTE,ptr);
}

static void PrimeNonObjDefaults(){
    for(int i=0;i<NUMOBJECTS;i++){
        if(Objects[i].gtype!=G_OBJ){
            float baseR = 3.f + 1.f*i;
            Objects[i].orbitR   = baseR;
            Objects[i].orbitSpd = 0.35f - 0.02f*i;
            Objects[i].selfSpd  = 0.8f  + 0.05f*i;
            Objects[i].scale    = (Objects[i].gtype==G_SPHERE)?1.f:1.1f;
            Objects[i].camLocal[0]=0.f;
            Objects[i].camLocal[1]=0.3f*Objects[i].scale;
            Objects[i].camLocal[2]=3.f*Objects[i].scale+2.f;
        }
    }
}

static void ApplyMaterial(int idx){
    idx = (idx%NUM_MATS+NUM_MATS)%NUM_MATS;
    SetMaterial(gMats[idx].dif[0], gMats[idx].dif[1], gMats[idx].dif[2], gMats[idx].shininess);
}

// ---------------------------------------------------------------------
int main(int argc,char* argv[]){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(WindowWidth,WindowHeight);
    MainWindow = glutCreateWindow("CS450/550 P5 - Texture Mapping (10 Objects)");
    glutSetWindow(MainWindow);

    glutReshapeFunc(Resize);
    glutDisplayFunc(Display);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(MouseButton);
    glutMotionFunc(MouseMotion);
    glutVisibilityFunc(Visibility);
    glutIdleFunc(Animate);

    InitGraphics();
    InitLists();
    Reset();

    glutMainLoop();
    return 0;
}

// ---------------------------------------------------------------------
void InitGraphics(){
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);

    PrimeNonObjDefaults();

    for(int i=0;i<NUMOBJECTS;i++) LoadTextureBMP(Objects[i].bmpFile,&Objects[i].texObject);
    for(int i=0;i<NUMOBJECTS;i++){
        if(Objects[i].gtype==G_OBJ && Objects[i].objFile && Objects[i].objSourceDL==0){
            Objects[i].objSourceDL = LoadObjMtlFiles((char*)Objects[i].objFile);
        }
    }
}

void InitLists(){
    for(int i=0;i<NUMOBJECTS;i++){
        Objects[i].displayList = glGenLists(1);
        glNewList(Objects[i].displayList, GL_COMPILE);
            switch(Objects[i].gtype){
                case G_SPHERE: OsuSphere(1.f,64,64); break;
                case G_CUBE:   OsuCube(1.f); break;
                case G_CYL:    OsuCylinder(0.5f,1.0f,64,32); break;
                case G_CONE:   OsuCone(1.0f,0.2f,1.0f,64,32); break;
                case G_TORUS:  OsuTorus(0.25f,1.0f,96,96); break;
                case G_OBJ:
                    if(Objects[i].objSourceDL) glCallList(Objects[i].objSourceDL);
                    else OsuSphere(1.f,48,32);
                    break;
            }
        glEndList();
    }
}

void Reset(){
    UseTexture   = true;
    ObjMotionOn  = true;
    LightMotionOn= true;
    LightType    = 0;
    MatPaletteIdx= 0;
    NowObject    = 0;
    LookSlot     = -1;

    CamDist = 12.f; CamAzim = 45.f; CamElev = 20.f;
    ViewBiasYaw = 0.f; ViewBiasPitch = 0.f;
}

void Animate(){
    int ms = glutGet(GLUT_ELAPSED_TIME);
    ms %= MS_PER_CYCLE;
    TimeFrac = (float)ms / (float)MS_PER_CYCLE;
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void Display(){
    glutSetWindow(MainWindow);
    glDrawBuffer(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glViewport(0,0,WindowWidth,WindowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0,(float)WindowWidth/(float)WindowHeight,0.1,200.0);

    // update world positions
    for(int i=0;i<NUMOBJECTS;i++){
        float thetaOrbit = ObjMotionOn ? (2.f*M_PI*Objects[i].orbitSpd*TimeFrac) : 0.f;
        Objects[i].wx = Objects[i].orbitR * cosf(thetaOrbit);
        Objects[i].wz = Objects[i].orbitR * sinf(thetaOrbit);
        Objects[i].wy = (i%2)?0.4f:0.f;
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if(LookSlot<0){
        float cx,cy,cz; SphericalToXYZ(CamDist,CamAzim,CamElev,&cx,&cy,&cz);
        gluLookAt(cx,cy,cz, 0,0,0, 0,1,0);
    }else if(LookSlot<NUMOBJECTS){
        const ObjectInfo& o = Objects[LookSlot];
        float cx = o.wx + o.camLocal[0];
        float cy = o.wy + o.camLocal[1];
        float cz = o.wz + o.camLocal[2];

        float dirx = o.wx - cx, diry = o.wy - cy, dirz = o.wz - cz;
        float r = sqrtf(dirx*dirx+diry*diry+dirz*dirz);
        float az = RAD2DEG(atan2f(dirz,dirx));
        float el = RAD2DEG(asinf((r>1e-5f)?(diry/r):0.f));
        az += ViewBiasYaw; el = Clamp(el+ViewBiasPitch,-89.f,89.f);

        float tx,ty,tz; SphericalToXYZ(r,az,el,&tx,&ty,&tz); tx+=cx; ty+=cy; tz+=cz;
        gluLookAt(cx,cy,cz,  tx,ty,tz,  0,1,0);
    }else{
        gluLookAt(0,18,40,  0,0,0,  0,1,0);
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    float Ltheta = LightMotionOn ? (2.f*M_PI*TimeFrac) : 0.f;
    float Lx = 15.f*cosf(Ltheta), Ly = 10.f + 2.f*sinf(2.f*Ltheta), Lz = 15.f*sinf(Ltheta);

    if(LightType==0){
        SetPointLight(GL_LIGHT0, Lx,Ly,Lz, 1.f,1.f,1.f);
    }else{
        const ObjectInfo& t = Objects[NowObject];
        SetSpotLight(GL_LIGHT0, Lx,Ly,Lz, t.wx-Lx,t.wy-Ly,t.wz-Lz, 1.f,1.f,1.f);
    }

    const ObjectInfo& cur = Objects[NowObject];
    float thetaSpin = ObjMotionOn ? (2.f*M_PI*cur.selfSpd*TimeFrac) : 0.f;

    // textures per the prof's steps: enable→bind→env→draw→disable
    if(UseTexture){
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, cur.texObject);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }else{
        glDisable(GL_TEXTURE_2D);
    }

    ApplyMaterial(MatPaletteIdx);

    glPushMatrix();
        glTranslatef(cur.wx,cur.wy,cur.wz);
        glRotatef(RAD2DEG(thetaSpin),0,1,0);
        glScalef(cur.scale,cur.scale,cur.scale);
        glCallList(cur.displayList);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glBegin(GL_LINES);
        glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(2,0,0);
        glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,2,0);
        glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,2);
    glEnd();

    glutSwapBuffers();
    glFlush();
}

void Resize(int w,int h){
    WindowWidth = (w>1?w:1);
    WindowHeight= (h>1?h:1);
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void Keyboard(unsigned char c,int x,int y){
    (void)x; (void)y;

    switch(c){
        case 't': case 'T': UseTexture=!UseTexture; break;
        case 'l': case 'L': MatPaletteIdx = (MatPaletteIdx+1)%NUM_MATS; break;
        case 'k': case 'K': LightType = (LightType+1)%2; break; // Point↔Spot
        case ';':           ObjMotionOn=!ObjMotionOn; break;
        case '\'':          LightMotionOn=!LightMotionOn; break;

        // cycle LookAt slots only; do not reuse number keys
        case '1':
            if(LookSlot<0) LookSlot=0;
            else { LookSlot++; if(LookSlot>NUMOBJECTS) LookSlot=-1; }
            break;

        // object rotation (cycle current selection)
        case ',': NowObject = (NowObject-1+NUMOBJECTS)%NUMOBJECTS; break;
        case '.': NowObject = (NowObject+1)%NUMOBJECTS; break;

        case 'r': case 'R': Reset(); break;
        case 27: case 'q': case 'Q': exit(0); break;
    }
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void MouseButton(int button,int state,int x,int y){
    int b=0;
    if(button==GLUT_LEFT_BUTTON) b=1;
    if(button==GLUT_MIDDLE_BUTTON) b=2;
    if(button==GLUT_RIGHT_BUTTON) b=4;
    if(state==GLUT_DOWN){ ActiveButton|=b; Xmouse=x; Ymouse=y; }
    else{ ActiveButton&=~b; }
}

void MouseMotion(int x,int y){
    int dx=x-Xmouse, dy=y-Ymouse;
    if(LookSlot<0){
        if(ActiveButton&1){ CamAzim+=0.5f*dx; CamElev+=0.5f*dy; CamElev=Clamp(CamElev,-89.f,89.f); }
        if(ActiveButton&4){ CamDist*=(1.f-0.003f*dy); CamDist=Clamp(CamDist,3.f,80.f); }
    }else{
        if(ActiveButton&1){ ViewBiasYaw+=0.4f*dx; ViewBiasPitch+=0.4f*dy; ViewBiasPitch=Clamp(ViewBiasPitch,-45.f,45.f); }
        if(ActiveButton&4){
            int s = (LookSlot>=0 && LookSlot<NUMOBJECTS)? LookSlot : NowObject;
            float& cz = Objects[s].camLocal[2];
            cz*=(1.f-0.003f*dy); cz=Clamp(cz,1.f,30.f);
        }
    }
    Xmouse=x; Ymouse=y;
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void Visibility(int state){
    glutIdleFunc(state==GLUT_VISIBLE? Animate: nullptr);
}

// ---------------------------------------------------------------------
// Provide the Unit() definition expected by loadobjmtlfiles.h and others.
// Returns vector length; writes the normalized vector to out.
// ---------------------------------------------------------------------
float Unit( float v[3], float out[3] ){
    float d = sqrtf(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    if(d > 0.f){ out[0]=v[0]/d; out[1]=v[1]/d; out[2]=v[2]/d; }
    else{ out[0]=out[1]=out[2]=0.f; }
    return d;
}
