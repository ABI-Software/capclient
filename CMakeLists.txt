cmake_minimum_required(VERSION 2.8)
PROJECT(CAPClient)
ADD_DEFINITIONS(-DREPORT_GL_ERRORS -DOPENGL_API -DUSE_PARAMETER_ON)
ADD_DEFINITIONS(-DFE_VALUE_IS_DOUBLE)

# Set the CMISS_ROOT
SET(CMISS_ROOT "" CACHE PATH "Path to the third party folder")
IF(NOT EXISTS ${CMISS_ROOT})
	MESSAGE(FATAL_ERROR "Please define CMISS_ROOT.")
ELSE()
	SET(ENV{CMISS_ROOT} ${CMISS_ROOT})
	MESSAGE(STATUS "CMISS_ROOT = " $ENV{CMISS_ROOT})
ENDIF()

# build type: Debug (default) or Release
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Choose the type of build, options are: Debug Release."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
MESSAGE(STATUS "CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "CMAKE_SYSTEM_NAME is ${CMAKE_SYSTEM_NAME}")

IF(CMAKE_BUILD_TYPE STREQUAL Release)
  MESSAGE(STATUS "Building Release")
  ADD_DEFINITIONS(-DOPTIMISED) 
  SET(BUILD_VERSION cmgui-wx)
  SET(CAP_CLIENT_EXECUTABLE CAPClient)
ELSE()
  MESSAGE(STATUS "Building Debug")
  ADD_DEFINITIONS(-g2 -gdwarf-2)
  SET(BUILD_VERSION cmgui-wx-debug)
  SET(CAP_CLIENT_EXECUTABLE CAPClient_debug)
ENDIF()

# ARCHITECTURE
IF(APPLE)
  ADD_DEFINITIONS(-DCMGUI -DUNIX -DDARWIN)
  SET(LIB_ARCH_DIR i386-32-darwin)
  SET(CMAKE_OSX_ARCHITECTURES i386 CACHE STRINGS "OSX architecture" FORCE)
  SET(CMAKE_CXX_FLAGS "-arch i386" CACHE STRINGS "OSX CXX flags architecture" FORCE)
  SET(CMAKE_EXE_LINKER_FLAGS "-arch i386" CACHE STRINGS "OSX linker flags architecture" FORCE)
ENDIF(APPLE)
IF(WIN32)
  ADD_DEFINITIONS(-mno-cygwin -DWIN32 -DLIBXML_STATIC)
  SET(CMAKE_EXE_LINKER_FLAGS -mno-cygwin)
  SET(LIB_ARCH_DIR i386-win32)
ENDIF(WIN32)
IF(CMAKE_SYSTEM_NAME STREQUAL Linux)
  ADD_DEFINITIONS(-DUNIX)
  SET(LIB_ARCH_DIR i686-linux)
ENDIF()

INCLUDE_DIRECTORIES(
    ${CMISS_ROOT}/cmgui/source
    ${CMISS_ROOT}/third_party/${LIB_ARCH_DIR}/include/gdcm-2.0
    ${CMISS_ROOT}/third_party/include/${LIB_ARCH_DIR}/libxml2
    ${CMISS_ROOT}/gmm-3.1/include
    ${CMISS_ROOT}/boost
    ${CMISS_ROOT}/itk/${LIB_ARCH_DIR}/include/InsightToolkit/Utilities/vxl/vcl
    ${CMISS_ROOT}/itk/${LIB_ARCH_DIR}/include/InsightToolkit/Utilities/vxl/core
)

SET(CAPClient_SRC ${PROJECT_SOURCE_DIR}/src)
SET(SOURCES 
  ${CAPClient_SRC}/MainWindow.cpp
  ${CAPClient_SRC}/main.cpp
  ${CAPClient_SRC}/FileSystem.cpp
  ${CAPClient_SRC}/CmguiExtensions.cpp
  ${CAPClient_SRC}/DICOMImage.cpp
  ${CAPClient_SRC}/ImageSlice.cpp
  ${CAPClient_SRC}/ImageSet.cpp
  ${CAPClient_SRC}/CAPModelLVPS4X4.cpp
  ${CAPClient_SRC}/CmguiManager.cpp
  ${CAPClient_SRC}/CAPModeller.cpp
  ${CAPClient_SRC}/CAPTimeSmoother.cpp
  ${CAPClient_SRC}/CAPModellingMode.cpp
  ${CAPClient_SRC}/DataPoint.cpp
  ${CAPClient_SRC}/CAPHtmlWindow.cpp
  ${CAPClient_SRC}/CAPXMLFile.cpp
  ${CAPClient_SRC}/CAPXMLFileHandler.cpp
  ${CAPClient_SRC}/CAPAnnotationFile.cpp
  ${CAPClient_SRC}/ImageBrowseWindow.cpp
  ${CAPClient_SRC}/CAPMaterial.cpp
  ${CAPClient_SRC}/PlatformInfo.cpp
  ${CAPClient_SRC}/CAPContour.cpp
  ${CAPClient_SRC}/CmguiImageSliceGraphics.cpp
  ${CAPClient_SRC}/AnnotationWindow.cpp
  ${CAPClient_SRC}/AnnotationEditor.cpp
  #${CAPClient_SRC}/DataSet.cpp
)

SET(ITK_BINDIR ${CMISS_ROOT}/itk/${LIB_ARCH_DIR}/lib/InsightToolkit)
SET(ITK_LIBPATH_PREFIX -L)
SET(ITK_LIB_PREFIX -l)
SET(ITK_LIB_SUFFIX)
SET(ITK_BIN_CONFIG_DIR)
SET(ITK_LIB 
  #${ITK_LIBPATH_PREFIX}${ITK_BINDIR}/bin${ITK_BIN_CONFIG_DIR} 
  ${ITK_LIBPATH_PREFIX}${ITK_BINDIR} 
  ${ITK_LIB_PREFIX}ITKAlgorithms${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}ITKStatistics${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}ITKBasicFilters${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}ITKCommon${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itkvnl_algo${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itkvnl${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itkv3p_netlib${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itkvcl${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itksys${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}ITKDICOMParser${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itkzlib${ITK_LIB_SUFFIX} 
  #${ITK_LIB_PREFIX}itkzlib${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itktiff${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itkjpeg12${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itkjpeg16${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}ITKNrrdIO${ITK_LIB_SUFFIX}
)

SET(IMAGEMAGICK_PATH ${CMISS_ROOT}/third_party)
SET(IMAGEMAGICK_LIB 
  ${IMAGEMAGICK_PATH}/lib/${LIB_ARCH_DIR}/libMagickCore.a 
  ${IMAGEMAGICK_PATH}/lib/${LIB_ARCH_DIR}/libtiff.a 
  ${IMAGEMAGICK_PATH}/lib/${LIB_ARCH_DIR}/libpng.a 
  ${IMAGEMAGICK_PATH}/lib/${LIB_ARCH_DIR}/libjpeg.a 
  ${IMAGEMAGICK_PATH}/lib/${LIB_ARCH_DIR}/libbz2.a 
  ${IMAGEMAGICK_PATH}/lib/${LIB_ARCH_DIR}/libz.a
  -L${IMAGEMAGICK_PATH}/${LIB_ARCH_DIR}/lib
  -lgdcmCWRAPPER
  -lgdcmMSFF 
  -lgdcmDSED 
  -lgdcmzlib 
  -lgdcmDICT 
  -lgdcmCommon 
  -lgdcmIOD 
  -lgdcmopenjpeg 
  -lgdcmjpeg8 
  -lgdcmjpeg16 
  -lgdcmjpeg12 
  -lgdcmexpat
)

SET(INTERPRETER_PATH ${CMISS_ROOT}/perl_interpreter)
IF(WIN32)
  SET(INTERPRETER_LIB ${INTERPRETER_PATH}/lib/${LIB_ARCH_DIR}/libperlinterpreter-includeperl.a)
ELSE(WIN32)
  SET(INTERPRETER_LIB ${INTERPRETER_PATH}/lib/${LIB_ARCH_DIR}/libperlinterpreter.a)
ENDIF(WIN32)

SET(XML2_PATH ${CMISS_ROOT}/third_party)
SET(XML2_LIB
  ${XML2_PATH}/lib/${LIB_ARCH_DIR}/libxml2.a
  ${XML2_PATH}/lib/${LIB_ARCH_DIR}/libz.a
#  -liconv
)

SET(CMGUI_LIB ${CMISS_ROOT}/cmgui/lib/${LIB_ARCH_DIR}/${BUILD_VERSION}/libcmgui.a)

# set 3rd party wxWidgets
SET(wxWidgets_ROOT_DIR ${CMISS_ROOT}/wxWidgets)
SET(wxWidgets_LIB_DIR ${CMISS_ROOT}/wxWidgets/lib)
FIND_PACKAGE(wxWidgets COMPONENTS xrc gl xml adv html core base) 

IF(wxWidgets_FOUND)
  # I need to force the wxWidget configuration & rc to the CMISS_ROOT folder
  SET(wxWidgets_CONFIG_EXECUTABLE ${CMISS_ROOT}/wxWidgets/bin/wx-config CACHE STRINGS "wxConfig" FORCE)
  SET(wxWidgets_wxrc_EXECUTABLE ${CMISS_ROOT}/wxWidgets/bin/wxrc CACHE STRINGS "wxConfig" FORCE)

  MESSAGE(STATUS "including: " ${wxWidgets_USE_FILE})
  INCLUDE(${wxWidgets_USE_FILE})
  MESSAGE(STATUS "wxWidgets found")
  MESSAGE(STATUS "linking: " ${wxWidgets_LIBRARIES})
  MESSAGE(STATUS "lib dir: " ${wxWidgets_LIB_DIR})
  ADD_DEFINITIONS(-DWX_USER_INTERFACE)
ELSE(wxWidgets_FOUND)
  # For convenience. When we cannot continue, inform the user
  MESSAGE(FATAL_ERROR "wxWidgets not found!")
ENDIF(wxWidgets_FOUND)

IF(APPLE)
  SET(common_LIBS "-liconv")
ENDIF(APPLE)
IF(WIN32)
  SET(common_LIBS "-lws2_32")
ENDIF(WIN32)
IF(CMAKE_SYSTEM_NAME STREQUAL Linux)
  SET(common_LIBS -lcrypt -lz)
ENDIF()

# copy XRC files and the Data directory
SET(XRC_FILES
    MainWindow.xrc
    UserCommentDialog.xrc
    ImageBrowseWindow.xrc
    UserCommentDialog.xrc 
)
FILE(COPY ${XRC_FILES} Data DESTINATION ${PROJECT_BINARY_DIR})

ADD_EXECUTABLE(${CAP_CLIENT_EXECUTABLE} ${SOURCES})
#SET(wxMathPlot ${CMISS_ROOT}/wxMathPlot/lib/libmathplot.a)

# and for each of your dependant executable/library targets:
TARGET_LINK_LIBRARIES(${CAP_CLIENT_EXECUTABLE} ${CMGUI_LIB} ${IMAGEMAGICK_LIB}
    ${INTERPRETER_LIB} ${XML2_LIB} ${ITK_LIB} ${wxWidgets_LIBRARIES} 
    ${common_LIBS})
    
# Unit Testing
OPTION(ENABLE_CAPCLIENT_UNIT_TEST "Enable CAPClient Unit Tests" OFF)
IF(${ENABLE_CAPCLIENT_UNIT_TEST})
    ADD_SUBDIRECTORY(test)
ENDIF()
