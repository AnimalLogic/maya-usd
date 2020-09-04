
#include <mayaUsdUtils/TransformManipulatorEx.h>
#include <mayaUsdUtils/TransformCache.h>

#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCache.h>

#include <gtest/gtest.h>
#include <fstream>

using namespace MayaUsdUtils;

#define COMPARE_MAT4(A, B, eps) \
 EXPECT_NEAR(A[0][0], B[0][0], eps); \
 EXPECT_NEAR(A[0][1], B[0][1], eps); \
 EXPECT_NEAR(A[0][2], B[0][2], eps); \
 EXPECT_NEAR(A[0][3], B[0][3], eps); \
 EXPECT_NEAR(A[1][0], B[1][0], eps); \
 EXPECT_NEAR(A[1][1], B[1][1], eps); \
 EXPECT_NEAR(A[1][2], B[1][2], eps); \
 EXPECT_NEAR(A[1][3], B[1][3], eps); \
 EXPECT_NEAR(A[2][0], B[2][0], eps); \
 EXPECT_NEAR(A[2][1], B[2][1], eps); \
 EXPECT_NEAR(A[2][2], B[2][2], eps); \
 EXPECT_NEAR(A[2][3], B[2][3], eps); \
 EXPECT_NEAR(A[3][0], B[3][0], eps); \
 EXPECT_NEAR(A[3][1], B[3][1], eps); \
 EXPECT_NEAR(A[3][2], B[3][2], eps); \
 EXPECT_NEAR(A[3][3], B[3][3], eps); 

//----------------------------------------------------------------------------------------------------------------------
// Test that a simple TRS transform evaluates correctly
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformCache, LocalSpaceEval)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp T = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble);
  UsdGeomXformOp R = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble);
  UsdGeomXformOp S = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble);
  T.Set(GfVec3d(1.0, 2.0, 3.0));
  R.Set(GfVec3d(4.0, 5.0, 6.0));
  S.Set(GfVec3d(7.0, 8.0, 9.0));

  TransformCache cache;
  auto local = cache.Local(xform.GetPrim(), UsdTimeCode::Default());
  EXPECT_EQ(1, cache.GetNumEntries());
  auto world = cache.World(xform.GetPrim(), UsdTimeCode::Default());
  auto invLocal = cache.InverseLocal(xform.GetPrim(), UsdTimeCode::Default());
  auto invWorld = cache.InverseWorld(xform.GetPrim(), UsdTimeCode::Default());
  EXPECT_EQ(1, cache.GetNumEntries());

  GfMatrix4d usdResult;
  bool reset;
  xform.GetLocalTransformation(&usdResult, &reset, UsdTimeCode::Default());
  COMPARE_MAT4(local, usdResult, 1e-5);
  COMPARE_MAT4(world, usdResult, 1e-5);
 
  GfMatrix4d invUsdResult = usdResult.GetInverse(); 
  COMPARE_MAT4(invLocal, invUsdResult, 1e-5);
  COMPARE_MAT4(invWorld, invUsdResult, 1e-5);

  EXPECT_EQ(UsdTimeCode::Default(), cache.CurrentTime());
}

//----------------------------------------------------------------------------------------------------------------------
// Test that world space matrices evaluate correctly.
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformCache, WorldSpaceEval)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/parent"));

  UsdGeomXformOp Tp = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble);
  UsdGeomXformOp Rp = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble);
  UsdGeomXformOp Sp = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble);
  Tp.Set(GfVec3d(1.0, 2.0, 3.0));
  Rp.Set(GfVec3d(4.0, 5.0, 6.0));
  Sp.Set(GfVec3d(7.0, 8.0, 9.0));

  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/parent/xform"));

  UsdGeomXformOp T = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble);
  UsdGeomXformOp R = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble);
  UsdGeomXformOp S = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble);
  T.Set(GfVec3d(1.0, 2.0, 3.0));
  R.Set(GfVec3d(4.0, 5.0, 6.0));
  S.Set(GfVec3d(7.0, 8.0, 9.0));

  TransformCache cache;
  auto local = cache.Local(xform.GetPrim(), UsdTimeCode::Default());
  EXPECT_EQ(2, cache.GetNumEntries());
  auto world = cache.World(xform.GetPrim(), UsdTimeCode::Default());
  auto invLocal = cache.InverseLocal(xform.GetPrim(), UsdTimeCode::Default());
  auto invWorld = cache.InverseWorld(xform.GetPrim(), UsdTimeCode::Default());
  EXPECT_EQ(2, cache.GetNumEntries());

  UsdGeomXformCache xcache(UsdTimeCode::Default());
  GfMatrix4d usdWorld = xcache.GetLocalToWorldTransform(xform.GetPrim());
  bool reset;
  GfMatrix4d usdResult;
  xform.GetLocalTransformation(&usdResult, &reset, UsdTimeCode::Default());
  COMPARE_MAT4(local, usdResult, 1e-5);
  COMPARE_MAT4(world, usdWorld, 1e-5);
 
  GfMatrix4d invUsdResult = usdResult.GetInverse(); 
  GfMatrix4d invUsdWorld = usdWorld.GetInverse();
  COMPARE_MAT4(invLocal, invUsdResult, 1e-5);
  COMPARE_MAT4(invWorld, invUsdWorld, 1e-5);

  EXPECT_EQ(UsdTimeCode::Default(), cache.CurrentTime());
}

//----------------------------------------------------------------------------------------------------------------------
// Make sure we take note of the reset transform stack.
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformCache, ResetTransformStack)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/parent"));

  UsdGeomXformOp Tp = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble);
  UsdGeomXformOp Rp = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble);
  UsdGeomXformOp Sp = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble);
  Tp.Set(GfVec3d(1.0, 2.0, 3.0));
  Rp.Set(GfVec3d(4.0, 5.0, 6.0));
  Sp.Set(GfVec3d(7.0, 8.0, 9.0));

  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/parent/xform"));
  xform.SetResetXformStack(true);

  UsdGeomXformOp T = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble);
  UsdGeomXformOp R = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble);
  UsdGeomXformOp S = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble);
  T.Set(GfVec3d(1.0, 2.0, 3.0));
  R.Set(GfVec3d(4.0, 5.0, 6.0));
  S.Set(GfVec3d(7.0, 8.0, 9.0));

  TransformCache cache;
  auto local = cache.Local(xform.GetPrim(), UsdTimeCode::Default());
  EXPECT_EQ(2, cache.GetNumEntries());
  auto world = cache.World(xform.GetPrim(), UsdTimeCode::Default());
  auto invLocal = cache.InverseLocal(xform.GetPrim(), UsdTimeCode::Default());
  auto invWorld = cache.InverseWorld(xform.GetPrim(), UsdTimeCode::Default());
  EXPECT_EQ(2, cache.GetNumEntries());
  bool reset;;
  GfMatrix4d usdResult;
  xform.GetLocalTransformation(&usdResult, &reset, UsdTimeCode::Default());
  COMPARE_MAT4(local, usdResult, 1e-5);
  COMPARE_MAT4(world, usdResult, 1e-5);
 
  GfMatrix4d invUsdResult = usdResult.GetInverse(); 
  COMPARE_MAT4(invLocal, invUsdResult, 1e-5);
  COMPARE_MAT4(invWorld, invUsdResult, 1e-5);

  EXPECT_EQ(UsdTimeCode::Default(), cache.CurrentTime());
}