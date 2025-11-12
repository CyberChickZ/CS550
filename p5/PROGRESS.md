# Project Progress Log

## Overview
Course: CS 450/550 — Fall 2025, Project #5 (Texture Mapping). Entry point is `sample.cpp`, which compiles every Osu* helper plus the BMP/OBJ loaders directly by inclusion. This log captures my steps and decisions while evolving the Project 4 base into the Project 5 deliverable.

## Baseline (Project 4 Initialization)
- Cloned the P4 codebase and confirmed the existing orbit/animation controls using `make sample && ./sample`.
- Documented keyboard bindings already supported (object cycling, toggles, camera slots) so new texture/light features align with prior UX expectations.
- Captured the original assignment brief (object list, texture requirements, GL_MODULATE lighting behavior, moving light) inside this file for quick reference.

## Texture-Mapping Tasks Completed
1. Verified that every Osu* module and the OBJ loaders remain included in `sample.cpp`; noted optional helpers (`keytime.cpp`, `glslprogram.cpp`) for potential reactivation.
2. Audited OBJ/BMP assets in the repo root (e.g., `dino.obj`, `venus.bmp`) and mapped each to the `Objects[]` table to guarantee one-to-one texture coverage across slots 0–9.
3. Established constants such as `MS_PER_CYCLE` and the `Animate()` time fraction to drive light and orbit motion, matching the assignment guidance.
4. Reviewed the `Objects[]` struct fields (texture IDs, display lists, camera offsets) to ensure hooks exist for: texture toggling (`t`), light mode cycling (`'`), object selection (`,` `.`), and LookSlot camera locking.

## Pending / Next Steps
- Implement or confirm the GL texture creation path per the assignment summary (BmpToTexture usage, `glTexParameteri`, per-object display lists).
- Demonstrate both "no texture" and "texture modulated" render paths with keyboard toggles, ensuring lighting remains dynamic.
- Produce the Canvas deliverables: updated `sample.cpp`, PDF write-up (with screenshot + description of texture steps), and narrated demo video link.

Keep this log updated whenever new functionality or documentation is added so grading artifacts can reference a single audit trail.
