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

#include "SIMD.h"
#include "TransformManipulator.h"
#include "TransformMaths.h"

#include <pxr/usd/usdGeom/xform.h>
#include <pxr/base/gf/rotation.h>
#include <iostream>

namespace MayaUsdUtils {

TfToken TransformManipulator::primaryRotateSuffix;
TfToken TransformManipulator::primaryScaleSuffix;
TfToken TransformManipulator::primaryTranslateSuffix;

//----------------------------------------------------------------------------------------------------------------------------------------------------------
MAYA_USD_UTILS_PUBLIC
TransformManipulator::TransformManipulator(const UsdPrim prim, const TfToken opName, const TransformManipulator::ManipulatorMode mode, const UsdTimeCode& tc)
  : _prim(prim)
{
  _parentFrame.SetIdentity();
  _manipMode = mode;
  _ops = UsdGeomXformable(prim).GetOrderedXformOps(&_resetsXformStack);
  _Construct(opName, tc);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
void TransformManipulator::_Construct(const TfToken opName, const UsdTimeCode& tc)
{
  const size_t opCount = _ops.size();
  const TfToken empty(""); 
  if(opName == empty)
  {
    switch(_manipMode)
    {
    case TransformManipulator::kRotate:
      {
        if(!primaryRotateSuffix.IsEmpty())
        {
          for(_opIndex = 0; _opIndex < opCount; ++_opIndex)
          {
            // early out for user specified suffix
            if(_ops[_opIndex].HasSuffix(primaryRotateSuffix))
            {
              auto type = _ops[_opIndex].GetOpType();
              if(type != UsdGeomXformOp::TypeTransform &&
                type != UsdGeomXformOp::TypeTranslate &&
                type != UsdGeomXformOp::TypeScale)
              { 
                break;
              }
            }
          }
        }

        // if no special case found
        if(_opIndex == opCount || primaryRotateSuffix.IsEmpty())
        {
          for(_opIndex = 0; _opIndex < opCount; ++_opIndex)
          {
            // if we have an op with no suiffix, use that
            if(_ops[_opIndex].GetOpName() == TfToken("xformOp:orient") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:rotateX") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:rotateY") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:rotateZ") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:rotateXYZ") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:rotateXZY") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:rotateYXZ") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:rotateYZX") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:rotateZXY") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:rotateZYX") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:rotateXYZ:rotate") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:rotateXZY:rotate") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:rotateYXZ:rotate") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:rotateYZX:rotate") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:rotateZXY:rotate") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:rotateZYX:rotate"))
            {
              break;
            }
          }
          // if nothing found, see if we have a matrix we can modify
          if(_opIndex == opCount)
          {
            for(_opIndex = 0; _opIndex < opCount; ++_opIndex)
            {
              if(_ops[_opIndex].GetOpType() == UsdGeomXformOp::TypeTransform)
                break;
            }
          }
        }
      }
      break;

    case TransformManipulator::kScale:
      {
        if(!primaryScaleSuffix.IsEmpty())
        {
          for(_opIndex = 0; _opIndex < opCount; ++_opIndex)
          {
            // early out for user specified suffix
            if(_ops[_opIndex].HasSuffix(primaryScaleSuffix))
            {
              auto type = _ops[_opIndex].GetOpType();
              if(type == UsdGeomXformOp::TypeScale || type == UsdGeomXformOp::TypeTransform)
              { 
                break;
              }
            }
          }
        }
        // if no special case found
        if(_opIndex == opCount || primaryScaleSuffix.IsEmpty())
        {
          for(_opIndex = 0; _opIndex < opCount; ++_opIndex)
          {
            if(_ops[_opIndex].GetOpName() == TfToken("xformOp:scale") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:scale:scale"))
            {
              break;
            }
          }
          // if nothing found, see if we have a matrix we can modify
          if(_opIndex == opCount)
          {
            for(_opIndex = 0; _opIndex < opCount; ++_opIndex)
            {
              if(_ops[_opIndex].GetOpType() == UsdGeomXformOp::TypeTransform)
                break;
            }
          }
        }
      }
      break;
    case TransformManipulator::kTranslate:
      {
        if(!primaryTranslateSuffix.IsEmpty())
        {
          for(_opIndex = 0; _opIndex < opCount; ++_opIndex)
          {
            // early out for user specified suffix
            if(_ops[_opIndex].HasSuffix(primaryTranslateSuffix))
            {
              auto type = _ops[_opIndex].GetOpType();
              if(type == UsdGeomXformOp::TypeTranslate || type == UsdGeomXformOp::TypeTransform) 
              { 
                break;
              }
            }
          }
        }

        // if no special case found
        if(_opIndex == opCount || primaryTranslateSuffix.IsEmpty())
        {
          for(_opIndex = 0; _opIndex < opCount; ++_opIndex)
          {
            if(_ops[_opIndex].GetOpName() == TfToken("xformOp:translate") || 
               _ops[_opIndex].GetOpName() == TfToken("xformOp:translate:translate"))
            {
              break;
            }
          }
          // if nothing found, see if we have a matrix we can modify
          if(_opIndex == opCount)
          {
            for(_opIndex = 0; _opIndex < opCount; ++_opIndex)
            {
              if(_ops[_opIndex].GetOpType() == UsdGeomXformOp::TypeTransform)
                break;
            }
          }
        }
      }
      break;

    case TransformManipulator::kGuess:
      {
        throw std::runtime_error("Cannot guess the type of an un-named xformOp");
      }
    }
  }
  else
  {
    for(_opIndex = 0; _opIndex < opCount; ++_opIndex)
    {
      if(_ops[_opIndex].GetOpName() == opName)
        break;
    }
  }
  if(_opIndex == opCount)
  {
    throw std::runtime_error(std::string("unable to find xform op on prim: ") + opName.GetString());
  }
  UpdateToTime(tc, _manipMode);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
MAYA_USD_UTILS_PUBLIC
TransformManipulator::TransformManipulator(const UsdPrim prim, const uint32_t opIndex, const TransformManipulator::ManipulatorMode mode, const UsdTimeCode& tc)
  : _opIndex(opIndex), _prim(prim)
{
  _ops = UsdGeomXformable(prim).GetOrderedXformOps(&_resetsXformStack);
  if(_opIndex >= _ops.size())
  {
    throw std::range_error(std::string("invalid op index"));
  }
  UpdateToTime(tc, mode);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
TransformManipulator::ManipulatorMode TransformManipulator::ManipMode() const
{
  return _manipMode; 
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
MAYA_USD_UTILS_PUBLIC
bool TransformManipulator::CanRotate() const
{
  if(const auto xop = op())
  {
    switch(xop.GetOpType())
    {
    case UsdGeomXformOp::TypeTranslate:
    case UsdGeomXformOp::TypeScale:
    case UsdGeomXformOp::TypeInvalid:
      return false;
    default:
      break;
    }
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
MAYA_USD_UTILS_PUBLIC
bool TransformManipulator::CanTranslate() const
{
  if(const auto xop = op())
  {
    switch(xop.GetOpType())
    {
    case UsdGeomXformOp::TypeTransform:
    case UsdGeomXformOp::TypeTranslate:
      return true;
    default:
      break;
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
MAYA_USD_UTILS_PUBLIC
bool TransformManipulator::CanScale() const
{
  if(const auto xop = op())
  {
    switch(xop.GetOpType())
    {
    case UsdGeomXformOp::TypeTransform:
    case UsdGeomXformOp::TypeScale:
      return true;
    default:
      break;
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
MAYA_USD_UTILS_PUBLIC
void TransformManipulator::UpdateToTime(const UsdTimeCode& tc, UsdGeomXformCache& cache, const TransformManipulator::ManipulatorMode mode)
{
  _manipMode = mode;
  _timeCode = tc;

  // grab type of op 
  auto xop = op();
  auto opType = xop.GetOpType();

  // if we are to guess the manipulator mode in use...
  if(_manipMode == kGuess)
  {
    // if the type is a translation (or a matrix - default to possibly wrong here!)
    if(opType == UsdGeomXformOp::TypeTransform || 
       opType == UsdGeomXformOp::TypeTranslate)
    {
      _manipMode = kTranslate;
    }
    else
    // if the op type is a scale
    if(opType == UsdGeomXformOp::TypeScale)
    {
      _manipMode = kScale;
    }
    else
    // otherwise assume a rotation
    {
      _manipMode = kRotate;
    }
  }

  // evaluate the initial guess for the coordinate frame 
  _coordFrame = EvaluateCoordinateFrameForIndex(_ops, _opIndex, _timeCode);
  _postFrame = EvaluateCoordinateFrameForRange(_ops, _opIndex + 1, _ops.size(), _timeCode);
  _invPostFrame = _postFrame.GetInverse();
  _qpostFrame = matrixToQuat((const d256*)&_postFrame);

  // If we have a matrix transformation, depending on the type of manip used, 
  // we may have to offset the coordinate frame slightly
  if(opType == UsdGeomXformOp::TypeTransform)
  {
    switch(_manipMode)
    {
    default:
    case kTranslate:
      /* nothing to do. Translation frame will be correct */
      break;

    case kRotate:
      /* we need to offset the coordinate frame by the translation */
      {
        alignas(32) GfMatrix4d matrix;
        if(xop.Get(&matrix, _timeCode))
        {
          // transform offset by coordinate frame, and apply to coordinate frame
          GfVec4d offset(matrix[3][0], matrix[3][1], matrix[3][2], 0.0);
          offset = offset * _coordFrame;
          _coordFrame[3][0] += offset[0];
          _coordFrame[3][1] += offset[1];
          _coordFrame[3][2] += offset[2];
        }
      }
      break;

    case kScale:
      /* we need to offset the coordinate frame by the translation & rotation */
      {
        d256 matrix[4];
        if(xop.Get((GfMatrix4d*)matrix, _timeCode))
        {
          // orthogonalise matrix to remove scaling
          auto sx = splat4d(std::sqrt(dot3(matrix[0], matrix[0])));
          auto sy = splat4d(std::sqrt(dot3(matrix[1], matrix[1])));
          auto sz = splat4d(std::sqrt(dot3(matrix[2], matrix[2])));
          matrix[0] = div4d(matrix[0], sx);
          matrix[1] = div4d(matrix[1], sy);
          matrix[2] = div4d(matrix[2], sz);
          // offset the coordinate frame
          multiply4x4((d256*)&_coordFrame, matrix, (const d256*)&_coordFrame);
        }
      }
      break;
    }
  }

  // If you happen to be throwing from one of these exceptions, it means the xform op 
  switch(_manipMode)
  {
  default:
  case kTranslate:
    if(!CanTranslate())
    {
      throw std::runtime_error(std::string("Cannot translate transform op: ") + xop.GetName().GetString());
    }
    break;
  case kRotate:
    if(!CanRotate())
    {
      throw std::runtime_error(std::string("Cannot rotate transform op: ") + xop.GetName().GetString());
    }
    break;
  case kScale:
    if(!CanScale())
    {
      throw std::runtime_error(std::string("Cannot scale transform op: ") + xop.GetName().GetString());
    }
    break;
  }

  if(!_resetsXformStack)
  {
    alignas(32) auto _parentFrame = cache.GetParentToWorldTransform(_prim);
    this->_parentFrame = _parentFrame;
    multiply4x4((d256*)&_worldFrame, (const d256*)&_coordFrame, (const d256*)&_parentFrame);
    _invWorldFrame = _worldFrame.GetInverse();
    _invCoordFrame = _coordFrame.GetInverse();
    _qcoordFrame = matrixToQuat((const d256*)&_coordFrame);
    _qworldFrame = matrixToQuat((const d256*)&_worldFrame);
  }
  else
  {
    // set to identity
    const d256 x = set4d(1.0, 0.0, 0.0, 0.0);
    const d256 y = set4d(0.0, 1.0, 0.0, 0.0);
    const d256 z = set4d(0.0, 0.0, 1.0, 0.0);
    const d256 w = set4d(0.0, 0.0, 0.0, 1.0);
    storeu4d(_worldFrame[0], x);
    storeu4d(_invWorldFrame[0], x);
    storeu4d(_worldFrame[1], y);
    storeu4d(_invWorldFrame[1], y);
    storeu4d(_worldFrame[2], z);
    storeu4d(_invWorldFrame[2], z);
    storeu4d(_worldFrame[3], w);
    storeu4d(_invWorldFrame[3], w);
    _qworldFrame = w;
    this->_parentFrame.SetIdentity();
    _invCoordFrame = _coordFrame.GetInverse();
    _qcoordFrame = matrixToQuat((const d256*)&_coordFrame);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
MAYA_USD_UTILS_PUBLIC
bool TransformManipulator::Translate(const GfVec3d& translateChange, const Space space)
{
  auto xformOp = op();
  // check to make sure this transform op can be translated!
  if(_manipMode != kTranslate)
  {
    if(xformOp.GetOpType() == UsdGeomXformOp::TypeTransform)
    {
      // update to time can convert the coordinate frame in this case
      UpdateToTime(_timeCode, kTranslate);
    }
    else
    {
      return false;
    }
  }

  d256 temp = set4d(translateChange[0], translateChange[1], translateChange[2], 0);
  switch(space)
  {
  case kTransform: break;
  case kWorld: temp = transform4d(temp, (const d256*)&_invWorldFrame); break;
  case kPreTransform: temp = transform4d(temp, (const d256*)&_invCoordFrame); break;
  case kPostTransform: temp = transform4d(temp, (const d256*)&_postFrame); break;
  }

  // if the change is close to zero, ignore it. 
  {
    const auto atemp = abs4d(temp);
    const auto eps = splat4d(1e-6);
    if( (movemask4d(cmpgt4d(atemp, eps)) & 0x7) == 0 )
    {
      // technically not a failure. It just doesn't seem worth applying the change...
      return true;
    }
  }

  if(xformOp.GetOpType() == UsdGeomXformOp::TypeTranslate)
  {
    // grab the current value from USD
    d256 original = zero4d();
    const auto precision = op().GetPrecision();
    switch(precision)
    {
    case UsdGeomXformOp::PrecisionDouble:
      {
        xformOp.Get((GfVec3d*)&original, _timeCode);
      }
      break;

    case UsdGeomXformOp::PrecisionFloat:
      {
        f128 v;
        xformOp.Get((GfVec3f*)&v, _timeCode);
        original = cvt4f_to_4d(v);
      }
      break;

    case UsdGeomXformOp::PrecisionHalf:
      {
        i128 v;
        xformOp.Get((GfVec3h*)&v, _timeCode);
        original = cvt4f_to_4d(cvtph4(v));
      }
      break;
    }

    // sum offset
    temp = add4d(temp, original);
    // write back into USD
    switch(precision)
    {
    case UsdGeomXformOp::PrecisionDouble:
      {
        void* ptr = &temp;
        xformOp.Set(*(GfVec3d*)ptr, _timeCode);
      }
      break;

    case UsdGeomXformOp::PrecisionFloat:
      {
        f128 v = cvt4d_to_4f(temp);
        void* ptr = &v;
        xformOp.Set(*(GfVec3f*)ptr, _timeCode);
      }
      break;

    case UsdGeomXformOp::PrecisionHalf:
      {
        i128 v = cvtph4(cvt4d_to_4f(temp));
        void* ptr = &v;
        xformOp.Set(*(GfVec3h*)ptr, _timeCode);
      }
      break;
    }
  }
  else
  if(xformOp.GetOpType() == UsdGeomXformOp::TypeTransform)
  {
    d256 matrix[4] = {
      set4d(1.0, 0.0, 0.0, 0.0),
      set4d(0.0, 1.0, 0.0, 0.0),
      set4d(0.0, 0.0, 1.0, 0.0),
      set4d(0.0, 0.0, 0.0, 1.0)
    };
    void* ptr = matrix;
    xformOp.Get((GfMatrix4d*)ptr, _timeCode);
    matrix[3] = add4d(temp, matrix[3]);
    xformOp.Set(*(GfMatrix4d*)ptr, _timeCode);
  }
  else
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
static inline bool isClose(const double a, const double b)
{
  return std::abs(a - b) < 1e-6f;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
MAYA_USD_UTILS_PUBLIC
bool TransformManipulator::Scale(const GfVec3d& scaleChange, const Space space)
{
  // when in strange coordinate frames, scaling can only be safely applied if the XYZ values are uniform
  switch(space)
  {
  case kTransform: /* no problem here, scale away */ break;
  case kPostTransform: 
    {
      // if the scale is non uniform
      if(!isClose(scaleChange[0], scaleChange[1]) || 
         !isClose(scaleChange[0], scaleChange[2]))
      {
        // then we can still process this, so long as there are no other rotation ops after this scale op
        for(uint32_t i = _opIndex + 1, count = _ops.size(); i < count; ++i)
        {
          switch(_ops[i].GetOpType())
          {
          case UsdGeomXformOp::TypeInvalid: 
          case UsdGeomXformOp::TypeTranslate: 
          case UsdGeomXformOp::TypeScale:
            break;

          default:
            /* must contain a rotation of some kind. I could be pedantic and check to see if identity rotation, but seems overkill */
            return false;
          }
        }
      }
      
    }
    break;
  case kWorld:
  case kPreTransform:
    {
      // we can't really process this without introducing shear, so let's not bother :)
      if(!isClose(scaleChange[0], scaleChange[1]) || 
         !isClose(scaleChange[0], scaleChange[2]))
      {
        return false;
      }
    }
    break;
  }

  auto xformOp = op();
  // check to make sure this transform op can be translated!
  if(_manipMode != kScale)
  {
    if(xformOp.GetOpType() == UsdGeomXformOp::TypeTransform)
    {
      // update to time can convert the coordinate frame in this case
      UpdateToTime(_timeCode, kScale);
    }
    else
    {
      return false;
    }
  }

  d256 temp = set4d(scaleChange[0], scaleChange[1], scaleChange[2], 0.0);

  // if the change is close to one, ignore it. 
  {
    const auto abstemp = abs4d(sub4d(temp, splat4d(1.0)));
    const auto eps = splat4d(1e-6);
    if( (movemask4d(cmpgt4d(abstemp, eps)) & 0x7) == 0 )
    {
      // technically not a failure. It just doesn't seem worth applying the change...
      return true;
    }
  }

  if(xformOp.GetOpType() == UsdGeomXformOp::TypeScale)
  {
    // grab the current value from USD
    d256 original = set4d(1.0, 1.0, 1.0, 0.0);
    const auto precision = xformOp.GetPrecision();
    switch(precision)
    {
    case UsdGeomXformOp::PrecisionDouble:
      {
        xformOp.Get((GfVec3d*)&original, _timeCode);
      }
      break;

    case UsdGeomXformOp::PrecisionFloat:
      {
        f128 v;
        xformOp.Get((GfVec3f*)&v, _timeCode);
        original = cvt4f_to_4d(v);
      }
      break;

    case UsdGeomXformOp::PrecisionHalf:
      {
        i128 v;
        xformOp.Get((GfVec3h*)&v, _timeCode);
        original = cvt4f_to_4d(cvtph4(v));
      }
      break;
    }

    // multiply scaling
    temp = mul4d(temp, original);

    // write back into USD
    switch(precision)
    {
    case UsdGeomXformOp::PrecisionDouble:
      {
        void* ptr = &temp;
        xformOp.Set(*(GfVec3d*)ptr, _timeCode);
      }
      break;

    case UsdGeomXformOp::PrecisionFloat:
      {
        f128 v = cvt4d_to_4f(temp);
        void* ptr = &v;
        xformOp.Set(*(GfVec3f*)ptr, _timeCode);
      }
      break;

    case UsdGeomXformOp::PrecisionHalf:
      {
        i128 v = cvtph4(cvt4d_to_4f(temp));
        void* ptr = &v;
        xformOp.Set(*(GfVec3h*)ptr, _timeCode);
      }
      break;
    }
  }
  else
  if(xformOp.GetOpType() == UsdGeomXformOp::TypeTransform)
  {
    d256 matrix[4] = {
      set4d(1.0, 0.0, 0.0, 0.0),
      set4d(0.0, 1.0, 0.0, 0.0),
      set4d(0.0, 0.0, 1.0, 0.0),
      set4d(0.0, 0.0, 0.0, 1.0)
    };
    void* ptr = matrix;
    xformOp.Get((GfMatrix4d*)ptr, _timeCode);
    matrix[0] = mul4d(permute4d<0,0,0,0>(temp), matrix[0]);
    matrix[1] = mul4d(permute4d<1,1,1,1>(temp), matrix[1]);
    matrix[2] = mul4d(permute4d<2,2,2,2>(temp), matrix[2]);
    xformOp.Set(*(GfMatrix4d*)ptr, _timeCode);
  }
  else
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
MAYA_USD_UTILS_PUBLIC
bool TransformManipulator::Rotate(const GfQuatd& quatChange, Space space)
{
  d256 temp = set4d(quatChange.GetImaginary()[0], quatChange.GetImaginary()[1], quatChange.GetImaginary()[2], quatChange.GetReal());

  // if the change is close to zero, ignore it. 
  {
    const auto atemp = abs4d(sub4d(temp, set4d(0.0, 0.0, 0.0, 1.0)));
    const auto eps = splat4d(1e-6);
    if(!movemask4d(cmpgt4d(atemp, eps)))
    {
      // technically not a failure. It just doesn't seem worth applying an identity rotation
      return true;
    }
  }

  auto xformOp = op();
  // check to make sure this transform op can be rotated!
  if(_manipMode != kRotate)
  {
    if(xformOp.GetOpType() == UsdGeomXformOp::TypeTransform)
    {
      // update to time can convert the coordinate frame in this case
      UpdateToTime(_timeCode, kRotate);
    }
    else
    {
      return false;
    }
  }

  // a utility method that applies an rotational offset to the original rotation quaternion, 
  // in the correct coordinate frame (e.g. local, parent, world), and returns the resulting
  // euler angle triplet
  auto processMatrixRotation = [this] (d256 offset, const Space space)
  {
    switch(space)
    {
    default:
      {
        // grab original matrix
        d256 matrix[4] = {
          set4d(1.0, 0.0, 0.0, 0.0),
          set4d(0.0, 1.0, 0.0, 0.0),
          set4d(0.0, 0.0, 1.0, 0.0),
          set4d(0.0, 0.0, 0.0, 1.0)
        };
        void* ptr = matrix;
        op().Get((GfMatrix4d*)ptr, _timeCode);
        d256 rmatrix[4];
        quatToMatrix(offset, rmatrix);
        matrix[0] = rotate(matrix[0], rmatrix);
        matrix[1] = rotate(matrix[1], rmatrix);
        matrix[2] = rotate(matrix[2], rmatrix);
        op().Set(*(GfMatrix4d*)ptr, _timeCode);
      }
      break;
    case kWorld:
      {
        // grab original matrix
        d256 omatrix[4] = {
          set4d(1.0, 0.0, 0.0, 0.0),
          set4d(0.0, 1.0, 0.0, 0.0),
          set4d(0.0, 0.0, 1.0, 0.0),
          set4d(0.0, 0.0, 0.0, 1.0)
        };
        op().Get((GfMatrix4d*)omatrix, _timeCode);

        d256 rotateMatrix[4], rmatrix[4] = {loadu4d(_worldFrame[0]), loadu4d(_worldFrame[1]), loadu4d(_worldFrame[2]), zero4d()};
        quatToMatrix(offset, rotateMatrix);
        d256 translateToAddBackIn = omatrix[3];
        rotateMatrix[3] = omatrix[3] = zero4d();
        multiply(rmatrix, omatrix, rmatrix);
        // remove translation
        rmatrix[3] = zero4d();
        multiply(rotateMatrix, rmatrix, rotateMatrix);
        // add translation back in here
        rotateMatrix[3] = loadu4d(_worldFrame[3]);
        // bang into local space
        multiply4x4(rmatrix, rotateMatrix, (d256*)&_invWorldFrame);
        rmatrix[3] = translateToAddBackIn;
        void* ptr = rmatrix;
        op().Set(*(GfMatrix4d*)ptr, _timeCode);
      }
      break;

    case kPreTransform:
      {
        // grab original matrix
        d256 omatrix[4] = {
          set4d(1.0, 0.0, 0.0, 0.0),
          set4d(0.0, 1.0, 0.0, 0.0),
          set4d(0.0, 0.0, 1.0, 0.0),
          set4d(0.0, 0.0, 0.0, 1.0)
        };
        op().Get((GfMatrix4d*)omatrix, _timeCode);

        d256 rotateMatrix[4], rmatrix[4] = {loadu4d(_coordFrame[0]), loadu4d(_coordFrame[1]), loadu4d(_coordFrame[2]), zero4d()};
        quatToMatrix(offset, rotateMatrix);
        d256 translateToAddBackIn = omatrix[3];
        rotateMatrix[3] = omatrix[3] = zero4d();
        multiply(rmatrix, omatrix, rmatrix);
        // remove translation
        rmatrix[3] = zero4d();
        multiply(rotateMatrix, rmatrix, rotateMatrix);
        // add translation back in here
        rotateMatrix[3] = loadu4d(_coordFrame[3]);
        // bang into local space
        multiply4x4(rmatrix, rotateMatrix, (d256*)&_invWorldFrame);
        rmatrix[3] = translateToAddBackIn;
        void* ptr = rmatrix;
        op().Set(*(GfMatrix4d*)ptr, _timeCode);
      }
      break;

    case kPostTransform:
      {
        // grab original matrix
        d256 omatrix[4] = {
          set4d(1.0, 0.0, 0.0, 0.0),
          set4d(0.0, 1.0, 0.0, 0.0),
          set4d(0.0, 0.0, 1.0, 0.0),
          set4d(0.0, 0.0, 0.0, 1.0)
        };
        op().Get((GfMatrix4d*)omatrix, _timeCode);
        //d256 rotateMatrix[4],  rmatrix[4] = {loadu4d(_postFrame[0]), loadu4d(_postFrame[1]), loadu4d(_postFrame[2]), zero4d()};
        //quatToMatrix(offset, rotateMatrix);
        //multiply(rotateMatrix, rmatrix, rotateMatrix);
        //multiply(rmatrix, omatrix, rotateMatrix);

      }
      break;
      
    }
  };
  
  // a utility method that applies an rotational offset to the original rotation quaternion, 
  // in the correct coordinate frame (e.g. local, parent, world), and returns the resulting
  // euler angle triplet
  auto process3AxisRotation = [this] (d256 original, d256 offset, const Space space, const RotationOrder order)
  {
    GfVec3f rot(0);
    switch(space)
    {
    default:
      {
        d256 new_rotation = multiplyQuat(original, offset);
        extractEuler(new_rotation, order, rot);
      }
      break;
    case kWorld:
      {
        d256 rotateMatrix[4], rmatrix[4] = {loadu4d(_worldFrame[0]), loadu4d(_worldFrame[1]), loadu4d(_worldFrame[2]), zero4d()}, omatrix[4];
        quatToMatrix(offset, rotateMatrix);
        quatToMatrix(original, omatrix);
        rotateMatrix[3] = omatrix[3] = zero4d();
        multiply(rmatrix, omatrix, rmatrix);
        // remove translation
        rmatrix[3] = zero4d();
        multiply(rotateMatrix, rmatrix, rotateMatrix);
        // add translation back in here
        rotateMatrix[3] = loadu4d(_worldFrame[3]);
        multiply4x4(rmatrix, rotateMatrix, (d256*)&_invWorldFrame);
        d256 ctest = cross(rmatrix[0], rmatrix[1]);
        if(dot3(ctest, rmatrix[2]) < 0)
          rmatrix[2] = sub4d(zero4d(), rmatrix[2]);
        extractEuler(rmatrix, order , rot);
      }
      break;
    case kPreTransform:
      {
        d256 rotateMatrix[4], rmatrix[4] = {loadu4d(_coordFrame[0]), loadu4d(_coordFrame[1]), loadu4d(_coordFrame[2]), zero4d()}, omatrix[4];
        quatToMatrix(offset, rotateMatrix);
        quatToMatrix(original, omatrix);
        rotateMatrix[3] = omatrix[3] = zero4d();
        multiply(rmatrix, omatrix, rmatrix);
        // remove translation
        rmatrix[3] = zero4d();
        multiply(rotateMatrix, rmatrix, rotateMatrix);
        rotateMatrix[3] = loadu4d(_worldFrame[3]);
        multiply4x4(rmatrix, rotateMatrix, (d256*)&_invCoordFrame);
        d256 ctest = cross(rmatrix[0], rmatrix[1]);
        if(dot3(ctest, rmatrix[2]) < 0)
          rmatrix[2] = sub4d(zero4d(), rmatrix[2]);
        extractEuler(rmatrix, order , rot);
      }
      break;
    case kPostTransform:
      {
        d256 rotateMatrix[4], frameMatrix[4] = {loadu4d(_postFrame[0]), loadu4d(_postFrame[1]), loadu4d(_postFrame[2]), zero4d()}, originalMatrix[4];
        quatToMatrix(offset, rotateMatrix);
        quatToMatrix(original, originalMatrix);
        rotateMatrix[3] = originalMatrix[3] = zero4d();
        // compute object space rotation
        multiply(frameMatrix, frameMatrix, originalMatrix);
        // apply child rotation
        multiply(rotateMatrix, rotateMatrix, frameMatrix);
        // remove effect of post xform ops 
        multiply4x4(rotateMatrix, (d256*)&_invPostFrame, rotateMatrix);
        d256 ctest = cross(rotateMatrix[0], rotateMatrix[1]);
        if(dot3(ctest, rotateMatrix[2]) < 0)
          rotateMatrix[2] = sub4d(zero4d(), rotateMatrix[2]);
        extractEuler(rotateMatrix, order , rot);
      }
      break;
      
    }
    // convert back to degrees
    return rot * (180.0f / M_PI);
  };

  // a utility method that applies an rotational offset to the original rotation quaternion, 
  // in the correct coordinate frame (e.g. local, parent, world), and returns the new rotation 
  // as a quaternion
  auto process1AxisRotation = [this] (const d256 original, d256 offset, const Space space)
  {
    d256 new_rotation;
    // convert rotation offset into the correct coordinate frame for the transformation
    switch(space)
    {
    default:
    case kTransform:
      new_rotation = multiplyQuat(original, offset);
      break;
    case kWorld:
      new_rotation = multiplyQuat(_qworldFrame, original);
      new_rotation = multiplyQuat(offset, new_rotation);
      new_rotation = multiplyQuat(quatInvert(_qworldFrame), new_rotation);
      break;
    case kPreTransform:
      new_rotation = multiplyQuat(_qcoordFrame, original);
      new_rotation = multiplyQuat(offset, new_rotation);
      new_rotation = multiplyQuat(quatInvert(_qcoordFrame), new_rotation);
      break;
    case kPostTransform:
      new_rotation = multiplyQuat(original, multiplyQuat(_qpostFrame, offset));
      break;
    }
    return new_rotation;
  };

  const auto precision = xformOp.GetPrecision();
  switch(xformOp.GetOpType())
  {
  case UsdGeomXformOp::TypeTransform:
    {
      processMatrixRotation(temp, space);
    }
    break;

  case UsdGeomXformOp::TypeRotateX:
    {
      double original = 0;
      switch(precision)
      {
      case UsdGeomXformOp::PrecisionDouble:
        {
          xformOp.Get(&original, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          float foriginal = 0;
          xformOp.Get(&foriginal, _timeCode);
          original = double(foriginal);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          GfHalf foriginal = 0;
          xformOp.Get(&foriginal, _timeCode);
          original = double(float(foriginal));
        }
        break;
      }
      // convert to half angle radians
      auto half_angle = degreesToHalfAngle(original);
      auto orig_quat = set4d(std::sin(half_angle), 0, 0, std::cos(half_angle));
      auto new_rotation = process1AxisRotation(orig_quat, temp, space);
      
      // and back to degrees
      const double xrotate = halfAngleToDegrees(std::atan2(get<0>(new_rotation), get<3>(new_rotation)));
      original = xrotate;

      switch(precision)
      {
      case UsdGeomXformOp::PrecisionDouble:
        xformOp.Set(original, _timeCode);
        break;

      case UsdGeomXformOp::PrecisionFloat:
        xformOp.Set(float(original), _timeCode);
        break;

      case UsdGeomXformOp::PrecisionHalf:
        xformOp.Set(GfHalf(float(original)), _timeCode);
        break;
      }
    }
    break;

  case UsdGeomXformOp::TypeRotateY:
    {
      double original = 0;
      switch(precision)
      {
      case UsdGeomXformOp::PrecisionDouble:
        {
          xformOp.Get(&original, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          float foriginal = 0;
          xformOp.Get(&foriginal, _timeCode);
          original = double(foriginal);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          GfHalf foriginal = 0;
          xformOp.Get(&foriginal, _timeCode);
          original = double(float(foriginal));
        }
        break;
      }
      // convert to half angle radians
      auto half_angle = degreesToHalfAngle(original);
      auto orig_quat = set4d(0, std::sin(half_angle), 0, std::cos(half_angle));
      auto new_rotation = process1AxisRotation(orig_quat, temp, space);
      
      // and back to degrees
      const double yrotate = halfAngleToDegrees(std::atan2(get<1>(new_rotation), get<3>(new_rotation)));
      original = yrotate;

      switch(precision)
      {
      case UsdGeomXformOp::PrecisionDouble:
        xformOp.Set(original, _timeCode);
        break;

      case UsdGeomXformOp::PrecisionFloat:
        xformOp.Set(float(original), _timeCode);
        break;

      case UsdGeomXformOp::PrecisionHalf:
        xformOp.Set(GfHalf(float(original)), _timeCode);
        break;
      }
    }
    break;

  case UsdGeomXformOp::TypeRotateZ:
    {
      double original = 0;
      switch(precision)
      {
      case UsdGeomXformOp::PrecisionDouble:
        {
          xformOp.Get(&original, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          float foriginal = 0;
          xformOp.Get(&foriginal, _timeCode);
          original = double(foriginal);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          GfHalf foriginal = 0;
          xformOp.Get(&foriginal, _timeCode);
          original = double(float(foriginal));
        }
        break;
      }
      // convert to half angle radians
      auto half_angle = degreesToHalfAngle(original);
      auto orig_quat = set4d(0, 0, std::sin(half_angle), std::cos(half_angle));
      auto new_rotation = process1AxisRotation(orig_quat, temp, space);
      
      // and back to degrees
      const double zrotate = halfAngleToDegrees(std::atan2(get<2>(new_rotation), get<3>(new_rotation)));
      original = zrotate;

      switch(precision)
      {
      case UsdGeomXformOp::PrecisionDouble:
        xformOp.Set(original, _timeCode);
        break;

      case UsdGeomXformOp::PrecisionFloat:
        xformOp.Set(float(original), _timeCode);
        break;

      case UsdGeomXformOp::PrecisionHalf:
        xformOp.Set(GfHalf(float(original)), _timeCode);
        break;
      }
    }
    break;

  case UsdGeomXformOp::TypeRotateXYZ:
    {
      switch(precision)
      {
      case UsdGeomXformOp::PrecisionDouble:
        {
          d256 original = zero4d();
          xformOp.Get((GfVec3d*)&original, _timeCode);
          original = Quat_from_EulerXYZ(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kXYZ);
          xformOp.Set(GfVec3d(rot), _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          f128 forig = zero4f();
          xformOp.Get((GfVec3f*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(forig);
          original = Quat_from_EulerXYZ(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kXYZ);
          xformOp.Set(GfVec3f(rot), _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          i128 forig = zero4i();
          xformOp.Get((GfVec3h*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(cvtph4(forig));
          original = Quat_from_EulerXYZ(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kXYZ);
          xformOp.Set(GfVec3h(rot), _timeCode);
        }
        break;
      }
    }
    break;

  case UsdGeomXformOp::TypeRotateXZY:
    {
      switch(precision)
      {
      case UsdGeomXformOp::PrecisionDouble:
        {
          d256 original = zero4d();
          xformOp.Get((GfVec3d*)&original, _timeCode);
          original = Quat_from_EulerXZY(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kXZY);
          xformOp.Set(GfVec3d(rot), _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          f128 forig = zero4f();
          xformOp.Get((GfVec3f*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(forig);
          original = Quat_from_EulerXZY(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kXZY);
          xformOp.Set(rot, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          i128 forig = zero4i();
          xformOp.Get((GfVec3h*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(cvtph4(forig));
          original = Quat_from_EulerXZY(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kXZY);
          xformOp.Set(GfVec3h(rot), _timeCode);
        }
        break;
      }
    }
    break;

  case UsdGeomXformOp::TypeRotateYXZ:
    {
      switch(precision)
      {
      case UsdGeomXformOp::PrecisionDouble:
        {
          d256 original = zero4d();
          xformOp.Get((GfVec3d*)&original, _timeCode);
          original = Quat_from_EulerYXZ(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kYXZ);
          xformOp.Set(GfVec3d(rot), _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          f128 forig = zero4f();
          xformOp.Get((GfVec3f*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(forig);
          original = Quat_from_EulerYXZ(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kYXZ);
          xformOp.Set(rot, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          i128 forig = zero4i();
          xformOp.Get((GfVec3h*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(cvtph4(forig));
          original = Quat_from_EulerYXZ(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kYXZ);
          xformOp.Set(GfVec3h(rot), _timeCode);
        }
        break;
      }
    }
    break;

  case UsdGeomXformOp::TypeRotateYZX:
    {
      switch(precision)
      {
      case UsdGeomXformOp::PrecisionDouble:
        {
          d256 original = zero4d();
          xformOp.Get((GfVec3d*)&original, _timeCode);
          original = Quat_from_EulerYZX(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kYZX);
          xformOp.Set(GfVec3d(rot), _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          f128 forig = zero4f();
          xformOp.Get((GfVec3f*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(forig);
          original = Quat_from_EulerYZX(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kYZX);
          xformOp.Set(rot, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          i128 forig = zero4i();
          xformOp.Get((GfVec3h*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(cvtph4(forig));
          original = Quat_from_EulerYZX(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kYZX);
          xformOp.Set(GfVec3h(rot), _timeCode);
        }
        break;
      }
    }
    break;

  case UsdGeomXformOp::TypeRotateZXY:
    {
      switch(precision)
      {
      case UsdGeomXformOp::PrecisionDouble:
        {
          d256 original = zero4d();
          xformOp.Get((GfVec3d*)&original, _timeCode);
          original = Quat_from_EulerZXY(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kZXY);
          xformOp.Set(GfVec3d(rot), _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          f128 forig = zero4f();
          xformOp.Get((GfVec3f*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(forig);
          original = Quat_from_EulerZXY(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kZXY);
          xformOp.Set(rot, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          i128 forig = zero4i();
          xformOp.Get((GfVec3h*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(cvtph4(forig));
          original = Quat_from_EulerZXY(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kZXY);
          xformOp.Set(GfVec3h(rot), _timeCode);
        }
        break;
      }
    }
    break;

  case UsdGeomXformOp::TypeRotateZYX:
    {
      switch(precision)
      {
      case UsdGeomXformOp::PrecisionDouble:
        {
          d256 original = zero4d();
          xformOp.Get((GfVec3d*)&original, _timeCode);
          original = Quat_from_EulerZYX(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kZYX);
          xformOp.Set(GfVec3d(rot), _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          f128 forig = zero4f();
          xformOp.Get((GfVec3f*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(forig);
          original = Quat_from_EulerZYX(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kZYX);
          xformOp.Set(rot, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          i128 forig = zero4i();
          xformOp.Get((GfVec3h*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(cvtph4(forig));
          original = Quat_from_EulerZYX(degreesToHalfAngle(original));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kZYX);
          xformOp.Set(GfVec3h(rot), _timeCode);
        }
        break;
      }
    }
    break;

  case UsdGeomXformOp::TypeOrient:
    {
      switch(precision)
      {
      case UsdGeomXformOp::PrecisionDouble:
        {
          d256 original = set4d(0, 0, 0, 1.0);
          xformOp.Get((GfQuatd*)&original, _timeCode);
          original = multiplyQuat(original, temp);
          void* ptr = &original;
          xformOp.Set(*(GfQuatd*)ptr, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          f128 original = set4f(0, 0, 0, 1.0f);
          void* ptr = &original;
          xformOp.Get((GfQuatf*)ptr, _timeCode);
          original = cvt4d_to_4f(multiplyQuat(cvt4f_to_4d(original), temp));
          xformOp.Set(*(GfQuatf*)ptr, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          i128 original = zero4i();
          void* ptr = &original;
          xformOp.Get((GfQuath*)ptr, _timeCode);
          original = cvtph4(cvt4d_to_4f(multiplyQuat(cvt4f_to_4d(cvtph4(original)), temp)));
          xformOp.Set(*(GfQuath*)ptr, _timeCode);
        }
        break;
      }
    }
    break;
    
  // unsupported transform op type
  default:
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
MAYA_USD_UTILS_PUBLIC
bool TransformManipulator::RotateX(const double radianChange, Space space)
{
  if(space == kTransform)
  {
    auto xformOp = op();
    const auto precision = xformOp.GetPrecision();
    switch(xformOp.GetOpType())
    {
    case UsdGeomXformOp::TypeRotateX:
      {
        switch(precision)
        {
        case UsdGeomXformOp::PrecisionDouble:
          {
            double original = 0;
            xformOp.Get(&original, _timeCode);
            original += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            float original = 0;
            xformOp.Get(&original, _timeCode);
            original += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            GfHalf original = 0;
            xformOp.Get(&original, _timeCode);
            original += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;
        }
        return true;
      }

    // directly modifying the x value only ever works when the rotation order begins with X
    case UsdGeomXformOp::TypeRotateXYZ:
    case UsdGeomXformOp::TypeRotateXZY:
      {
        switch(precision)
        {
        case UsdGeomXformOp::PrecisionDouble:
          {
            GfVec3d original(0);
            xformOp.Get(&original, _timeCode);
            original[0] += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            GfVec3f original(0);
            xformOp.Get(&original, _timeCode);
            original[0] += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            GfVec3h original(0);
            xformOp.Get(&original, _timeCode);
            original[0] += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;
        }
        return true;
      }

    case UsdGeomXformOp::TypeRotateYZX:
    case UsdGeomXformOp::TypeRotateZYX:
    case UsdGeomXformOp::TypeRotateYXZ:
    case UsdGeomXformOp::TypeRotateZXY:
    case UsdGeomXformOp::TypeTransform:
    case UsdGeomXformOp::TypeOrient:
      /* These have to be handled via quats sadly :( */
      break;

    // unsupported op type
    case UsdGeomXformOp::TypeRotateY:
    case UsdGeomXformOp::TypeRotateZ:
    default:
      return false;
    }
  }
  const double sr = std::sin(radianChange * 0.5f);
  const double cr = std::cos(radianChange * 0.5f);
  return Rotate(GfQuatd(cr, sr, 0, 0), space);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
MAYA_USD_UTILS_PUBLIC
bool TransformManipulator::RotateY(const double radianChange, Space space)
{
  if(space == kTransform)
  {
    auto xformOp = op();
    const auto precision = xformOp.GetPrecision();
    switch(xformOp.GetOpType())
    {
    case UsdGeomXformOp::TypeRotateY:
      {
        switch(precision)
        {
        case UsdGeomXformOp::PrecisionDouble:
          {
            double original = 0;
            xformOp.Get(&original, _timeCode);
            original += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            float original = 0;
            xformOp.Get(&original, _timeCode);
            original += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            GfHalf original = 0;
            xformOp.Get(&original, _timeCode);
            original += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;
        }
        return true;
      }

    // directly modifying the y value only ever works when the rotation order begins with Y
    case UsdGeomXformOp::TypeRotateYXZ:
    case UsdGeomXformOp::TypeRotateYZX:
      {
        switch(precision)
        {
        case UsdGeomXformOp::PrecisionDouble:
          {
            GfVec3d original(0);
            xformOp.Get(&original, _timeCode);
            original[1] += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            GfVec3f original(0);
            xformOp.Get(&original, _timeCode);
            original[1] += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            GfVec3h original(0);
            xformOp.Get(&original, _timeCode);
            original[1] += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;
        }
        return true;
      }

    case UsdGeomXformOp::TypeRotateXYZ:
    case UsdGeomXformOp::TypeRotateXZY:
    case UsdGeomXformOp::TypeRotateZXY:
    case UsdGeomXformOp::TypeRotateZYX:
    case UsdGeomXformOp::TypeTransform:
    case UsdGeomXformOp::TypeOrient:
      /* These have to be handled via quats sadly :( */
      break;
      
    // unsupported transform op type
    case UsdGeomXformOp::TypeRotateX:
    case UsdGeomXformOp::TypeRotateZ:
    default:
      return false;
    }
  }
  const double sr = std::sin(radianChange * 0.5f);
  const double cr = std::cos(radianChange * 0.5f);
  return Rotate(GfQuatd(cr, 0, sr, 0), space);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
MAYA_USD_UTILS_PUBLIC
bool TransformManipulator::RotateZ(const double radianChange, Space space)
{
  if(space == kTransform)
  {
    auto xformOp = op();
    const auto precision = xformOp.GetPrecision();
    switch(xformOp.GetOpType())
    {
    case UsdGeomXformOp::TypeRotateZ:
      {
        switch(precision)
        {
        case UsdGeomXformOp::PrecisionDouble:
          {
            double original = 0;
            xformOp.Get(&original, _timeCode);
            original += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            float original = 0;
            xformOp.Get(&original, _timeCode);
            original += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            GfHalf original = 0;
            xformOp.Get(&original, _timeCode);
            original += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;
        }
        return true;
      }

    // directly modifying the Z value only ever works when the rotation order begins with Z
    case UsdGeomXformOp::TypeRotateZXY:
    case UsdGeomXformOp::TypeRotateZYX:
      {
        switch(precision)
        {
        case UsdGeomXformOp::PrecisionDouble:
          {
            GfVec3d original(0);
            xformOp.Get(&original, _timeCode);
            original[2] += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            GfVec3f original(0);
            xformOp.Get(&original, _timeCode);
            original[2] += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            GfVec3h original(0);
            xformOp.Get(&original, _timeCode);
            original[2] += radiansToDegrees(radianChange);
            xformOp.Set(original, _timeCode);
          }
          break;
        }
        return true;
      }

    case UsdGeomXformOp::TypeRotateXYZ:
    case UsdGeomXformOp::TypeRotateXZY:
    case UsdGeomXformOp::TypeRotateYXZ:
    case UsdGeomXformOp::TypeRotateYZX:
    case UsdGeomXformOp::TypeTransform:
    case UsdGeomXformOp::TypeOrient:
      /* These have to be handled via quats sadly :( */
      break;
      
    // unsupported transform op type
    case UsdGeomXformOp::TypeRotateX:
    case UsdGeomXformOp::TypeRotateY:
    default:
      return false;
    }
  }
  const double sr = std::sin(radianChange * 0.5f);
  const double cr = std::cos(radianChange * 0.5f);
  return Rotate(GfQuatd(cr, 0, 0, sr), space);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
GfQuatd TransformManipulator::Rotation() const
{
  if(CanRotate())
  {
    GfQuatd rotate;
    storeu4d(&rotate, _Rotation(op(), _timeCode));
    return rotate;
  }
  return GfQuatd(1.0, 0.0, 0.0, 0.0);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
GfVec3d TransformManipulator::Translation() const
{
  if(CanTranslate())
  {
    const d256 translate = _Translation(op(), _timeCode);
    return GfVec3d(get<0>(translate), get<1>(translate), get<2>(translate));
  }
  return GfVec3d(0.0);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
GfVec3d TransformManipulator::Scale() const
{
  if(CanScale())
  {
    const d256 scale = _Scale(op(), _timeCode);
    return GfVec3d(get<0>(scale), get<1>(scale), get<2>(scale));
  }
  return GfVec3d(1.0);
}


} // MayaUsdUtils
