cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0042 NEW)
set (CMAKE_CXX_STANDARD 14)

project (hello)

set(PUERTS_LIB_ROOT ../../puerts_libs)
set(PUERTS_INCLUDE ${PUERTS_LIB_ROOT}/include)
set(PUERTS_SRC ${PUERTS_LIB_ROOT}/src)


file(GLOB SOURCE_FILES "src/hello.cc", "src/TestClass.cc", "src/AdvanceTestClass.cc")

include_directories(
    ${PUERTS_INCLUDE} # for template support
    "src"
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${PUERTS_SRC}/pesapi_adpt.c)

target_compile_definitions (${PROJECT_NAME} PRIVATE BUILDING_PES_EXTENSION) 
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
