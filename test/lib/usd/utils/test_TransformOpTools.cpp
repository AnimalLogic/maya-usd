#include <mayaUsdUtils/TransformOpTools.h>

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
TEST(TransformOpProcessor, translate_d)
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
  GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
  second.Set(GfVec3d(0.1, 0.2, 0.3));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                           0.0, 1.0, 0.0, 0.0,
                           0.0, 0.0, 1.0, 0.0,
                           1.0, 2.0, 3.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
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
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-5f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using float precision
// The code should concatonate these into a single vec3 and simply assign the resulting translation. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, translate_f)
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
  GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("second"));
  second.Set(GfVec3f(0.1f, 0.2f, 0.3f));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                           0.0, 1.0, 0.0, 0.0,
                           0.0, 0.0, 1.0, 0.0,
                           1.0, 2.0, 3.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
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
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-5f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using float precision
// The code should concatonate these into a single vec3 and simply assign the resulting translation. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, translate_h)
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
  GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("second"));
  second.Set(GfVec3h(0.1f, 0.2f, 0.3f));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                           0.0, 1.0, 0.0, 0.0,
                           0.0, 0.0, 1.0, 0.0,
                           1.0, 2.0, 3.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
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
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-4f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-4f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-4f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-4f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-4f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using a mix of precision
// The code should concatonate these into a single vec3 and simply assign the resulting translation. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, translate_dfh)
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
  GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("second"));
  second.Set(GfVec3f(0.1f, 0.2f, 0.3f));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          1.0, 2.0, 3.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
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
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
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
TEST(TransformOpProcessor, scale_translate_d)
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
  GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
  second.Set(GfVec3d(0.1, 0.2, 0.3));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 10.0, 0.0, 0.0,
                          0.0, 0.0, 100.0, 0.0,
                          1.0, 20.0, 300.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
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
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 4, UsdTimeCode::Default());
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
TEST(TransformOpProcessor, scale_translate_f)
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
  GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionFloat, TfToken("second"));
  second.Set(GfVec3f(0.1f, 0.2f, 0.3f));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 10.0, 0.0, 0.0,
                          0.0, 0.0, 100.0, 0.0,
                          1.0, 20.0, 300.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
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
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 4, UsdTimeCode::Default());
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
TEST(TransformOpProcessor, scale_translate_h)
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
  GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddTranslateOp(UsdGeomXformOp::PrecisionHalf, TfToken("second"));
  second.Set(GfVec3h(0.1f, 0.2f, 0.3f));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 10.0, 0.0, 0.0,
                          0.0, 0.0, 100.0, 0.0,
                          1.0, 20.0, 300.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
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
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, scale_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 5e-3f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 4, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 5e-3f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 5e-3f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using double precision
// The code should concatonate these into a single vec3 and simply assign the resulting translation. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, scale_d)
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
  GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
  second.Set(GfVec3d(0.1, 0.2, 0.3));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 2.0, 0.0, 0.0,
                          0.0, 0.0, 3.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
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
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-5f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using float precision
// The code should concatonate these into a single vec3 and simply assign the resulting translation. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, scale_f)
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
  GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddScaleOp(UsdGeomXformOp::PrecisionFloat, TfToken("second"));
  second.Set(GfVec3f(0.1f, 0.2f, 0.3f));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 2.0, 0.0, 0.0,
                          0.0, 0.0, 3.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
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
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-5f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using float precision
// The code should concatonate these into a single vec3 and simply assign the resulting translation. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, scale_h)
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
  GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddScaleOp(UsdGeomXformOp::PrecisionHalf, TfToken("second"));
  second.Set(GfVec3h(0.1f, 0.2f, 0.3f));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 2.0, 0.0, 0.0,
                          0.0, 0.0, 3.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
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
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-4f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-3f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-3f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that up to 3 translations in a row evaluate correctly using a mix of precision
// The code should concatonate these into a single vec3 and simply assign the resulting translation. 
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, scale_dfh)
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
  GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);

  UsdGeomXformOp second = xform.AddScaleOp(UsdGeomXformOp::PrecisionFloat, TfToken("second"));
  second.Set(GfVec3f(0.1f, 0.2f, 0.3f));
  GfMatrix4d second_result(1.0, 0.0, 0.0, 0.0,
                          0.0, 2.0, 0.0, 0.0,
                          0.0, 0.0, 3.0, 0.0,
                          0.0, 0.0, 0.0, 1.0);
                          
  ops = xform.GetOrderedXformOps(&resetsXformStack);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
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
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 0, UsdTimeCode::Default());
  COMPARE_MAT4(result, first_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
  COMPARE_MAT4(result, second_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
  COMPARE_MAT4(result, third_result, 1e-5f);
  result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
  COMPARE_MAT4(result, final_result, 1e-5f);

  // sanity check - make sure our matrices match USD
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, final_result, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that single-axis rotations are correctly evaluated with differing precision
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, rotate_x)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(23.0);

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformOpProcessor, rotate_y)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateYOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(23.0);

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformOpProcessor, rotate_z)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(23.0);

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Test that three-axis rotations are correctly evaluated with differing precision
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, rotate_xyz)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformOpProcessor, rotate_xzy)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformOpProcessor, rotate_yxz)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateYXZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformOpProcessor, rotate_yzx)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformOpProcessor, rotate_zxy)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateZXYOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformOpProcessor, rotate_zyx)
{
  {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    ASSERT_TRUE(stage);
    UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

    UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
    first.Set(GfVec3d(23.0, 31.0, -22.9));

    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformOpProcessor, combined_rotation)
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);

    UsdGeomXformOp second = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("second"));
    second.Set(GfVec3d(randf(), randf(), randf()));

    ops = xform.GetOrderedXformOps(&resetsXformStack);
    final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
    
    UsdGeomXformOp third = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionDouble, TfToken("third"));
    third.Set(GfVec3d(randf(), randf(), randf()));

    ops = xform.GetOrderedXformOps(&resetsXformStack);
    final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 1e-5f);
  }
}

TEST(TransformOpProcessor, orient)
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
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
    GfMatrix4d final_result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default()), result;
    EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
    COMPARE_MAT4(result, final_result, 2.6e-3f);
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Test that inverse scale ops evaluate correctly
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, inverse_scaled)
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
  GfMatrix4d evaluated = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

TEST(TransformOpProcessor, inverse_scalef)
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
  GfMatrix4d evaluated = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

TEST(TransformOpProcessor, inverse_scaleh)
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
  GfMatrix4d evaluated = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that inverse translate ops evaluate correctly
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, inverse_translated)
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
  GfMatrix4d evaluated = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

TEST(TransformOpProcessor, inverse_translatef)
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
  GfMatrix4d evaluated = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

TEST(TransformOpProcessor, inverse_translateh)
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
  GfMatrix4d evaluated = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that inverse rotate ops evaluate correctly
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, inverse_rotated)
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
  GfMatrix4d evaluated = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

TEST(TransformOpProcessor, inverse_rotatef)
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
  GfMatrix4d evaluated = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

TEST(TransformOpProcessor, inverse_rotateh)
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
  GfMatrix4d evaluated = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default()), result;
  EXPECT_TRUE(xform.GetLocalTransformation(&result, ops, UsdTimeCode::Default()));
  COMPARE_MAT4(result, evaluated, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Test that ops that have no value set don't accumulate any garbage as a result
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, no_scale_value)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddScaleOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), first.GetOpName());
  processor.Scale(GfVec3d(2.0, 0.5, 0.3), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3d scale;
  first.Get(&scale);
  EXPECT_NEAR(2.0, scale[0], 1e-5f);
  EXPECT_NEAR(0.5, scale[1], 1e-5f);
  EXPECT_NEAR(0.3, scale[2], 1e-5f);

  processor.Scale(GfVec3d(2.0, 0.5, 0.3), MayaUsdUtils::TransformOpProcessor::kTransform);

  first.Get(&scale);
  EXPECT_NEAR(2.0 * 2.0, scale[0], 1e-5f);
  EXPECT_NEAR(0.5 * 0.5, scale[1], 1e-5f);
  EXPECT_NEAR(0.3 * 0.3, scale[2], 1e-5f);
}

TEST(TransformOpProcessor, no_translate_value)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), first.GetOpName());
  processor.Translate(GfVec3d(2.0, 0.5, 0.3), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3d translate;
  first.Get(&translate);
  EXPECT_NEAR(2.0, translate[0], 1e-5f);
  EXPECT_NEAR(0.5, translate[1], 1e-5f);
  EXPECT_NEAR(0.3, translate[2], 1e-5f);

  processor.Translate(GfVec3d(2.0, 0.5, 0.3), MayaUsdUtils::TransformOpProcessor::kTransform);

  first.Get(&translate);
  EXPECT_NEAR(4.0, translate[0], 1e-5f);
  EXPECT_NEAR(1.0, translate[1], 1e-5f);
  EXPECT_NEAR(0.6, translate[2], 1e-5f);
}

TEST(TransformOpProcessor, no_rotate_value)
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

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate[0], 1e-5f);
  EXPECT_NEAR(0.0, rotate[1], 1e-5f);
  EXPECT_NEAR(0.0, rotate[2], 1e-5f);

  // 45 degrees around X
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate[0], 1e-5f);
  EXPECT_NEAR(0.0, rotate[1], 1e-5f);
  EXPECT_NEAR(0.0, rotate[2], 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Make sure we can rotate single axis rotations (double)
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, rotateXd)
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
  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), first.GetOpName());
  // 45 degrees around X
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  double rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5);

  // 45 degrees around X
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5);
}

TEST(TransformOpProcessor, rotateYd)
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
  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), first.GetOpName());
  // 45 degrees around Y
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  double rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5);

  // 45 degrees around Y
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5);
}

TEST(TransformOpProcessor, rotateZd)
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
  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), first.GetOpName());
  // 45 degrees around Z
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  double rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5);

  // 45 degrees around Z
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5);
}

//----------------------------------------------------------------------------------------------------------------------
// Make sure we can rotate three axis rotations (float)
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, rotateXf)
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
  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), first.GetOpName());
  // 45 degrees around X
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  float rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5f);

  // 45 degrees around X
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5f);
}

TEST(TransformOpProcessor, rotateYf)
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
  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), first.GetOpName());
  // 45 degrees around X
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  float rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5f);

  // 45 degrees around X
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5f);
}

TEST(TransformOpProcessor, rotateZf)
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
  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), first.GetOpName());
  // 45 degrees around X
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  float rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5f);

  // 45 degrees around X
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// Make sure we can rotate three axis rotations (half)
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, rotateXh)
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
  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), first.GetOpName());
  // 45 degrees around X
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  GfHalf rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5f);

  // 45 degrees around X
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5f);
}

TEST(TransformOpProcessor, rotateYh)
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
  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), first.GetOpName());
  // 45 degrees around X
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  GfHalf rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5f);

  // 45 degrees around X
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5f);
}

TEST(TransformOpProcessor, rotateZh)
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
  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), first.GetOpName());
  // 45 degrees around X
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  GfHalf rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.0, rotate, 1e-5f);

  // 45 degrees around X
  processor.Rotate(Q, MayaUsdUtils::TransformOpProcessor::kTransform);

  first.Get(&rotate);
  EXPECT_NEAR(90.0, rotate, 1e-5f);
}

//----------------------------------------------------------------------------------------------------------------------
// When modifying the 'X' angle in an XYZ rotation, we can go down an optimised path that simply sets the 'X' value
// directly.
//----------------------------------------------------------------------------------------------------------------------

TEST(TransformOpProcessor, rotateXYZd_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateX(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(15.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-5f);
}
TEST(TransformOpProcessor, rotateXYZf_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateX(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(15.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-5f);
}
TEST(TransformOpProcessor, rotateXYZh_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateX(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

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

TEST(TransformOpProcessor, rotateXZYd_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateX(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(15.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-5f);
}
TEST(TransformOpProcessor, rotateXZYf_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateX(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(15.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-5f);
}
TEST(TransformOpProcessor, rotateXZYh_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateX(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

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

TEST(TransformOpProcessor, rotateYXZd_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(23.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-5f);
}
TEST(TransformOpProcessor, rotateYXZf_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(23.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-5f);
}
TEST(TransformOpProcessor, rotateYXZh_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

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

TEST(TransformOpProcessor, rotateYZXd_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(23.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-5f);
}
TEST(TransformOpProcessor, rotateYZXf_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(23.0, rotate[1], 1e-5f);
  EXPECT_NEAR(42.0, rotate[2], 1e-5f);
}
TEST(TransformOpProcessor, rotateYZXh_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  processor.RotateY(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

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

TEST(TransformOpProcessor, rotateZXYd_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZXYOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateZ(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(47.0, rotate[2], 1e-5f);
}
TEST(TransformOpProcessor, rotateZXYf_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZXYOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateZ(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(47.0, rotate[2], 1e-5f);
}
TEST(TransformOpProcessor, rotateZXYh_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZXYOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateZ(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

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

TEST(TransformOpProcessor, rotateZYXd_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));
  first.Set(GfVec3d(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateZ(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(47.0, rotate[2], 1e-5f);
}
TEST(TransformOpProcessor, rotateZYXf_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));
  first.Set(GfVec3f(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateZ(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(10.0, rotate[0], 1e-5f);
  EXPECT_NEAR(18.0, rotate[1], 1e-5f);
  EXPECT_NEAR(47.0, rotate[2], 1e-5f);
}
TEST(TransformOpProcessor, rotateZYXh_rotate_first)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));
  first.Set(GfVec3h(10, 18, 42));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  processor.RotateZ(5.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

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
TEST(TransformOpProcessor, rotateXYZd)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(19.390714, rotate[0], 1e-5f);
  EXPECT_NEAR(-52.266911, rotate[1], 1e-5f);
  EXPECT_NEAR(32.255846, rotate[2], 1e-5f);
}

TEST(TransformOpProcessor, rotateXYZf)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(19.390714, rotate[0], 1e-5f);
  EXPECT_NEAR(-52.266911, rotate[1], 1e-5f);
  EXPECT_NEAR(32.255846, rotate[2], 1e-5f);
}

TEST(TransformOpProcessor, rotateXYZh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

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
TEST(TransformOpProcessor, rotateXZYd)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.915175, rotate[0], 1e-5f);
  EXPECT_NEAR(-56.79962, rotate[1], 1e-5f);
  EXPECT_NEAR(19.063526, rotate[2], 1e-5f);
}

TEST(TransformOpProcessor, rotateXZYf)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(45.915175, rotate[0], 1e-5f);
  EXPECT_NEAR(-56.79962, rotate[1], 1e-5f);
  EXPECT_NEAR(19.063526, rotate[2], 1e-5f);
}

TEST(TransformOpProcessor, rotateXZYh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateXZYOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

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
TEST(TransformOpProcessor, rotateYXZd)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYXZOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(11.723195, rotate[0], 1e-5f);
  EXPECT_NEAR(-53.873641, rotate[1], 1e-5f);
  EXPECT_NEAR(47.811204, rotate[2], 1e-5f);
}

TEST(TransformOpProcessor, rotateYXZf)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYXZOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(11.723195, rotate[0], 1e-5f);
  EXPECT_NEAR(-53.873641, rotate[1], 1e-5f);
  EXPECT_NEAR(47.811204, rotate[2], 1e-5f);
}

TEST(TransformOpProcessor, rotateYXZh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYXZOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

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
TEST(TransformOpProcessor, rotateYZXd)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(17.170789, rotate[0], 1e-5f);
  EXPECT_NEAR(-41.238612, rotate[1], 1e-5f);
  EXPECT_NEAR(46.508833, rotate[2], 1e-5f);
}

TEST(TransformOpProcessor, rotateYZXf)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(17.170789, rotate[0], 1e-5f);
  EXPECT_NEAR(-41.238612, rotate[1], 1e-5f);
  EXPECT_NEAR(46.508833, rotate[2], 1e-5f);
}

TEST(TransformOpProcessor, rotateYZXh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateYZXOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

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
TEST(TransformOpProcessor, rotateZXYd)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZXYOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(42.759017, rotate[0], 1e-5f);
  EXPECT_NEAR(-38.164457, rotate[1], 1e-5f);
  EXPECT_NEAR(26.413781, rotate[2], 1e-5f);
}

TEST(TransformOpProcessor, rotateZXYf)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZXYOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(42.759017, rotate[0], 1e-5f);
  EXPECT_NEAR(-38.164457, rotate[1], 1e-5f);
  EXPECT_NEAR(26.413781, rotate[2], 1e-5f);
}

TEST(TransformOpProcessor, rotateZXYh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZXYOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

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
TEST(TransformOpProcessor, rotateZYXd)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionDouble, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3d rotate;
  first.Get(&rotate);
  EXPECT_NEAR(49.6261, rotate[0], 1e-5f);
  EXPECT_NEAR(-26.980493, rotate[1], 1e-5f);
  EXPECT_NEAR(54.49695, rotate[2], 1e-5f);
}

TEST(TransformOpProcessor, rotateZYXf)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionFloat, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

  GfVec3f rotate;
  first.Get(&rotate);
  EXPECT_NEAR(49.6261, rotate[0], 1e-5f);
  EXPECT_NEAR(-26.980493, rotate[1], 1e-5f);
  EXPECT_NEAR(54.49695, rotate[2], 1e-5f);
}

TEST(TransformOpProcessor, rotateZYXh)
{
  UsdStageRefPtr stage = UsdStage::CreateInMemory();
  ASSERT_TRUE(stage);
  UsdGeomXform xform = UsdGeomXform::Define(stage, SdfPath("/xform"));

  UsdGeomXformOp first = xform.AddRotateZYXOp(UsdGeomXformOp::PrecisionHalf, TfToken("first"));

  bool resetsXformStack = false;
  std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&resetsXformStack);

  MayaUsdUtils::TransformOpProcessor processor(xform.GetPrim(), 0);
  // 45 degrees around X
  processor.RotateY(-38.164457 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateX(42.759017 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);
  processor.RotateZ(26.413781 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kTransform);

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
TEST(TransformOpProcessor, rotate_world_space_d_no_scale)
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
    GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
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
    GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 1, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformOpProcessor processor(child.GetPrim(), 1);

  // rotate 15 degrees around X in world space
  {
    GfVec3d rotate(0);
    child_rotate.Get(&rotate);
    processor.RotateX(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kWorld);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(13.218885, rotate[0], 1e-5f);
    EXPECT_NEAR(-3.843776, rotate[1], 1e-5f);
    EXPECT_NEAR(-6.439076, rotate[2], 1e-5f);
  }

  // rotate 15 degrees around Y in world space
  {
    processor.RotateY(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kWorld);
    GfVec3d rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(16.968949, rotate[0], 1e-5f);
    EXPECT_NEAR(10.668345, rotate[1], 1e-5f);
    EXPECT_NEAR(-5.533085, rotate[2], 1e-5f);
  }

  // rotate 15 degrees around Z in world space
  {
    processor.RotateZ(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kWorld);
    GfVec3d rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(22.315417, rotate[0], 1e-5f);
    EXPECT_NEAR(7.835664, rotate[1], 1e-5f);
    EXPECT_NEAR(9.086346, rotate[2], 1e-5f);
  }
}

#if 0
//----------------------------------------------------------------------------------------------------------------------
// Using an XYZ rotation order, replicate some rotations that may occur with the Maya rotate tool, and check we end up 
// with the same result. The Y and Z rotations will utilise quaternions.
//----------------------------------------------------------------------------------------------------------------------
TEST(TransformOpProcessor, rotate_world_space_d)
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
  parent_scale.Set(GfVec3d(2.0, 1.5, 3.0));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(1.731201,0.699451,0.716736,0,-0.456485,1.403647,-0.267203,0,-1.192939,0.135404,2.749284,0,-2,3,1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
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
    GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformOpProcessor processor(child.GetPrim(), 1);

  // rotate 15 degrees around X in world space
  {
    GfVec3d rotate(0);
    child_rotate.Get(&rotate);
    std::cout << rotate[0] << ' ' << rotate[1] << ' ' << rotate[2] << std::endl;
    processor.RotateX(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kWorld);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(6.698577, rotate[0], 1e-5f);
    std::cout << '\n';
    EXPECT_NEAR(-2.552219, rotate[1], 1e-5f);
    std::cout << '\n';
    EXPECT_NEAR(-8.557557, rotate[2], 1e-5f);
    std::cout << '\n';
    std::cout << '\n';
  }

  // rotate 15 degrees around Y in world space
  {
    processor.RotateY(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kWorld);
    GfVec3d rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(7.969821, rotate[0], 1e-5f);
    std::cout << '\n';
    EXPECT_NEAR(7.172523, rotate[1], 1e-5f);
    std::cout << '\n';
    EXPECT_NEAR(-6.933309, rotate[2], 1e-5f);
    std::cout << '\n';
    std::cout << '\n';
  }

  // rotate 15 degrees around Z in world space
  {
    processor.RotateZ(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kWorld);
    GfVec3d rotate(0);
    child_rotate.Get(&rotate);
    EXPECT_NEAR(10.730779, rotate[0], 1e-5f);
    std::cout << '\n';
    EXPECT_NEAR(5.337441, rotate[1], 1e-5f);
    std::cout << '\n';
    EXPECT_NEAR(11.312078, rotate[2], 1e-5f);
    std::cout << '\n';
    std::cout << '\n';
  }
}

TEST(TransformOpProcessor, rotate_world_space_f)
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
  parent_scale.Set(GfVec3d(2.0, 1.5, 3.0));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(1.731201,0.699451,0.716736,0,-0.456485,1.403647,-0.267203,0,-1.192939,0.135404,2.749284,0,-2,3,1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat, TfToken("child_rotate"));
  child_translate.Set(GfVec3f(2.0, 1.0, 2.0));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0,0,1,0,0,0,0,1,0,2,1,2,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformOpProcessor processor(child.GetPrim(), 1);

  // rotate 15 degrees around X in world space
  {
    processor.RotateX(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kWorld);
    GfVec3f rotate;
    child_rotate.Get(&rotate);
    EXPECT_NEAR(6.698577f, rotate[0], 1e-5f);
    EXPECT_NEAR(-2.552219f, rotate[1], 1e-5f);
    EXPECT_NEAR(-8.557557f, rotate[2], 1e-5f);
  }

  // rotate 15 degrees around Y in world space
  {
    processor.RotateY(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kWorld);
    GfVec3f rotate;
    child_rotate.Get(&rotate);
    EXPECT_NEAR(7.969821f, rotate[0], 1e-5f);
    EXPECT_NEAR(7.172523f, rotate[1], 1e-5f);
    EXPECT_NEAR(-6.933309f, rotate[2], 1e-5f);
  }

  // rotate 15 degrees around Z in world space
  {
    processor.RotateZ(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kWorld);
    GfVec3f rotate;
    child_rotate.Get(&rotate);
    EXPECT_NEAR(10.730779f, rotate[0], 1e-5f);
    EXPECT_NEAR(5.337441f, rotate[1], 1e-5f);
    EXPECT_NEAR(11.312078f, rotate[2], 1e-5f);
  }
}

TEST(TransformOpProcessor, rotate_world_space_h)
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
  parent_scale.Set(GfVec3d(2.0, 1.5, 3.0));

  // sanity check parent matrix matches result from maya 
  const GfMatrix4d parent_matrix(1.731201,0.699451,0.716736,0,-0.456485,1.403647,-0.267203,0,-1.192939,0.135404,2.749284,0,-2,3,1,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = parent.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 3, UsdTimeCode::Default());
    COMPARE_MAT4(result, parent_matrix, 1e-5f);
  }

  // now specify a child transform with a rotation
  UsdGeomXform child = UsdGeomXform::Define(stage, SdfPath("/xform/child"));
  UsdGeomXformOp child_translate = child.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("child_translate"));
  UsdGeomXformOp child_rotate = child.AddRotateXYZOp(UsdGeomXformOp::PrecisionHalf, TfToken("child_rotate"));
  child_translate.Set(GfVec3h(2.0f, 1.0f, 2.0f));

  // sanity check starting matrix against maya result
  const GfMatrix4d child_matrix(1,0,0,0,0,1,0,0,0,0,1,0,2,1,2,1);
  {
    bool resetsXformStack = false;
    std::vector<UsdGeomXformOp> ops = child.GetOrderedXformOps(&resetsXformStack);
    GfMatrix4d result = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, 2, UsdTimeCode::Default());
    COMPARE_MAT4(result, child_matrix, 1e-5f);
  }
  
  MayaUsdUtils::TransformOpProcessor processor(child.GetPrim(), 1);

  // rotate 15 degrees around X in world space
  {
    processor.RotateX(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kWorld);
    GfVec3h rotate;
    child_rotate.Get(&rotate);
    EXPECT_NEAR(6.698577f, rotate[0], 1e-5f);
    EXPECT_NEAR(-2.552219f, rotate[1], 1e-5f);
    EXPECT_NEAR(-8.557557f, rotate[2], 1e-5f);
  }

  // rotate 15 degrees around Y in world space
  {
    processor.RotateY(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kWorld);
    GfVec3h rotate;
    child_rotate.Get(&rotate);
    EXPECT_NEAR(7.969821f, rotate[0], 1e-5f);
    EXPECT_NEAR(7.172523f, rotate[1], 1e-5f);
    EXPECT_NEAR(-6.933309f, rotate[2], 1e-5f);
  }

  // rotate 15 degrees around Z in world space
  {
    processor.RotateZ(15.0 * (M_PI / 180.0), MayaUsdUtils::TransformOpProcessor::kWorld);
    GfVec3h rotate;
    child_rotate.Get(&rotate);
    EXPECT_NEAR(10.730779f, rotate[0], 1e-5f);
    EXPECT_NEAR(5.337441f, rotate[1], 1e-5f);
    EXPECT_NEAR(11.312078f, rotate[2], 1e-5f);
  }
}
#endif