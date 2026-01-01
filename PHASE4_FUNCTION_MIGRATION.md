# Phase 4: Function-by-Function Migration Guide

## Overview

This document provides a focused, step-by-step guide for migrating all 20 functions from the old single-timeline API to the new multi-timeline API. Each function follows the same pattern established by `AddTranslationPointAtCamera`.

**IMPORTANT WORKFLOW:** After completing each function migration, STOP and wait for user review before proceeding to the next function. Do NOT automatically continue without explicit user approval.

## CRITICAL: Functional Equivalence Checklist

**Before marking any function as complete, verify ALL of the following:**

1. ✅ **All validation checks preserved**: Every `if (!...)` check from the old function must be in the new function
2. ✅ **All state checks preserved**: Checks like `if (m_isPlaybackRunning)` or `if (m_isRecording)` must be preserved
3. ✅ **All side effects preserved**: Calls to `StopPlayback()`, `log::info()`, etc. must be preserved
4. ✅ **All error handling preserved**: Return values on error conditions must match the old behavior
5. ✅ **All logging preserved**: Every log statement from the old function must be in the new function
6. ✅ **Parameter order correct**: Timeline ID and plugin handle come first, then original parameters
7. ✅ **Return type correct**: Match the old return type (int for point addition, bool for removal/clear, etc.)
8. ✅ **Lock placement correct**: `std::lock_guard<std::mutex> lock(m_timelineMutex);` at function start
9. ✅ **Ownership validation**: `GetTimeline(a_timelineID, a_pluginHandle)` returns nullptr if not owned
10. ✅ **All 7 files updated**: TimelineManager.h, TimelineManager.cpp, FCSE_SKSEFunctions.psc, plugin.cpp, FCSE_API.h, ModAPI.h, ModAPI.cpp
11. ✅ **Phase 2 duplicate removed**: Delete the Phase 2 version from the multi-timeline section in TimelineManager.h
12. ✅ **Temporary compilation fixes applied**: Update any old functions that call this newly-migrated function
13. ✅ **NO [[nodiscard]] on void returns**: Only add [[nodiscard]] for functions that return int/bool/size_t, NOT void
14. ✅ **NO migration comments in Papyrus**: Do NOT add "; [Multi-timeline API Phase 4 - Migrated]" comments
15. ✅ **Read OLD function carefully**: Line-by-line comparison to ensure NOTHING is missed

**The new function must be FUNCTIONALLY EQUIVALENT to the old version, except for:**
- Timeline ID and plugin handle parameters added
- Using `state->m_timeline` instead of `m_timeline`
- Using `state->m_isPlaybackRunning` instead of `m_isPlaybackRunning` (where applicable)
- Ownership validation via `GetTimeline()`

**DO NOT remove or skip any logic from the original function!**

## Common Recurring Mistakes to AVOID

### ❌ MISTAKE 1: Adding [[nodiscard]] to void functions
**WRONG:**
```cpp
[[nodiscard]] virtual void StartRecording(...) const noexcept = 0;  // ❌ NO!
```
**CORRECT:**
```cpp
virtual void StartRecording(...) const noexcept = 0;  // ✅ Only for int/bool/size_t returns
```

### ❌ MISTAKE 2: Forgetting to remove Phase 2 duplicate in TimelineManager.h
After migration, the function should only appear ONCE in TimelineManager.h (in the public section at the top), not also in the "Multi-timeline API - timeline-ID versions (Phase 2)" section at the bottom.

### ❌ MISTAKE 3: Adding migration comments to Papyrus script
**WRONG:**
```papyrus
; [Multi-timeline API Phase 4 - Migrated]  // ❌ NO! Don't add this
bool Function FCSE_RemoveTranslationPoint(...)
```
**CORRECT:**
```papyrus
; Remove a translation point from the timeline  // ✅ Keep it simple
bool Function FCSE_RemoveTranslationPoint(...)
```

### ❌ MISTAKE 4: Missing StopPlayback or other side effect calls
Compare the old function line-by-line. If it calls `StopPlayback()`, the new function must call `StopPlayback(a_timelineID)`.

### ❌ MISTAKE 5: Not updating old functions that call the migrated function
After migrating a function, check if any OLD (not-yet-migrated) functions call it. Update them to use:
- Private helpers like `m_timeline.ClearPoints()` directly
- Or construct proper objects and call private helpers

## Migration Pattern Reference

### Completed Example: `AddTranslationPointAtCamera`

This function has been fully migrated and serves as the reference pattern for all remaining functions.

#### TimelineManager.h Declaration
```cpp
// Old (removed):
// size_t AddTranslationPointAtCamera(float a_time, bool a_easeIn, bool a_easeOut, InterpolationMode a_interpolationMode);

// New (current):
int AddTranslationPointAtCamera(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, bool a_easeIn, bool a_easeOut, InterpolationMode a_interpolationMode);
```

#### TimelineManager.cpp Implementation
```cpp
int TimelineManager::AddTranslationPointAtCamera(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, bool a_easeIn, bool a_easeOut, InterpolationMode a_interpolationMode) {
    std::lock_guard<std::mutex> lock(m_timelineMutex);
    
    TimelineState* state = GetTimeline(a_timelineID, a_pluginHandle);
    if (!state) {
        return -1;
    }
    
    auto* playerCamera = RE::PlayerCamera::GetSingleton();
    if (!playerCamera) {
        return -1;
    }
    
    auto* cameraState = playerCamera->currentState.get();
    if (!cameraState) {
        return -1;
    }
    
    RE::NiPoint3 cameraPos = cameraState->translation;
    
    Transition transition(a_time, a_interpolationMode, a_easeIn, a_easeOut);
    TranslationPoint point(transition, PointType::kWorld, cameraPos);
    
    return static_cast<int>(state->m_timeline.AddTranslationPoint(point));
}
```

#### Papyrus Script (FCSE_SKSEFunctions.psc)
```papyrus
; Add a translation point that captures current camera position
; modName: name of your mod's ESP/ESL file (e.g., "MyMod.esp")
; timelineID: timeline ID to add the point to
; time: time in seconds when this point occurs
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; interpolationMode: 0=None, 1=Linear, 2=CubicHermite (default)
; Returns: index of the added point, or -1 on failure
int Function FCSE_AddTranslationPointAtCamera(string modName, int timelineID, float time, bool easeIn = false, bool easeOut = false, int interpolationMode = 2) global native
```

#### Papyrus C++ Binding (plugin.cpp)
```cpp
int AddTranslationPointAtCamera(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID, float a_time, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
    SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
    if (handle == 0) {
        log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
        return -1;
    }
    return FCSE::TimelineManager::GetSingleton().AddTranslationPointAtCamera(
        static_cast<size_t>(a_timelineID), 
        handle, 
        a_time, a_easeIn, a_easeOut, 
        ToInterpolationMode(a_interpolationMode)
    );
}
```

#### ModAPI Interface (FCSE_API.h)
```cpp
/// <summary>
/// Add a translation point that captures camera position at the start of playback.
/// This point can be used to start playback smoothly from the last camera position, and return to it later.
/// </summary>
/// <param name="a_timelineID">Timeline ID to add the point to</param>
/// <param name="a_pluginHandle">Plugin handle of the calling plugin (use SKSE::GetPluginHandle())</param>
/// <param name="a_time">Time in seconds when this point occurs</param>
/// <param name="a_easeIn">Apply ease-in at the start of interpolation (default: false)</param>
/// <param name="a_easeOut">Apply ease-out at the end of interpolation (default: false)</param>
/// <param name="a_interpolationMode">Interpolation mode: 0=None, 1=Linear, 2=CubicHermite (default)</param>
/// <returns>Index of the added point, or -1 on failure</returns>
[[nodiscard]] virtual int AddTranslationPointAtCamera(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept = 0;
```

#### ModAPI Implementation (ModAPI.h + ModAPI.cpp)
```cpp
// ModAPI.h declaration:
virtual int AddTranslationPointAtCamera(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept override;

// ModAPI.cpp implementation:
int Messaging::FCSEInterface::AddTranslationPointAtCamera(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, bool a_easeIn, bool a_easeOut, int a_interpolationMode) const noexcept {
    // Pass through caller's plugin handle - they own the timeline, not FCSE!
    return FCSE::TimelineManager::GetSingleton().AddTranslationPointAtCamera(
        a_timelineID, 
        a_pluginHandle,  // ← Caller's handle, NOT SKSE::GetPluginHandle()
        a_time, a_easeIn, a_easeOut, 
        FCSE::ToInterpolationMode(a_interpolationMode)
    );
}
```

### Key Migration Principles

#### 1. Three-Tier Ownership System

**Papyrus API (for ESP/ESL mods):**
- Accepts `string modName` as first parameter
- Accepts `int timelineID` as second parameter
- C++ binding converts mod name to plugin handle via `FCSE::ModNameToHandle()`
- Returns `-1` if mod name invalid (mod not loaded)

**ModAPI (for external SKSE plugins):**
- Accepts `size_t a_timelineID` as first parameter
- Accepts `SKSE::PluginHandle a_pluginHandle` as second parameter
- External plugin passes THEIR handle (obtained via SKSE::GetPluginHandle() in their code)
- ModAPI implementation passes through the caller's handle (does NOT call GetPluginHandle() itself)

**Internal FCSE (keyboard controls):**
- Uses FCSE's own handle via SKSE::GetPluginHandle()
- Tracks timeline IDs internally

#### 2. Timeline ID is Mandatory

- No placeholders or defaults
- Required at ALL API levels
- Papyrus, ModAPI, and internal code must all provide explicit timeline IDs

#### 3. Return Type Pattern

**For Point Addition Functions:**
- Returns `int` (not `size_t` or `bool`)
- Success: Returns point index (≥ 0)
- Failure: Returns `-1`

**For Modification Functions:**
- Returns `bool`
- Success: Returns `true`
- Failure: Returns `false`

**For Query Functions:**
- Returns `int` (count)
- Failure: Returns `-1` (timeline not found)

#### 4. Implementation Pattern

All functions follow this structure:
```cpp
ReturnType TimelineManager::FunctionName(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, /* other params */) {
    std::lock_guard<std::mutex> lock(m_timelineMutex);
    
    TimelineState* state = GetTimeline(a_timelineID, a_pluginHandle);
    if (!state) {
        return ErrorValue;  // -1 for int, false for bool
    }
    
    // Function implementation using state->m_timeline
    
    return SuccessValue;
}
```

## Helper Function: ModNameToHandle()

Located in `FCSE_Utils.h` and `FCSE_Utils.cpp`:

```cpp
SKSE::PluginHandle ModNameToHandle(const char* a_modName);
```

**Purpose:** Convert ESP/ESL file name to plugin handle using game's mod index system

**Implementation:**
- Uses `RE::TESDataHandler::GetSingleton()` to access loaded files
- Searches `dataHandler->files` for matching `fileName`
- Returns `file->compileIndex` as plugin handle (unique per load order)
- Returns `0` if mod not found or dataHandler unavailable

**Benefits:**
- Uses native game mod index system (no synthetic handles)
- Validates mod is actually loaded in current save
- Human-readable mod names in logs
- Works with both ESP and ESL files

## Migration Steps (Per Function)

### Important: Two-Phase Approach for Dependencies

**Phase A: Temporary Fixes for Compilation**
- When migrating a function, some OLD (not-yet-migrated) functions may call it
- These old functions need **temporary updates** to compile against the new signature
- These temporary fixes use private helpers or construct objects manually
- Example: When `AddTranslationPoint` was migrated, old `StartRecording()` was temporarily updated to call the private helper `AddTranslationPoint(const TranslationPoint&)`

**Phase B: Actual Function Migration**
- When migrating a function, **ALWAYS use the Phase 2/3 implementation** as the source
- **IGNORE any temporary changes** made to the old function for compilation
- The Phase 2/3 version in the multi-timeline section is the correct, final implementation
- Simply move that implementation to replace the old one

**Critical Rule:** Temporary compilation fixes in old functions are DISPOSABLE. Always migrate from the clean Phase 2/3 implementation, not from the temporarily-patched old version.

### Step 1: Update TimelineManager

#### 1a. Update Declaration (TimelineManager.h)
- Locate old function declaration in public section
- Add `size_t a_timelineID` as **first parameter**
- Add `SKSE::PluginHandle a_pluginHandle` as **second parameter**
- Change return type if needed (see Return Type Pattern above)
- Remove old parameterless version from multi-timeline section (if it exists)

#### 1b. Update Implementation (TimelineManager.cpp)
- **CRITICAL:** Locate the Phase 2/3 implementation in the multi-timeline section (around line 680+)
- **DO NOT use the old function** even if it was temporarily modified for compilation
- Copy the Phase 2/3 implementation to replace the old function at its original location
- Delete the Phase 2/3 duplicate from the multi-timeline section
- The Phase 2/3 version already has the correct structure:
  ```cpp
  std::lock_guard<std::mutex> lock(m_timelineMutex);
  
  TimelineState* state = GetTimeline(a_timelineID, a_pluginHandle);
  if (!state) {
      return ErrorValue;
  }
  
  // Use state->m_timeline instead of m_timeline
  ```
- Remove duplicate multi-timeline version (if it exists)

#### 1c. Handle Temporary Compilation Fixes (If Needed)
- If OTHER not-yet-migrated functions call this newly-migrated function, they will break
- Apply **temporary fixes** to those old functions:
  - Use private helper overloads like `AddTranslationPoint(const TranslationPoint&)` if available
  - Or construct proper objects (Transition, TranslationPoint, etc.) and call helpers
- These temporary fixes are DISPOSABLE - when you migrate those functions later, use the Phase 2/3 version
- Do NOT propagate temporary fixes into the migrated function

### Step 2: Update Papyrus API

#### 2a. Update Script File (FCSE_SKSEFunctions.psc)
- Locate function in Papyrus script
- Add `string modName` as **first parameter** (after `global`)
- Add `int timelineID` as **second parameter**
- Update return type to `int` (for point addition) or `bool` (for modifications)
- Update comments to document new parameters

#### 2b. Update C++ Binding (plugin.cpp)
- Locate function in plugin.cpp
- Add `RE::BSFixedString a_modName` as **first parameter** (after `RE::StaticFunctionTag*`)
- Add `int a_timelineID` as **second parameter**
- Add mod name validation:
  ```cpp
  SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
  if (handle == 0) {
      log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", 
                 __FUNCTION__, a_modName.c_str());
      return -1;  // or false for bool functions
  }
  ```
- Update TimelineManager call with `static_cast<size_t>(a_timelineID)` and `handle`

### Step 3: Update ModAPI

#### 3a. Update Interface (FCSE_API.h)
- Locate the OLD function declaration in `IVFCSE1` interface
- **REPLACE** (not add) the old declaration with the new signature:
  - Add `size_t a_timelineID` as **first parameter**
  - Add `SKSE::PluginHandle a_pluginHandle` as **second parameter**
  - Update return type if needed
  - Update documentation comments
- **CRITICAL**: You must REPLACE the existing function, not add a new one alongside it

#### 3b. Update Implementation Declaration (ModAPI.h)
- Locate function in `FCSEInterface` class
- Match signature from FCSE_API.h (including `const noexcept override`)

#### 3c. Update Implementation (ModAPI.cpp)
- Locate function implementation
- Add both new parameters to signature
- **CRITICAL:** Pass through `a_pluginHandle` parameter (do NOT call SKSE::GetPluginHandle())
- Update TimelineManager call with both parameters

### Step 4: Update Keyboard Controls (if applicable)

- Most functions don't need updates (controls already use timeline-aware code)
- If function is called from ControlsManager.cpp, verify signature matches
- Internal FCSE code uses SKSE::GetPluginHandle() to get FCSE's own handle

### Step 5: Verify Compilation

- Build project
- Check for zero errors
- Resolve any missed call sites (compiler will identify them)

### Step 6: Wait for User Review

**IMPORTANT:** After completing each function migration:
1. Stop and report completion to the user
2. Summarize what was changed (which files, which APIs updated)
3. Wait for user to review the changes
4. Only proceed to the next function when explicitly instructed by the user

Do NOT automatically continue to the next function without user approval.

## Function Migration Checklist

### Group 1: Translation Point Addition (3 functions)
- [x] `AddTranslationPointAtCamera` ✅ **COMPLETED** (reference example)
- [x] `AddTranslationPoint` ✅ **COMPLETED**
- [x] `AddTranslationPointAtRef` ✅ **COMPLETED**

### Group 2: Rotation Point Addition (3 functions)
- [x] `AddRotationPointAtCamera` ✅ **COMPLETED**
- [x] `AddRotationPoint` ✅ **COMPLETED**
- [ ] `AddRotationPointAtRef`

### Group 3: Point Removal (2 functions)
- [x] `RemoveTranslationPoint` ✅ **COMPLETED**
- [x] `RemoveRotationPoint` ✅ **COMPLETED**

### Group 4: Timeline Management (1 function)
- [x] `ClearTimeline` ✅ **COMPLETED**

### Group 5: Query Functions (2 functions)
- [x] `GetTranslationPointCount` ✅ **COMPLETED**
- [x] `GetRotationPointCount` ✅ **COMPLETED**

### Group 6: Recording (2 functions)
- [x] `StartRecording` ✅ **COMPLETED**
- [x] `StopRecording` ✅ **COMPLETED**

### Group 7: Playback (5 functions)
- [x] `StartPlayback` ✅ **COMPLETED**
- [x] `StopPlayback` ✅ **COMPLETED**
- [x] `PausePlayback` ✅ **COMPLETED**
- [x] `ResumePlayback` ✅ **COMPLETED**
- [x] `IsPlaybackRunning` ✅ **COMPLETED**

### Group 8: Import/Export (2 functions)
- [ ] `AddTimelineFromFile`
- [ ] `ExportTimeline`

### Group 9: Cleanup
- [ ] Remove `Timeline m_timeline` member from TimelineManager
- [ ] Remove old single-timeline state variables (m_isRecording, m_playbackSpeed, etc.)

## Progress Tracking

**Current Status:** 18/20 functions migrated
- ✅ AddTranslationPointAtCamera (January 1, 2026)
- ✅ AddTranslationPoint (January 1, 2026)
- ✅ AddTranslationPointAtRef (January 1, 2026)
- ✅ AddRotationPointAtCamera (January 1, 2026)
- ✅ AddRotationPoint (January 1, 2026)
- ✅ AddRotationPointAtRef (January 1, 2026)
- ✅ RemoveTranslationPoint (January 1, 2026)
- ✅ RemoveRotationPoint (January 1, 2026)
- ✅ ClearTimeline (January 1, 2026)
- ✅ GetTranslationPointCount (January 1, 2026)
- ✅ GetRotationPointCount (January 1, 2026)
- ✅ StartRecording (January 1, 2026)
- ✅ StopRecording (January 1, 2026)
- ✅ StartPlayback (January 1, 2026)
- ✅ StopPlayback (January 1, 2026)
- ✅ PausePlayback (January 1, 2026)
- ✅ ResumePlayback (January 1, 2026)
- ✅ IsPlaybackRunning (January 1, 2026)

### Group 8: Import/Export ✅ COMPLETE
- ✅ AddTimelineFromFile (January 1, 2026)
- ✅ ExportTimeline (January 1, 2026)

**✅ ALL 20 FUNCTIONS MIGRATED - PHASE 4 COMPLETE!**

## Quick Reference: File Locations

- **TimelineManager header:** `include/TimelineManager.h`
- **TimelineManager implementation:** `src/TimelineManager.cpp`
- **Papyrus script:** `source/scripts/FCSE_SKSEFunctions.psc`
- **Papyrus C++ binding:** `src/plugin.cpp`
- **ModAPI interface:** `include/FCSE_API.h`
- **ModAPI declaration:** `include/ModAPI.h`
- **ModAPI implementation:** `src/ModAPI.cpp`
- **Keyboard controls:** `src/ControlsManager.cpp`
- **Helper utilities:** `include/FCSE_Utils.h` and `src/FCSE_Utils.cpp`

## Common Pitfalls

1. **Don't use SKSE::GetPluginHandle() in ModAPI implementation** - pass through caller's handle
2. **Don't forget Papyrus script (.psc) updates** - both .psc and plugin.cpp must be updated
3. **Don't forget ModAPI.h declaration** - must match FCSE_API.h interface
4. **Timeline ID and plugin handle are mandatory** - no placeholders or defaults
5. **Query functions don't require ownership** - use `GetTimeline(a_timelineID)` without pluginHandle
6. **Return -1 for errors, not 0** - 0 is a valid point index
