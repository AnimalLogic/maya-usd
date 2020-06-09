//
// Copyright 2019 Luma Pictures
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
#ifndef MTOH_VIEW_OVERRIDE_H
#define MTOH_VIEW_OVERRIDE_H

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>

#include <maya/MCallbackIdArray.h>
#include <maya/MMessage.h>
#include <maya/MString.h>
#include <maya/MViewport2Renderer.h>

#include <pxr/pxr.h>
#include <pxr/imaging/glf/glew.h>
#include <pxr/base/tf/singleton.h>

#if USD_VERSION_NUM > 2002
#include <pxr/imaging/hd/driver.h>
#endif
#include <pxr/imaging/hd/engine.h>
#include <pxr/imaging/hd/renderIndex.h>
#include <pxr/imaging/hd/rprimCollection.h>
#include <pxr/imaging/hdSt/renderDelegate.h>

#if USD_VERSION_NUM >= 1911
#include <pxr/imaging/hd/rendererPlugin.h>
#else
#include <pxr/imaging/hdx/rendererPlugin.h>
PXR_NAMESPACE_OPEN_SCOPE
using HdRendererPlugin = HdxRendererPlugin;
PXR_NAMESPACE_CLOSE_SCOPE
#endif
#include <pxr/imaging/hdx/taskController.h>

#if WANT_UFE_BUILD
#include <ufe/observer.h>
#endif // WANT_UFE_BUILD

#include <hdMaya/delegates/delegate.h>
#include <hdMaya/delegates/params.h>

#include "defaultLightDelegate.h"
#include "renderGlobals.h"
#include "utils.h"

PXR_NAMESPACE_OPEN_SCOPE

using HgiUniquePtr = std::unique_ptr<class Hgi>;

class MtohRenderOverride : public MHWRender::MRenderOverride {
public:
    MtohRenderOverride(const MtohRendererDescription& desc);
    ~MtohRenderOverride() override;

    static void UpdateRenderGlobals();

    /// The names of all render delegates that are being used by at least
    /// one modelEditor panel.
    static std::vector<MString> AllActiveRendererNames();

    /// Returns a list of rprims in the render index for the given render
    /// delegate.
    ///
    /// Intended mostly for use in debugging and testing.
    static SdfPathVector RendererRprims(
        TfToken rendererName, bool visibleOnly = false);

    /// Returns the scene delegate id for the given render delegate and
    /// scene delegate names.
    ///
    /// Intended mostly for use in debugging and testing.
    static SdfPath RendererSceneDelegateId(
        TfToken rendererName, TfToken sceneDelegateName);

    MStatus Render(const MHWRender::MDrawContext& drawContext);

    void ClearHydraResources();
    void SelectionChanged();

    MString uiName() const override {
        return MString(_rendererDesc.displayName.GetText());
    }

    MHWRender::DrawAPI supportedDrawAPIs() const override;

    MStatus setup(const MString& destination) override;
    MStatus cleanup() override;

    bool startOperationIterator() override;
    MHWRender::MRenderOperation* renderOperation() override;
    bool nextRenderOperation() override;

private:
    typedef std::pair<MString, MCallbackIdArray> PanelCallbacks;
    typedef std::vector<PanelCallbacks> PanelCallbacksList;

    static MtohRenderOverride* _GetByName(TfToken rendererName);

    void _InitHydraResources();
    void _RemovePanel(MString panelName);
    void _SelectionChanged();
    void _DetectMayaDefaultLighting(const MHWRender::MDrawContext& drawContext);
    void _UpdateRenderGlobals();
    void _UpdateRenderDelegateOptions();

    inline PanelCallbacksList::iterator _FindPanelCallbacks(MString panelName) {
        // There should never be that many render panels, so linear iteration
        // should be fine

        return std::find_if(
            _renderPanelCallbacks.begin(), _renderPanelCallbacks.end(),
            [&panelName](const PanelCallbacks& item) {
                return item.first == panelName;
            });
    }

    // Callbacks
    static void _ClearHydraCallback(void* data);
    static void _TimerCallback(float, float, void* data);
    static void _SelectionChangedCallback(void* data);
    static void _PanelDeletedCallback(const MString& panelName, void* data);
    static void _RendererChangedCallback(
        const MString& panelName, const MString& oldRenderer,
        const MString& newRenderer, void* data);
    static void _RenderOverrideChangedCallback(
        const MString& panelName, const MString& oldOverride,
        const MString& newOverride, void* data);

    MtohRendererDescription _rendererDesc;

    std::vector<MHWRender::MRenderOperation*> _operations;
    std::vector<MCallbackId> _callbacks;
    PanelCallbacksList _renderPanelCallbacks;
    MtohRenderGlobals _globals;

    std::mutex _convergenceMutex;
    std::chrono::system_clock::time_point _lastRenderTime;
    std::atomic<bool> _needsClear;

#if USD_VERSION_NUM > 2002
    /// Hgi and HdDriver should be constructed before HdEngine to ensure they
    /// are destructed last. Hgi may be used during engine/delegate destruction.
    HgiUniquePtr _hgi;
    HdDriver _hgiDriver;
#endif
    HdEngine _engine;
    HdRendererPlugin* _rendererPlugin = nullptr;
    HdxTaskController* _taskController = nullptr;
    HdRenderIndex* _renderIndex = nullptr;
    std::unique_ptr<MtohDefaultLightDelegate> _defaultLightDelegate = nullptr;
    HdxSelectionTrackerSharedPtr _selectionTracker;
    HdRprimCollection _renderCollection{
        HdTokens->geometry,
        HdReprSelector(
#if MAYA_APP_VERSION >= 2019
            HdReprTokens->refined
#else
            HdReprTokens->smoothHull
#endif
        ),
        SdfPath::AbsoluteRootPath()};
    HdRprimCollection _selectionCollection{
        HdReprTokens->wire, HdReprSelector(HdReprTokens->wire)
    };
    GlfSimpleLight _defaultLight;

    std::vector<HdMayaDelegatePtr> _delegates;

    SdfPath _ID;

    int _currentOperation = -1;

    const bool _isUsingHdSt = false;
    bool _initializedViewport = false;
    bool _hasDefaultLighting = false;
    bool _renderGlobalsHaveChanged = false;
    bool _selectionChanged = true;
    bool _isConverged = false;

#if WANT_UFE_BUILD
    UFE_NS::Observer::Ptr _ufeSelectionObserver;
#endif // WANT_UFE_BUILD
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // MTOH_VIEW_OVERRIDE_H
