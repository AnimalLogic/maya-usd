//
// Copyright 2020 Animal Logic
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
#ifndef MAYAUSDUTILS_TRANSFOM_EVALUATOR_H_
#define MAYAUSDUTILS_TRANSFOM_EVALUATOR_H_

#include "Api.h"
#include <pxr/base/gf/matrix4d.h>

#include <pxr/usd/usdGeom/xformOp.h>
#include "SIMD.h"

PXR_NAMESPACE_USING_DIRECTIVE

namespace MayaUsdUtils {

//----------------------------------------------------------------------------------------------------------------------------------------------------------
/// \brief  A class that can efficiently evaluate a list of UsdGeomXformOp's. 
//----------------------------------------------------------------------------------------------------------------------------------------------------------
class TransformEvaluator
{
public:

  /// given some list of UsdGeomXformOp's, evaluate the coordinate frame needed for the ops between a given range
  MAYA_USD_UTILS_PUBLIC
  static GfMatrix4d EvaluateCoordinateFrameForRange(const std::vector<UsdGeomXformOp>& ops, uint32_t start, uint32_t end, const UsdTimeCode& timeCode);

  /// given some list of UsdGeomXformOp's, evaluate the coordinate frame needed for the op at the given index. 
  /// This does not evaluate the xform op at that index (i.e. If the first op in ops is a translate, then requesting
  /// index zero will return the identity)
  inline
  static GfMatrix4d EvaluateCoordinateFrameForIndex(const std::vector<UsdGeomXformOp>& ops, uint32_t index, const UsdTimeCode& timeCode)
    { return EvaluateCoordinateFrameForRange(ops, 0, index, timeCode); }

  /// given some list of UsdGeomXformOp's, evaluate the matrix for the entire stack of xform ops
  inline
  static GfMatrix4d EvaluateMatrix(const std::vector<UsdGeomXformOp>& ops, const UsdTimeCode& timeCode)
    { return EvaluateCoordinateFrameForRange(ops, 0, ops.size(), timeCode); }

protected:
  // helper methods to extract scale/rotation/translation from the transform op
  MAYA_USD_UTILS_PUBLIC
  static d256 _Scale(const UsdGeomXformOp& op, const UsdTimeCode& timeCode);
  MAYA_USD_UTILS_PUBLIC
  static d256 _Rotation(const UsdGeomXformOp& op, const UsdTimeCode& timeCode);
  MAYA_USD_UTILS_PUBLIC
  static d256 _Translation(const UsdGeomXformOp& op, const UsdTimeCode& timeCode);
};

} // MayaUsdUtils

#endif