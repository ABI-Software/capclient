
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

SET(UNIT_TEST_SRC
	UnitTestCAPMath.cpp
	UnitTestTotalLeastSquares.cpp
	UnitTestCAPXMLFile.cpp
	${CAPCLIENT_SOURCE_DIR}/src/CAPXMLFile.cpp
	UnitTestImageSet.cpp
	${CAPCLIENT_SOURCE_DIR}/src/CAPAnnotationFile.cpp
)

SET(UNIT_TEST_MODELLER_SRC
	UnitTestCAPModeller.cpp
	${CAPCLIENT_SOURCE_DIR}/src/CAPModellingMode.cpp
	${CAPCLIENT_SOURCE_DIR}/src/CAPModeller.cpp
	${CAPCLIENT_SOURCE_DIR}/src/CAPTimeSmoother.cpp
)

SET(UNIT_TEST_ANNOTATION_SRC
	UnitTestCAPAnnotationFile.cpp
	${CAPCLIENT_SOURCE_DIR}/src/CAPAnnotationFile.cpp
	UnitTestAnnotationEditor.cpp
	${CAPCLIENT_SOURCE_DIR}/src/AnnotationEditor.cpp
)

SET(UNIT_TEST_IMAGE_BROWSER_SRC
	UnitTestImageBrowser.cpp
	${CAPCLIENT_BINARY_DIR}/src/ui/ImageBrowserWindowUI.cpp
)

# copy XML files
FILE(COPY SampleAnalysisUsingXsd.xml SampleAnnotationFile.xml DESTINATION ${PROJECT_BINARY_DIR})

ADD_EXECUTABLE(UnitTest ${UNIT_TEST_SRC})
ADD_TEST(UnitTest UnitTest)

ADD_EXECUTABLE(UnitTestCAPModeller ${UNIT_TEST_MODELLER_SRC})
ADD_TEST(UnitTestCAPModeller UnitTestCAPModeller)

ADD_EXECUTABLE(UnitTestCAPAnnotationFile ${UNIT_TEST_ANNOTATION_SRC})
ADD_TEST(UnitTestCAPAnnotationFile UnitTestCAPAnnotationFile)

#ADD_EXECUTABLE(UnitTestImageBrowser ${UNIT_TEST_IMAGE_BROWSER_SRC})
#ADD_TEST(UnitTesetImageBrowser UnitTestImageBrowser)

TARGET_LINK_LIBRARIES(UnitTest ${GTEST_MAIN_LIBRARIES} ${CMGUI_LIBRARIES})
    
TARGET_LINK_LIBRARIES(UnitTestCAPModeller ${GTEST_MAIN_LIBRARIES} ${CMGUI_LIBRARIES})

TARGET_LINK_LIBRARIES(UnitTestCAPAnnotationFile ${GTEST_MAIN_LIBRARIES} ${CMGUI_LIBRARIES})
    
#TARGET_LINK_LIBRARIES(UnitTestImageBrowser ${GTEST_MAIN_LIBRARIES} ${CMGUI_LIBRARIES})

