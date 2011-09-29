
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(CAPClient)

SET(CAPCLIENT_VERSION_MAJOR 2)
SET(CAPCLIENT_VERSION_MINOR 0)
SET(CAPCLIENT_VERSION_PATCH 0)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
INCLUDE(Macros)

DEFINE_ARCHITECTURE_DIR()
# Try and find CMISS_ROOT variable so we can prime the 
# finding of the Cmgui and GDCM libraries.  Otherwise
# they can be set directly or entered when using a gui tool.
IF(CMISS_ROOT)
	SET(Cmgui_DIR "${CMISS_ROOT}/${ARCHITECTURE_DIR}/lib/cmake")
	SET(GDCM_DIR "${CMISS_ROOT}/third_party/${ARCHITECTURE_DIR}/lib/gdcm-2.0")
ELSEIF(EXISTS $ENV{CMISS_ROOT})
	SET(Cmgui_DIR "$ENV{CMISS_ROOT}/${ARCHITECTURE_DIR}/lib/cmake")
	SET(GDCM_DIR "$ENV{CMISS_ROOT}/third_party/${ARCHITECTURE_DIR}/lib/gdcm-2.0")
ENDIF(CMISS_ROOT)
# Set up some variables that control the build.  The following variables also appear in 
# a CMake gui application.
CACHE_VAR_WITH_DEFAULT(CAP_INSTALL_PREFIX "/usr/local/" PATH "Install path prefix, prepended onto install directories.")
SET(CMAKE_INSTALL_PREFIX ${CAP_INSTALL_PREFIX} CACHE INTERNAL "Internalise CMAKE_INSTALL_PREFIX, manipulate via CAP_INSTALL_PREFIX." FORCE)

CACHE_VAR_WITH_DEFAULT( CAP_BUILD_TYPE "Debug" STRING "Choose the type of build, options are: Debug, Release, MinSizeRel, RelWithDebugInfo.")
SET( CMAKE_BUILD_TYPE ${CAP_BUILD_TYPE} CACHE INTERNAL "Internalise CMAKE_BUILD_TYPE, manipulate via CAP_BUILD_TYPE." FORCE)

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
#INCLUDE(source_files.cmake)
SET( APP_SRCS
	src/cmgui/utilities.cpp
	src/AnnotationEditor.cpp
	src/AnnotationWindow.cpp
	src/capclientapp.cpp
	src/CAPAnnotationFile.cpp
	src/CAPContour.cpp
	src/CAPHtmlWindow.cpp
	src/CAPMaterial.cpp
	src/CAPModelLVPS4X4.cpp
	src/CAPModeller.cpp
	src/CAPModellingMode.cpp
	src/CAPTimeSmoother.cpp
	src/CAPXMLFile.cpp
	src/CAPXMLFileHandler.cpp
	src/cmguicallbacks.cpp
	src/CmguiExtensions.cpp
	src/CmguiImageSliceGraphics.cpp
	src/cmguipanel.cpp
	src/DICOMImage.cpp
	src/DataPoint.cpp
	src/FileSystem.cpp
	src/imagebrowser.cpp
	src/imagebrowserwindow.cpp
	src/ImageSet.cpp
	src/ImageSlice.cpp
	src/labelledslice.cpp
	src/labelledtexture.cpp
	src/capclientwindow.cpp
	src/PlatformInfo.cpp
	src/capclient.cpp
	)

#INCLUDE(header_files.cmake)
SET( APP_HDRS
	src/abstractlabelled.h
	src/cmgui/utilities.h
	src/AnnotationEditor.h
	src/AnnotationWindow.h
	src/Archiver.h
	src/CAPAnnotationFile.h
	src/CAPBasis.h
	src/CAPBinaryVolumeParameterDialog.h
	src/CAPContour.h
	src/CAPEulaDialog.h
	src/CAPHtmlWindow.h
	src/CAPMaterial.h
	src/CAPMath.h
	src/CAPModelLVPS4X4.h
	src/CAPModeller.h
	src/CAPModellingMode.h
	src/CAPTimeSmoother.h
	src/CAPTotalLeastSquares.h
	src/CAPXMLFile.h
	src/CAPXMLFileHandler.h
	src/cmguicallbacks.h
	src/CmguiExtensions.h
	src/CmguiImageSliceGraphics.h
	src/cmguipanel.h
	src/Config.h
	src/DICOMImage.h
	src/DataPoint.h
	src/FileSystem.h
	src/GMMFactory.h
	src/IAnnotationWindow.h
	src/imagebrowserwindow.h
	src/iimagebrowser.h
	src/imagebrowser.h
	src/ImageSet.h
	src/ImageSetBuilder.h
	src/ImageSlice.h
	src/ImageSliceGraphics.h
	src/IsoSurfaceCapture.h
	src/labelledslice.h
	src/labelledtexture.h
	src/capclient.h
	src/capclientwindow.h
	src/PlatformInfo.h
	src/SliceInfo.h
	src/SolverLibraryFactory.h
	src/UserCommentDialog.h
	src/VNLFactory.h
	src/VolumeGraph.h
	src/capclientapp.h
	)
#INCLUDE(xrc_files.cmake)
SET( APP_XRC
	src/ui/AnnotationWindowUI.xrc
	src/ui/ImageBrowserWindowUI.xrc
	src/ui/UserCommentDialogUI.xrc
	src/ui/CAPClientWindowUI.xrc
	)

# Find the Cmgui libraries, using a gui the Cmgui_DIR variable
# can be set if needed.  Alternatively it can be set on the 
# command line when creating the CMake cache.
FIND_PACKAGE(Cmgui REQUIRED)
FIND_PACKAGE(GDCM REQUIRED)
#INCLUDE(${GDCM_USE_FILE}) # don't use this it is bad.
SET(GDCM_INCLUDE_DIRS "${GDCM_INSTALL_PREFIX}/include/gdcm-2.0")
# CAP Client needs the gmm header files if they are not on the
# system include path it needs to be set independently.
FIND_PATH(CAP_GMM_INCLUDE_DIR gmm/gmm.h)
IF(NOT CAP_GMM_INCLUDE_DIR)
	MESSAGE(FATAL_ERROR "Could not find required header 'gmm/gmm.h'.")
ENDIF()
SET(GMM_INCLUDE_DIRS "${CAP_GMM_INCLUDE_DIR}")

# Set some extra compiler flags if gcc is detected
IF( CMAKE_COMPILER_IS_GNUCXX)
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wextra -Wall")
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wextra -Wall")
ENDIF( CMAKE_COMPILER_IS_GNUCXX)

# Convert the xrc files into .cpp and .h files in the
# current build directory tree.
XRC_TO_CPP("${APP_XRC}" UI_SRCS_AND_HDRS)

# Create an executable to convert text files into c strings
# in a header file.
ADD_EXECUTABLE(txt2include src/utils/txt2include.c)
GET_TARGET_PROPERTY(TXT2INCLUDE_EXE txt2include LOCATION)
#INCLUDE(txt_files.cmake)
SET(TXT_FILES
	src/templates/vert.prog
	src/templates/frag.prog
	)
SET(TXT_HDRS)
FOREACH(TXT_FILE ${TXT_FILES})
	GET_FILENAME_COMPONENT(TXT_FILE_NAME ${TXT_FILE} NAME)
	SET(TXT_FILE_HDR src/${TXT_FILE_NAME}.h)
	ADD_CUSTOM_COMMAND(OUTPUT ${TXT_FILE_HDR}
		COMMAND cmake -E make_directory src
		COMMAND ${TXT2INCLUDE_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/${TXT_FILE} ${TXT_FILE_HDR}
		DEPENDS txt2include
		MAIN_DEPENDENCY ${TXT_FILES})
	LIST(APPEND TXT_HDRS ${TXT_FILE_HDR})
ENDFOREACH(TXT_FILE ${TXT_FILES})

# Must set the values defined in CMGUI_DEFINITIONS
FOREACH(DEF ${CMGUI_DEFINITIONS})
	ADD_DEFINITIONS(-D${DEF})
ENDFOREACH(DEF ${CMGUI_DEFINITIONS})

# Set the include directories, Cmgui does not know that it is dependent on gdcm??? 
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMGUI_INCLUDE_DIRS} ${GDCM_INCLUDE_DIRS} ${GMM_INCLUDE_DIRS})# not needed when using GDCM_USE_FILE, urrgh.

# Set the CAP Client executable with all sources and headers.  Set headers here also so 
# that some IDEs recognise them and include them in their projects.
ADD_EXECUTABLE(${CAP_CLIENT_EXECUTABLE} ${APP_SRCS} ${APP_HDRS} ${UI_SRCS_AND_HDRS} ${TXT_HDRS})

# Set some target properties, helpful to visually determine properties of a build.
SET_TARGET_PROPERTIES(${CAP_CLIENT_EXECUTABLE} PROPERTIES DEBUG_POSTFIX "d" MINSIZEREL_POSTFIX "msr" RELWITHDEBUGINFO_POSTFIX "rwdi")

# and for each of your dependant executable/library targets:
TARGET_LINK_LIBRARIES(${CAP_CLIENT_EXECUTABLE} ${CMGUI_LIBRARIES})

# these libraries are not pulled in by cmgui or one of it's dependencies
# is not reporting that it needs them
TARGET_LINK_LIBRARIES(${CAP_CLIENT_EXECUTABLE}
	"${GDCM_INSTALL_PREFIX}/lib/libgdcmDICT.a"
	"${GDCM_INSTALL_PREFIX}/lib/libgdcmDSED.a"
	"${GDCM_INSTALL_PREFIX}/lib/libgdcmCommon.a"
	"${GDCM_INSTALL_PREFIX}/lib/libgdcmIOD.a"
	"${GDCM_INSTALL_PREFIX}/lib/libgdcmMSFF.a"
	"${GDCM_INSTALL_PREFIX}/lib/libgdcmCWRAPPER.a"
	"${GDCM_INSTALL_PREFIX}/lib/libgdcmuuid.a")
	
# Unit Testing
IF(CAP_ENABLE_UNIT_TESTS)
	SET(CAPCLIENT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
	SET(CAPCLIENT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
	ADD_SUBDIRECTORY(test)
ENDIF()

IF(CAP_PRINT_SUMMARY)
	MESSAGE(STATUS "")
	MESSAGE(STATUS "======== CAP Client Configuration Summary ========")
	MESSAGE(STATUS "CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
	MESSAGE(STATUS "CMAKE_SYSTEM_NAME is ${CMAKE_SYSTEM_NAME}")
#	MESSAGE(STATUS "CMAKE_CXX_FLAGS is ${CMAKE_CXX_FLAGS}")
	MESSAGE(STATUS "==================================================")
	MESSAGE(STATUS "")
ENDIF()
