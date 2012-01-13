
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(CAPClient)

SET(CAPCLIENT_VERSION_MAJOR 2)
SET(CAPCLIENT_VERSION_MINOR 0)
SET(CAPCLIENT_VERSION_PATCH 0)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
INCLUDE(Macros)

# Set up some variables that control the build.  The following variables also appear in 
# a CMake gui application.
CACHE_VAR_WITH_DEFAULT(CAP_INSTALL_PREFIX "/usr/local/" PATH "Install path prefix, prepended onto install directories.")
SET(CMAKE_INSTALL_PREFIX ${CAP_INSTALL_PREFIX} CACHE INTERNAL "Internalise CMAKE_INSTALL_PREFIX, manipulate via CAP_INSTALL_PREFIX." FORCE)
MARK_AS_ADVANCED(CAP_INSTALL_PREFIX)

CACHE_VAR_WITH_DEFAULT(CAP_BUILD_TYPE "Debug" STRING "Choose the type of build, options are: Debug, Release, MinSizeRel, RelWithDebugInfo.")
SET(CMAKE_BUILD_TYPE ${CAP_BUILD_TYPE} CACHE INTERNAL "Internalise CMAKE_BUILD_TYPE, manipulate via CAP_BUILD_TYPE." FORCE)

CACHE_VAR_WITH_DEFAULT(CAP_GTEST_ROOT "/usr/local" PATH "Root directory of the gtest libraries and header files.")
CACHE_VAR_WITH_DEFAULT(CAP_CMGUI_CONFIG_DIR "/usr/local/lib/cmake" PATH "The directory that contains the cmgui-config.cmake file.")

OPTION_WITH_DEFAULT(CAP_ENABLE_UNIT_TESTS "Enable CAPClient Unit Tests." OFF)
OPTION_WITH_DEFAULT(CAP_ENABLE_GUI_UNIT_TESTS "Enable CAPClient GUI Unit Tests." OFF)
OPTION_WITH_DEFAULT(CAP_PRINT_SUMMARY "Print a configuration summary." ON)

# wxWidgets 2.8.10 can only be built in 32-bit mode on Mac
# Cmgui uses wxWidgets 2.8.10 so that too is only built in 
# 32-bit mode.  Therefore force the CAP Client to also be 
# built in 32-bit mode on Mac.
IF( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
	SET( CMAKE_OSX_ARCHITECTURES i386)
ENDIF( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )

SET(CAP_CLIENT_EXECUTABLE capclient)

# Read in the source, header and gui files
SET( APP_HDRS
	src/cmgui/extensions.h
	src/cmgui/utilities.h
	src/AnnotationEditor.h
	src/AnnotationWindow.h
	src/Archiver.h
	src/io/annotationfile.h
	src/math/basis.h
	src/CAPBinaryVolumeParameterDialog.h
	src/CAPContour.h
	src/ui/euladialog.h
	src/ui/htmlwindow.h
	src/material.h
	src/math/algebra.h
	src/model/heart.h
	src/model/modeller.h
	src/model/imodeller.h
	src/model/modellingmode.h
	src/model/modellingpoint.h
	src/math/timesmoother.h
	src/math/totalleastsquares.h
	src/io/modelfile.h
	src/io/xmlfilehandler.h
	src/cmgui/callbacks.h
	src/cmgui/sceneviewerpanel.h
	src/capclientconfig.h
	src/utils/debug.h
	src/dicomimage.h
	src/logmsg.h
	src/utils/filesystem.h
	src/utils/misc.h
	src/math/gmmfactory.h
	src/IAnnotationWindow.h
	src/imagebrowserwindow.h
	src/iimagebrowser.h
	src/imagebrowser.h
	src/IsoSurfaceCapture.h
	src/labelledslice.h
	src/capclient.h
	src/logwindow.h
	src/capclientwindow.h
	src/PlatformInfo.h
	src/math/solverlibraryfactory.h
	src/standardheartdefinitions.h
	src/UserCommentDialog.h
	src/math/vnlfactory.h
	src/VolumeGraph.h
	src/capclientapp.h
	src/textureslice.h
	)

SET( APP_SRCS
	src/cmgui/extensions.cpp
	src/cmgui/utilities.cpp
	src/AnnotationEditor.cpp
	src/AnnotationWindow.cpp
	src/capclientapp.cpp
	src/io/annotationfile.cpp
	src/CAPContour.cpp
	src/ui/htmlwindow.cpp
	src/material.cpp
	src/model/heart.cpp
	src/model/modeller.cpp
	src/model/modellingmode.cpp
	src/model/modellingpoint.cpp
	src/math/timesmoother.cpp
	src/io/modelfile.cpp
	src/io/xmlfilehandler.cpp
	src/logmsg.cpp
	src/cmgui/callbacks.cpp
	src/cmgui/sceneviewerpanel.cpp
	src/dicomimage.cpp
	src/logwindow.cpp
	src/utils/filesystem.cpp
	src/utils/misc.cpp
	src/imagebrowser.cpp
	src/imagebrowserwindow.cpp
	src/labelledslice.cpp
	src/capclientwindow.cpp
	src/capclientwindowheartmodel.cpp
	src/standardheartdefinitions.cpp
	src/PlatformInfo.cpp
	src/capclient.cpp
	src/textureslice.cpp
	)

IF(MSVC)
	SET( APP_SRCS ${APP_SRCS} src/win32/dirent.c src/win32/linuxutils.c src/win32/capclient.rc)
	SET( APP_HDRS ${APP_HDRS} src/win32/dirent.h src/win32/linuxutils.h)
ENDIF()

SET( APP_XRC
	src/ui/AnnotationWindowUI.xrc
	src/ui/ImageBrowserWindowUI.xrc
	src/ui/UserCommentDialogUI.xrc
	src/ui/CAPClientWindowUI.xrc
	src/ui/logdialogui.xrc
	)

SET(MISSING_DEPENDENCY)
# Find the Cmgui libraries, using a gui the CAP_CMGUI_CONFIG_DIR
# variable can be set.  Alternatively it can be set on the 
# command line when creating the CMake cache.
SET(Cmgui_DIR ${CAP_CMGUI_CONFIG_DIR})
FIND_PACKAGE(Cmgui)
IF(NOT CMGUI_FOUND)
	LIST(APPEND MISSING_DEPENDENCY 0)
	LIST(LENGTH MISSING_DEPENDENCY COUNT)
	SET(CAP_ERROR_MESSAGE "${CAP_ERROR_MESSAGE}\n\t${COUNT}. Could not find cmgui-config.cmake.")
ENDIF()
# Need to find boost headers
FIND_PATH(CAP_BOOST_INCLUDE_DIR boost/shared_ptr.hpp)
IF(NOT EXISTS ${CAP_BOOST_INCLUDE_DIR}/boost/shared_ptr.hpp)
	LIST(APPEND MISSING_DEPENDENCY 0)
	LIST(LENGTH MISSING_DEPENDENCY COUNT)
	SET(CAP_ERROR_MESSAGE "${CAP_ERROR_MESSAGE}\n\t${COUNT}. Could not find boost/shared_ptr.hpp.")
ENDIF()
# CAP Client needs the gmm header files if they are not on the
# system include path it needs to be set independently.
FIND_PATH(CAP_GMM_INCLUDE_DIR gmm/gmm.h)
IF(NOT EXISTS ${CAP_GMM_INCLUDE_DIR}/gmm/gmm.h)
	LIST(APPEND MISSING_DEPENDENCY 0)
	LIST(LENGTH MISSING_DEPENDENCY COUNT)
	SET(CAP_ERROR_MESSAGE "${CAP_ERROR_MESSAGE}\n\t${COUNT}. Could not find gmm/gmm.h.")
ENDIF()
# If we are testing find GTest also
IF(CAP_ENABLE_UNIT_TESTS OR CAP_ENABLE_GUI_UNIT_TESTS)
	SET(GTEST_ROOT ${CAP_GTEST_ROOT})
	FIND_PACKAGE(GTest)
	IF(NOT GTEST_FOUND)
		LIST(APPEND MISSING_DEPENDENCY 0)
		LIST(LENGTH MISSING_DEPENDENCY COUNT)
		SET(CAP_ERROR_MESSAGE "${CAP_ERROR_MESSAGE}\n\t${COUNT}. Could not find gtest.")
	ENDIF()
ENDIF()


LIST(LENGTH MISSING_DEPENDENCY COUNT)
IF(${COUNT} GREATER 0)
	MESSAGE(FATAL_ERROR "\n${COUNT} configuration errors found:${CAP_ERROR_MESSAGE}")
ENDIF()
SET(BOOST_INCLUDE_DIRS "${CAP_BOOST_INCLUDE_DIR}")
SET(GMM_INCLUDE_DIRS "${CAP_GMM_INCLUDE_DIR}")

# Set some extra compiler flags if gcc is detected
IF( CMAKE_COMPILER_IS_GNUCXX)
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wextra -Wall")
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wextra -Wall")
ENDIF( CMAKE_COMPILER_IS_GNUCXX)

# Convert the xrc files into .cpp and .h files in the
# current build directory tree.
XRC_TO_CPP("${CMAKE_CURRENT_BINARY_DIR}/src/ui" "${APP_XRC}" UI_SRCS_AND_HDRS)

SET(CAPCLIENT_VERSION_STRING ${CAPCLIENT_VERSION_MAJOR}.${CAPCLIENT_VERSION_MINOR}.${CAPCLIENT_VERSION_PATCH})
# Configure capclientconfig header file.
CONFIGURE_FILE(src/capclientconfig.h.cmake src/capclientconfig.h)
CONFIGURE_FILE(src/html/aboutcapclient.html.cmake src/aboutcapclient.html)

# Create an executable to convert text files into c strings
# in a header file.
SET(TXT2INCLUDE_TARGET txt2include)
ADD_EXECUTABLE(${TXT2INCLUDE_TARGET} src/utils/txt2include.c)
SET(TXT_FILES
	src/templates/vert.prog
	src/templates/frag.prog
	src/templates/globalsmoothtvmatrix.dat
	src/templates/timevaryingprior.dat
	src/templates/globalsmoothperframematrix.dat
	src/templates/globalmapbeziertohermite.dat
	src/templates/globalhermiteparam.exelem
	src/templates/heartmodel.exnode
	src/templates/prior.dat
	src/images/mri_icon.png
	src/images/abi_icon.png
	)
	
SET(TXT_HDRS)
SET(TEXT_HDR_PATH_PREFIX "src/hexified")
FOREACH(TXT_FILE ${TXT_FILES})
	HEXIFY_FILE(${TXT2INCLUDE_TARGET} "${TEXT_HDR_PATH_PREFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/${TXT_FILE}")
	GET_FILENAME_COMPONENT(TXT_FILE_NAME ${TXT_FILE} NAME)
	SET(TXT_FILE_HDR "${TEXT_HDR_PATH_PREFIX}/${TXT_FILE_NAME}.h")
	LIST(APPEND TXT_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${TXT_FILE_HDR}")
ENDFOREACH()

# special case for our configured html page because it is in the build tree
HEXIFY_FILE( ${TXT2INCLUDE_TARGET} "${TEXT_HDR_PATH_PREFIX}" "${CMAKE_CURRENT_BINARY_DIR}/src/aboutcapclient.html")
SET(ABOUTCAPCLIENT_HDR "${TEXT_HDR_PATH_PREFIX}/aboutcapclient.html.h")
LIST(APPEND TXT_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${ABOUTCAPCLIENT_HDR}")

IF(MSVC)
	# The _SCL_SECURE_NO_DEPRECATE define is required for gmm/gmm.h version 4.1.
	# The _CRT_SECURE_NO_WARNINGS define is required to stop visual studio from being annoying.
	SET(PLATFORM_DEFS _SCL_SECURE_NO_DEPRECATE _CRT_SECURE_NO_WARNINGS _CRTDBG_MAP_ALLOC)
ENDIF()

IF(MSVC_IDE)
	# I want to group the source files by the directory layout for Visual Studio
	GROUP_SOURCE_FILES(${APP_SRCS} ${APP_HDRS} ${UI_SRCS_AND_HDRS} ${TXT_HDRS})
ENDIF()

# Must set the values defined in ZINC_DEFINITIONS
FOREACH(DEF ${CMGUI_DEFINITIONS} ${PLATFORM_DEFS})
	ADD_DEFINITIONS(-D${DEF})
ENDFOREACH()

# Set the include directories, Cmgui does not know that it is dependent on gdcm??? 
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/capclient ${CMGUI_INCLUDE_DIRS} ${GDCM_INCLUDE_DIRS} ${GMM_INCLUDE_DIRS} ${BOOST_INCLUDE_DIRS})# not needed when using GDCM_USE_FILE, urrgh.
IF(MSVC)
	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src/win32)
ENDIF()
# Set the CAP Client executable with all sources and headers.  Set headers here also so 
# that some IDEs recognise them and include them in their projects.
ADD_EXECUTABLE(${CAP_CLIENT_EXECUTABLE} WIN32 ${APP_SRCS} ${APP_HDRS} ${UI_SRCS_AND_HDRS} ${TXT_HDRS})

# Set some target properties, helpful to visually determine properties of a build.
SET_TARGET_PROPERTIES(${CAP_CLIENT_EXECUTABLE} PROPERTIES DEBUG_POSTFIX "d" MINSIZEREL_POSTFIX "msr" RELWITHDEBUGINFO_POSTFIX "rwdi" COMPILE_DEFINITIONS_RELEASE CAP_CLIENT_RELEASE_BUILD)

# and for each of your dependant executable/library targets:
TARGET_LINK_LIBRARIES(${CAP_CLIENT_EXECUTABLE} ${CMGUI_LIBRARIES})

# Unit Testing
IF(CAP_ENABLE_UNIT_TESTS)
	SET(CAPCLIENT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
	SET(CAPCLIENT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
	ADD_SUBDIRECTORY(test)
ENDIF()

IF(CAP_PRINT_SUMMARY)
	MESSAGE(STATUS "")
	MESSAGE(STATUS "======== CAP Client Configuration Summary ========")
	MESSAGE(STATUS "CMAKE_SYSTEM_NAME is '${CMAKE_SYSTEM_NAME}'")
	MESSAGE(STATUS "CMAKE_BUILD_TYPE is '${CMAKE_BUILD_TYPE}'")
	MESSAGE(STATUS "Boost include dir: ${CAP_BOOST_INCLUDE_DIR}")
	MESSAGE(STATUS "GMM++ include dir: ${CAP_GMM_INCLUDE_DIR}")
	IF(CAP_ENABLE_UNIT_TESTS OR CAP_ENABLE_GUI_UNIT_TESTS)
		MESSAGE(STATUS "GTest include dir: ${GTEST_INCLUDE_DIRS}")
	ENDIF()
	MESSAGE(STATUS "Enable unit tests: ${CAP_ENABLE_UNIT_TESTS}")
	MESSAGE(STATUS "Enable gui tests: ${CAP_ENABLE_GUI_UNIT_TESTS}")
	MESSAGE(STATUS "==================================================")
	MESSAGE(STATUS "")
ENDIF()
