cmake_minimum_required(VERSION 2.6)
PROJECT(CAPClient)
ADD_DEFINITIONS(-DREPORT_GL_ERRORS -DOPENGL_API -DUSE_PARAMETER_ON)
#SET(CMAKE_BUILD_TYPE Release)
MESSAGE("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")

IF(CMAKE_BUILD_TYPE STREQUAL Release)
  MESSAGE("Building Release")
  ADD_DEFINITIONS(-DOPTIMISED) 
  SET(BUILD_VERSION cmgui-wx)
ELSE()
  MESSAGE("Building Debug")
  ADD_DEFINITIONS(-g2 -gdwarf-2)
  SET(BUILD_VERSION cmgui-wx-debug)
ENDIF()

#how to include common.Makefile?
SET(CMISS_ROOT $ENV{CMISS_ROOT})
IF(APPLE)
  ADD_DEFINITIONS(-DCMGUI -DUNIX -DDARWIN)
  SET(LIB_ARCH_DIR i386-32-darwin)
ENDIF(APPLE)
IF(WIN32)
  ADD_DEFINITIONS(-mno-cygwin -DWIN32)
  SET(LIB_ARCH_DIR i386-win32)
ENDIF(WIN32)
INCLUDE_DIRECTORIES(${CMISS_ROOT}/cmgui/source)
INCLUDE_DIRECTORIES(${CMISS_ROOT}/image_libraries/${LIB_ARCH_DIR}/include/gdcm-2.0)
INCLUDE_DIRECTORIES(${CMISS_ROOT}/gmm-3.1/include)
#INCLUDE_DIRECTORIES(${CMISS_ROOT}/boost_1_38_0)

SET(SRCS 
  src/ViewerFrame.cpp
  src/main.cpp
  src/Filesystem.cpp
  src/CmguiExtensions.cpp
  src/DICOMImage.cpp
  src/ImageSlice.cpp
  src/ImageSet.cpp
  src/CAPModelLVPS4X4.cpp
  src/CmguiManager.cpp
  src/CAPModeller.cpp
  src/CAPTimeSmoother.cpp
  src/CAPModellingMode.cpp
  src/DataPoint.cpp
  #src/DataSet.cpp
)

SET(ITK_BINDIR ${CMISS_ROOT}/itk/lib/${LIB_ARCH_DIR})
SET(ITK_LIBPATH_PREFIX -L)
SET(ITK_LIB_PREFIX -l)
SET(ITK_LIB_SUFFIX)
SET(ITK_BIN_CONFIG_DIR)
SET(ITK_LIB 
  ${ITK_LIBPATH_PREFIX}${ITK_BINDIR}/bin${ITK_BIN_CONFIG_DIR} 
  ${ITK_LIB_PREFIX}ITKAlgorithms${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}ITKStatistics${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}ITKBasicFilters${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}ITKCommon${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itkvnl${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itkvnl_algo${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itkvnl${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itknetlib${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itksys${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}ITKDICOMParser${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itkzlib${ITK_LIB_SUFFIX} 
  #${ITK_LIB_PREFIX}itkzlib${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itktiff${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itkjpeg12${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}itkjpeg16${ITK_LIB_SUFFIX} 
  ${ITK_LIB_PREFIX}ITKNrrdIO${ITK_LIB_SUFFIX}
)

SET(IMAGEMAGICK_PATH ${CMISS_ROOT}/image_libraries)
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
  -lgdcmmd5 
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

SET(XML2_PATH ${CMISS_ROOT}/image_libraries)
SET(XML2_LIB ${XML2_PATH}/lib/${LIB_ARCH_DIR}/libxml2.a)

SET(CMGUI_LIB ${CMISS_ROOT}/cmgui/lib/${LIB_ARCH_DIR}/${BUILD_VERSION}/libcmgui.a)

FIND_PACKAGE(wxWidgets COMPONENTS xrc gl xml adv html core base) 
IF(wxWidgets_FOUND)
  message("including: " ${wxWidgets_USE_FILE})
  INCLUDE(${wxWidgets_USE_FILE})
  MESSAGE("wxWidgets found")
  message("linking: " ${wxWidgets_LIBRARIES})
  message("lib dir: " ${wxWidgets_LIB_DIR})
  ADD_DEFINITIONS(-DWX_USER_INTERFACE)
ELSE(wxWidgets_FOUND)
  # For convenience. When we cannot continue, inform the user
  MESSAGE("wxWidgets not found!")
ENDIF(wxWidgets_FOUND)

#LIST(REMOVE_ITEM wxWidgets_LIBRARIES -lwxtiffd-2.8 -lwxjpegd-2.8 -lwxpngd-2.8 -lz -lpthread)
#LIST(REMOVE_ITEM wxWidgets_LIBRARIES -lwxtiff-2.8 -lwxjpeg-2.8 -lwxpng-2.8 -lz -lpthread)
#message("After REMOVE_ITEM: " ${wxWidgets_LIBRARIES})

IF(WIN32)
  SET(common_LIBS "-lws2_32")
ENDIF(WIN32)

ADD_EXECUTABLE(CAPClient ${SRCS})
#SET(wxMathPlot ${CMISS_ROOT}/wxMathPlot/lib/libmathplot.a)

# and for each of your dependant executable/library targets:
TARGET_LINK_LIBRARIES(CAPClient ${CMGUI_LIB} ${IMAGEMAGICK_LIB}
    ${INTERPRETER_LIB} ${XML2_LIB} ${ITK_LIB} ${wxWidgets_LIBRARIES} 
    ${common_LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

# Unit Testing
ENABLE_TESTING()

INCLUDE_DIRECTORIES(${CMISS_ROOT}/gtest-1.3.0/include )
SET(UNIT_TEST_SRC
  test/UnitTest.cpp
)

ADD_EXECUTABLE(UnitTest ${UNIT_TEST_SRC})
ADD_TEST(UnitTest ${EXECUTABLE_OUTPUT_PATH}/UnitTest)

SET(GTEST_LIB ${CMISS_ROOT}/gtest-1.3.0/make/gtest_main.a)
TARGET_LINK_LIBRARIES(UnitTest ${GTEST_LIB})
