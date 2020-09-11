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
#include <mayaUsdUtils/TransformManipulatorEx.h>

#include <pxr/usd/usdGeom/xform.h>

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
// Test that ops that have no value set don't accumulate any garbage as a result
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, no_scale_value)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), first.GetOpName());
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());
  EXPECT_TRUE(processor.Scale(GfVec3d(2.0, 0.5, 0.3), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3d scale;
  first.Get(&scale);
  EXPECT_NEAR(2.0, scale[0], 1e-5f);
  EXPECT_NEAR(0.5, scale[1], 1e-5f);
  EXPECT_NEAR(0.3, scale[2], 1e-5f);

  EXPECT_TRUE(processor.Scale(GfVec3d(2.0, 0.5, 0.3), MayaUsdUtils::TransformManipulator::kTransform));

  first.Get(&scale);
  EXPECT_NEAR(2.0 * 2.0, scale[0], 1e-5f);
  EXPECT_NEAR(0.5 * 0.5, scale[1], 1e-5f);
  EXPECT_NEAR(0.3 * 0.3, scale[2], 1e-5f);
}

TEST(TransformManipulator, no_translate_value)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), first.GetOpName());
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());
  EXPECT_TRUE(processor.Translate(GfVec3d(2.0, 0.5, 0.3), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3d translate;
  first.Get(&translate);
  EXPECT_NEAR(2.0, translate[0], 1e-5f);
  EXPECT_NEAR(0.5, translate[1], 1e-5f);
  EXPECT_NEAR(0.3, translate[2], 1e-5f);

  EXPECT_TRUE(processor.Translate(GfVec3d(2.0, 0.5, 0.3), MayaUsdUtils::TransformManipulator::kTransform));

  first.Get(&translate);
  EXPECT_NEAR(4.0, translate[0], 1e-5f);
  EXPECT_NEAR(1.0, translate[1], 1e-5f);
  EXPECT_NEAR(0.6, translate[2], 1e-5f);
}

TEST(TransformManipulator, no_rotate_value)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  double sx = std::sin( (45.0f * (M_PI / 360.0) ) );
  double cx = std::cos( (45.0f * (M_PI / 360.0) ) );
  GfQuatd Q(cx, sx, 0, 0);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate[0], 1e-5f);
  EXPECT_NEAR(0.0, rotate[1], 1e-5f);
  EXPECT_NEAR(0.0, rotate[2], 1e-5f);

  // 45 degrees around X
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate[0], 1e-5f);
  EXPECT_NEAR(0.0, rotate[1], 1e-5f);
  EXPECT_NEAR(0.0, rotate[2], 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Make sure we can rotate single axis rotations (double)
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, rotateXd)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  double sx = std::sin( (45.0 * (M_PI / 360.0) ) );
  double cx = std::cos( (45.0 * (M_PI / 360.0) ) );
  GfQuatd Q(cx, sx, 0, 0);
  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), first.GetOpName());
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  double rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5);

  // 45 degrees around X
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5);
}

TEST(TransformManipulator, rotateYd)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);


  double sx = std::sin( (45.0 * (M_PI / 360.0) ) );
  double cx = std::cos( (45.0 * (M_PI / 360.0) ) );
  GfQuatd Q(cx, 0, sx, 0);
  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), first.GetOpName());
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around Y
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  double rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5);

  // 45 degrees around Y
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5);
}

TEST(TransformManipulator, rotateZd)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  double sx = std::sin( (45.0 * (M_PI / 360.0) ) );
  double cx = std::cos( (45.0 * (M_PI / 360.0) ) );
  GfQuatd Q(cx, 0, 0, sx);
  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), first.GetOpName());
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around Z
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  double rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5);

  // 45 degrees around Z
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5);
}

//----------------------------------------------------------------------------------------------------------------------
// Make sure we can rotate three axis rotations (float)
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, rotateXf)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  double sx = std::sin( (45.0 * (M_PI / 360.0) ) );
  double cx = std::cos( (45.0 * (M_PI / 360.0) ) );
  GfQuatd Q(cx, sx, 0, 0);
  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), first.GetOpName());
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  float rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5f);

  // 45 degrees around X
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5f);
}

TEST(TransformManipulator, rotateYf)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  double sx = std::sin( (45.0 * (M_PI / 360.0) ) );
  double cx = std::cos( (45.0 * (M_PI / 360.0) ) );
  GfQuatd Q(cx, 0, sx, 0);
  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), first.GetOpName());
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  float rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5f);

  // 45 degrees around X
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5f);
}

TEST(TransformManipulator, rotateZf)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  double sx = std::sin( (45.0 * (M_PI / 360.0) ) );
  double cx = std::cos( (45.0 * (M_PI / 360.0) ) );
  GfQuatd Q(cx, 0, 0, sx);
  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), first.GetOpName());
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  float rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5f);

  // 45 degrees around X
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Make sure we can rotate three axis rotations (half)
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, rotateXh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  double sx = std::sin( (45.0 * (M_PI / 360.0) ) );
  double cx = std::cos( (45.0 * (M_PI / 360.0) ) );
  GfQuatd Q(cx, sx, 0, 0);
  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), first.GetOpName());
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  GfHalf rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5f);

  // 45 degrees around X
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5f);
}

TEST(TransformManipulator, rotateYh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  double sx = std::sin( (45.0 * (M_PI / 360.0) ) );
  double cx = std::cos( (45.0 * (M_PI / 360.0) ) );
  GfQuatd Q(cx, 0, sx, 0);
  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), first.GetOpName());
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  GfHalf rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5f);

  // 45 degrees around X
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5f);
}

TEST(TransformManipulator, rotateZh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  double sx = std::sin( (45.0 * (M_PI / 360.0) ) );
  double cx = std::cos( (45.0 * (M_PI / 360.0) ) );
  GfQuatd Q(cx, 0, 0, sx);
  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), first.GetOpName());
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  GfHalf rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5f);

  // 45 degrees around X
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kTransform));

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// When modifying the 'X' angle in an XYZ rotation, we can go down an optimised path that simply sets the 'X' value
// directly.
//----------------------------------------------------------------------------------------------------------------------

TEST(TransformManipulator, rotateXYZd_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateX(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(15.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-5f);
}
TEST(TransformManipulator, rotateXYZf_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateX(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(15.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-5f);
}
TEST(TransformManipulator, rotateXYZh_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateX(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3h rotate;
  first.Get(&rotate);
  EXPECT_NEAR(15.0, rotate[0], 1e-3f);
  EXPECT_NEAR(18.0, rotate[1], 1e-3f);
  EXPECT_NEAR(42.0, rotate[2], 1e-3f);
}

//----------------------------------------------------------------------------------------------------------------------
// When modifying the 'X' angle in an XZY rotation, we can go down an optimised path that simply sets the 'X' value
// directly.
//----------------------------------------------------------------------------------------------------------------------

TEST(TransformManipulator, rotateXZYd_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateX(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(15.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-5f);
}
TEST(TransformManipulator, rotateXZYf_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateX(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(15.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-5f);
}
TEST(TransformManipulator, rotateXZYh_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateX(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3h rotate;
  first.Get(&rotate);
  EXPECT_NEAR(15.0, rotate[0], 1e-3f);
  EXPECT_NEAR(18.0, rotate[1], 1e-3f);
  EXPECT_NEAR(42.0, rotate[2], 1e-3f);
}

//----------------------------------------------------------------------------------------------------------------------
// When modifying the 'Y' angle in an YXZ rotation, we can go down an optimised path that simply sets the 'Y' value
// directly.
//----------------------------------------------------------------------------------------------------------------------

TEST(TransformManipulator, rotateYXZd_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(23.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-5f);
}
TEST(TransformManipulator, rotateYXZf_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(23.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-5f);
}
TEST(TransformManipulator, rotateYXZh_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3h rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(23.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-3f);
}

//----------------------------------------------------------------------------------------------------------------------
// When modifying the 'Y' angle in an YZX rotation, we can go down an optimised path that simply sets the 'Y' value
// directly.
//----------------------------------------------------------------------------------------------------------------------

TEST(TransformManipulator, rotateYZXd_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(23.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-5f);
}
TEST(TransformManipulator, rotateYZXf_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(23.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-5f);
}
TEST(TransformManipulator, rotateYZXh_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  EXPECT_TRUE(processor.RotateY(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3h rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(23.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-3f);
}

//----------------------------------------------------------------------------------------------------------------------
// When modifying the 'Y' angle in an YXZ rotation, we can go down an optimised path that simply sets the 'Y' value
// directly.
//----------------------------------------------------------------------------------------------------------------------

TEST(TransformManipulator, rotateZXYd_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZXYOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateZ(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(47.0, rotate[2], 1e-5f);
}
TEST(TransformManipulator, rotateZXYf_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZXYOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateZ(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(47.0, rotate[2], 1e-5f);
}
TEST(TransformManipulator, rotateZXYh_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZXYOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateZ(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3h rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(47.0, rotate[2], 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// When modifying the 'Z' angle in an ZYX rotation, we can go down an optimised path that simply sets the 'Z' value
// directly.
//----------------------------------------------------------------------------------------------------------------------

TEST(TransformManipulator, rotateZYXd_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateZ(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(47.0, rotate[2], 1e-5f);
}
TEST(TransformManipulator, rotateZYXf_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateZ(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(47.0, rotate[2], 1e-5f);
}
TEST(TransformManipulator, rotateZYXh_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  EXPECT_TRUE(processor.RotateZ(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3h rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(47.0, rotate[2], 1e-5f);
}


//----------------------------------------------------------------------------------------------------------------------
// Using an XYZ rotation order, replicate some rotations that may occur with the Maya rotate tool, and check we end up 
// with the same result. The Y and Z rotations will utilise quaternions.
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, rotateXYZd)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(19.390714, rotate[0], 1e-5f);
  EXPECT_NEAR(-52.266911, rotate[1], 1e-5f);
  EXPECT_NEAR(32.255846, rotate[2], 1e-5f);
}

TEST(TransformManipulator, rotateXYZf)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(19.390714, rotate[0], 1e-5f);
  EXPECT_NEAR(-52.266911, rotate[1], 1e-5f);
  EXPECT_NEAR(32.255846, rotate[2], 1e-5f);
}

TEST(TransformManipulator, rotateXYZh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3h rotate;
  first.Get(&rotate);
  EXPECT_NEAR(19.390714, rotate[0], 1e-1f);
  EXPECT_NEAR(-52.266911, rotate[1], 1e-1f);
  EXPECT_NEAR(32.255846, rotate[2], 1e-1f);
}

//----------------------------------------------------------------------------------------------------------------------
// Using an XZY rotation order, replicate some rotations that may occur with the Maya rotate tool, and check we end up 
// with the same result. The Y and Z rotations will utilise quaternions.
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, rotateXZYd)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.915175, rotate[0], 1e-5f);
  EXPECT_NEAR(-56.79962, rotate[1], 1e-5f);
  EXPECT_NEAR(19.063526, rotate[2], 1e-5f);
}

TEST(TransformManipulator, rotateXZYf)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.915175, rotate[0], 1e-5f);
  EXPECT_NEAR(-56.79962, rotate[1], 1e-5f);
  EXPECT_NEAR(19.063526, rotate[2], 1e-5f);
}

TEST(TransformManipulator, rotateXZYh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3h rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.915175, rotate[0], 1e-1f);
  EXPECT_NEAR(-56.79962, rotate[1], 1e-1f);
  EXPECT_NEAR(19.063526, rotate[2], 1e-1f);
}

//----------------------------------------------------------------------------------------------------------------------
// Using an YXZ rotation order, replicate some rotations that may occur with the Maya rotate tool, and check we end up 
// with the same result. The Y and Z rotations will utilise quaternions.
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, rotateYXZd)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYXZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(11.723195, rotate[0], 1e-5f);
  EXPECT_NEAR(-53.873641, rotate[1], 1e-5f);
  EXPECT_NEAR(47.811204, rotate[2], 1e-5f);
}

TEST(TransformManipulator, rotateYXZf)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYXZOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(11.723195, rotate[0], 1e-5f);
  EXPECT_NEAR(-53.873641, rotate[1], 1e-5f);
  EXPECT_NEAR(47.811204, rotate[2], 1e-5f);
}

TEST(TransformManipulator, rotateYXZh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYXZOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3h rotate;
  first.Get(&rotate);
  EXPECT_NEAR(11.723195, rotate[0], 1e-1f);
  EXPECT_NEAR(-53.873641, rotate[1], 1e-1f);
  EXPECT_NEAR(47.811204, rotate[2], 1e-1f);
}

//----------------------------------------------------------------------------------------------------------------------
// Using an YZX rotation order, replicate some rotations that may occur with the Maya rotate tool, and check we end up 
// with the same result. The Y and Z rotations will utilise quaternions.
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, rotateYZXd)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(17.170789, rotate[0], 1e-5f);
  EXPECT_NEAR(-41.238612, rotate[1], 1e-5f);
  EXPECT_NEAR(46.508833, rotate[2], 1e-5f);
}

TEST(TransformManipulator, rotateYZXf)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(17.170789, rotate[0], 1e-5f);
  EXPECT_NEAR(-41.238612, rotate[1], 1e-5f);
  EXPECT_NEAR(46.508833, rotate[2], 1e-5f);
}

TEST(TransformManipulator, rotateYZXh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3h rotate;
  first.Get(&rotate);
  EXPECT_NEAR(17.170789, rotate[0], 1e-1f);
  EXPECT_NEAR(-41.238612, rotate[1], 1e-1f);
  EXPECT_NEAR(46.508833, rotate[2], 1e-1f);
}


//----------------------------------------------------------------------------------------------------------------------
// Using an ZYX rotation order, replicate some rotations that may occur with the Maya rotate tool, and check we end up 
// with the same result. The Y and Z rotations will utilise quaternions.
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, rotateZXYd)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZXYOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(42.759017, rotate[0], 1e-5f);
  EXPECT_NEAR(-38.164457, rotate[1], 1e-5f);
  EXPECT_NEAR(26.413781, rotate[2], 1e-5f);
}

TEST(TransformManipulator, rotateZXYf)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZXYOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(42.759017, rotate[0], 1e-5f);
  EXPECT_NEAR(-38.164457, rotate[1], 1e-5f);
  EXPECT_NEAR(26.413781, rotate[2], 1e-5f);
}

TEST(TransformManipulator, rotateZXYh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZXYOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3h rotate;
  first.Get(&rotate);
  EXPECT_NEAR(42.759017, rotate[0], 1e-1f);
  EXPECT_NEAR(-38.164457, rotate[1], 1e-1f);
  EXPECT_NEAR(26.413781, rotate[2], 1e-1f);
}

//----------------------------------------------------------------------------------------------------------------------
// Using an ZYX rotation order, replicate some rotations that may occur with the Maya rotate tool, and check we end up 
// with the same result. The Y and Z rotations will utilise quaternions.
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, rotateZYXd)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(49.6261, rotate[0], 1e-5f);
  EXPECT_NEAR(-26.980493, rotate[1], 1e-5f);
  EXPECT_NEAR(54.49695, rotate[2], 1e-5f);
}

TEST(TransformManipulator, rotateZYXf)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(49.6261, rotate[0], 1e-5f);
  EXPECT_NEAR(-26.980493, rotate[1], 1e-5f);
  EXPECT_NEAR(54.49695, rotate[2], 1e-5f);
}

TEST(TransformManipulator, rotateZYXh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  // 45 degrees around X
  EXPECT_TRUE(processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));
  EXPECT_TRUE(processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

  GfVec3h rotate;
  first.Get(&rotate);
  EXPECT_NEAR(49.6261, rotate[0], 1e-1f);
  EXPECT_NEAR(-26.980493, rotate[1], 1e-1f);
  EXPECT_NEAR(54.49695, rotate[2], 1e-1f);
}

//----------------------------------------------------------------------------------------------------------------------
// Using an XYZ rotation order, replicate some rotations that may occur with the Maya rotate tool, and check we end up 
// with the same result. The Y and Z rotations will utilise quaternions.
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, rotate_world_space_d_no_scale)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));

  parent_translate.Set(GfVec3d(-2.0, 3.0, 1.0));
  parent_rotate.Set(GfVec3d(-11.0, -21.0, 22.0));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(0.865601,0.349725,0.358368,0,-0.304323,0.935764,-0.178136,0,-0.397646,0.0451345,0.916428,0,-2,3,1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  child_translate.Set(GfVec3d(2.0, 1.0, 2.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0,0,1,0,0,0,0,1,0,2,1,2,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }

  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  {
    const GfMatrix4d expected(0.865601,0.349725,0.358368,0,-0.304323,0.935764,-0.178136,0,-0.397646,0.0451345,0.916428,0,-1.368415,4.725484,3.371456,1);
    COMPARE_MAT4(expected, processor.WorldFrame(), 1e-5f);
  }

  // rotate 15 degrees around X in world space
  {
    GfVec3d rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_TRUE(processor.RotateX(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    child_rotate.Get(&rotate);
    EXPECT_NEAR(13.218885, rotate[0], 1e-5f);
    EXPECT_NEAR(-3.843776, rotate[1], 1e-5f);
    EXPECT_NEAR(-6.439076, rotate[2], 1e-5f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.RotateY(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(16.968949, rotate[0], 1e-5f);
    EXPECT_NEAR(10.668345, rotate[1], 1e-5f);
    EXPECT_NEAR(-5.533085, rotate[2], 1e-5f);
  }

  // rotate 15 degrees around Z in world space
  {
    EXPECT_TRUE(processor.RotateZ(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(22.315417, rotate[0], 1e-5f);
    EXPECT_NEAR(7.835664, rotate[1], 1e-5f);
    EXPECT_NEAR(9.086346, rotate[2], 1e-5f);
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Apply a world space translation on a simple set up
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, translate_world_space_d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-3.0, -2.0, -1.0));
  parent_rotate.Set(GfVec3d(10.0, 15.0, 20.0));
  parent_scale.Set(GfVec3d(1.1, 1.2, 1.3));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(0.998441,0.363403,-0.284701,0,-0.353509,1.128946,0.201278,0,0.388579,-0.0987992,1.236627,0,-3,-2,-1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  child_translate.Set(GfVec3d(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0,0,1,0,0,0,0,1,0,3,4,5,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(6.810837, translate[0], 1e-5f);
    EXPECT_NEAR(7.022931, translate[1], 1e-5f);
    EXPECT_NEAR(9.024255, translate[2], 1e-5f);
  }

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(10.621674, translate[0], 1e-5f);
    EXPECT_NEAR(10.045862, translate[1], 1e-5f);
    EXPECT_NEAR(13.04851, translate[2], 1e-5f);
  }
}

TEST(TransformManipulator, translate_world_space_f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-3.0, -2.0, -1.0));
  parent_rotate.Set(GfVec3d(10.0, 15.0, 20.0));
  parent_scale.Set(GfVec3d(1.1, 1.2, 1.3));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(0.998441,0.363403,-0.284701,0,-0.353509,1.128946,0.201278,0,0.388579,-0.0987992,1.236627,0,-3,-2,-1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("child_translate"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  child_translate.Set(GfVec3f(3.0f, 4.0f, 5.0f));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0,0,1,0,0,0,0,1,0,3,4,5,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  // rotate 15 degrees around X in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(6.810837, translate[0], 1e-5f);
    EXPECT_NEAR(7.022931, translate[1], 1e-5f);
    EXPECT_NEAR(9.024255, translate[2], 1e-5f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(10.621674, translate[0], 1e-5f);
    EXPECT_NEAR(10.045862, translate[1], 1e-5f);
    EXPECT_NEAR(13.04851, translate[2], 1e-5f);
  }
}

TEST(TransformManipulator, translate_world_space_h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-3.0, -2.0, -1.0));
  parent_rotate.Set(GfVec3d(10.0, 15.0, 20.0));
  parent_scale.Set(GfVec3d(1.1, 1.2, 1.3));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(0.998441,0.363403,-0.284701,0,-0.353509,1.128946,0.201278,0,0.388579,-0.0987992,1.236627,0,-3,-2,-1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("child_translate"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  child_translate.Set(GfVec3h(3.0f, 4.0f, 5.0f));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0,0,1,0,0,0,0,1,0,3,4,5,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  // rotate 15 degrees around X in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(6.810837, translate[0], 1e-2f);
    EXPECT_NEAR(7.022931, translate[1], 1e-2f);
    EXPECT_NEAR(9.024255, translate[2], 1e-2f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(10.621674, translate[0], 1e-2f);
    EXPECT_NEAR(10.045862, translate[1], 1e-2f);
    EXPECT_NEAR(13.04851, translate[2], 1e-2f);
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Test world space translations against the results given by Maya (double precision).
// Tested with a varying number of xform operations of different types
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, translate_parent_space1_d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-3.0, -2.0, -1.0));
  parent_rotate.Set(GfVec3d(10.0, 15.0, 20.0));
  parent_scale.Set(GfVec3d(1.1, 1.2, 1.3));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(0.998441,0.363403,-0.284701,0,-0.353509,1.128946,0.201278,0,0.388579,-0.0987992,1.236627,0,-3,-2,-1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  child_translate.Set(GfVec3d(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0,0,1,0,0,0,0,1,0,3,4,5,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  // translate in parent space - this should be the same as a simple local space transform in this case
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kParent));
    GfVec3d translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(7.3, translate[0], 1e-5f);
    EXPECT_NEAR(8.4, translate[1], 1e-5f);
    EXPECT_NEAR(9.5, translate[2], 1e-5f);
  }

  // translate in parent space - this should be the same as a simple local space transform in this case
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kParent));
    GfVec3d translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(11.6, translate[0], 1e-5f);
    EXPECT_NEAR(12.8, translate[1], 1e-5f);
    EXPECT_NEAR(14.0, translate[2], 1e-5f);
  }
}

TEST(TransformManipulator, translate_world_space2_d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));

  parent_rotate.Set(GfVec3d(22.0, 33.0, 44.0));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,0,0,0,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  child_translate.Set(GfVec3d(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0, 0,1,0,0, 0,0,1,0, 3,4,5,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  // rotate 15 degrees around X in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(5.706661, translate[0], 1e-5f);
    EXPECT_NEAR(6.833556, translate[1], 1e-5f);
    EXPECT_NEAR(11.537962, translate[2], 1e-5f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(8.413323, translate[0], 1e-5f);
    EXPECT_NEAR(9.667113, translate[1], 1e-5f);
    EXPECT_NEAR(18.075923, translate[2], 1e-5f);
  }
}

TEST(TransformManipulator, translate_world_space3_d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));

  parent_rotate.Set(GfVec3d(22.0, 33.0, 44.0));
  parent_translate.Set(GfVec3d(3.0, 4.0, 5.0));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,0,0,0,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(parent.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  // rotate 15 degrees around X in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d translate(0);
    parent_translate.Get(&translate);

    EXPECT_NEAR(5.706661, translate[0], 1e-5f);
    EXPECT_NEAR(6.833556, translate[1], 1e-5f);
    EXPECT_NEAR(11.537962, translate[2], 1e-5f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d translate(0);
    parent_translate.Get(&translate);
    EXPECT_NEAR(8.413323, translate[0], 1e-5f);
    EXPECT_NEAR(9.667113, translate[1], 1e-5f);
    EXPECT_NEAR(18.075923, translate[2], 1e-5f);
  }
}

TEST(TransformManipulator, translate_world_space4_d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));
  parent_scale.Set(GfVec3d(2, 2, 2));

  // sanity check starting matrix against maya result
  const GfMatrix4d parent_matrix(2, 0, 0,0,  0, 2, 0, 0,  0, 0, 2, 0, 0, 0, 0, 1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }
  
  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  child_rotate.Set(GfVec3d(22.0, 33.0, 44.0));
  child_translate.Set(GfVec3d(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,2.937999,5.389119,3.510778,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d presult = TransformManipulator::EvaluateCoordinateFrameForIndex(parent.GetOrderedXformOps(&resetsXformStack), 1, UsdTimeCode::Default());
    GfMatrix4d cresult = TransformManipulator::EvaluateCoordinateFrameForIndex(child.GetOrderedXformOps(&resetsXformStack), 2, UsdTimeCode::Default());
    GfMatrix4d result = cresult * presult;
    GfMatrix4d expected(1.206578,1.165179,-1.089278,0,-0.994625,1.617376,0.628343,0,1.246952,0.162639,1.555204,0,5.875997,10.778238,7.021556,1);
    COMPARE_MAT4(result, expected, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  // rotate 15 degrees around X in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(4.353331, translate[0], 1e-5f);
    EXPECT_NEAR(5.416778, translate[1], 1e-5f);
    EXPECT_NEAR(8.268981, translate[2], 1e-5f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(5.706661, translate[0], 1e-5f);
    EXPECT_NEAR(6.833556, translate[1], 1e-5f);
    EXPECT_NEAR(11.537962, translate[2], 1e-5f);
  }
}

TEST(TransformManipulator, translate_world_space5_d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));
  parent_scale.Set(GfVec3d(2, 2, 2));

  // sanity check starting matrix against maya result
  const GfMatrix4d parent_matrix(2, 0, 0,0,  0, 2, 0, 0,  0, 0, 2, 0, 0, 0, 0, 1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }
  
  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  child_translate.Set(GfVec3d(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0, 3, 4, 5, 1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(5.150, translate[0], 1e-5f);
    EXPECT_NEAR(6.200, translate[1], 1e-5f);
    EXPECT_NEAR(7.250, translate[2], 1e-5f);
  }

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(7.3, translate[0], 1e-5f);
    EXPECT_NEAR(8.4, translate[1], 1e-5f);
    EXPECT_NEAR(9.5, translate[2], 1e-5f);
  }
}

TEST(TransformManipulator, translate_world_space6_d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));


  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  child_rotate.Set(GfVec3d(22.0, 33.0, 44.0));
  child_translate.Set(GfVec3d(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,0,0,0,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
    
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(5.706661, translate[0], 1e-5f);
    EXPECT_NEAR(6.833556, translate[1], 1e-5f);
    EXPECT_NEAR(11.537962, translate[2], 1e-5f);
  }

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(8.413323, translate[0], 1e-5f);
    EXPECT_NEAR(9.667113, translate[1], 1e-5f);
    EXPECT_NEAR(18.075923, translate[2], 1e-5f);
  }
}

TEST(TransformManipulator, translate_world_space7_d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));
  parent_scale.Set(GfVec3d(2, 2, 2));

  // sanity check starting matrix against maya result
  const GfMatrix4d parent_matrix(2, 0, 0,0,  0, 2, 0, 0,  0, 0, 2, 0, 0, 0, 0, 1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }
  
  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  child_rotate.Set(GfVec3d(22.0, 33.0, 44.0));
  child_translate.Set(GfVec3d(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,2.937999,5.389119,3.510778,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d presult = TransformManipulator::EvaluateCoordinateFrameForIndex(parent.GetOrderedXformOps(&resetsXformStack), 1, UsdTimeCode::Default());
    GfMatrix4d cresult = TransformManipulator::EvaluateCoordinateFrameForIndex(child.GetOrderedXformOps(&resetsXformStack), 2, UsdTimeCode::Default());
    GfMatrix4d result = cresult * presult;
    GfMatrix4d expected(1.206578,1.165179,-1.089278,0,-0.994625,1.617376,0.628343,0,1.246952,0.162639,1.555204,0,5.875997,10.778238,7.021556,1);
    COMPARE_MAT4(result, expected, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(4.353331, translate[0], 1e-5f);
    EXPECT_NEAR(5.416778, translate[1], 1e-5f);
    EXPECT_NEAR(8.268981, translate[2], 1e-5f);
  }

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(5.706661, translate[0], 1e-5f);
    EXPECT_NEAR(6.833556, translate[1], 1e-5f);
    EXPECT_NEAR(11.537962, translate[2], 1e-5f);
  }
}

TEST(TransformManipulator, translate_world_space8_d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-3.0, -2.0, -1.0));
  parent_rotate.Set(GfVec3d(10.0, 15.0, 20.0));
  parent_scale.Set(GfVec3d(1.1, 1.2, 1.3));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(0.998441,0.363403,-0.284701,0,-0.353509,1.128946,0.201278,0,0.388579,-0.0987992,1.236627,0,-3,-2,-1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  child_rotate.Set(GfVec3d(22, 33, 44));
  child_translate.Set(GfVec3d(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,2.937999,5.389119,3.510778,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }

  {
    bool resetsXformStack = false;
    GfMatrix4d presult = TransformManipulator::EvaluateCoordinateFrameForIndex(parent.GetOrderedXformOps(&resetsXformStack), 3, UsdTimeCode::Default());
    GfMatrix4d cresult = TransformManipulator::EvaluateCoordinateFrameForIndex(child.GetOrderedXformOps(&resetsXformStack), 2, UsdTimeCode::Default());
    const GfMatrix4d expected(0.184763,0.930759,-0.72801,0,-0.660335,0.701201,0.692869,0,0.895916,0.241552,0.800467,0,-0.607473,4.804837,3.589779,1);
    GfMatrix4d result = cresult * presult;
    COMPARE_MAT4(result, expected, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  {
    const GfMatrix4d expected(0.184763,0.930759,-0.72801,0,-0.660335,0.701201,0.692869,0,0.895916,0.241552,0.800467,0,-3,-2,-1,1);
    COMPARE_MAT4(expected, processor.WorldFrame(), 1e-5f);
  }
  
  // rotate 15 degrees around X in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(4.868398, translate[0], 1e-5f);
    EXPECT_NEAR(5.813738, translate[1], 1e-5f);
    EXPECT_NEAR(10.751057, translate[2], 1e-5f);
  }
    {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d presult = TransformManipulator::EvaluateCoordinateFrameForIndex(parent.GetOrderedXformOps(&resetsXformStack), 3, UsdTimeCode::Default());
    GfMatrix4d cresult = TransformManipulator::EvaluateCoordinateFrameForIndex(child.GetOrderedXformOps(&resetsXformStack), 2, UsdTimeCode::Default());
    const GfMatrix4d expected(0.184763,0.930759,-0.72801,0,-0.660335,0.701201,0.692869,0,0.895916,0.241552,0.800467,0,3.692527,9.204837,8.089779,1);
    GfMatrix4d result = cresult * presult;
    COMPARE_MAT4(result, expected, 1e-5f);
  }

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(6.736796, translate[0], 1e-5f);
    EXPECT_NEAR(7.627476, translate[1], 1e-5f);
    EXPECT_NEAR(16.502115, translate[2], 1e-5f);
  }
    {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d presult = TransformManipulator::EvaluateCoordinateFrameForIndex(parent.GetOrderedXformOps(&resetsXformStack), 3, UsdTimeCode::Default());
    GfMatrix4d cresult = TransformManipulator::EvaluateCoordinateFrameForIndex(child.GetOrderedXformOps(&resetsXformStack), 2, UsdTimeCode::Default());
    const GfMatrix4d expected(0.184763,0.930759,-0.72801,0,-0.660335,0.701201,0.692869,0,0.895916,0.241552,0.800467,0,-0.607473 + 4.3 + 4.3,4.4 + 4.4 +4.804837,4.5 + 4.5 + 3.589779,1);
    GfMatrix4d result = cresult * presult;
    COMPARE_MAT4(result, expected, 1e-5f);
  }
}


//----------------------------------------------------------------------------------------------------------------------
// Test world space translations against the results given by Maya (double precision).
// Tested with a varying number of xform operations of different types
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, translate_parent_space1_f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-3.0, -2.0, -1.0));
  parent_rotate.Set(GfVec3d(10.0, 15.0, 20.0));
  parent_scale.Set(GfVec3d(1.1, 1.2, 1.3));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(0.998441,0.363403,-0.284701,0,-0.353509,1.128946,0.201278,0,0.388579,-0.0987992,1.236627,0,-3,-2,-1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("child_translate"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  child_translate.Set(GfVec3f(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0,0,1,0,0,0,0,1,0,3,4,5,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  // translate in parent space - this should be the same as a simple local space transform in this case
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kParent));
    GfVec3f translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(7.3, translate[0], 1e-5f);
    EXPECT_NEAR(8.4, translate[1], 1e-5f);
    EXPECT_NEAR(9.5, translate[2], 1e-5f);
  }

  // translate in parent space - this should be the same as a simple local space transform in this case
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kParent));
    GfVec3f translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(11.6, translate[0], 1e-5f);
    EXPECT_NEAR(12.8, translate[1], 1e-5f);
    EXPECT_NEAR(14.0, translate[2], 1e-5f);
  }
}

TEST(TransformManipulator, translate_world_space2_f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));

  parent_rotate.Set(GfVec3d(22.0, 33.0, 44.0));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,0,0,0,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("child_translate"));
  child_translate.Set(GfVec3f(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0, 0,1,0,0, 0,0,1,0, 3,4,5,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  // rotate 15 degrees around X in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(5.706661, translate[0], 1e-5f);
    EXPECT_NEAR(6.833556, translate[1], 1e-5f);
    EXPECT_NEAR(11.537962, translate[2], 1e-5f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(8.413323, translate[0], 1e-5f);
    EXPECT_NEAR(9.667113, translate[1], 1e-5f);
    EXPECT_NEAR(18.075923, translate[2], 1e-5f);
  }
}

TEST(TransformManipulator, translate_world_space3_f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("parent_translate"));

  parent_rotate.Set(GfVec3d(22.0, 33.0, 44.0));
  parent_translate.Set(GfVec3f(3.0, 4.0, 5.0));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,0,0,0,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(parent.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  // rotate 15 degrees around X in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f translate(0);
    parent_translate.Get(&translate);

    EXPECT_NEAR(5.706661, translate[0], 1e-5f);
    EXPECT_NEAR(6.833556, translate[1], 1e-5f);
    EXPECT_NEAR(11.537962, translate[2], 1e-5f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f translate(0);
    parent_translate.Get(&translate);
    EXPECT_NEAR(8.413323, translate[0], 1e-5f);
    EXPECT_NEAR(9.667113, translate[1], 1e-5f);
    EXPECT_NEAR(18.075923, translate[2], 1e-5f);
  }
}

TEST(TransformManipulator, translate_world_space4_f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));
  parent_scale.Set(GfVec3d(2, 2, 2));

  // sanity check starting matrix against maya result
  const GfMatrix4d parent_matrix(2, 0, 0,0,  0, 2, 0, 0,  0, 0, 2, 0, 0, 0, 0, 1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }
  
  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("child_translate"));
  child_rotate.Set(GfVec3d(22.0, 33.0, 44.0));
  child_translate.Set(GfVec3f(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,2.937999,5.389119,3.510778,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d presult = TransformManipulator::EvaluateCoordinateFrameForIndex(parent.GetOrderedXformOps(&resetsXformStack), 1, UsdTimeCode::Default());
    GfMatrix4d cresult = TransformManipulator::EvaluateCoordinateFrameForIndex(child.GetOrderedXformOps(&resetsXformStack), 2, UsdTimeCode::Default());
    GfMatrix4d result = cresult * presult;
    GfMatrix4d expected(1.206578,1.165179,-1.089278,0,-0.994625,1.617376,0.628343,0,1.246952,0.162639,1.555204,0,5.875997,10.778238,7.021556,1);
    COMPARE_MAT4(result, expected, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  // rotate 15 degrees around X in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(4.353331, translate[0], 1e-5f);
    EXPECT_NEAR(5.416778, translate[1], 1e-5f);
    EXPECT_NEAR(8.268981, translate[2], 1e-5f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(5.706661, translate[0], 1e-5f);
    EXPECT_NEAR(6.833556, translate[1], 1e-5f);
    EXPECT_NEAR(11.537962, translate[2], 1e-5f);
  }
}

TEST(TransformManipulator, translate_world_space5_f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));
  parent_scale.Set(GfVec3d(2, 2, 2));

  // sanity check starting matrix against maya result
  const GfMatrix4d parent_matrix(2, 0, 0,0,  0, 2, 0, 0,  0, 0, 2, 0, 0, 0, 0, 1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }
  
  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("child_translate"));
  child_translate.Set(GfVec3f(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0, 3, 4, 5, 1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(5.150, translate[0], 1e-5f);
    EXPECT_NEAR(6.200, translate[1], 1e-5f);
    EXPECT_NEAR(7.250, translate[2], 1e-5f);
  }

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(7.3, translate[0], 1e-5f);
    EXPECT_NEAR(8.4, translate[1], 1e-5f);
    EXPECT_NEAR(9.5, translate[2], 1e-5f);
  }
}

TEST(TransformManipulator, translate_world_space6_f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));


  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("child_translate"));
  child_rotate.Set(GfVec3d(22.0, 33.0, 44.0));
  child_translate.Set(GfVec3f(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,0,0,0,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
    
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(5.706661, translate[0], 1e-5f);
    EXPECT_NEAR(6.833556, translate[1], 1e-5f);
    EXPECT_NEAR(11.537962, translate[2], 1e-5f);
  }

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(8.413323, translate[0], 1e-5f);
    EXPECT_NEAR(9.667113, translate[1], 1e-5f);
    EXPECT_NEAR(18.075923, translate[2], 1e-5f);
  }
}

TEST(TransformManipulator, translate_world_space7_f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));
  parent_scale.Set(GfVec3d(2, 2, 2));

  // sanity check starting matrix against maya result
  const GfMatrix4d parent_matrix(2, 0, 0,0,  0, 2, 0, 0,  0, 0, 2, 0, 0, 0, 0, 1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }
  
  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("child_translate"));
  child_rotate.Set(GfVec3d(22.0, 33.0, 44.0));
  child_translate.Set(GfVec3f(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,2.937999,5.389119,3.510778,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d presult = TransformManipulator::EvaluateCoordinateFrameForIndex(parent.GetOrderedXformOps(&resetsXformStack), 1, UsdTimeCode::Default());
    GfMatrix4d cresult = TransformManipulator::EvaluateCoordinateFrameForIndex(child.GetOrderedXformOps(&resetsXformStack), 2, UsdTimeCode::Default());
    GfMatrix4d result = cresult * presult;
    GfMatrix4d expected(1.206578,1.165179,-1.089278,0,-0.994625,1.617376,0.628343,0,1.246952,0.162639,1.555204,0,5.875997,10.778238,7.021556,1);
    COMPARE_MAT4(result, expected, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(4.353331, translate[0], 1e-5f);
    EXPECT_NEAR(5.416778, translate[1], 1e-5f);
    EXPECT_NEAR(8.268981, translate[2], 1e-5f);
  }

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(5.706661, translate[0], 1e-5f);
    EXPECT_NEAR(6.833556, translate[1], 1e-5f);
    EXPECT_NEAR(11.537962, translate[2], 1e-5f);
  }
}

TEST(TransformManipulator, translate_world_space8_f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-3.0, -2.0, -1.0));
  parent_rotate.Set(GfVec3d(10.0, 15.0, 20.0));
  parent_scale.Set(GfVec3d(1.1, 1.2, 1.3));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(0.998441,0.363403,-0.284701,0,-0.353509,1.128946,0.201278,0,0.388579,-0.0987992,1.236627,0,-3,-2,-1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("child_translate"));
  child_rotate.Set(GfVec3d(22, 33, 44));
  child_translate.Set(GfVec3f(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,2.937999,5.389119,3.510778,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }

  {
    bool resetsXformStack = false;
    GfMatrix4d presult = TransformManipulator::EvaluateCoordinateFrameForIndex(parent.GetOrderedXformOps(&resetsXformStack), 3, UsdTimeCode::Default());
    GfMatrix4d cresult = TransformManipulator::EvaluateCoordinateFrameForIndex(child.GetOrderedXformOps(&resetsXformStack), 2, UsdTimeCode::Default());
    const GfMatrix4d expected(0.184763,0.930759,-0.72801,0,-0.660335,0.701201,0.692869,0,0.895916,0.241552,0.800467,0,-0.607473,4.804837,3.589779,1);
    GfMatrix4d result = cresult * presult;
    COMPARE_MAT4(result, expected, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  {
    const GfMatrix4d expected(0.184763,0.930759,-0.72801,0,-0.660335,0.701201,0.692869,0,0.895916,0.241552,0.800467,0,-3,-2,-1,1);
    COMPARE_MAT4(expected, processor.WorldFrame(), 1e-5f);
  }
  
  // rotate 15 degrees around X in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(4.868398, translate[0], 1e-5f);
    EXPECT_NEAR(5.813738, translate[1], 1e-5f);
    EXPECT_NEAR(10.751057, translate[2], 1e-5f);
  }
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d presult = TransformManipulator::EvaluateCoordinateFrameForIndex(parent.GetOrderedXformOps(&resetsXformStack), 3, UsdTimeCode::Default());
    GfMatrix4d cresult = TransformManipulator::EvaluateCoordinateFrameForIndex(child.GetOrderedXformOps(&resetsXformStack), 2, UsdTimeCode::Default());
    const GfMatrix4d expected(0.184763,0.930759,-0.72801,0,-0.660335,0.701201,0.692869,0,0.895916,0.241552,0.800467,0,3.692527,9.204837,8.089779,1);
    GfMatrix4d result = cresult * presult;
    COMPARE_MAT4(result, expected, 1e-5f);
  }

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(6.736796, translate[0], 1e-5f);
    EXPECT_NEAR(7.627476, translate[1], 1e-5f);
    EXPECT_NEAR(16.502115, translate[2], 1e-5f);
  }
    {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d presult = TransformManipulator::EvaluateCoordinateFrameForIndex(parent.GetOrderedXformOps(&resetsXformStack), 3, UsdTimeCode::Default());
    GfMatrix4d cresult = TransformManipulator::EvaluateCoordinateFrameForIndex(child.GetOrderedXformOps(&resetsXformStack), 2, UsdTimeCode::Default());
    const GfMatrix4d expected(0.184763,0.930759,-0.72801,0,-0.660335,0.701201,0.692869,0,0.895916,0.241552,0.800467,0,-0.607473 + 4.3 + 4.3,4.4 + 4.4 +4.804837,4.5 + 4.5 + 3.589779,1);
    GfMatrix4d result = cresult * presult;
    COMPARE_MAT4(result, expected, 1e-5f);
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Test world space translations against the results given by Maya (half precision).
// Tested with a varying number of xform operations of different types
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, translate_parent_space1_h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-3.0, -2.0, -1.0));
  parent_rotate.Set(GfVec3d(10.0, 15.0, 20.0));
  parent_scale.Set(GfVec3d(1.1, 1.2, 1.3));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(0.998441,0.363403,-0.284701,0,-0.353509,1.128946,0.201278,0,0.388579,-0.0987992,1.236627,0,-3,-2,-1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("child_translate"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  child_translate.Set(GfVec3h(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0,0,1,0,0,0,0,1,0,3,4,5,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  // translate in parent space - this should be the same as a simple local space transform in this case
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kParent));
    GfVec3h translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(7.3, translate[0], 2e-3f);
    EXPECT_NEAR(8.4, translate[1], 3.2e-3f);
    EXPECT_NEAR(9.5, translate[2], 2e-3f);
  }

  // translate in parent space - this should be the same as a simple local space transform in this case
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kParent));
    GfVec3h translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(11.6, translate[0], 2e-3f);
    EXPECT_NEAR(12.8, translate[1], 3.2e-3f);
    EXPECT_NEAR(14.0, translate[2], 2.3e-3f);
  }
}

TEST(TransformManipulator, translate_world_space2_h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));

  parent_rotate.Set(GfVec3d(22.0, 33.0, 44.0));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,0,0,0,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("child_translate"));
  child_translate.Set(GfVec3h(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0, 0,1,0,0, 0,0,1,0, 3,4,5,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  // rotate 15 degrees around X in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(5.706661, translate[0], 2e-3f);
    EXPECT_NEAR(6.833556, translate[1], 2e-3f);
    EXPECT_NEAR(11.537962, translate[2], 2e-3f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(8.413323, translate[0], 2e-3f);
    EXPECT_NEAR(9.667113, translate[1], 3.1e-3f);
    EXPECT_NEAR(18.075923, translate[2], 2.3e-3f);
  }
}

TEST(TransformManipulator, translate_world_space3_h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("parent_translate"));

  parent_rotate.Set(GfVec3d(22.0, 33.0, 44.0));
  parent_translate.Set(GfVec3h(3.0, 4.0, 5.0));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,0,0,0,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(parent.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  // rotate 15 degrees around X in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h translate(0);
    parent_translate.Get(&translate);

    EXPECT_NEAR(5.706661, translate[0], 2e-3f);
    EXPECT_NEAR(6.833556, translate[1], 2e-3f);
    EXPECT_NEAR(11.537962, translate[2], 2e-3f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h translate(0);
    parent_translate.Get(&translate);
    EXPECT_NEAR(8.413323, translate[0], 2e-3f);
    EXPECT_NEAR(9.667113, translate[1], 3.1e-3f);
    EXPECT_NEAR(18.075923, translate[2], 2.3e-3f);
  }
}

TEST(TransformManipulator, translate_world_space4_h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));
  parent_scale.Set(GfVec3d(2, 2, 2));

  // sanity check starting matrix against maya result
  const GfMatrix4d parent_matrix(2, 0, 0,0,  0, 2, 0, 0,  0, 0, 2, 0, 0, 0, 0, 1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }
  
  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("child_translate"));
  child_rotate.Set(GfVec3d(22.0, 33.0, 44.0));
  child_translate.Set(GfVec3h(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,2.937999,5.389119,3.510778,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d presult = TransformManipulator::EvaluateCoordinateFrameForIndex(parent.GetOrderedXformOps(&resetsXformStack), 1, UsdTimeCode::Default());
    GfMatrix4d cresult = TransformManipulator::EvaluateCoordinateFrameForIndex(child.GetOrderedXformOps(&resetsXformStack), 2, UsdTimeCode::Default());
    GfMatrix4d result = cresult * presult;
    GfMatrix4d expected(1.206578,1.165179,-1.089278,0,-0.994625,1.617376,0.628343,0,1.246952,0.162639,1.555204,0,5.875997,10.778238,7.021556,1);
    COMPARE_MAT4(result, expected, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  // rotate 15 degrees around X in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(4.353331, translate[0], 2e-3f);
    EXPECT_NEAR(5.416778, translate[1], 2e-3f);
    EXPECT_NEAR(8.268981, translate[2], 3.4e-3f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(5.706661, translate[0], 3.6e-3f);
    EXPECT_NEAR(6.833556, translate[1], 2.4e-3f);
    EXPECT_NEAR(11.537962, translate[2], 6.8e-3f);
  }
}

TEST(TransformManipulator, translate_world_space5_h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));
  parent_scale.Set(GfVec3d(2, 2, 2));

  // sanity check starting matrix against maya result
  const GfMatrix4d parent_matrix(2, 0, 0,0,  0, 2, 0, 0,  0, 0, 2, 0, 0, 0, 0, 1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }
  
  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("child_translate"));
  child_translate.Set(GfVec3h(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0, 3, 4, 5, 1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(5.150, translate[0], 2e-3f);
    EXPECT_NEAR(6.200, translate[1], 2e-3f);
    EXPECT_NEAR(7.250, translate[2], 2e-3f);
  }

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(7.3, translate[0], 3.2e-3f);
    EXPECT_NEAR(8.4, translate[1], 2e-3f);
    EXPECT_NEAR(9.5, translate[2], 2e-3f);
  }
}

TEST(TransformManipulator, translate_world_space6_h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));


  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("child_translate"));
  child_rotate.Set(GfVec3d(22.0, 33.0, 44.0));
  child_translate.Set(GfVec3h(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,0,0,0,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
    
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(5.706661, translate[0], 2e-3f);
    EXPECT_NEAR(6.833556, translate[1], 2e-3f);
    EXPECT_NEAR(11.537962, translate[2], 2e-3f);
  }

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(8.413323, translate[0], 2e-3f);
    EXPECT_NEAR(9.667113, translate[1], 3.1e-3f);
    EXPECT_NEAR(18.075923, translate[2], 2.3e-3f);
  }
}

TEST(TransformManipulator, translate_world_space7_h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));
  parent_scale.Set(GfVec3d(2, 2, 2));

  // sanity check starting matrix against maya result
  const GfMatrix4d parent_matrix(2, 0, 0,0,  0, 2, 0, 0,  0, 0, 2, 0, 0, 0, 0, 1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }
  
  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("child_translate"));
  child_rotate.Set(GfVec3d(22.0, 33.0, 44.0));
  child_translate.Set(GfVec3h(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,2.937999,5.389119,3.510778,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d presult = TransformManipulator::EvaluateCoordinateFrameForIndex(parent.GetOrderedXformOps(&resetsXformStack), 1, UsdTimeCode::Default());
    GfMatrix4d cresult = TransformManipulator::EvaluateCoordinateFrameForIndex(child.GetOrderedXformOps(&resetsXformStack), 2, UsdTimeCode::Default());
    GfMatrix4d result = cresult * presult;
    GfMatrix4d expected(1.206578,1.165179,-1.089278,0,-0.994625,1.617376,0.628343,0,1.246952,0.162639,1.555204,0,5.875997,10.778238,7.021556,1);
    COMPARE_MAT4(result, expected, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  {
    processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld);
    GfVec3h translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(4.353331, translate[0], 2e-3f);
    EXPECT_NEAR(5.416778, translate[1], 2e-3f);
    EXPECT_NEAR(8.268981, translate[2], 3.4e-3f);
  }

  {
    processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld);
    GfVec3h translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(5.706661, translate[0], 3.6e-3f);
    EXPECT_NEAR(6.833556, translate[1], 2.4e-3f);
    EXPECT_NEAR(11.537962, translate[2], 6.8e-3f);
  }
}

TEST(TransformManipulator, translate_world_space8_h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-3.0, -2.0, -1.0));
  parent_rotate.Set(GfVec3d(10.0, 15.0, 20.0));
  parent_scale.Set(GfVec3d(1.1, 1.2, 1.3));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(0.998441,0.363403,-0.284701,0,-0.353509,1.128946,0.201278,0,0.388579,-0.0987992,1.236627,0,-3,-2,-1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("child_translate"));
  child_rotate.Set(GfVec3d(22, 33, 44));
  child_translate.Set(GfVec3h(3.0, 4.0, 5.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(0.603289,0.58259,-0.544639,0,-0.497312,0.808688,0.314172,0,0.623476,0.0813195,0.777602,0,2.937999,5.389119,3.510778,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }

  {
    bool resetsXformStack = false;
    GfMatrix4d presult = TransformManipulator::EvaluateCoordinateFrameForIndex(parent.GetOrderedXformOps(&resetsXformStack), 3, UsdTimeCode::Default());
    GfMatrix4d cresult = TransformManipulator::EvaluateCoordinateFrameForIndex(child.GetOrderedXformOps(&resetsXformStack), 2, UsdTimeCode::Default());
    const GfMatrix4d expected(0.184763,0.930759,-0.72801,0,-0.660335,0.701201,0.692869,0,0.895916,0.241552,0.800467,0,-0.607473,4.804837,3.589779,1);
    GfMatrix4d result = cresult * presult;
    COMPARE_MAT4(result, expected, 1e-5f);
  }
  
  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  {
    const GfMatrix4d expected(0.184763,0.930759,-0.72801,0,-0.660335,0.701201,0.692869,0,0.895916,0.241552,0.800467,0,-3,-2,-1,1);
    COMPARE_MAT4(expected, processor.WorldFrame(), 1e-5f);
  }
  
  // rotate 15 degrees around X in world space
  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h translate(0);
    child_translate.Get(&translate);

    EXPECT_NEAR(4.868398, translate[0], 2e-3f);
    EXPECT_NEAR(5.813738, translate[1], 2e-3f);
    EXPECT_NEAR(10.751057, translate[2], 2e-3f);
  }
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d presult = TransformManipulator::EvaluateCoordinateFrameForIndex(parent.GetOrderedXformOps(&resetsXformStack), 3, UsdTimeCode::Default());
    GfMatrix4d cresult = TransformManipulator::EvaluateCoordinateFrameForIndex(child.GetOrderedXformOps(&resetsXformStack), 2, UsdTimeCode::Default());
    const GfMatrix4d expected(0.184763,0.930759,-0.72801,0,-0.660335,0.701201,0.692869,0,0.895916,0.241552,0.800467,0,3.692527,9.204837,8.089779,1);
    GfMatrix4d result = cresult * presult;
    COMPARE_MAT4(result, expected, 2.5e-3f);
  }

  {
    EXPECT_TRUE(processor.Translate(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h translate(0);
    child_translate.Get(&translate);
    EXPECT_NEAR(6.736796, translate[0], 2.5e-3f);
    EXPECT_NEAR(7.627476, translate[1], 2.5e-3f);
    EXPECT_NEAR(16.502115, translate[2], 2.2e-3f);
  }
    {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d presult = TransformManipulator::EvaluateCoordinateFrameForIndex(parent.GetOrderedXformOps(&resetsXformStack), 3, UsdTimeCode::Default());
    GfMatrix4d cresult = TransformManipulator::EvaluateCoordinateFrameForIndex(child.GetOrderedXformOps(&resetsXformStack), 2, UsdTimeCode::Default());
    const GfMatrix4d expected(0.184763,0.930759,-0.72801,0,-0.660335,0.701201,0.692869,0,0.895916,0.241552,0.800467,0,-0.607473 + 4.3 + 4.3,4.4 + 4.4 +4.804837,4.5 + 4.5 + 3.589779,1);
    GfMatrix4d result = cresult * presult;
    COMPARE_MAT4(result, expected, 4.5e-2f);
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Scaling in world space is only valid if the scale is uniform. Non uniform scales are rejected.
//----------------------------------------------------------------------------------------------------------------------

TEST(TransformManipulator, scale_world_space_d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-3.0, -2.0, -1.0));
  parent_rotate.Set(GfVec3d(10.0, 15.0, 20.0));
  parent_scale.Set(GfVec3d(1.1, 1.2, 1.3));

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_scale = child.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_scale"));
  child_rotate.Set(GfVec3d(22, 33, 44));
  child_translate.Set(GfVec3d(3.0, 4.0, 5.0));
  child_scale.Set(GfVec3d(-2.0, 5.0, 3.0));

  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 2);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());
  
  {
    // non-uniform scales are ignored in world space
    EXPECT_FALSE(processor.Scale(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d scale(0);
    child_scale.Get(&scale);

    // should be the same as the original
    EXPECT_NEAR(-2.0, scale[0], 1e-5f);
    EXPECT_NEAR(5.0, scale[1], 1e-5f);
    EXPECT_NEAR(3.0, scale[2], 1e-5f);
  }
  {
    // uniform scales are handled in world space
    EXPECT_TRUE(processor.Scale(GfVec3d(4.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d scale(0);
    child_scale.Get(&scale);

    // should be the same as the original * 4.0
    EXPECT_NEAR(-8.0, scale[0], 1e-5f);
    EXPECT_NEAR(20.0, scale[1], 1e-5f);
    EXPECT_NEAR(12.0, scale[2], 1e-5f);
  }
}

TEST(TransformManipulator, scale_world_space_f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-3.0, -2.0, -1.0));
  parent_rotate.Set(GfVec3d(10.0, 15.0, 20.0));
  parent_scale.Set(GfVec3d(1.1, 1.2, 1.3));

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_scale = child.AddScaleOp(UsdGeomXformOp::PrecisionFloat, TfToken("child_scale"));
  child_rotate.Set(GfVec3d(22, 33, 44));
  child_translate.Set(GfVec3d(3.0, 4.0, 5.0));
  child_scale.Set(GfVec3f(-2.0, 5.0, 3.0));

  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 2);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());
  
  {
    // non-uniform scales are ignored in world space
    EXPECT_FALSE(processor.Scale(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f scale(0);
    child_scale.Get(&scale);

    // should be the same as the original
    EXPECT_NEAR(-2.0, scale[0], 1e-5f);
    EXPECT_NEAR(5.0, scale[1], 1e-5f);
    EXPECT_NEAR(3.0, scale[2], 1e-5f);
  }
  {
    // uniform scales are handled in world space
    EXPECT_TRUE(processor.Scale(GfVec3d(4.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f scale(0);
    child_scale.Get(&scale);

    // should be the same as the original * 4.0
    EXPECT_NEAR(-8.0, scale[0], 1e-5f);
    EXPECT_NEAR(20.0, scale[1], 1e-5f);
    EXPECT_NEAR(12.0, scale[2], 1e-5f);
  }
}

TEST(TransformManipulator, scale_world_space_h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-3.0, -2.0, -1.0));
  parent_rotate.Set(GfVec3d(10.0, 15.0, 20.0));
  parent_scale.Set(GfVec3d(1.1, 1.2, 1.3));

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_scale = child.AddScaleOp(UsdGeomXformOp::PrecisionHalf, TfToken("child_scale"));
  child_rotate.Set(GfVec3d(22, 33, 44));
  child_translate.Set(GfVec3d(3.0, 4.0, 5.0));
  child_scale.Set(GfVec3h(-2.0, 5.0, 3.0));

  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 2);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());
  
  {
    // non-uniform scales are ignored in world space
    EXPECT_FALSE(processor.Scale(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h scale(0);
    child_scale.Get(&scale);

    // should be the same as the original
    EXPECT_NEAR(-2.0, scale[0], 1e-5f);
    EXPECT_NEAR(5.0, scale[1], 1e-5f);
    EXPECT_NEAR(3.0, scale[2], 1e-5f);
  }
  {
    // uniform scales are handled in world space
    EXPECT_TRUE(processor.Scale(GfVec3d(4.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h scale(0);
    child_scale.Get(&scale);

    // should be the same as the original * 4.0
    EXPECT_NEAR(-8.0, scale[0], 1e-5f);
    EXPECT_NEAR(20.0, scale[1], 1e-5f);
    EXPECT_NEAR(12.0, scale[2], 1e-5f);
  }
}


//----------------------------------------------------------------------------------------------------------------------
// Scaling in world space is only valid if the scale is uniform. Non uniform scales are rejected.
//----------------------------------------------------------------------------------------------------------------------

TEST(TransformManipulator, scale_parent_space_d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-3.0, -2.0, -1.0));
  parent_rotate.Set(GfVec3d(10.0, 15.0, 20.0));
  parent_scale.Set(GfVec3d(1.1, 1.2, 1.3));

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_scale = child.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_scale"));
  child_rotate.Set(GfVec3d(22, 33, 44));
  child_translate.Set(GfVec3d(3.0, 4.0, 5.0));
  child_scale.Set(GfVec3d(-2.0, 5.0, 3.0));

  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 2);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());
  
  {
    // non-uniform scales are ignored in world space
    EXPECT_FALSE(processor.Scale(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kParent));
    GfVec3d scale(0);
    child_scale.Get(&scale);

    // should be the same as the original
    EXPECT_NEAR(-2.0, scale[0], 1e-5f);
    EXPECT_NEAR(5.0, scale[1], 1e-5f);
    EXPECT_NEAR(3.0, scale[2], 1e-5f);
  }
  {
    // uniform scales are handled in world space
    EXPECT_TRUE(processor.Scale(GfVec3d(4.0), MayaUsdUtils::TransformManipulator::kParent));
    GfVec3d scale(0);
    child_scale.Get(&scale);

    // should be the same as the original * 4.0
    EXPECT_NEAR(-8.0, scale[0], 1e-5f);
    EXPECT_NEAR(20.0, scale[1], 1e-5f);
    EXPECT_NEAR(12.0, scale[2], 1e-5f);
  }
}

TEST(TransformManipulator, scale_parent_space_f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-3.0, -2.0, -1.0));
  parent_rotate.Set(GfVec3d(10.0, 15.0, 20.0));
  parent_scale.Set(GfVec3d(1.1, 1.2, 1.3));

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_scale = child.AddScaleOp(UsdGeomXformOp::PrecisionFloat, TfToken("child_scale"));
  child_rotate.Set(GfVec3d(22, 33, 44));
  child_translate.Set(GfVec3d(3.0, 4.0, 5.0));
  child_scale.Set(GfVec3f(-2.0, 5.0, 3.0));

  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 2);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());
  
  {
    // non-uniform scales are ignored in world space
    EXPECT_FALSE(processor.Scale(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kParent));
    GfVec3f scale(0);
    child_scale.Get(&scale);

    // should be the same as the original
    EXPECT_NEAR(-2.0, scale[0], 1e-5f);
    EXPECT_NEAR(5.0, scale[1], 1e-5f);
    EXPECT_NEAR(3.0, scale[2], 1e-5f);
  }
  {
    // uniform scales are handled in world space
    EXPECT_TRUE(processor.Scale(GfVec3d(4.0), MayaUsdUtils::TransformManipulator::kParent));
    GfVec3f scale(0);
    child_scale.Get(&scale);

    // should be the same as the original * 4.0
    EXPECT_NEAR(-8.0, scale[0], 1e-5f);
    EXPECT_NEAR(20.0, scale[1], 1e-5f);
    EXPECT_NEAR(12.0, scale[2], 1e-5f);
  }
}

TEST(TransformManipulator, scale_parent_space_h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-3.0, -2.0, -1.0));
  parent_rotate.Set(GfVec3d(10.0, 15.0, 20.0));
  parent_scale.Set(GfVec3d(1.1, 1.2, 1.3));

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_scale = child.AddScaleOp(UsdGeomXformOp::PrecisionHalf, TfToken("child_scale"));
  child_rotate.Set(GfVec3d(22, 33, 44));
  child_translate.Set(GfVec3d(3.0, 4.0, 5.0));
  child_scale.Set(GfVec3h(-2.0, 5.0, 3.0));

  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 2);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());
  
  {
    // non-uniform scales are ignored in world space
    EXPECT_FALSE(processor.Scale(GfVec3d(4.3, 4.4, 4.5), MayaUsdUtils::TransformManipulator::kParent));
    GfVec3h scale(0);
    child_scale.Get(&scale);

    // should be the same as the original
    EXPECT_NEAR(-2.0, scale[0], 1e-5f);
    EXPECT_NEAR(5.0, scale[1], 1e-5f);
    EXPECT_NEAR(3.0, scale[2], 1e-5f);
  }
  {
    // uniform scales are handled in world space
    EXPECT_TRUE(processor.Scale(GfVec3d(4.0), MayaUsdUtils::TransformManipulator::kParent));
    GfVec3h scale(0);
    child_scale.Get(&scale);

    // should be the same as the original * 4.0
    EXPECT_NEAR(-8.0, scale[0], 1e-5f);
    EXPECT_NEAR(20.0, scale[1], 1e-5f);
    EXPECT_NEAR(12.0, scale[2], 1e-5f);
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Using an XYZ rotation order, replicate some rotations that may occur with the Maya rotate tool, and check we end up 
// with the same result. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, rotate_world_space_with_uniform_scale_d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-2.0, 3.0, 1.0));
  parent_rotate.Set(GfVec3d(-11.0, -21.0, 22.0));
  parent_scale.Set(GfVec3d(2,2,2));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(1.731201,0.699451,0.716736,0,-0.608647,1.871529,-0.356271,0,-0.795293,0.090269,1.832856,0,-2,3,1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  child_translate.Set(GfVec3d(2.0, 1.0, 2.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0,0,1,0,0,0,0,1,0,2,1,2,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }

  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  {
    const GfMatrix4d expected(1.731201,0.699451,0.716736,0,-0.608647,1.871529,-0.356271,0,-0.795293,0.090269,1.832856,0,-0.73683,6.450968,5.742912,1);
    COMPARE_MAT4(expected, processor.WorldFrame(), 1e-5f);
  }

  // rotate 15 degrees around X in world space
  {
    GfVec3d rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_TRUE(processor.RotateX(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    child_rotate.Get(&rotate);
    EXPECT_NEAR(13.218885, rotate[0], 1e-5f);
    EXPECT_NEAR(-3.843776, rotate[1], 1e-5f);
    EXPECT_NEAR(-6.439076, rotate[2], 1e-5f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.RotateY(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(16.968949, rotate[0], 1e-5f);
    EXPECT_NEAR(10.668345, rotate[1], 1e-5f);
    EXPECT_NEAR(-5.533085, rotate[2], 1e-5f);
  }

  // rotate 15 degrees around Z in world space
  {
    EXPECT_TRUE(processor.RotateZ(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(22.315417, rotate[0], 1e-5f);
    EXPECT_NEAR(7.835664, rotate[1], 1e-5f);
    EXPECT_NEAR(9.086346, rotate[2], 1e-5f);
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Using an XYZ rotation order, replicate some rotations that may occur with the Maya rotate tool, and check we end up 
// with the same result. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, rotate_world_space_with_uniform_scale_f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-2.0, 3.0, 1.0));
  parent_rotate.Set(GfVec3d(-11.0, -21.0, 22.0));
  parent_scale.Set(GfVec3d(2,2,2));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(1.731201,0.699451,0.716736,0,-0.608647,1.871529,-0.356271,0,-0.795293,0.090269,1.832856,0,-2,3,1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat, TfToken("child_rotate"));
  child_translate.Set(GfVec3d(2.0, 1.0, 2.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0,0,1,0,0,0,0,1,0,2,1,2,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }

  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  {
    const GfMatrix4d expected(1.731201,0.699451,0.716736,0,-0.608647,1.871529,-0.356271,0,-0.795293,0.090269,1.832856,0,-0.73683,6.450968,5.742912,1);
    COMPARE_MAT4(expected, processor.WorldFrame(), 1e-5f);
  }

  // rotate 15 degrees around X in world space
  {
    EXPECT_TRUE(processor.RotateX(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(13.218885, rotate[0], 1e-5f);
    EXPECT_NEAR(-3.843776, rotate[1], 1e-5f);
    EXPECT_NEAR(-6.439076, rotate[2], 1e-5f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.RotateY(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(16.968949, rotate[0], 1e-5f);
    EXPECT_NEAR(10.668345, rotate[1], 1e-5f);
    EXPECT_NEAR(-5.533085, rotate[2], 1e-5f);
  }

  // rotate 15 degrees around Z in world space
  {
    EXPECT_TRUE(processor.RotateZ(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(22.315417, rotate[0], 1e-5f);
    EXPECT_NEAR(7.835664, rotate[1], 1e-5f);
    EXPECT_NEAR(9.086346, rotate[2], 1e-5f);
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Using an XYZ rotation order, replicate some rotations that may occur with the Maya rotate tool, and check we end up 
// with the same result. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, rotate_world_space_with_uniform_scale_h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-2.0, 3.0, 1.0));
  parent_rotate.Set(GfVec3d(-11.0, -21.0, 22.0));
  parent_scale.Set(GfVec3d(2,2,2));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(1.731201,0.699451,0.716736,0,-0.608647,1.871529,-0.356271,0,-0.795293,0.090269,1.832856,0,-2,3,1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionHalf, TfToken("child_rotate"));
  child_translate.Set(GfVec3d(2.0, 1.0, 2.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0,0,1,0,0,0,0,1,0,2,1,2,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }

  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  {
    const GfMatrix4d expected(1.731201,0.699451,0.716736,0,-0.608647,1.871529,-0.356271,0,-0.795293,0.090269,1.832856,0,-0.73683,6.450968,5.742912,1);
    COMPARE_MAT4(expected, processor.WorldFrame(), 1e-5f);
  }

  // rotate 15 degrees around X in world space
  {
    EXPECT_TRUE(processor.RotateX(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(13.218885, rotate[0], 1.4e-3f);
    EXPECT_NEAR(-3.843776, rotate[1], 2.7e-5f);
    EXPECT_NEAR(-6.439076, rotate[2], 1.6e-2f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.RotateY(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(16.968949, rotate[0], 2e-3f);
    EXPECT_NEAR(10.668345, rotate[1], 3.6e-2f);
    EXPECT_NEAR(-5.533085, rotate[2], 2e-2f);
  }

  // rotate 15 degrees around Z in world space
  {
    EXPECT_TRUE(processor.RotateZ(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(22.315417, rotate[0], 3e-2f);
    EXPECT_NEAR(7.835664, rotate[1], 4.2e-2f);
    EXPECT_NEAR(9.086346, rotate[2], 4.1e-2f);
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Using an XYZ rotation order, replicate some rotations that may occur with the Maya rotate tool, and check we end up 
// with the same result. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, rotate_world_space_with_non_uniform_scale_d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-2.0, 3.0, 1.0));
  parent_rotate.Set(GfVec3d(-11.0, -21.0, 22.0));
  parent_scale.Set(GfVec3d(2,3,4));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(1.731201,0.699451,0.716736,0,-0.91297,2.807293,-0.534407,0,-1.590586,0.180538,3.665712,0,-2,3,1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_rotate"));
  child_translate.Set(GfVec3d(2.0, 1.0, 2.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0,0,1,0,0,0,0,1,0,2,1,2,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }

  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  {
    const GfMatrix4d expected(1.731201,0.699451,0.716736,0,-0.91297,2.807293,-0.534407,0,-1.590586,0.180538,3.665712,0,-2.631739,7.567271,9.230489,1);
    COMPARE_MAT4(expected, processor.WorldFrame(), 1e-5f);
  }

  // rotate 15 degrees around X in world space
  {
    GfVec3d rotate(0);
    EXPECT_TRUE(processor.RotateX(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    child_rotate.Get(&rotate);
    EXPECT_NEAR(9.991373, rotate[0], 1e-5f);
    EXPECT_NEAR(-1.930805, rotate[1], 1e-2f);
    EXPECT_NEAR(-4.302774, rotate[2], 1e-5f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.RotateY(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(13.515801, rotate[0], 1e-3f);
    EXPECT_NEAR(5.457887, rotate[1], 0.11);   //< Fairly high difference - shearing effects?
    EXPECT_NEAR(-3.5485, rotate[2], 0.146);   //< Fairly high difference - shearing effects?
  }

  // rotate 15 degrees around Z in world space
  {
    EXPECT_TRUE(processor.RotateZ(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3d rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(17.381871, rotate[0], 1e-3f);
    EXPECT_NEAR(4.011838, rotate[1], 0.088);   //< Fairly high difference - shearing effects?
    EXPECT_NEAR(5.998051, rotate[2], 0.083);   //< Fairly high difference - shearing effects?
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Using an XYZ rotation order, replicate some rotations that may occur with the Maya rotate tool, and check we end up 
// with the same result. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, rotate_world_space_with_non_uniform_scale_f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-2.0, 3.0, 1.0));
  parent_rotate.Set(GfVec3d(-11.0, -21.0, 22.0));
  parent_scale.Set(GfVec3d(2,3,4));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(1.731201,0.699451,0.716736,0,-0.91297,2.807293,-0.534407,0,-1.590586,0.180538,3.665712,0,-2,3,1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat, TfToken("child_rotate"));
  child_translate.Set(GfVec3d(2.0, 1.0, 2.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0,0,1,0,0,0,0,1,0,2,1,2,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }

  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  {
    const GfMatrix4d expected(1.731201,0.699451,0.716736,0,-0.91297,2.807293,-0.534407,0,-1.590586,0.180538,3.665712,0,-2.631739,7.567271,9.230489,1);
    COMPARE_MAT4(expected, processor.WorldFrame(), 1e-5f);
  }

  // rotate 15 degrees around X in world space
  {
    GfVec3f rotate(0);
    EXPECT_TRUE(processor.RotateX(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    child_rotate.Get(&rotate);
    EXPECT_NEAR(9.991373, rotate[0], 1e-5f);
    EXPECT_NEAR(-1.930805, rotate[1], 1e-2f);
    EXPECT_NEAR(-4.302774, rotate[2], 1e-5f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.RotateY(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(13.515801, rotate[0], 1e-3f);
    EXPECT_NEAR(5.457887, rotate[1], 0.11);   //< Fairly high difference - shearing effects?
    EXPECT_NEAR(-3.5485, rotate[2], 0.146);   //< Fairly high difference - shearing effects?
  }

  // rotate 15 degrees around Z in world space
  {
    EXPECT_TRUE(processor.RotateZ(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3f rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(17.381871, rotate[0], 1e-3f);
    EXPECT_NEAR(4.011838, rotate[1], 0.088);   //< Fairly high difference - shearing effects?
    EXPECT_NEAR(5.998051, rotate[2], 0.083);   //< Fairly high difference - shearing effects?
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Using an XYZ rotation order, replicate some rotations that may occur with the Maya rotate tool, and check we end up 
// with the same result. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, rotate_world_space_with_non_uniform_scale_h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_translate = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_translate"));
  UsdGeomXformOp parent_rotate = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_rotate"));
  UsdGeomXformOp parent_scale = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_scale"));

  parent_translate.Set(GfVec3d(-2.0, 3.0, 1.0));
  parent_rotate.Set(GfVec3d(-11.0, -21.0, 22.0));
  parent_scale.Set(GfVec3d(2,3,4));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(1.731201,0.699451,0.716736,0,-0.91297,2.807293,-0.534407,0,-1.590586,0.180538,3.665712,0,-2,3,1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionHalf, TfToken("child_rotate"));
  child_translate.Set(GfVec3d(2.0, 1.0, 2.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0,0,1,0,0,0,0,1,0,2,1,2,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformManipulator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }

  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  {
    const GfMatrix4d expected(1.731201,0.699451,0.716736,0,-0.91297,2.807293,-0.534407,0,-1.590586,0.180538,3.665712,0,-2.631739,7.567271,9.230489,1);
    COMPARE_MAT4(expected, processor.WorldFrame(), 1e-5f);
  }

  // rotate 15 degrees around X in world space
  {
    GfVec3h rotate(0);
    EXPECT_TRUE(processor.RotateX(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    child_rotate.Get(&rotate);
    EXPECT_NEAR(9.991373, rotate[0], 8.2e-3f);
    EXPECT_NEAR(-1.930805, rotate[1], 2e-2f);
    EXPECT_NEAR(-4.302774, rotate[2], 2e-2f);
  }

  // rotate 15 degrees around Y in world space
  {
    EXPECT_TRUE(processor.RotateY(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(13.515801, rotate[0], 1e-3f);
    EXPECT_NEAR(5.457887, rotate[1], 0.12);   //< Fairly high difference - shearing effects?
    EXPECT_NEAR(-3.5485, rotate[2], 0.147);   //< Fairly high difference - shearing effects?
  }

  // rotate 15 degrees around Z in world space
  {
    EXPECT_TRUE(processor.RotateZ(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));
    GfVec3h rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(17.381871, rotate[0], 8.8e-2f);
    EXPECT_NEAR(4.011838, rotate[1], 0.089);   //< Fairly high difference - shearing effects?
    EXPECT_NEAR(5.998051, rotate[2], 0.083);   //< Fairly high difference - shearing effects?
  }
}

//----------------------------------------------------------------------------------------------------------------------
// When using a xform op of type TypeTransform, ensure that when kRotate mode is requested, the correct coordinate
// frame is computed
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, matrix_transform_op_correct_frame_translate)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_transform = parent.AddTransformOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_transform"));
  const GfMatrix4d matrixTransform(4,0,0,0,0,3.535534,3.535534,0,0,-4.242641,4.242641,0,1,2,3,1);
  parent_transform.Set(matrixTransform);

  MayaUsdUtils::TransformManipulator processor(parent.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  const GfMatrix4d expectedFrame(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
  COMPARE_MAT4(expectedFrame, processor.ManipulatorFrame(), 1e-5f);
  COMPARE_MAT4(expectedFrame, processor.WorldFrame(), 1e-5f);

  EXPECT_TRUE(processor.Translate(GfVec3d(2, 3, 4)));
  const GfMatrix4d expectedResult(4,0,0,0,0,3.535534,3.535534,0,0,-4.242641,4.242641,0,3,5,7,1);
  GfMatrix4d evaluated;
  parent_transform.Get(&evaluated);
  COMPARE_MAT4(expectedResult, evaluated, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// When using a xform op of type TypeTransform, ensure that when kRotate mode is requested, the correct coordinate
// frame is computed
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, matrix_transform_op_correct_frame_rotate)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_transform = parent.AddTransformOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_transform"));
  const GfMatrix4d matrixTransform(4,0,0,0,0,3.535534,3.535534,0,0,-4.242641,4.242641,0,1,2,3,1);
  parent_transform.Set(matrixTransform);

  MayaUsdUtils::TransformManipulator processor(parent.GetPrim(), 0, MayaUsdUtils::TransformManipulator::kRotate);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());

  const GfMatrix4d expectedFrame(1,0,0,0, 0,1,0,0, 0,0,1,0, 1,2,3,1);
  COMPARE_MAT4(expectedFrame, processor.ManipulatorFrame(), 1e-5f);
  COMPARE_MAT4(expectedFrame, processor.WorldFrame(), 1e-5f);

  EXPECT_TRUE(processor.RotateX(45.0 * M_PI / 180.0));
  const GfMatrix4d expectedResult(4,0,0,0,0,0,5,0,0,-6,0,0,1,2,3,1);
  GfMatrix4d evaluated;
  parent_transform.Get(&evaluated);
  COMPARE_MAT4(expectedResult, evaluated, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// When using a xform op of type TypeTransform, ensure that when kRotate mode is requested, the correct coordinate
// frame is computed
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, matrix_transform_op_correct_frame_rotate2)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_transform = parent.AddTransformOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_transform"));
  const GfMatrix4d matrixTransform(4,0,0,0,0,3.535534,3.535534,0,0,-4.242641,4.242641,0,1,2,3,1);
  parent_transform.Set(matrixTransform);

  MayaUsdUtils::TransformManipulator processor(parent.GetPrim(), 0, MayaUsdUtils::TransformManipulator::kRotate);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());

  const GfMatrix4d expectedFrame(1,0,0,0, 0,1,0,0, 0,0,1,0, 1,2,3,1);
  COMPARE_MAT4(expectedFrame, processor.ManipulatorFrame(), 1e-5f);
  COMPARE_MAT4(expectedFrame, processor.WorldFrame(), 1e-5f);

  EXPECT_TRUE(processor.RotateY(15.0 * M_PI / 180.0));
  const GfMatrix4d expectedResult(3.863703,0,-1.035276,0,0.915064,3.535534,3.415064,0,1.098076,-4.242641,4.098076,0,1,2,3,1);
  GfMatrix4d evaluated;
  parent_transform.Get(&evaluated);
  COMPARE_MAT4(expectedResult, evaluated, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// When using a xform op of type TypeTransform, ensure that when kScale mode is requested, the correct coordinate
// frame is computed
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, matrix_transform_op_correct_frame_scale)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp parent_transform = parent.AddTransformOp(UsdGeomXformOp::PrecisionDouble, TfToken("parent_transform"));
  const GfMatrix4d matrixTransform(4,0,0,0,0,3.535534,3.535534,0,0,-4.242641,4.242641,0,1,2,3,1);
  parent_transform.Set(matrixTransform);

  MayaUsdUtils::TransformManipulator processor(parent.GetPrim(), 0, MayaUsdUtils::TransformManipulator::kScale);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());

  const GfMatrix4d expectedFrame(1,0,0,0,0,0.707107,0.707107,0,0,-0.707107,0.707107,0, 1,2,3,1);
  COMPARE_MAT4(expectedFrame, processor.ManipulatorFrame(), 1e-5f);
  COMPARE_MAT4(expectedFrame, processor.WorldFrame(), 1e-5f);

  EXPECT_TRUE(processor.Scale(GfVec3d(2, 1, 3)));
  const GfMatrix4d expectedResult(8,0,0,0,0,3.535534,3.535534,0,0,-12.727922,12.727922,0,1,2,3,1);
  GfMatrix4d evaluated;
  parent_transform.Get(&evaluated);
  COMPARE_MAT4(expectedResult, evaluated, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that we can rotate a matrix op in world space
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, matrix_transform_op_world_rotate)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble).Set(GfVec3d(1, 2, 3));
  parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble).Set(GfVec3d(15, 30, 45));

  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_transform = child.AddTransformOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_transform"));

  const GfMatrix4d matrixTransform(4,0,0,0,0,3.535534,3.535534,0,0,-4.242641,4.242641,0,1,2,3,1);
  child_transform.Set(matrixTransform);

  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0, MayaUsdUtils::TransformManipulator::kRotate);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());

  EXPECT_TRUE(processor.RotateX(15.0 * M_PI / 180.0, MayaUsdUtils::TransformManipulator::kWorld));

  const GfMatrix4d expectedResult(3.914815,0.493652,0.656151,0,-1.026176,2.859477,3.97119,0,0.0252416,-4.86594,3.51027,0,1,2,3,1);
  GfMatrix4d evaluated;
  child_transform.Get(&evaluated);
  COMPARE_MAT4(expectedResult, evaluated, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that we can rotate a matrix op in parent space
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, matrix_transform_op_parent_rotate)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble).Set(GfVec3d(1, 2, 3));
  parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble).Set(GfVec3d(15, 30, 45));
  UsdGeomXformOp child_transform = parent.AddTransformOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_transform"));

  const GfMatrix4d matrixTransform(4,0,0,0,0,3.535534,3.535534,0,0,-4.242641,4.242641,0,1,2,3,1);
  child_transform.Set(matrixTransform);

  MayaUsdUtils::TransformManipulator processor(parent.GetPrim(), 2, MayaUsdUtils::TransformManipulator::kRotate);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());

  EXPECT_TRUE(processor.RotateX(15.0 * M_PI / 180.0, MayaUsdUtils::TransformManipulator::kParent));

  const GfMatrix4d expectedResult(3.914815,0.493652,0.656151,0,-1.026176,2.859477,3.97119,0,0.0252416,-4.86594,3.51027,0,1,2,3,1);
  GfMatrix4d evaluated;
  child_transform.Get(&evaluated);
  COMPARE_MAT4(expectedResult, evaluated, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that we can rotate a matrix op in world space
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, matrix_transform_op_world_translate)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble).Set(GfVec3d(1, 2, 3));
  parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble).Set(GfVec3d(15, 30, 45));

  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_transform = child.AddTransformOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_transform"));

  const GfMatrix4d matrixTransform(4,0,0,0,0,3.535534,3.535534,0,0,-4.242641,4.242641,0,1,2,3,1);
  child_transform.Set(matrixTransform);

  MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0, MayaUsdUtils::TransformManipulator::kTranslate);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

  EXPECT_TRUE(processor.Translate(GfVec3d(-2.0, 3.0, 1.0), MayaUsdUtils::TransformManipulator::kWorld));

  const GfMatrix4d expectedResult(4,0,0,0,0,3.535534,3.535534,0,0,-4.242641,4.242641,0,1.112372,5.730714,3.262959,1);
  GfMatrix4d evaluated;
  child_transform.Get(&evaluated);
  COMPARE_MAT4(expectedResult, evaluated, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that we can rotate a matrix op in parent space
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, matrix_transform_op_parent_translate)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble).Set(GfVec3d(1, 2, 3));
  parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble).Set(GfVec3d(15, 30, 45));
  UsdGeomXformOp child_transform = parent.AddTransformOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_transform"));

  const GfMatrix4d matrixTransform(4,0,0,0,0,3.535534,3.535534,0,0,-4.242641,4.242641,0,1,2,3,1);
  child_transform.Set(matrixTransform);

  MayaUsdUtils::TransformManipulator processor(parent.GetPrim(), 2, MayaUsdUtils::TransformManipulator::kRotate);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());

  EXPECT_TRUE(processor.Translate(GfVec3d(-2.0, 3.0, 1.0), MayaUsdUtils::TransformManipulator::kParent));

  const GfMatrix4d expectedResult(4,0,0,0,0,3.535534,3.535534,0,0,-4.242641,4.242641,0,1.112372,5.730714,3.262959,1);
  GfMatrix4d evaluated;
  child_transform.Get(&evaluated);
  COMPARE_MAT4(expectedResult, evaluated, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Simplest negative scale case. This shouldn't need to apply any special negative scale handling, so it should just work
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, negative_scale_and_translate_local)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  // initialise a parent transform
  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp T = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("T"));
  UsdGeomXformOp R = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("R"));
  UsdGeomXformOp S = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("S"));
  T.Set(GfVec3d(2.0, 3.0, 1.0));
  R.Set(GfVec3d(15.0, 30.0, 45.0));
  S.Set(GfVec3d(-2.2, 3.3, 1.1));

  // make a small translation modification
  {
    MayaUsdUtils::TransformManipulator processor(parent.GetPrim(), 0, MayaUsdUtils::TransformManipulator::kTranslate);
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());
    EXPECT_TRUE(processor.Translate(GfVec3d(-1, -2, -4), MayaUsdUtils::TransformManipulator::kTransform));

    GfVec3d result;
    T.Get(&result);

    EXPECT_NEAR(2.0 - 1.0, result[0], 1e-5f);
    EXPECT_NEAR(3.0 - 2.0, result[1], 1e-5f);
    EXPECT_NEAR(1.0 - 4.0, result[2], 1e-5f);
  }

  // make a small rotation modification
  {
    MayaUsdUtils::TransformManipulator processor(parent.GetPrim(), 1, MayaUsdUtils::TransformManipulator::kRotate);
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
    EXPECT_TRUE(processor.RotateX(45.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kTransform));

    GfVec3d result;
    R.Get(&result);

    EXPECT_NEAR(60.0, result[0], 1e-5f);
    EXPECT_NEAR(30.0, result[1], 1e-5f);
    EXPECT_NEAR(45.0, result[2], 1e-5f);
  }

  // make a small scale modification
  {
    MayaUsdUtils::TransformManipulator processor(parent.GetPrim(), 2, MayaUsdUtils::TransformManipulator::kScale);
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());
    EXPECT_TRUE(processor.Scale(GfVec3d(10.0, 20.0, 30.0), MayaUsdUtils::TransformManipulator::kTransform));

    GfVec3d result;
    S.Get(&result);

    EXPECT_NEAR(-22.0, result[0], 1e-5f);
    EXPECT_NEAR(66.0, result[1], 1e-5f);
    EXPECT_NEAR(33.0, result[2], 1e-5f);
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Simplest negative scale case. This shouldn't need to apply any special negative scale handling. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, uniform_negative_scale_and_transform_world)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp T = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("T"));
  UsdGeomXformOp R = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("R"));
  UsdGeomXformOp S = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("S"));
  T.Set(GfVec3d(0.0, 0.0, 0.0));
  R.Set(GfVec3d(0.0, 0.0, 0.0));
  S.Set(GfVec3d(-2.2, -2.2, -2.2));

  // initialise a parent transform
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp Tc = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("T"));
  UsdGeomXformOp Rc = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("R"));
  UsdGeomXformOp Sc = child.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("S"));
  Tc.Set(GfVec3d(2.0, 3.0, 1.0));
  Rc.Set(GfVec3d(15.0, 30.0, 45.0));
  Sc.Set(GfVec3d(2.2, -3.3, 1.1));

  // make a small translation modification
  {
    MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0, MayaUsdUtils::TransformManipulator::kTranslate);
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());
    EXPECT_TRUE(processor.Translate(GfVec3d(-1, -2, -4), MayaUsdUtils::TransformManipulator::kWorld));

    GfVec3d result;
    Tc.Get(&result);

    EXPECT_NEAR(2.454545, result[0], 1e-5f);
    EXPECT_NEAR(3.909091, result[1], 1e-5f);
    EXPECT_NEAR(2.818182, result[2], 1e-5f);
  }

  // make a small rotation modification
  {
    MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1, MayaUsdUtils::TransformManipulator::kRotate);
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
    EXPECT_TRUE(processor.RotateX(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));

    GfVec3d result;
    Rc.Get(&result);

    EXPECT_NEAR(26.155986, result[0], 1e-5f);
    EXPECT_NEAR(18.933424, result[1], 1e-5f);
    EXPECT_NEAR(49.654204, result[2], 1e-5f);
  }

  // make a small scale modification
  {
    MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 2, MayaUsdUtils::TransformManipulator::kScale);
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());
    EXPECT_TRUE(processor.Scale(GfVec3d(-3, -3, -3), MayaUsdUtils::TransformManipulator::kWorld));

    GfVec3d result;
    Sc.Get(&result);

    EXPECT_NEAR(2.2 * -3.0, result[0], 1e-5f);
    EXPECT_NEAR(-3.3 * -3.0, result[1], 1e-5f);
    EXPECT_NEAR(1.1 * -3.0, result[2], 1e-5f);
  }
}

#if 0
// 
// I've disabled these tests for now. 
// 
// I'm a little unsure how Maya is handling the case where the parent matrix has a negative non-uniform scale in 1 or 3 axes, 
// and we're applying a world space rotation to the child transform. 
// 
// The results are always going to be *wrong* when you do this (since we have to account for shear), so in that regard, 
// my approach is no less wrong than Maya's, however it would be nice to make this final edge case match the result of Maya :(
//

//----------------------------------------------------------------------------------------------------------------------
// Simplest negative scale case. This shouldn't need to apply any special negative scale handling. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, negative_non_uniform_scale_and_translate_world1)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp T = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("T"));
  UsdGeomXformOp R = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("R"));
  UsdGeomXformOp S = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("S"));
  T.Set(GfVec3d(0.0, 0.0, 0.0));
  R.Set(GfVec3d(0.0, 0.0, 0.0));
  S.Set(GfVec3d(-2.2, 3.3, 1.1));

  // initialise a parent transform
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp Tc = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("T"));
  UsdGeomXformOp Rc = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("R"));
  UsdGeomXformOp Sc = child.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("S"));
  Tc.Set(GfVec3d(2.0, 3.0, 1.0));
  Rc.Set(GfVec3d(15.0, 30.0, 45.0));
  Sc.Set(GfVec3d(2.2, -3.3, 1.1));

  // make a small translation modification
  {
    MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0, MayaUsdUtils::TransformManipulator::kTranslate);
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());
    EXPECT_TRUE(processor.Translate(GfVec3d(-1, -2, -4), MayaUsdUtils::TransformManipulator::kWorld));

    GfVec3d result;
    Tc.Get(&result);

    EXPECT_NEAR(2.454545, result[0], 1e-5f);
    EXPECT_NEAR(2.393939, result[1], 1e-5f);
    EXPECT_NEAR(-2.636364, result[2], 1e-5f);
  }

  // make a small rotation modification
  {
    MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1, MayaUsdUtils::TransformManipulator::kRotate);
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
    EXPECT_TRUE(processor.RotateX(45.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));

    GfVec3d result;
    Rc.Get(&result);

    EXPECT_NEAR(62.752755, result[0], 1e-5f);
    EXPECT_NEAR(-25.013615, result[1], 1e-5f);
    EXPECT_NEAR(47.487338, result[2], 1e-5f);
  }

  // make a small scale modification
  {
    MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 2, MayaUsdUtils::TransformManipulator::kScale);
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());
    EXPECT_TRUE(processor.Scale(GfVec3d(-3, -3, -3), MayaUsdUtils::TransformManipulator::kWorld));

    GfVec3d result;
    Sc.Get(&result);

    EXPECT_NEAR(2.2 * -3.0, result[0], 1e-5f);
    EXPECT_NEAR(-3.3 * -3.0, result[1], 1e-5f);
    EXPECT_NEAR(1.1 * -3.0, result[2], 1e-5f);
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Simplest negative scale case. This shouldn't need to apply any special negative scale handling. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, negative_non_uniform_scale_and_translate_world2)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);

  UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
  UsdGeomXformOp T = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("T"));
  UsdGeomXformOp R = parent.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("R"));
  UsdGeomXformOp S = parent.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("S"));
  T.Set(GfVec3d(2.0, 3.0, 1.0));
  R.Set(GfVec3d(15.0, 30.0, 45.0));
  S.Set(GfVec3d(-2.2, 3.3, 1.1));

  // initialise a parent transform
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp Tc = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("T"));
  UsdGeomXformOp Rc = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("R"));
  UsdGeomXformOp Sc = child.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("S"));
  Tc.Set(GfVec3d(2.0, 3.0, 1.0));
  Rc.Set(GfVec3d(15.0, 30.0, 45.0));
  Sc.Set(GfVec3d(2.2, -3.3, 1.1));

  // make a small translation modification
  {
    MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 0, MayaUsdUtils::TransformManipulator::kTranslate);
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());
    EXPECT_TRUE(processor.Translate(GfVec3d(-1, -2, -4), MayaUsdUtils::TransformManipulator::kWorld));

    GfVec3d result;
    Tc.Get(&result);

    EXPECT_NEAR(1.925962, result[0], 1e-5f);
    EXPECT_NEAR(2.438149, result[1], 1e-5f);
    EXPECT_NEAR(-2.806883, result[2], 1e-5f);
  }

  // make a small rotation modification
  {
    MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 1, MayaUsdUtils::TransformManipulator::kRotate);
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
    EXPECT_TRUE(processor.RotateX(45.0 * (M_PI / 180.0), MayaUsdUtils::TransformManipulator::kWorld));

    GfVec3d result;
    Rc.Get(&result);

    EXPECT_NEAR(90.312074, result[0], 1e-5f);
    EXPECT_NEAR(21.092268, result[1], 1e-5f);
    EXPECT_NEAR(43.367621, result[2], 1e-5f);
  }

  // make a small scale modification
  {
    MayaUsdUtils::TransformManipulator processor(child.GetPrim(), 2, MayaUsdUtils::TransformManipulator::kScale);
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());
    EXPECT_TRUE(processor.Scale(GfVec3d(-3, -3, -3), MayaUsdUtils::TransformManipulator::kWorld));

    GfVec3d result;
    Sc.Get(&result);

    EXPECT_NEAR(2.2 * -3.0, result[0], 1e-5f);
    EXPECT_NEAR(-3.3 * -3.0, result[1], 1e-5f);
    EXPECT_NEAR(1.1 * -3.0, result[2], 1e-5f);
  }
}
#endif

//----------------------------------------------------------------------------------------------------------------------
// Make sure we can apply an object space translation using double precision
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, os_translated)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
  UsdGeomXformOp third = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("third"));

  first.Set(GfVec3d(15, 30, 45));
  second.Set(GfVec3d(2, 2, 2));
  third.Set(GfVec3d(12, 24, 36));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());
  
  // 
  EXPECT_TRUE(processor.Translate(GfVec3d(3, 4, 5), MayaUsdUtils::TransformManipulator::kPostTransform));

  GfVec3d translate;
  second.Get(&translate);
  EXPECT_NEAR(4.411486, translate[0], 1e-5f);
  EXPECT_NEAR(7.30331, translate[1], 1e-5f);
  EXPECT_NEAR(6.007449, translate[2], 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Make sure we can apply an object space translation using float precision
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, os_translatef)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("second"));
  UsdGeomXformOp third = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat, TfToken("third"));

  first.Set(GfVec3f(15, 30, 45));
  second.Set(GfVec3f(2, 2, 2));
  third.Set(GfVec3f(12, 24, 36));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());
  
  // 
  EXPECT_TRUE(processor.Translate(GfVec3d(3, 4, 5), MayaUsdUtils::TransformManipulator::kPostTransform));

  GfVec3f translate;
  second.Get(&translate);
  EXPECT_NEAR(4.411486, translate[0], 1e-5f);
  EXPECT_NEAR(7.30331, translate[1], 1e-5f);
  EXPECT_NEAR(6.007449, translate[2], 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Make sure we can apply an object space translation using half precision
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, os_translateh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("second"));
  UsdGeomXformOp third = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionHalf, TfToken("third"));

  first.Set(GfVec3h(15, 30, 45));
  second.Set(GfVec3h(2, 2, 2));
  third.Set(GfVec3h(12, 24, 36));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 1);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());
  
  // 
  EXPECT_TRUE(processor.Translate(GfVec3d(3, 4, 5), MayaUsdUtils::TransformManipulator::kPostTransform));

  GfVec3h translate;
  second.Get(&translate);
  EXPECT_NEAR(4.411486, translate[0], 2e-3f);
  EXPECT_NEAR(7.30331, translate[1], 2e-3f);
  EXPECT_NEAR(6.007449, translate[2], 1e-3f);
}

//----------------------------------------------------------------------------------------------------------------------
// Make sure we can apply an object space rotations using float precision. Proof:
//
// $loc1 = `spaceLocator`;
// $loc2 = `spaceLocator`;
// $loc3 = `spaceLocator`;
// $loc4 = `spaceLocator`;
// 
// parent $loc2[0] $loc1[0];
// parent $loc3[0] $loc2[0];
// 
// setAttr ($loc2[0] + ".r") 12 24 36;
// 
// select -r $loc3[0];
// select -add $loc4[0];
// parentConstraint -mo -weight 1;
// 
// setAttr ($loc3[0] + ".r") 45 0 0;
// getAttr ($loc4[0] + ".r"); // 31.258249 27.887712 -11.179626
// 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, os_rotated)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  UsdGeomXformOp third = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("third"));

  third.Set(GfVec3d(12, 24, 36));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  
  double angle = 45.0 * (M_PI / 360.0);
  double sa = std::sin(angle);
  double ca = std::cos(angle);
  GfQuatd Q(ca, sa, 0, 0);

  // 
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kPostTransform));

  GfVec3d R;
  first.Get(&R);
  EXPECT_NEAR(31.258249, R[0], 1e-5f);
  EXPECT_NEAR(27.887712, R[1], 1e-5f);
  EXPECT_NEAR(-11.179626, R[2], 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Make sure we can apply an object space rotations using float precision
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, os_rotatef)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  UsdGeomXformOp third = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat, TfToken("third"));

  //first.Set(GfVec3f(15, 30, 45));
  third.Set(GfVec3f(12, 24, 36));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  
  double angle = 45.0 * (M_PI / 360.0);
  double sa = std::sin(angle);
  double ca = std::cos(angle);
  GfQuatd Q(ca, sa, 0, 0);

  // 
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kPostTransform));

  GfVec3f R;
  first.Get(&R);
  EXPECT_NEAR(31.258249, R[0], 1e-5f);
  EXPECT_NEAR(27.887712, R[1], 1e-5f);
  EXPECT_NEAR(-11.179626, R[2], 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Make sure we can apply an object space rotations using float precision
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, os_rotateh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  UsdGeomXformOp third = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionHalf, TfToken("third"));

  //first.Set(GfVec3f(15, 30, 45));
  third.Set(GfVec3h(12, 24, 36));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  
  double angle = 45.0 * (M_PI / 360.0);
  double sa = std::sin(angle);
  double ca = std::cos(angle);
  GfQuatd Q(ca, sa, 0, 0);

  // 
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kPostTransform));

  GfVec3h R;
  first.Get(&R);
  EXPECT_NEAR(31.258249, R[0], 1e-2f);
  EXPECT_NEAR(27.887712, R[1], 1e-2f);
  EXPECT_NEAR(-11.179626, R[2], 1e-2f);
}

//----------------------------------------------------------------------------------------------------------------------
// Make sure we can apply an object space rotations using double precision. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, os_rotate2d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  UsdGeomXformOp third = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("third"));

  first.Set(GfVec3d(10, 10, 10));
  third.Set(GfVec3d(12, 24, 36));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  
  double angle = 45.0 * (M_PI / 360.0);
  double sa = std::sin(angle);
  double ca = std::cos(angle);
  GfQuatd Q(ca, sa, 0, 0);

  // 
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kPostTransform));

  GfVec3d R;
  first.Get(&R);
  EXPECT_NEAR(41.238679, R[0], 1e-5f);
  EXPECT_NEAR(39.309652, R[1], 1e-5f);
  EXPECT_NEAR(3.50343, R[2], 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Make sure we can apply an object space rotations using float precision. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, os_rotate2f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  UsdGeomXformOp third = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat, TfToken("third"));

  first.Set(GfVec3f(10, 10, 10));
  third.Set(GfVec3f(12, 24, 36));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  
  double angle = 45.0 * (M_PI / 360.0);
  double sa = std::sin(angle);
  double ca = std::cos(angle);
  GfQuatd Q(ca, sa, 0, 0);

  // 
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kPostTransform));

  GfVec3f R;
  first.Get(&R);
  EXPECT_NEAR(41.238679, R[0], 1e-5f);
  EXPECT_NEAR(39.309652, R[1], 1e-5f);
  EXPECT_NEAR(3.50343, R[2], 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Make sure we can apply an object space rotations using float precision. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, os_rotate2h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  UsdGeomXformOp third = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionHalf, TfToken("third"));

  first.Set(GfVec3h(10, 10, 10));
  third.Set(GfVec3h(12, 24, 36));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
  EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());
  
  double angle = 45.0 * (M_PI / 360.0);
  double sa = std::sin(angle);
  double ca = std::cos(angle);
  GfQuatd Q(ca, sa, 0, 0);

  // 
  EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kPostTransform));

  GfVec3h R;
  first.Get(&R);
  EXPECT_NEAR(41.238679, R[0], 1.2e-1f);
  EXPECT_NEAR(39.309652, R[1], 1e-2f);
  EXPECT_NEAR(3.50343, R[2], 1e-2f);
}

//----------------------------------------------------------------------------------------------------------------------
// Testing the edge cases when scale is zero (hoping to avoid INFs/NANs)
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulator, zero_scale)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp T = xform.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("T"));
  UsdGeomXformOp R = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat, TfToken("R"));
  UsdGeomXformOp S = xform.AddScaleOp(UsdGeomXformOp::PrecisionFloat, TfToken("S"));

  T.Set(GfVec3f(0, 0, 0));
  R.Set(GfVec3f(0, 0, 0));
  S.Set(GfVec3f(0, 0, 0));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  {
    MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 0);
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());
    
    // 
    EXPECT_TRUE(processor.Translate(GfVec3d(2, 2, 2), MayaUsdUtils::TransformManipulator::kPostTransform));

    GfVec3f V;
    T.Get(&V);
    EXPECT_NEAR(0, V[0], 1e-5f);
    EXPECT_NEAR(0, V[1], 1e-5f);
    EXPECT_NEAR(0, V[2], 1e-5f);
    R.Get(&V);
    EXPECT_NEAR(0, V[0], 1e-5f);
    EXPECT_NEAR(0, V[1], 1e-5f);
    EXPECT_NEAR(0, V[2], 1e-5f);
    S.Get(&V);
    EXPECT_NEAR(0, V[0], 1e-5f);
    EXPECT_NEAR(0, V[1], 1e-5f);
    EXPECT_NEAR(0, V[2], 1e-5f);
  }

  // The behaviour differs from Maya slightly here. 
  // 
  // If you call: rotate -r -os 45 0 0
  //
  // Maya will set the rotation to [45, 0, 0]. I set it to [0, 0, 0]
  {
    MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 1);
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());

    double angle = 45.0 * (M_PI / 360.0);
    double sa = std::sin(angle);
    double ca = std::cos(angle);
    GfQuatd Q(ca, sa, 0, 0);

    // 
    EXPECT_TRUE(processor.Rotate(Q, MayaUsdUtils::TransformManipulator::kPostTransform));
    GfVec3f V;
    T.Get(&V);
    EXPECT_NEAR(0, V[0], 1e-5f);
    EXPECT_NEAR(0, V[1], 1e-5f);
    EXPECT_NEAR(0, V[2], 1e-5f);
    R.Get(&V);
    EXPECT_NEAR(0, V[0], 1e-5f);
    EXPECT_NEAR(0, V[1], 1e-5f);
    EXPECT_NEAR(0, V[2], 1e-5f);
    S.Get(&V);
    EXPECT_NEAR(0, V[0], 1e-5f);
    EXPECT_NEAR(0, V[1], 1e-5f);
    EXPECT_NEAR(0, V[2], 1e-5f);
  }

  {
    MayaUsdUtils::TransformManipulator processor(xform.GetPrim(), 2);
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());

    // 
    EXPECT_TRUE(processor.Scale(GfVec3d(1, 1, 1), MayaUsdUtils::TransformManipulator::kPostTransform));
    GfVec3f V;
    T.Get(&V);
    EXPECT_NEAR(0, V[0], 1e-5f);
    EXPECT_NEAR(0, V[1], 1e-5f);
    EXPECT_NEAR(0, V[2], 1e-5f);
    R.Get(&V);
    EXPECT_NEAR(0, V[0], 1e-5f);
    EXPECT_NEAR(0, V[1], 1e-5f);
    EXPECT_NEAR(0, V[2], 1e-5f);
    S.Get(&V);
    EXPECT_NEAR(0, V[0], 1e-5f);
    EXPECT_NEAR(0, V[1], 1e-5f);
    EXPECT_NEAR(0, V[2], 1e-5f);
  }
}
