cmake_minimum_required(VERSION 3.0.0)
project(sp4gl VERSION 0.1.0)

include(CTest)
enable_testing()

message("project dir: ${PROJECT_SOURCE_DIR} cmake dir: ${CMAKE_SOURCE_DIR}")

add_library(glfw3 STATIC IMPORTED)
set_target_properties(glfw3 PROPERTIES
  IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/libs/glfw3.lib"
  INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/include/glfw3"
)

add_library(glad "${PROJECT_SOURCE_DIR}/thirdparty/glad.c")
target_include_directories(glad PRIVATE "${PROJECT_SOURCE_DIR}/include")

set(IMGUI_DIR "${PROJECT_SOURCE_DIR}/thirdparty/imgui/")
file(GLOB IMGUI_SRCS "${IMGUI_DIR}/*.cpp")
add_library(imgui ${IMGUI_SRCS})
target_include_directories(imgui PRIVATE "${PROJECT_SOURCE_DIR}/include/imgui")
target_include_directories(imgui PRIVATE "${PROJECT_SOURCE_DIR}/include/")

add_library(stb INTERFACE)

add_library(glm INTERFACE)
set_target_properties(glm PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/include"
)

add_library(assimp STATIC IMPORTED)
set_target_properties(assimp PROPERTIES
  IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/libs/assimp-vc142-mtd.lib"
  INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/include/assimp"
)

set(LIBS glfw3 glad opengl32 stb glm assimp imgui)

set(SRCS src/main.cpp src/shader.cpp src/image.cpp src/mesh.cpp src/model.cpp src/textures.cpp)

add_executable(sp4gl ${SRCS})
target_link_libraries(sp4gl ${LIBS})
target_include_directories(sp4gl PRIVATE "${PROJECT_SOURCE_DIR}/include/sp4gl")
target_include_directories(sp4gl PRIVATE "${PROJECT_SOURCE_DIR}/include")


add_custom_command(TARGET sp4gl POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${PROJECT_SOURCE_DIR}/src/shaders"
  $<TARGET_FILE_DIR:sp4gl>/shaders
)

add_custom_command(TARGET sp4gl POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${PROJECT_SOURCE_DIR}/libs/assimp-vc142-mtd.dll"
  $<TARGET_FILE_DIR:sp4gl>/assimp-vc142-mtd.dll
)

add_custom_command(TARGET sp4gl POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${PROJECT_SOURCE_DIR}/textures"
  $<TARGET_FILE_DIR:sp4gl>/textures
)


add_custom_command(TARGET sp4gl POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${PROJECT_SOURCE_DIR}/resources"
  $<TARGET_FILE_DIR:sp4gl>/resources
)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
