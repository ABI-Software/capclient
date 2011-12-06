
# CAP Client Macros

MACRO( DEFINE_ARCHITECTURE_DIR )
	STRING( TOLOWER ${CMAKE_SYSTEM_NAME} OPERATING_SYSTEM ) 
	IF(CMAKE_CL_64)
		SET( ARCHITECTURE_DIR ${CMAKE_SYSTEM_PROCESSOR}_64-${OPERATING_SYSTEM} )
	ELSE(CMAKE_CL_64)
		SET( ARCHITECTURE_DIR ${CMAKE_SYSTEM_PROCESSOR}-${OPERATING_SYSTEM} )
	ENDIF(CMAKE_CL_64)
ENDMACRO( DEFINE_ARCHITECTURE_DIR )

MACRO( OPTION_WITH_DEFAULT OPTION_NAME OPTION_STRING OPTION_DEFAULT )
	IF( NOT DEFINED ${OPTION_NAME} )
		SET( ${OPTION_NAME} ${OPTION_DEFAULT} )
	ENDIF( NOT DEFINED ${OPTION_NAME} )

	OPTION( ${OPTION_NAME} "${OPTION_STRING}" ${${OPTION_NAME}} )
ENDMACRO( OPTION_WITH_DEFAULT OPTION_NAME OPTION_STRING OPTION_DEFAULT )

MACRO( CACHE_VAR_WITH_DEFAULT OPTION_NAME OPTION_DEFAULT OPTION_TYPE OPTION_STRING )
	IF( NOT DEFINED ${OPTION_NAME} )
		SET( ${OPTION_NAME} ${OPTION_DEFAULT} )
	ENDIF( NOT DEFINED ${OPTION_NAME} )

	SET( ${OPTION_NAME} "${OPTION_DEFAULT}" CACHE ${OPTION_TYPE} "${OPTION_STRING}" )
ENDMACRO( CACHE_VAR_WITH_DEFAULT OPTION_NAME OPTION_DEFAULT OPTION_TYPE OPTION_STRING )

MACRO( SET_TRUE_IF_NOT_DEFINED VARIABLE )
	IF( NOT DEFINED ${VARIABLE} )
		SET( ${VARIABLE} TRUE )
	ENDIF( NOT DEFINED ${VARIABLE} )
ENDMACRO( SET_TRUE_IF_NOT_DEFINED VARIABLE )

MACRO( SET_FALSE_IF_NOT_DEFINED VARIABLE )
	IF( NOT DEFINED ${VARIABLE} )
		SET( ${VARIABLE} FALSE )
	ENDIF( NOT DEFINED ${VARIABLE} )
ENDMACRO( SET_FALSE_IF_NOT_DEFINED VARIABLE )

# CAP Client Functions

FUNCTION( XRC_TO_CPP XRC_OUTPUT_DIR XRC_SRCS _OUTFILES )
	# Have to do each .xrc file individually if I want separate
	# .cpp/.h files.  Which I do.
	FOREACH( XRC_SRC ${XRC_SRCS} )
		GET_FILENAME_COMPONENT(XRC_SRC_ABS ${XRC_SRC} ABSOLUTE)
		GET_FILENAME_COMPONENT(XRC_SRC_FILE ${XRC_SRC} NAME)
		GET_FILENAME_COMPONENT(XRC_SRC_FCN ${XRC_SRC} NAME_WE)
		GET_FILENAME_COMPONENT(XRC_SRC_PATH ${XRC_SRC} PATH)
		GET_FILENAME_COMPONENT(XRC_SRC_EXT ${XRC_SRC} EXT)

		STRING( REPLACE "${XRC_SRC_EXT}" ".cpp" XRC_CPP_SRC "${XRC_SRC_FILE}" )
		SET( XRC_CPP_SRC_ABS "${XRC_OUTPUT_DIR}/${XRC_CPP_SRC}" )
		STRING( REPLACE ".cpp" ".h" XRC_CPP_HDR "${XRC_CPP_SRC}" )
		SET( XRC_HDR_SRC_ABS "${XRC_OUTPUT_DIR}/${XRC_CPP_HDR}" )

		# My replacement for the WXWIDGETS_ADD_RESOURCE function
		LIST(APPEND WXRC_OUTPUT "${XRC_CPP_SRC_ABS}" "${XRC_HDR_SRC_ABS}")
		SET_SOURCE_FILES_PROPERTIES("${XRC_HDR_SRC_ABS}"
			PROPERTIES GENERATED TRUE)
		SET_SOURCE_FILES_PROPERTIES("${XRC_CPP_SRC_ABS}"
			PROPERTIES GENERATED TRUE)
		ADD_CUSTOM_COMMAND(OUTPUT "${XRC_CPP_SRC_ABS}" "${XRC_HDR_SRC_ABS}"
			COMMAND cmake -E make_directory "${XRC_OUTPUT_DIR}"
			COMMAND cmake -E chdir "${XRC_OUTPUT_DIR}" ${wxWidgets_wxrc_EXECUTABLE} 
				--cpp-code --extra-cpp-code --output=${XRC_CPP_SRC} ${XRC_SRC_ABS} --function=wxXmlInit_${XRC_SRC_FCN}
			DEPENDS ${XRC_SRC_ABS})
	ENDFOREACH( XRC_SRC ${XRC_SRCS} )

	SET( ${_OUTFILES} ${WXRC_OUTPUT} PARENT_SCOPE )
ENDFUNCTION( XRC_TO_CPP XRC_SRCS _OUTFILES )

FUNCTION(MAKE_DIRECTORY_P_STRING DIR_PATH OUTPUT_COMMAND)
	STRING(REGEX REPLACE "/" ";" DIRS_LIST ${DIR_PATH})
	SET(CURRENT_DIR)
	SET(DIR_SEPARATOR)
	SET(MAKE_DIRECTORY_P_COMMAND)
	FOREACH(DIR ${DIRS_LIST})
		SET(CURRENT_DIR ${CURRENT_DIR}${DIR_SEPARATOR}${DIR})
		SET(MAKE_DIRECTORY_P_COMMAND ${MAKE_DIRECTORY_P_COMMAND} COMMAND cmake -E make_directory "${CURRENT_DIR}")
		SET(DIR_SEPARATOR "/")
	ENDFOREACH()
	SET(${OUTPUT_COMMAND} ${MAKE_DIRECTORY_P_COMMAND} PARENT_SCOPE)
ENDFUNCTION()

FUNCTION(HEXIFY_FILE HEXIFY_TARGET HEXIFY_OUTPUT_PATH_PREFIX HEXIFY_FILE_NAME)
	#message(STATUS "input: ${HEXIFY_TARGET} ${HEXIFY_OUTPUT_PATH_PREFIX} ${HEXIFY_FILE_NAME}")
	GET_FILENAME_COMPONENT(TXT_FILE_NAME ${HEXIFY_FILE_NAME} NAME)
	SET(TXT_FILE_HDR ${HEXIFY_OUTPUT_PATH_PREFIX}/${TXT_FILE_NAME}.h)
	MAKE_DIRECTORY_P_STRING(${HEXIFY_OUTPUT_PATH_PREFIX} MAKE_DIRECTORY_P)
	GET_TARGET_PROPERTY(HEXIFY_EXE ${HEXIFY_TARGET} LOCATION)
	#message(STATUS "make dir: ${HEXIFY_EXE}")
	ADD_CUSTOM_COMMAND(OUTPUT "${TXT_FILE_HDR}"
		${MAKE_DIRECTORY_P}
		COMMAND ${HEXIFY_EXE} "${HEXIFY_FILE_NAME}" "${TXT_FILE_HDR}"
		DEPENDS ${HEXIFY_TARGET})
ENDFUNCTION()

FUNCTION(GROUP_SOURCE_FILES)
	FOREACH(F ${ARGV})
		STRING(REGEX REPLACE ".*src/" "" SRC_RELATIVE_F ${F})
		STRING(REGEX MATCHALL "([^/]+/)" RESULT ${SRC_RELATIVE_F})
		SET(FOLDER "${RESULT}")
		LIST(LENGTH RESULT RESULT_LEN)
		IF(${RESULT_LEN} GREATER 0)
			STRING(REGEX REPLACE "/;|/" "\\\\" FOLDER_MSVC ${FOLDER}) 
			# If the file doesn't end in a 'c' or 'cpp' then it's a header file
			STRING(REGEX MATCH "c[p]*$" SOURCE_FILE ${F})
			IF(SOURCE_FILE)
				SET(F_LOCATION "Source Files\\${FOLDER_MSVC}")
			ELSE()
				SET(F_LOCATION "Header Files\\${FOLDER_MSVC}")
			ENDIF()
		ELSE()
			STRING(REGEX MATCH "c[p]*$" SOURCE_FILE ${F})
			IF(SOURCE_FILE)
				SET(F_LOCATION "Source Files")
			ELSE()
				SET(F_LOCATION "Header Files")
			ENDIF()
		ENDIF()
		SOURCE_GROUP(${F_LOCATION} FILES ${F})
	ENDFOREACH()
ENDFUNCTION()
