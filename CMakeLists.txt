# ============================================================================
#  LearnOpenGL - TEST project
#
#  Links the GLFW library that lives ONE LEVEL UP from this folder:
#
#      learnOpenGL_cn/
#      |-- threePartyLibs/          <- prebuilt GLFW (the only dependency)
#      |   |-- include/GLFW/glfw3.h
#      |   |-- include/GLFW/glfw3native.h
#      |   |-- include/glad/glad.h
#      |   `-- libs/glfw3.lib       (x64 Release, /MD)
#      |       libs/glfw3d.lib      (x64 Debug,   /MDd)
#      `-- TEST/                    <- THIS project
#          |-- CMakeLists.txt
#          `-- src/main.cpp
#
#  Quick start A -- Ninja (NO .sln is generated):
#      # run inside an x64 MSVC environment, e.g. one of:
#      #   * "x64 Native Tools Command Prompt for VS 2022"
#      #   * ...\VC\Auxiliary\Build\vcvarsall.bat x64   (cmd)
#      cmake -S . -B build -G Ninja
#      cmake --build build
#      build\bin\LearnOpenGL_TEST.exe
#
#  Quick start B -- Visual Studio generator (generates a .sln):
#      cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64
#      cmake --build build-vs --config Debug
#      build-vs\bin\Debug\LearnOpenGL_TEST.exe
#
#  NOTE: both prebuilt libraries are 64-bit (x64) MSVC builds -- Debug links
#  glfw3d.lib (/MDd), Release links glfw3.lib (/MD). A 32-bit configuration is
#  rejected below. Rebuild them from glfw-3.5.1 to change this.
# ============================================================================
cmake_minimum_required(VERSION 3.16)

project(LearnOpenGL_TEST
        VERSION 0.1.0
        LANGUAGES C CXX
        DESCRIPTION "OpenGL learning sandbox that links the prebuilt GLFW library")

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Sources are UTF-8 (they contain Chinese comments). MSVC otherwise assumes the
# local ANSI code page (936/GBK) and can mis-parse multibyte comment bytes, which
# may silently swallow the NEXT line of code -- a classic cause of bizarre
# runtime behaviour. /utf-8 makes MSVC read sources as UTF-8 (and silences the
# C4819 warning).
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Single-config generators (Ninja, Makefiles) leave CMAKE_BUILD_TYPE empty by
# default; pick Debug (the usual choice while learning).
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug/Release/RelWithDebInfo/MinSizeRel)" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

# Keep the .exe in a generator-independent place: <build>/bin[/<config>]
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Emit compile_commands.json so editors / language servers (clangd, VS Code
# C/C++, Cursor, Neovim, ...) learn the same include paths the compiler uses --
# this is what makes "go to definition" on <GLFW/glfw3.h> work.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Sources -- every .cpp / .c directly under src/ is picked up automatically, so
# you can drop in a new file (e.g. src/shader.cpp) without editing this file.
#
#   * add_executable() cannot take a directory; file(GLOB) is how you get the
#     "just use the whole folder" behavior.
#   * CONFIGURE_DEPENDS makes Ninja/Make re-run CMake when files are added or
#     removed, so new sources are noticed without a manual reconfigure.
#   * Headers are not listed (they are pulled in via #include).
#   * To also scan sub-folders, swap GLOB for GLOB_RECURSE.
# ---------------------------------------------------------------------------
file(GLOB LEARNOPENGL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_executable(LearnOpenGL_TEST ${LEARNOPENGL_SOURCES})

# ---------------------------------------------------------------------------
# GLFW -- prebuilt static library, one directory level up (../threePartyLibs).
# This project only *links* the prebuilt library; it never builds GLFW itself.
# ---------------------------------------------------------------------------
get_filename_component(LEARN_OPENGL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

set(GLFW_PREBUILT_DIR "${LEARN_OPENGL_ROOT}/threePartyLibs"
    CACHE PATH "Folder holding the prebuilt GLFW (include/ + libs/)")

set(GLFW_INCLUDE_DIR     "${GLFW_PREBUILT_DIR}/include")
set(GLFW_LIBRARY_DEBUG   "${GLFW_PREBUILT_DIR}/libs/glfw3d.lib")   # /MDd
set(GLFW_LIBRARY_RELEASE "${GLFW_PREBUILT_DIR}/libs/glfw3.lib")    # /MD

foreach(_glfw_lib IN ITEMS "${GLFW_LIBRARY_DEBUG}" "${GLFW_LIBRARY_RELEASE}")
    if(NOT EXISTS "${_glfw_lib}")
        message(FATAL_ERROR
            "Prebuilt GLFW library not found:\n  ${_glfw_lib}\n"
            "Expected layout:\n"
            "  ${GLFW_PREBUILT_DIR}/include/GLFW/glfw3.h\n"
            "  ${GLFW_PREBUILT_DIR}/libs/glfw3d.lib   (Debug)\n"
            "  ${GLFW_PREBUILT_DIR}/libs/glfw3.lib    (Release)")
    endif()
endforeach()

# Both prebuilt libraries are 64-bit (x64). Linking one into a 32-bit image
# fails with LNK1112/LNK4272, so fail early and readably.
if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR
        "The prebuilt GLFW libraries are 64-bit (x64), but this configuration targets 32-bit.\n"
        "Configure an x64 build instead, e.g.:\n"
        "  cmake -S . -B build -G Ninja                              (x64 dev prompt)\n"
        "  cmake -S . -B build-vs -G \"Visual Studio 17 2022\" -A x64")
endif()

target_include_directories(LearnOpenGL_TEST PRIVATE "${GLFW_INCLUDE_DIR}")

# Match the CRT variant: Debug build -> glfw3d.lib, everything else -> glfw3.lib.
target_link_libraries(LearnOpenGL_TEST PRIVATE
    "$<$<CONFIG:Debug>:${GLFW_LIBRARY_DEBUG}>"
    "$<$<NOT:$<CONFIG:Debug>>:${GLFW_LIBRARY_RELEASE}>")

message(STATUS "GLFW: linking prebuilt library (Debug: ${GLFW_LIBRARY_DEBUG} | Release: ${GLFW_LIBRARY_RELEASE})")

# ---------------------------------------------------------------------------
# System dependencies: OpenGL (opengl32.lib).
# Used by GLFW's WGL backend and by the gl* calls you add as the tutorial
# progresses. Ships with the Windows SDK, so the bare lib name is enough.
# (gdi32/user32/shell32 are already part of MSVC's default library set.)
# ---------------------------------------------------------------------------
target_link_libraries(LearnOpenGL_TEST PRIVATE opengl32)
