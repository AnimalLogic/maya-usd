# Locate or checkout a maya-usd source tree
#
# 3 cases to cover:
# A. WIP MODE - we're working, building and testing locally - we work with a local clone of maya-usd (we check it out if it doesn't exist - use default branch))
# B. RELEASE MODE - we're doing a release locally - build whatever tag is specified by the MAYA_USD_GIT_TAG - would normally be either be HEAD of develop or a release branch
# C. NON_INTERACTIVE MODE - we're running tests triggered by Jenkins - build whatever tag is specified by the MAYA_USD_GIT_TAG
#
# This code requires the following variables:
#
# MAYA_USD_WIP_REPO_DIR           - in the case of WIP mode, where does the checked out maya-usd repo live?
#
# This code sets the following variables:
#
#  MAYA_USD_BUILD_DIR           - path to the maya-usd build tree
#  MAYA_USD_SOURCE_DIR          - path to the maya-usd source tree
#  MAYA_USD_AL_PLUGIN_DIR    	- path to the AL Plugin within the maya-usd source tree


cmake_minimum_required(VERSION 3.11)
include(FetchContent)

set(USING_JENKINS 0) 
IF(DEFINED ENV{BUILD_URL})
    message("I think I'm in  Jenkins")
    set(USING_JENKINS 1) 
ENDIF()

set(RELEASING 0) 
if (CENTRAL)
	 set(RELEASING 1) 
ENDIF()

IF (NOT RELEASING AND NOT USING_JENKINS)
	IF (EXISTS ${MAYA_USD_WIP_REPO_DIR})
		message("WIP Mode: found local maya-usd install. Using maya-usd from ${MAYA_USD_WIP_REPO_DIR}")
	ELSE()
		message("WIP Mode: didn't found local maya-usd install..let's git clone to ${MAYA_USD_WIP_REPO_DIR}")
		set(FETCHCONTENT_QUIET off)
		file(MAKE_DIRECTORY ${MAYA_USD_WIP_REPO_DIR})
		FetchContent_Declare(
		  maya-usd
		  GIT_REPOSITORY 	https://github.al.com.au/rnd/maya-usd.git
		  SOURCE_DIR 		${MAYA_USD_WIP_REPO_DIR} #@todo: Only override this if we're in interactive mode
		  GIT_TAG           dev
		)
		FetchContent_Populate(maya-usd)
	ENDIF()
	set (MAYA_USD_SOURCE_DIR ${MAYA_USD_WIP_REPO_DIR})
	set (MAYA_USD_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
ELSEIF (RELEASING)
	message("Release Mode: let's clone the repo")
	set(FETCHCONTENT_QUIET off)
	FetchContent_Declare(
	  maya-usd
	  GIT_REPOSITORY https://github.al.com.au/rnd/maya-usd.git
	  GIT_TAG        master   #@todo we can release from a release branch also
	)
	FetchContent_Populate(maya-usd)
	set (MAYA_USD_SOURCE_DIR ${maya-usd_SOURCE_DIR})
	set (MAYA_USD_BUILD_DIR ${maya-usd_BINARY_DIR})
	#@todo: at release time we should store the commit ID so we can backtrack to the source code 
	#@todo: maybe rez can already handle this via https://github.com/nerdvegas/rez/wiki/Package-Definition-Guide#revision?
ELSEIF (USING_JENKINS)
	#@todo: I dont think this covers all cases when we would be building from the command line... we basically need to cover non-interactive shell cases
	message("Jenkins Mode: let's clone repo using tag $ENV{MAYA_USD_GIT_TAG}")
	set(FETCHCONTENT_QUIET off)
	FetchContent_Declare(
	  maya-usd
	  GIT_REPOSITORY https://github.al.com.au/rnd/maya-usd.git
	  GIT_TAG $ENV{MAYA_USD_GIT_TAG}
	)
	FetchContent_Populate(maya-usd)
	set (MAYA_USD_SOURCE_DIR ${maya-usd_SOURCE_DIR})
	set (MAYA_USD_BUILD_DIR ${maya-usd_BINARY_DIR})
	#@todo having to set this feels wrong... The CMAKE_BINARY_DIR is being set to the root of the build tree, not to what's needed for the sub-build
	set(INSTALL_DIR_SUFFIX _deps/maya-usd-build)
ENDIF() 


SET(MAYA_USD_AL_PLUGIN_DIR "${MAYA_USD_SOURCE_DIR}/plugin/al")

MESSAGE("maya-usd-subproject: MAYA_USD_BUILD_DIR is ${MAYA_USD_BUILD_DIR}")
MESSAGE("maya-usd-subproject: MAYA_USD_SOURCE_DIR is ${MAYA_USD_SOURCE_DIR}")
MESSAGE("maya-usd-subproject: MAYA_USD_AL_PLUGIN_DIR is ${MAYA_USD_AL_PLUGIN_DIR}")
