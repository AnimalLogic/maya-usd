## 1.0.3 (2020-04-22)

### Build (maya-usd-build)
+ Fixed Known issues from 1.0.2 and 1.0.1 below
+ Disable rpath/runpath #52
+ adding python path to _mayaUsd.lib #56
+ build /test/lib/usd/utils in place of usd utils build #48
+ [PIPE-2390]: Fix various AL_USDMaya-1.0.2 path bugs #45


### AL_USDMaya (maya-usd)
+ Inject common LD_LIBRARY_PATH vars into add_maya_test #70



## 1.0.2 (2020-04-15)

### Build (maya-usd-build)

### AL_USDMaya (maya-usd)
+ [PIPE-2089] Added '.variantFallbacks' attribute to store custom fallbacks #37
+ [PIPE-2318] Add option to write extents during geometry export. #64
+ [PIPE-2347] Consider implicit default values for "custom" and "variability" fields #63
+ Merge in changes from ADSKPublic dev from 31st Jan - 9th April 2020 (up to 1d77003c3), resolving a number of merge conflicts



### Known Issues
+ Can't import AL.usd.schemas.maya
+ include directory has moved (probably requires an update to maya-usd-build AL_USDMaya.cmake?)

## 1.0.1 (2020-03-23)


### Build (maya-usd-build)
+ updated to support usdBase 0.20.2

### AL_USDMaya (maya-usd)
+ [PIPE-1950] Ignored setting playback range if no time code found from USD. 
+ [PIPE-2311] fixed up some doxygen warnings
+ [PIPE-2079] Added support of animated near/far clipping planes
+ [PIPE-2258] ensure_TRS_values_match_commonAPI) correctly set inverse transform ops  ensure translation values are written using double precision, and ensure we output "xformOp:translate" rather than "xformOp:translate:translate
+ [PIPE-1950] Ignored setting playback range if no time code found from USD #50


### Known Issues
+ rpath being used on libMayaUSD

## 1.0.0(2020-03-05)

### Build (maya-usd-build)
+ updated to support usdBase 0.20.2

### AL_USDMaya (maya-usd)
+ First release of code based off github.com/autodesk/maya-usd 

### Known Issues
+ rpath being used on libMayaUSD




