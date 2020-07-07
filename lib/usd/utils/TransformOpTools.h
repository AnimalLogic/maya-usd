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
#pragma once

#include "Api.h"
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/tf/token.h>

#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/xformable.h>
#include <pxr/usd/usdGeom/xformCache.h>

PXR_NAMESPACE_USING_DIRECTIVE

namespace MayaUsdUtils {

class alignas(32) TransformOpProcessor
{
  GfMatrix4d _invParentFrame;
  GfMatrix4d _invWorldFrame;
  GfMatrix4d _worldFrame;
  std::vector<UsdGeomXformOp> _ops;
  uint32_t _opIndex;
  UsdTimeCode _timeCode = UsdTimeCode::Default();
  UsdPrim _prim;
  bool _resetsXformStack = false;
  UsdGeomXformOp op() const { return _ops[_opIndex]; }
public:

  enum Space
  {
    kTransform,
    kWorld,
    kParent
  };

  MAYA_USD_UTILS_PUBLIC
  TransformOpProcessor(const UsdPrim prim, const TfToken opName, const UsdTimeCode& tc = UsdTimeCode::Default());

  MAYA_USD_UTILS_PUBLIC
  TransformOpProcessor(const UsdPrim prim, const uint32_t opIndex, const UsdTimeCode& tc = UsdTimeCode::Default());

  /// re-evaluate the matrices
  MAYA_USD_UTILS_PUBLIC
  void UpdateToTime(const UsdTimeCode& tc, UsdGeomXformCache& cache);
  void UpdateToTime(const UsdTimeCode& tc) { UsdGeomXformCache cache; UpdateToTime(tc, cache); }

  /// apply a translation offset to the xform op
  MAYA_USD_UTILS_PUBLIC
  bool Translate(const GfVec3d& translateChange, Space space = kTransform);

  /// apply a scale offset to the xform op
  MAYA_USD_UTILS_PUBLIC
  bool Scale(const GfVec3d& scaleChange, Space space = kTransform);

  /// apply a rotational offset to the xform op. 
  /// NOTE: This is primarily useful for rotating objects via the sphere (rather than axis rings of the rotate manip)
  /// It's likely that using this method won't result in 'nice' eulers afterwards. If you want 'nice' eulers,
  MAYA_USD_UTILS_PUBLIC
  bool Rotate(const GfQuatd& quatChange, Space space);

  /// apply a rotational offset to the X axis
  MAYA_USD_UTILS_PUBLIC
  bool RotateX(const double radianChange, Space space = kTransform);

  /// apply a rotational offset to the Y axis
  MAYA_USD_UTILS_PUBLIC
  bool RotateY(const double radianChange, Space space = kTransform);

  /// apply a rotational offset to the Z axis
  MAYA_USD_UTILS_PUBLIC
  bool RotateZ(const double radianChange, Space space = kTransform);

  /// return the coordinate frame for the transform op - i.e. the 'origin' for the manipulator
  const GfMatrix4d& CoordinateFrame() const 
    { return _worldFrame; }

  /// given some list of UsdGeomXformOp's, evaluate the coordinate frame needed for the op at the given index. 
  /// This does not evaluate the xform op at that index (i.e. If the first op in ops is a translate, then requesting
  /// index zero will return the identity)
  /// I should really extract this method and pass to Pixar. The code is more performant than UsdGeomXformable::GetLocalTransformation.
  MAYA_USD_UTILS_PUBLIC
  static GfMatrix4d EvaluateCoordinateFrameForIndex(const std::vector<UsdGeomXformOp>& ops, uint32_t index, const UsdTimeCode& timeCode);
};

} // MayaUsdUtils
