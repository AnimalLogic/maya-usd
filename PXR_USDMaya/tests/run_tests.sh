#!/bin/bash

export LD_LIBRARY_PATH=$1/third_party/maya/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$1/lib/python:$PYTHONPATH
export PXR_PLUGINPATH_NAME=$1/plugin/usd:$1/lib/usd:$PXR_PLUGINPATH_NAME
export MAYA_PLUG_IN_PATH=$1/third_party/maya/plugin
export MAYA_SCRIPT_PATH=$1/lib:$1/third_party/maya/lib/usd/usdMaya/resources

ctest --output-on-failure
