
#include "TransformCache.h"
#include "TransformEvaluator.h"
#include "TransformMaths.h"
#include "SIMD.h"
#include <stack>

namespace MayaUsdUtils {

const TransformCache::_MatrixCache& TransformCache::_CacheTransform(UsdPrim prim, UsdTimeCode timeCode)
{
  // if the time code changes, clear the cache
  if(_timeCode != timeCode)
  {
    _timeCode = timeCode;
    _cache.clear();
  }

  // check to see if path is already in the cache
  SdfPath path = prim.GetPrimPath();
  auto it = _cache.find(path);
  if(it != _cache.end())
    return it->second;

  std::stack<SdfPath> cachePaths;
  // walk up path, adding items we need to cache  
  while(path != SdfPath::AbsoluteRootPath())
  {
    cachePaths.push(path);
    path = path.GetParentPath();
    it = _cache.find(path);
    if(it != _cache.end())
      break;
  }

  // grab world matrix of highest entry in map, or set to identity if no parent exists 
  _MatrixCache entry;
  if(it != _cache.end())
  {
    entry.ws = it->second.ws;
  }
  else
  {
    entry.ws.SetIdentity();
  }

  auto stage = prim.GetStage();

  // now walk back down the paths, creating cache entries for each item
  while(!cachePaths.empty())
  {
    // grab first item from stack
    SdfPath primPath = cachePaths.top();
    cachePaths.pop();

    // grab prim
    auto primAtPath = stage->GetPrimAtPath(primPath);

    // run a quick check to ensure compatible with xform, if not, don't cache
    UsdGeomXformable xform(primAtPath);
    if(!xform)
      continue;

    // compute local matrix for op
    bool reset;
    auto ops = xform.GetOrderedXformOps(&reset);
    entry.ls = TransformEvaluator::EvaluateCoordinateFrameForIndex(ops, uint32_t(ops.size()), timeCode);

    // 
    if(!reset)
    {
      multiply4x4((d256*)&entry.ws, (const d256*)&entry.ls, (const d256*)&entry.ws); 
    }
    else
    {
      entry.ws = entry.ls;
    }
    entry.ils = entry.ls.GetInverse();
    entry.iws = entry.ws.GetInverse();
    it = _cache.emplace(primPath, entry).first;
  }

  // check for condition where the path is not transformable.
  if(it == _cache.end())
    throw std::runtime_error(std::string("Error, unable to locate any transformable prims it the path provided: ") + path.GetString());

  // the iterator should now be pointing to the last valid xformable prim we found
  return it->second;
}

/// get the current timecode this cache is caching 
UsdTimeCode TransformCache::CurrentTime() const
{
  return _timeCode;
}

/// returns how many prim paths are in the cache
size_t TransformCache::GetNumEntries() const
{
  return _cache.size();
}

} // MayaUsdUtils
