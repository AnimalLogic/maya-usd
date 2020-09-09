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
#ifndef MAYAUSDUTILS_TRANSFOM_OP_INSERTER_H_
#define MAYAUSDUTILS_TRANSFOM_OP_INSERTER_H_

#include "Api.h"
#include <pxr/usd/usd/editTarget.h>
#include <pxr/usd/usdGeom/xformable.h>
#include <pxr/usd/usd/prim.h>

PXR_NAMESPACE_USING_DIRECTIVE

namespace MayaUsdUtils {

/// \brief  a simple struct containing the info needed to undo an transform op creation operation
struct TransformOpInserterUndoInfo
{
  UsdEditTarget fEditTarget;
  UsdGeomXformOp fOp;
  bool fCreatedOp = false;
  bool fCreatedOrderedAttr = false;

  UsdEditTarget EditTarget() const { return fEditTarget; }
  UsdGeomXformOp Op() const { return fOp; }
  bool CreatedOp() const { return fCreatedOp; }
  bool CreatedOrderedAttr() const { return fCreatedOrderedAttr; }
};

/// \brief Interface to create xform ops for translate/scale/rotate tools
class TransformOpInserter
  : public TfRefBase, public TfWeakBase
{
public:

  TransformOpInserter() = default;
  ~TransformOpInserter() = default;

  MAYA_USD_UTILS_PUBLIC
  virtual UsdGeomXformOp InsertRotate(UsdGeomXformable& xform);
  MAYA_USD_UTILS_PUBLIC
  virtual UsdGeomXformOp InsertTranslate(UsdGeomXformable& xform);
  MAYA_USD_UTILS_PUBLIC
  virtual UsdGeomXformOp InsertScale(UsdGeomXformable& xform);

  MAYA_USD_UTILS_PUBLIC
  TransformOpInserterUndoInfo DoInsertRotate(UsdGeomXformable& xform);
  MAYA_USD_UTILS_PUBLIC
  TransformOpInserterUndoInfo DoInsertTranslate(UsdGeomXformable& xform);
  MAYA_USD_UTILS_PUBLIC
  TransformOpInserterUndoInfo DoInsertScale(UsdGeomXformable& xform);
  MAYA_USD_UTILS_PUBLIC
  bool DoRemoveOp(UsdGeomXformable& xform, const TransformOpInserterUndoInfo& op);
};
typedef TfRefPtr<TransformOpInserter> TransformOpInserterRefPtr;

} // MayaUsdUtils

#endif
