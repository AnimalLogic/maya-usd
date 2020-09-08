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
//  bool SetScale(const GfVec3d& scale, Space space = kTransform);
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulatorEx, SetScale)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);

    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));
    UsdGeomXformOp first = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(2.0, 2.0, 2.0));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

    MayaUsdUtils::TransformManipulatorEx processor(xform.GetPrim(), first.GetOpName());
    EXPECT_TRUE(processor.SetScale(GfVec3d(3.0, 4.0, 5.0), MayaUsdUtils::TransformManipulator::kTransform));
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());

    auto S = processor.Scale();
    GfVec3d s;
    first.Get(&s);

    EXPECT_NEAR(3.0, S[0], 1e-5f);
    EXPECT_NEAR(4.0, S[1], 1e-5f);
    EXPECT_NEAR(5.0, S[2], 1e-5f);
    EXPECT_NEAR(3.0, s[0], 1e-5f);
    EXPECT_NEAR(4.0, s[1], 1e-5f);
    EXPECT_NEAR(5.0, s[2], 1e-5f);
  }
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);

    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));
    UsdGeomXformOp first = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(2.0, 2.0, 2.0));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

    MayaUsdUtils::TransformManipulatorEx processor(xform.GetPrim(), first.GetOpName());
    EXPECT_TRUE(processor.SetScale(GfVec3d(3.0, 4.0, 5.0), MayaUsdUtils::TransformManipulator::kObject));
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());

    auto S = processor.Scale();
    GfVec3d s;
    first.Get(&s);

    EXPECT_NEAR(3.0, S[0], 1e-5f);
    EXPECT_NEAR(4.0, S[1], 1e-5f);
    EXPECT_NEAR(5.0, S[2], 1e-5f);
    EXPECT_NEAR(3.0, s[0], 1e-5f);
    EXPECT_NEAR(4.0, s[1], 1e-5f);
    EXPECT_NEAR(5.0, s[2], 1e-5f);
  }
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);

    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));
    UsdGeomXformOp first = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(2.0, 2.0, 2.0));

    UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
    UsdGeomXformOp cscale = child.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("cscale"));
    cscale.Set(GfVec3d(3.0, 4.0, 5.0));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);

    MayaUsdUtils::TransformManipulatorEx processor(child.GetPrim(), cscale.GetOpName());
    EXPECT_TRUE(processor.SetScale(GfVec3d(6.0, 6.0, 6.0), MayaUsdUtils::TransformManipulator::kWorld));
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());

    auto S = processor.Scale();
    GfVec3d s;
    cscale.Get(&s);

    // Now I'd expect this to be [3, 3, 3], however Maya ignores are parent scaling, and just sets the value
    // directly in object space. Basically kWorld, kParent, kObject have no effect.
    EXPECT_NEAR(6.0, S[0], 1e-5f);
    EXPECT_NEAR(6.0, S[1], 1e-5f);
    EXPECT_NEAR(6.0, S[2], 1e-5f);
    EXPECT_NEAR(6.0, s[0], 1e-5f);
    EXPECT_NEAR(6.0, s[1], 1e-5f);
    EXPECT_NEAR(6.0, s[2], 1e-5f);
  }
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);

    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));
    UsdGeomXformOp first = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(2.0, 2.0, 2.0));

    UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
    UsdGeomXformOp cscale = child.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("cscale"));
    cscale.Set(GfVec3d(3.0, 4.0, 5.0));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);

    MayaUsdUtils::TransformManipulatorEx processor(child.GetPrim(), cscale.GetOpName());
    EXPECT_TRUE(processor.SetScale(GfVec3d(6.0, 6.0, 6.0), MayaUsdUtils::TransformManipulator::kParent));
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kScale, processor.ManipMode());

    auto S = processor.Scale();
    GfVec3d s;
    cscale.Get(&s);

    EXPECT_NEAR(6.0, S[0], 1e-5f);
    EXPECT_NEAR(6.0, S[1], 1e-5f);
    EXPECT_NEAR(6.0, S[2], 1e-5f);
    EXPECT_NEAR(6.0, s[0], 1e-5f);
    EXPECT_NEAR(6.0, s[1], 1e-5f);
    EXPECT_NEAR(6.0, s[2], 1e-5f);
  }
}

//----------------------------------------------------------------------------------------------------------------------
//  bool SetScale(const GfVec3d& scale, Space space = kTransform);
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulatorEx, SetTranslate)
{
  // make sure simple translation set up works (transform-space translation)
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);

    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));
    UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(2.0, 2.0, 2.0));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

    MayaUsdUtils::TransformManipulatorEx processor(xform.GetPrim(), first.GetOpName());
    EXPECT_TRUE(processor.SetTranslate(GfVec3d(3.0, 4.0, 5.0), MayaUsdUtils::TransformManipulator::kTransform));
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

    auto S = processor.Translation();
    GfVec3d s;
    first.Get(&s);

    EXPECT_NEAR(3.0, S[0], 1e-5f);
    EXPECT_NEAR(4.0, S[1], 1e-5f);
    EXPECT_NEAR(5.0, S[2], 1e-5f);
    EXPECT_NEAR(3.0, s[0], 1e-5f);
    EXPECT_NEAR(4.0, s[1], 1e-5f);
    EXPECT_NEAR(5.0, s[2], 1e-5f);
  }
  // make sure simple translation set up works (parent-space translation)
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);

    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));
    UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(2.0, 2.0, 2.0));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

    MayaUsdUtils::TransformManipulatorEx processor(xform.GetPrim(), first.GetOpName());
    EXPECT_TRUE(processor.SetTranslate(GfVec3d(3.0, 4.0, 5.0), MayaUsdUtils::TransformManipulator::kParent));
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

    auto S = processor.Translation();
    GfVec3d s;
    first.Get(&s);

    EXPECT_NEAR(3.0, S[0], 1e-5f);
    EXPECT_NEAR(4.0, S[1], 1e-5f);
    EXPECT_NEAR(5.0, S[2], 1e-5f);
    EXPECT_NEAR(3.0, s[0], 1e-5f);
    EXPECT_NEAR(4.0, s[1], 1e-5f);
    EXPECT_NEAR(5.0, s[2], 1e-5f);
  }
  // make sure simple translation set up works (parent-space translation with parent)
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);

    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));
    UsdGeomXformOp rotate = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("rot"));
    rotate.Set(GfVec3d(15.0, 30.0, 45.0));
    UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(2.0, 2.0, 2.0));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

    MayaUsdUtils::TransformManipulatorEx processor(xform.GetPrim(), first.GetOpName());
    EXPECT_TRUE(processor.SetTranslate(GfVec3d(3.0, 4.0, 5.0), MayaUsdUtils::TransformManipulator::kParent));
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

    auto S = processor.Translation();
    GfVec3d s;
    first.Get(&s);

    EXPECT_NEAR(1.786607, S[0], 1e-5f);
    EXPECT_NEAR(2.444276, S[1], 1e-5f);
    EXPECT_NEAR(6.390113, S[2], 1e-5f);
    EXPECT_NEAR(1.786607, s[0], 1e-5f);
    EXPECT_NEAR(2.444276, s[1], 1e-5f);
    EXPECT_NEAR(6.390113, s[2], 1e-5f);
  }
  // make sure parent transform is included in world space computation
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);

    UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
    UsdGeomXformOp pt = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("t"));
    pt.Set(GfVec3d(1.0, 1.0, 1.0));
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
    UsdGeomXformOp rotate = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("rot"));
    rotate.Set(GfVec3d(15.0, 30.0, 45.0));
    UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(2.0, 2.0, 2.0));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

    MayaUsdUtils::TransformManipulatorEx processor(xform.GetPrim(), first.GetOpName());
    EXPECT_TRUE(processor.SetTranslate(GfVec3d(3.0, 4.0, 5.0), MayaUsdUtils::TransformManipulator::kWorld));
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

    auto S = processor.Translation();
    GfVec3d s;
    first.Get(&s);
    
    EXPECT_NEAR(1.061862, S[0], 1e-5f);
    EXPECT_NEAR(2.03712, S[1], 1e-5f);
    EXPECT_NEAR(4.870584, S[2], 1e-5f);
    EXPECT_NEAR(1.061862, s[0], 1e-5f);
    EXPECT_NEAR(2.03712, s[1], 1e-5f);
    EXPECT_NEAR(4.870584, s[2], 1e-5f);
  }
  // make sure parent transform is not included in parent space computation
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);

    UsdGeomXform parent = UsdGeomXform::Define(stage, SdfPath("/xform"));
    UsdGeomXformOp pt = parent.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("t"));
    pt.Set(GfVec3d(1.0, 1.0, 1.0));
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
    UsdGeomXformOp rotate = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("rot"));
    rotate.Set(GfVec3d(15.0, 30.0, 45.0));
    UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(2.0, 2.0, 2.0));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

    MayaUsdUtils::TransformManipulatorEx processor(xform.GetPrim(), first.GetOpName());
    EXPECT_TRUE(processor.SetTranslate(GfVec3d(3.0, 4.0, 5.0), MayaUsdUtils::TransformManipulator::kParent));
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

    auto S = processor.Translation();
    GfVec3d s;
    first.Get(&s);
    
    EXPECT_NEAR(1.786607, S[0], 1e-5f);
    EXPECT_NEAR(2.444276, S[1], 1e-5f);
    EXPECT_NEAR(6.390113, S[2], 1e-5f);
    EXPECT_NEAR(1.786607, s[0], 1e-5f);
    EXPECT_NEAR(2.444276, s[1], 1e-5f);
    EXPECT_NEAR(6.390113, s[2], 1e-5f);
  }
  // make sure object space translations work
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);

    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));
    UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(2.0, 2.0, 2.0));
    UsdGeomXformOp rotate = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("rot"));
    rotate.Set(GfVec3d(15.0, 30.0, 45.0));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

    MayaUsdUtils::TransformManipulatorEx processor(xform.GetPrim(), first.GetOpName());
    EXPECT_TRUE(processor.SetTranslate(GfVec3d(3.0, 4.0, 5.0), MayaUsdUtils::TransformManipulator::kPostTransform));
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kTranslate, processor.ManipMode());

    auto S = processor.Translation();
    GfVec3d s;
    first.Get(&s);
    
    EXPECT_NEAR(2.093687, S[0], 1e-5f);
    EXPECT_NEAR(5.727662, S[1], 1e-5f);
    EXPECT_NEAR(3.579157, S[2], 1e-5f);
    EXPECT_NEAR(2.093687, s[0], 1e-5f);
    EXPECT_NEAR(5.727662, s[1], 1e-5f);
    EXPECT_NEAR(3.579157, s[2], 1e-5f);
  }
}


//----------------------------------------------------------------------------------------------------------------------
//  bool SetRotate(const GfQuat& scale, Space space = kTransform);
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformManipulatorEx, SetRotate)
{
  // make sure simple translation set up works (transform-space translation)
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);

    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));
    UsdGeomXformOp T = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("T"));
    UsdGeomXformOp R = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("R"));
    T.Set(GfVec3d(2.0, 2.0, 2.0));
    R.Set(GfVec3d(0, 0, 0));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

    double angle = 45.0 * (M_PI / 360.0);
    double sa = std::sin(angle);
    double ca = std::cos(angle);
    GfQuatd Q(ca, sa, 0, 0);

    MayaUsdUtils::TransformManipulatorEx processor(xform.GetPrim(), R.GetOpName());
    EXPECT_TRUE(processor.SetRotate(Q, MayaUsdUtils::TransformManipulator::kTransform));
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());

    auto RR = processor.Rotation();
    GfVec3d r;
    R.Get(&r);

    EXPECT_NEAR(Q.GetImaginary()[0], RR.GetImaginary()[0], 1e-2f);
    EXPECT_NEAR(Q.GetImaginary()[1], RR.GetImaginary()[1], 1e-2f);
    EXPECT_NEAR(Q.GetImaginary()[2], RR.GetImaginary()[2], 1e-2f);
    EXPECT_NEAR(Q.GetReal(), RR.GetReal(), 1e-5f);
    EXPECT_NEAR(45.0, r[0], 1e-5f);
    EXPECT_NEAR(0.0, r[1], 1e-5f);
    EXPECT_NEAR(0.0, r[2], 1e-5f);
  }

  // make sure simple translation set up works (transform-space translation)
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);

    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));
    UsdGeomXformOp T = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("T"));
    UsdGeomXformOp R = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("R"));
    T.Set(GfVec3d(2.0, 2.0, 2.0));
    R.Set(GfVec3d(0, 0, 0));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

    double angle = 45.0 * (M_PI / 360.0);
    double sa = std::sin(angle);
    double ca = std::cos(angle);
    GfQuatd Q(ca, sa, 0, 0);

    MayaUsdUtils::TransformManipulatorEx processor(xform.GetPrim(), R.GetOpName());
    EXPECT_TRUE(processor.SetRotate(Q, MayaUsdUtils::TransformManipulator::kTransform));
    EXPECT_EQ(MayaUsdUtils::TransformManipulator::kRotate, processor.ManipMode());

    auto RR = processor.Rotation();
    GfVec3d r;
    R.Get(&r);

    EXPECT_NEAR(Q.GetImaginary()[0], RR.GetImaginary()[0], 1e-2f);
    EXPECT_NEAR(Q.GetImaginary()[1], RR.GetImaginary()[1], 1e-2f);
    EXPECT_NEAR(Q.GetImaginary()[2], RR.GetImaginary()[2], 1e-2f);
    EXPECT_NEAR(Q.GetReal(), RR.GetReal(), 1e-2f);
    EXPECT_NEAR(45.0, r[0], 1e-5f);
    EXPECT_NEAR(0.0, r[1], 1e-5f);
    EXPECT_NEAR(0.0, r[2], 1e-5f);
  }
}

