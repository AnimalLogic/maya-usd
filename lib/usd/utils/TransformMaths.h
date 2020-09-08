#ifndef MAYAUSDUTILS_TRANSFOM_MATHS_H_
#define MAYAUSDUTILS_TRANSFOM_MATHS_H_

#include "SIMD.h"

namespace MayaUsdUtils {
namespace {

template<typename T> inline T radiansToDegrees(const T& rad) { return rad * T(180.0 / M_PI); }
template<typename T> inline T degreesToRadians(const T& rad) { return rad * T(M_PI / 180.0); }
template<typename T> inline T halfAngleToDegrees(const T& rad) { return rad * T(360.0 / M_PI); }
template<typename T> inline T degreesToHalfAngle(const T& rad) { return rad * T(M_PI / 360.0); }

template<> inline GfVec3d radiansToDegrees(const GfVec3d& rad) { return rad * double(180.0 / M_PI); }
template<> inline GfVec3d degreesToRadians(const GfVec3d& rad) { return rad * double(M_PI / 180.0); }
template<> inline GfVec3d halfAngleToDegrees(const GfVec3d& rad) { return rad * double(360.0 / M_PI); }
template<> inline GfVec3d degreesToHalfAngle(const GfVec3d& rad) { return rad * double(M_PI / 360.0); }

template<> inline d256 radiansToDegrees(const d256& rad) { return mul4d(rad, splat4d(180.0 / M_PI)); }
template<> inline d256 degreesToRadians(const d256& rad) { return mul4d(rad, splat4d(M_PI / 180.0)); }
template<> inline d256 halfAngleToDegrees(const d256& rad) { return mul4d(rad, splat4d(360.0 / M_PI)); }
template<> inline d256 degreesToHalfAngle(const d256& rad) { return mul4d(rad, splat4d(M_PI / 360.0)); }

//---------------------------------------------------------------------------------------------
// Note: If needed, it's possible to sneak a bit more performance from this code. I stopped 
// short of adding a vectorised sincos() implementation, but that would be possible....
//---------------------------------------------------------------------------------------------
inline d256 Quat_from_EulerXYZ(const d256 half_angles)
{
  const double x = get<0>(half_angles);
  const double y = get<1>(half_angles);
  const double z = get<2>(half_angles);
  const double sx = std::sin(x);
  const double cx = std::cos(x);
  const double sy = std::sin(y);
  const double cy = std::cos(y);
  const double sz = std::sin(z);
  const double cz = std::cos(z);
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
  const double x = get<0>(half_angles);
  const double y = get<1>(half_angles);
  const double z = get<2>(half_angles);
  const double sx = std::sin(x);
  const double cx = std::cos(x);
  const double sy = std::sin(y);
  const double cy = std::cos(y);
  const double sz = std::sin(z);
  const double cz = std::cos(z);
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
  const double x = get<0>(half_angles);
  const double y = get<1>(half_angles);
  const double z = get<2>(half_angles);
  const double sx = std::sin(x);
  const double cx = std::cos(x);
  const double sy = std::sin(y);
  const double cy = std::cos(y);
  const double sz = std::sin(z);
  const double cz = std::cos(z);
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
  const double x = get<0>(half_angles);
  const double y = get<1>(half_angles);
  const double z = get<2>(half_angles);
  const double sx = std::sin(x);
  const double cx = std::cos(x);
  const double sy = std::sin(y);
  const double cy = std::cos(y);
  const double sz = std::sin(z);
  const double cz = std::cos(z);
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
  const double x = get<0>(half_angles);
  const double y = get<1>(half_angles);
  const double z = get<2>(half_angles);
  const double sx = std::sin(x);
  const double cx = std::cos(x);
  const double sy = std::sin(y);
  const double cy = std::cos(y);
  const double sz = std::sin(z);
  const double cz = std::cos(z);
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
  const double x = get<0>(half_angles);
  const double y = get<1>(half_angles);
  const double z = get<2>(half_angles);
  const double sx = std::sin(x);
  const double cx = std::cos(x);
  const double sy = std::sin(y);
  const double cy = std::cos(y);
  const double sz = std::sin(z);
  const double cz = std::cos(z);
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

inline void extractEuler(const d256 M[4], RotationOrder rotOrder, GfVec3f& rot)
{
  double mat[4][4];
  store4d(mat[0], M[0]);
  store4d(mat[1], M[1]);
  store4d(mat[2], M[2]);
  store4d(mat[3], M[3]);
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
    rot[rowSin] = s*M_PI/2.0;
    rot[rowCos] = 0;
  }
  else
  if (std::fabs(mat[row][col] + 1) < epsilon)
  {    
    rot[row] = std::atan2(-s*mat[rowSin][colCos], mat[rowSin][colSin]);
    rot[rowSin] = -s*M_PI/2.0;
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
  return get<0>(ab) + get<1>(ab) + get<2>(ab);
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

  double W = 1.0 + get<0>(frame[0]) + get<1>(frame[1]) + get<2>(frame[2]);
  W = std::sqrt(W);
  const double qx = (get<2>(frame[1]) - get<1>(frame[2]));
  const double qy = (get<0>(frame[2]) - get<2>(frame[0]));
  const double s = (0.5 / W);
  const double qz = (get<1>(frame[0]) - get<0>(frame[1]));
  return set4d(qx * s, qy * s, qz * s, W * 0.5);
}

inline d256 normaliseVec3(const d256& v)
{
  auto l2 = mul4d(v, v);
  l2 = splat4d( std::sqrt(get<0>(l2) + get<1>(l2) + get<2>(l2)) );
  return div4d(v, l2);
}

} // end anon

/// there is room for improvement here!
inline GfVec3d QuatToEulerXYZ(const GfQuatd q)
{
  GfVec3f rotate;
  extractEuler(loadu4d(&q), RotationOrder::kXYZ, rotate);
  return radiansToDegrees(GfVec3d(rotate[0], rotate[1], rotate[2]));
}

inline GfQuatd QuatFromEulerXYZ(const GfVec3d& degrees)
{
  GfVec3d h = degreesToHalfAngle(degrees);
  auto ha = set4d(h[0], h[1], h[2], 0.0);
  auto q = Quat_from_EulerXYZ(ha);
  GfQuatd Q;
  storeu4d(&Q, q);
  return Q;
}

inline GfQuatd QuatFromEulerXYZ(double x, double y, double z)
{
  return QuatFromEulerXYZ(GfVec3d(x, y, z));
}

}

#endif
