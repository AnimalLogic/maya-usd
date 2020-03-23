# Intro

This repo is a "meta-package" which contains the necessary files required to build, test and release the maya-usd project inside the Animal Logic ecosystem.

It's set up to work with the Animal fork of maya-usd https://github.al.com.au/rnd/maya-usd/

Contents:
+ rez package files
+ cmake files
+ docker setup
+ jenkins setup

We layer our rez build system on top of the opensource CMake infrastructure...   A lot of our CMake/build infrastructure is just translating between cmake variables/options that rez exposes, and cmake variables/option that the open source build needs.

There are a few specific things in the CMake, such as:
+ We don't rely on cmake "FindPackage" macros for Boost, USD, Maya, Python et al as rez provides us with more precise information
+ To avoid having a dependency on AL_USDMaya when working with USD files, some of the schema metadata defined here is filtered out of the relevant pluginInfo.json file, and is expected to be part of the AL_USDCommonSchemas. AL_USDCommonSchemas is included by our tests, but not when running a standard environment, so please be aware of this when attempting to use functionality which relies on ths metadata (There is an argument for adding this dependency)
+ Rez has it's own install code/logic which we use
+ we run tests, generate documentation etc in Rez and/or AL specific ways


# Important branches
+ master - AL standard release branch
+ develop - AL integration branch (merge to develop when ready)
+ dev - should keep parity with Autodesk https://github.com/autodesk/maya-usd dev branch

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
**Note that the workflows we describe are provisional and will certainly need to be tuned over the next few months!**

## working with the AL Fork of maya-usd (standard dev workflow)
We attempt to cover 3 workflows

### Working locally
if you're working interactively, maya-usd-build expects to find a local clone of the maya-usd repo in a sibling directory of maya-usd-build, and will check one out if it doesn't find it. It defaults to the "dev" branch (or whatever the default is) but you can change it to whatever you want

When you build it will build the source files in that sibling maya-usd folder. 
You work with, commit, push and generally manipulate the source files in that local maya-usd directory/git repository as you would normally.

You make changes to the build files in this repository (so a code review of a Production feature might include 2 PRs - one from maya-usd-build and one from maya-usd

### Releasing
When you rez-release/rez-unleash, we look for the local/sibling  git clone of maya-usd as above, and as of rez 2.47.4.4, thesame validation checks are run on the subrepo as are run on the rez package repo.. i.e:
+ no untracked files
+ no uncommitted files
+ releasing from a release branch etc

Post release, we:
+ tag the sub repo (i.e maya-usd) in the same way we tag the repo we're releasing (i.e git tag "AL_USDMaya-MAJOR.MINOR.PATCH" commitID)
+ add the commit id, and history to the package.py of the package we're releasing


### Non-interactive use
For non-interactive use (e.g Jenkins) we need to think more about how it should work - i.e how do we drive the checkout of a particular commit or branch from the build repo? (e.g when running unit tests on AL)

# Repositories

Some labels for the various repos we interact with when pushing, pulling etc)
1. https://github.com/autodesk/maya-usd **ADSKPublic**
2. https://github.com/AnimalLogic/maya-usd **ALPublic**
3. https://github.al.com.au/rnd/maya-usd **ALInternal**
3. https://github.al.com.au/rnd/maya-usd-build **Build**

Handy snippet for commonly used repos
```
git remote add ALPublic https://github.com/AnimalLogic/maya-usd/
git remote add ADSKPublic https://github.com/Autodesk/maya-usd/
git remote add BlueSkyPublic https://github.com/BlueSkyStudios/maya-usd/
git remote add LumaPublic https://github.com/LumaPictures/maya-usd/
```


# Tests
1. When a PR is opened or updated on  **ALInternal**, Jenkins will run some tests http://hudson:8081/hudson/job/maya-usd/ These tests use docker for a "vanilla" open source build that doesn't rely on any AL specific environment
2. When the dev branch is changed, Jenkins will run the same tests - see http://hudson:8081/hudson/job/maya-usd-dev
3. This repository **Build** should run tests using rez/AL environment when changes are made to the build intrastructure (and probably **ALInternal** - but this is @todo

For more info on the docker tests, see [docker cookbook](https://www.al.com.au:8443/display/~eoinm/Docker+Cookbook+for+maya-usd+tests) for some info. Some of the files used in this process are here https://github.al.com.au/rnd/maya-usd-build/tree/develop/automatedTesting


# Interacting with the Open source repositories

## Pulling updates from ADSKPublic
pull in any changes we want updates ADSKPublic dev branch. This should just be a:
```git pull ADSKPublic dev```

Before pushing the changes to **ALInternal** - make sure the SHAs of the HEAD commits are the same - I saw some cases where they weren't (not sure why) and it caused a bit of merge madness.

Note that before merging these to develop (or some other release branch)  be careful of things like:

+ ABI/API breaking changes
+ changed dependencies on external libraries
+ changes to maya serialisation (Node attributes etc, MpxData etc)
+ Reliance on Updated versions of USD 

**For those reasons, it's probably a good idea to create a PR before merging those changes to develop**


## Pushing PRs to ADSKPublic
Normally we would open a PR against the develop branch on **ALInternal**, go through the code review process etc

Then:
+ update the  PR on **ALInternal** to have the dev branch as it's base ( Make sure the dev branch is up to date (@todo - automate this)) so tests will run, merge conflicts will be detected etc
+ merge into develop for production use/testing/release
+ Tag the branc as "merged_to_develop"

Once some time (a week?) has passed of the feature in production:

+ Make sure the dev branch on **ALPublic** is still up to date (@todo - automate this)
+ Push the branch to **ALPublic** (you can rebase or squash if you want to hide the commit history)
+ Create the PR against **ADSKPublic** dev branch from **ALPublic**
+ Update the table https://www.al.com.au:8443/display/~eoinm/Pull+Requests+AL_USDMaya+to+maya-usd that tracks the state of various PRs in the 3 repos (note, when opening a PR against **ALPublic**, please prefix the PR Title with AL#XXX where XXX is the ID of the internal PR
+ label the PR as below

### Labelling PRs 
For each PR we open in **ALInternal**, we should label as one (and only one) of:
* [pushToADSKPublic](https://github.al.com.au/rnd/maya-usd/pulls?q=is%3Apr+is%3Aopen+label%3AtoPushToADSKPublic)
* [pushedToADSKPublic](https://github.al.com.au/rnd/maya-usd/labels/pushedToADSKPublic)
* [mergedInADSKPublic](https://github.al.com.au/rnd/maya-usd/pulls?q=is%3Apr+is%3Aopen+label%3AmergedInADSKPublic) For these we don't normally need to merge them into our develop branch as we'll get them by updating "dev"

### Tips for PRs on **ADSKPublic**
+ **Description**: Make sure the description is rock solid and explains exactly what you've done and what problem you're solving. if it's extending existing functionality or beaviour - describe that, and how you've improved, extended or modified it. See https://github.com/Autodesk/maya-usd/pull/185 for an example. Reviewers in the open source world don't know anything about Animal Logic internal processes or workflows, so assume minimal context
+ **Title** - prefix  with the "[al]" prefix (it's just a convention, nothing relies on it), and postfix with the AL Internal PR e.g "[al] ColorSet import/export: support USD displayOpacity primvar (#1015)"
+ **Branch Name** - We should probably establish a convention for branch names, but we don't have one so far.. so either use the branchname from **ALInternal**, or if you want to use a different one - do so, but record it in the [Internal->External PR table](https://www.al.com.au:8443/display/~eoinm/Pull+Requests+AL_USDMaya+to+maya-usd). Note to push to a different named branch, just separate original and remote with a colon e.g "git push ALPublic displayOpacity1:AL1015-support_USD_displayOpacity"
+ Make any dependencies/duplications on other PRs explicit (i.e mention in the description) so they know in which order to merge
+ As usual make sure you have tests, good comments etc
+ There's a WIP coding standard/style guide [here](https://docs.google.com/document/d/1Jvbpfh2WNzHxGQtjqctZ1K1lnpaAtHOUwm0kmmEcxjY/edit)
+ Don't add Version.h to the PR - @todo fix this tiresome problem!



# FAQ
## Why are the build files a separate git repo to the source files?
See https://www.al.com.au:8443/display/~eoinm/Options+for+dev+workflow+Transition+from+AL_USDMaya+to+maya-usd for background info

## Where are the docs?
The most up to date docs are here https://github.al.com.au/rnd/AL_USDMaya/blob/develop/docs, and https://github.al.com.au/rnd/maya-usd/tree/master/plugin/al/docs - we should reconcile the 2
