Scriptname _ts_FCSE_PapyrusFunctions

; Get plugin version
int Function GetFCSEPluginVersion() global native

; Add a camera path point at current camera location
; typeStr: "translation" or "rotation" (or "t" or "r")
; time: duration in seconds
; easeIn: ease in at start
; easeOut: ease out at end
Function AddCameraPathPoint(string typeStr, float time, bool easeIn, bool easeOut) global native
