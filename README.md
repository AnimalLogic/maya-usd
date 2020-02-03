# maya-usd-build

This package contains the necessary files required to build, test and release the maya-usd project, specifically the Animal fork of it https://github.al.com.au/rnd/maya-usd/

Contents:
+ rez package files
+ cmake files
+ docker setup
+ jenkins setup

We layer our rez build system on top of the opensource CMake infrastructure, and call it. A lot of our CMake/build infrastructure is just translating between data that rez exposes, and information that the open source build needs.

There are a few special things, such as:

+ To avoid having a dependency on AL_USDMaya when working with USD files, some of the schema metadata defined here is filtered out of the relevant pluginInfo.json file, and is expected to be part of the AL_USDCommonSchemas. See here. AL_USDCommonSchemas is included by our tests, but not when running a standard environment, so please be aware of this when attempting to use functionality which relies on ths metadata (There is an argument for adding this dependency)


## What can we actually build and release?

Currently we build the following as rez packages
+ AL_USDTransaction
+ AL_USDUtils
+ AL_USDMaya plugin

We intend to be able to soon package and release
+ pxrUSDMaya plugin
+ adskUSDMaya plugin

Note that all 3 plugins depend on a common library called "mayaUsd". This is currently released as part of AL_USDMaya but once we have a need to build one of the other plugins, we will release this as a separate library



# Workflows
We attempt to cover 3 workflows

## Working locally
if you're working interactively, the system expects to find a local clone of the maya-usd repo in a sibling directory of maya-usd-build, and will check one out if it doesn't find it. It defaults to the "dev" branch (or whatever the default is) but you can change it to whatever you want

When you build it will build the source files in that sibling maya-usd folder. 
You work with, commit, push and generally manipulate the source files in that local maya-usd directory/git repository as you would normally.

You make changes to the build files in this repository (so a code review of a Production feature might include 2 PRs - one from maya-usd-build and one from maya-usd

## Release
When you rez-release/rez-unleash, we don't work with the local clone, but look for a git tag in your package.py - and then checkout a fresh copy of maya-usd using that tag, and release it.
+ Currently this is partially working
+ We need to extend the release process so it tags the maya-usd repo in the same way it tags the repo we're releasing


## Non-interactive use
For non-interactive use (e.g Jenkins) it should ideally do something to release above - but we need to finesse this workflow

