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

//----------------------------------------------------------------------------------------------------------------------
/// \brief  a simple struct containing the info needed to undo an transform op creation operation
//----------------------------------------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------------------------------------
/// \brief  Interface to create xform ops for translate/scale/rotate tools
//----------------------------------------------------------------------------------------------------------------------
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

/**! An example python transform op inserter implementation (re-implements the default behaviour)

from pxr import Sdf, Usd, UsdGeom
from mayaUsdUtils import TransformManipulator, TransformOpInserter, TransformEvaluator
import mayaUsd.ufe
from pxr import Gf

#
# This interface provides the methods for creating new xform ops (when no xform op exists)
# This example re-implements the default op inserter behaviour. 
#
class CustomTransformOpInserter(TransformOpInserter):

    #
    # So I'm going to make the assumption here that you *probably* want to manipulate the very 
    # first translate in the xform op stack?
    # 
    # uniform token[] xformOpOrder = ["xformOp:translate", "xformOp:translate:rotatePivotTranslate", 
    #                                 "xformOp:translate:rotatePivot", "xformOp:rotateXYZ", 
    #                                 "!invert!xformOp:translate:rotatePivot", "xformOp:translate:scalePivotTranslate", 
    #                                 "xformOp:translate:scalePivot", "xformOp:scale", "!invert!xformOp:translate:scalePivot"]
    # 
    def InsertTranslate(self, xform):
        ops = xform.GetOrderedXformOps()
        op = xform.AddTranslateOp(UsdGeom.XformOp.PrecisionDouble)
        ops.insert(0, op)
        xform.SetXformOpOrder(ops, reset);
        return op

    #
    # For rotation, I'm going to attempt a reasonably sensible guess. 
    # 
    # uniform token[] xformOpOrder = ["xformOp:translate", "xformOp:translate:rotatePivotTranslate", 
    #                                 "xformOp:translate:rotatePivot", "xformOp:rotateXYZ", 
    #                                                                     ^^ This one ^^
    #                                 "!invert!xformOp:translate:rotatePivot", "xformOp:translate:scalePivotTranslate", 
    #                                 "xformOp:translate:scalePivot", "xformOp:scale", "!invert!xformOp:translate:scalePivot"]
    #
    def InsertRotate(self, xform):
        ops = xform.GetOrderedXformOps()
        op = xform.AddRotateXYZOp(UsdGeom.XformOp.PrecisionFloat)
        if len(ops) != 0:
            count = 0
            for o in ops:
                if o.GetOpType() != UsdGeom.Xform.TypeTranslate:
                    break
                if o.IsInverseOp():
                    break
                count = count + 1
            ops.insert(count, op)
            xform.SetXformOpOrder(ops);
        return op

    #
    # So I'm going to make the assumption here that you *probably* want to manipulate the very 
    # last scale in the xform op stack?
    # 
    # uniform token[] xformOpOrder = ["xformOp:translate", "xformOp:translate:rotatePivotTranslate", 
    #                                 "xformOp:translate:rotatePivot", "xformOp:rotateXYZ", 
    #                                 "!invert!xformOp:translate:rotatePivot", "xformOp:translate:scalePivotTranslate", 
    #                                 "xformOp:translate:scalePivot", "xformOp:scale", "!invert!xformOp:translate:scalePivot"]
    #                                                                 ^^ This one ^^
    # 
    def InsertScale(self, xform):
        ops = xform.GetOrderedXformOps()
        precision = UsdGeom.XformOp.PrecisionDouble
        op = xform.AddScaleOp(precision)
        op.Set(Gf.Vec3f(1, 1, 1))
        count = len(ops)
        # check for the existance of a pivot
        if count > 0:
            isInverse = ops[count-1].IsInverseOp()
            isTranslate = (ops[count-1].GetOpType() == UsdGeom.XformOp.TypeTranslate)
            names = ops[count-1].SplitName()
            last = names[len(names) - 1]
            if last == "scalePivot" or last == "pivot":
                ops.insert(count - 1, op)
                xform.SetXformOpOrder(ops)
        return op


mayaUsd.ufe.setTransformOpInserter(CustomTransformOpInserter())
*/

} // MayaUsdUtils

#endif
