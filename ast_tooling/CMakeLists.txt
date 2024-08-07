CMAKE_MINIMUM_REQUIRED(VERSION 2.13.4)
PROJECT(AST_TOOLING)

# GENERATE CXX FLAGS
SET(CMAKE_CXX_FLAGS "-std=c++14 -fno-rtti -fPIC -fcolor-diagnostics")

IF(APPLE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden -mmacosx-version-min=12.0")
ENDIF()


IF(AST_TOOLING_BUILD_TYPE STREQUAL "Release")
    SET(CMAKE_CXX_FLAGS "-O3 -DNDEBUG ${CMAKE_CXX_FLAGS}")
ELSE()
    SET(CMAKE_CXX_FLAGS "-O0 -g ${CMAKE_CXX_FLAGS}")
ENDIF()

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(AST_TOOLING_VERSION "1.0")

# CONFIG LLVM
IF(LLVM_ROOT)
    IF(NOT EXISTS ${LLVM_ROOT}/include/llvm)
        MESSAGE(FATAL_ERROR "LLVM_ROOT (${LLVM_ROOT}) is not a valid LLVM install. Could not find ${LLVM_ROOT}/include/llvm")
    ENDIF()
    MESSAGE("LLVM_ROOT: ${LLVM_ROOT}")
    IF(EXISTS ${LLVM_ROOT}/lib/cmake/llvm)
        SET(LLVM_DIR ${LLVM_ROOT}/lib/cmake/llvm)
    ELSE()
        SET(LLVM_DIR ${LLVM_ROOT}/share/llvm/cmake)
    ENDIF()
    SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LLVM_DIR}")
    INCLUDE(LLVMConfig)
ELSE()
    MESSAGE(FATAL_ERROR "LLVM_ROOT is required")
ENDIF()

INCLUDE_DIRECTORIES( ${LLVM_INCLUDE_DIRS} )
LINK_DIRECTORIES( ${LLVM_LIBRARY_DIRS} )
ADD_DEFINITIONS( ${LLVM_DEFINITIONS} )


STRING(REGEX MATCH "[0-9]+\\.[0-9]+(\\.[0-9]+)?" LLVM_VERSION_RELEASE ${LLVM_PACKAGE_VERSION})
MESSAGE(STATUS "Found LLVM LLVM_PACKAGE_VERSION: ${LLVM_PACKAGE_VERSION} - LLVM_VERSION_RELEASE: ${LLVM_VERSION_RELEASE}")
MESSAGE(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
LLVM_MAP_COMPONENTS_TO_LIBNAMES(REQ_LLVM_LIBRARIES asmparser bitreader instrumentation mcparser option support frontendopenmp)

SET(CLANG_LIBRARIES
    clangToolingCore
    clangTooling
    clangFrontend
    clangDriver
    clangSerialization
    clangParse
    clangSema
    clangAnalysis
    clangEdit
    clangASTMatchers
    clangAST
    clangLex
    clangBasic)


INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

ADD_SUBDIRECTORY(lib)
ADD_EXECUTABLE(astlint main.cpp)

TARGET_LINK_LIBRARIES(astlint
    ASToolingDriver
    clangStaticAnalyzerFrontend
    clangStaticAnalyzerCheckers
    clangStaticAnalyzerCore
    clangRewriteFrontend
    clangRewrite
    clangCrossTU
    clangIndex
    ${CLANG_LIBRARIES}
    ${REQ_LLVM_LIBRARIES}
    )
