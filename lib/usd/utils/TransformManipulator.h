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
#ifndef MAYAUSDUTILS_TRANSFOM_MANIPULATOR_H_
#define MAYAUSDUTILS_TRANSFOM_MANIPULATOR_H_

#include "Api.h"
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/tf/token.h>

#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/xformable.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include "SIMD.h"
#include "TransformEvaluator.h"

PXR_NAMESPACE_USING_DIRECTIVE

namespace MayaUsdUtils {

//----------------------------------------------------------------------------------------------------------------------
/// \brief  A class that is able to manipulate a transform op in a stack (using relative offsets)
//----------------------------------------------------------------------------------------------------------------------
class alignas(32) TransformManipulator : public TransformEvaluator
{
public:

  MAYA_USD_UTILS_PUBLIC
  static TfToken primaryRotateSuffix;
  MAYA_USD_UTILS_PUBLIC
  static TfToken primaryScaleSuffix;
  MAYA_USD_UTILS_PUBLIC
  static TfToken primaryTranslateSuffix;

  // when processing matrix transform ops, the coordinate frame for the manipulator will change 
  // depending on whether we are setting up for scale, rotatation, or translation
  enum ManipulatorMode
  {
    kTranslate, 
    kRotate, 
    kScale,
    kGuess //< for most ops, this will just work. For matrix ops, you'll need to be more specific
  };

  // Given that a single xform op can be a part of an xformOp stack, it implies that there are 4 
  // possible coordinate frames you may want to define the translation/rotation ops in. 
  //
  // World:              [stack after op][-- xform op --][stack before op][parent world matrix]
  //                                                                offset is applied here ---^
  //
  // Parent:             [stack after op][-- xform op --][stack before op][parent world matrix]
  // [PreTransform]                             offset is applied here ---^
  //
  // Transform:          [stack after op][-- xform op --][stack before op][parent world matrix]
  //               offset is applied directly ---^
  //
  // Object:             [stack after op][-- xform op --][stack before op][parent world matrix]
  // [PostTransform]    ^--- offset is applied here
  // 
  enum Space
  {
    kWorld,
    kPreTransform,
    kTransform,
    kPostTransform,
    kParent = kPreTransform,
    kObject = kPostTransform
  };

  MAYA_USD_UTILS_PUBLIC
  TransformManipulator(const UsdPrim prim, const TfToken opName, ManipulatorMode mode = kGuess, const UsdTimeCode& tc = UsdTimeCode::Default());

  MAYA_USD_UTILS_PUBLIC
  TransformManipulator(const UsdPrim prim, const uint32_t opIndex, ManipulatorMode mode = kGuess, const UsdTimeCode& tc = UsdTimeCode::Default());

  /// re-evaluate the internal coordinate frames on time change
  MAYA_USD_UTILS_PUBLIC
  void UpdateToTime(const UsdTimeCode& tc, UsdGeomXformCache& cache, ManipulatorMode mode = kGuess);
  void UpdateToTime(const UsdTimeCode& tc, ManipulatorMode mode = kGuess) { UsdGeomXformCache cache; UpdateToTime(tc, cache, mode); }

  //--------------------------------------------------------------------------------------------------------------------
  // given the xform op currently assigned to this processor, can we scale, rotate, and/or translate the op? 
  // (In some cases, e.g. matrices, all may be supported)
  //--------------------------------------------------------------------------------------------------------------------

  /// returns true if the current xfrom op can be rotated
  MAYA_USD_UTILS_PUBLIC
  bool CanRotate() const;

  /// returns true if the current xfrom op can be rotated in the local x axis
  bool CanRotateX() const
    { return CanRotate() && op().GetOpType() != UsdGeomXformOp::TypeRotateY && op().GetOpType() != UsdGeomXformOp::TypeRotateZ; }

  /// returns true if the current xfrom op can be rotated in the local y axis
  bool CanRotateY() const
    { return CanRotate() && op().GetOpType() != UsdGeomXformOp::TypeRotateX && op().GetOpType() != UsdGeomXformOp::TypeRotateZ; }

  /// returns true if the current xfrom op can be rotated in the local z axis
  bool CanRotateZ() const
    { return CanRotate() && op().GetOpType() != UsdGeomXformOp::TypeRotateX && op().GetOpType() != UsdGeomXformOp::TypeRotateY; }

  /// returns true if the current xfrom op can be translated
  MAYA_USD_UTILS_PUBLIC
  bool CanTranslate() const;

  /// returns true if the current xfrom op can be scaled
  MAYA_USD_UTILS_PUBLIC
  bool CanScale() const;

  //--------------------------------------------------------------------------------------------------------------------
  // Compute the current transform op value - all values in local space
  //--------------------------------------------------------------------------------------------------------------------

  /// returns the current orientation as a quat (If CanRotate() returns false, the identity quat is returned)
  MAYA_USD_UTILS_PUBLIC
  GfQuatd Rotation() const;

  /// returns the current translation as a vec3 (If CanTranslate() returns false, [0,0,0] is returned)
  MAYA_USD_UTILS_PUBLIC
  GfVec3d Translation() const;

  /// returns the current scale as a vec3 (If CanScale() returns false, [1,1,1] is returned)
  MAYA_USD_UTILS_PUBLIC
  GfVec3d Scale() const;

  //--------------------------------------------------------------------------------------------------------------------
  // Compute the current transform op value. 
  //--------------------------------------------------------------------------------------------------------------------

  /// returns the coordinate frame for this manipulator where the transformation is the identity (e.g. the local origin)
  inline const GfMatrix4d& ManipulatorFrame() const
    { return _coordFrame; }

  /// returns the inclusive matrix of the manipulator frame, and the transformation of the xform op applied 
  inline GfMatrix4d ManipulatorMatrix() const
    { return EvaluateCoordinateFrameForIndex(_ops, _opIndex + 1, _timeCode); }

  /// returns the transform op being manipulated
  inline GfMatrix4d MayaManipulatorMatrix() const
    { return _ops[_opIndex].GetOpTransform(_timeCode); }


  /// returns the current manipulator mode
  MAYA_USD_UTILS_PUBLIC
  ManipulatorMode ManipMode() const;

  //--------------------------------------------------------------------------------------------------------------------
  // Apply relative transformations to the Transform Op
  //--------------------------------------------------------------------------------------------------------------------

  /// apply a translation offset to the xform op
  MAYA_USD_UTILS_PUBLIC
  bool Translate(const GfVec3d& translateChange, Space space = kTransform);

  /// apply a scale offset to the xform op
  MAYA_USD_UTILS_PUBLIC
  bool Scale(const GfVec3d& scaleChange, Space space = kTransform);

  /// apply a rotational offset to the X axis
  MAYA_USD_UTILS_PUBLIC
  bool RotateX(const double radianChange, Space space = kTransform);

  /// apply a rotational offset to the Y axis
  MAYA_USD_UTILS_PUBLIC
  bool RotateY(const double radianChange, Space space = kTransform);

  /// apply a rotational offset to the Z axis
  MAYA_USD_UTILS_PUBLIC
  bool RotateZ(const double radianChange, Space space = kTransform);

  /// apply a rotational offset to the xform op. 
  /// NOTE: This is primarily useful for rotating objects via the sphere (rather than axis rings of the rotate manip)
  /// It's likely that using this method won't result in 'nice' eulers afterwards. 
  /// If you want 'nice' eulers (as much as is possible with a rotate tool), then prefer to use the axis rotation 
  /// methods, RotateX etc. 
  /// It should also be noted that this method may end up being called by the RotateX/RotateY/RotateZ methods if 
  /// either the rotation is not a simple one - i.e. a simple RotateX xform op 
  MAYA_USD_UTILS_PUBLIC
  bool Rotate(const GfQuatd& quatChange, Space space);

  //--------------------------------------------------------------------------------------------------------------------
  // Query Coordinate frames
  //--------------------------------------------------------------------------------------------------------------------

  /// return the world space coordinate frame for the transform op (i.e. PreviousOpsInStack * ParentWorld)
  const GfMatrix4d& WorldFrame() const 
    { return _worldFrame; }

  /// return the parent space coordinate frame for the transform op
  const GfMatrix4d& ParentFrame() const 
    { return _parentFrame; }

  /// return the combined transform ops that appear after the xform op being modified
  const GfMatrix4d& PostTransformFrame() const 
    { return _postFrame; }

  /// return the coordinate frame for the transform op - i.e. the 'origin' for the manipulator
  const GfMatrix4d& CoordinateFrame() const 
    { return _coordFrame; }

  /// return the inverted coordinate frame for the transform op - i.e. the 'origin' for the manipulator
  const GfMatrix4d& InvCoordinateFrame() const 
    { return _invCoordFrame; }

  /// return the combined inverted transform ops that appear after the xform op being modified
  const GfMatrix4d& InvPostTransformFrame() const 
    { return _invPostFrame; }

  const std::vector<UsdGeomXformOp>& ops() const { return _ops; }
  UsdGeomXformOp op() const { return _ops[_opIndex]; }
  uint32_t opIndex() const { return _opIndex; }

  //--------------------------------------------------------------------------------------------------------------------
  // Methods used for Python bindings only
  //--------------------------------------------------------------------------------------------------------------------

  GfMatrix4d _WorldFrame() const 
    { return _worldFrame; }
  GfMatrix4d _ParentFrame() const 
    { return _parentFrame; }
  GfMatrix4d _PostTransformFrame() const 
    { return _postFrame; }
  GfMatrix4d _CoordinateFrame() const 
    { return _coordFrame; }
  GfMatrix4d _InvCoordinateFrame() const 
    { return _invCoordFrame; }
  GfMatrix4d _InvPostTransformFrame() const 
    { return _invPostFrame; }
  std::vector<UsdGeomXformOp> _GetOps() const { return _ops; }
  UsdGeomXformOp _GetOp() const { return _ops[_opIndex]; }
  uint32_t _GetOpIndex() const { return _opIndex; }

protected:
  void _Construct(const TfToken opName, const UsdTimeCode& tc);
  GfMatrix4d _coordFrame;
  GfMatrix4d _worldFrame;
  GfMatrix4d _parentFrame;
  GfMatrix4d _postFrame;
  GfMatrix4d _invCoordFrame;
  GfMatrix4d _invWorldFrame;
  GfMatrix4d _invPostFrame;
  d256 _qcoordFrame;
  d256 _qworldFrame;
  d256 _qparentFrame;
  d256 _qpostFrame;
  std::vector<UsdGeomXformOp> _ops;
  uint32_t _opIndex;
  UsdTimeCode _timeCode = UsdTimeCode::Default();
  UsdPrim _prim;
  ManipulatorMode _manipMode;
  bool _resetsXformStack = false;
};

} // MayaUsdUtils

#endif
