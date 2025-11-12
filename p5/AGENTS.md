# Repository Guidelines

## Project Structure & Module Organization
`sample.cpp` is the single translation unit that glues together the helper modules (e.g., `osusphere.cpp`, `osucylindercone.cpp`, `setmaterial.cpp`) located in the repo root. Geometry assets live alongside source (`*.obj`, `*.bmp`) so texture and mesh paths remain relative to the executable. Vendor headers (`glm`, `glew.h`, `glut.h`, `freeglut*.h`) are committed to keep builds self-contained; include them via `#include "<file>"` as done in `sample.cpp` to avoid system-path drift.

## Build, Test, and Development Commands
- `make sample` — Compiles `sample.cpp` plus the included helper sources into the `sample` executable using the macOS GLUT toolchain shown in `Makefile`.
- `./sample` — Runs the solar-system demo; pass through runtime keys (see in-app help overlay) to validate new features.
- `make sample CXXFLAGS="-g -Wall"` — (Optional) add diagnostics when debugging; keep the include list identical to `sample.cpp`.

## Coding Style & Naming Conventions
Stick to 4-space indentation, brace-on-new-line for functions, and upper camel case for exported routines (`InitGraphics`, `Animate`). Module-level globals should remain mixed-case (`UseTexture`, `MainWindow`) to match existing state. Prefer `static` helpers in translation units, and keep magic numbers behind `#define`s or `constexpr`. When adding math or GL utilities, colocate them with related modules and document keyboard bindings in-line comments as seen near the control toggles.

## Testing Guidelines
There is no automated harness; rely on running `./sample` to verify render paths, texture loading, and input bindings. When adding geometry or animation, sanity-check on both the default orbit camera and each LookSlot to ensure no NaNs enter the matrices. If you add shader work, log GLSL compile errors to `stderr` and describe manual repro steps in the PR.

## Commit & Pull Request Guidelines
The history favors short, imperative subjects (e.g., `Added zoom with =/- keys`). Keep summaries under ~70 chars and expand details in the body if multiple systems change. For PRs, include: (1) purpose and feature flags touched, (2) screenshots or short screen recordings of the new view, (3) steps to run `make sample && ./sample` for reviewers, and (4) any keyboard additions. Link the corresponding Canvas/issue item when applicable to keep grading context.
