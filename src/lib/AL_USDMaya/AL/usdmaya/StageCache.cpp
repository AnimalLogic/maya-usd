//
// Copyright 2017 Animal Logic
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.//
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
#include "AL/maya/Common.h"
#include "AL/usdmaya/StageCache.h"
#include "AL/usdmaya/DebugCodes.h"
#include "AL/usdmaya/EventHandler.h"

#include "maya/MGlobal.h"

#include "pxr/usd/usd/stage.h"

namespace AL {
namespace usdmaya {

CallbackId StageCache::g_beforeNewCallbackId = 0;
CallbackId StageCache::g_beforeLoadCallbackId = 0;
static EventId g_stageCacheCleared = 0;

//----------------------------------------------------------------------------------------------------------------------
static void onMayaSceneUpdateCallback(void* clientData)
{
  TF_DEBUG(ALUSDMAYA_TRANSLATORS).Msg("Clean the usdMaya cache on maya scene update.\n");
  StageCache::Clear();
}

//----------------------------------------------------------------------------------------------------------------------
UsdStageCache& StageCache::Get(bool forcePopulate)
{
  static UsdStageCache theCacheForcePopulate;
  static UsdStageCache theCache;

  // IMPORTANT: At every NEW scene in Maya we clear the USD stage cache.
  if (g_beforeNewCallbackId == 0)
  {
    g_beforeNewCallbackId = MayaEventManager::instance().registerCallback(
        onMayaSceneUpdateCallback,
        "BeforeNew",
        "ClearStageCacheOnFileNew",
        0x10000);
    g_beforeLoadCallbackId = MayaEventManager::instance().registerCallback(
        onMayaSceneUpdateCallback,
        "BeforeOpen",
        "ClearStageCacheOnFileOpen",
        0x10000);

    g_stageCacheCleared = EventScheduler::getScheduler().registerEvent("OnUsdStageCacheCleared", kUSDMayaEventType);
  }

  return forcePopulate ? theCacheForcePopulate : theCache;
}

//----------------------------------------------------------------------------------------------------------------------
void StageCache::Clear()
{
  StageCache::Get(true).Clear();
  StageCache::Get(false).Clear();
  EventScheduler::getScheduler().triggerEvent(g_stageCacheCleared);
}

//----------------------------------------------------------------------------------------------------------------------
void StageCache::removeCallbacks()
{
  if (g_stageCacheCleared)
  {
    EventScheduler::getScheduler().unregisterEvent(g_stageCacheCleared);
    g_stageCacheCleared = 0;
  }
  if (g_beforeNewCallbackId)
  {
    MayaEventManager::instance().unregisterCallback(g_beforeNewCallbackId);
    g_beforeNewCallbackId = 0;
  }
  if (g_beforeLoadCallbackId)
  {
    MayaEventManager::instance().unregisterCallback(g_beforeLoadCallbackId);
    g_beforeLoadCallbackId = 0;
  }
}
//----------------------------------------------------------------------------------------------------------------------
} // usdmaya
} // AL
//----------------------------------------------------------------------------------------------------------------------

