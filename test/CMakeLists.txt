
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(CAPClient_UnitTest)

MESSAGE(STATUS "Entering ${PROJECT_NAME}")

# Unit Testing
ENABLE_TESTING()

INCLUDE_DIRECTORIES(
    ${CMISS_ROOT}/gtest/include 
    ${CAPClient_SRC}
)

SET(UNIT_TEST_SRC
  UnitTestCAPMath.cpp
  UnitTestTotalLeastSquares.cpp
  UnitTestCAPXMLFile.cpp
  ${CAPClient_SRC}/CAPXMLFile.cpp
  UnitTestImageSet.cpp
  ${CAPClient_SRC}/CAPAnnotationFile.cpp
)

SET(UNIT_TEST_MODELLER_SRC
  UnitTestCAPModeller.cpp
  ${CAPClient_SRC}/CAPModellingMode.cpp
  ${CAPClient_SRC}/CAPModeller.cpp
  ${CAPClient_SRC}/CAPTimeSmoother.cpp
)

SET(UNIT_TEST_ANNOTATION_SRC
  UnitTestCAPAnnotationFile.cpp
  ${CAPClient_SRC}/CAPAnnotationFile.cpp
  UnitTestAnnotationEditor.cpp
  ${CAPClient_SRC}/AnnotationEditor.cpp
)

SET(UNIT_TEST_IMAGE_BROWSER_SRC
  UnitTestImageBrowser.cpp
)

# copy XML files
FILE(COPY SampleAnalysisUsingXsd.xml SampleAnnotationFile.xml DESTINATION ${PROJECT_BINARY_DIR})

ADD_EXECUTABLE(UnitTest ${UNIT_TEST_SRC})
ADD_TEST(UnitTest UnitTest)

ADD_EXECUTABLE(UnitTestCAPModeller ${UNIT_TEST_MODELLER_SRC})
ADD_TEST(UnitTestCAPModeller UnitTestCAPModeller)

ADD_EXECUTABLE(UnitTestCAPAnnotationFile ${UNIT_TEST_ANNOTATION_SRC})
ADD_TEST(UnitTestCAPAnnotationFile UnitTestCAPAnnotationFile)

ADD_EXECUTABLE(UnitTestImageBrowser ${UNIT_TEST_IMAGE_BROWSER_SRC})
ADD_TEST(UnitTesetImageBrowser UnitTestImageBrowser)

SET(GTEST_LIB ${CMISS_ROOT}/gtest/make/gtest_main.a)

TARGET_LINK_LIBRARIES(UnitTest ${GTEST_LIB} 
    ${XML2_LIB} ${ITK_LIB} ${common_LIBS})
    
TARGET_LINK_LIBRARIES(UnitTestCAPModeller ${GTEST_LIB}
    ${ITK_LIB})

TARGET_LINK_LIBRARIES(UnitTestCAPAnnotationFile ${GTEST_LIB}
    ${XML2_LIB} ${common_LIBS})
    
TARGET_LINK_LIBRARIES(UnitTestImageBrowser ${GTEST_LIB})

