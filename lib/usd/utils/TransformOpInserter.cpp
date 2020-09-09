//
// Copyright 2020 Autodesk
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

#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/sdf/attributeSpec.h>

#include <mayaUsdUtils/TransformManipulator.h>
#include <mayaUsdUtils/SIMD.h>


namespace MayaUsdUtils {

//------------------------------------------------------------------------------
UsdGeomXformOp TransformOpInserter::InsertRotate(UsdGeomXformable& xform)
{
    //
    // For rotation, I'm going to attempt a reasonably sensible guess. 
    // 
    // uniform token[] xformOpOrder = ["xformOp:translate", "xformOp:translate:rotatePivotTranslate", 
    //                                 "xformOp:translate:rotatePivot", "xformOp:rotateXYZ", 
    //                                                                     ^^ This one ^^
    //                                 "!invert!xformOp:translate:rotatePivot", "xformOp:translate:scalePivotTranslate", 
    //                                 "xformOp:translate:scalePivot", "xformOp:scale", "!invert!xformOp:translate:scalePivot"]
    // 
    bool reset;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&reset);
    auto op = xform.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat);
    if(ops.empty())
    {
        // do nothing, rotate will have just been added, so will be the only op in the stack
    }
    else
    {
        // step through the non-inverted translations in the stack
        auto it = ops.begin();
        while(it != ops.end())
        {
            auto isTranslateOp = (it->GetOpType() == UsdGeomXformOp::TypeTranslate);
            if(!isTranslateOp)
                break;

            auto isInverseOp = it->IsInverseOp();
            if(isInverseOp)
                break;
            ++it;
        }

        ops.insert(it, op);

        // update the xform op order
        xform.SetXformOpOrder(ops, reset);
    }
    return op;
}

//------------------------------------------------------------------------------
UsdGeomXformOp TransformOpInserter::InsertTranslate(UsdGeomXformable& xform)
{
    //
    // So I'm going to make the assumption here that you *probably* want to manipulate the very 
    // first translate in the xform op stack?
    // 
    // uniform token[] xformOpOrder = ["xformOp:translate", "xformOp:translate:rotatePivotTranslate", 
    //                                 "xformOp:translate:rotatePivot", "xformOp:rotateXYZ", 
    //                                 "!invert!xformOp:translate:rotatePivot", "xformOp:translate:scalePivotTranslate", 
    //                                 "xformOp:translate:scalePivot", "xformOp:scale", "!invert!xformOp:translate:scalePivot"]
    // 
    // 
    bool reset;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&reset);
    auto op = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble);
    ops.insert(ops.begin(), op);
    xform.SetXformOpOrder(ops, reset);
    return op;
}

//------------------------------------------------------------------------------
UsdGeomXformOp TransformOpInserter::InsertScale(UsdGeomXformable& xform)
{
    //
    // So I'm going to make the assumption here that you *probably* want to manipulate the very 
    // last scale in the xform op stack?
    // 
    // uniform token[] xformOpOrder = ["xformOp:translate", "xformOp:translate:rotatePivotTranslate", 
    //                                 "xformOp:translate:rotatePivot", "xformOp:rotateXYZ", 
    //                                 "!invert!xformOp:translate:rotatePivot", "xformOp:translate:scalePivotTranslate", 
    //                                 "xformOp:translate:scalePivot", "xformOp:scale", "!invert!xformOp:translate:scalePivot"]
    //                                                                 ^^ This one ^^
    // 
    bool reset;
    std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&reset);
    auto op = xform.AddScaleOp(UsdGeomXformOp::PrecisionFloat);
    op.Set(GfVec3f(1.0f, 1.0f, 1.0f));
    if(ops.empty())
    {
        // do nothing, scale will have just been added, so will be the only op in the stack
    }
    else
    {
        auto& back = ops.back();
        auto isInverseOp = back.IsInverseOp();
        auto isTranslateOp = (back.GetOpType() == UsdGeomXformOp::TypeTranslate);
        if(isInverseOp && isTranslateOp)
        {
            // if we have a scale pivot at the end of the stack, insert before last item
            if(back.HasSuffix(TfToken("scalePivot")) || back.HasSuffix(TfToken("pivot")))
            {
                ops.insert(ops.end() - 1, op);
            }
            else
            {
                // default - add to end of transform stack
                ops.push_back(op);
            }
        }
        else
        {
            // default - add to end of transform stack
            ops.push_back(op);
        }
        // update the xform op order
        xform.SetXformOpOrder(ops, reset);
    }
    return op;
}

//------------------------------------------------------------------------------
TransformOpInserterUndoInfo TransformOpInserter::DoInsertRotate(UsdGeomXformable& xform)
{
    auto prim = xform.GetPrim();
    auto xformOpOrderAttr = prim.GetAttribute(TfToken("xformOpOrder"));
    auto fCreatedOrderedAttr = xformOpOrderAttr ? !xformOpOrderAttr.HasAuthoredValue() : true;
    auto stage = prim.GetStage();
    return TransformOpInserterUndoInfo{ stage->GetEditTarget(), InsertRotate(xform), true, fCreatedOrderedAttr };
}

//------------------------------------------------------------------------------
TransformOpInserterUndoInfo TransformOpInserter::DoInsertTranslate(UsdGeomXformable& xform)
{
    auto prim = xform.GetPrim();
    auto xformOpOrderAttr = prim.GetAttribute(TfToken("xformOpOrder"));
    auto fCreatedOrderedAttr = xformOpOrderAttr ? !xformOpOrderAttr.HasAuthoredValue() : true;
    auto stage = prim.GetStage();
    return TransformOpInserterUndoInfo{ stage->GetEditTarget(), InsertTranslate(xform), true, fCreatedOrderedAttr };
}   

//------------------------------------------------------------------------------
TransformOpInserterUndoInfo TransformOpInserter::DoInsertScale(UsdGeomXformable& xform)
{
    auto prim = xform.GetPrim();
    auto xformOpOrderAttr = prim.GetAttribute(TfToken("xformOpOrder"));
    auto fCreatedOrderedAttr = xformOpOrderAttr ? !xformOpOrderAttr.HasAuthoredValue() : true;
    auto stage = prim.GetStage();
    return TransformOpInserterUndoInfo{ stage->GetEditTarget(), InsertScale(xform), true, fCreatedOrderedAttr };
}

//------------------------------------------------------------------------------
bool TransformOpInserter::DoRemoveOp(UsdGeomXformable& xform, const TransformOpInserterUndoInfo& info)
{
    if(info.fCreatedOp)
    {
        auto prim = xform.GetPrim();
        SdfPrimSpecHandle specHandle = info.fEditTarget.GetPrimSpecForScenePath(prim.GetPath());
        if(specHandle)
        {
            // annoyingly, we have to get the xform ops first, otherwise removal of the 
            // attribute spec will cause a bother later on.
            bool reset;
            auto ops = xform.GetOrderedXformOps(&reset);

            auto opName = info.fOp.GetName(); 
            {
                auto attrSpecView = specHandle->GetAttributes();
                for(auto spec : attrSpecView)
                {
                    if(opName == spec->GetName())
                    {
                        specHandle->RemoveProperty(spec);
                        break;
                    }
                }
            }

            // if when creating the original translate op we added a new xformOpOrder attribute
            // as a side effect, be sure we remove that here.
            if(info.fCreatedOrderedAttr)
            {
                auto attrSpecView = specHandle->GetAttributes();
                for(auto spec : attrSpecView)
                {
                    if("xformOpOrder" == spec->GetName())
                    {
                        specHandle->RemoveProperty(spec);
                        break;
                    }
                }
            }
            else
            // otherwise hunt for the xformOp in the list, and remove it.
            {
                for(auto it = ops.begin(); it != ops.end(); ++it)
                {
                    if(it->GetName() == opName)
                    {
                        ops.erase(it);
                        xform.SetXformOpOrder(ops, reset);
                        break;
                    }
                }
            }
        }
    }
    return info.fCreatedOp;
}

}
