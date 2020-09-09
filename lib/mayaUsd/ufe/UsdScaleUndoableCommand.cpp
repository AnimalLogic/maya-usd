//
// Copyright 2019 Autodesk
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
#include "UsdScaleUndoableCommand.h"

#include "private/Utils.h"
#include <mayaUsd/ufe/Utils.h>
#include <mayaUsd/base/debugCodes.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/sdf/attributeSpec.h>

#include <mayaUsdUtils/TransformManipulator.h>

MAYAUSD_NS_DEF {
namespace ufe {

static bool ExistingOpHasSamples(const UsdGeomXformOp& op)
{
	return op.GetNumTimeSamples() != 0;
}

UsdScaleUndoableCommand::UsdScaleUndoableCommand(
    const UsdSceneItem::Ptr& item, double x, double y, double z, const UsdTimeCode& timeCode
) : Ufe::ScaleUndoableCommand(item),
	fPrim(ufePathToPrim(item->path())),
	fPrevValue(0,0,0),
	fNewValue(x, y, z),
	fPath(item->path()),
	fTimeCode(timeCode)
{
    try 
    {
        MayaUsdUtils::TransformManipulator proc(fPrim, TfToken(""), MayaUsdUtils::TransformManipulator::kScale, timeCode);
        auto op = proc.op();
		// only write time samples if op already has samples
		if(!ExistingOpHasSamples(op))
		{
			fTimeCode = UsdTimeCode::Default();
		}
        fPrevValue = proc.Scale();

	    auto stage = fPrim.GetStage();
    	fInfo = MayaUsdUtils::TransformOpInserterUndoInfo{ stage->GetEditTarget(), proc.op(), false, false };
    }
    catch(const std::exception& e)
    {
		UsdGeomXformable xform(fPrim);
		fInfo = getTransformOpInserter()->DoInsertScale(xform);
		fPrevValue = GfVec3d(1.0, 1.0, 1.0);
    }
}

UsdScaleUndoableCommand::~UsdScaleUndoableCommand()
{}

/*static*/
UsdScaleUndoableCommand::Ptr UsdScaleUndoableCommand::create(
    const UsdSceneItem::Ptr& item, double x, double y, double z, const UsdTimeCode& timeCode
)
{
	auto cmd = std::make_shared<MakeSharedEnabler<UsdScaleUndoableCommand>>(
        item, x, y, z, timeCode);
    return cmd;

}

void UsdScaleUndoableCommand::undo()
{
	UsdGeomXformable xform(fPrim);
    if(!getTransformOpInserter()->DoRemoveOp(xform, fInfo))
    {
        switch(fInfo.fOp.GetOpType())
        {
        case UsdGeomXformOp::TypeScale:
            {
                switch(fInfo.fOp.GetPrecision())
                {
                case UsdGeomXformOp::PrecisionHalf:
                    {
                        fInfo.fOp.Set(GfVec3h(fPrevValue[0], fPrevValue[1], fPrevValue[2]), fTimeCode);
                    }
                    break;
                case UsdGeomXformOp::PrecisionFloat:
                    {
                        fInfo.fOp.Set(GfVec3f(fPrevValue[0], fPrevValue[1], fPrevValue[2]), fTimeCode);
                    }
                    break;
                case UsdGeomXformOp::PrecisionDouble:
                    {
                        fInfo.fOp.Set(fPrevValue, fTimeCode);
                    }
                    break;
                }
            }
            break;

        case UsdGeomXformOp::TypeTransform:
            {
                GfMatrix4d M;
                fInfo.fOp.Get(&M, fTimeCode);
                GfVec3d relativeScalar(fNewValue[0] / fPrevValue[0], fNewValue[1] / fPrevValue[1], fNewValue[2] / fPrevValue[2]);
                M[0][0] *= relativeScalar[0]; M[0][1] *= relativeScalar[0]; M[0][2] *= relativeScalar[0];
                M[1][0] *= relativeScalar[1]; M[1][1] *= relativeScalar[1]; M[1][2] *= relativeScalar[1];
                M[2][0] *= relativeScalar[2]; M[2][1] *= relativeScalar[2]; M[2][2] *= relativeScalar[2];
                fInfo.fOp.Set(M, fTimeCode);
            }
            break;

        default:
            break;
        }
    }
}

void UsdScaleUndoableCommand::redo()
{
}

//------------------------------------------------------------------------------
// Ufe::ScaleUndoableCommand overrides
//------------------------------------------------------------------------------

bool UsdScaleUndoableCommand::scale(double x, double y, double z)
{
    fNewValue = GfVec3d(x, y, z);
    try
    {
        MayaUsdUtils::TransformManipulator proc(fPrim, TfToken(""), MayaUsdUtils::TransformManipulator::kScale, fTimeCode);
		auto s = proc.Scale();
		
		// do nothing 
		if(GfIsClose(fNewValue, s, 1e-5f))
		{
			return true;
		}

        GfVec3d diff(fNewValue[0] / s[0], fNewValue[1] / s[1], fNewValue[2] / s[2]);
        proc.Scale(diff);
    }
    catch(std::exception e)
    {
        return false;
    }
    return true;
}

} // namespace ufe
} // namespace MayaUsd
