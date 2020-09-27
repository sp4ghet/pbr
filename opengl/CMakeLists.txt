cmake_minimum_required(VERSION 3.0.0)
project(sp4gl VERSION 0.1.0)

include(CTest)
enable_testing()


add_library(glfw3 STATIC IMPORTED)
set_target_properties(glfw3 PROPERTIES
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/libs/glfw3.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/include/glfw3"
)

add_library(glad "${CMAKE_SOURCE_DIR}/thirdparty/glad.c")
target_include_directories(glad PRIVATE "${CMAKE_SOURCE_DIR}/include/glad")
target_include_directories(glad PRIVATE "${CMAKE_SOURCE_DIR}/include/KHR")
target_include_directories(glad PUBLIC "${CMAKE_SOURCE_DIR}/include/glad")
target_include_directories(glad PUBLIC "${CMAKE_SOURCE_DIR}/include/KHR")


set(LIBS glfw3 glad opengl32)

set(SRCS src/main.cpp src/shader.cpp)

add_executable(sp4gl ${SRCS})
target_link_libraries(sp4gl ${LIBS})
target_include_directories(sp4gl PRIVATE "${CMAKE_SOURCE_DIR}/include/sp4gl")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
