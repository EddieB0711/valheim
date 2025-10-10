INCLUDE(../odin.cmake/odin.cmake)
SET(PROJECT_NAME valheim.app.opengl)
ODIN_SETUP_EXE(${PROJECT_NAME})
TARGET_LINK_LIBRARIES(
    ${PROJECT_NAME}
    PRIVATE
    valheim.opengl
)