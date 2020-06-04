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

#include "UsdTRSUndoableCommandBase.h"
#include "private/Utils.h"

#include <ufe/scene.h>
#include <ufe/sceneNotification.h>

MAYAUSD_NS_DEF {
namespace ufe {

template<class V>
UsdTRSUndoableCommandBase<V>::UsdTRSUndoableCommandBase(
    const UsdSceneItem::Ptr& item, double x, double y, double z, const UsdTimeCode& timeCode
) : fItem(item), fNewValue(x, y, z), fTimeCode(timeCode)
{
}

template<class V>
void UsdTRSUndoableCommandBase<V>::initialize()
{
    Ufe::Scene::instance().addObjectPathChangeObserver(this->shared_from_this());
}

template<class V>
void UsdTRSUndoableCommandBase<V>::operator()(
    const Ufe::Notification& n
)
{
    if (auto renamed = dynamic_cast<const Ufe::ObjectRename*>(&n)) {
        checkNotification(renamed);
    }
    else if (auto reparented = dynamic_cast<const Ufe::ObjectReparent*>(&n)) {
        checkNotification(reparented);
    }
}

template<class V>
void UsdTRSUndoableCommandBase<V>::undoImp()
{
    perform(fPrevValue[0], fPrevValue[1], fPrevValue[2]);
    // Todo : We would want to remove the xformOp
    // (SD-06/07/2018) Haven't found a clean way to do it - would need to investigate
}

template<class V>
void UsdTRSUndoableCommandBase<V>::redoImp()
{
    perform(fNewValue[0], fNewValue[1], fNewValue[2]);
}

template<class V>
template<class N>
void UsdTRSUndoableCommandBase<V>::checkNotification(const N* notification)
{
    if (notification->previousPath() == path()) {
        fItem = std::dynamic_pointer_cast<UsdSceneItem>(notification->item());
    }
}

template<class V>
void UsdTRSUndoableCommandBase<V>::perform(double x, double y, double z)
{
    fNewValue = V(x, y, z);
    performImp(x, y, z);
    fDoneOnce = true;
}

template<class V>
bool UsdTRSUndoableCommandBase<V>::cannotInit() const
{
    return false;
}

template class UsdTRSUndoableCommandBase<GfVec3f>;
template class UsdTRSUndoableCommandBase<GfVec3d>;

} // namespace ufe
} // namespace MayaUsd
