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
#include "UsdRotateUndoableCommand.h"

#include "private/Utils.h"
#include "mayaUsdUtils/MayaTransformAPI.h"
#include "../base/debugCodes.h"

#include <mayaUsdUtils/MayaTransformAPI.h>

MAYAUSD_NS_DEF {
namespace ufe {

TfToken UsdRotateUndoableCommand::rotXYZ("xformOp:rotateXYZ");

UsdRotateUndoableCommand::UsdRotateUndoableCommand(
    const UsdSceneItem::Ptr& item, double x, double y, double z, const UsdTimeCode& timeCode)
	: Ufe::RotateUndoableCommand(item),
      UsdTRSUndoableCommandBase(item, x, y, z, timeCode)
{
}

UsdRotateUndoableCommand::~UsdRotateUndoableCommand()
{}

/*static*/
UsdRotateUndoableCommand::Ptr UsdRotateUndoableCommand::create(
    const UsdSceneItem::Ptr& item, double x, double y, double z, const UsdTimeCode& timeCode)
{
	auto cmd = std::make_shared<MakeSharedEnabler<UsdRotateUndoableCommand>>(
        item, x, y, z, timeCode);
    cmd->initialize();
    return cmd;
}

void UsdRotateUndoableCommand::undo()
{
  	UsdTRSUndoableCommandBase::undoImp();
}

void UsdRotateUndoableCommand::redo()
{
    redoImp();
}

void UsdRotateUndoableCommand::addEmptyAttribute()
{
    performImp(0, 0, 0);	// Add an empty rotate
}

void UsdRotateUndoableCommand::performImp(double x, double y, double z)
{
	MayaUsdUtils::MayaTransformAPI api(prim());
	const auto order = api.rotateOrder();
	TF_DEBUG(MAYAUSD_UFE_MANIPULATORS).Msg("UsdRotateUndoableCommand::undo %s (%lf, %lf, %lf) [%d] @%lf\n", 
		path().string().c_str(), x, y, z, int(order), timeCode().GetValue());

	api.rotate(float(M_PI) * GfVec3f(x, y, z) / 180.0f, order, timeCode());
}

//------------------------------------------------------------------------------
// Ufe::RotateUndoableCommand overrides
//------------------------------------------------------------------------------

bool UsdRotateUndoableCommand::rotate(double x, double y, double z)
{
	perform(x, y, z);
	return true;
}

} // namespace ufe
} // namespace MayaUsd
