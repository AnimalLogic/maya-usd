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
#include "UsdTranslateUndoableCommand.h"

#include "Utils.h"
#include "private/Utils.h"
#include <mayaUsd/base/debugCodes.h>

#include <mayaUsdUtils/TransformOpTools.h>
#include <iostream>

MAYAUSD_NS_DEF {
namespace ufe {

static bool ExistingOpHasSamples(const UsdGeomXformOp& op)
{
	return op.GetNumTimeSamples() != 0;
}

//------------------------------------------------------------------------------
UsdTranslateUndoableCommand::UsdTranslateUndoableCommand(
    const UsdSceneItem::Ptr& item, double x, double y, double z, const UsdTimeCode& timeCode
) : Ufe::TranslateUndoableCommand(item),
	fPrim(ufePathToPrim(item->path())),
	fPrevValue(0,0,0),
	fNewValue(x, y, z),
	fPath(item->path()),
	fTimeCode(timeCode)
{
    try 
    {
        MayaUsdUtils::TransformOpProcessor proc(fPrim, TfToken(""), MayaUsdUtils::TransformOpProcessor::kTranslate, timeCode);
        fOp = proc.op();
		// only write time samples if op already has samples
		if(!ExistingOpHasSamples(fOp))
		{
			fTimeCode = UsdTimeCode::Default();
		}
        fPrevValue = proc.Translation();
    }
    catch(const std::exception& e)
    {
		// use default time code if using a new op?
		fTimeCode = UsdTimeCode::Default();
        
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
        UsdGeomXformable xform(fPrim);
        bool reset;
        std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&reset);
        fOp = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble);
        ops.insert(ops.begin(), fOp);
        xform.SetXformOpOrder(ops, reset);
    }
}

//------------------------------------------------------------------------------
UsdTranslateUndoableCommand::~UsdTranslateUndoableCommand()
{}

//------------------------------------------------------------------------------
UsdTranslateUndoableCommand::Ptr UsdTranslateUndoableCommand::create(
    const UsdSceneItem::Ptr& item, double x, double y, double z, const UsdTimeCode& timeCode
)
{
    auto cmd = std::make_shared<MakeSharedEnabler<UsdTranslateUndoableCommand>>(
        item, x, y, z, timeCode);
    return cmd;
}

//------------------------------------------------------------------------------
void UsdTranslateUndoableCommand::undo()
{
    // do nothing
    if(GfIsClose(fNewValue, fPrevValue, 1e-5f))
    {
        return;
    }
    switch(fOp.GetOpType())
    {
    case UsdGeomXformOp::TypeTranslate:
        {
            switch(fOp.GetPrecision())
            {
            case UsdGeomXformOp::PrecisionHalf:
                {
                    fOp.Set(GfVec3h(fPrevValue[0], fPrevValue[1], fPrevValue[2]), fTimeCode);
                }
                break;

            case UsdGeomXformOp::PrecisionFloat:
                {
                    fOp.Set(GfVec3f(fPrevValue[0], fPrevValue[1], fPrevValue[2]), fTimeCode);
                }
                break;
                
            case UsdGeomXformOp::PrecisionDouble:
                {
                    fOp.Set(fPrevValue, fTimeCode);
                }
                break;
            }
        }
        break;

    case UsdGeomXformOp::TypeTransform:
        {
            GfMatrix4d M;
            fOp.Get(&M, fTimeCode);
            M[3][0] = fPrevValue[0];
            M[3][1] = fPrevValue[1];
            M[3][2] = fPrevValue[2];
            fOp.Set(M, fTimeCode);
        }
        break;

    default:
        break;
    }
}

//------------------------------------------------------------------------------
void UsdTranslateUndoableCommand::redo()
{
}

//------------------------------------------------------------------------------
// Ufe::TranslateUndoableCommand overrides
//------------------------------------------------------------------------------

bool UsdTranslateUndoableCommand::translate(double x, double y, double z)
{
    fNewValue = GfVec3d(x, y, z);

    // do nothing
    if(GfIsClose(fNewValue, fPrevValue, 1e-5f))
    {
        return true;
    }
    try
    {
        MayaUsdUtils::TransformOpProcessor proc(fPrim, TfToken(""), MayaUsdUtils::TransformOpProcessor::kTranslate, fTimeCode);
        auto diff = fNewValue - proc.Translation();
        proc.Translate(diff);
    }
    catch(std::exception e)
    {
        return false;
    }
    return true;
}

} // namespace ufe
} // namespace MayaUsd
