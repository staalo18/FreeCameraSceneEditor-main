# **FreeCameraSceneEditor (FCSE) - Project Context Document**
*Complete technical reference for architectural understanding and refactoring*

---

## **Project Overview**

### **What is FCSE?**
FreeCameraSceneEditor (FCSE) is an SKSE plugin for Skyrim Anniversary Edition that enables cinematic camera path creation and playback. It allows users to record smooth camera movements in Skyrim's free camera mode, edit them with interpolation and easing, and play them back for creating machinima, screenshots, or dynamic scene presentations.

**Core Features:**
- **Timeline-Based Camera Control**: Create camera paths using keyframe-based timelines with translation (position) and rotation points
- **Three Point Types**:
  - **World Points**: Static coordinates in world space
  - **Reference Points**: Dynamic tracking of game objects (NPCs, items) with offset support
  - **Camera Points**: Capture current camera position/rotation during recording
- **Flexible Recording**: Real-time camera path recording with 1-second sampling intervals
- **Advanced Interpolation**: Support for linear and cubic Hermite interpolation with per-point and global easing
- **Import/Export**: Save/load camera paths as `.fcse` timeline files with load-order independent reference tracking
- **Three API Surfaces**:
  - **Papyrus Scripts**: Full scripting integration for quest/scene automation
  - **C++ Mod API**: Native plugin-to-plugin communication for other SKSE mods
  - **Debug Hotkeys**: 8 keyboard shortcuts for testing and manual control
- **Optional TrueHUD Integration**: Real-time 3D path visualization during playback

**Target Users:**
- Skyrim machinima creators needing cinematic camera control
- Modders building scripted scenes with dynamic camera movements
- Screenshot artists creating complex camera animations
- Developers extending FCSE functionality through the C++ API

---

## **Developer Quick Start**

### **Project Architecture Summary**
FCSE follows a **singleton manager pattern** with **template-based timeline engines**. The core design separates concerns into:
1. **API Layer**: Three entry points (Papyrus, C++ Mod API, Keyboard) → all funnel into `TimelineManager`
2. **Orchestration Layer**: `TimelineManager` singleton coordinates recording/playback state
3. **Timeline Layer**: `Timeline` class pairs translation and rotation tracks with metadata
4. **Engine Layer**: `TimelineTrack<T>` template handles interpolation and playback mechanics
5. **Storage Layer**: `CameraPath<T>` template manages point collections and file I/O

**Key Design Patterns:**
- **Singleton**: `TimelineManager`, `APIManager`, `ControlsManager` (thread-safe Meyer's singleton)
- **Paired Tracks**: `Timeline` class coordinates independent `TimelineTrack<TranslationPath>` and `TimelineTrack<RotationPath>`
- **Three-Tier Encapsulation**: 
  - `Timeline` hides `TimelineTrack` implementation from `TimelineManager`
  - `TimelineTrack` hides `Path` implementation via wrapper methods (GetPath() private)
  - `Path` fully encapsulated - only accessible through `TimelineTrack` wrappers
- **Template Specialization**: `TimelineTrack<T>` provides type-safe interpolation for position and rotation data
- **Track Independence**: Translation and rotation tracks can have different point counts and keyframe times
- **Dependency Injection**: All interpolation/easing math lives in external `_ts_SKSEFunctions` workspace
- **Hook-Based Updates**: Skyrim's main loop (`MainUpdateHook`) drives all timeline updates

**File Structure:**
```
src/
  ├─ plugin.cpp           # SKSE entry point + Papyrus API bindings (20+ functions)
  ├─ TimelineManager.cpp  # Recording/playback orchestration (singleton)
  ├─ Timeline.cpp         # Timeline class implementations (paired track coordination)
  ├─ CameraPath.cpp       # Point storage + import/export (.fcse file format)
  ├─ ControlsManager.cpp  # Keyboard input handling (8 debug hotkeys)
  ├─ Hooks.cpp            # Game loop interception (MainUpdateHook)
  ├─ APIManager.cpp       # External API requests (TrueHUD integration)
  └─ FCSE_Utils.cpp       # Hermite interpolation + file parsing helpers

include/
  ├─ TimelineTrack.h      # TimelineTrack<T> template (declaration + implementation)
  ├─ Timeline.h           # Timeline class declaration (non-template wrapper)
  ├─ CameraPath.h         # Point storage templates (TranslationPath, RotationPath)
  ├─ CameraTypes.h        # Core enums (InterpolationMode, PointType, PlaybackMode)
  ├─ TimelineManager.h    # Main orchestrator interface
  ├─ FCSE_API.h           # C++ Mod API interface definition
  └─ API/TrueHUDAPI.h     # External API header (v3)
```

**Data Flow:**
```
User Input (Papyrus/Hotkey/ModAPI)
  ↓
TimelineManager (validate, orchestrate)
  ↓
Timeline::AddTranslationPoint() / AddRotationPoint()
  ↓
TimelineTrack<PathType>::AddPoint() (internal: stores in m_path via private GetPath())
  ↓
MainUpdateHook (every frame)
  ↓
TimelineManager::Update()
  ├─ RecordTimeline() → sample camera @ 1Hz → Timeline::AddPoint()
  ├─ PlayTimeline() → Timeline::UpdatePlayback(deltaTime) 
  │                 → Timeline::GetTranslation/Rotation(time)
  │                 → TimelineTrack::GetPointAtTime(t)
  │                 → (internal: m_path access for interpolation)
  └─ DrawTimeline() → TrueHUD->DrawLine() (if available)
  ↓
Apply to RE::FreeCameraState (position/rotation)
```

**Encapsulation Boundaries:**
- **TimelineManager** → sees only Timeline public API
- **Timeline** → sees TimelineTrack public API (GetPath() hidden)
- **TimelineTrack** → directly accesses m_path member (Path fully encapsulated)

**Common Tasks:**
- **Adding New API Functions**: Bind in `plugin.cpp` (Papyrus), implement in `TimelineManager`, expose in `FCSE_API.h` (Mod API)
- **Changing Interpolation**: Modify `TimelineTrack<T>::GetPointAtTime()` in `Timeline.inl` (calls interpolation helpers)
- **File Format Changes**: Update `CameraPath::ExportPath()` and `AddPathFromFile()` in `CameraPath.cpp`
- **New Point Types**: Extend `PointType` enum in `CameraTypes.h`, add cases in `TranslationPoint::GetPoint()` / `RotationPoint::GetPoint()` methods

**Build Dependencies:**
- **CommonLibSSE-NG**: SKSE plugin framework (set `COMMONLIBSSE_PATH` env var)
- **_ts_SKSEFunctions**: Utility library (must be sibling directory) - provides interpolation/easing functions
- **vcpkg**: Dependency manager (fmt, spdlog, rapidcsv, simpleini)
- **CMake 3.29+**: Build system with MSVC presets

**Testing Entry Points:**
1. Launch Skyrim AE with SKSE
2. Open console: `tfc` (enable free camera)
3. Press `8` to start recording (or call `FCSE_StartRecording()` from Papyrus)
4. Move camera, then press `9` to stop
5. Exit free camera (`tfc`), then press `7` to play timeline
6. Check logs: `Documents/My Games/Skyrim Special Edition/SKSE/FreeCamSceneEditor.log`

---

## **Quick Reference**

### **Architecture at a Glance:**
```
Entry Points (3 surfaces):
  ├─ Papyrus API (plugin.cpp) → FCSE_SKSEFunctions script
  ├─ Mod API (ModAPI.h) → FCSE_API::IVFCSE1 interface
  └─ Keyboard (ControlsManager.cpp) → Hardcoded DXScanCode handlers

Core Engine:
  ├─ TimelineManager (singleton) → Orchestrates recording/playback
  │   └─ Timeline (paired tracks) → Coordinates translation + rotation
  │       ├─ TimelineTrack<TranslationPath> → Position keyframes
  │       └─ TimelineTrack<RotationPath> → Rotation keyframes
  └─ TimelineTrack<T> (template) → Interpolation + playback state
      └─ CameraPath<T> (template) → Point storage + import/export

Update Loop (Hooks.cpp):
  MainUpdateHook → TimelineManager::Update() every frame
    ├─ DrawTimeline() → TrueHUD visualization
    ├─ PlayTimeline() → Apply interpolated camera state
    └─ RecordTimeline() → Sample camera every 1 second
```

### **Key Data Structures:**
- **Transition:** `{time, mode, easeIn, easeOut}` - Keyframe metadata
- **PointType:** `kWorld` (static), `kReference` (dynamic), `kCamera` (baked)
- **InterpolationMode:** `kNone`, `kLinear`, `kCubicHermite`
- **PlaybackMode:** `kEnd` (stop), `kLoop` (wrap with offset)

### **Critical Patterns:**
- **Point Construction:** Always pass `Transition` object (not raw time)
- **Type Conversion:** `int` → `InterpolationMode` at API boundaries
- **User Rotation:** Accumulated offset pattern (`m_rotationOffset`)
- **kCamera Baking:** `UpdateCameraPoints()` at `StartPlayback()`

---

## **1. Project Overview**
- **Purpose**: Skyrim Special Edition SKSE plugin for cinematographic camera control
- **Core Feature**: Timeline-based camera movement with keyframe interpolation
- **Language**: C++ (SKSE CommonLibSSE-NG framework)
- **Current State**: Single-timeline implementation with Papyrus API, Mod API, and keyboard controls
- **Future Goal:** Multi-timeline support with exclusive playback/recording

---

## **2. Entry Points & API Surface**

### **2.1 Plugin Initialization (`plugin.cpp`)**
```
SKSEPlugin_Load()
├─> _ts_SKSEFunctions::InitializeLogging() [log level from INI]
├─> Register SKSE message listener → MessageHandler()
├─> Register Papyrus functions → FCSE::Interface::FCSEFunctions()
└─> Install Hooks → Hooks::Install()

MessageHandler() [SKSE lifecycle events]
├─> kDataLoaded/kPostLoad/kPostPostLoad: APIs::RequestAPIs()
└─> kPostLoadGame/kNewGame:
    ├─> APIs::RequestAPIs()
    └─> Register ControlsManager as input event sink

RequestPluginAPI(InterfaceVersion) [Mod API entry]
└─> Returns FCSEInterface singleton for V1
```

**Key Constants:**
- Plugin version: Encoded as `major * 10000 + minor * 100 + patch`
- Log levels: 0-6 (spdlog), defaults to 3 (info) if invalid
- INI path: `SKSE/Plugins/FreeCameraSceneEditor.ini`

---

### **2.2 Papyrus API (`plugin.cpp` - FCSE::Interface namespace)**
**Registration:** All functions bound to `FCSE_SKSEFunctions` Papyrus script

**Timeline Building Functions:**
| Papyrus Function | Return | Parameters | Notes |
|-----------------|--------|------------|-------|
| `FCSE_AddTranslationPointAtCamera` | int | time, easeIn, easeOut, interpolationMode | Captures camera position |
| `FCSE_AddTranslationPoint` | int | time, posX, posY, posZ, easeIn, easeOut, interpolationMode | Absolute world position |
| `FCSE_AddTranslationPointAtRef` | int | time, ref, offsetX/Y/Z, isOffsetRelative, easeIn, easeOut, interpolationMode | Ref-based position |
| `FCSE_AddRotationPointAtCamera` | int | time, easeIn, easeOut, interpolationMode | Captures camera rotation |
| `FCSE_AddRotationPoint` | int | time, pitch, yaw, easeIn, easeOut, interpolationMode | Absolute world rotation |
| `FCSE_AddRotationPointAtRef` | int | time, ref, offsetPitch/Yaw, isOffsetRelative, easeIn, easeOut, interpolationMode | Ref-based rotation |
| `FCSE_RemoveTranslationPoint` | void | index | Remove by index |
| `FCSE_RemoveRotationPoint` | void | index | Remove by index |
| `FCSE_ClearTimeline` | void | notifyUser | Clear all points |
| `FCSE_GetTranslationPointCount` | int | - | Query point count |
| `FCSE_GetRotationPointCount` | int | - | Query point count |

**Recording Functions:**
| Papyrus Function | Parameters | Notes |
|-----------------|------------|-------|
| `FCSE_StartRecording` | - | Begin capturing camera movement |
| `FCSE_StopRecording` | - | Stop capturing |

**Playback Functions:**
| Papyrus Function | Parameters | Notes |
|-----------------|------------|-------|
| `FCSE_StartPlayback` | speed, globalEaseIn, globalEaseOut, useDuration, duration | Begin timeline playback |
| `FCSE_StopPlayback` | - | Stop playback |
| `FCSE_PausePlayback` | - | Pause playback |
| `FCSE_ResumePlayback` | - | Resume from pause |
| `FCSE_IsPlaybackPaused` | - | Query pause state |
| `FCSE_IsPlaybackRunning` | - | Query playback state |
| `FCSE_AllowUserRotation` | allow | Enable/disable user camera control during playback |
| `FCSE_IsUserRotationAllowed` | - | Query user rotation state |

**Import/Export Functions:**
| Papyrus Function | Parameters | Notes |
|-----------------|------------|-------|
| `FCSE_AddTimelineFromFile` | filePath, timeOffset | Import INI file |
| `FCSE_ExportTimeline` | filePath | Export to INI file |

**Type Conversion Layer:**
- **InterpolationMode**: Papyrus passes `int` (0=None, 1=Linear, 2=CubicHermite), converted via `ToInterpolationMode()` in `CameraTypes.h`
- **Return Values**: TimelineManager returns `size_t`, cast to `int` for Papyrus

---

### **2.3 Mod API (`ModAPI.h`, `FCSE_API.h`)**
**Purpose:** Binary interface for other SKSE plugins to call FCSE functions

**Access Pattern:**
```cpp
auto api = reinterpret_cast<FCSE_API::IVFCSE1*>(
    RequestPluginAPI(FCSE_API::InterfaceVersion::V1)
);
```

**Interface: `IVFCSE1`** (pure virtual, defined in `FCSE_API.h`)
- **Thread Safety:** `GetFCSEThreadId()` returns TID for thread validation
- **Version Check:** `GetFCSEPluginVersion()` returns encoded version
- **Timeline Functions:** Identical signatures to Papyrus API, but `noexcept` and `const`
  - All functions delegate to `TimelineManager::GetSingleton()`
  - Parameters use C++ native types (no Papyrus conversions)
  - `int interpolationMode` converted via `ToInterpolationMode()` at API boundary

**Implementation: `FCSEInterface`** (in `ModAPI.h`, implemented in `Messaging` namespace)
- Singleton pattern: `GetSingleton()` returns static instance
- Private constructor/destructor
- Stores `apiTID` member for thread tracking
- All methods marked `const noexcept override`

---

### **2.4 Keyboard Controls (`ControlsManager.cpp`)**
**Integration:** Registered as `RE::BSTEventSink<RE::InputEvent*>` on game load

**Hardcoded Keybindings** (DXScanCode values):
| Key | Action | TimelineManager Call |
|-----|--------|----------------------|
| 2 | Toggle Pause | `PausePlayback()` / `ResumePlayback()` |
| 3 | Stop Playback | `StopPlayback()` |
| 4 | Toggle User Rotation | `AllowUserRotation(!IsUserRotationAllowed())` |
| 5 | **Debug Scene** | Complex hardcoded camera path demo |
| 6 | Clear Timeline | `ClearTimeline()` |
| 7 | Start Playback | `StartPlayback()` |
| 8 | Start Recording | `StartRecording()` |
| 9 | Stop Recording | `StopRecording()` |
| 10 | Export | `ExportTimeline("SKSE/Plugins/FCSE_CameraPath.ini")` |
| 11 | Import | `AddTimelineFromFile("SKSE/Plugins/FCSE_CameraPath.ini")` |

**Key 5 Scene Details:**
- Looks up FormID `0xd8c56` as `RE::TESObjectREFR` (must be an actor)
- Builds 11-point camera path: camera → actor tracking → player → camera
- Uses `GetTargetPoint()` to calculate head bone offset from actor's position
- Adds +20 units to Y offset (forward direction, not upward)
- Demonstrates all point types: kCamera, kReference (world-space and local-space)

**Input Filtering:**
- Ignores input when `RE::UI::GetSingleton()->GameIsPaused()` is true
- Only processes `kButton` events with `IsDown()` state

---

### **2.5 External APIs (`APIManager.h/cpp`)**
**Purpose:** Request APIs from other SKSE plugins

**Currently Integrated:**
- **TrueHUD API v3** (`TRUEHUD_API::IVTrueHUD3`)
  - Requested at multiple SKSE lifecycle stages (workaround for messaging bug)
  - Stored in static inline pointer: `APIs::TrueHUD`
  - **Usage:**
    - `TimelineManager::DrawTimeline()` (line 286): Calls `DrawLine(point1, point2)` to visualize camera path in 3D world space
    - `FCSE_Utils::SetHUDMenuVisible()` (line 52): Calls `SetHUDMenuVisible()` to control menu visibility
  - Optional: Functions only execute if TrueHUD is installed and API is available

---

### **2.6 Hooks (`Hooks.h/cpp`)**
**Purpose:** Intercept Skyrim game loop and input handling

**Installed Hooks:**
1. **MainUpdateHook** (RELOCATION_ID 35565/36564)
   - Calls `TimelineManager::Update()` every frame
   - Uses trampoline pattern with `write_call<5>()`

2. **LookHook** (VTable hooks on `RE::LookHandler`)
   - **ProcessThumbstick** (vfunc 0x2): Gamepad camera rotation
   - **ProcessMouseMove** (vfunc 0x3): Mouse camera rotation
   - **Behavior:** Blocks input if `IsPlaybackRunning() && !IsUserRotationAllowed()`
   - Sets `SetUserTurning(true)` when input detected

3. **MovementHook** (VTable hooks on `RE::MovementHandler`)
   - **ProcessThumbstick** (vfunc 0x2): Gamepad movement
   - **ProcessButton** (vfunc 0x4): WASD movement keys
   - **Behavior:** Blocks forward/back/strafe input during playback
   - Checks against `userEvents->forward/back/strafeLeft/strafeRight`

**Hook Philosophy:**
- Preserve original behavior via `_OriginalFunction` pattern
- Only intercept when `IsPlaybackRunning()` and game not paused
- Track user interaction state (`SetUserTurning()`) for recording

---

## **3. Type System & Conventions**

### **3.1 Core Enums (CameraTypes.h)**

**InterpolationMode:**
```cpp
enum class InterpolationMode {
    kNone = 0,
    kLinear = 1,
    kCubicHermite = 2
};
```
- **Converter Function:** `ToInterpolationMode(int)` in `CameraTypes.h`
- **Valid Values:** 0-2 (validator bounds check)
- **Usage:** API layers pass `int`, convert before calling TimelineManager

**PointType:**
```cpp
enum struct PointType {
    kWorld = 0,      // Static world point
    kReference = 1,  // Dynamic reference-based point
    kCamera = 2      // Static camera-based point (initialized at StartPlayback)
};
```
- **Converter Function:** `ToPointType(int)` in `CameraTypes.h`
- **Usage:** Determines how points calculate their world position during playback

**PlaybackMode:**
```cpp
enum class PlaybackMode : int {
    kEnd = 0,   // Stop at end of timeline (default)
    kLoop = 1   // Restart from beginning when timeline completes
};
```
- **Stored in INI:** Saved/loaded with timeline files
- **Applied to both tracks:** Timeline class synchronizes mode across translation and rotation

### **3.2 Naming Conventions**
- **Member Variables:** `m_` prefix (e.g., `m_translationTimeline`)
- **Function Parameters:** `a_` prefix (e.g., `a_time`, `a_reference`)
- **Logger:** Uses `log::info`, `log::warn`, `log::error` (CommonLib pattern)

### **3.3 Singleton Pattern**
- **TimelineManager**, **ControlsManager**, **FCSEInterface**, **APIManager**
- All use `GetSingleton()` returning static instance reference

---

## **4. Dependencies & Build System**

### **4.1 External Libraries (vcpkg)**
- **CommonLibSSE-NG**: SKSE plugin framework
- **spdlog**: Logging
- **fmt**: String formatting
- **simpleini**: INI file parsing
- **rapidcsv**: CSV parsing (unused?)
- **DirectXMath**, **DirectXTK**: Math utilities

### **4.2 Build Configuration**
- **CMake + Ninja**: Build system
- **MSVC compiler**: cl.exe
- **Precompiled Header:** `PCH.h`
- **Output:** `.dll` plugin loaded by SKSE

---

## **5. Core Architecture: TimelineManager & Timeline System**

### **5.1 TimelineManager Overview**
**Role:** Orchestrates recording, playback, and timeline manipulation  
**Pattern:** Singleton with a single Timeline instance (paired translation + rotation tracks)

**Member Variables:**
```cpp
// Timeline storage
Timeline m_timeline;  // Paired translation + rotation tracks

// Recording state
bool m_isRecording;                     // Currently capturing camera
float m_currentRecordingTime;           // Elapsed time since recording start
float m_recordingInterval = 1.0f;       // Sample rate (1 point per second)
float m_lastRecordedPointTime;          // Last sample timestamp

// Playback state
bool m_isPlaybackRunning;               // Active playback
float m_playbackSpeed;                  // Time multiplier (duration / timeline duration)
bool m_globalEaseIn/Out;                // Apply easing to entire timeline
float m_playbackDuration;               // Target playback length (user-specified or calculated)
bool m_isShowingMenus;                  // Pre-playback UI state (for restoration)
bool m_showMenusDuringPlayback;         // UI visibility during playback
bool m_userTurning;                     // Flag: user initiated camera rotation (set by LookHook)
bool m_allowUserRotation;               // Permission: allow user rotation during playback
RE::BSTPoint2<float> m_rotationOffset;  // Accumulated user rotation delta
RE::NiPoint2 m_lastFreeRotation;        // Third-person camera rotation (pre-playback snapshot)
```

---

### **5.2 Update Loop (Frame-by-Frame)**
**Called by:** `MainUpdateHook::Nullsub()` every frame

```cpp
void TimelineManager::Update() {
    if (UI->GameIsPaused()) {
        if (m_isPlaybackRunning) ui->ShowMenus(m_isShowingMenus);  // Restore UI on pause
        return;
    } else if (m_isPlaybackRunning) {
        ui->ShowMenus(m_showMenusDuringPlayback);
    }
    
    DrawTimeline();    // Visualize path via TrueHUD (if available)
    PlayTimeline();    // Update camera from interpolated points
    RecordTimeline();  // Sample camera position if recording
}
```

**Critical Insight:** All three operations coexist in the loop, but guards prevent conflicts:
- `PlayTimeline()` checks `m_isPlaybackRunning && !m_isRecording`
- `RecordTimeline()` checks `m_isRecording`
- `DrawTimeline()` only draws when `!m_isPlaybackRunning && !m_isRecording`

---

### **5.3 Recording System**

#### **Recording Lifecycle:**
```
StartRecording()
├─> Validate: Free camera mode, not already recording/playing
├─> ClearTimeline(notify=false)
├─> Capture initial point (easeIn=true)
└─> Set m_isRecording = true

RecordTimeline() [called every frame]
├─> Update m_currentRecordingTime += deltaTime
├─> If (time - lastSample >= interval):
│   ├─> GetCameraPos/Rotation (from _ts_SKSEFunctions)
│   ├─> AddTranslationPoint(kWorld, pos)
│   ├─> AddRotationPoint(kWorld, rotation)
│   └─> Update m_lastRecordedPointTime
└─> If not in free camera: auto-call StopRecording()

StopRecording()
├─> Capture final point (easeOut=true)
└─> Set m_isRecording = false
```

**Key Characteristics:**
- **PointType:** Always creates `kWorld` points (static coordinates)
- **Sampling:** Fixed 1-second intervals (`m_recordingInterval`)
- **Interpolation:** All recorded points use `kCubicHermite` mode
- **Position:** `_ts_SKSEFunctions::GetCameraPos()` returns current camera world position
- **Rotation:** `_ts_SKSEFunctions::GetCameraRotation()` returns pitch (x) and yaw (z)
- **Auto-Stop:** Exits free camera → terminates recording

---

### **5.4 Playback System**

#### **Playback Lifecycle:**
```
StartPlayback(speed, globalEaseIn, globalEaseOut, useDuration, duration)
├─> Validate: ≥1 point, not in free camera, not recording/playing, duration > 0
├─> Calculate playback speed:
│   ├─> useDuration=true: speed = timelineDuration / duration
│   └─> useDuration=false: speed = speed parameter
├─> Save pre-playback state:
│   ├─> m_isShowingMenus = ui->IsShowingMenus()
│   └─> m_lastFreeRotation = ThirdPersonState->freeRotation
├─> Reset timelines: ResetTimeline(), UpdateCameraPoints()
├─> Enter free camera mode: ToggleFreeCameraMode(false)
└─> Set m_isPlaybackRunning = true

PlayTimeline() [called every frame]
├─> Validate: playback running, not recording, free camera active, points exist
├─> Update timeline playback time:
│   ├─> deltaTime = GetRealTimeDeltaTime() * m_playbackSpeed
│   ├─> m_translationTimeline.UpdateTimeline(deltaTime)
│   └─> m_rotationTimeline.UpdateTimeline(deltaTime)
├─> Apply global easing (if enabled):
│   ├─> linearProgress = playbackTime / timelineDuration
│   ├─> easedProgress = ApplyEasing(linearProgress, easeIn, easeOut)
│   └─> sampleTime = easedProgress * timelineDuration
├─> Sample interpolated points:
│   ├─> position = m_translationTimeline.GetPointAtTime(sampleTime)
│   └─> rotation = m_rotationTimeline.GetPointAtTime(sampleTime)
├─> Handle user rotation:
│   ├─> IF (m_userTurning && m_allowUserRotation):
│   │   ├─> Update m_rotationOffset = current - timeline
│   │   ├─> Reset m_userTurning flag
│   │   └─> Don't override camera (user controls rotation)
│   └─> ELSE: Apply rotation = timeline + m_rotationOffset
├─> Write to FreeCameraState: translation, rotation.x (pitch), rotation.y (yaw)
└─> Check completion: if both timelines stopped → StopPlayback()

StopPlayback()
├─> If in free camera:
│   ├─> Exit free camera mode
│   ├─> Restore m_isShowingMenus
│   └─> Restore m_lastFreeRotation (for third-person)
└─> Reset all playback state flags and timeline positions
```

**Critical Behaviors:**
- **User Rotation Control:**
  - `m_userTurning` flag set by `LookHook` when user moves mouse/thumbstick
  - `m_allowUserRotation` enables accumulated offset mode
  - Offset persists across frames (allows looking around during playback)
  - Offset calculation: `NormalRelativeAngle(current - timeline)`

- **Global Easing:**
  - Applied to **sample time**, not speed
  - Affects entire timeline progress curve
  - Independent of per-point easing

- **Camera State:**
  - Playback forces `kFree` camera state
  - Directly writes `FreeCameraState->translation` and `->rotation`
  - Movement/look hooks block user input during playback

---

### **5.5 Timeline & TimelineTrack Architecture**

#### **Timeline Class (Non-Template Wrapper)**
**Purpose:** High-level coordinator for paired translation + rotation tracks  
**Location:** `Timeline.h` (declaration), `Timeline.cpp` (implementation)

**Member Variables:**
```cpp
TranslationTrack m_translationTrack;  // Position keyframes
RotationTrack m_rotationTrack;        // Rotation keyframes

uint32_t m_timelineID{ 0 };           // Unique identifier (for future multi-timeline)
float m_playbackSpeed{ 1.0f };        // Time multiplier
bool m_globalEaseIn{ false };         // Apply easing to entire timeline
bool m_globalEaseOut{ false };        // Apply easing to entire timeline
```

**Type Aliases:**
```cpp
using TranslationTrack = TimelineTrack<TranslationPath>;
using RotationTrack = TimelineTrack<RotationPath>;
```

**Public API - Complete Encapsulation:**

Timeline now provides complete encapsulation - TimelineManager never directly accesses TimelineTrack. All operations go through Timeline's public API.

**Point Management:**
```cpp
size_t AddTranslationPoint(const TranslationPoint& a_point);  // Returns new point count
size_t AddRotationPoint(const RotationPoint& a_point);        // Returns new point count
void RemoveTranslationPoint(size_t a_index);
void RemoveRotationPoint(size_t a_index);
void ClearPoints();  // Clears both tracks
```

**Coordinated Playback:**
```cpp
void UpdatePlayback(float a_deltaTime);  // Updates both tracks in sync
void StartPlayback();      // Bakes kCamera points in both tracks
void ResetPlayback();      // Resets both tracks to time 0
void PausePlayback();
void ResumePlayback();
```

**Query Methods:**
```cpp
// State queries (OR logic: true if either track matches)
float GetPlaybackTime() const;  // Returns translation track playback time
bool IsPlaying() const;         // true if translation OR rotation track is playing
bool IsPaused() const;          // true if translation OR rotation track is paused

// Count and duration
size_t GetTranslationPointCount() const;
size_t GetRotationPointCount() const;
float GetDuration() const;  // max(translation.GetDuration(), rotation.GetDuration())

// Playback mode (applied to both tracks)
PlaybackMode GetPlaybackMode() const;
float GetLoopTimeOffset() const;
void SetPlaybackMode(PlaybackMode a_mode);
void SetLoopTimeOffset(float a_offset);
```

**Sampling:**
```cpp
RE::NiPoint3 GetTranslation(float a_time) const;     // Query translation at time
RE::BSTPoint2<float> GetRotation(float a_time) const; // Query rotation at time
```

**Import/Export:**
```cpp
// Camera point access (for TimelineManager::Add...PointAtCamera)
// Note: Takes parameters (time, easeIn, easeOut) - creates TransitionPoint with PointType::kCamera
TranslationPoint GetTranslationPointAtCamera(float a_time, bool a_easeIn, bool a_easeOut) const;
RotationPoint GetRotationPointAtCamera(float a_time, bool a_easeIn, bool a_easeOut) const;

// File I/O (delegates to Path::AddPathFromFile / Path::ExportPath)
bool AddTranslationPathFromFile(std::ifstream& a_file, float a_timeOffset = 0.0f, float a_conversionFactor = 1.0f);
bool AddRotationPathFromFile(std::ifstream& a_file, float a_timeOffset = 0.0f, float a_conversionFactor = 1.0f);
bool ExportTranslationPath(std::ofstream& a_file, float a_conversionFactor = 1.0f) const;
bool ExportRotationPath(std::ofstream& a_file, float a_conversionFactor = 1.0f) const;
```

**Visualization:**
```cpp
RE::NiPoint3 GetTranslationPointPosition(size_t a_index) const;  // For DrawTimeline visualization
```

**Private Track Access:**
```cpp
// These are now private - only accessible within Timeline.cpp implementations
TranslationTrack& GetTranslationTrack();
RotationTrack& GetRotationTrack();
const TranslationTrack& GetTranslationTrack() const;
const RotationTrack& GetRotationTrack() const;
```

**Architectural Benefits:**
1. **Complete Encapsulation:** TimelineManager has no direct access to TimelineTrack internals
2. **Unified Interface:** All operations go through Timeline public API
3. **OR Logic:** IsPlaying()/IsPaused() check both tracks (returns true if either matches)
4. **Future-Proof:** Track implementation can change without affecting TimelineManager
5. **Single Point of Access:** All track coordination logic contained in Timeline class

**Key Insight:** Timeline class enforces paired track coordination while preserving track independence (different point counts/times allowed). TimelineTrack is now a pure implementation detail.

---

#### **TimelineTrack<PathType> Template (Low-Level Engine)**
**Purpose:** Generic interpolation and playback engine for any path type  
**Location:** `TimelineTrack.h` (single-file template: declaration + implementation)  
**Access:** Private to Timeline class only (not exposed to TimelineManager)

**Template Specializations:**
```cpp
TimelineTrack<TranslationPath>  // Position timeline (RE::NiPoint3 values)
TimelineTrack<RotationPath>     // Rotation timeline (RE::BSTPoint2<float> values)
```

**Member Variables:**
```cpp
PathType m_path;                          // CameraPath<TransitionPoint> - stores ordered points (PRIVATE ACCESS)
float m_playbackTime{ 0.0f };             // Current position in timeline (seconds)
bool m_isPlaying{ false };                // Playback active
bool m_isPaused{ false };                 // Playback paused
PlaybackMode m_playbackMode{ PlaybackMode::kEnd };  // kEnd (stop) or kLoop (wrap)
float m_loopTimeOffset{ 0.0f };           // Extra time for loop interpolation (last→first)
```

**Type Traits (provided by PathType):**
```cpp
PathType::TransitionPoint  // TranslationPoint or RotationPoint
PathType::ValueType        // RE::NiPoint3 or RE::BSTPoint2<float>
```

**Encapsulation Architecture:**

TimelineTrack now provides complete path encapsulation through wrapper methods. The `GetPath()` accessor is private (internal use only).

**Public Path Operations (wrapper methods):**
```cpp
// Sampling
TransitionPoint GetPointAtCamera(float a_time, bool a_easeIn, bool a_easeOut) const;  // Delegate to m_path
const TransitionPoint& GetPoint(size_t a_index) const;                                // Delegate to m_path

// File I/O
bool AddPathFromFile(std::ifstream& a_file, float a_timeOffset, float a_conversionFactor);  // Delegate to m_path
bool ExportPath(std::ofstream& a_file, float a_conversionFactor) const;                    // Delegate to m_path
```

**Private Path Access (internal only):**
```cpp
PathType& GetPath() { return m_path; }              // For internal modifications
const PathType& GetPath() const { return m_path; }  // For internal const access
```

**Three-Tier Encapsulation:**
1. **Timeline** (public API) → hides TimelineTrack implementation
2. **TimelineTrack** (engine) → hides Path implementation (via wrapper methods)
3. **Path** (storage) → fully encapsulated, only accessible through TimelineTrack wrappers

**Core Operations:**

| Method | Behavior |
|--------|----------|
| `AddPoint(point)` | Insert sorted by time, calls `ResetTimeline()` |
| `UpdateTimeline(deltaTime)` | Advance `m_playbackTime`, handle loop wrap or end |
| `GetPointAtTime(time)` | Calculate segment index + progress, interpolate (returns ValueType) |
| `StartPlayback()` | Call `UpdateCameraPoints()` (bake kCamera), set playing |
| `ResetTimeline()` | Zero playback time, clear playing/paused flags |
| `GetPlaybackTime()` | Returns current playback position |

**Interpolation Dispatch:**
```cpp
ValueType GetInterpolatedPoint(index, progress) {
    switch (point.m_transition.m_mode) {
        case kNone: return currentPoint.m_point;
        case kLinear: return GetPointLinear(index, progress);
        case kCubicHermite: return GetPointCubicHermite(index, progress);
    }
}
```

**Cubic Hermite Logic:**
- **Tangent Calculation:** Uses 4-point neighborhood (p0, p1, p2, p3)
- **Loop Mode:** Modulo wrapping for neighbor indices
- **End Mode:** Clamp neighbors at boundaries
- **Progress:** Apply per-point easing before interpolation
- **Angular (Rotation):** Uses `CubicHermiteInterpolateAngular()` for angle wrapping

---

### **5.6 CameraPath<T> Template (Storage Layer)**

**Purpose:** Point collection with type-specific I/O operations  
**Location:** `CameraPath.h` (declaration), `CameraPath.cpp` (implementation)

**Concrete Implementations:**
```cpp
class TranslationPath : public CameraPath<TranslationPoint> {
public:
    using TransitionPoint = TranslationPoint;
    using ValueType = RE::NiPoint3;  // Type returned by GetPoint()
    
    TranslationPoint GetPointAtCamera(float a_time, bool a_easeIn, bool a_easeOut) const override;  // Creates kCamera point
    bool AddPathFromFile(...) override;
    bool ExportPath(...) const override;
};

class RotationPath : public CameraPath<RotationPoint> {
public:
    using TransitionPoint = RotationPoint;
    using ValueType = RE::BSTPoint2<float>;  // Type returned by GetPoint()
    
    RotationPoint GetPointAtCamera(float a_time, bool a_easeIn, bool a_easeOut) const override;  // Creates kCamera point
    bool AddPathFromFile(...) override;
    bool ExportPath(...) const override;
};
```

**Key Insights:** 
- `ValueType` alias resolves naming conflict with `PointType` enum (kWorld/kReference/kCamera)
- `GetPointAtCamera()` is const-correct: creates new point without modifying path state
- Takes time/easing parameters, returns TransitionPoint with PointType::kCamera and transition metadata

---

### **5.7 Point Construction Patterns**

#### **Transition Object:**
```cpp
struct Transition {
    float m_time;                // Keyframe timestamp (seconds)
    InterpolationMode m_mode;    // kNone, kLinear, kCubicHermite
    bool m_easeIn, m_easeOut;    // Apply smoothing
};
```

#### **TranslationPoint Constructor:**
```cpp
TranslationPoint(
    const Transition& a_transition,         // Time + interpolation settings
    PointType a_pointType,                  // kWorld, kReference, kCamera
    const RE::NiPoint3& a_point,            // World position (kWorld/kCamera baked)
    const RE::NiPoint3& a_offset = {},      // Offset for kReference/kCamera
    RE::TESObjectREFR* a_reference = null,  // Target ref (kReference only)
    bool a_isOffsetRelative = false         // Local space offset (kReference only)
)
```

**PointType Semantics:**

| Type | `m_point` | `m_offset` | `m_reference` | `GetPoint()` Behavior |
|------|-----------|------------|---------------|----------------------|
| **kWorld** | Static position | Unused | null | Returns `m_point` directly |
| **kReference** | Unused (mutable cache) | World/local offset | Required | `ref->GetPosition() + (rotated offset if relative)` |
| **kCamera** | Baked at StartPlayback | Initial offset | null | Returns baked `m_point` |

**Critical: kCamera Baking**
- Created with `PointType::kCamera` and `m_offset`
- `GetPointAtCamera()` calculates: `GetCameraPos() + m_offset`
- `UpdateCameraPoints()` calls `GetPointAtCamera()` → stores result in `m_point`
- After baking, behaves like `kWorld` (static position)

#### **RotationPoint Constructor:**
```cpp
RotationPoint(
    const Transition& a_transition,
    PointType a_pointType,
    const RE::BSTPoint2<float>& a_point,    // Pitch/Yaw (kWorld/kCamera baked)
    const RE::BSTPoint2<float>& a_offset = {},
    RE::TESObjectREFR* a_reference = null,
    bool a_isOffsetRelative = false
)
```

**PointType Semantics:**

| Type | Behavior |
|------|----------|
| **kWorld** | Static pitch/yaw angles |
| **kReference** (relative=false) | Camera looks at ref, offset adjusts aim direction |
| **kReference** (relative=true) | Uses ref's heading + offset (ignores camera position) |
| **kCamera** | Baked at StartPlayback: `GetCameraRotation() + m_offset` |

**Reference Rotation Math:**
- **relative=false:** Calculate camera→ref direction, build coordinate frame, apply offset as local rotation
- **relative=true:** Use `ref->GetHeading()` or `GetAngleX/Z()`, add offset directly

---

### **5.8 Import/Export System**

#### **INI File Format:**
```ini
[General]
Version=10203           ; major*10000 + minor*100 + patch
UseDegrees=1            ; Convert angles (always 1 for export)
PlaybackMode=0          ; 0=kEnd, 1=kLoop
LoopTimeOffset=0.0      ; Interpolation time for loop wrap

[TranslationPoints]
; Point fields vary by PointType (see below)

[RotationPoints]
; Point fields vary by PointType
```

**Field Encoding by PointType:**

| PointType | Translation Fields | Rotation Fields |
|-----------|-------------------|-----------------|
| **kWorld** | PositionX/Y/Z | Pitch, Yaw |
| **kReference** | OffsetX/Y/Z, RefFormID, IsOffsetRelative | OffsetPitch/Yaw, RefFormID, IsOffsetRelative |
| **kCamera** | OffsetX/Y/Z | OffsetPitch/Yaw |

**All Points Include:**
- `Time`, `InterpolationMode` (0/1/2), `EaseIn` (0/1), `EaseOut` (0/1), `PointType` (0/1/2)

**Import Process (TimelineManager.cpp):**
```
AddTimelineFromFile(path, timeOffset)
├─> Read General section (version, degrees, playback mode, loop offset)
├─> Import translation: m_timeline.GetTranslationTrack().GetPath().AddPathFromFile(stream, offset, 1.0f)
├─> Rewind stream
├─> Import rotation: m_timeline.GetRotationTrack().GetPath().AddPathFromFile(stream, offset, degToRad)
├─> Set playback mode/offset: m_timeline.SetPlaybackMode(), SetLoopTimeOffset()
└─> Log point counts
```

**Export Process (TimelineManager.cpp):**
```
ExportTimeline(path)
├─> Write General section (get mode from m_timeline.GetTranslationTrack().GetPlaybackMode())
├─> Export translation: m_timeline.GetTranslationTrack().GetPath().ExportPath(stream, 1.0f)
├─> Export rotation: m_timeline.GetRotationTrack().GetPath().ExportPath(stream, radToDeg)
└─> Log point counts
```

**Critical Details:**
- **Inline Comments:** Import supports `;` comments (via CSimpleIniA)
- **Time Offset:** Applied to all imported point times (for timeline concatenation)
- **Conversion Factor:** Translation uses 1.0 (no conversion), Rotation uses deg/rad conversion
- **Playback Mode:** Read from General section, applied via `m_timeline.SetPlaybackMode()` (syncs both tracks)
- **Version Check:** Warns if file version ≠ plugin version (non-blocking)

---

### **5.9 DrawTimeline() Visualization**

**Conditions:**
- `APIs::TrueHUD != nullptr` (TrueHUD plugin loaded)
- Points exist (translation or rotation)
- `!m_isPlaybackRunning && !m_isRecording`
- Free camera mode active

**Behavior (TimelineManager.cpp):**
```cpp
for (i = 0; i < m_timeline.GetTranslationPointCount() - 1; ++i) {
    auto& point1 = m_timeline.GetTranslationTrack().GetPath().GetPoint(i).m_point;
    auto& point2 = m_timeline.GetTranslationTrack().GetPath().GetPoint(i+1).m_point;
    APIs::TrueHUD->DrawLine(point1, point2);
}
```

**Visual Result:** Line segments connecting translation keyframes (visible in HUD)

---

### **5.10 Point Modification Rules**---

### **5.9 Point Modification Rules**

**Thread Safety:** Currently none (single-threaded SKSE)
### **5.10 Point Modification Rules**

**Thread Safety:** Currently none (single-threaded SKSE)

**Playback Protection (TimelineManager.cpp):**
All modification methods check `m_isPlaybackRunning`:
```cpp
if (m_isPlaybackRunning) {
    log::info("Timeline modified during playback, stopping");
    StopPlayback();
}
```

**Protected Operations:**
- `AddTranslationPoint()`, `AddRotationPoint()` (return `size_t` - new point count)
- `RemoveTranslationPoint()`, `RemoveRotationPoint()`
- `AddTimelineFromFile()`

**Note:** `ClearTimeline()` blocks if `m_isRecording` (prevents accidental wipe)

---

## **6. Supporting Systems**

### **6.1 Import/Export Implementation (CameraPath.cpp)**

**Reference Resolution Strategy:**
1. **EditorID First** (load-order independent)
   - `RE::TESForm::LookupByEditorID<RE::TESObjectREFR>(editorID)`
   - Validates plugin name if `RefPlugin` field present
   - **Requires:** po3's Tweaks for reliable EditorID support
2. **FormID Fallback** (load-order dependent)
   - `RE::TESForm::LookupByID(formID)` from hex string
3. **Failure Handling:** Creates kWorld point using offset as absolute position/rotation

**INI Parsing:**
- Uses `ParseFCSETimelineFileSections()` with callback pattern
- Inline comments (`;`) supported and stripped from values
- Graceful error handling: logs warnings, skips invalid entries

**Export Warnings:**
- Logs if reference has no EditorID (portability risk)
- Always writes FormID + plugin name for debugging

---

### **6.2 Timeline Architecture: Current Structure**

**File Organization:**
- **TimelineTrack.h**: Complete template implementation (declaration + all methods)
  - `TimelineTrack<PathType>` template class
  - Type aliases: `TranslationTrack`, `RotationTrack`
  - All template method implementations inline (no separate .inl file)
- **Timeline.h**: Non-template Timeline class declaration
  - Paired track coordinator (wraps TranslationTrack + RotationTrack)
  - Includes `TimelineTrack.h` for template definitions
- **Timeline.cpp**: Timeline class implementations
  - All non-template Timeline methods

**Design Rationale:**
- **Single-file templates**: TimelineTrack.h contains both declaration and implementation (no .inl split)
- **Clean separation**: Template code (TimelineTrack) in one file, non-template code (Timeline) in another
- **Automatic dependencies**: Timeline.h includes TimelineTrack.h, so users only need to include Timeline.h

**Historical Notes:**
- **Previous structure** (Timeline_old.h, now deleted):
  - Original design: Single template `Timeline<PathType>` (all-in-one)
  - Type aliases: `TranslationTimeline`, `RotationTimeline` 
  - TimelineManager stored two separate timeline instances
- **Refactoring migration**:
  - Renamed `Timeline<T>` → `TimelineTrack<T>` (emphasizes engine role)
  - Created new `Timeline` wrapper class (paired track coordination)
  - Split implementation: Timeline.inl (deleted) → merged into TimelineTrack.h
  - Benefits: Better encapsulation, clearer API, multi-timeline ready
---

### **6.3 Utility Functions (_ts_SKSEFunctions)**

**Camera Functions:**
```cpp
RE::NiPoint3 GetCameraPos()         // World position
RE::NiPoint3 GetCameraRotation()    // {pitch, yaw, roll} in radians
float GetCameraPitch/Yaw()          // Individual components
```

**Angle Utilities:**
```cpp
float NormalRelativeAngle(angle)    // Wrap to [-π, π]
float NormalAbsoluteAngle(angle)    // Wrap to [0, 2π]
```

**Timing:**
```cpp
float GetRealTimeDeltaTime()        // Frame delta (from RELOCATION_ID)
```

**Easing:**
```cpp
float ApplyEasing(t, easeIn, easeOut) {
    if (easeIn && easeOut) return SCurveFromLinear(t, 0.33, 0.66);  // S-curve
    if (easeIn) return InterpEaseIn(0, 1, t, 2.0);                   // Quadratic ease-in
    if (easeOut) return 1 - InterpEaseIn(0, 1, 1-t, 2.0);            // Quadratic ease-out
    return t;                                                         // Linear
}
```

**INI File Access:**
```cpp
T GetValueFromINI(vm, stackId, "key:section", filePath, defaultValue)
// Supports: bool, long, double, string
// Uses CSimpleIniA for parsing
// Path resolved from Data/ directory
```

---

### **6.4 Helper Functions (FCSE_Utils)**

**Cubic Hermite Interpolation:**
```cpp
// Standard (positions, linear values)
CubicHermiteInterpolate(a0, a1, a2, a3, t)
  ├─> Catmull-Rom tangents: m1 = (a2-a0)*0.5, m2 = (a3-a1)*0.5
  └─> Hermite basis: h00, h10, h01, h11 polynomial

// Angular (pitch/yaw, wraps correctly)
CubicHermiteInterpolateAngular(a0, a1, a2, a3, t)
  ├─> Convert angles to unit circle (sin/cos)
  ├─> Interpolate in 2D space
  └─> Convert back via atan2(sin, cos)
```

**HUD Control:**
```cpp
SetHUDMenuVisible(visible)
  ├─> Requires TrueHUD API
  └─> Sets TrueHUD menu visibility via uiMovie->SetVisible()
```

**GetTargetPoint(actor)** - Returns head bone position:
```cpp
1. Get race->bodyPartData
2. Find kHead body part (fallback: kTotal)
3. Lookup bone node: NiAVObject_LookupBoneNodeByName(actor3D, targetName)
4. Returns NiPointer<RE::NiAVObject> (world transform)
```
**Usage:** Calculate head bone offset in ControlsManager Key 5 demo
- Computes offset from actor's position to head bone world position
- Adds +20 units to Y (forward direction relative to actor's heading)
- Used for local-space kReference translation points

**ParseFCSETimelineFileSections():**
- Parses INI sections with callback pattern
- Handles inline comments (`;` anywhere in value)
- Trims whitespace from keys/values
- Returns true if file parsed (even with errors in individual entries)

---

## **7. Critical Implementation Details**

### **Multi-Timeline Migration Considerations**

**Current Single-Timeline State:**
- Two parallel timelines: `m_translationTimeline`, `m_rotationTimeline`
- 12+ bool/float state members (recording, playback, user rotation)
- No timeline IDs - direct member access pattern

**Key Complexity Areas for Multi-Timeline:**
1. **Update() Loop** - Currently calls Play/Record/Draw directly
   - Need: Iterate active timelines, track which is playing/recording
2. **Camera State Management** - Single FreeCameraState updated per frame
   - Need: Determine which timeline controls camera (exclusive playback)
3. **User Rotation Offset** - Single `m_rotationOffset` accumulator
   - Need: Per-timeline offset tracking
4. **Playback State Restoration** - `m_lastFreeRotation`, `m_isShowingMenus`
   - Need: Per-timeline snapshots
5. **Point Baking** - `UpdateCameraPoints()` bakes kCamera on StartPlayback
   - Need: Track baking per timeline, handle re-baking on switch
6. **API Surface** - All functions currently implicit (singleton pattern)
   - Need: Add timeline ID parameters, return TimelineError codes
7. **Import/Export** - Single file = entire timeline
   - Need: Track which timeline owns imported data

**Point Construction Already Compatible:**
- `Transition` + `PointType` pattern works for multi-timeline
- No timeline ID stored in points (good - timeline manages its points)

---

## **8. Known Limitations & Unimplemented Features**

### **Implemented & Verified:**
- ✅ **GetTargetPoint()** - Returns head bone's `NiAVObject` (world transform) via `NiAVObject_LookupBoneNodeByName`
- ✅ **SetHUDMenuVisible()** - Controls TrueHUD menu visibility via `uiMovie->SetVisible()`
- ✅ **ModAPI.cpp** - Simple pass-through to TimelineManager (converts `int` → `InterpolationMode`)
- ✅ **Offsets.h** - Defines `NiAVObject_LookupBoneNodeByName` (RELOCATION_ID 74481/76207, from True Directional Movement)

### **Known Limitations:**
- ⚠️ **Reference Deletion Safety:** No validation if `m_reference` becomes invalid during playback (dangling pointer risk)
  - `GetPoint()` will crash if reference deleted
  - No RefHandle tracking or validity checks
- ⚠️ **Loop + kCamera:** Baked values don't update on loop wrap (snapshot taken once at StartPlayback)
- ⚠️ **Gimbal Lock:** GetYaw/GetPitch functions unreliable near ±90° pitch (documented in _ts_SKSEFunctions.h)
- ⚠️ **Thread Safety:** None - all operations assume SKSE main thread (single-threaded by design)

---

## **9. External Dependencies**

**Required:**
- CommonLibSSE-NG (SKSE framework)
- spdlog (logging)
- SimpleIni (INI parsing with comment support)

**Optional:**
- TrueHUD API v3 (for DrawTimeline visualization)
- po3's Tweaks (for reliable EditorID in exports)

**Build Tools:**
- CMake 3.29+, Ninja, MSVC (cl.exe)
- vcpkg for dependencies

