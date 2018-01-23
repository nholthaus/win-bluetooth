PROJECT(tests)

SET(SOURCE_CODE 
main.cpp
)

LIST(APPEND LIBRARIES
gtest
win-bluetooth
)

ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCE_CODE})
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIBRARIES})
IF(WIN32)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/machine:x64")
ENDIF(WIN32)

ADD_TEST(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

# use gtags if present on the system to populate the vscode intellisense
find_program(GTAGS name gtags HINTS /usr/local/bin DOC "Path to gtags parser")
if(GTAGS)
	add_custom_target(gtags COMMAND gtags WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} COMMENT "Generating intellisense data...")
	add_dependencies(${PROJECT_NAME} gtags)
endif(GTAGS)

