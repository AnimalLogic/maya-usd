//
// Copyright 2017 Animal Logic
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef MAYAUSDUTILS_TRANSFOM_OP_TOOLS_H_
#define MAYAUSDUTILS_TRANSFOM_OP_TOOLS_H_

#include "Api.h"
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/tf/token.h>

#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/xformable.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include "SIMD.h"
#include "TransformManipulator.h"

PXR_NAMESPACE_USING_DIRECTIVE

namespace MayaUsdUtils {

//----------------------------------------------------------------------------------------------------------------------------------------------------------
/// All methods in TransformManipulator deal with relative offsets. This class extends the base and adds support to set xform ops to specific positions
/// and orientations. All methods in this class are implemented using methods from the base class.  
//----------------------------------------------------------------------------------------------------------------------------------------------------------
class TransformManipulatorEx : public TransformManipulator
{
public:

  TransformManipulatorEx(const UsdPrim prim, const TfToken opName, ManipulatorMode mode = kGuess, const UsdTimeCode& tc = UsdTimeCode::Default())
    : TransformManipulator(prim, opName, mode, tc) {}

  TransformManipulatorEx(const UsdPrim prim, const uint32_t opIndex, ManipulatorMode mode = kGuess, const UsdTimeCode& tc = UsdTimeCode::Default())
    : TransformManipulator(prim, opIndex, mode, tc) {}

  /// set the translate value on the translate op xform op
  MAYA_USD_UTILS_PUBLIC
  bool SetTranslate(const GfVec3d& position, Space space = kTransform);

  /// set the scale value on the xform op
  MAYA_USD_UTILS_PUBLIC
  bool SetScale(const GfVec3d& scale, Space space = kTransform);

  /// set transform op to world space orientation
  MAYA_USD_UTILS_PUBLIC
  bool SetRotate(const GfQuatd& orientation, Space space = kTransform);

  //--------------------------------------------------------------------------------------------------------------------------------------------------------
  // Compute the current transform op value - all values in local space
  //--------------------------------------------------------------------------------------------------------------------------------------------------------

  /// returns the current orientation as a quat (If CanRotate() returns false, the identity quat is returned)
  inline
  GfQuatd Rotation() const
    { return TransformManipulator::Rotation(); }

  /// returns the current translation as a vec3 (If CanTranslate() returns false, [0,0,0] is returned)
  inline
  GfVec3d Translation() const
    { return TransformManipulator::Translation(); }

  /// returns the current scale as a vec3 (If CanScale() returns false, [1,1,1] is returned)
  inline
  GfVec3d Scale() const
    { return TransformManipulator::Scale(); }

  //--------------------------------------------------------------------------------------------------------------------------------------------------------
  // static 'one-hit' versions
  //--------------------------------------------------------------------------------------------------------------------------------------------------------

  /// apply a translation offset to the xform op
  MAYA_USD_UTILS_PUBLIC
  static bool Translate(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfVec3d& translateChange, Space space = kTransform);

  /// apply a scale offset to the xform op
  MAYA_USD_UTILS_PUBLIC
  static bool Scale(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfVec3d& scaleChange, Space space = kTransform);

  /// apply a rotational offset to the X axis
  MAYA_USD_UTILS_PUBLIC
  static bool RotateX(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const double radianChange, Space space = kTransform);

  /// apply a rotational offset to the Y axis
  MAYA_USD_UTILS_PUBLIC
  static bool RotateY(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const double radianChange, Space space = kTransform);

  /// apply a rotational offset to the Z axis
  MAYA_USD_UTILS_PUBLIC
  static bool RotateZ(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const double radianChange, Space space = kTransform);

  /// apply a rotational offset to the xform op. 
  /// NOTE: This is primarily useful for rotating objects via the sphere (rather than axis rings of the rotate manip)
  /// It's likely that using this method won't result in 'nice' eulers afterwards. 
  /// If you want 'nice' eulers (as much as is possible with a rotate tool), then prefer to use the axis rotation 
  /// methods, RotateX etc. 
  /// It should also be noted that this method may end up being called by the RotateX/RotateY/RotateZ methods if 
  /// either the rotation is not a simple one - i.e. a simple RotateX xform op 
  MAYA_USD_UTILS_PUBLIC
  static bool Rotate(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfQuatd& quatChange, Space space);

  /// set the translate value on the translate op xform op
  MAYA_USD_UTILS_PUBLIC
  static bool SetTranslate(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfVec3d& position, Space space = kTransform);

  /// set the scale value on the xform op
  MAYA_USD_UTILS_PUBLIC
  static bool SetScale(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfVec3d& scale, Space space = kTransform);

  /// set transform op to world space orientation
  MAYA_USD_UTILS_PUBLIC
  static bool SetRotate(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfQuatd& orientation, Space space = kTransform);

  //--------------------------------------------------------------------------------------------------------------------------------------------------------
  // The static version of Rotate/Translate etc, end up hiding the base class implementations. inline them here. 
  //--------------------------------------------------------------------------------------------------------------------------------------------------------

  /// apply a translation offset to the xform op
  inline
  bool Translate(const GfVec3d& translateChange, Space space = kTransform)
    { return TransformManipulator::Translate(translateChange, space); }

  /// apply a scale offset to the xform op
  inline
  bool Scale(const GfVec3d& scaleChange, Space space = kTransform)
    { return TransformManipulator::Scale(scaleChange, space); }

  /// apply a rotational offset to the X axis
  inline
  bool RotateX(const double radianChange, Space space = kTransform)
    { return TransformManipulator::RotateX(radianChange, space); }

  /// apply a rotational offset to the Y axis
  inline
  bool RotateY(const double radianChange, Space space = kTransform)
    { return TransformManipulator::RotateY(radianChange, space); }

  /// apply a rotational offset to the Z axis
  inline
  bool RotateZ(const double radianChange, Space space = kTransform)
    { return TransformManipulator::RotateZ(radianChange, space); }

  /// apply a rotational offset to the xform op. 
  /// NOTE: This is primarily useful for rotating objects via the sphere (rather than axis rings of the rotate manip)
  /// It's likely that using this method won't result in 'nice' eulers afterwards. 
  /// If you want 'nice' eulers (as much as is possible with a rotate tool), then prefer to use the axis rotation 
  /// methods, RotateX etc. 
  /// It should also be noted that this method may end up being called by the RotateX/RotateY/RotateZ methods if 
  /// either the rotation is not a simple one - i.e. a simple RotateX xform op 
  inline
  bool Rotate(const GfQuatd& quatChange, Space space)
    { return TransformManipulator::Rotate(quatChange, space); }
};

} // MayaUsdUtils

#endif
