/*
    CS 450/550 -- Fall 2025
    Project #6: Shaders, I (pig-in-the-python on snakeH.obj)
    Author:        Haochuan Zhang   <zhanhaoc@oregonstate.edu>
    Course:        OSU CS 550 — Fall 2025, Project #6: Shaders
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
#include "glslprogram.cpp"

// ---------------------------------------------------------------------
#define DEG2RAD(d)   ((d)*(M_PI/180.f))
#define RAD2DEG(r)   ((r)*(180.f/M_PI))

static inline float Clamp(float v,float lo,float hi){ return v<lo?lo:(v>hi?hi:v); }

// window
int   MainWindow = 0;
int   WindowWidth = 1024, WindowHeight = 768;

// time / toggles
float GlobalTime = 0.f;
bool  UseTexture = false;    // 't' (default off for solid color)
bool  ObjMotionOn = false;   // ';'
bool  LightMotionOn = false; // '''
int   LightType = 0;         // 0=Point, 1=Spot  ('k')
bool  LightBeamOn = false;   // 'b'
int   MatPaletteIdx = 0;     // 'l'
int   LightColorIdx = 0;     // 'c'
int   NowObject = 0;         // 0..9, cycled by ',' '.'
int   LookSlot = -1;         // -1=free, 0..9 objects, 10=FarView  ('1')

// free camera
float CamDist = 12.f, CamAzim = 45.f, CamElev = 20.f;
int   ActiveButton = 0, Xmouse=0, Ymouse=0;

// bias while LookAt-locked
float ViewBiasYaw = 0.f, ViewBiasPitch = 0.f;

// shader program (P6)
static GLSLProgram PigProgram;
static bool PigProgramReady = false;
// 鼠标/键盘以外的动画参数：鼓包沿 X 周期移动（5s 一圈），高度随正弦起伏，可切换方向/动画
static float PigTime01 = 0.f;   // [0,1) cyclic time for P6 effects
static float PigD = 0.f;        // bulge center x
static float PigH = 0.f;        // bulge height scale
static bool  PigForward = true; // tail->head when true
static bool  PigHeightAnim = true; // animate height
static float PigWidth = 8.f;    // 鼓包宽度，可统一传给 shader

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
    float       orbitMinor=0.f;
    float       orbitPhase=0.f;
    float       orbitCenter[3] = {0.f,0.f,0.f};
    float       orbitTilt=0.f;
    float       orbitIncline=0.f;     // degrees; tilt orbit plane around X
    float       orbitAscending=0.f;   // degrees; rotate plane around Y
    float       selfSpd=0.f;
    float       selfPhase=0.f;
    float       spinAxis[3] = {0.f,1.f,0.f};
    float       scale=1.f;
    float       wx=0.f, wy=0.f, wz=0.f;
    float       camLocal[3] = {0.f,0.f,3.f};
    GLuint      objSourceDL=0;
    const char* objFile=nullptr;
};

static ObjectInfo Objects[] = {
    // Treat the snake as the primary object (index 0) so defaults leave it at the origin.
    { "Snake", "earth.bmp", G_OBJ, '0', 0,0,
      0.0f,0.0f, 0.0f,0.0f,{0.f,0.f,0.f},0.0f,
      0.0f, 0.0f,
      0.5f,0.0f,{0.f,1.f,0.f},
      1.0f, 0,0,0,
      {0.f,0.5f,20.0f},
      0, "snakeH.obj" },
};
static const int NUMOBJECTS = (int)(sizeof(Objects)/sizeof(Objects[0]));
static const int SUN_INDEX = 0;

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

static float Dot3(const float a[3], const float b[3]){
    return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}

static void Cross3(const float a[3], const float b[3], float out[3]){
    out[0] = a[1]*b[2] - a[2]*b[1];
    out[1] = a[2]*b[0] - a[0]*b[2];
    out[2] = a[0]*b[1] - a[1]*b[0];
}

static void Normalize3(float v[3]){
    float len = sqrtf(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    if(len < 1e-5f){ v[0]=0.f; v[1]=1.f; v[2]=0.f; return; }
    v[0]/=len; v[1]/=len; v[2]/=len;
}

static void BuildPlaneBasis(const float normal[3], float u[3], float v[3]){
    float n[3] = {normal[0],normal[1],normal[2]};
    Normalize3(n);
    float ref[3] = {0.f,1.f,0.f};
    if(fabsf(Dot3(n,ref)) > 0.9f){
        ref[0]=1.f; ref[1]=0.f; ref[2]=0.f;
    }
    Cross3(ref,n,u);
    Normalize3(u);
    Cross3(n,u,v);
    Normalize3(v);
}

static void PrimeNonObjDefaults(){
    for(int i=0;i<NUMOBJECTS;i++){
        const bool isSun = (i==SUN_INDEX);
        if(Objects[i].orbitR==0.f && !isSun){
            float baseR = 3.f + 1.f*i;
            Objects[i].orbitR = baseR;
        }
        if(Objects[i].orbitMinor==0.f && !isSun){
            Objects[i].orbitMinor = 0.8f*Objects[i].orbitR;
        }
        if(Objects[i].orbitSpd==0.f && !isSun){
            Objects[i].orbitSpd = 0.20f - 0.015f*i;
        }
        if(Objects[i].orbitTilt==0.f && !isSun){
            Objects[i].orbitTilt = 0.15f;
        }
        if(Objects[i].orbitIncline==0.f && !isSun){
            Objects[i].orbitIncline = 1.5f * i;
        }
        if(Objects[i].orbitAscending==0.f && !isSun){
            Objects[i].orbitAscending = 12.f * i;
        }
        if(Objects[i].selfSpd==0.f){
            Objects[i].selfSpd  = 0.8f  + 0.05f*i;
        }
        if(Objects[i].scale==0.f){
            Objects[i].scale    = (Objects[i].gtype==G_SPHERE)?1.f:1.1f;
        }
        Normalize3(Objects[i].spinAxis);
        if(fabsf(Objects[i].camLocal[2])<1e-4f){
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

static const GLfloat SUN_EMISSION[4]    = {1.4f,1.2f,0.5f,1.f};
static const GLfloat LIGHT_EMISSION[4]  = {1.3f,1.3f,1.1f,1.f};
static const GLfloat NO_EMISSION[4]     = {0.f,0.f,0.f,1.f};
static const float   LIGHT_INTENSITY    = 1.4f;
static const float   ORBIT_SPEED_SCALE = 0.25f;
static const float   SPIN_SPEED_SCALE  = 0.25f;

struct LightColorPreset{
    float rgb[3];
};
static LightColorPreset gLightColors[] = {
    {{1.f,1.f,1.f}},
    {{1.0f,0.85f,0.5f}},
    {{0.6f,0.8f,1.0f}},
    {{1.0f,0.6f,0.7f}}
};
static const int NUM_LIGHT_COLORS = (int)(sizeof(gLightColors)/sizeof(gLightColors[0]));
static const char* kLightColorNames[] = { "White", "Warm", "Cool", "Rose" };

static void ResetOrbitCamera(){
    LookSlot = -1;
    CamDist = 24.f;
    CamAzim = 45.f;
    CamElev = 22.f;
    ViewBiasYaw = 0.f;
    ViewBiasPitch = 0.f;
}

static void BakeDisplayListForObject(int idx){
    if(idx<0 || idx>=NUMOBJECTS) return;
    glNewList(Objects[idx].displayList, GL_COMPILE);
    switch(Objects[idx].gtype){
        case G_SPHERE: OsuSphere(1.f,64,64); break;
        case G_CUBE:   OsuCube(1.f); break;
        case G_CYL:    OsuCylinder(0.5f,1.0f,64,32); break;
        case G_CONE:   OsuCone(1.0f,0.2f,1.0f,64,32); break;
        case G_TORUS:  OsuTorus(0.25f,1.0f,96,96); break;
        case G_OBJ:
            if(Objects[idx].objSourceDL) glCallList(Objects[idx].objSourceDL);
            else OsuSphere(1.f,48,32);
            break;
    }
    glEndList();
}

static const GeomType kShapeCycle[] = { G_SPHERE, G_CUBE, G_CYL, G_CONE, G_TORUS };
static const int NUM_SHAPE_CYCLE = (int)(sizeof(kShapeCycle)/sizeof(kShapeCycle[0]));

static const char* GeomTypeName(GeomType g){
    switch(g){
        case G_SPHERE: return "Sphere";
        case G_CUBE:   return "Cube";
        case G_CYL:    return "Cylinder";
        case G_CONE:   return "Cone";
        case G_TORUS:  return "Torus";
        case G_OBJ:    return "OBJ";
    }
    return "?";
}

static int CurrentMutableIndex(){
    if(LookSlot>=0 && LookSlot<NUMOBJECTS) return LookSlot;
    return NowObject;
}

static void AdjustCurrentShape(int step){
    if(NUM_SHAPE_CYCLE == 0) return;
    int idx = CurrentMutableIndex();
    if(idx<0 || idx>=NUMOBJECTS) return;
    if(idx==SUN_INDEX) return;
    GeomType cur = Objects[idx].gtype;
    int curIdx = 0;
    bool found=false;
    for(int i=0;i<NUM_SHAPE_CYCLE;i++){
        if(kShapeCycle[i]==cur){ curIdx=i; found=true; break; }
    }
    if(!found) curIdx=0;
    int nextIdx = (curIdx + step)%NUM_SHAPE_CYCLE;
    if(nextIdx<0) nextIdx += NUM_SHAPE_CYCLE;
    Objects[idx].gtype = kShapeCycle[nextIdx];
    glutSetWindow(MainWindow);
    BakeDisplayListForObject(idx);
}

static void AdjustViewZoom(float delta){
    if(LookSlot>=0 && LookSlot<NUMOBJECTS){
        float& cz = Objects[LookSlot].camLocal[2];
        cz = Clamp(cz + delta, 1.f, 50.f);
    }else{
        CamDist = Clamp(CamDist + delta, 3.f, 150.f);
    }
}

static float DrawSegment(float x,float y,const char* text,float r,float g,float b){
    if(!text) return x;
    glColor3f(r,g,b);
    glRasterPos2f(x,y);
    int width = 0;
    for(const char* c=text; *c; ++c){
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
        width += glutBitmapWidth(GLUT_BITMAP_8_BY_13, *c);
    }
    return x + (float)width;
}

static void DrawOverlay(){
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0,WindowWidth,0,WindowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    GLboolean depth = glIsEnabled(GL_DEPTH_TEST);
    GLboolean texture = glIsEnabled(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glColor3f(1.f,1.f,1.f);

    int activeIdx = CurrentMutableIndex();
    activeIdx = (activeIdx<0)?0:activeIdx;
    if(activeIdx>=NUMOBJECTS) activeIdx = NUMOBJECTS-1;
    const ObjectInfo& active = Objects[activeIdx];
    const char* geomName = GeomTypeName(active.gtype);
    const char* colorName = kLightColorNames[(LightColorIdx%NUM_LIGHT_COLORS+NUM_LIGHT_COLORS)%NUM_LIGHT_COLORS];
    float zoomVal = (LookSlot<0) ? CamDist :
        ((LookSlot>=0 && LookSlot<NUMOBJECTS)? Objects[LookSlot].camLocal[2] : CamDist);
    char buf[64];
    float labelCol[3] = {0.85f,0.85f,0.85f};
    float objCol[3]   = {1.0f,0.9f,0.4f};
    float shapeCol[3] = {0.6f,0.9f,1.0f};
    float matCol[3]   = {0.9f,0.6f,0.5f};
    float colorCol[3] = {1.0f,0.7f,0.8f};
    float zoomCol[3]  = {0.7f,1.0f,0.7f};
    float onCol[3]    = {0.5f,1.0f,0.5f};
    float offCol[3]   = {1.0f,0.4f,0.4f};

    float y = WindowHeight - 15.f;
    float cursor = 10.f;
    cursor = DrawSegment(cursor,y,"Object (1-0): ",labelCol[0],labelCol[1],labelCol[2]);
    cursor = DrawSegment(cursor,y,active.name,objCol[0],objCol[1],objCol[2]);
    cursor = DrawSegment(cursor,y," | Shape (,/.): ",labelCol[0],labelCol[1],labelCol[2]);
    DrawSegment(cursor,y,geomName,shapeCol[0],shapeCol[1],shapeCol[2]);

    y -= 15.f; cursor = 10.f;
    cursor = DrawSegment(cursor,y,"Material (l/L): ",labelCol[0],labelCol[1],labelCol[2]);
    snprintf(buf,sizeof(buf),"%d",MatPaletteIdx);
    cursor = DrawSegment(cursor,y,buf,matCol[0],matCol[1],matCol[2]);
    cursor = DrawSegment(cursor,y," | Light Color (c/C): ",labelCol[0],labelCol[1],labelCol[2]);
    cursor = DrawSegment(cursor,y,colorName,colorCol[0],colorCol[1],colorCol[2]);
    cursor = DrawSegment(cursor,y," | View Zoom (+/-): ",labelCol[0],labelCol[1],labelCol[2]);
    snprintf(buf,sizeof(buf),"%.2f",zoomVal);
    DrawSegment(cursor,y,buf,zoomCol[0],zoomCol[1],zoomCol[2]);

    y -= 15.f; cursor = 10.f;
    cursor = DrawSegment(cursor,y,"Light Mode (k): ",labelCol[0],labelCol[1],labelCol[2]);
    cursor = DrawSegment(cursor,y,(LightType==1)?"SPOT":"POINT",shapeCol[0],shapeCol[1],shapeCol[2]);
    cursor = DrawSegment(cursor,y," | Light Motion ('): ",labelCol[0],labelCol[1],labelCol[2]);
    cursor = DrawSegment(cursor,y,LightMotionOn?"ON":"OFF",
        LightMotionOn?onCol[0]:offCol[0],
        LightMotionOn?onCol[1]:offCol[1],
        LightMotionOn?onCol[2]:offCol[2]);
    cursor = DrawSegment(cursor,y," | Light Beam (b): ",labelCol[0],labelCol[1],labelCol[2]);
    cursor = DrawSegment(cursor,y,LightBeamOn?"ON":"OFF",
        LightBeamOn?onCol[0]:offCol[0],
        LightBeamOn?onCol[1]:offCol[1],
        LightBeamOn?onCol[2]:offCol[2]);
    cursor = DrawSegment(cursor,y," | Object Motion (;/space): ",labelCol[0],labelCol[1],labelCol[2]);
    DrawSegment(cursor,y,ObjMotionOn?"ON":"OFF",
        ObjMotionOn?onCol[0]:offCol[0],
        ObjMotionOn?onCol[1]:offCol[1],
        ObjMotionOn?onCol[2]:offCol[2]);

    y -= 15.f; cursor = 10.f;
    cursor = DrawSegment(cursor,y,"Pig Dir (p): ",labelCol[0],labelCol[1],labelCol[2]);
    cursor = DrawSegment(cursor,y,PigForward?"Tail->Head":"Head->Tail",
        shapeCol[0],shapeCol[1],shapeCol[2]);
    cursor = DrawSegment(cursor,y," | Pig Height (h): ",labelCol[0],labelCol[1],labelCol[2]);
    DrawSegment(cursor,y,PigHeightAnim?"Anim":"Static",
        PigHeightAnim?onCol[0]:offCol[0],
        PigHeightAnim?onCol[1]:offCol[1],
        PigHeightAnim?onCol[2]:offCol[2]);

    if(lighting) glEnable(GL_LIGHTING); else glDisable(GL_LIGHTING);
    if(texture)  glEnable(GL_TEXTURE_2D); else glDisable(GL_TEXTURE_2D);
    if(depth)    glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

static void DrawLightMarker(const float pos[3], bool drawBeam, const float target[3]){
    glPushMatrix();
    glTranslatef(pos[0],pos[1],pos[2]);
    OsuSphere(0.4f,32,16);
    if(drawBeam && target){
        float dir[3] = {target[0]-pos[0], target[1]-pos[1], target[2]-pos[2]};
        Normalize3(dir);
        float beamLen = 30.f;
        float tip[3] = {dir[0]*beamLen, dir[1]*beamLen, dir[2]*beamLen};
        float helper[3] = {0.f,1.f,0.f};
        if(fabsf(Dot3(dir,helper))>0.9f){
            helper[0]=1.f; helper[1]=0.f; helper[2]=0.f;
        }
        float side[3]; Cross3(dir,helper,side); Normalize3(side);
        float up2[3]; Cross3(dir,side,up2); Normalize3(up2);
        float base = 0.8f;
        glBegin(GL_LINES);
            glVertex3f(0.f,0.f,0.f);
            glVertex3f(tip[0],tip[1],tip[2]);
            float basePos[3] = {tip[0]-dir[0], tip[1]-dir[1], tip[2]-dir[2]};
            glVertex3f(basePos[0],basePos[1],basePos[2]);
            glVertex3f(basePos[0]+side[0]*base, basePos[1]+side[1]*base, basePos[2]+side[2]*base);
            glVertex3f(basePos[0],basePos[1],basePos[2]);
            glVertex3f(basePos[0]-side[0]*base, basePos[1]-side[1]*base, basePos[2]-side[2]*base);
            glVertex3f(basePos[0],basePos[1],basePos[2]);
            glVertex3f(basePos[0]+up2[0]*base, basePos[1]+up2[1]*base, basePos[2]+up2[2]*base);
            glVertex3f(basePos[0],basePos[1],basePos[2]);
            glVertex3f(basePos[0]-up2[0]*base, basePos[1]-up2[1]*base, basePos[2]-up2[2]*base);
        glEnd();
    }
    glPopMatrix();
}

// ---------------------------------------------------------------------
int main(int argc,char* argv[]){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(WindowWidth,WindowHeight);
    MainWindow = glutCreateWindow("CS450/550 P6 - Shaders (Snake)");
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

    PigProgram.Init(); // 初始化扩展查询，确保 Mac 旧 GL 也能创建 shader
    // 尝试加载 P6 shader（顶点鼓包 + 片段光照），失败则回退固定功能
    PigProgramReady = PigProgram.Create((char*)"PigInPython.vert", (char*)"PigInPython.frag");
    PigProgram.SetVerbose(false);

    for(int i=0;i<NUMOBJECTS;i++) LoadTextureBMP(Objects[i].bmpFile,&Objects[i].texObject);
    for(int i=0;i<NUMOBJECTS;i++){
        if(Objects[i].gtype==G_OBJ && Objects[i].objFile && Objects[i].objSourceDL==0){
            Objects[i].objSourceDL = LoadObjMtlFiles((char*)Objects[i].objFile);
        }
    }
    if(!PigProgramReady){
        fprintf(stderr,"Failed to create PigInPython shader program.\n");
    }
}

void InitLists(){
    for(int i=0;i<NUMOBJECTS;i++){
        Objects[i].displayList = glGenLists(1);
        BakeDisplayListForObject(i);
    }
}

void Reset(){
    UseTexture   = false;
    ObjMotionOn  = false;
    LightMotionOn= false;
    LightType    = 0;
    MatPaletteIdx= 0;
    LightColorIdx= 0;
    NowObject    = (NUMOBJECTS>1)?1:0;

    ResetOrbitCamera();
}

void Animate(){
    int ms = glutGet(GLUT_ELAPSED_TIME);
    GlobalTime = 0.001f * (float)ms;

    // P6 Time in [0,1) using a fixed cycle (ms wraps each cycle).
    const int MS_PER_CYCLE = 5000; // faster bulge travel（5秒一圈）
    int cyc = ms % MS_PER_CYCLE;
    PigTime01 = (float)cyc / (float)MS_PER_CYCLE;

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

    // update world positions with elliptical orbits
    for(int i=0;i<NUMOBJECTS;i++){
        ObjectInfo& obj = Objects[i];
        float major = obj.orbitR;
        float minor = (obj.orbitMinor>0.f)? obj.orbitMinor : major;
        float angle = obj.orbitPhase;
        if(ObjMotionOn){
            angle += obj.orbitSpd * GlobalTime * ORBIT_SPEED_SCALE;
        }
        float localX = major * cosf(angle);
        float localZ = minor * sinf(angle);
        float localY = 0.f;
        float asc = DEG2RAD(obj.orbitAscending);
        float ca = cosf(asc), sa = sinf(asc);
        float rotX = localX*ca - localZ*sa;
        float rotZ = localX*sa + localZ*ca;
        float rotY = localY;
        float inc = DEG2RAD(obj.orbitIncline);
        float ci = cosf(inc), si = sinf(inc);
        float tiltY = rotY*ci - rotZ*si;
        float tiltZ = rotY*si + rotZ*ci;
        float cx = obj.orbitCenter[0];
        float cy = obj.orbitCenter[1];
        float cz = obj.orbitCenter[2];
        obj.wx = cx + rotX;
        obj.wz = cz + tiltZ;
        obj.wy = cy + tiltY + obj.orbitTilt * sinf(angle*0.5f);
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

    const ObjectInfo& sun = Objects[SUN_INDEX];
    float lightAngle = LightMotionOn ? (0.95f*GlobalTime) : 0.f;
    // Add a small deterministic wobble so lights don’t circle a perfectly fixed center.
    float wobbleX = 1.5f * sinf(0.37f*GlobalTime + 1.1f);
    float wobbleY = 0.8f * sinf(0.22f*GlobalTime + 0.4f);
    float wobbleZ = 1.8f * cosf(0.51f*GlobalTime + 0.9f);
    float center[3] = {sun.wx + wobbleX, sun.wy + 6.67f + wobbleY, sun.wz + wobbleZ};
    float nPrimary[3] = {0.f, cosf(DEG2RAD(45.f)), sinf(DEG2RAD(45.f))};
    float uPrimary[3], vPrimary[3];
    BuildPlaneBasis(nPrimary,uPrimary,vPrimary);
    float nSecondary[3] = {1.f,0.f,0.f};
    float uSecondary[3], vSecondary[3];
    BuildPlaneBasis(nSecondary,uSecondary,vSecondary);

    float radius0 = 16.f + 1.5f * sinf(0.73f*GlobalTime + 0.2f);
    float radius1 = 20.f + 1.2f * cosf(0.55f*GlobalTime + 1.3f);
    float light0Pos[3] = {
        center[0] + radius0*(cosf(lightAngle)*uPrimary[0] + sinf(lightAngle)*vPrimary[0]),
        center[1] + radius0*(cosf(lightAngle)*uPrimary[1] + sinf(lightAngle)*vPrimary[1]),
        center[2] + radius0*(cosf(lightAngle)*uPrimary[2] + sinf(lightAngle)*vPrimary[2])
    };
    float angle2 = -(LightMotionOn ? (0.75f*GlobalTime) : 0.f);
    float light1Pos[3] = {
        center[0] + radius1*(cosf(angle2)*uSecondary[0] + sinf(angle2)*vSecondary[0]),
        center[1] + radius1*(cosf(angle2)*uSecondary[1] + sinf(angle2)*vSecondary[1]),
        center[2] + radius1*(cosf(angle2)*uSecondary[2] + sinf(angle2)*vSecondary[2])
    };

    const LightColorPreset& lcol = gLightColors[(LightColorIdx%NUM_LIGHT_COLORS+NUM_LIGHT_COLORS)%NUM_LIGHT_COLORS];
    float lr = Clamp(lcol.rgb[0]*LIGHT_INTENSITY, 0.f, 1.5f);
    float lg = Clamp(lcol.rgb[1]*LIGHT_INTENSITY, 0.f, 1.5f);
    float lb = Clamp(lcol.rgb[2]*LIGHT_INTENSITY, 0.f, 1.5f);

    const ObjectInfo& targetObj = Objects[NowObject];
    if(LightType==0){
        SetPointLight(GL_LIGHT0, light0Pos[0],light0Pos[1],light0Pos[2], lr,lg,lb);
        SetPointLight(GL_LIGHT1, light1Pos[0],light1Pos[1],light1Pos[2], lr,lg,lb);
    }else{
        SetSpotLight(GL_LIGHT0, light0Pos[0],light0Pos[1],light0Pos[2],
            targetObj.wx-light0Pos[0], targetObj.wy-light0Pos[1], targetObj.wz-light0Pos[2],
            lr,lg,lb);
        SetSpotLight(GL_LIGHT1, light1Pos[0],light1Pos[1],light1Pos[2],
            targetObj.wx-light1Pos[0], targetObj.wy-light1Pos[1], targetObj.wz-light1Pos[2],
            lr,lg,lb);
    }

    if(UseTexture){
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }else{
        glDisable(GL_TEXTURE_2D);
    }

    ApplyMaterial(MatPaletteIdx);

    if(PigProgramReady){
        PigProgram.Use();
        // 根据 PigTime01 计算鼓包中心位置与高度，传给 shader
        if(PigForward){
            PigD = -13.f + PigTime01 * (9.f + 13.f); // tail -> head
        }else{
            PigD =  9.f - PigTime01 * (9.f + 13.f); // head -> tail
        }
        // bulge height modulation using sine; keep non-negative
        const float A = 1.2f;
        const float F = 1.0f;
        if(PigHeightAnim){
            float s = A * sinf(F * (2.f*M_PI * PigTime01));
            PigH = 1.0f + 0.8f * s; // range approx [0.2, 1.8]
        }else{
            PigH = 1.0f; // static mid height
        }

        PigProgram.SetUniformVariable((char*)"uPigD", PigD);
        PigProgram.SetUniformVariable((char*)"uPigH", PigH);
        PigProgram.SetUniformVariable((char*)"uPigW", PigWidth);
    }

    for(int i=0;i<NUMOBJECTS;i++){
        const ObjectInfo& obj = Objects[i];
        float thetaSpin = ObjMotionOn ? (obj.selfSpd*GlobalTime*SPIN_SPEED_SCALE + obj.selfPhase) : obj.selfPhase;
        if(UseTexture){
            glBindTexture(GL_TEXTURE_2D, obj.texObject);
        }
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, (i==SUN_INDEX && UseTexture)?SUN_EMISSION:NO_EMISSION);
        glPushMatrix();
            glTranslatef(obj.wx,obj.wy,obj.wz);
            glRotatef(RAD2DEG(thetaSpin), obj.spinAxis[0], obj.spinAxis[1], obj.spinAxis[2]);
            glScalef(obj.scale,obj.scale,obj.scale);
            glCallList(obj.displayList);
        glPopMatrix();
    }
    if(PigProgramReady){
        PigProgram.UnUse();
    }
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, NO_EMISSION);

    // draw visible markers for both light sources
    glDisable(GL_TEXTURE_2D);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, LIGHT_EMISSION);
    float targetPos[3] = {targetObj.wx,targetObj.wy,targetObj.wz};
    DrawLightMarker(light0Pos, LightBeamOn, targetPos);
    DrawLightMarker(light1Pos, LightBeamOn, targetPos);
    if(UseTexture){
        glEnable(GL_TEXTURE_2D);
    }
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, NO_EMISSION);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glBegin(GL_LINES);
        glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(2,0,0);
        glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,2,0);
        glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,2);
    glEnd();

    DrawOverlay();

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
        case 'b': case 'B': LightBeamOn = !LightBeamOn; break;
        case 'c': case 'C': LightColorIdx = (LightColorIdx+1)%NUM_LIGHT_COLORS; break;
        case ';':           ObjMotionOn=!ObjMotionOn; break;
        case '\'':          LightMotionOn=!LightMotionOn; break;
        // 鼓包方向/高度动画开关
        case 'p': case 'P': PigForward = !PigForward; break;
        case 'h': case 'H': PigHeightAnim = !PigHeightAnim; break;

        // view shortcuts: '1' = lock to snake, '0' = free/orbit view
        case '1':
            LookSlot = 0;
            NowObject = 0;
            break;
        case '0':
            NowObject = 0;
            ResetOrbitCamera();
            break;
        case '~':
        case '`':
            ResetOrbitCamera();
            break;

        case ' ':
            ObjMotionOn = !ObjMotionOn;
            break;

        // shape tweak for current target / zoom controls
        case ',': AdjustCurrentShape(-1); break;
        case '.': AdjustCurrentShape(1); break;
        case '+': case '=': AdjustViewZoom(-0.8f); break;
        case '-': case '_': AdjustViewZoom(0.8f); break;

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
