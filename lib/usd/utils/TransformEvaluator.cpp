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

#include "TransformEvaluator.h"
#include "TransformMaths.h"

#include <pxr/usd/usdGeom/xform.h>
#include <pxr/base/gf/rotation.h>
#include <iostream>

namespace MayaUsdUtils {

//----------------------------------------------------------------------------------------------------------------------
/// \brief  given an ordered set of XformOps, this method will evaluate the coordinate frame for a specifici xform op. 
///         By and large it will do this by concatonating operations of the same type, e.g. 
///          - successive translations will be grouped into 1 translation
///          - successive scales will be grouped into 1 scale
///          - successive rotations will be grouped into 1 quaternion
///         This will then be evaluated 
//----------------------------------------------------------------------------------------------------------------------
GfMatrix4d TransformEvaluator::EvaluateCoordinateFrameForRange(
  const std::vector<UsdGeomXformOp>& ops, 
  uint32_t start, 
  uint32_t end, 
  const UsdTimeCode& timeCode)
{
  auto iter = ops.begin() + start;
  auto last = ops.begin() + end;
  if(last > ops.end() || iter > ops.end()) 
  {
    throw;
  }

  // the computed coordinate frame - initially the identity
  d256 frame[4] = {
    set4d(1.0, 0.0, 0.0, 0.0),
    set4d(0.0, 1.0, 0.0, 0.0),
    set4d(0.0, 0.0, 1.0, 0.0),
    set4d(0.0, 0.0, 0.0, 1.0)
  };

  UsdGeomXformOp::Type lastType = UsdGeomXformOp::TypeInvalid;
  while(iter < last)
  {
    switch (iter->GetOpType())
    {
    //-------------------------------------------------------------------------------------
    // accumulate any translations into a single offset value, which we can apply to the 
    // matrix at the end. 
    // Note: During this accumulation of the offset, the 'w' value will be garbage. This is
    // removed prior to applying to the matrix. 
    //-------------------------------------------------------------------------------------
    case UsdGeomXformOp::TypeTranslate:
      {
        d256 offset = zero4d();
        do
        {
          offset = add4d(offset, _Translation(*iter, timeCode));
          ++iter;
        }
        while(iter != last && iter->GetOpType() == UsdGeomXformOp::TypeTranslate);

        // the 'w' value in offset is nonsense at this point. Replace it with 1.0. 
        offset = select4d<1, 1, 1, 0>(offset, splat4d(1.0));

        // if these translations are the very first in the stack, 
        // just assign the resulting translation directly, rather 
        // than trying to pointlessly rotate it (and accumulate error in the process).
        // typically Translate, RotatePivot, and RotatePivotTranslate could be 
        // evaluated in this way. 
        if(lastType == UsdGeomXformOp::TypeInvalid)
        {
          frame[3] = offset;
        }
        else
        {
          // If however this translation is in the middle of the transform stack, 
          // we need to re-orient the offset by the current coordinate frame before
          // applying the translation.
          frame[3] = add4d(frame[3], rotate(offset, frame));
        }
        lastType = UsdGeomXformOp::TypeTranslate;
      }
      break;

    //-------------------------------------------------------------------------------------
    // starting with a scaling of (1,1,1), successively multiply each scaling value we find
    // until we end up with a single scaling op to apply. It's unlikely there will be
    // multiple scales applied in succession, but might as well follow the same pattern.
    //-------------------------------------------------------------------------------------
    case UsdGeomXformOp::TypeScale:
      {
        d256 scaling = splat4d(1.0);
        do
        {
          scaling = mul4d(scaling, _Scale(*iter, timeCode));
          ++iter;
        }
        while(iter != last && iter->GetOpType() == UsdGeomXformOp::TypeScale);

        // apply scaling to each axis
        frame[0] = mul4d(permute4d<0, 0, 0, 0>(scaling), frame[0]);
        frame[1] = mul4d(permute4d<1, 1, 1, 1>(scaling), frame[1]);
        frame[2] = mul4d(permute4d<2, 2, 2, 2>(scaling), frame[2]);

        lastType = UsdGeomXformOp::TypeScale;
      }
      break;

    //-------------------------------------------------------------------------------------
    // for transforms, just multiply our frame with them and move on. There is no benefit
    // accumulating transforms in this case. 
    //-------------------------------------------------------------------------------------
    case UsdGeomXformOp::TypeTransform:
      {
        d256 dmatrix[4];
        switch(iter->GetPrecision())
        {
        case UsdGeomXformOp::PrecisionDouble:
          {
            // just grab matrix
            iter->Get((GfMatrix4d*)dmatrix, timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            // USD doesn't appear to support the GfMatrix4f type ?
            #if 0
            // grab as float
            f128 matrix[4];
            iter->Get((GfMatrix4f*)matrix, timeCode);

            // convert to double
            dmatrix[0] = cvt4f_to_4d(matrix[0]);
            dmatrix[1] = cvt4f_to_4d(matrix[1]);
            dmatrix[2] = cvt4f_to_4d(matrix[2]);
            dmatrix[3] = cvt4f_to_4d(matrix[3]);
            #endif
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            // USD doesn't appear to have a GfMatrix4h ?
            #if 0
            // grab as half
            i128 matrix[2];
            iter->Get((GfMatrix4h*)matrix, timeCode);

            // convert to float
            f256 fmatrix[2] = {
              cvtph8(matrix[0]),
              cvtph8(matrix[1])
            };

            // convert to double
            dmatrix[0] = cvt4f_to_4d(extract4f(fmatrix[0], 0));
            dmatrix[1] = cvt4f_to_4d(extract4f(fmatrix[0], 1));
            dmatrix[2] = cvt4f_to_4d(extract4f(fmatrix[1], 0));
            dmatrix[3] = cvt4f_to_4d(extract4f(fmatrix[1], 1));
            #endif
          }
          break;
        }

        multiply4x4(frame, dmatrix);
        lastType = UsdGeomXformOp::TypeTransform;
      }
      break;

    //-------------------------------------------------------------------------------------
    // No op. Only handling this case so I can lazily use the default case for rotation.
    //-------------------------------------------------------------------------------------
    case UsdGeomXformOp::TypeInvalid:
      break;

    //-------------------------------------------------------------------------------------
    // The default case is used to handle all rotation types. Successive rotations are 
    // accumulated as a quaternion, which is then converted to a matrix, before being 
    // multiplied with our frame.
    // Quats have a few advantages here, and possibly some disadvantages. 
    // 
    // Pros:
    // + Pretty trivial to generate a quat from an euler angle triplet directly. 
    // + Inverting a quat is extremely fast
    // Cons:
    // - People seem to be scared of them :)
    // - Quat mult can end up being slower than a 4x4 matrix mult 
    // - rotating a vector by a quat is more expensive that rotating by a matrix
    //
    // Looking through the various options here, accumulating eulers would be downright daft, 
    // accumulating as matrices seems very much like overkill, so I went down the quat route.
    //-------------------------------------------------------------------------------------
    default:
      {
        // util to identify whether an op is a rotation or not.
        auto isRotation = [](const UsdGeomXformOp& op) {
          switch(op.GetOpType())
          {
          case UsdGeomXformOp::TypeInvalid:
          case UsdGeomXformOp::TypeTranslate:
          case UsdGeomXformOp::TypeScale:
          case UsdGeomXformOp::TypeTransform:
            return false;
          default:
            break;
          }
          return true;
        };

        // grab first rotation as a quat
        d256 rotation = _Rotation(*iter, timeCode);
        ++iter;

        // accumulate any additional rotations
        while(iter != last && isRotation(*iter))
        {
          rotation = multiplyQuat(rotation, _Rotation(*iter, timeCode));
          ++iter;
        }

        // convert final quat to matrix
        d256 rotateMatrix[4];
        quatToMatrix(rotation, rotateMatrix);

        // and transform coordinate frame
        multiply(frame, rotateMatrix);

        // a bit naughty, but make sure we tag it as something a bit rotationy
        lastType = UsdGeomXformOp::TypeOrient;
      }
      break;
    }
  }
  GfMatrix4d r;
  storeu4d(r[0], frame[0]);
  storeu4d(r[1], frame[1]);
  storeu4d(r[2], frame[2]);
  storeu4d(r[3], frame[3]);
  return r;
}

//----------------------------------------------------------------------------------------------------------------------
d256 TransformEvaluator::_Rotation(const UsdGeomXformOp& op, const UsdTimeCode& timeCode)
{
  d256 result = set4d(0, 0, 0, 1.0);

  const bool isInverse = op.IsInverseOp();
  switch (op.GetOpType())
  {
  //-------------------------------------------------------------------------------------
  // error
  //-------------------------------------------------------------------------------------
  case UsdGeomXformOp::TypeTranslate:
    break;

  //-------------------------------------------------------------------------------------
  // error
  //-------------------------------------------------------------------------------------
  case UsdGeomXformOp::TypeScale:
    break;

  //-------------------------------------------------------------------------------------
  // for transforms, just multiply our frame with them and move on. There is no benefit
  // accumulating transforms in this case. 
  //-------------------------------------------------------------------------------------
  case UsdGeomXformOp::TypeTransform:
    {
      d256 dmatrix[4] = {
        set4d(1.0, 0.0, 0.0, 0.0),
        set4d(0.0, 1.0, 0.0, 0.0),
        set4d(0.0, 0.0, 1.0, 0.0),
        set4d(0.0, 0.0, 0.0, 1.0)
      };
      switch(op.GetPrecision())
      {
      case UsdGeomXformOp::PrecisionDouble:
        {
          // just grab matrix
          op.Get((GfMatrix4d*)dmatrix, timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          // USD doesn't appear to support the GfMatrix4f type ?
          #if 0
          // grab as float
          f128 matrix[4];
          op.Get((GfMatrix4f*)matrix, timeCode);

          // convert to double
          dmatrix[0] = cvt4f_to_4d(matrix[0]);
          dmatrix[1] = cvt4f_to_4d(matrix[1]);
          dmatrix[2] = cvt4f_to_4d(matrix[2]);
          dmatrix[3] = cvt4f_to_4d(matrix[3]);
          #endif
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          // USD doesn't appear to have a GfMatrix4h ?
          #if 0
          // grab as half
          i128 matrix[2];
          op.Get((GfMatrix4h*)matrix, timeCode);

          // convert to float
          f256 fmatrix[2] = {
            cvtph8(matrix[0]),
            cvtph8(matrix[1])
          };

          // convert to double
          dmatrix[0] = cvt4f_to_4d(extract4f(fmatrix[0], 0));
          dmatrix[1] = cvt4f_to_4d(extract4f(fmatrix[0], 1));
          dmatrix[2] = cvt4f_to_4d(extract4f(fmatrix[1], 0));
          dmatrix[3] = cvt4f_to_4d(extract4f(fmatrix[1], 1));
          #endif
        }
        break;
      }
      result = matrixToQuat(dmatrix);
    }
    break;

  //-------------------------------------------------------------------------------------
  // No op. Only handling this case so I can lazily use the default case for rotation.
  //-------------------------------------------------------------------------------------
  case UsdGeomXformOp::TypeInvalid:
    break;

  //-------------------------------------------------------------------------------------
  // The default case is used to handle all rotation types. Successive rotations are 
  // accumulated as a quaternion, which is then converted to a matrix, before being 
  // multiplied with our frame.
  // Quats have a few advantages here, and possibly some disadvantages. 
  // 
  // Pros:
  // + Pretty trivial to generate a quat from an euler angle triplet directly. 
  // + Inverting a quat is extremely fast
  // Cons:
  // - People seem to be scared of them :)
  // - Quat mult can end up being slower than a 4x4 matrix mult 
  // - rotating a vector by a quat is more expensive that rotating by a matrix
  //
  // Looking through the various options here, accumulating eulers would be downright daft, 
  // accumulating as matrices seems very much like overkill, so I went down the quat route.
  //-------------------------------------------------------------------------------------
  default:
    {
      // extract an angle value in degrees, and convert it into (radians / 2)
      auto getDouble = [timeCode, op]()
      {
        switch(op.GetPrecision())
        {
        case UsdGeomXformOp::PrecisionDouble:
          {
            double v;
            op.Get(&v, timeCode);
            return degreesToHalfAngle(v);
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            float v;
            op.Get(&v, timeCode);
            return degreesToHalfAngle(double(v));
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            GfHalf v;
            op.Get(&v, timeCode);
            return degreesToHalfAngle(double(cvtph1(v.bits())));
          }
          break;
        }
        return 0.0;
      };

      // extract an trio of angle values in degrees, and convert to: (radians / 2)
      auto getXYZ = [timeCode, op]()
      {
        switch(op.GetPrecision())
        {
        case UsdGeomXformOp::PrecisionDouble:
          {
            d256 v;
            op.Get((GfVec3d*)&v, timeCode);
            return degreesToHalfAngle(v);
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            f128 v;
            op.Get((GfVec3f*)&v, timeCode);
            return degreesToHalfAngle(cvt4f_to_4d(v));
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            i128 v;
            op.Get((GfVec3h*)&v, timeCode);
            return degreesToHalfAngle(cvt4f_to_4d(cvtph4(v)));
          }
          break;
        }
        return zero4d();
      };

      // given the various 
      auto getQuat = [timeCode, getXYZ, getDouble, op]()
      {
        auto opType = op.GetOpType();
        switch(opType)
        {
        // splat straight into quat form
        case UsdGeomXformOp::TypeRotateX: 
          {
            const double half_angle = getDouble();
            const double sa = std::sin(half_angle);
            const double ca = std::cos(half_angle);
            return set4d(sa, 0, 0, ca);
          }
          break;

        case UsdGeomXformOp::TypeRotateY:
          {
            const double half_angle = getDouble();
            const double sa = std::sin(half_angle);
            const double ca = std::cos(half_angle);
            return set4d(0, sa, 0, ca);
          }
          break;

        case UsdGeomXformOp::TypeRotateZ:
          {
            const double half_angle = getDouble();
            const double sa = std::sin(half_angle);
            const double ca = std::cos(half_angle);
            return set4d(0, 0, sa, ca);
          }
          break;

        case UsdGeomXformOp::TypeRotateXYZ: 
          {
            const d256 half_angle = getXYZ();
            return Quat_from_EulerXYZ(half_angle);
          }
          break;

        case UsdGeomXformOp::TypeRotateXZY: 
          {
            const d256 half_angle = getXYZ();
            return Quat_from_EulerXZY(half_angle);
          }
          break;

        case UsdGeomXformOp::TypeRotateYXZ: 
          {
            const d256 half_angle = getXYZ();
            return Quat_from_EulerYXZ(half_angle);
          }
          break;

        case UsdGeomXformOp::TypeRotateYZX: 
          {
            const d256 half_angle = getXYZ();
            return Quat_from_EulerYZX(half_angle);
          }
          break;

        case UsdGeomXformOp::TypeRotateZYX: 
          {
            const d256 half_angle = getXYZ();
            return Quat_from_EulerZYX(half_angle);
          }
          break;

        case UsdGeomXformOp::TypeRotateZXY: 
          {
            const d256 half_angle = getXYZ();
            return Quat_from_EulerZXY(half_angle);
          }
          break;

        // splat straight into quat 
        case UsdGeomXformOp::TypeOrient: 
          {
            switch(op.GetPrecision())
            {
            case UsdGeomXformOp::PrecisionDouble:
              {
                d256 v;
                op.Get((GfQuatd*)&v, timeCode);
                return v;
              }
              break;

            case UsdGeomXformOp::PrecisionFloat:
              {
                f128 v;
                op.Get((GfQuatf*)&v, timeCode);
                return cvt4f_to_4d(v);
              }
              break;

            case UsdGeomXformOp::PrecisionHalf:
              {
                i128 v;
                op.Get((GfQuath*)&v, timeCode);
                auto temp = cvtph4(v);
                return cvt4f_to_4d(temp);
              }
              break;
            }
          }
          break;

        default:
          break; 
        }
        return set4d(0.0, 0.0, 0.0, 1.0);
      };

      // grab first rotation as a quat
      result = getQuat();
    }
    break;
  }
  if(isInverse)
    result = xor4d(result, set4d(-0.0, -0.0, -0.0, 0));
  return result;
}

//----------------------------------------------------------------------------------------------------------------------
d256 TransformEvaluator::_Translation(const UsdGeomXformOp& op, const UsdTimeCode& timeCode)
{
  d256 result = zero4d();
  const bool isInverse = op.IsInverseOp();
  switch (op.GetOpType())
  {
  //-------------------------------------------------------------------------------------
  // error
  //-------------------------------------------------------------------------------------
  case UsdGeomXformOp::TypeTranslate:
    {
      switch(op.GetPrecision())
      {
      case UsdGeomXformOp::PrecisionDouble:
        {
          op.Get((GfVec3d*)&result, timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          f128 v;
          op.Get((GfVec3f*)&v, timeCode);
          result = cvt4f_to_4d(v);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          i128 v;
          op.Get((GfVec3h*)&v, timeCode);
          auto temp = cvtph4(v);
          result = cvt4f_to_4d(temp);
        }
        break;
      }
    }
    break;

  //-------------------------------------------------------------------------------------
  // for transforms, just multiply our frame with them and move on. There is no benefit
  // accumulating transforms in this case. 
  //-------------------------------------------------------------------------------------
  case UsdGeomXformOp::TypeTransform:
    {
      d256 dmatrix[4] = {
        set4d(1.0, 0.0, 0.0, 0.0),
        set4d(0.0, 1.0, 0.0, 0.0),
        set4d(0.0, 0.0, 1.0, 0.0),
        set4d(0.0, 0.0, 0.0, 1.0)
      };
      switch(op.GetPrecision())
      {
      case UsdGeomXformOp::PrecisionDouble:
        {
          // just grab matrix
          op.Get((GfMatrix4d*)dmatrix, timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          // USD doesn't appear to support the GfMatrix4f type ?
          #if 0
          // grab as float
          f128 matrix[4];
          op.Get((GfMatrix4f*)matrix, timeCode);

          // convert to double
          dmatrix[0] = cvt4f_to_4d(matrix[0]);
          dmatrix[1] = cvt4f_to_4d(matrix[1]);
          dmatrix[2] = cvt4f_to_4d(matrix[2]);
          dmatrix[3] = cvt4f_to_4d(matrix[3]);
          #endif
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          // USD doesn't appear to have a GfMatrix4h ?
          #if 0
          // grab as half
          i128 matrix[2];
          op.Get((GfMatrix4h*)matrix, timeCode);

          // convert to float
          f256 fmatrix[2] = {
            cvtph8(matrix[0]),
            cvtph8(matrix[1])
          };

          // convert to double
          dmatrix[0] = cvt4f_to_4d(extract4f(fmatrix[0], 0));
          dmatrix[1] = cvt4f_to_4d(extract4f(fmatrix[0], 1));
          dmatrix[2] = cvt4f_to_4d(extract4f(fmatrix[1], 0));
          dmatrix[3] = cvt4f_to_4d(extract4f(fmatrix[1], 1));
          #endif
        }
        break;
      }
      // extract translation from matrix
      result = select4d<1, 1, 1, 0>(dmatrix[3], result);
    }
    break;

  //-------------------------------------------------------------------------------------
  // error
  //-------------------------------------------------------------------------------------
  default:
    break;
  }

  // negate if inverse op
  if(isInverse)
  {
    result = xor4d(result, set4d(-0.0, -0.0, -0.0, 0.0));
  }
  return result;
}

//----------------------------------------------------------------------------------------------------------------------
d256 TransformEvaluator::_Scale(const UsdGeomXformOp& op, const UsdTimeCode& timeCode)
{
  d256 result = set4d(1.0, 1.0, 1.0, 0.0);
  const bool isInverse = op.IsInverseOp();
  switch (op.GetOpType())
  {
  //-------------------------------------------------------------------------------------
  // error
  //-------------------------------------------------------------------------------------
  case UsdGeomXformOp::TypeScale:
    {
      switch(op.GetPrecision())
      {
      case UsdGeomXformOp::PrecisionDouble:
        {
          op.Get((GfVec3d*)&result, timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          f128 v;
          op.Get((GfVec3f*)&v, timeCode);
          result = cvt4f_to_4d(v);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          i128 v;
          op.Get((GfVec3h*)&v, timeCode);
          auto temp = cvtph4(v);
          result = cvt4f_to_4d(temp);
        }
        break;
      }
    }
    break;

  //-------------------------------------------------------------------------------------
  // for transforms, just multiply our frame with them and move on. There is no benefit
  // accumulating transforms in this case. 
  //-------------------------------------------------------------------------------------
  case UsdGeomXformOp::TypeTransform:
    {
      d256 dmatrix[4] = {
        set4d(1.0, 0.0, 0.0, 0.0),
        set4d(0.0, 1.0, 0.0, 0.0),
        set4d(0.0, 0.0, 1.0, 0.0),
        set4d(0.0, 0.0, 0.0, 1.0)
      };
      switch(op.GetPrecision())
      {
      case UsdGeomXformOp::PrecisionDouble:
        {
          // just grab matrix
          op.Get((GfMatrix4d*)dmatrix, timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          // USD doesn't appear to support the GfMatrix4f type ?
          #if 0
          // grab as float
          f128 matrix[4];
          op.Get((GfMatrix4f*)matrix, timeCode);

          // convert to double
          dmatrix[0] = cvt4f_to_4d(matrix[0]);
          dmatrix[1] = cvt4f_to_4d(matrix[1]);
          dmatrix[2] = cvt4f_to_4d(matrix[2]);
          dmatrix[3] = cvt4f_to_4d(matrix[3]);
          #endif
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          // USD doesn't appear to have a GfMatrix4h ?
          #if 0
          // grab as half
          i128 matrix[2];
          op.Get((GfMatrix4h*)matrix, timeCode);

          // convert to float
          f256 fmatrix[2] = {
            cvtph8(matrix[0]),
            cvtph8(matrix[1])
          };

          // convert to double
          dmatrix[0] = cvt4f_to_4d(extract4f(fmatrix[0], 0));
          dmatrix[1] = cvt4f_to_4d(extract4f(fmatrix[0], 1));
          dmatrix[2] = cvt4f_to_4d(extract4f(fmatrix[1], 0));
          dmatrix[3] = cvt4f_to_4d(extract4f(fmatrix[1], 1));
          #endif
        }
        break;
      }

      d256 testz = cross(dmatrix[0], dmatrix[1]);
      double sx = dot3(dmatrix[0], dmatrix[0]);
      double sy = dot3(dmatrix[1], dmatrix[1]);
      double sz = dot3(dmatrix[2], dmatrix[2]);
      if(dot3(testz, dmatrix[2]) < 0)
        sz = -sz;
      result = sqrt4d(set4d(sx, sy, sz, 0.0));
    }
    break;

  //-------------------------------------------------------------------------------------
  // error
  //-------------------------------------------------------------------------------------
  default:
    break;
  }

  // negate if inverse op
  if(isInverse)
  {
    result = div4d(splat4d(1.0), result);
  }
  return result;
}

} // MayaUsdUtils
