scriptName GtsPlugin hidden

; Get Distance To Camera
;
; This will get the distance to the camera

;==========================================
;===================C A M E R A           |
;==========================================

Float function GetDistanceToCamera(Actor target) global native
Function SetFeetTracking(Bool enabled) global native

;==================================================================================
;==================================================================================
;==================================================================================

;==========================================
;===================D A M A G E           |
;==========================================

; Sets global damage mult for Stomps
function SetSizeDamageMultiplier(Float bonus) global native

; Get size related damage modifier.
; This functions reports damage modifiers for: Normal, Fall, Sprint and High Heel damage bonuses.
; The damage is calculated through the .dll.
; 0 = Get Normal    Size-Related Damage
; 1 = Get Sprint    Size-Related Damage
; 2 = Get Fall      Size-Related Damage
; 3 = Get High Heel Size-Related Damage
Float function GetSizeRelatedDamage(Actor caster, Float attribute) global native

; Gets increased size damage debuff
Float function GetSizeVulnerability(Actor target) global native

; apply increased size-related damage debuff 
Bool function ModSizeVulnerability(Actor target, Float amt) global native


;==================================================================================
;==================================================================================
;==================================================================================

;==========================================
;===================A T T R I B U T E S   |
;==========================================
;Reports bonus attributes in %
; 1 = health
; 2 = carry weight
; 3 = speed
; 4 = damage
Float function GetAttributeBonus(Actor target, float value) global native

; Reports bonus attribute in flat numbers
; 1 = health
; 2 = carry weight
Float function GetFlatAttributeBonus(Actor target, float value) global native

; Check if Hit Growth is allowed
Float function GetHitGrowth(Actor target) global native

; Set hit growth
; 1 == True
; 0 == False 
Bool function SetHitGrowth(Actor target, float allow) global native

; This is the time it takes to reach half of the target height
; By default this is 0.05s (instantaneous)
;
; Value is saved into the cosave
Bool function SetGrowthHalfLife(Actor target, Float halflife) global native

;Get half life of the target
Float function GetGrowthHalfLife(Actor target) global native

;Sets progression mult
Float function SetProgressionMultiplier(float value) global native


;======================================================================================
;======================================================================================
;======================================================================================



;==========================================
;===================A N I M A T I O N     |
;==========================================

; This sets the speed at which anims are played
; 1.0 is normal speed while 2.0 is twice as fast
;
; Value is saved into the cosave
Bool function SetAnimSpeed(Actor target, Float animspeed) global native

;======================================================================================
;======================================================================================
;======================================================================================

; Format a number to a string with specified significant figures
;
; Uses sprintf
;
; e.g.
; String formatted_number = SigFig(10.2323, 3)
; ; formatted_number should now be 10.2
String function SigFig(Float number, Int sf) global native


;==========================================
;===================H I G H H E E L S     |
;==========================================

; Controls if the HH correction method is enabled or not
;
; Value is saved into the cosave
Function SetIsHighHeelEnabled(Bool enabled) global native
Bool Function GetIsHighHeelEnabled() global native
; Self-explanatory
Function SetIsHHFurnitureEnabled(Bool enabled) global native

;======================================================================================
;======================================================================================
;======================================================================================

;==========================================
;===================A I                   |
;==========================================

Function SetVoreAi(bool enabled) global native
Function SetStompAi(bool enabled) global native
Function SetSandwichAi(bool enabled) global native    


;======================================================================================
;======================================================================================
;======================================================================================


;==========================================
;===================V O R E               |
;==========================================

;Allows/Disallows Player to be eaten, default: false
Function SetAllowPlayerVore(Bool enabled) global native

;Disallow/Allow non-combat random vore for followers
Function SetOnlyCombatVore(Bool enabled) global native
    

;CTD-free Set Critical Stage function
Function DisintegrateTarget(Actor target) global native

; returns the value of Devourment compatibility
Bool function GetDevourmentCompatibility() global native   
    
; enables/disables Devourment compatibility
Function SetDevourmentCompatibility(Bool enabled) global native
    
; returns temp data for Actor - used for 'was dragon absorbed' detection       
    
;==================================================
;===================M I S C F U N C T I O N S     |
;==================================================

function IncreaseSizeLimit(float value, Actor caster) global native
function IncreaseMassLimit(float value, Actor caster) global native
    
Bool function WasDragonEaten() global native 

Bool function DragonCheck(Actor target) global native    

; Allow/Get precise damage toggle
Bool Function GetPreciseDamage() global native

; Controls if the Anim and Walk speeds adjustments are enabled or not
;
; Value is saved into the cosave
Function SetIsSpeedAdjusted(Bool enabled) global native
Bool Function GetIsSpeedAdjusted() global native
; These control the variables in the speed adjustment formula
;
; The formula is
; 1/(1+(k*(x-1.0))^(n*s))^(1/s)
; https://www.desmos.com/calculator/klvqenjooi
; Values are saved into the cosave
Float Function GetSpeedParameterK() global native
Float Function SetSpeedParameterK(Float k) global native
Float Function GetSpeedParameterN() global native
Function SetSpeedParameterN(Float n) global native
Function GetSpeedParameterS() global native
Function SetSpeedParameterS(Float s) global native

; Extra Actor state functions
Bool function IsJumping(Actor target) global native
Bool function IsInAir(Actor target) global native

; Global Tremor Scales
;
; These control the scale of the camera/controller
; shakes on footstep
;
; 1.0 is default
; 0.0 means off
;
; Values are saved into the cosave
Float Function GetTremorScale() global native
Function SetTremorScale(Float value) global native
Float Function GetTremorScaleNPC() global native
Function SetTremorScaleNPC(Float value) global native

; Debug functions
Float Function GetExperimentFloat() global native
Function SetExperimentFloat(Float value) global native
;======================================================================================
;======================================================================================
;======================================================================================