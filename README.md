# maya-usd-build

This package contains the:
+ rez
+ cmake
+ docker
+ jenkins 
files required to build the maya-usd project, specifically the Animal fork of it https://github.al.com.au/rnd/maya-usd/

## What can we actually build and release?

Currently we build the following as rez packages
+ AL_USDTransaction
+ AL_USDUtils
+ AL_USDMaya plugin

We intend to be able to soon package and release
+ pxrUSDMaya plugin
+ adskUSDMaya plugin

Note that all 3 plugins depend on a common library called "mayaUsd". This is currently released as part of AL_USDMaya but once we have a need to build one of the other plugins, we will release this as a separate library



## Rules for what gets built
+ if you're working locally, we expect to find a maya-usd git repo in a sibling directory of maya-usd-build.
When you build it will build the source files in the sibling maya-usd folder. You work with, commit, push and generally manipulate the source files in that local maya-usd directory/git repository.
+ When you rez-release/rez-unleash, it will not work with that local folder, but look for a git tag in your package.py - it will then checkout a fresh copy of maya-usd using that tag, and release it
+ For non-interactive use (e.g Jenkins) it should ideally do the same - we need to find a good way of checking for this case (probably the non-existence of the sibling maya-usd directory is not enough
