# Locate or checkout a maya-usd source tree
#
# 2 cases to cover:
# A. LOCAL MODE - we're working, building, testing or releasing from a local clone of maya-usd (this script checks it out if it doesn't exist - and uses default branch))h
# B. NONINTERACTIVE_MODE - we're building offline, running tests triggered by Jenkins etc - build whatever tag is specified by the MAYA_USD_GIT_TAG
#
# This code requires the following variables:
#
# MAYA_USD_LOCAL_REPO_DIR           - in the case of Local mode, where does the checked out maya-usd repo live?
#
# This code sets the following variables:
#
#  MAYA_USD_BUILD_DIR           - path to the maya-usd build tree
#  MAYA_USD_SOURCE_DIR          - path to the maya-usd source tree
#  MAYA_USD_AL_PLUGIN_DIR    	- path to the AL Plugin within the maya-usd source tree


cmake_minimum_required(VERSION 3.11)
include(FetchContent)

# Use either the tag set in MAYA_USD_GIT_TAG env var, or default to develop
SET (HAVE_INITIAL_MAYA_USD_TAG 0)
SET (INITIAL_MAYA_USD_TAG "develop")   #default branch
if (NOT ("$ENV{MAYA_USD_GIT_TAG}" STREQUAL ""))
   SET (INITIAL_MAYA_USD_TAG $ENV{MAYA_USD_GIT_TAG})
   SET (HAVE_INITIAL_MAYA_USD_TAG 1)
endif()


set(LOCAL_MODE 1) 
IF(DEFINED ENV{BUILD_URL})
    message("I think I'm in Jenkins")
    set(LOCAL_MODE 0) 
ENDIF()


# Tries to set the branch of maya-usd to the same branch name as maya-usd-build
# if and only if:
# A value has not been supplied for the env var MAYA_USD_GIT_TAG (can be set in the package.py or in the calling cmake)
# The same branch name is not already set
#
# Note that it these 2 conditions are true, an attempt will be made to set the branch - this may fail if a branch of the same name 
# does not exist 
function(set_maya_usd_branch)
    if (NOT ${HAVE_INITIAL_MAYA_USD_TAG})
        execute_process(
          COMMAND git rev-parse --abbrev-ref HEAD
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          OUTPUT_VARIABLE MAYA_USD_GIT_BRANCH
          OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if (NOT ${MAYA_USD_GIT_BRANCH} STREQUAL ${INITIAL_MAYA_USD_TAG})
            MESSAGE("maya-usd-subproject: try and set maya-usd branch to ${MAYA_USD_GIT_BRANCH}")
            execute_process(
              COMMAND git checkout ${MAYA_USD_GIT_BRANCH}
              WORKING_DIRECTORY ${MAYA_USD_LOCAL_REPO_DIR}
            )
        endif()
    endif()
endfunction()



IF (LOCAL_MODE)
	IF (EXISTS ${MAYA_USD_LOCAL_REPO_DIR})
		message("Local Mode: found local maya-usd install. Using maya-usd from ${MAYA_USD_LOCAL_REPO_DIR}")
	ELSE()
		message("Local Mode: didn't found local maya-usd install..let's git clone to ${MAYA_USD_WIP_REPO_DIR}")
		set(FETCHCONTENT_QUIET off)
		file(MAKE_DIRECTORY ${MAYA_USD_LOCAL_REPO_DIR})
		FetchContent_Declare(
		  maya-usd
		  GIT_REPOSITORY 	https://github.al.com.au/rnd/maya-usd.git
		  SOURCE_DIR 		${MAYA_USD_LOCAL_REPO_DIR} #@todo: Only override this if we're in interactive mode
		  GIT_TAG ${INITIAL_MAYA_USD_TAG}
		)
		FetchContent_Populate(maya-usd)
		set_maya_usd_branch()
	ENDIF()
	set (MAYA_USD_SOURCE_DIR ${MAYA_USD_LOCAL_REPO_DIR})
	set (MAYA_USD_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
ELSE()
	message("maya-usd-subproject: Jenkins/CI Mode: let's clone repo using tag ${INITIAL_MAYA_USD_TAG}")
	set(FETCHCONTENT_QUIET off)
	FetchContent_Declare(
	  maya-usd
	  GIT_REPOSITORY https://github.al.com.au/rnd/maya-usd.git
	  GIT_TAG ${INITIAL_MAYA_USD_TAG} 
	)
	FetchContent_Populate(maya-usd)
	set (MAYA_USD_SOURCE_DIR ${maya-usd_SOURCE_DIR})
	set (MAYA_USD_BUILD_DIR ${maya-usd_BINARY_DIR})
	#@todo having to set this feels wrong... The CMAKE_BINARY_DIR is being set to the root of the build tree, not to what's needed for the sub-build
	set(INSTALL_DIR_SUFFIX _deps/maya-usd-build)
	set_maya_usd_branch()
ENDIF() 

SET(MAYA_USD_AL_PLUGIN_DIR "${MAYA_USD_SOURCE_DIR}/plugin/al")

MESSAGE("maya-usd-subproject: MAYA_USD_BUILD_DIR is ${MAYA_USD_BUILD_DIR}")
MESSAGE("maya-usd-subproject: MAYA_USD_SOURCE_DIR is ${MAYA_USD_SOURCE_DIR}")
MESSAGE("maya-usd-subproject: MAYA_USD_AL_PLUGIN_DIR is ${MAYA_USD_AL_PLUGIN_DIR}")
