cmake_minimum_required(VERSION 2.6)
PROJECT(Cmgui)
#ADD_DEFINITIONS(-DCMGUI -DUNIX -DDARWIN)
ADD_DEFINITIONS(-DREPORT_GL_ERRORS -DOPENGL_API -DUSE_PARAMETER_ON)
#ADD_DEFINITIONS(-DWX_USER_INTERFACE)
#ADD_DEFINITIONS(-g2 -gdwarf-2)
ADD_DEFINITIONS(-o3 -NDEBUG)

#how to include common.Makefile?
SET(CMISS_ROOT $ENV{CMISS_ROOT})
IF(APPLE)
  SET(LIB_ARCH_DIR i386-32-darwin)
  ADD_DEFINITIONS(-DDARWIN)
ENDIF(APPLE)
IF(WIN32)
  ADD_DEFINITIONS(-mno-cygwin -DWIN32)
  SET(LIB_ARCH_DIR i386-win32)
  SET(CMAKE_EXE_LINKER_FLAGS -mno-cygwin)
ENDIF(WIN32)
SET(BUILD_VERSION cmgui-wx-debug)
INCLUDE_DIRECTORIES(${CMISS_ROOT}/cmgui/source)
INCLUDE_DIRECTORIES(${CMISS_ROOT}/image_libraries/${LIB_ARCH_DIR}/include/gdcm-2.0)
INCLUDE_DIRECTORIES(${CMISS_ROOT}/gmm-3.1/include)
INCLUDE_DIRECTORIES(${CMISS_ROOT}/boost_1_38_0)

SET(SRCS 
src/ViewerFrame.cpp
src/main.cpp
src/Filesystem.cpp
src/CmguiExtensions.cpp
src/DICOMImage.cpp
src/ImageSet.cpp
src/CAPModelLVPS4X4.cpp
src/CmguiManager.cpp
src/CAPModeller.cpp
)

SET(ITK_BINDIR ${CMISS_ROOT}/itk/lib/${LIB_ARCH_DIR})
SET(ITK_LIBPATH_PREFIX -L)
SET(ITK_LIB_PREFIX -l)
SET(ITK_LIB_SUFFIX)
SET(ITK_BIN_CONFIG_DIR)
SET(ITK_LIB ${ITK_LIBPATH_PREFIX}${ITK_BINDIR}/bin${ITK_BIN_CONFIG_DIR} ${ITK_LIB_PREFIX}ITKAlgorithms${ITK_LIB_SUFFIX} ${ITK_LIB_PREFIX}ITKStatistics${ITK_LIB_SUFFIX} ${ITK_LIB_PREFIX}ITKBasicFilters${ITK_LIB_SUFFIX} ${ITK_LIB_PREFIX}ITKCommon${ITK_LIB_SUFFIX} ${ITK_LIB_PREFIX}itkvnl${ITK_LIB_SUFFIX} ${ITK_LIB_PREFIX}itkvnl_algo${ITK_LIB_SUFFIX} ${ITK_LIB_PREFIX}itkvnl${ITK_LIB_SUFFIX} ${ITK_LIB_PREFIX}itknetlib${ITK_LIB_SUFFIX} ${ITK_LIB_PREFIX}itksys${ITK_LIB_SUFFIX} ${ITK_LIB_PREFIX}ITKDICOMParser${ITK_LIB_SUFFIX} ${ITK_LIB_PREFIX}itkzlib${ITK_LIB_SUFFIX} ${ITK_LIB_PREFIX}itkzlib${ITK_LIB_SUFFIX} ${ITK_LIB_PREFIX}itktiff${ITK_LIB_SUFFIX} ${ITK_LIB_PREFIX}itkjpeg12${ITK_LIB_SUFFIX} ${ITK_LIB_PREFIX}itkjpeg16${ITK_LIB_SUFFIX} ${ITK_LIB_PREFIX}ITKNrrdIO${ITK_LIB_SUFFIX})

SET(IMAGEMAGICK_PATH ${CMISS_ROOT}/image_libraries)
SET(IMAGEMAGICK_LIB ${IMAGEMAGICK_PATH}/lib/${LIB_ARCH_DIR}/libMagickCore.a ${IMAGEMAGICK_PATH}/lib/${LIB_ARCH_DIR}/libtiff.a ${IMAGEMAGICK_PATH}/lib/${LIB_ARCH_DIR}/libpng.a ${IMAGEMAGICK_PATH}/lib/${LIB_ARCH_DIR}/libjpeg.a ${IMAGEMAGICK_PATH}/lib/${LIB_ARCH_DIR}/libbz2.a ${IMAGEMAGICK_PATH}/lib/${LIB_ARCH_DIR}/libz.a)
SET(IMAGEMAGICK_LIB ${IMAGEMAGICK_LIB} -L${IMAGEMAGICK_PATH}/${LIB_ARCH_DIR}/lib -lgdcmCWRAPPER -lgdcmMSFF -lgdcmDSED -lgdcmzlib -lgdcmmd5 -lgdcmDICT -lgdcmCommon -lgdcmIOD -lgdcmopenjpeg -lgdcmjpeg8 -lgdcmjpeg16 -lgdcmjpeg12 -lgdcmexpat)


SET(INTERPRETER_PATH ${CMISS_ROOT}/perl_interpreter)
#   INTERPRETER_DEFINES = -DPERL_INTERPRETER
IF(WIN32)
  SET(INTERPRETER_LIB ${INTERPRETER_PATH}/lib/${LIB_ARCH_DIR}/libperlinterpreter-includeperl.a)
ELSE(WIN32)
  SET(INTERPRETER_LIB ${INTERPRETER_PATH}/lib/${LIB_ARCH_DIR}/libperlinterpreter.a)
ENDIF(WIN32)

SET(XML2_PATH ${CMISS_ROOT}/image_libraries)
SET(XML2_LIB ${XML2_PATH}/lib/${LIB_ARCH_DIR}/libxml2.a)

SET(CMGUI_LIB ${CMISS_ROOT}/cmgui/lib/${LIB_ARCH_DIR}/${BUILD_VERSION}/libcmgui.a)

FIND_PACKAGE(wxWidgets COMPONENTS core base xrc gl xml adv html) 
IF(wxWidgets_FOUND)
  message("including: " ${wxWidgets_USE_FILE})
  INCLUDE(${wxWidgets_USE_FILE})
  MESSAGE("wxWidgets found")
  message("linking: " ${wxWidgets_LIBRARIES})
  message("lib dir: " ${wxWidgets_LIB_DIR})
ADD_DEFINITIONS(-DWX_USER_INTERFACE)
IF(WIN32)
  ADD_DEFINITIONS(-D__WXDEBUG__)
ELSE(WIN32)
  ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES -D__WXDEBUG__ -D__WXMAC_)
ENDIF(WIN32)
ELSE(wxWidgets_FOUND)
  # For convenience. When we cannot continue, inform the user
  MESSAGE("wxWidgets not found!")
ENDIF(wxWidgets_FOUND)

LIST(REMOVE_ITEM wxWidgets_LIBRARIES -lwxtiffd-2.8 -lwxjpegd-2.8 -lwxpngd-2.8 -lz -lpthread)
message("After REMOVE_ITEM: " ${wxWidgets_LIBRARIES})

IF(WIN32)
  LIST(REMOVE_ITEM wxWidgets_LIBRARIES -lopengl32 -lglu32)
  SET(wxWidgets_LIBRARIES ${wxWidgets_LIBRARIES} -L${CMISS_ROOT}/mesa/lib/${LIB_ARCH_DIR} -opengl32 -lglu32)
  message("WIN32 :" ${wxWidgets_LIBRARIES})
  SET(common_LIBS "-lws2_32 -lgdi32 -lmsimg32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -lnetapi32 -luuid -lwsock32 -lmpr -lwinmm -lversion -lodbc32 -lstdc++")
ENDIF(WIN32)

#TODO remove the following when Cmgui supports basis evaluation
SET(TEMP
CimBasis/Cim1DFourierBasis.cpp
CimBasis/CimCubicHermiteBasis.cpp
CimBasis/CimBasis.cpp
CimBasis/CimLinearBasis.cpp
CimBasis/CimBiCubicHermiteLinearBasis.cpp
CimBasis/CimTriLinearBasis.cpp
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/CimBasis)

ADD_EXECUTABLE(CAPClient ${SRCS} ${TEMP})
# and for each of your dependant executable/library targets:
IF(APPLE)
TARGET_LINK_LIBRARIES(CAPClient ${wxWidgets_LIBRARIES} ${CMGUI_LIB} ${ITK_LIB} ${IMAGEMAGICK_LIB} 
    ${INTERPRETER_LIB} ${XML2_LIB} ${common_LIBS})  
ENDIF(APPLE)
IF(WIN32)
#test
SET(wxWidgets_LIBRARIES_TEMP  /cygdrive/c/cmiss/wxWidgets/lib/libwx_mswd_xrc-2.8.a /cygdrive/c/cmiss/wxWidgets/lib/libwx_mswd_gl-2.8.a /cygdrive/c/cmiss/wxWidgets/lib/libwx_based_xml-2.8.a /cygdrive/c/cmiss/wxWidgets/lib/libwx_mswd_adv-2.8.a /cygdrive/c/cmiss/wxWidgets/lib/libwx_mswd_html-2.8.a /cygdrive/c/cmiss/wxWidgets/lib/libwx_mswd_core-2.8.a /cygdrive/c/cmiss/wxWidgets/lib/libwx_based-2.8.a -L/cygdrive/c/cmiss/mesa/lib/i386-win32  -lopengl32 -lglu32 -lcomctl32 -lctl3d32 -L/cygdrive/c/cmiss/wxWidgets/lib -lwxexpatd-2.8) 
TARGET_LINK_LIBRARIES(CAPClient ${CMGUI_LIB} ${wxWidgets_LIBRARIES_TEMP} ${INTERPRETER_LIB} ${IMAGEMAGICK_LIB} 
     ${XML2_LIB} ${ITK_LIB} ${common_LIBS})  
ENDIF(WIN32)

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
