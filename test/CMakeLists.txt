
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
	SET( CAPCLIENT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE PATH "Location of the CAP Client src directory.")
	SET(TXT2INCLUDE_TARGET txt2include)
	ADD_EXECUTABLE(${TXT2INCLUDE_TARGET} ${CAPCLIENT_ROOT_DIR}/src/utils/txt2include.c)

	# local testing option
	IF( NOT DEFINED CAPTEST_ENABLE_GUI_UNIT_TESTS )
		SET( CAPTEST_ENABLE_GUI_UNIT_TESTS OFF )
	ENDIF()

	OPTION( CAPTEST_ENABLE_GUI_UNIT_TESTS "Enable CAPClient GUI Unit Tests." ${CAPTEST_ENABLE_GUI_UNIT_TESTS} )

	# Find dependent packages
	FIND_PACKAGE(GTest REQUIRED)
	FIND_PACKAGE(Cmgui REQUIRED)
	FIND_PATH(CAPTEST_GMM_INCLUDE_DIR gmm/gmm.h)
	IF(NOT EXISTS ${CAPTEST_GMM_INCLUDE_DIR}/gmm/gmm.h)
		MESSAGE(FATAL_ERROR "Could not find required header 'gmm/gmm.h'.")
	ENDIF()
	FIND_PATH(CAPTEST_BOOST_INCLUDE_DIR boost/shared_ptr.hpp)
	IF(NOT EXISTS ${CAPTEST_BOOST_INCLUDE_DIR}/boost/shared_ptr.hpp)
		MESSAGE(FATAL_ERROR "Could not find required header 'boost/shared_ptr.hpp'.")
	ENDIF()
ELSE()
	SET(CAPTEST_GMM_INCLUDE_DIR "${CAP_GMM_INCLUDE_DIR}")
	SET(CAPTEST_BOOST_INCLUDE_DIR "${CAP_BOOST_INCLUDE_DIR}")
ENDIF()

SET(CMAKE_MODULE_PATH "${CAPCLIENT_ROOT_DIR}/cmake/")
INCLUDE(Macros)

SET(GMM_INCLUDE_DIRS "${CAPTEST_GMM_INCLUDE_DIR}")
SET(FILESYSTEM_TESTDIR "${CMAKE_CURRENT_SOURCE_DIR}/testdir")
SET(DICOMIMAGE_IMAGEDIR "${CMAKE_CURRENT_SOURCE_DIR}/imagedir")
SET(SAMPLEANALYSISXML_FILE "${CMAKE_CURRENT_SOURCE_DIR}/SampleAnalysisUsingXsd.xml")
SET(SAMPLEANNOTATION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/SampleAnnotationFile.xml")
SET(SAMPLEIMAGES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/sampleimages.xml")
CONFIGURE_FILE(unittestconfigure.h.cmake unittestconfigure.h)

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/src
	${GTEST_INCLUDE_DIRS}
	${CAPCLIENT_ROOT_DIR}/src
	${CAPCLIENT_ROOT_DIR}/test
	${CMGUI_INCLUDE_DIRS}
	${GDCM_INCLUDE_DIRS}
	${GMM_INCLUDE_DIRS}
)

SET(UNIT_TESTS)
SET(GUI_TESTS)

LIST(APPEND UNIT_TESTS Math)
SET(Math_SRC
	UnitTestCAPMath.cpp
	UnitTestTotalLeastSquares.cpp
)

LIST(APPEND UNIT_TESTS Xml)
SET(Xml_SRC
	UnitTestCAPXMLFile.cpp
	${CAPCLIENT_ROOT_DIR}/src/dicomimage.cpp
	${CAPCLIENT_ROOT_DIR}/src/PlatformInfo.cpp
	${CAPCLIENT_ROOT_DIR}/src/utils/filesystem.cpp
	${CAPCLIENT_ROOT_DIR}/src/utils/misc.cpp
	${CAPCLIENT_ROOT_DIR}/src/datapoint.cpp
	${CAPCLIENT_ROOT_DIR}/src/model/modellingpoint.cpp
	${CAPCLIENT_ROOT_DIR}/src/CAPContour.cpp
	${CAPCLIENT_ROOT_DIR}/src/labelledslice.cpp
	${CAPCLIENT_ROOT_DIR}/src/standardheartdefinitions.cpp
	${CAPCLIENT_ROOT_DIR}/src/CAPXMLFile.cpp
	${CAPCLIENT_ROOT_DIR}/src/CAPXMLFileHandler.cpp
	${CAPCLIENT_ROOT_DIR}/src/CAPAnnotationFile.cpp
)

LIST(APPEND UNIT_TESTS ImageSet)
SET(ImageSet_SRC
	UnitTestImageSet.cpp
	${CAPCLIENT_ROOT_DIR}/src/standardheartdefinitions.cpp
)

LIST(APPEND UNIT_TESTS Modeller)
SET(Modeller_SRC
	UnitTestModeller.cpp
	${CAPCLIENT_ROOT_DIR}/src/model/modellingmode.cpp
	${CAPCLIENT_ROOT_DIR}/src/model/modeller.cpp
	${CAPCLIENT_ROOT_DIR}/src/model/heart.cpp
	${CAPCLIENT_ROOT_DIR}/src/datapoint.cpp
	${CAPCLIENT_ROOT_DIR}/src/model/modellingpoint.cpp
	${CAPCLIENT_ROOT_DIR}/src/capclientwindowheartmodel.cpp
	${CAPCLIENT_ROOT_DIR}/src/cmgui/extensions.cpp
	${CAPCLIENT_ROOT_DIR}/src/standardheartdefinitions.cpp
	${CAPCLIENT_ROOT_DIR}/src/math/timesmoother.cpp
	${CAPCLIENT_ROOT_DIR}/src/utils/filesystem.cpp
)

LIST(APPEND UNIT_TESTS CAPAnnotationFile)
SET(CAPAnnotationFile_SRC
	UnitTestCAPAnnotationFile.cpp
	${CAPCLIENT_ROOT_DIR}/src/CAPAnnotationFile.cpp
)

LIST(APPEND UNIT_TESTS AnnotationEditor)
SET(AnnotationEditor_SRC
	UnitTestAnnotationEditor.cpp
	${CAPCLIENT_ROOT_DIR}/src/AnnotationEditor.cpp
	${CAPCLIENT_ROOT_DIR}/src/CAPAnnotationFile.cpp
)

LIST(APPEND UNIT_TESTS FileSystem)
SET(FileSystem_SRC
	UnitTestFileSystem.cpp
	${CAPCLIENT_ROOT_DIR}/src/utils/filesystem.cpp
)

LIST(APPEND UNIT_TESTS UtilsMisc)
SET(UtilsMisc_SRC
	unittestutilsmisc.cpp
	${CAPCLIENT_ROOT_DIR}/src/utils/misc.cpp
)

LIST(APPEND UNIT_TESTS DICOMImage)
Set(DICOMImage_SRC
	UnitTestDICOMImage.cpp
	${CAPCLIENT_ROOT_DIR}/src/dicomimage.cpp
	)

IF(CAP_ENABLE_GUI_UNIT_TESTS OR CAPTEST_ENABLE_GUI_UNIT_TESTS)
	LIST(APPEND GUI_TESTS ImageBrowser)
	XRC_TO_CPP("${CMAKE_CURRENT_BINARY_DIR}/src/ui" "${CAPCLIENT_ROOT_DIR}/src/ui/ImageBrowserWindowUI.xrc" ImageBrowser_UI_SRCS_AND_HDRS)
	SET(ImageBrowser_SRC
		unittestimagebrowser.cpp
		${CAPCLIENT_ROOT_DIR}/src/cmgui/sceneviewerpanel.cpp
		${CAPCLIENT_ROOT_DIR}/src/imagebrowser.cpp
		${CAPCLIENT_ROOT_DIR}/src/standardheartdefinitions.cpp
		${CAPCLIENT_ROOT_DIR}/src/imagebrowserwindow.cpp
		${CAPCLIENT_ROOT_DIR}/src/cmgui/extensions.cpp
		${CAPCLIENT_ROOT_DIR}/src/material.cpp
		${CAPCLIENT_ROOT_DIR}/src/dicomimage.cpp
		${CAPCLIENT_ROOT_DIR}/src/labelledslice.cpp
		${CAPCLIENT_ROOT_DIR}/src/utils/filesystem.cpp
		${CAPCLIENT_ROOT_DIR}/src/utils/misc.cpp
		${ImageBrowser_UI_SRCS_AND_HDRS}
	)

	LIST(APPEND GUI_TESTS SelfDeletion)
	XRC_TO_CPP("${CMAKE_CURRENT_BINARY_DIR}/src/ui" "ui/selfdeletionwindowui.xrc" SelfDeletion_UI_SRCS_AND_HDRS)
	SET(SelfDeletion_SRC
		UnitTestSelfDeletion.cpp
		selfdeletionwindow.cpp
		${SelfDeletion_UI_SRCS_AND_HDRS}
		)
		
	LIST(APPEND GUI_TESTS LogWindow)
	LIST(APPEND XRC_LogWindow "${CAPCLIENT_ROOT_DIR}/src/ui/logdialogui.xrc" "ui/testlogwindowui.xrc")
	XRC_TO_CPP("${CMAKE_CURRENT_BINARY_DIR}/src/ui" "${XRC_LogWindow}" LogWindow_UI_SRCS_ANS_HDRS)
	SET(LogWindow_SRC
		unittestlogwindow.cpp
		${CAPCLIENT_ROOT_DIR}/src/logwindow.cpp
		${CAPCLIENT_ROOT_DIR}/src/logmsg.cpp
		${CAPCLIENT_ROOT_DIR}/src/utils/misc.cpp
		${CAPCLIENT_ROOT_DIR}/src/utils/filesystem.cpp
		${LogWindow_UI_SRCS_ANS_HDRS}
		)

ENDIF()

IF(MSVC)
	SET(CAPAnnotationFile_SRC
		${CAPAnnotationFile_SRC}
		${CAPCLIENT_ROOT_DIR}/src/win32/dirent.c
		)
		
	SET(FileSystem_SRC
		${FileSystem_SRC}
		${CAPCLIENT_ROOT_DIR}/src/win32/dirent.c
		${CAPCLIENT_ROOT_DIR}/src/win32/linuxutils.c
		)
	
	SET(Modeller_SRC
		${Modeller_SRC}
		${CAPCLIENT_ROOT_DIR}/src/win32/dirent.c
		${CAPCLIENT_ROOT_DIR}/src/win32/linuxutils.c
		)
	
	SET(Xml_SRC
		${Xml_SRC}
		${CAPCLIENT_ROOT_DIR}/src/win32/dirent.c
		${CAPCLIENT_ROOT_DIR}/src/win32/linuxutils.c
		)
	
	IF(CAP_ENABLE_GUI_UNIT_TESTS OR CAPTEST_ENABLE_GUI_UNIT_TESTS)
		SET(ImageBrowser_SRC
			${ImageBrowser_SRC}
			${CAPCLIENT_ROOT_DIR}/src/win32/dirent.c
			${CAPCLIENT_ROOT_DIR}/src/win32/linuxutils.c
			)
			
		SET(LogWindow_SRC
			${LogWindow_SRC}
			${CAPCLIENT_ROOT_DIR}/src/win32/dirent.c
			${CAPCLIENT_ROOT_DIR}/src/win32/linuxutils.c
			)
	ENDIF()
ENDIF()

# copy XML files
#FILE(COPY SampleAnalysisUsingXsd.xml SampleAnnotationFile.xml DESTINATION ${PROJECT_BINARY_DIR})

FOREACH( TEST ${UNIT_TESTS} )
	ADD_EXECUTABLE(UnitTest${TEST} ${${TEST}_SRC})
	ADD_TEST(UnitTest${TEST} UnitTest${TEST})
	TARGET_LINK_LIBRARIES(UnitTest${TEST} ${GTEST_MAIN_LIBRARIES} ${CMGUI_LIBRARIES})
	SET_TARGET_PROPERTIES(UnitTest${TEST} PROPERTIES  COMPILE_DEFINITIONS "UnitTest${TEST}" )
ENDFOREACH()

IF(CAP_ENABLE_GUI_UNIT_TESTS OR CAPTEST_ENABLE_GUI_UNIT_TESTS)
	FOREACH( TEST ${GUI_TESTS} )
		ADD_EXECUTABLE(GuiTest${TEST} ${${TEST}_SRC})
		ADD_TEST(GuiTest${TEST} GuiTest${TEST})
		TARGET_LINK_LIBRARIES(GuiTest${TEST} ${GTEST_MAIN_LIBRARIES} ${CMGUI_LIBRARIES})
	ENDFOREACH()
ENDIF()


