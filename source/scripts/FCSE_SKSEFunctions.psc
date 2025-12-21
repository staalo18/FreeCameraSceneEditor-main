Scriptname FCSE_SKSEFunctions

; Get plugin version
int Function GetFCSEPluginVersion() global native

; Add a translation point at a specified position
; time: time in seconds when this point occurs
; posX, posY, posZ: position coordinates
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; Returns: index of the added point
int Function AddTranslationPoint(float time, float posX, float posY, float posZ, bool easeIn = false, bool easeOut = false) global native

; Add a translation point relative to a reference object
; time: time in seconds when this point occurs
; reference: the object reference to track
; offsetX, offsetY, offsetZ: offset from reference position
; isOffsetRelative: if true, offset is relative to reference's heading (local space), otherwise world space
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; Returns: index of the added point
int Function AddTranslationPointAtRef(float time, ObjectReference reference, float offsetX, float offsetY, float offsetZ, bool isOffsetRelative = false, bool easeIn = false, bool easeOut = false) global native

; Add a translation point at the current camera position
; time: time in seconds when this point occurs
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; Returns: index of the added point
int Function AddTranslationPointAtCamera(float time, bool easeIn = false, bool easeOut = false) global native

; Add a rotation point with specified pitch and yaw
; time: time in seconds when this point occurs
; pitch: pitch rotation
; yaw: yaw rotation
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; Returns: index of the added point
int Function AddRotationPoint(float time, float pitch, float yaw, bool easeIn = false, bool easeOut = false) global native

; Add a rotation point using camera-to-reference direction plus offset
; time: time in seconds when this point occurs
; reference: the object reference to track
; offsetPitch: pitch offset from camera-to-reference direction
; offsetYaw: yaw offset from camera-to-reference direction. A value of 0 means looking directly at the reference.
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; Returns: index of the added point
int Function AddRotationPointAtRef(float time, ObjectReference reference, float offsetPitch, float offsetYaw, bool easeIn = false, bool easeOut = false) global native

; Add a rotation point at the current camera rotation
; time: time in seconds when this point occurs
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; Returns: index of the added point
int Function AddRotationPointAtCamera(float time, bool easeIn = false, bool easeOut = false) global native

; Start recording camera movements to the timeline
Function StartRecording() global native

; Stop recording camera movements
Function StopRecording() global native

; Edit an existing translation point
; index: index of the point to edit
; time: new time in seconds
; posX, posY, posZ: new position coordinates
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; Returns: new index of the point after potential re-sorting
int Function EditTranslationPoint(int index, float time, float posX, float posY, float posZ, bool easeIn, bool easeOut) global native

; Edit an existing rotation point
; index: index of the point to edit
; time: new time in seconds
; pitch: new pitch rotation (in radians or degrees depending on configuration)
; yaw: new yaw rotation (in radians or degrees depending on configuration)
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; Returns: new index of the point after potential re-sorting
int Function EditRotationPoint(int index, float time, float pitch, float yaw, bool easeIn, bool easeOut) global native

; Remove a translation point from the timeline
; index: index of the point to remove
Function RemoveTranslationPoint(int index) global native

; Remove a rotation point from the timeline
; index: index of the point to remove
Function RemoveRotationPoint(int index) global native

; Clear the entire camera timeline
; notifyUser: whether to show a notification to the user
Function ClearTimeline(bool notifyUser = true) global native

; Get the number of translation points in the timeline
; Returns: number of translation points
int Function GetTranslationPointCount() global native

; Get the number of rotation points in the timeline
; Returns: number of rotation points
int Function GetRotationPointCount() global native

; Start playback with advanced options
; speed: playback speed multiplier (only used if useDuration=false)
; useDuration: if true, plays timeline over duration seconds
;              if false, plays timeline with speed as speed multiplier
; duration: total duration in seconds for entire timeline (only used if useDuration=true)
; globalEaseIn: apply ease-in at the start of entire traversal (both modes)
; globalEaseOut: apply ease-out at the end of entire traversal (both modes)
Function StartTraversal(float speed = 1.0, bool globalEaseIn = false, bool globalEaseOut = false, bool useDuration = false, float duration = 0.0) global native

; Stop playback of the camera timeline
Function StopTraversal() global native

; Check if timeline playback is currently active
; Returns: true if traversing, false otherwise
bool Function IsTraversing() global native

; Import a camera timeline from a file
; filePath: path to the file to import from
; Returns: true if successful, false otherwise
bool Function ImportTimeline(string filePath) global native

; Export the camera timeline to a file
; filePath: path to the file to export to
; Returns: true if successful, false otherwise
bool Function ExportTimeline(string filePath) global native
