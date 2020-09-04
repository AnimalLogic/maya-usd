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

#include "SIMD.h"
#include "TransformManipulatorEx.h"
#include "TransformMaths.h"

#include <pxr/usd/usdGeom/xform.h>
#include <pxr/base/gf/rotation.h>
#include <iostream>

namespace MayaUsdUtils {

//----------------------------------------------------------------------------------------------------------------------------------------------------------
bool TransformManipulatorEx::SetTranslate(const GfVec3d& position, Space space)
{
  if(CanTranslate())
  {
    d256 translate = _Translation(op(), _timeCode);
    if(space == kTransform)
    {
      d256 offset = sub4d(set4d(position[0], position[1], position[2], 0), translate);
      return TransformManipulator::Translate(GfVec3d(get<0>(offset), get<1>(offset), get<2>(offset)), space);
    }
    if(space == kWorld)
    {
      d256* pworldFrame = (d256*)&_worldFrame;
      d256 worldPos = transform(translate, pworldFrame);
      d256 world_offset = sub4d(set4d(position[0], position[1], position[2], 1.0), worldPos); 
      return TransformManipulator::Translate(GfVec3d(get<0>(world_offset), get<1>(world_offset), get<2>(world_offset)), kWorld);
    }
    if(space == kPreTransform)
    {
      d256* pcoordFrame = (d256*)&_coordFrame;
      d256 worldPos = transform(translate, pcoordFrame);
      d256 parent_offset = sub4d(set4d(position[0], position[1], position[2], 1.0), worldPos); 
      return TransformManipulator::Translate(GfVec3d(get<0>(parent_offset), get<1>(parent_offset), get<2>(parent_offset)), kPreTransform);
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
bool TransformManipulatorEx::SetScale(const GfVec3d& scale, Space space)
{
  if(CanScale())
  {
    d256 original = _Scale(op(), _timeCode);
    d256 offset = div4d(set4d(scale[0], scale[1], scale[2], 0.0), original); 
    return TransformManipulator::Scale(GfVec3d(get<0>(offset), get<1>(offset), get<2>(offset)), kWorld);
  }
  return false;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
bool TransformManipulatorEx::SetRotate(const GfQuatd& orientation, Space space)
{
  if(CanRotate())
  {
    d256 final_orient = loadu4d(&orientation);
    d256 rotate = _Rotation(op(), _timeCode);
    if(space == kTransform)
    {
      d256 offset = multiplyQuat(quatInvert(rotate), final_orient);
      GfQuatd Q(get<3>(offset), get<0>(offset), get<1>(offset), get<2>(offset));
      double* D = (double*)&Q;
      return TransformManipulator::Rotate(Q, space);
    }
    if(space == kWorld)
    {
      d256 world_rotate = multiplyQuat(quatInvert(rotate), _qworldFrame);
      d256 world_offset = multiplyQuat(world_rotate, final_orient);
      return TransformManipulator::Rotate(GfQuatd(get<3>(world_offset), get<0>(world_offset), get<1>(world_offset), get<2>(world_offset)), space);
    }
    if(space == kPreTransform)
    {
      d256 world_rotate = multiplyQuat(quatInvert(rotate), _qcoordFrame);
      d256 world_offset = multiplyQuat(world_rotate, final_orient);
      return TransformManipulator::Rotate(GfQuatd(get<3>(world_offset), get<0>(world_offset), get<1>(world_offset), get<2>(world_offset)), space);
    }
  }
  return false;
}

bool TransformManipulatorEx::Translate(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfVec3d& translateChange, Space space)
{
  TransformManipulatorEx proc(prim, rotateAttr, TransformManipulatorEx::kTranslate, timeCode);
  return proc.Translate(translateChange, space);
}

bool TransformManipulatorEx::Scale(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfVec3d& scaleChange, Space space)
{
  TransformManipulatorEx proc(prim, rotateAttr, TransformManipulatorEx::kScale, timeCode);
  return proc.Scale(scaleChange, space);
}

bool TransformManipulatorEx::RotateX(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const double radianChange, Space space)
{
  TransformManipulatorEx proc(prim, rotateAttr, TransformManipulatorEx::kRotate, timeCode);
  return proc.RotateX(radianChange, space);
}

bool TransformManipulatorEx::RotateY(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const double radianChange, Space space)
{
  TransformManipulatorEx proc(prim, rotateAttr, TransformManipulatorEx::kRotate, timeCode);
  return proc.RotateY(radianChange, space);
}

bool TransformManipulatorEx::RotateZ(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const double radianChange, Space space)
{
  TransformManipulatorEx proc(prim, rotateAttr, TransformManipulatorEx::kRotate, timeCode);
  return proc.RotateZ(radianChange, space);
}

bool TransformManipulatorEx::Rotate(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfQuatd& quatChange, Space space)
{
  TransformManipulatorEx proc(prim, rotateAttr, TransformManipulatorEx::kTranslate, timeCode);
  return proc.Rotate(quatChange, space);
}

bool TransformManipulatorEx::SetTranslate(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfVec3d& position, Space space)
{
  TransformManipulatorEx proc(prim, rotateAttr, TransformManipulatorEx::kTranslate, timeCode);
  return proc.SetTranslate(position, space);
}

bool TransformManipulatorEx::SetScale(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfVec3d& scale, Space space)
{
  TransformManipulatorEx proc(prim, rotateAttr, TransformManipulatorEx::kTranslate, timeCode);
  return proc.SetScale(scale, space);
}

bool TransformManipulatorEx::SetRotate(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfQuatd& orientation, Space space)
{
  TransformManipulatorEx proc(prim, rotateAttr, TransformManipulatorEx::kRotate, timeCode);
  return proc.SetRotate(orientation, space);
}


} // MayaUsdUtils
