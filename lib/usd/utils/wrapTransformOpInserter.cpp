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
#include "TransformOpInserter.h"

#include <pxr/base/tf/pyPtrHelpers.h>
#include <pxr/base/tf/makePyConstructor.h>
#include <pxr/base/tf/pyResultConversions.h>
#include <pxr/base/tf/pyPolymorphic.h>

#include <boost/python.hpp>

using namespace boost::python;

PXR_NAMESPACE_USING_DIRECTIVE

class PythonTransformOpInserter
  : public MayaUsdUtils::TransformOpInserter,
    public TfPyPolymorphic<MayaUsdUtils::TransformOpInserter>
{
public:
    static TfRefPtr<PythonTransformOpInserter> New()
    {
      return TfCreateRefPtr(new PythonTransformOpInserter);
    }

    UsdGeomXformOp InsertRotate(UsdGeomXformable& xform) override
    {
        return this->CallVirtual<UsdGeomXformOp>("InsertRotate", &PythonTransformOpInserter::InsertRotate)(xform);
    }
    UsdGeomXformOp InsertTranslate(UsdGeomXformable& xform) override
    {
        return this->CallVirtual<UsdGeomXformOp>("InsertTranslate", &PythonTransformOpInserter::InsertTranslate)(xform);
    }
    UsdGeomXformOp InsertScale(UsdGeomXformable& xform) override
    {
        return this->CallVirtual<UsdGeomXformOp>("InsertScale", &PythonTransformOpInserter::InsertScale)(xform);
    }
};

void wrapTransformOpInserter()
{
  {
    typedef MayaUsdUtils::TransformOpInserterUndoInfo This;
    class_<This>("TransformOpInserterUndoInfo", no_init)
      .def("EditTarget", &This::EditTarget)
      .def("Op", &This::Op)
      .def("CreatedOp", &This::CreatedOp)
      .def("CreatedOrderedAttr", &This::CreatedOrderedAttr)
    ;
  }
  {
    typedef TfWeakPtr<PythonTransformOpInserter> PythonTransformOpInserterPtr;
    typedef MayaUsdUtils::TransformOpInserter This;
    class_<PythonTransformOpInserter, PythonTransformOpInserterPtr, boost::noncopyable>
      ("TransformOpInserter", no_init)
      .def(TfPyRefAndWeakPtr())
      .def(TfMakePyConstructor(&PythonTransformOpInserter::New))
      .def("DoInsertRotate", &This::DoInsertRotate)
      .def("DoInsertTranslate", &This::DoInsertTranslate)
      .def("DoInsertScale", &This::DoInsertScale)
      .def("DoRemoveOp", &This::DoRemoveOp)
      .def("InsertRotate", &This::InsertRotate, &PythonTransformOpInserter::InsertRotate)
      .def("InsertTranslate", &This::InsertTranslate, &PythonTransformOpInserter::InsertTranslate)
      .def("InsertScale", &This::InsertScale, &PythonTransformOpInserter::InsertScale)
    ;
  }
}
