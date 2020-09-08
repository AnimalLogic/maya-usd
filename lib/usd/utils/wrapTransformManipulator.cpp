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
#include "TransformManipulatorEx.h"

#include <pxr/base/tf/pyPtrHelpers.h>
#include <pxr/base/tf/makePyConstructor.h>
#include <pxr/base/tf/pyResultConversions.h>

#include <boost/python.hpp>

using namespace boost::python;

PXR_NAMESPACE_USING_DIRECTIVE

void wrapTransformManipulator()
{
  {
    typedef MayaUsdUtils::TransformManipulatorEx::Space This;
    enum_<This>("Space")
        .value("kWorld", This::kWorld)
        .value("kPreTransform", This::kPreTransform)
        .value("kTransform", This::kTransform)
        .value("kPostTransform", This::kPostTransform);
  }
  {
    typedef MayaUsdUtils::TransformManipulatorEx::ManipulatorMode This;
    enum_<This>("ManipulatorMode")
        .value("kTranslate", This::kTranslate)
        .value("kRotate", This::kRotate)
        .value("kScale", This::kScale)
        .value("kGuess", This::kGuess);
  }
  {
    typedef MayaUsdUtils::TransformManipulatorEx This;
    class_<This>("TransformManipulator", no_init)
      .def(init<UsdPrim, TfToken, This::ManipulatorMode, UsdTimeCode>((arg("prim"), arg("opName"), arg("manipMode"), arg("timeCode"))))
      .def("UpdateToTime", (void (This::*)(const UsdTimeCode&, UsdGeomXformCache&, This::ManipulatorMode)) &This::UpdateToTime)
      .def("UpdateToTime", (void (This::*)(const UsdTimeCode&, This::ManipulatorMode)) &This::UpdateToTime)
      .def("CanRotate", &This::CanRotate)
      .def("CanRotateX", &This::CanRotateX)
      .def("CanRotateY", &This::CanRotateY)
      .def("CanRotateZ", &This::CanRotateZ)
      .def("CanTranslate", &This::CanTranslate)
      .def("CanScale", &This::CanScale)
      .def("Rotation", &This::Rotation)
      .def("Translation", &This::Translation)
      .def("Scale", (GfVec3d (This::*)() const) &This::Scale)
      .def("Scale", (bool (This::*)(const GfVec3d&, This::Space)) &This::Scale)
      .def("Translate", (bool (This::*)(const GfVec3d&, This::Space)) &This::Translate)
      .def("RotateX", (bool (This::*)(const double, This::Space)) &This::RotateX)
      .def("RotateY", (bool (This::*)(const double, This::Space)) &This::RotateY)
      .def("RotateZ", (bool (This::*)(const double, This::Space)) &This::RotateZ)
      .def("Rotate", (bool (This::*)(const GfQuatd&, This::Space)) &This::Rotate)
      .def("SetTranslate", (bool (This::*)(const GfVec3d&, This::Space)) &This::SetTranslate)
      .def("SetScale", (bool (This::*)(const GfVec3d&, This::Space)) &This::SetScale)
      .def("SetRotate", (bool (This::*)(const GfQuatd&, This::Space)) &This::SetRotate)
      .def("ManipMode", &This::ManipMode)
      .def("ManipulatorMatrix", &This::ManipulatorMatrix)
      .def("MayaManipulatorMatrix", &This::MayaManipulatorMatrix)
      .def("WorldFrame", &This::_WorldFrame)
      .def("ParentFrame", &This::_ParentFrame)
      .def("PostTransformFrame", &This::_PostTransformFrame)
      .def("CoordinateFrame", &This::_CoordinateFrame)
      .def("InvCoordinateFrame", &This::_InvCoordinateFrame)
      .def("InvPostTransformFrame", &This::_InvPostTransformFrame)
      .def("GetOps", &This::_GetOps)
      .def("GetOp", &This::_GetOp)
      .def("GetOpIndex", &This::_GetOpIndex)
    ;
  }
}
