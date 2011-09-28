
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(CAPClient_UnitTest)

# Unit Testing
ENABLE_TESTING()

# Find gtest
FIND_PACKAGE(GTest REQUIRED)

INCLUDE_DIRECTORIES(
	${GTEST_INCLUDE_DIRS}
#	${CAPCLIENT_SOURCE_DIR}/src
#	${CAPCLIENT_BINARY_DIR}/src
)

SET(UnitTest_SRC
	UnitTestCAPMath.cpp
	UnitTestTotalLeastSquares.cpp
	UnitTestCAPXMLFile.cpp
	${CAPCLIENT_SOURCE_DIR}/src/CAPXMLFile.cpp
	UnitTestImageSet.cpp
	${CAPCLIENT_SOURCE_DIR}/src/CAPAnnotationFile.cpp
)

SET(UnitTestCAPModeller_SRC
	UnitTestCAPModeller.cpp
	${CAPCLIENT_SOURCE_DIR}/src/CAPModellingMode.cpp
	${CAPCLIENT_SOURCE_DIR}/src/CAPModeller.cpp
	${CAPCLIENT_SOURCE_DIR}/src/CAPTimeSmoother.cpp
)

SET(UnitTestCAPAnnotationFile_SRC
	UnitTestCAPAnnotationFile.cpp
	${CAPCLIENT_SOURCE_DIR}/src/CAPAnnotationFile.cpp
	UnitTestAnnotationEditor.cpp
	${CAPCLIENT_SOURCE_DIR}/src/AnnotationEditor.cpp
)

SET(UnitTestFileSystem_SRC
	UnitTestFileSystem.cpp
	${CAPCLIENT_SOURCE_DIR}/src/FileSystem.cpp
)

SET(GuiTestImageBrowser_SRC
	UnitTestImageBrowser.cpp
	${CAPCLIENT_SOURCE_DIR}/src/cmguipanel.cpp
	${CAPCLIENT_SOURCE_DIR}/src/imagebrowser.cpp
	${CAPCLIENT_SOURCE_DIR}/src/imagebrowserwindow.cpp
	${CAPCLIENT_BINARY_DIR}/src/ui/ImageBrowserWindowUI.cpp
	${CAPCLIENT_SOURCE_DIR}/src/cmgui/utilities.cpp
	${CAPCLIENT_SOURCE_DIR}/src/CAPMaterial.cpp
	${CAPCLIENT_SOURCE_DIR}/src/labelledslice.cpp
	${CAPCLIENT_SOURCE_DIR}/src/labelledtexture.cpp
	${CAPCLIENT_SOURCE_DIR}/src/FileSystem.cpp
)

# copy XML files
FILE(COPY SampleAnalysisUsingXsd.xml SampleAnnotationFile.xml DESTINATION ${PROJECT_BINARY_DIR})

SET(UNIT_TESTS UnitTest UnitTestCAPModeller UnitTestCAPAnnotationFile UnitTestFileSystem)
SET(GUI_TESTS GuiTestImageBrowser)

FOREACH( TEST ${UNIT_TESTS} )
	ADD_EXECUTABLE(${TEST} ${${TEST}_SRC})
	ADD_TEST(${TEST} ${TEST})
	TARGET_LINK_LIBRARIES(${TEST} ${GTEST_MAIN_LIBRARIES} ${CMGUI_LIBRARIES})
ENDFOREACH()

IF(CAP_ENABLE_GUI_UNIT_TESTS)
	FOREACH( TEST ${GUI_TESTS} )
		ADD_EXECUTABLE(${TEST} ${${TEST}_SRC})
		ADD_TEST(${TEST} ${TEST})
		TARGET_LINK_LIBRARIES(${TEST} ${GTEST_MAIN_LIBRARIES} ${CMGUI_LIBRARIES})
	ENDFOREACH()
ENDIF()


