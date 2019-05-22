cmake_minimum_required( VERSION 3.0 )

project( "streams_test" )

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()


add_executable( ${PROJECT_NAME} tests/main.cpp )

if(MSVC)
  target_compile_options( ${PROJECT_NAME} PRIVATE /W4 /WX )
else()
  target_compile_options( ${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror )
endif()


target_include_directories( ${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include" )

target_sources( ${PROJECT_NAME} PRIVATE "include/mvd/streams/access_policy.h" )
target_sources( ${PROJECT_NAME} PRIVATE "include/mvd/streams/async_stream.h" )
target_sources( ${PROJECT_NAME} PRIVATE "include/mvd/streams/observer.h" )
target_sources( ${PROJECT_NAME} PRIVATE "include/mvd/streams/operators.h" )
target_sources( ${PROJECT_NAME} PRIVATE "include/mvd/streams/stream.h" )
target_sources( ${PROJECT_NAME} PRIVATE "include/mvd/streams/subject.h" )

target_sources( ${PROJECT_NAME} PRIVATE "tests/async_stream.test.cpp" )
target_sources( ${PROJECT_NAME} PRIVATE "tests/observer.test.cpp" )
target_sources( ${PROJECT_NAME} PRIVATE "tests/operators.test.cpp" )
target_sources( ${PROJECT_NAME} PRIVATE "tests/stream.test.cpp" )

target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})