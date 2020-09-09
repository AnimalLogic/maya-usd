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
#ifndef MAYAUSDUTILS_TRANSFOM_CACHE_H_
#define MAYAUSDUTILS_TRANSFOM_CACHE_H_

#include "Api.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/tf/token.h>

#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/xformable.h>
#include <immintrin.h>
#include <map>

PXR_NAMESPACE_USING_DIRECTIVE

namespace MayaUsdUtils {

//----------------------------------------------------------------------------------------------------------------------
/// A class that uses the slightly more efficient TransformEvaluator matrix evaluation to create an xform cache. 
/// There is no validation performed by this class to see if the prims are from the same stage, so beware!
//----------------------------------------------------------------------------------------------------------------------
class TransformCache
{
public:

  MAYA_USD_UTILS_PUBLIC
  TransformCache() = default;

  MAYA_USD_UTILS_PUBLIC
  ~TransformCache() = default;

  MAYA_USD_UTILS_PUBLIC
  TransformCache(const TransformCache& tc) = default;

  // return the local space transform for the prim. 
  // If the prim is not xformable (or one of the prims parents is not transformable either), then an exception is thrown
  inline
  const GfMatrix4d& Local(UsdPrim prim, UsdTimeCode timeCode = UsdTimeCode::EarliestTime());
  
  // return the world space transform for the prim. 
  // If the prim is not xformable (or one of the prims parents is not transformable either), then an exception is thrown
  inline
  const GfMatrix4d& World(UsdPrim prim, UsdTimeCode timeCode = UsdTimeCode::EarliestTime());
  
  // return the inverse local space transform for the prim. 
  // If the prim is not xformable (or one of the prims parents is not transformable either), then an exception is thrown
  inline
  const GfMatrix4d& InverseLocal(UsdPrim prim, UsdTimeCode timeCode = UsdTimeCode::EarliestTime());

  // return the inverse world space transform for the prim. 
  // If the prim is not xformable (or one of the prims parents is not transformable either), then an exception is thrown
  inline
  const GfMatrix4d& InverseWorld(UsdPrim prim, UsdTimeCode timeCode = UsdTimeCode::EarliestTime());

  /// get the current timecode this cache is caching 
  MAYA_USD_UTILS_PUBLIC
  UsdTimeCode CurrentTime() const;

  /// returns how many prim paths are in the cache
  MAYA_USD_UTILS_PUBLIC
  size_t GetNumEntries() const;

  //--------------------------------------------------------------------------------------------------------------------
  // Methods for python bindings only
  //--------------------------------------------------------------------------------------------------------------------
  inline
  const GfMatrix4d _Local(UsdPrim prim, UsdTimeCode timeCode = UsdTimeCode::EarliestTime()) { return Local(prim, timeCode); }
  inline
  const GfMatrix4d _World(UsdPrim prim, UsdTimeCode timeCode = UsdTimeCode::EarliestTime()) { return World(prim, timeCode); }
  inline
  const GfMatrix4d _InverseLocal(UsdPrim prim, UsdTimeCode timeCode = UsdTimeCode::EarliestTime()) { return InverseLocal(prim, timeCode); }
  inline
  const GfMatrix4d _InverseWorld(UsdPrim prim, UsdTimeCode timeCode = UsdTimeCode::EarliestTime()) { return InverseWorld(prim, timeCode); }

private:
  UsdTimeCode _timeCode;
  struct _MatrixCache { GfMatrix4d ls; GfMatrix4d ws; GfMatrix4d ils; GfMatrix4d iws; };
  MAYA_USD_UTILS_PUBLIC
  const _MatrixCache& _CacheTransform(UsdPrim prim, UsdTimeCode timeCode);
  std::map<SdfPath, _MatrixCache> _cache;
};

//----------------------------------------------------------------------------------------------------------------------
inline
const GfMatrix4d& TransformCache::Local(UsdPrim prim, UsdTimeCode timeCode)
{
  return _CacheTransform(prim, timeCode).ls;
}

//----------------------------------------------------------------------------------------------------------------------
inline
const GfMatrix4d& TransformCache::World(UsdPrim prim, UsdTimeCode timeCode)
{
  return _CacheTransform(prim, timeCode).ws;
}

//----------------------------------------------------------------------------------------------------------------------
inline
const GfMatrix4d& TransformCache::InverseLocal(UsdPrim prim, UsdTimeCode timeCode)
{
  return _CacheTransform(prim, timeCode).ils;
}

//----------------------------------------------------------------------------------------------------------------------
inline
const GfMatrix4d& TransformCache::InverseWorld(UsdPrim prim, UsdTimeCode timeCode)
{
  return _CacheTransform(prim, timeCode).iws;
}


} // MayaUsdUtils

#endif 
