
#include "TransformCache.h"
#include "TransformOpTools.h"
#include "SIMD.h"
#include <stack>

namespace MayaUsdUtils {

namespace {

// rotate an offset vector by the coordinate frame
inline d256 transform4d(const d256 offset, const d256 frame[4])
{
  const d256 xxx = permute4d<0, 0, 0, 0>(offset);
  const d256 yyy = permute4d<1, 1, 1, 1>(offset);
  const d256 zzz = permute4d<2, 2, 2, 2>(offset);
  const d256 www = permute4d<3, 3, 3, 3>(offset);
  return fmadd4d(www, frame[3], fmadd4d(zzz, frame[2], fmadd4d(yyy, frame[1], mul4d(xxx, frame[0]))));
}

// frame *= childTransform
inline void multiply4x4(d256 output[4], const d256 childTransform[4], const d256 parentTransform[4])
{
  const d256 mx = transform4d(childTransform[0], parentTransform);
  const d256 my = transform4d(childTransform[1], parentTransform);
  const d256 mz = transform4d(childTransform[2], parentTransform);
  output[3] = transform4d(childTransform[3], parentTransform);
  output[0] = mx;
  output[1] = my;
  output[2] = mz;
}
}

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
    entry.ls = TransformOpProcessor::EvaluateCoordinateFrameForIndex(ops, uint32_t(ops.size()), timeCode);

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
