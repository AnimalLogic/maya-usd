//
// Copyright 2018 Luma Pictures
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http:#www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include <hdmaya/delegates/sceneDelegate.h>

#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/tf/type.h>

#include <pxr/usd/sdf/assetPath.h>
#include <pxr/usd/usdGeom/tokens.h>

#include <pxr/imaging/hd/camera.h>
#include <pxr/imaging/hd/material.h>
#include <pxr/imaging/hd/mesh.h>
#include <pxr/imaging/hd/rprim.h>
#include <pxr/imaging/hd/tokens.h>

#include <pxr/imaging/hdx/renderSetupTask.h>
#include <pxr/imaging/hdx/renderTask.h>
#include <pxr/imaging/hdx/tokens.h>

#include <maya/MDGMessage.h>
#include <maya/MDagPath.h>
#include <maya/MItDag.h>
#include <maya/MString.h>

#include <hdmaya/adapters/adapterRegistry.h>
#include <hdmaya/delegates/delegateDebugCodes.h>
#include <hdmaya/delegates/delegateRegistry.h>
#include <hdmaya/utils.h>

PXR_NAMESPACE_OPEN_SCOPE

namespace {

void _nodeAdded(MObject& obj, void* clientData) {
    auto* delegate = reinterpret_cast<HdMayaSceneDelegate*>(clientData);
    MDagPath dag;
    MStatus status = MDagPath::getAPathTo(obj, dag);
    if (status) { delegate->InsertDag(dag); }
}

template <typename T>
inline void _FindAdapter(const SdfPath&, const std::function<void(T*)>&) {
    // Do nothing.
}

template <typename T, typename M0, typename... M>
inline void _FindAdapter(
    const SdfPath& id, const std::function<void(T*)>& f, const M0& m0,
    const M&... m) {
    auto* adapterPtr = TfMapLookupPtr(m0, id);
    if (adapterPtr == nullptr) {
        _FindAdapter<T>(id, f, m...);
    } else {
        f(static_cast<T*>(adapterPtr->get()));
    }
}

// This will be nicer to use with automatic parameter deduction for lambdas in
// C++14.
template <typename T, typename R>
inline R _GetValue(const SdfPath&, const std::function<R(T*)>&) {
    return {};
}

template <typename T, typename R, typename M0, typename... M>
inline R _GetValue(
    const SdfPath& id, const std::function<R(T*)>& f, const M0& m0,
    const M&... m) {
    auto* adapterPtr = TfMapLookupPtr(m0, id);
    if (adapterPtr == nullptr) {
        return _GetValue<T, R>(id, f, m...);
    } else {
        return f(static_cast<T*>(adapterPtr->get()));
    }
}

template <typename T>
inline void _MapAdapter(const std::function<void(T*)>&) {
    // Do nothing.
}

template <typename T, typename M0, typename... M>
inline void _MapAdapter(
    const std::function<void(T*)>& f, const M0& m0, const M&... m) {
    for (auto& it : m0) { f(static_cast<T*>(it.second.get())); }
    _MapAdapter<T>(f, m...);
}

} // namespace

TF_DEFINE_PRIVATE_TOKENS(
    _tokens,
    (HdMayaSceneDelegate)((FallbackMaterial, "__fallback_material__")));

TF_REGISTRY_FUNCTION(TfType) {
    TfType::Define<HdMayaSceneDelegate, TfType::Bases<HdMayaDelegate> >();
}

TF_REGISTRY_FUNCTION_WITH_TAG(HdMayaDelegateRegistry, HdMayaSceneDelegate) {
    HdMayaDelegateRegistry::RegisterDelegate(
        _tokens->HdMayaSceneDelegate,
        [](HdRenderIndex* parentIndex, const SdfPath& id) -> HdMayaDelegatePtr {
            return std::static_pointer_cast<HdMayaDelegate>(
                std::make_shared<HdMayaSceneDelegate>(parentIndex, id));
        });
}

HdMayaSceneDelegate::HdMayaSceneDelegate(
    HdRenderIndex* renderIndex, const SdfPath& delegateID)
    : HdMayaDelegateCtx(renderIndex, delegateID),
      _fallbackMaterial(delegateID.AppendChild(_tokens->FallbackMaterial)) {}

HdMayaSceneDelegate::~HdMayaSceneDelegate() {
    for (auto callback : _callbacks) { MMessage::removeCallback(callback); }
    _MapAdapter<HdMayaAdapter>(
        [](HdMayaAdapter* a) { a->RemoveCallbacks(); }, _shapeAdapters,
        _lightAdapters, _materialAdapters);
}

void HdMayaSceneDelegate::Populate() {
    HdMayaAdapterRegistry::LoadAllPlugin();
    auto& renderIndex = GetRenderIndex();
    MItDag dagIt(MItDag::kDepthFirst, MFn::kInvalid);
    dagIt.traverseUnderWorld(true);
    for (; !dagIt.isDone(); dagIt.next()) {
        MDagPath path;
        dagIt.getPath(path);
        InsertDag(path);
    }
    MStatus status;
    auto id =
        MDGMessage::addNodeAddedCallback(_nodeAdded, "dagNode", this, &status);
    if (status) { _callbacks.push_back(id); }

    // Adding fallback material sprim to the render index.
    renderIndex.InsertSprim(
        HdPrimTypeTokens->material, this, _fallbackMaterial);
}

void HdMayaSceneDelegate::RemoveAdapter(const SdfPath& id) {
    // FIXME: Improve this function!
    HdMayaShapeAdapterPtr adapter;
    if (TfMapLookup(_shapeAdapters, id, &adapter) && adapter != nullptr) {
        adapter->RemovePrim();
        adapter->RemoveCallbacks();
        _shapeAdapters.erase(id);
        return;
    }

    HdMayaLightAdapterPtr lightAdapter;
    if (TfMapLookup(_lightAdapters, id, &lightAdapter) &&
        lightAdapter != nullptr) {
        lightAdapter->RemovePrim();
        lightAdapter->RemoveCallbacks();
        _lightAdapters.erase(id);
        return;
    }

    HdMayaMaterialAdapterPtr materialAdapter;
    if (TfMapLookup(_materialAdapters, id, &materialAdapter) &&
        materialAdapter != nullptr) {
        materialAdapter->RemovePrim();
        materialAdapter->RemoveCallbacks();
        _materialAdapters.erase(id);
    }
}

void HdMayaSceneDelegate::InsertDag(const MDagPath& dag) {
    TF_DEBUG(HDMAYA_DELEGATE_INSERTDAG)
        .Msg(
            "HdMayaSceneDelegate::InsertDag::"
            "GetLightsEnabled()=%i\n",
            GetLightsEnabled());
    // We don't care about transforms.
    if (dag.hasFn(MFn::kTransform)) { return; }

    MFnDagNode dagNode(dag);
    if (dagNode.isIntermediateObject()) { return; }

    // FIXME: put this into a function!
    if (dag.hasFn(MFn::kLight)) {
        if (GetLightsEnabled()) {
            TF_DEBUG(HDMAYA_DELEGATE_INSERTDAG)
                .Msg(
                    "HdMayaSceneDelegate::InsertDag::"
                    "found light\n");
            auto adapterCreator =
                HdMayaAdapterRegistry::GetLightAdapterCreator(dag);
            if (adapterCreator == nullptr) { return; }
            const auto id = GetPrimPath(dag);
            if (TfMapLookupPtr(_lightAdapters, id) != nullptr) { return; }
            auto adapter = adapterCreator(this, dag);
            if (adapter == nullptr || !adapter->IsSupported()) { return; }
            adapter->Populate();
            adapter->CreateCallbacks();
            _lightAdapters.insert({id, adapter});
        }
    } else {
        auto adapterCreator =
            HdMayaAdapterRegistry::GetShapeAdapterCreator(dag);
        if (adapterCreator == nullptr) { return; }
        const auto id = GetPrimPath(dag);
        if (TfMapLookupPtr(_shapeAdapters, id) != nullptr) { return; }
        auto adapter = adapterCreator(this, dag);
        if (adapter == nullptr || !adapter->IsSupported()) { return; }
        adapter->Populate();
        adapter->CreateCallbacks();
        _shapeAdapters.insert({id, adapter});
        GetMaterialId(id);
    }
}

void HdMayaSceneDelegate::SetParams(const HdMayaParams& params) {
    const auto& oldParams = GetParams();
    if (oldParams.displaySmoothMeshes != params.displaySmoothMeshes) {
        // I couldn't find any other way to turn this on / off.
        // I can't convert HdRprim to HdMesh easily and no simple way
        // to get the type of the HdRprim from the render index.
        // If we want to allow creating multiple rprims and returning an id
        // to a subtree, we need to use the HasType function and the mark dirty
        // from each adapter.
        _MapAdapter<HdMayaDagAdapter>(
            [](HdMayaDagAdapter* a) {
                if (a->HasType(HdPrimTypeTokens->mesh)) {
                    a->MarkDirty(HdChangeTracker::DirtyTopology);
                }
            },
            _shapeAdapters);
    }
    // We need to trigger rebuilding shaders.
    if (oldParams.textureMemoryPerTexture != params.textureMemoryPerTexture) {
        _MapAdapter<HdMayaMaterialAdapter>(
            [](HdMayaMaterialAdapter* a) {
                a->MarkDirty(HdMaterial::AllDirty);
            },
            _materialAdapters);
    }
    HdMayaDelegate::SetParams(params);
}

void HdMayaSceneDelegate::PopulateSelectedPaths(
    const MSelectionList& mayaSelection, SdfPathVector& selectedSdfPaths) {
    _MapAdapter<HdMayaDagAdapter>(
        [&mayaSelection, &selectedSdfPaths](HdMayaDagAdapter* a) {
            auto dagPath = a->GetDagPath();
            for (; dagPath.length(); dagPath.pop()) {
                if (mayaSelection.hasItem(dagPath)) {
                    selectedSdfPaths.push_back(a->GetID());
                    return;
                }
            }
        },
        _shapeAdapters);
}

void HdMayaSceneDelegate::PopulateSelectedPaths(
    const MSelectionList& mayaSelection, HdSelection* selection) {
    _MapAdapter<HdMayaDagAdapter>(
        [&mayaSelection, &selection](HdMayaDagAdapter* a) {
            auto dagPath = a->GetDagPath();
            for (; dagPath.length(); dagPath.pop()) {
                if (mayaSelection.hasItem(dagPath)) {
                    a->PopulateSelection(
                        HdSelection::HighlightModeSelect, selection);
                    return;
                }
            }
        },
        _shapeAdapters);
}

HdMeshTopology HdMayaSceneDelegate::GetMeshTopology(const SdfPath& id) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_MESH_TOPOLOGY)
        .Msg("HdMayaSceneDelegate::GetMeshTopology(%s)\n", id.GetText());
    return _GetValue<HdMayaShapeAdapter, HdMeshTopology>(
        id,
        [](HdMayaShapeAdapter* a) -> HdMeshTopology {
            return a->GetMeshTopology();
        },
        _shapeAdapters);
}

GfRange3d HdMayaSceneDelegate::GetExtent(const SdfPath& id) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_EXTENT)
        .Msg("HdMayaSceneDelegate::GetExtent(%s)\n", id.GetText());
    return _GetValue<HdMayaShapeAdapter, GfRange3d>(
        id, [](HdMayaShapeAdapter* a) -> GfRange3d { return a->GetExtent(); },
        _shapeAdapters);
}

GfMatrix4d HdMayaSceneDelegate::GetTransform(const SdfPath& id) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_TRANSFORM)
        .Msg("HdMayaSceneDelegate::GetTransform(%s)\n", id.GetText());
    return _GetValue<HdMayaDagAdapter, GfMatrix4d>(
        id, [](HdMayaDagAdapter* a) -> GfMatrix4d { return a->GetTransform(); },
        _shapeAdapters, _lightAdapters);
}

bool HdMayaSceneDelegate::IsEnabled(const TfToken& option) const {
    TF_DEBUG(HDMAYA_DELEGATE_IS_ENABLED)
        .Msg("HdMayaSceneDelegate::IsEnabled(%s)\n", option.GetText());
    // Maya scene can't be accessed on multiple threads,
    // so I don't think this is safe to enable.
    if (option == HdOptionTokens->parallelRprimSync) { return false; }

    TF_WARN(
        "HdMayaSceneDelegate::IsEnabled(%s) -- Unsupported option.\n",
        option.GetText());
    return false;
}

VtValue HdMayaSceneDelegate::Get(const SdfPath& id, const TfToken& key) {
    TF_DEBUG(HDMAYA_DELEGATE_GET)
        .Msg("HdMayaSceneDelegate::Get(%s, %s)\n", id.GetText(), key.GetText());
    return _GetValue<HdMayaAdapter, VtValue>(
        id, [&key](HdMayaAdapter* a) -> VtValue { return a->Get(key); },
        _shapeAdapters, _lightAdapters, _materialAdapters);
}

HdPrimvarDescriptorVector HdMayaSceneDelegate::GetPrimvarDescriptors(
    const SdfPath& id, HdInterpolation interpolation) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_PRIMVAR_DESCRIPTORS)
        .Msg(
            "HdMayaSceneDelegate::GetPrimvarDescriptors(%s, %i)\n",
            id.GetText(), interpolation);
    return _GetValue<HdMayaShapeAdapter, HdPrimvarDescriptorVector>(
        id,
        [&interpolation](HdMayaShapeAdapter* a) -> HdPrimvarDescriptorVector {
            return a->GetPrimvarDescriptors(interpolation);
        },
        _shapeAdapters);
}

VtValue HdMayaSceneDelegate::GetLightParamValue(
    const SdfPath& id, const TfToken& paramName) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_LIGHT_PARAM_VALUE)
        .Msg(
            "HdMayaSceneDelegate::GetLightParamValue(%s, %s)\n", id.GetText(),
            paramName.GetText());
    return _GetValue<HdMayaLightAdapter, VtValue>(
        id,
        [&paramName](HdMayaLightAdapter* a) -> VtValue {
            return a->GetLightParamValue(paramName);
        },
        _lightAdapters);
}

bool HdMayaSceneDelegate::GetVisible(const SdfPath& id) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_VISIBLE)
        .Msg("HdMayaSceneDelegate::GetVisible(%s)\n", id.GetText());
    return _GetValue<HdMayaDagAdapter, bool>(
        id, [](HdMayaDagAdapter* a) -> bool { return a->GetVisible(); },
        _shapeAdapters, _lightAdapters);
}

bool HdMayaSceneDelegate::GetDoubleSided(const SdfPath& id) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_DOUBLE_SIDED)
        .Msg("HdMayaSceneDelegate::GetDoubleSided(%s)\n", id.GetText());
    return _GetValue<HdMayaShapeAdapter, bool>(
        id, [](HdMayaShapeAdapter* a) -> bool { return a->GetDoubleSided(); },
        _shapeAdapters);
}

HdCullStyle HdMayaSceneDelegate::GetCullStyle(const SdfPath& id) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_CULL_STYLE)
        .Msg("HdMayaSceneDelegate::GetCullStyle(%s)\n", id.GetText());
    return HdCullStyleDontCare;
}

HdDisplayStyle HdMayaSceneDelegate::GetDisplayStyle(const SdfPath& id) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_DISPLAY_STYLE)
        .Msg("HdMayaSceneDelegate::GetDisplayStyle(%s)\n", id.GetText());
    HdDisplayStyle style;
    style.flatShadingEnabled = false;
    style.displacementEnabled = false;
    return style;
}

SdfPath HdMayaSceneDelegate::GetMaterialId(const SdfPath& id) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_MATERIAL_ID)
        .Msg("HdMayaSceneDelegate::GetDoubleSided(%s)\n", id.GetText());
    auto shapeAdapter = TfMapLookupPtr(_shapeAdapters, id);
    if (shapeAdapter == nullptr) { return _fallbackMaterial; }
    auto material = shapeAdapter->get()->GetMaterial();
    if (material == MObject::kNullObj) { return _fallbackMaterial; }
    auto materialId = GetMaterialPath(material);
    if (TfMapLookupPtr(_materialAdapters, materialId) != nullptr) {
        return materialId;
    }

    auto materialCreator =
        HdMayaAdapterRegistry::GetMaterialAdapterCreator(material);
    if (materialCreator == nullptr) { return _fallbackMaterial; }
    auto materialAdapter = materialCreator(materialId, this, material);
    if (materialAdapter == nullptr) { return _fallbackMaterial; }
    materialAdapter->Populate();
    materialAdapter->CreateCallbacks();
    _materialAdapters.insert({materialId, materialAdapter});
    return materialId;
}

std::string HdMayaSceneDelegate::GetSurfaceShaderSource(const SdfPath& id) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_SURFACE_SHADER_SOURCE)
        .Msg("HdMayaSceneDelegate::GetSurfaceShaderSource(%s)\n", id.GetText());
    if (id == _fallbackMaterial) {
        return HdMayaMaterialAdapter::GetPreviewSurfaceSource();
    }
    return _GetValue<HdMayaMaterialAdapter, std::string>(
        id,
        [](HdMayaMaterialAdapter* a) -> std::string {
            return a->GetSurfaceShaderSource();
        },
        _materialAdapters);
}

std::string HdMayaSceneDelegate::GetDisplacementShaderSource(
    const SdfPath& id) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_DISPLACEMENT_SHADER_SOURCE)
        .Msg(
            "HdMayaSceneDelegate::GetDisplacementShaderSource(%s)\n",
            id.GetText());
    if (id == _fallbackMaterial) {
        return HdMayaMaterialAdapter::GetPreviewDisplacementSource();
    }
    return _GetValue<HdMayaMaterialAdapter, std::string>(
        id,
        [](HdMayaMaterialAdapter* a) -> std::string {
            return a->GetDisplacementShaderSource();
        },
        _materialAdapters);
}

VtValue HdMayaSceneDelegate::GetMaterialParamValue(
    const SdfPath& id, const TfToken& paramName) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_MATERIAL_PARAM_VALUE)
        .Msg(
            "HdMayaSceneDelegate::GetMaterialParamValue(%s, %s)\n",
            id.GetText(), paramName.GetText());
    if (id == _fallbackMaterial) {
        return HdMayaMaterialAdapter::GetPreviewMaterialParamValue(paramName);
    }
    return _GetValue<HdMayaMaterialAdapter, VtValue>(
        id,
        [&paramName](HdMayaMaterialAdapter* a) -> VtValue {
            return a->GetMaterialParamValue(paramName);
        },
        _materialAdapters);
}

HdMaterialParamVector HdMayaSceneDelegate::GetMaterialParams(
    const SdfPath& id) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_MATERIAL_PARAMS)
        .Msg("HdMayaSceneDelegate::GetMaterialParams(%s)\n", id.GetText());
    if (id == _fallbackMaterial) {
        return HdMayaMaterialAdapter::GetPreviewMaterialParams();
    }
    return _GetValue<HdMayaMaterialAdapter, HdMaterialParamVector>(
        id,
        [](HdMayaMaterialAdapter* a) -> HdMaterialParamVector {
            return a->GetMaterialParams();
        },
        _materialAdapters);
}

VtValue HdMayaSceneDelegate::GetMaterialResource(const SdfPath& id) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_MATERIAL_RESOURCE)
        .Msg("HdMayaSceneDelegate::GetMaterialResource(%s)\n", id.GetText());
    if (id == _fallbackMaterial) {
        return HdMayaMaterialAdapter::GetPreviewMaterialResource(id);
    }
    auto ret = _GetValue<HdMayaMaterialAdapter, VtValue>(
        id,
        [](HdMayaMaterialAdapter* a) -> VtValue {
            return a->GetMaterialResource();
        },
        _materialAdapters);
    return ret.IsEmpty() ? HdMayaMaterialAdapter::GetPreviewMaterialResource(id)
                         : ret;
}

TfTokenVector HdMayaSceneDelegate::GetMaterialPrimvars(const SdfPath& id) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_MATERIAL_PRIMVARS)
        .Msg("HdMayaSceneDelegate::GetMaterialPrimvars(%s)\n", id.GetText());
    return {};
}

HdTextureResource::ID HdMayaSceneDelegate::GetTextureResourceID(
    const SdfPath& textureId) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_TEXTURE_RESOURCE_ID)
        .Msg(
            "HdMayaSceneDelegate::GetTextureResourceID(%s)\n",
            textureId.GetText());
    return _GetValue<HdMayaMaterialAdapter, HdTextureResource::ID>(
        textureId.GetPrimPath(),
        [&textureId](HdMayaMaterialAdapter* a) -> HdTextureResource::ID {
            return a->GetTextureResourceID(textureId.GetNameToken());
        },
        _materialAdapters);
}

HdTextureResourceSharedPtr HdMayaSceneDelegate::GetTextureResource(
    const SdfPath& textureId) {
    TF_DEBUG(HDMAYA_DELEGATE_GET_TEXTURE_RESOURCE)
        .Msg(
            "HdMayaSceneDelegate::GetTextureResource(%s)\n",
            textureId.GetText());
    return _GetValue<HdMayaMaterialAdapter, HdTextureResourceSharedPtr>(
        textureId.GetPrimPath(),
        [&textureId](HdMayaMaterialAdapter* a) -> HdTextureResourceSharedPtr {
            return a->GetTextureResource(textureId.GetNameToken());
        },
        _materialAdapters);
}

PXR_NAMESPACE_CLOSE_SCOPE
