# Intro

This package contains the necessary files required to build, test and release the maya-usd project inside the Animal Logic ecosystem.
It's set up to work with the Animal fork of maya-usd https://github.al.com.au/rnd/maya-usd/

Contents:
+ rez package files
+ cmake files
+ docker setup
+ jenkins setup

We layer our rez build system on top of the opensource CMake infrastructure...   A lot of our CMake/build infrastructure is just translating between data that rez exposes, and information that the open source build needs.

There are a few specific things, such as:
+ We don't rely on cmake "FindPackage" macros for Boost, USD etc as rez provides us with the same information
+ To avoid having a dependency on AL_USDMaya when working with USD files, some of the schema metadata defined here is filtered out of the relevant pluginInfo.json file, and is expected to be part of the AL_USDCommonSchemas. See here. AL_USDCommonSchemas is included by our tests, but not when running a standard environment, so please be aware of this when attempting to use functionality which relies on ths metadata (There is an argument for adding this dependency)


# Important branches
+ master - AL standard release branch
+ develop - AL integration branch (merge to develop when ready)
+ dev - should keep parity with Autodesk https://github.com/autodesk/maya-usd  dev branch

## What does this repo build and release?

Currently we build the following as rez packages
+ AL_USDTransaction
+ AL_USDUtils
+ AL_USDMaya plugin

We soon intend to be able to rez package and release
+ pxrUSDMaya plugin
+ adskUSDMaya plugin

Note that all 3 plugins depend on a common library called "mayaUsd". This is currently released as part of AL_USDMaya but once we have a need to build one of the other plugins, we will release this as a separate library



# Workflows

## working with the AL Fork of maya-usd (standard dev workflow)
We attempt to cover 3 workflows

### Working locally
if you're working interactively, maya-usd-build expects to find a local clone of the maya-usd repo in a sibling directory of maya-usd-build, and will check one out if it doesn't find it. It defaults to the "dev" branch (or whatever the default is) but you can change it to whatever you want

When you build it will build the source files in that sibling maya-usd folder. 
You work with, commit, push and generally manipulate the source files in that local maya-usd directory/git repository as you would normally.

You make changes to the build files in this repository (so a code review of a Production feature might include 2 PRs - one from maya-usd-build and one from maya-usd

### Releasing
When you rez-release/rez-unleash, we don't work with the local clone, but look for a git tag in your package.py - and then checkout a fresh copy of maya-usd using that tag, and release it.
+ Currently this is partially working
+ We need to extend the release process so it tags the maya-usd repo in the same way it tags the repo we're releasing


### Non-interactive use
For non-interactive use (e.g Jenkins) it should ideally do something to release above - but we need to finesse this workflow

# Tests
1. When any changes are made to github.al.com.au/rnd/maya-usd, Jenkins will run some tests. These tests use docker for a "vanilla" open source build that doesn't rely on any AL specific environment, see  https://www.al.com.au:8443/display/~eoinm/Docker+Cookbok+for+maya-usd+tests for some info. Some of the files used in this process are here https://github.al.com.au/rnd/maya-usd-build/tree/develop/automatedTesting
2. This repository should run tests using AL infrastructure when changes are made to it - this is @todo

# Interacting with the Open source repository

Some labels for the various repos we interact with
1. https://github.com/autodesk/maya-usd **ADSKPublic**
2. https://github.com/AnimalLogic/maya-usd **ALPublic**
3. https://github.al.com.au/rnd/maya-usd **ALInternal**

List of common remotes for consistency (need these when pushing, pulling etc)
```
remote add ALPublic https://github.com/AnimalLogic/maya-usd/
remote add ADSKPublic https://github.com/Autodesk/maya-usd/
remote add BlueSkyPublic https://github.com/BlueSkyStudios/maya-usd/
remote add LumaPublic https://github.com/LumaPictures/maya-usd/
```

## Pulling updates from ADSKPublic
pull in any changes we want updates ADSKPublic dev branch. This should just be a git pull "git pull ADSKPublic dev"

Note that before merging these to develop (or some other release branch)  be careful of things like:

+ ABI/API breaking changes
+ changed dependencies
+ changes to maya serialisation (proxy Node attribute, MpxData etc)
+ Updated versionss of USD 



## Pushing PRs to ADSKPublic
Normally we would:
+ open a PR against **ALInternal**
+ and once approved, push the branch to **ALPublic**
+ Make sure the dev branch on **ALPublic** is up to date (@todo - automate this)
+ Create the PR against **ADSKPublic** dev branch from **ALPublic**
+ Update the table https://www.al.com.au:8443/display/~eoinm/Pull+Requests+AL_USDMaya+to+maya-usd that tracks the state of various PRs in the 3 repos (note, when opening a PR against **ALPublic**, please prefix the PR Title with AL#XXX where XXX is the ID of the internal PR
+ Add the [pushedToADSKPublic](https://github.al.com.au/rnd/maya-usd/labels/pushedToADSKPublic) tag to the **ALInternal** PR

Additional notes: 
+ For each PR we make internally, we should tag as "nopushtoadsk" or "pushtoasdk". We want to keep "nopushtoadsk" to a minimum as these cause merge conflicts


# FAQ
## Why do this as a separate git repo?
See https://www.al.com.au:8443/display/~eoinm/Options+for+dev+workflow+Transition+from+AL_USDMaya+to+maya-usd for background info

## Where are the docs
Autodesk have removed the AL_USDMaya docs from their repo. The most up to date docs are here https://github.al.com.au/rnd/AL_USDMaya/blob/develop/docs. We should probably move those docs to this repo
