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
#pragma once

#include <vector>
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/sdf/path.h"

PXR_NAMESPACE_USING_DIRECTIVE

namespace AL {
namespace usdmaya {
namespace nodes {
namespace proxy {

//----------------------------------------------------------------------------------------------------------------------
/// \brief  The prim filter needs to know about some state provided in the proxy shape node. In order to maintain a
///         separation between the filter and the proxy (so that it's easy to test!), this class acts as a bridge between
///         the two.
//----------------------------------------------------------------------------------------------------------------------
struct PrimFilterInterface
{
  /// \brief  Given a path to a prim, this method will return some type information for the prim found at that path,
  ///         which the proxy shape has previously cached (i.e. the old state of the prim prior to a variant switch).
  ///         If the proxy shape is aware of the prim, and the returned info is valid, true will be returned. If the
  ///         proxy shape is unaware of the prim (i.e. a variant switch has created it), then false will be returned.
  /// \param  path the path to the prim we are querying
  /// \return true if the prim is known about, and the info structure contains valid information. False if the prim is
  ///         an unknown type
  virtual TfToken getTypeForPath(const SdfPath& path) = 0;

  /// \brief  for a specific type, this method should return whether it supports update, and if that type requires a
  ///         DAG path to be created.
  /// \param  type the type to query
  /// \param  supportsUpdate returned value that indicates if the type in question can be updated
  /// \param  requiresParent returned value that indicates whether the type in question needs a DAG path to be created
  /// \return returns false if the type is unknown, true otherwise
  virtual bool getTypeInfo(TfToken type, bool& supportsUpdate, bool& requiresParent) = 0;
};

//----------------------------------------------------------------------------------------------------------------------
/// \brief  A class to filter the prims during a variant switch
//----------------------------------------------------------------------------------------------------------------------
class PrimFilter
{
public:

  /// \brief  constructor constructs the prim filter
  /// \param  previousPrims the previous set of prims that existed in the stage
  /// \param  newPrimSet the new set of prims that have been created
  /// \param  proxy the proxy shape
  PrimFilter(const SdfPathVector& previousPrims, const std::vector<UsdPrim>& newPrimSet, PrimFilterInterface* proxy);

  /// \brief  returns the set of prims to create
  inline const std::vector<UsdPrim>& newPrimSet() const
    { return m_newPrimSet; }

  /// \brief  returns the set of prims that require created transforms
  inline const std::vector<UsdPrim>& transformsToCreate() const
    { return m_transformsToCreate; }

  /// \brief  returns the list of prims that needs to be updated
  inline const std::vector<UsdPrim>& updatablePrimSet() const
    { return m_updatablePrimSet; }

  /// \brief  returns the list of prims that have been removed from the stage
  inline const SdfPathVector& removedPrimSet() const
    { return m_removedPrimSet; }

private:
  std::vector<UsdPrim> m_newPrimSet;
  std::vector<UsdPrim> m_transformsToCreate;
  std::vector<UsdPrim> m_updatablePrimSet;
  SdfPathVector m_removedPrimSet;
};

//----------------------------------------------------------------------------------------------------------------------
} // proxy
} // nodes
} // usdmaya
} // AL
//----------------------------------------------------------------------------------------------------------------------

