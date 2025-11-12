# Project Progress Log

## Overview
Course: CS 450/550 — Fall 2025, Project #5 (Texture Mapping). Entry point is `sample.cpp`, which compiles every Osu* helper plus the BMP/OBJ loaders directly by inclusion. This log captures my steps and decisions while evolving the Project 4 base into the Project 5 deliverable.

## Baseline (Project 4 Initialization)
- Cloned the P4 codebase and confirmed the existing orbit/animation controls using `make sample && ./sample`.
- Documented keyboard bindings already supported (object cycling, toggles, camera slots) so new texture/light features align with prior UX expectations.
- Captured the original assignment brief (object list, texture requirements, GL_MODULATE lighting behavior, moving light) inside this file for quick reference.

## Texture-Mapping Tasks Completed
**Step 0 — Bootstrapping**
1. Verified that every Osu* module and the OBJ loaders remain included in `sample.cpp`; noted optional helpers (`keytime.cpp`, `glslprogram.cpp`) for potential reactivation.
2. Audited the committed BMP/OBJ assets (e.g., `mercury.bmp`, `ducky.obj`) and confirmed the build pulls everything from the repo root so no external downloads are needed.
3. Established timing helpers (`GlobalTime`, `Animate()`) so future animation can run off continuous seconds, not modulo cycles, matching the assignment guidance.

**Step 1 — Planet Roster & Always-On Rendering**
4. Rebuilt the `Objects[]` table so ten bodies (duck sun + 9 planets) are explicitly defined with textures, orbit radii, scales, per-planet phases, and camera offsets; added a custom `sun.bmp` and mapped ducky to the Sol slot.
5. Updated `Display()` to render every object each frame (rather than only the selected one) so the solar-system layout is always visible while `,`/`.` merely choose the “active” slot for targeting and lighting tweaks.

**Step 2 — Texture & Orbit Mechanics**
6. Introduced per-object elliptical orbits and independent spin rates: `GlobalTime` now drives major/minor axes, orbital tilt, and spin phases for each planet; ducky used emission to stand out initially.
7. Reworked the first moving light (GL_LIGHT0) so it orbits above the sun (center at ducky’s position, +Y bias), travels on a visible circular path, and is represented by a marker sphere to make demos clearer.

**Step 3 — Camera & Palette Controls**
8. Rebound camera hotkeys: digits `1-0` now directly lock the view to specific bodies, while backtick/tilde resets the orbital camera to the default overview without cycling.
9. Added a light-color palette (`c`/`C`) feeding both the GL light and the visible light marker so it’s easy to demonstrate different tint combinations live.

**Step 4 — Runtime Editing Tools**
10. `,`/`.` now mutate the current target’s geometry (Sphere/Cube/Cylinder/Cone/Torus) via display-list rebuilding, while `+`/`-` control the view zoom (either the free-orbit camera distance or the active LookSlot chase distance). The mutable target follows the current LookSlot or last selection, so shape changes happen in-place without reselecting objects.

**Step 5 — Lighting & HUD Polish**
11. Refined the light’s orbit (lower/tilted altitude, faster angular speed) and replaced the spotlight arrow with a long laser/crosshair line so direction is obvious. Light intensity parameters were boosted for better visual impact.
12. Added a top-of-screen HUD with colored values showing Object/Shape/Scale, Material index, Light color, Zoom, and toggle states (Light Mode, Light Motion, Object Motion) alongside their hotkeys.

**Step 6 — Tempo Adjustments**
13. Slowed each planet’s orbit speed and self-spin to ~25% of the previous rates via shared scalars (`ORBIT_SPEED_SCALE`, `SPIN_SPEED_SCALE`), while keeping the light on a faster cycle for contrast.

**Step 7 — True Solar-System Motion**
14. Added per-planet orbital inclines, ascending nodes, and arbitrary spin axes so ellipses no longer share the same plane and rotations tilt realistically; updated the orbit math to rotate ellipses into their inclined planes and spin meshes about arbitrary axes.

**Step 8 — Dual Counter-Rotating Lights**
15. Introduced a second light whose orbit plane is perpendicular to the first; both lights now counter-rotate (one clockwise, one counterclockwise) on larger radii, share the color palette, and render visible emissive markers/laser beams in spotlight mode. The ducky sun’s scale dropped to 80% and only emits when textures are enabled, keeping attention on the moving lights.

## Code Snapshot Highlights
```cpp
// sample.cpp (excerpt)
struct ObjectInfo{
    const char* name;
    const char* bmpFile;
    GeomType    gtype;
    char        key;
    GLuint      texObject=0;
    GLuint      displayList=0;
    float       orbitR=0.f, orbitSpd=0.f, orbitMinor=0.f, orbitPhase=0.f;
    float       orbitCenter[3] = {0.f,0.f,0.f};
    float       orbitTilt=0.f, orbitIncline=0.f, orbitAscending=0.f;
    float       selfSpd=0.f, selfPhase=0.f, spinAxis[3] = {0.f,1.f,0.f};
    float       scale=1.f, wx=0.f, wy=0.f, wz=0.f;
    float       camLocal[3] = {0.f,0.f,3.f};
    GLuint      objSourceDL=0;
    const char* objFile=nullptr;
};

static const float ORBIT_SPEED_SCALE = 0.25f;
static const float SPIN_SPEED_SCALE  = 0.25f;

float GlobalTime = 0.f;

void Animate(){
    int ms = glutGet(GLUT_ELAPSED_TIME);
    GlobalTime = 0.001f * (float)ms;   // keeps time monotonic
    glutPostRedisplay();
}

static void DrawOverlay(){
    // orthographic HUD with colored labels for Object, Shape, Scale, Material, Light Color, Zoom,
    // Light Mode, Motion toggles. Uses DrawSegment() to colorize each value.
}
```

## Pending / Next Steps
- Confirm the GL texture creation path per the assignment summary (BmpToTexture usage, `glTexParameteri`, per-object display lists) and document any deviations.
- Demonstrate both "no texture" and "texture modulated" render paths with keyboard toggles, ensuring lighting remains dynamic.
- Produce the Canvas deliverables: updated `sample.cpp`, PDF write-up (with screenshot + description of texture steps), and narrated demo video link.

Keep this log updated whenever new functionality or documentation is added so grading artifacts can reference a single audit trail.
