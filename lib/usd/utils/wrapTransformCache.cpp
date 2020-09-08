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
#include "TransformCache.h"

#include <pxr/base/tf/pyPtrHelpers.h>
#include <pxr/base/tf/makePyConstructor.h>
#include <pxr/base/tf/pyResultConversions.h>

#include <boost/python.hpp>

using namespace boost::python;

PXR_NAMESPACE_USING_DIRECTIVE

void wrapTransformCache()
{  {
    typedef MayaUsdUtils::TransformCache This;
    class_<This>("TransformCache")
      .def("Local", &This::_Local)
      .def("World", &This::_World)
      .def("InverseLocal", &This::_InverseLocal)
      .def("InverseWorld", &This::_InverseWorld)
      .def("CurrentTime", &This::CurrentTime)
      .def("GetNumEntries", &This::GetNumEntries)
    ;
  }
}
