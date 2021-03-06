cmake_minimum_required( VERSION 3.0 )

project( "streams" )

set( CMAKE_CXX_STANDARD 14)
set( CMAKE_CXX_STANDARD_REQUIRED ON)
set( CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Unit Tests
set( TEST_PROJECT_NAME "streams_test" )

add_executable( ${TEST_PROJECT_NAME} tests/main.cpp )

if(MSVC)
  target_compile_options( ${TEST_PROJECT_NAME} PRIVATE /W4 /WX 
    /wd4324 # disable structure was padded due to alignment specifier
    /w44265 # non-virtual destructor but virtual methods
    #/w44355 # this pointer used in ctor initializer list
    /w44484 # function not marked override, virtual
    /w44485 # function not marked override, virtual
    /w44548 # expression has no effect
    /w44555 # expression has no effect
    #/w44668 # checking undefined preprocessor value with #if 
    #/w44365 # int->uint conversion
    /w44350 # warning when binding an rvalue to a non-const-ref
    /w44867 # pointer to member function was not initialized
    /w44986 # exception specification does not match previous...
    /w45038 # member initialization order
  )  
else()
  target_compile_options( ${TEST_PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic )
endif()


target_include_directories( ${TEST_PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include" )

target_sources( ${TEST_PROJECT_NAME} PRIVATE "include/mvd/streams/access_policy.h" )
target_sources( ${TEST_PROJECT_NAME} PRIVATE "include/mvd/streams/basic_async_stream.h" )
target_sources( ${TEST_PROJECT_NAME} PRIVATE "include/mvd/streams/basic_stream.h" )
target_sources( ${TEST_PROJECT_NAME} PRIVATE "include/mvd/streams/observer.h" )
target_sources( ${TEST_PROJECT_NAME} PRIVATE "include/mvd/streams/operators.h" )

target_sources( ${TEST_PROJECT_NAME} PRIVATE "tests/basic_async_stream.test.cpp" )
target_sources( ${TEST_PROJECT_NAME} PRIVATE "tests/basic_stream.test.cpp" )
target_sources( ${TEST_PROJECT_NAME} PRIVATE "tests/observer.test.cpp" )
target_sources( ${TEST_PROJECT_NAME} PRIVATE "tests/operators.test.cpp" )

target_link_libraries(${TEST_PROJECT_NAME} ${CONAN_LIBS})


# Examples 

set( BASICS_EXAMPLE_NAME "basics_example" )

add_executable( ${BASICS_EXAMPLE_NAME} examples/basics/main.cpp )

if(MSVC)
  target_compile_options( ${BASICS_EXAMPLE_NAME} PRIVATE /W4 /WX 
    /wd4324 # disable structure was padded due to alignment specifier
    /w44265 # non-virtual destructor but virtual methods
    #/w44355 # this pointer used in ctor initializer list
    /w44484 # function not marked override, virtual
    /w44485 # function not marked override, virtual
    /w44548 # expression has no effect
    /w44555 # expression has no effect
    #/w44668 # checking undefined preprocessor value with #if 
    #/w44365 # int->uint conversion
    /w44350 # warning when binding an rvalue to a non-const-ref
    /w44867 # pointer to member function was not initialized
    /w44986 # exception specification does not match previous...
    /w45038 # member initialization order
  )  
else()
  target_compile_options( ${BASICS_EXAMPLE_NAME} PRIVATE -Wall -Wextra -pedantic )
endif()

target_include_directories( ${BASICS_EXAMPLE_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include" )
