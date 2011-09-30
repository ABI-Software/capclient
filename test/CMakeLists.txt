
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(CAPClient_UnitTest)

# Unit Testing
ENABLE_TESTING()

# Check to see if CAPCLIENT_ROOT_DIR is defined.  If not
# then this file is not being read as a subdirectory of
# ${CAPCLIENT_ROOT_DIR}/CMakeLists.txt and we need to
# find and set up all the dependencies for ourselves.
IF(NOT CAPCLIENT_ROOT_DIR)
	# Making the assumption here that the test directory is not 
	# going to be separated from the  rest of the CAP Client 
	# files.
	SET(CAPCLIENT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
ELSE()
	SET(CAPTEST_GMM_INCLUDE_DIR "${CAP_GMM_INCLUDE_DIR}")
ENDIF()

SET(CMAKE_MODULE_PATH "${CAPCLIENT_ROOT_DIR}/cmake/")
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

# local testing option
OPTION_WITH_DEFAULT(CAPTEST_ENABLE_GUI_UNIT_TESTS "Enable CAPClient GUI Unit Tests." OFF)

# Find dependent packages
FIND_PACKAGE(GTest REQUIRED)
FIND_PACKAGE(Cmgui REQUIRED)
FIND_PACKAGE(GDCM REQUIRED)
FIND_PATH(CAPTEST_GMM_INCLUDE_DIR gmm/gmm.h)
IF(NOT CAPTEST_GMM_INCLUDE_DIR)
	MESSAGE(FATAL_ERROR "Could not find required header 'gmm/gmm.h'.")
ENDIF()
SET(GMM_INCLUDE_DIRS "${CAPTEST_GMM_INCLUDE_DIR}")


INCLUDE_DIRECTORIES(
	${GTEST_INCLUDE_DIRS}
	${CAPCLIENT_ROOT_DIR}/src
	${CMGUI_INCLUDE_DIRS}
	${GDCM_INCLUDE_DIRS}
	${GMM_INCLUDE_DIRS}
#	${CAPCLIENT_BINARY_DIR}/src
)

SET(Math_SRC
	UnitTestCAPMath.cpp
	UnitTestTotalLeastSquares.cpp
)

SET(Xml_SRC
	UnitTestCAPXMLFile.cpp
	${CAPCLIENT_ROOT_DIR}/src/CAPXMLFile.cpp
	${CAPCLIENT_ROOT_DIR}/src/CAPAnnotationFile.cpp
)

SET(ImageSet_SRC
	UnitTestImageSet.cpp
)

SET(CAPModeller_SRC
	UnitTestCAPModeller.cpp
	${CAPCLIENT_ROOT_DIR}/src/CAPModellingMode.cpp
	${CAPCLIENT_ROOT_DIR}/src/CAPModeller.cpp
	${CAPCLIENT_ROOT_DIR}/src/CAPTimeSmoother.cpp
)

SET(CAPAnnotationFile_SRC
	UnitTestCAPAnnotationFile.cpp
	${CAPCLIENT_ROOT_DIR}/src/CAPAnnotationFile.cpp
)

SET(AnnotationEditor_SRC
	UnitTestAnnotationEditor.cpp
	${CAPCLIENT_ROOT_DIR}/src/AnnotationEditor.cpp
	${CAPCLIENT_ROOT_DIR}/src/CAPAnnotationFile.cpp
)

SET(FileSystem_SRC
	UnitTestFileSystem.cpp
	${CAPCLIENT_ROOT_DIR}/src/FileSystem.cpp
)

SET(GuiTestImageBrowser_SRC
	UnitTestImageBrowser.cpp
	${CAPCLIENT_ROOT_DIR}/src/cmguipanel.cpp
	${CAPCLIENT_ROOT_DIR}/src/imagebrowser.cpp
	${CAPCLIENT_ROOT_DIR}/src/imagebrowserwindow.cpp
	${CAPCLIENT_BINARY_DIR}/src/ui/ImageBrowserWindowUI.cpp
	${CAPCLIENT_ROOT_DIR}/src/cmgui/utilities.cpp
	${CAPCLIENT_ROOT_DIR}/src/material.cpp
	${CAPCLIENT_ROOT_DIR}/src/labelledslice.cpp
	${CAPCLIENT_ROOT_DIR}/src/labelledtexture.cpp
	${CAPCLIENT_ROOT_DIR}/src/FileSystem.cpp
)

# copy XML files
FILE(COPY SampleAnalysisUsingXsd.xml SampleAnnotationFile.xml DESTINATION ${PROJECT_BINARY_DIR})

SET(UNIT_TESTS Math Xml ImageSet CAPModeller CAPAnnotationFile AnnotationEditor FileSystem)
SET(GUI_TESTS GuiTestImageBrowser)

FOREACH( TEST ${UNIT_TESTS} )
	ADD_EXECUTABLE(UnitTest${TEST} ${${TEST}_SRC})
	ADD_TEST(UnitTest${TEST} UnitTest${TEST})
	TARGET_LINK_LIBRARIES(UnitTest${TEST} ${GTEST_MAIN_LIBRARIES} ${CMGUI_LIBRARIES})
ENDFOREACH()

IF(CAP_ENABLE_GUI_UNIT_TESTS OR CAPTEST_ENABLE_GUI_UNIT_TESTS)
	FOREACH( TEST ${GUI_TESTS} )
		ADD_EXECUTABLE(${TEST} ${${TEST}_SRC})
		ADD_TEST(${TEST} ${TEST})
		TARGET_LINK_LIBRARIES(${TEST} ${GTEST_MAIN_LIBRARIES} ${CMGUI_LIBRARIES})
	ENDFOREACH()
ENDIF()


