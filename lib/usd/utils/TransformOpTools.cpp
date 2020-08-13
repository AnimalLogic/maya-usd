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

#include "TransformOpTools.h"
#include "SIMD.h"

#include <pxr/usd/usdGeom/xform.h>
#include <pxr/base/gf/rotation.h>
#include <iostream>
#include <execinfo.h>

namespace MayaUsdUtils {

namespace 
{void stacktrace_to_cout()
{
  void** ptrs = (void**)alloca(sizeof(void*) * 16);
  const std::size_t count = backtrace(ptrs, 16);
  if(count)
  {
    backtrace_symbols_fd(ptrs, count, STDOUT_FILENO);
  }
}

//---------------------------------------------------------------------------------------------
// Note: If needed, it's possible to sneak a bit more performance from this code. I stopped 
// short of adding a vectorised sincos() implementation, but that would be possible....
//---------------------------------------------------------------------------------------------
inline d256 Quat_from_EulerXYZ(const d256 half_angles)
{
  alignas(32) double h[4];
  store4d(h, half_angles);
  const double sx = std::sin(h[0]);
  const double cx = std::cos(h[0]);
  const double sy = std::sin(h[1]);
  const double cy = std::cos(h[1]);
  const double sz = std::sin(h[2]);
  const double cz = std::cos(h[2]);
  const double czcy = cz * cy;
  const double czsy = cz * sy;
  const double szsy = sz * sy;
  const double szcy = sz * cy;
  return set4d(
     (czcy * sx) - (szsy * cx),
     (czsy * cx) + (szcy * sx),
    -(czsy * sx) + (szcy * cx),
     (czcy * cx) + (szsy * sx)
  );
}

inline d256 Quat_from_EulerXZY(const d256 half_angles)
{
  alignas(32) double h[4];
  store4d(h, half_angles);
  const double sx = std::sin(h[0]);
  const double cx = std::cos(h[0]);
  const double sy = std::sin(h[1]);
  const double cy = std::cos(h[1]);
  const double sz = std::sin(h[2]);
  const double cz = std::cos(h[2]);
  const double cycz = cy * cz;
  const double cysz = cy * sz;
  const double sycz = sy * cz;
  const double sysz = sy * sz;
  return set4d(
    (cycz * sx) + (sysz * cx),
    (cysz * sx) + (sycz * cx),
    (cysz * cx) - (sycz * sx),
    (cycz * cx) - (sysz * sx)
  );
}

inline d256 Quat_from_EulerYXZ(const d256 half_angles)
{
  alignas(32) double h[4];
  store4d(h, half_angles);
  const double sx = std::sin(h[0]);
  const double cx = std::cos(h[0]);
  const double sy = std::sin(h[1]);
  const double cy = std::cos(h[1]);
  const double sz = std::sin(h[2]);
  const double cz = std::cos(h[2]);
  const double czsx = cz * sx;
  const double czcx = cz * cx;
  const double szsx = sz * sx;
  const double szcx = sz * cx;
  return set4d(
    (czsx * cy) - (szcx * sy),
    (czcx * sy) + (szsx * cy),
    (czsx * sy) + (szcx * cy),
    (czcx * cy) - (szsx * sy)
  );
}

inline d256 Quat_from_EulerYZX(const d256 half_angles)
{
  alignas(32) double h[4];
  store4d(h, half_angles);
  const double sx = std::sin(h[0]);
  const double cx = std::cos(h[0]);
  const double sy = std::sin(h[1]);
  const double cy = std::cos(h[1]);
  const double sz = std::sin(h[2]);
  const double cz = std::cos(h[2]);
  const double cxsz = cx * sz;
  const double cxcz = cx * cz;
  const double sxsz = sx * sz;
  const double sxcz = sx * cz;
  return set4d(
    -(cxsz * sy) + (sxcz * cy),
     (cxcz * sy) - (sxsz * cy),
     (cxsz * cy) + (sxcz * sy),
     (cxcz * cy) + (sxsz * sy)
  );
}

inline d256 Quat_from_EulerZYX(const d256 half_angles)
{
  alignas(32) double h[4];
  store4d(h, half_angles);
  const double sx = std::sin(h[0]);
  const double cx = std::cos(h[0]);
  const double sy = std::sin(h[1]);
  const double cy = std::cos(h[1]);
  const double sz = std::sin(h[2]);
  const double cz = std::cos(h[2]); 
  const double cxsy = cx * sy;
  const double cxcy = cx * cy;
  const double sxsy = sx * sy;
  const double sxcy = sx * cy;
  return set4d(
    (cxsy * sz) + (sxcy * cz),
    (cxsy * cz) - (sxcy * sz),
    (cxcy * sz) + (sxsy * cz),
    (cxcy * cz) - (sxsy * sz)
  );
}

inline d256 Quat_from_EulerZXY(const d256 half_angles)
{
  alignas(32) double h[4];
  store4d(h, half_angles);
  const double sx = std::sin(h[0]);
  const double cx = std::cos(h[0]);
  const double sy = std::sin(h[1]);
  const double cy = std::cos(h[1]);
  const double sz = std::sin(h[2]);
  const double cz = std::cos(h[2]); 
  const double cysx = cy * sx;
  const double cycx = cy * cx;
  const double sysx = sy * sx;
  const double sycx = sy * cx;
  return set4d(
     (cysx * cz) + (sycx * sz),
    -(cysx * sz) + (sycx * cz),
     (cycx * sz) - (sysx * cz),
     (cycx * cz) + (sysx * sz)
  );
}


//-------------------------------------------------------------------------------------------
// /Autogenerated code.
//-------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
enum class RotationOrder
{
  kXYZ, 
  kYZX, 
  kZXY, 
  kXZY, 
  kYXZ, 
  kZYX
};

inline d256 multiplyQuat(const d256 parent, const d256 child)
{
  const d256 negw = set4d(0, 0, 0, -0.0);
  const d256 pWWWW = permute4d<3, 3, 3, 3>(parent);
  const d256 pXYZX = xor4d(permute4d<0, 1, 2, 0>(parent), negw);
  const d256 pZXYY = permute4d<2, 0, 1, 1>(parent);
  const d256 pYZXZ = xor4d(permute4d<1, 2, 0, 2>(parent), negw);
  const d256 cXYZW = permute4d<0, 1, 2, 3>(child);
  const d256 cWWWX = permute4d<3, 3, 3, 0>(child);
  const d256 cYZXY = permute4d<1, 2, 0, 1>(child);
  const d256 cZXYZ = permute4d<2, 0, 1, 2>(child);
  d256 rr = fmadd4d(pXYZX, cWWWX, mul4d(pWWWW, cXYZW));
  rr = fnmadd4d(pZXYY, cYZXY, rr);
  return fmadd4d(pYZXZ, cZXYZ, rr);
}

inline d256 cross(d256 b, d256 c)
{
  const d256 B1 = permute4d<1, 2, 0, 3>(b);
  const d256 C1 = permute4d<2, 0, 1, 3>(c);
  const d256 B2 = permute4d<2, 0, 1, 3>(b);
  const d256 C2 = permute4d<1, 2, 0, 3>(c);

	// (a)[0] = (b)[1] * (c)[2] - (c)[1] * (b)[2];
	// (a)[1] = (b)[2] * (c)[0] - (c)[2] * (b)[0];
	// (a)[2] = (b)[0] * (c)[1] - (c)[0] * (b)[1];

  return fmadd4d(B1, C1, mul4d(B2, C2));
}

inline void extractEuler(const d256 mat[4], RotationOrder rotOrder, GfVec3f& rot)
{
  const int mod3[6] = {0, 1, 2, 0, 1, 2};
  const int k1 = int(rotOrder) > 2 ? 2 : 1;
  const int k2 = 3 - k1;
  const int row = mod3[int(rotOrder)];
  const int col = mod3[k2 + row];
  const int colCos = mod3[col + k1];
  const int colSin = mod3[col + k2];
  const int rowSin = mod3[row + k1];
  const int rowCos = mod3[row + k2];
  const double s  = int(rotOrder) < 3 ? -1.0 : 1.0;

  const double epsilon = std::numeric_limits<double>::epsilon();
  if (std::fabs(mat[row][col] - 1) < epsilon)
  {
    rot[row] = std::atan2(s*mat[rowSin][colCos], mat[rowSin][colSin]);
    rot[rowSin] = s*M_PI/2;
    rot[rowCos] = 0;
  }
  else
  if (std::fabs(mat[row][col] + 1) < epsilon)
  {    
    rot[row] = std::atan2(-s*mat[rowSin][colCos], mat[rowSin][colSin]);
    rot[rowSin] = -s*M_PI/2;
    rot[rowCos] = 0;
  }
  else
  {
    rot[row] = std::atan2(-s*mat[rowSin][col], mat[rowCos][col] );
    rot[rowSin] = std::asin ( s*mat[row][col] );
    rot[rowCos] = std::atan2(-s*mat[row][colSin], mat[row][colCos] );
  }
}

inline d256 quatInvert(d256 quat)
{
  return xor4d(quat, set4d(-0.0, -0.0, -0.0, 0.0));
}

void quatToMatrix(d256 quat, d256 matrix[4])
{
  // the standard Quatd to matrix follows this approach:
  // 
  //   1 - 2yy - 2zz,      2xy + 2wz,      2xz - 2wy
  //       2xy - 2wz,  1 - 2xx - 2zz,      2yz + 2wx
  //       2xz + 2wy,      2yz - 2wx,  1 - 2xx - 2yy
  // 
  // Pretty much every term needs to be multiplied by 2, so let's cheat instead! Define the following:
  // 
  //   X = x * sqrt(2);
  //   Y = y * sqrt(2);
  //   Z = z * sqrt(2);
  //   W = w * sqrt(2);
  //
  // and now we can simplify the equation to:
  // 
  //   1 - YY - ZZ,      XY + ZW,      XZ - YW
  //       XY - ZW,  1 - XX - ZZ,      YZ + XW
  //       XZ + YW,      YZ - XW,  1 - XX - YY
  //
  // Whilst there are lots of common terms here, I'm only going to pre-calculate XY, XZ, and YZ. 
  // All of the other terms I will calculate using FMA instructions. This does mean I'll be 
  // duplicating some computations, however fmadd has a higher throughput that add or sub, so it's 
  // actually just quicker to duplicate those computations! (somewhat counter intuitively!)
  // 
  const d256 root2 = splat4d(1.4142135623730950488016887242097);
  //const d256 one = splat4d(1.0);
  const d256 XYZW = mul4d(quat, root2);

  d256 X;
  d256 Y;
  d256 Z;
  {
    auto YXX = permute4d<1, 0, 0, 3>(XYZW);
    auto YYZ = permute4d<1, 1, 2, 3>(XYZW);
    auto ZZY = permute4d<2, 2, 1, 3>(XYZW);
    auto ZWW = permute4d<2, 3, 3, 3>(XYZW);
    X = mul4d(YXX, YYZ);
    ZZY = xor4d(ZZY, set4d(0, 0, -0.0, 0));
    X = fmadd4d(ZZY, ZWW, X);
    X = xor4d(X, set4d(-0.0, 0, 0, 0));
    X = add4d(X, set4d(1.0, 0, 0, 0));
  }
  {
    auto XXY = permute4d<0, 0, 1, 3>(XYZW);
    auto YXZ = permute4d<1, 0, 2, 3>(XYZW);
    auto ZZX = permute4d<2, 2, 0, 3>(XYZW);
    auto WZW = permute4d<3, 2, 3, 3>(XYZW);
    Y = mul4d(XXY, YXZ);
    ZZX = xor4d(ZZX, set4d(-0.0, 0, 0, 0));
    Y = fmadd4d(ZZX, WZW, Y);
    Y = xor4d(Y, set4d(0, -0.0, 0, 0));
    Y = add4d(Y, set4d(0, 1.0, 0, 0));
  }
  {
    auto XYX = permute4d<0, 1, 0, 3>(XYZW);
    auto ZZX = permute4d<2, 2, 0, 3>(XYZW);
    auto YXY = permute4d<1, 0, 1, 3>(XYZW);
    auto WWY = permute4d<3, 3, 1, 3>(XYZW);
    Z = mul4d(XYX, ZZX);
    YXY = xor4d(YXY, set4d(0, -0.0, 0, 0));
    Z = fmadd4d(YXY, WWY, Z);
    Z = xor4d(Z, set4d(0, 0, -0.0, 0));
    Z = add4d(Z, set4d(0, 0, 1.0, 0));
  }
  matrix[0] = select4d<1, 1, 1, 0>(X, zero4d());
  matrix[1] = select4d<1, 1, 1, 0>(Y, zero4d());
  matrix[2] = select4d<1, 1, 1, 0>(Z, zero4d());
  matrix[3] = set4d(0, 0, 0, 1.0);
}

/// there is room for improvement here!
void extractEuler(const d256 q, RotationOrder rotOrder, GfVec3f& rot)
{
  d256 matrix[4];
  quatToMatrix(q, matrix);
  extractEuler(matrix, rotOrder, rot);
}

// rotate an offset vector by the coordinate frame
inline d256 rotate(const d256 offset, const d256 frame[4])
{
  const d256 xxx = permute4d<0, 0, 0, 0>(offset);
  const d256 yyy = permute4d<1, 1, 1, 1>(offset);
  const d256 zzz = permute4d<2, 2, 2, 2>(offset);
  return fmadd4d(zzz, frame[2], fmadd4d(yyy, frame[1], mul4d(xxx, frame[0])));
}

inline double dot3(d256 a, d256 b)
{
  d256 ab = mul4d(a, b);
  return ab[0] + ab[1] + ab[2];
}

// rotate an offset vector by the coordinate frame
inline d256 fastInverseRotate(const d256 offset, const d256 frame[4])
{
  return set4d(dot3(offset, frame[0]), dot3(offset, frame[1]), dot3(offset, frame[2]), 0);
}

// rotate an offset vector by the coordinate frame
inline d256 inverseRotate(const d256 offset, const d256 frame[4])
{
  d256 len2 = set4d(dot3(frame[0], frame[0]), dot3(frame[1], frame[1]), dot3(frame[2], frame[2]), 0.0);
  return div4d(set4d(dot3(offset, frame[0]), dot3(offset, frame[1]), dot3(offset, frame[2]), 0), len2);
}

// transform a point by the coordinate frame
inline d256 transform(const d256 offset, const d256 frame[4])
{
  return add4d(frame[3], rotate(offset, frame));
}

// transform a point by the inverse coordinate frame
inline d256 inverseTransform(d256 offset, const d256 frame[4])
{
  offset = sub4d(offset, frame[3]);
  auto r = inverseRotate(offset, frame);
  return r;
}


// frame *= childTransform
inline void multiply(d256 frame[4], const d256 childTransform[4])
{
  const d256 mx = rotate(childTransform[0], frame);
  const d256 my = rotate(childTransform[1], frame);
  const d256 mz = rotate(childTransform[2], frame);
  frame[3] = transform(childTransform[3], frame);
  frame[0] = mx;
  frame[1] = my;
  frame[2] = mz;
}

// rotate an offset vector by the coordinate frame
inline d256 transform4d(const d256 offset, const d256 frame[4])
{
  const d256 xxx = permute4d<0, 0, 0, 0>(offset);
  const d256 yyy = permute4d<1, 1, 1, 1>(offset);
  const d256 zzz = permute4d<2, 2, 2, 2>(offset);
  const d256 www = permute4d<3, 3, 3, 3>(offset);
  return fmadd4d(www, frame[3], fmadd4d(zzz, frame[2], fmadd4d(yyy, frame[1], mul4d(xxx, frame[0]))));
}

// frame *= childTransform
inline void multiply4x4(d256 frame[4], const d256 childTransform[4])
{
  const d256 mx = transform4d(childTransform[0], frame);
  const d256 my = transform4d(childTransform[1], frame);
  const d256 mz = transform4d(childTransform[2], frame);
  frame[3] = transform4d(childTransform[3], frame);
  frame[0] = mx;
  frame[1] = my;
  frame[2] = mz;
}
// frame *= childTransform
inline void multiply4x4(d256 output[4], const d256 childTransform[4], const d256 parentTransform[4])
{
  const d256 mx = transform4d(childTransform[0], parentTransform);
  const d256 my = transform4d(childTransform[1], parentTransform);
  const d256 mz = transform4d(childTransform[2], parentTransform);
  output[3] = transform4d(childTransform[3], parentTransform);
  output[0] = mx;
  output[1] = my;
  output[2] = mz;
}
// frame *= childTransform
inline void multiply(d256 output[4], const d256 childTransform[4], const d256 parentTransform[4])
{
  const d256 mx = rotate(childTransform[0], parentTransform);
  const d256 my = rotate(childTransform[1], parentTransform);
  const d256 mz = rotate(childTransform[2], parentTransform);
  output[3] = transform(childTransform[3], parentTransform);
  output[0] = mx;
  output[1] = my;
  output[2] = mz;
}

inline d256 matrixToQuat(const d256 iframe[4])
{
  d256 frame[3];

  // orthogonalise matrix
  double lx = std::sqrt(dot3(iframe[0], iframe[0]));
  double lz = std::sqrt(dot3(iframe[2], iframe[2]));
  frame[0] = div4d(iframe[0], splat4d(lx));
  frame[2] = div4d(iframe[2], splat4d(lz));
  frame[1] = cross(iframe[2], iframe[0]);
  frame[2] = cross(iframe[0], iframe[1]);

  double W = 1.0 + frame[0][0] + frame[1][1] + frame[2][2];
  W = std::sqrt(W);
  const double qx = (frame[1][2] - frame[2][1]);
  const double qy = (frame[2][0] - frame[0][2]);
  const double s = (0.5 / W);
  const double qz = (frame[0][1] - frame[1][0]);
  return set4d(qx * s, qy * s, qz * s, W * 0.5);
}

} // end anon


/// there is room for improvement here!
MAYA_USD_UTILS_PUBLIC
GfVec3d QuatToEulerXYZ(const GfQuatd q)
{
  GfVec3f rotate;
  extractEuler(loadu4d(&q), RotationOrder::kXYZ, rotate);
  return GfVec3d(rotate[0], rotate[1], rotate[2]) * (180.0 / M_PI);
}

MAYA_USD_UTILS_PUBLIC
GfQuatd QuatFromEulerXYZ(const GfVec3d& degrees)
{
  GfVec3d h = degrees * (M_PI / 360.0);
  auto ha = set4d(h[0], h[1], h[2], 0.0);
  auto q = Quat_from_EulerXYZ(ha);
  GfQuatd Q;
  storeu4d(&Q, q);
  return Q;
}

TfToken TransformOpProcessor::primaryRotateSuffix;
TfToken TransformOpProcessor::primaryScaleSuffix;
TfToken TransformOpProcessor::primaryTranslateSuffix;

//----------------------------------------------------------------------------------------------------------------------------------------------------------
MAYA_USD_UTILS_PUBLIC
TransformOpProcessor::TransformOpProcessor(const UsdPrim prim, const TfToken opName, const TransformOpProcessor::ManipulatorMode mode, const UsdTimeCode& tc)
  : _prim(prim)
{
  _manipMode = mode;
  _ops = UsdGeomXformable(prim).GetOrderedXformOps(&_resetsXformStack);
  const size_t opCount = _ops.size();

  const TfToken empty(""); 
  if(opName == empty)
  {
    switch(mode)
    {
    case TransformOpProcessor::kRotate:
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

        // if no special case found
        if(_opIndex == opCount)
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

    case TransformOpProcessor::kScale:
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
        // if no special case found
        if(_opIndex == opCount)
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
    case TransformOpProcessor::kTranslate:
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
        // if no special case found
        if(_opIndex == opCount)
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

    case TransformOpProcessor::kGuess:
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
  UpdateToTime(tc, mode);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
MAYA_USD_UTILS_PUBLIC
TransformOpProcessor::TransformOpProcessor(const UsdPrim prim, const uint32_t opIndex, const TransformOpProcessor::ManipulatorMode mode, const UsdTimeCode& tc)
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
TransformOpProcessor::ManipulatorMode TransformOpProcessor::ManipMode() const
{
  return _manipMode; 
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
MAYA_USD_UTILS_PUBLIC
bool TransformOpProcessor::CanRotate() const
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
bool TransformOpProcessor::CanTranslate() const
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
bool TransformOpProcessor::CanScale() const
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
void TransformOpProcessor::UpdateToTime(const UsdTimeCode& tc, UsdGeomXformCache& cache, const TransformOpProcessor::ManipulatorMode mode)
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
bool TransformOpProcessor::Translate(const GfVec3d& translateChange, const Space space)
{
  stacktrace_to_cout();
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
  std::cout << "translateChange " << temp[0] << ' ' << temp[1] << ' ' << temp[2] << '\n';
  switch(space)
  {
  case kTransform: break;
  case kWorld: temp = transform4d(temp, (const d256*)&_invWorldFrame); break;
  case kPreTransform: temp = transform4d(temp, (const d256*)&_invCoordFrame); break;
  case kPostTransform: temp = transform4d(temp, (const d256*)&_invPostFrame); break;
  }

  std::cout << "temp " << temp[0] << ' ' << temp[1] << ' ' << temp[2] << '\n';

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
  std::cout << "original " << original[0] << ' ' << original[1] << ' ' << original[2] << '\n';
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
        std::cout << "after " << temp[0] << ' ' << temp[1] << ' ' << temp[2] << '\n';
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
    xformOp.Get((GfMatrix4d*)matrix, _timeCode);
    matrix[3] = add4d(temp, matrix[3]);
    xformOp.Set(*(GfMatrix4d*)matrix, _timeCode);
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
bool TransformOpProcessor::Scale(const GfVec3d& scaleChange, const Space space)
{
  // when in strange coordinate frames, scaling can only be safely applied if the XYZ values are uniform
  switch(space)
  {
  case kTransform: break;
  case kWorld:
  case kPreTransform:
  case kPostTransform:
    {
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
        xformOp.Set(*(GfVec3d*)&temp, _timeCode);
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
    xformOp.Get((GfMatrix4d*)matrix, _timeCode);
    matrix[0] = mul4d(permute4d<0,0,0,0>(temp), matrix[0]);
    matrix[1] = mul4d(permute4d<1,1,1,1>(temp), matrix[1]);
    matrix[2] = mul4d(permute4d<2,2,2,2>(temp), matrix[2]);
    xformOp.Set(*(GfMatrix4d*)matrix, _timeCode);
  }
  else
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
MAYA_USD_UTILS_PUBLIC
bool TransformOpProcessor::Rotate(const GfQuatd& quatChange, Space space)
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
        op().Get((GfMatrix4d*)matrix, _timeCode);
        d256 rmatrix[4];
        quatToMatrix(offset, rmatrix);
        matrix[0] = rotate(matrix[0], rmatrix);
        matrix[1] = rotate(matrix[1], rmatrix);
        matrix[2] = rotate(matrix[2], rmatrix);
        op().Set(*(GfMatrix4d*)matrix, _timeCode);
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
        op().Set(*(GfMatrix4d*)rmatrix, _timeCode);
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
        op().Set(*(GfMatrix4d*)rmatrix, _timeCode);
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
        multiply4x4(rmatrix, rotateMatrix, (d256*)&_invPostFrame);
        rmatrix[3] = translateToAddBackIn;
        op().Set(*(GfMatrix4d*)rmatrix, _timeCode);
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
        d256 rotateMatrix[4], rmatrix[4] = {loadu4d(_coordFrame[0]), loadu4d(_coordFrame[1]), loadu4d(_coordFrame[2]), zero4d()}, omatrix[4];
        quatToMatrix(offset, rotateMatrix);
        quatToMatrix(original, omatrix);
        rotateMatrix[3] = omatrix[3] = zero4d();
        multiply(rmatrix, omatrix, rmatrix);
        // remove translation
        rmatrix[3] = zero4d();
        multiply(rotateMatrix, rmatrix, rotateMatrix);
        rotateMatrix[3] = loadu4d(_worldFrame[3]);
        multiply4x4(rmatrix, rotateMatrix, (d256*)&_invPostFrame);
        d256 ctest = cross(rmatrix[0], rmatrix[1]);
        if(dot3(ctest, rmatrix[2]) < 0)
          rmatrix[2] = sub4d(zero4d(), rmatrix[2]);
        extractEuler(rmatrix, order , rot);
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
      auto half_angle = original * (M_PI / 360.0);
      auto orig_quat = set4d(std::sin(half_angle), 0, 0, std::cos(half_angle));
      auto new_rotation = process1AxisRotation(orig_quat, temp, space);
      
      // and back to degrees
      const double xrotate = std::atan2(new_rotation[0], new_rotation[3]) * (360.0 / M_PI);
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
      auto half_angle = original * (M_PI / 360.0);
      auto orig_quat = set4d(0, std::sin(half_angle), 0, std::cos(half_angle));
      auto new_rotation = process1AxisRotation(orig_quat, temp, space);
      
      // and back to degrees
      const double yrotate = std::atan2(new_rotation[1], new_rotation[3]) * (360.0 / M_PI);
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
      auto half_angle = original * (M_PI / 360.0);
      auto orig_quat = set4d(0, 0, std::sin(half_angle), std::cos(half_angle));
      auto new_rotation = process1AxisRotation(orig_quat, temp, space);
      
      // and back to degrees
      const double zrotate = std::atan2(new_rotation[2], new_rotation[3]) * (360.0 / M_PI);
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
          original = Quat_from_EulerXYZ(mul4d(original, splat4d(M_PI / 360.0)));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kXYZ);
          xformOp.Set(GfVec3d(rot), _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          f128 forig = zero4f();
          xformOp.Get((GfVec3f*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(forig);
          original = Quat_from_EulerXYZ(mul4d(original, splat4d(M_PI / 360.0)));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kXYZ);
          xformOp.Set(GfVec3f(rot), _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          i128 forig = zero4i();
          xformOp.Get((GfVec3h*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(cvtph4(forig));
          original = Quat_from_EulerXYZ(mul4d(original, splat4d(M_PI / 360.0)));
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
          original = Quat_from_EulerXZY(mul4d(original, splat4d(M_PI / 360.0)));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kXZY);
          xformOp.Set(GfVec3d(rot), _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          f128 forig = zero4f();
          xformOp.Get((GfVec3f*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(forig);
          original = Quat_from_EulerXZY(mul4d(original, splat4d(M_PI / 360.0)));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kXZY);
          xformOp.Set(rot, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          i128 forig = zero4i();
          xformOp.Get((GfVec3h*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(cvtph4(forig));
          original = Quat_from_EulerXZY(mul4d(original, splat4d(M_PI / 360.0)));
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
          original = Quat_from_EulerYXZ(mul4d(original, splat4d(M_PI / 360.0)));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kYXZ);
          xformOp.Set(GfVec3d(rot), _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          f128 forig = zero4f();
          xformOp.Get((GfVec3f*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(forig);
          original = Quat_from_EulerYXZ(mul4d(original, splat4d(M_PI / 360.0)));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kYXZ);
          xformOp.Set(rot, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          i128 forig = zero4i();
          xformOp.Get((GfVec3h*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(cvtph4(forig));
          original = Quat_from_EulerYXZ(mul4d(original, splat4d(M_PI / 360.0)));
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
          original = Quat_from_EulerYZX(mul4d(original, splat4d(M_PI / 360.0)));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kYZX);
          xformOp.Set(GfVec3d(rot), _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          f128 forig = zero4f();
          xformOp.Get((GfVec3f*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(forig);
          original = Quat_from_EulerYZX(mul4d(original, splat4d(M_PI / 360.0)));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kYZX);
          xformOp.Set(rot, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          i128 forig = zero4i();
          xformOp.Get((GfVec3h*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(cvtph4(forig));
          original = Quat_from_EulerYZX(mul4d(original, splat4d(M_PI / 360.0)));
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
          original = Quat_from_EulerZXY(mul4d(original, splat4d(M_PI / 360.0)));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kZXY);
          xformOp.Set(GfVec3d(rot), _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          f128 forig = zero4f();
          xformOp.Get((GfVec3f*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(forig);
          original = Quat_from_EulerZXY(mul4d(original, splat4d(M_PI / 360.0)));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kZXY);
          xformOp.Set(rot, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          i128 forig = zero4i();
          xformOp.Get((GfVec3h*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(cvtph4(forig));
          original = Quat_from_EulerZXY(mul4d(original, splat4d(M_PI / 360.0)));
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
          original = Quat_from_EulerZYX(mul4d(original, splat4d(M_PI / 360.0)));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kZYX);
          xformOp.Set(GfVec3d(rot), _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          f128 forig = zero4f();
          xformOp.Get((GfVec3f*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(forig);
          original = Quat_from_EulerZYX(mul4d(original, splat4d(M_PI / 360.0)));
          auto rot = process3AxisRotation(original, temp, space, RotationOrder::kZYX);
          xformOp.Set(rot, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          i128 forig = zero4i();
          xformOp.Get((GfVec3h*)&forig, _timeCode);
          d256 original = cvt4f_to_4d(cvtph4(forig));
          original = Quat_from_EulerZYX(mul4d(original, splat4d(M_PI / 360.0)));
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
          xformOp.Set(*(GfQuatd*)&original, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionFloat:
        {
          f128 original = set4f(0, 0, 0, 1.0f);
          xformOp.Get((GfQuatf*)&original, _timeCode);
          original = cvt4d_to_4f(multiplyQuat(cvt4f_to_4d(original), temp));
          xformOp.Set(*(GfQuatf*)&original, _timeCode);
        }
        break;

      case UsdGeomXformOp::PrecisionHalf:
        {
          i128 original = zero4i();
          xformOp.Get((GfQuath*)&original, _timeCode);
          original = cvtph4(cvt4d_to_4f(multiplyQuat(cvt4f_to_4d(cvtph4(original)), temp)));
          void* ptr = &original;
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
bool TransformOpProcessor::RotateX(const double radianChange, Space space)
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
            original += radianChange * (180.0 / M_PI);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            float original = 0;
            xformOp.Get(&original, _timeCode);
            original += radianChange * float(180.0 / M_PI);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            GfHalf original = 0;
            xformOp.Get(&original, _timeCode);
            original += radianChange * float(180.0 / M_PI);
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
            original[0] += radianChange * (180.0 / M_PI);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            GfVec3f original(0);
            xformOp.Get(&original, _timeCode);
            original[0] += radianChange * (180.0 / M_PI);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            GfVec3h original(0);
            xformOp.Get(&original, _timeCode);
            original[0] += radianChange * (180.0 / M_PI);
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
bool TransformOpProcessor::RotateY(const double radianChange, Space space)
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
            original += radianChange * (180.0 / M_PI);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            float original = 0;
            xformOp.Get(&original, _timeCode);
            original += float(radianChange * (180.0 / M_PI));
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            GfHalf original = 0;
            xformOp.Get(&original, _timeCode);
            original += float(radianChange * (180.0 / M_PI));
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
            original[1] += radianChange * (180.0 / M_PI);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            GfVec3f original(0);
            xformOp.Get(&original, _timeCode);
            original[1] += radianChange * (180.0 / M_PI);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            GfVec3h original(0);
            xformOp.Get(&original, _timeCode);
            original[1] += radianChange * (180.0 / M_PI);
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
bool TransformOpProcessor::RotateZ(const double radianChange, Space space)
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
            original += radianChange * (180.0 / M_PI);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            float original = 0;
            xformOp.Get(&original, _timeCode);
            original += float(radianChange * (180.0 / M_PI));
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            GfHalf original = 0;
            xformOp.Get(&original, _timeCode);
            original += float(radianChange * (180.0 / M_PI));
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
            original[2] += radianChange * (180.0 / M_PI);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            GfVec3f original(0);
            xformOp.Get(&original, _timeCode);
            original[2] += radianChange * (180.0 / M_PI);
            xformOp.Set(original, _timeCode);
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            GfVec3h original(0);
            xformOp.Get(&original, _timeCode);
            original[2] += radianChange * (180.0 / M_PI);
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
/// \brief  given an ordered set of XformOps, this method will evaluate the coordinate frame for a specifici xform op. 
///         By and large it will do this by concatonating operations of the same type, e.g. 
///          - successive translations will be grouped into 1 translation
///          - successive scales will be grouped into 1 scale
///          - successive rotations will be grouped into 1 quaternion
///         This will then be evaluated 
//----------------------------------------------------------------------------------------------------------------------------------------------------------
GfMatrix4d TransformOpProcessor::EvaluateCoordinateFrameForRange(const std::vector<UsdGeomXformOp>& ops, uint32_t start, uint32_t end, const UsdTimeCode& timeCode)
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

//----------------------------------------------------------------------------------------------------------------------------------------------------------
d256 TransformOpProcessor::_Rotation(const UsdGeomXformOp& op, const UsdTimeCode& timeCode)
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
            return v * (M_PI / 180.0) * 0.5;
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            float v;
            op.Get(&v, timeCode);
            return v * (M_PI / 180.0) * 0.5;
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            GfHalf v;
            op.Get(&v, timeCode);
            return _cvtsh_ss(v.bits()) * (M_PI / 180.0) * 0.5;
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
            return mul4d(v, splat4d(M_PI / 360.0));
          }
          break;

        case UsdGeomXformOp::PrecisionFloat:
          {
            f128 v;
            op.Get((GfVec3f*)&v, timeCode);
            return mul4d(cvt4f_to_4d(v), splat4d(M_PI / 360.0));
          }
          break;

        case UsdGeomXformOp::PrecisionHalf:
          {
            i128 v;
            op.Get((GfVec3h*)&v, timeCode);
            return mul4d(cvt4f_to_4d(cvtph4(v)), splat4d(M_PI / 360.0));
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

//----------------------------------------------------------------------------------------------------------------------------------------------------------
d256 TransformOpProcessor::_Translation(const UsdGeomXformOp& op, const UsdTimeCode& timeCode)
{
  d256 result = set4d(0.0, 0.0, 0.0, 0.0);
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

//----------------------------------------------------------------------------------------------------------------------------------------------------------
d256 TransformOpProcessor::_Scale(const UsdGeomXformOp& op, const UsdTimeCode& timeCode)
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

//----------------------------------------------------------------------------------------------------------------------------------------------------------
GfQuatd TransformOpProcessor::Rotation() const
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
GfVec3d TransformOpProcessor::Translation() const
{
  if(CanTranslate())
  {
    const d256 translate = _Translation(op(), _timeCode);
    return GfVec3d(translate[0], translate[1], translate[2]);
  }
  return GfVec3d(0.0);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
GfVec3d TransformOpProcessor::Scale() const
{
  if(CanScale())
  {
    const d256 scale = _Scale(op(), _timeCode);
    return GfVec3d(scale[0], scale[1], scale[2]);
  }
  return GfVec3d(1.0);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
bool TransformOpProcessorEx::SetTranslate(const GfVec3d& position, Space space)
{
  if(CanTranslate())
  {
    d256 translate = _Translation(op(), _timeCode);
    if(space == kTransform)
    {
      d256 offset = sub4d(set4d(position[0], position[1], position[2], 0), translate);
      return TransformOpProcessor::Translate(GfVec3d(offset[0], offset[1], offset[2]), space);
    }
    if(space == kWorld)
    {
      d256* pworldFrame = (d256*)&_worldFrame;
      d256 worldPos = transform(translate, pworldFrame);
      d256 world_offset = sub4d(set4d(position[0], position[1], position[2], 1.0), worldPos); 
      return TransformOpProcessor::Translate(GfVec3d(world_offset[0], world_offset[1], world_offset[2]), kWorld);
    }
    if(space == kPreTransform)
    {
      d256* pcoordFrame = (d256*)&_coordFrame;
      d256 worldPos = transform(translate, pcoordFrame);
      d256 parent_offset = sub4d(set4d(position[0], position[1], position[2], 1.0), worldPos); 
      return TransformOpProcessor::Translate(GfVec3d(parent_offset[0], parent_offset[1], parent_offset[2]), kPreTransform);
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
bool TransformOpProcessorEx::SetScale(const GfVec3d& scale, Space space)
{
  if(CanScale())
  {
    d256 original = _Scale(op(), _timeCode);
    d256 offset = div4d(set4d(scale[0], scale[1], scale[2], 0.0), original); 
    return TransformOpProcessor::Scale(GfVec3d(offset[0], offset[1], offset[2]), kWorld);
  }
  return false;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
bool TransformOpProcessorEx::SetRotate(const GfQuatd& orientation, Space space)
{
  if(CanRotate())
  {
    d256 final_orient = loadu4d(&orientation);
    d256 rotate = _Rotation(op(), _timeCode);
    if(space == kTransform)
    {
      d256 offset = multiplyQuat(rotate, quatInvert(final_orient));
      return TransformOpProcessor::Rotate(GfQuatd(offset[0], offset[1], offset[2], offset[3]), space);
    }
    if(space == kWorld)
    {
      d256 world_rotate = multiplyQuat(rotate, _qworldFrame);
      d256 world_offset = multiplyQuat(world_rotate, quatInvert(final_orient));
      return TransformOpProcessor::Rotate(GfQuatd(world_offset[0], world_offset[1], world_offset[2], world_offset[3]), space);
    }
    if(space == kPreTransform)
    {
      d256 world_rotate = multiplyQuat(rotate, _qcoordFrame);
      d256 world_offset = multiplyQuat(world_rotate, quatInvert(final_orient));
      return TransformOpProcessor::Rotate(GfQuatd(world_offset[0], world_offset[1], world_offset[2], world_offset[3]), space);
    }
  }
  return false;
}

bool TransformOpProcessorEx::Translate(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfVec3d& translateChange, Space space)
{
  TransformOpProcessorEx proc(prim, rotateAttr, TransformOpProcessorEx::kTranslate, timeCode);
  return proc.Translate(translateChange, space);
}

bool TransformOpProcessorEx::Scale(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfVec3d& scaleChange, Space space)
{
  TransformOpProcessorEx proc(prim, rotateAttr, TransformOpProcessorEx::kScale, timeCode);
  return proc.Scale(scaleChange, space);
}

bool TransformOpProcessorEx::RotateX(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const double radianChange, Space space)
{
  TransformOpProcessorEx proc(prim, rotateAttr, TransformOpProcessorEx::kTranslate, timeCode);
  return proc.RotateX(radianChange, space);
}

bool TransformOpProcessorEx::RotateY(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const double radianChange, Space space)
{
  TransformOpProcessorEx proc(prim, rotateAttr, TransformOpProcessorEx::kTranslate, timeCode);
  return proc.RotateY(radianChange, space);
}

bool TransformOpProcessorEx::RotateZ(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const double radianChange, Space space)
{
  TransformOpProcessorEx proc(prim, rotateAttr, TransformOpProcessorEx::kTranslate, timeCode);
  return proc.RotateZ(radianChange, space);
}

bool TransformOpProcessorEx::Rotate(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfQuatd& quatChange, Space space)
{
  TransformOpProcessorEx proc(prim, rotateAttr, TransformOpProcessorEx::kTranslate, timeCode);
  return proc.Rotate(quatChange, space);
}

bool TransformOpProcessorEx::SetTranslate(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfVec3d& position, Space space)
{
  TransformOpProcessorEx proc(prim, rotateAttr, TransformOpProcessorEx::kTranslate, timeCode);
  return proc.SetTranslate(position, space);
}

bool TransformOpProcessorEx::SetScale(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfVec3d& scale, Space space)
{
  TransformOpProcessorEx proc(prim, rotateAttr, TransformOpProcessorEx::kTranslate, timeCode);
  return proc.SetScale(scale, space);
}

bool TransformOpProcessorEx::SetRotate(UsdPrim prim, TfToken rotateAttr, UsdTimeCode timeCode, const GfQuatd& orientation, Space space)
{
  TransformOpProcessorEx proc(prim, rotateAttr, TransformOpProcessorEx::kTranslate, timeCode);
  return proc.SetRotate(orientation, space);
}


} // MayaUsdUtils