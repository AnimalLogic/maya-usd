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
#include <mayaUsdUtils/TransformEvaluator.h>

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
// Test that up to 3 translations in a row evaluate correctly using double precision
// The code should concatonate these into a single vec3 and simply assign the resulting translation. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformEvaluator, translate_d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(1.0, 2.0, 3.0));
  GfMatrix4d first_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
  second.Set(GfVec3d(0.1, 0.2, 0.3));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                           0.0, 1.0, 0.0, 0.0,
                           0.0, 0.0, 1.0, 0.0,
                           1.0, 2.0, 3.0, 1.0);

  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);

  UsdGeomXformOp third = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("third"));
  third.Set(GfVec3d(0.01, 0.02, 0.03));
  GfMatrix4d third_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          1.1, 2.2, 3.3, 1.0);
  GfMatrix4d final_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          1.11, 2.22, 3.33, 1.0);

  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-5f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using float precision
// The code should concatonate these into a single vec3 and simply assign the resulting translation. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformEvaluator, translate_f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));


  UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(1.0f, 2.0f, 3.0f));
  GfMatrix4d first_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("second"));
  second.Set(GfVec3f(0.1f, 0.2f, 0.3f));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                           0.0, 1.0, 0.0, 0.0,
                           0.0, 0.0, 1.0, 0.0,
                           1.0, 2.0, 3.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);

  UsdGeomXformOp third = xform.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("third"));
  third.Set(GfVec3f(0.01f, 0.02f, 0.03f));
  GfMatrix4d third_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          1.1, 2.2, 3.3, 1.0);
  GfMatrix4d final_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          1.11, 2.22, 3.33, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-5f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using float precision
// The code should concatonate these into a single vec3 and simply assign the resulting translation. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformEvaluator, translate_h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));


  UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(1.0f, 2.0f, 3.0f));
  GfMatrix4d first_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("second"));
  second.Set(GfVec3h(0.1f, 0.2f, 0.3f));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                           0.0, 1.0, 0.0, 0.0,
                           0.0, 0.0, 1.0, 0.0,
                           1.0, 2.0, 3.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);

  UsdGeomXformOp third = xform.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("third"));
  third.Set(GfVec3h(0.01f, 0.02f, 0.03f));
  GfMatrix4d third_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          1.1, 2.2, 3.3, 1.0);
  GfMatrix4d final_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          1.11, 2.22, 3.33, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-4f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-4f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-4f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-4f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-4f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using a mix of precision
// The code should concatonate these into a single vec3 and simply assign the resulting translation. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformEvaluator, translate_dfh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));


  UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(1.0, 2.0, 3.0));
  GfMatrix4d first_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("second"));
  second.Set(GfVec3f(0.1f, 0.2f, 0.3f));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          1.0, 2.0, 3.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);

  UsdGeomXformOp third = xform.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("third"));
  third.Set(GfVec3h(0.01f, 0.02f, 0.03f));
  GfMatrix4d third_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          1.1, 2.2, 3.3, 1.0);
  GfMatrix4d final_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          1.11, 2.22, 3.33, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-5f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using double precision
// The code should concatonate these into a single vec3 and then transform by the prior evaluated matrix 
// (which will be a scaling matrix)
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformEvaluator, scale_translate_d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp scale = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("scale"));
  scale.Set(GfVec3d(1.0, 10.0, 100.0));
  GfMatrix4d scale_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);


  UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(1.0, 2.0, 3.0));
  GfMatrix4d first_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 10.0, 0.0, 0.0,
                          0.0, 0.0, 100.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
  second.Set(GfVec3d(0.1, 0.2, 0.3));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 10.0, 0.0, 0.0,
                          0.0, 0.0, 100.0, 0.0,
                          1.0, 20.0, 300.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);

  UsdGeomXformOp third = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("third"));
  third.Set(GfVec3d(0.01, 0.02, 0.03));
  GfMatrix4d third_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 10.0, 0.0, 0.0,
                          0.0, 0.0, 100.0, 0.0,
                          1.1, 22, 330, 1.0);
  GfMatrix4d final_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 10.0, 0.0, 0.0,
                          0.0, 0.0, 100.0, 0.0,
                          1.11, 22.2, 333, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 4, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-5f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using float precision
// The code should concatonate these into a single vec3 and then transform by the prior evaluated matrix 
// (which will be a scaling matrix)
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformEvaluator, scale_translate_f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp scale = xform.AddScaleOp(UsdGeomXformOp::PrecisionFloat, TfToken("scale"));
  scale.Set(GfVec3f(1.0f, 10.0f, 100.0f));
  GfMatrix4d scale_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);


  UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(1.0f, 2.0f, 3.0f));
  GfMatrix4d first_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 10.0, 0.0, 0.0,
                          0.0, 0.0, 100.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("second"));
  second.Set(GfVec3f(0.1f, 0.2f, 0.3f));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 10.0, 0.0, 0.0,
                          0.0, 0.0, 100.0, 0.0,
                          1.0, 20.0, 300.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);

  UsdGeomXformOp third = xform.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("third"));
  third.Set(GfVec3f(0.01f, 0.02f, 0.03f));
  GfMatrix4d third_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 10.0, 0.0, 0.0,
                          0.0, 0.0, 100.0, 0.0,
                          1.1, 22, 330, 1.0);
  GfMatrix4d final_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 10.0, 0.0, 0.0,
                          0.0, 0.0, 100.0, 0.0,
                          1.11, 22.2, 333, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 4, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-5f);


  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using float precision
// The code should concatonate these into a single vec3 and then transform by the prior evaluated matrix 
// (which will be a scaling matrix)
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformEvaluator, scale_translate_h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp scale = xform.AddScaleOp(UsdGeomXformOp::PrecisionHalf, TfToken("scale"));
  scale.Set(GfVec3h(1.0f, 10.0f, 100.0f));
  GfMatrix4d scale_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);


  UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(1.0f, 2.0f, 3.0f));
  GfMatrix4d first_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 10.0, 0.0, 0.0,
                          0.0, 0.0, 100.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("second"));
  second.Set(GfVec3h(0.1f, 0.2f, 0.3f));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 10.0, 0.0, 0.0,
                          0.0, 0.0, 100.0, 0.0,
                          1.0, 20.0, 300.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);

  UsdGeomXformOp third = xform.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("third"));
  third.Set(GfVec3h(0.01f, 0.02f, 0.03f));
  GfMatrix4d third_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 10.0, 0.0, 0.0,
                          0.0, 0.0, 100.0, 0.0,
                          1.1, 22, 330, 1.0);
  GfMatrix4d final_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 10.0, 0.0, 0.0,
                          0.0, 0.0, 100.0, 0.0,
                          1.11, 22.2, 333, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 5e-3f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 4, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 5e-3f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 5e-3f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using double precision
// The code should concatonate these into a single vec3 and simply assign the resulting translation. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformEvaluator, scale_d)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));


  UsdGeomXformOp first = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(1.0, 2.0, 3.0));
  GfMatrix4d first_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
  second.Set(GfVec3d(0.1, 0.2, 0.3));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 2.0, 0.0, 0.0,
                          0.0, 0.0, 3.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);

  UsdGeomXformOp third = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("third"));
  third.Set(GfVec3d(0.01, 0.02, 0.03));
  GfMatrix4d third_result(1.0 * 0.1, 0.0, 0.0, 0.0,
                          0.0, 2.0 * 0.2, 0.0, 0.0,
                          0.0, 0.0, 3.0 * 0.3, 0.0,
                          0.0, 0.0, 0.0, 1.0);
  GfMatrix4d final_result(1.0 * 0.1 * 0.01, 0.0, 0.0, 0.0,
                          0.0, 2.0 * 0.2 * 0.02, 0.0, 0.0,
                          0.0, 0.0, 3.0 * 0.3 * 0.03, 0.0,
                          0.0, 0.0, 0.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-5f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using float precision
// The code should concatonate these into a single vec3 and simply assign the resulting translation. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformEvaluator, scale_f)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));


  UsdGeomXformOp first = xform.AddScaleOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(1.0f, 2.0f, 3.0f));
  GfMatrix4d first_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddScaleOp(UsdGeomXformOp::PrecisionFloat, TfToken("second"));
  second.Set(GfVec3f(0.1f, 0.2f, 0.3f));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 2.0, 0.0, 0.0,
                          0.0, 0.0, 3.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);

  UsdGeomXformOp third = xform.AddScaleOp(UsdGeomXformOp::PrecisionFloat, TfToken("third"));
  third.Set(GfVec3f(0.01f, 0.02f, 0.03f));
  GfMatrix4d third_result(1.0 * 0.1, 0.0, 0.0, 0.0,
                          0.0, 2.0 * 0.2, 0.0, 0.0,
                          0.0, 0.0, 3.0 * 0.3, 0.0,
                          0.0, 0.0, 0.0, 1.0);
  GfMatrix4d final_result(1.0 * 0.1 * 0.01, 0.0, 0.0, 0.0,
                          0.0, 2.0 * 0.2 * 0.02, 0.0, 0.0,
                          0.0, 0.0, 3.0 * 0.3 * 0.03, 0.0,
                          0.0, 0.0, 0.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-5f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using float precision
// The code should concatonate these into a single vec3 and simply assign the resulting translation. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformEvaluator, scale_h)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));


  UsdGeomXformOp first = xform.AddScaleOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(1.0f, 2.0f, 3.0f));
  GfMatrix4d first_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddScaleOp(UsdGeomXformOp::PrecisionHalf, TfToken("second"));
  second.Set(GfVec3h(0.1f, 0.2f, 0.3f));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 2.0, 0.0, 0.0,
                          0.0, 0.0, 3.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);

  UsdGeomXformOp third = xform.AddScaleOp(UsdGeomXformOp::PrecisionHalf, TfToken("third"));
  third.Set(GfVec3h(0.01f, 0.02f, 0.03f));
  GfMatrix4d third_result(1.0 * 0.1, 0.0, 0.0, 0.0,
                          0.0, 2.0 * 0.2, 0.0, 0.0,
                          0.0, 0.0, 3.0 * 0.3, 0.0,
                          0.0, 0.0, 0.0, 1.0);
  GfMatrix4d final_result(1.0 * 0.1 * 0.01, 0.0, 0.0, 0.0,
                          0.0, 2.0 * 0.2 * 0.02, 0.0, 0.0,
                          0.0, 0.0, 3.0 * 0.3 * 0.03, 0.0,
                          0.0, 0.0, 0.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-4f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-3f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-3f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using a mix of precision
// The code should concatonate these into a single vec3 and simply assign the resulting translation. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformEvaluator, scale_dfh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));


  UsdGeomXformOp first = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(1.0, 2.0, 3.0));
  GfMatrix4d first_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddScaleOp(UsdGeomXformOp::PrecisionFloat, TfToken("second"));
  second.Set(GfVec3f(0.1f, 0.2f, 0.3f));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 2.0, 0.0, 0.0,
                          0.0, 0.0, 3.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);

  UsdGeomXformOp third = xform.AddScaleOp(UsdGeomXformOp::PrecisionHalf, TfToken("third"));
  third.Set(GfVec3h(0.01f, 0.02f, 0.03f));
  GfMatrix4d third_result(1.0 * 0.1, 0.0, 0.0, 0.0,
                          0.0, 2.0 * 0.2, 0.0, 0.0,
                          0.0, 0.0, 3.0 * 0.3, 0.0,
                          0.0, 0.0, 0.0, 1.0);
  GfMatrix4d final_result(1.0 * 0.1 * 0.01, 0.0, 0.0, 0.0,
                          0.0, 2.0 * 0.2 * 0.02, 0.0, 0.0,
                          0.0, 0.0, 3.0 * 0.3 * 0.03, 0.0,
                          0.0, 0.0, 0.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-5f);
  result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-5f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that single-axis rotations are correctly evaluated with differing precision
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformEvaluator, rotate_x)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(23.0);

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }

  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateXOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
    first.Set(23.0f);

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }

  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateXOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
    first.Set(GfHalf(23.0f));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformEvaluator, rotate_y)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateYOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(23.0);

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateYOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
    first.Set(23.0f);

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateYOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
    first.Set(GfHalf(23.0f));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformEvaluator, rotate_z)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(23.0);

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateZOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
    first.Set(23.0f);

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateZOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
    first.Set(GfHalf(23.0f));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Test that three-axis rotations are correctly evaluated with differing precision
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformEvaluator, rotate_xyz)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }

  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
    first.Set(GfVec3f(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }

  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
    first.Set(GfVec3h(23.0f, 31.0f, -22.9f));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformEvaluator, rotate_xzy)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }

  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
    first.Set(GfVec3f(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }

  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
    first.Set(GfVec3h(23.0f, 31.0f, -22.9f));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformEvaluator, rotate_yxz)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateYXZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }

  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateYXZOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
    first.Set(GfVec3f(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }

  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateYXZOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
    first.Set(GfVec3h(23.0f, 31.0f, -22.9f));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformEvaluator, rotate_yzx)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }

  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
    first.Set(GfVec3f(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }

  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
    first.Set(GfVec3h(23.0f, 31.0f, -22.9f));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformEvaluator, rotate_zxy)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateZXYOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }

  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateZXYOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
    first.Set(GfVec3f(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }

  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateZXYOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
    first.Set(GfVec3h(23.0f, 31.0f, -22.9f));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformEvaluator, rotate_zyx)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }

  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
    first.Set(GfVec3f(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }

  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
    first.Set(GfVec3h(23.0f, 31.0f, -22.9f));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformEvaluator, combined_rotation)
{
  auto randf = []()
  {
    return -180.0f + 360.0f*(rand()/float(RAND_MAX));
  };

  for(int k = 0; k < 1; ++k)
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(randf(), randf(), randf()));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);

    UsdGeomXformOp second = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
    second.Set(GfVec3d(randf(), randf(), randf()));

    ops = xform.GetOrderedXformOps(&resetsXformStack);
    final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
    
    UsdGeomXformOp third = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionDouble, TfToken("third"));
    third.Set(GfVec3d(randf(), randf(), randf()));

    ops = xform.GetOrderedXformOps(&resetsXformStack);
    final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformEvaluator, orient)
{
  auto randf = []()
  {
    return -1.0f + 2.0f*(rand()/float(RAND_MAX));
  };
  
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddOrientOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    GfQuatd q(randf(), randf(), randf(), randf());
    q.Normalize();
    first.Set(q);

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }

  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddOrientOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
    GfQuatf q(randf(), randf(), randf(), randf());
    q.Normalize();
    first.Set(q);

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }

  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddOrientOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
    GfQuath q(randf(), randf(), randf(), randf());
    q.Normalize();
    first.Set(q);

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 2.6e-3f);
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Test that inverse scale ops evaluate correctly
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformEvaluator, inverse_scaled)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(1.0, 2.0, 3.0));

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
  second.Set(GfVec3d(3.2, 2.2, 1.2));

  UsdGeomXformOp third = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"), true);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d evaluated = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

TEST(TransformEvaluator, inverse_scalef)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddScaleOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(1.0, 2.0, 3.0));

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
  second.Set(GfVec3d(3.2, 2.2, 1.2));

  UsdGeomXformOp third = xform.AddScaleOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"), true);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d evaluated = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

TEST(TransformEvaluator, inverse_scaleh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddScaleOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(1.0, 2.0, 3.0));

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
  second.Set(GfVec3d(3.2, 2.2, 1.2));

  UsdGeomXformOp third = xform.AddScaleOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"), true);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d evaluated = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that inverse translate ops evaluate correctly
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformEvaluator, inverse_translated)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(1.0, 2.0, 3.0));

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
  second.Set(GfVec3d(3.2, 2.2, 1.2));

  UsdGeomXformOp third = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"), true);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d evaluated = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

TEST(TransformEvaluator, inverse_translatef)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(1.0f, 2.0f, 3.0f));

  UsdGeomXformOp second = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
  second.Set(GfVec3d(3.2f, 2.2f, 1.2f));

  UsdGeomXformOp third = xform.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"), true);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d evaluated = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

TEST(TransformEvaluator, inverse_translateh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(1.0f, 2.0f, 3.0f));

  UsdGeomXformOp second = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
  second.Set(GfVec3d(3.2f, 2.2f, 1.2f));

  UsdGeomXformOp third = xform.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"), true);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d evaluated = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that inverse rotate ops evaluate correctly
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformEvaluator, inverse_rotated)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(1.0, 2.0, 3.0));

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
  second.Set(GfVec3d(3.2, 2.2, 1.2));

  UsdGeomXformOp third = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"), true);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d evaluated = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

TEST(TransformEvaluator, inverse_rotatef)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(1.0, 2.0, 3.0));

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
  second.Set(GfVec3d(3.2, 2.2, 1.2));

  UsdGeomXformOp third = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"), true);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d evaluated = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

TEST(TransformEvaluator, inverse_rotateh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(1.0, 2.0, 3.0));

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
  second.Set(GfVec3d(3.2, 2.2, 1.2));

  UsdGeomXformOp third = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"), true);

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
  GfMatrix4d evaluated = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}
