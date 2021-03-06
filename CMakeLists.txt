cmake_minimum_required(VERSION 2.8.12) 
include(cmake/functions.cmake)
include(cmake/glibc_cxx11_abi_detection.cmake)

### Different Configuration Types (Debug release)

# Only generate Debug and Release configuration types.
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# First choice is Debug
if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE INTERN "")
endif()


message(STATUS "CMAKE_VERSION = ${CMAKE_VERSION}")
message(STATUS "CMAKE_GENERATOR = ${CMAKE_GENERATOR}")

# Set a default toolchain file
string(TOLOWER "${CMAKE_GENERATOR}" GEN)
if(GEN MATCHES ".*visual studio.*")
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/platform/vs.cmake)
elseif(GEN MATCHES ".*mingw.*" OR MINGW)
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/platform/mingw.cmake)
else()
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/platform/linux.cmake)
endif()

message(STATUS "Using Toolchain File ${CMAKE_TOOLCHAIN_FILE}")

# Project Name
project(Inexor)

# Use folders in the resulting project files.
set_property(GLOBAL PROPERTY OS_FOLDERS ON)

# Warning if paths aren't set well
get_filename_component(MAINDIR "${CMAKE_SOURCE_DIR}" REALPATH)  # Main inexor folder
get_filename_component(BINDIR "${CMAKE_BINARY_DIR}" REALPATH)   # Where it generates the projects into

if(${MAINDIR} STREQUAL ${BINDIR})
    message(FATAL_ERROR "Keep your directories clean, don't generate the project-files in the main directory! ${MAINDIR} ${BINDIR}")
endif()

# Set the path to inexor/ explicitly to simplify some following definitions
set(SOURCE_DIR ${MAINDIR}/inexor)

### Platform Detection:

# Determine the project architecture.
if(NOT DEFINED PROJECT_ARCH)
  if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PROJECT_ARCH "x86_64")
  else()
    set(PROJECT_ARCH "x86")
  endif()

  message(STATUS "No PROJECT_ARCH value specified, using ${PROJECT_ARCH}")
endif()

if (PROJECT_ARCH STREQUAL "x86_64")
  set(X64 1)
else()
  set(X32 1)
endif()

# Determine the platform.
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  set(OS_MACOSX 1)
  set(OS_POSIX 1)
  message(STATUS "Platform: OSX ${PROJECT_ARCH}")
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  set(OS_LINUX 1)
  set(OS_POSIX 1)
  message(STATUS "Platform: Linux ${PROJECT_ARCH}")
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  set(OS_WINDOWS 1)
  message(STATUS "Platform: Windows ${PROJECT_ARCH}")
endif()

# Allow C++ programs to use stdint.h macros specified in the C99 standard that
# aren't in the C++ standard (e.g. UINT8_MAX, INT64_MIN, etc).
add_definitions(-D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS)

#### GCC, Clang ####

# TODO: Unify flags and lib detection on ALL plattforms as much as possible
# TODO: Use a more declarative style, like flags(COMPILER CXX MINGW += ... )\n flags(LINKER POSIX += ...) ...
if(OS_LINUX OR (OS_WINDOWS AND NOT MSVC))
    set(I_LIBTYPE                 SHARED)
    # -fomit-frame-pointer            = Don't keep the frame pointer for functions that don't need one
    # -fno-strict-aliasing            = Avoid assumptions regarding non-aliasing of objects of different types
    # -fsigned-char                   = Let the type char be signed, like signed char.
    # -pipe                           = Use pipes rather than temporary files for communication between build stages
    # -Wall                           = Enable all warnings
    # -Wno-switch                     = Execpt Warnings for missing switch cases
    # -fdiagnostics-show-option       = Show Warning IDs
    # -g                              = Generate debug information
    set(I_COMPILER_FLAGS          "-fomit-frame-pointer -fno-strict-aliasing -fsigned-char -pipe -Wall -Wno-switch -Wno-deprecated-declarations -g -Wno-missing-braces -fdiagnostics-show-option")
    # -std=c99                        = Use the C99 language standard
    set(I_C_COMPILER_FLAGS        "-std=c99")
    # -fno-threadsafe-statics         = Don't generate thread-safe statics
    # -std=c++11                      = Use the C++11 language standard
    set(I_CXX_COMPILER_FLAGS      "-fno-threadsafe-statics -std=c++14")
    # -O0                             = No optimization
    # -D_DEBUG                        = Debug build
    set(I_COMPILER_FLAGS_DEBUG    "-O0 -D_DEBUG")
    # -O3                             = Optimize for maximum speed
    # -fdata-sections                 = Enable linker optimizations to improve locality of reference for data sections
    # -ffunction-sections             = Enable linker optimizations to improve locality of reference for function sections
    # -fno-ident                      = Ignore the #ident directive
    # -DNDEBUG                        = Not a debug build
    # -U_FORTIFY_SOURCE               = Undefine _FORTIFY_SOURCE in case it was previously defined
    # -D_FORTIFY_SOURCE=1             = Add memory and string function protection (security feature)
    set(I_COMPILER_FLAGS_RELEASE  "-O3 -fdata-sections -ffunction-sections -fno-ident -DNDEBUG -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=1")
    # -Wl,-rpath,.                    = Set rpath so that libraries can be placed next to the executable
    set(I_LINKER_FLAGS            "-Wl,-rpath,.")
    # -Wl,-O1                         = Enable linker optimizations
    # -Wl,--as-needed                 = Only link libraries that export symbols used by the binary
    # -Wl,--gc-sections               = Remove unused code resulting from -fdata-sections and -function-sections
    set(I_LINKER_FLAGS_RELEASE    "-Wl,-O1 -Wl,--as-needed -Wl,--gc-sections")

    if(X64)
        # 64-bit architecture.
        set(I_COMPILER_FLAGS        "${I_COMPILER_FLAGS} -m64 -march=x86-64")
        set(I_LINKER_FLAGS          "${I_LINKER_FLAGS} -m64")
    else()
        # 32-bit architecture.
        set(I_COMPILER_FLAGS        "${I_COMPILER_FLAGS} -msse2 -mfpmath=sse -mmmx -m32")
        set(I_LINKER_FLAGS          "${I_LINKER_FLAGS} -m32")
    endif()

    # Allow the Large File Support (LFS) interface to replace the old interface.
    add_definitions(-D_FILE_OFFSET_BITS=64)

    # Define where to move the binary by install
    set(EXE_SUB_DIR "bin/linux/${PROJECT_ARCH}")
endif()

#### Visual Studio. ####

if(OS_WINDOWS)

    ## Cross compilation flags already set
    if(MSVC)
        set(I_LIBTYPE                 STATIC)
        # /MP                                   = Multiprocess compilation
        # /Gy                                   = Enable function-level linking
        # /Zi                                   = Enable program database
        # /fp:fast                              = Calculate with less precise floating point values
        # /EHsc                                 = assume extern C stuff never throws exceptions
        # /W2                                   = Warning level 2
        # /wd"4305"                             = Ignore "truncation from 'double' to 'float'" warning
        # /wd"4251"                             = Ignore "class 'type' needs to have dll-interface .." warning
        # /wd"4091"                             = Ignore "typedef .. where no variable gets declared ingored" warning
        # /wd"4996"                             = Ignore "Function call with parameters that may be unsafe" warning
        set(I_COMPILER_FLAGS          "-MP -Gy -Zi -fp:fast -EHsc -W2 -wd\"4305\" -wd\"4251\" -wd\"4996\" -wd\"4091\"")
        # /MTd                                  = Multithreaded debug runtime
        # /Od                                   = Disable optimizations
        # /RTC1                                 = Enable basic run-time checks
        # -D_DEBUG                              = Debug build
        set(I_COMPILER_FLAGS_DEBUG    "-MTd -RTC1 -Od -D_DEBUG")
        # /MT                                   = Multithreaded release runtime
        # /O2                                   = Optimize for maximum speed
        # /Ob2                                  = Inline any suitable function
        # /GF                                   = Enable string pooling
        # /D NDEBUG /D _NDEBUG                  = Not a debug build, skip code passages meant for debug.
        # /GL                                   = Whole Program Optimization: further optimize the program in linker stage (but calling the compiler).
        set(I_COMPILER_FLAGS_RELEASE  "-MT -O2 -Ob2 -GF -DNDEBUG -D_NDEBUG -GL")
        # /DEBUG                                = Generate debug information (even for release builds for our stackwalker)
        # /MANIFEST:NO                          = No default manifest
        # /SAFESEH:NO                           = Do Not Create a table of safe exception handlers (binary incompatible)
        set(I_LINKER_FLAGS            "-MANIFEST:NO -SAFESEH:NO -DEBUG ")
        # /OPT:REF                              = Eliminate Unreferenced Data (to revert changes to the binary caused by /DEBUG)
        # /OPT:ICF                              = Remove Redundant COMDATs (to revert changes to the binary caused by /DEBUG)
        # /INCREMENTAL:NO                       = Linking incremental (faster) does not work anymore with the previous two optimizations
        # /LTCG                                 = Link-time Code Generation: further optimisations in the linker stage.
        set(I_LINKER_FLAGS_RELEASE    "-LTCG -OPT:REF -OPT:ICF -INCREMENTAL:NO")

        # Add the library path as well
        foreach(dir ${CMAKE_LIBRARY_PATH}) # We need to do this seperately since LIBPATH does not accept more than one dir at once
            set(I_EXE_LINKER_FLAGS "${I_EXE_LINKER_FLAGS} -LIBPATH:${dir}")
        endforeach()
    endif()

    # Standard definitions
    # -DWIN32 -D_WIN32 -D_WINDOWS           = Windows platform
    # -DNOMINMAX                            = Use the standard's templated min/max
    # -D_WIN32_WINNT=0x0501                 = Target is Windows XP
    add_definitions(-DWIN32 -D_WIN32 -D_WINDOWS -DWINDOWS -DNOMINMAX -D_WIN32_WINNT=0x0501)
    if(X64)
        add_definitions(-DWIN64)
    endif()

    # Currently we only support VS 2015
    if(MSVC AND NOT MSVC_VERSION EQUAL 1900)
        message(WARNING "Use Visual Studio 2015 in order to compile Inexor. (Otherwise you would need to recompile the dependencies yourself)")
    endif()

    # Rename project_arch from x86_64 to 64 and x86 to 32 on windows:
    if(X64)
        set(PROJECT_ARCH "64")
    else()
        set(PROJECT_ARCH "32")
    endif()

    # Define where to move the binary by install
    set(EXE_SUB_DIR "bin/windows/win${PROJECT_ARCH}")
endif()

# Ability to use your environment own cflags/ldflags.. as well
if(NOT MSVC)
    option( USE_ENVFLAGS "Enable or Disable Use of environment own Compiler/Linker-Flags" ON)

    if(USE_ENVFLAGS)
        set(I_C_COMPILER_FLAGS      "${I_C_COMPILER_FLAGS} $ENV{CFLAGS}")
        set(I_CXX_COMPILER_FLAGS    "${I_CXX_COMPILER_FLAGS} $ENV{CXXFLAGS}")
        set(I_LINKER_FLAGS          "${I_LINKER_FLAGS} $ENV{LDFLAGS}")
    endif()
endif()

# Merge compiler/linker flags.
set(CMAKE_C_FLAGS                     "${I_COMPILER_FLAGS} ${I_C_COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS                   "${I_COMPILER_FLAGS} ${I_CXX_COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG             "${I_COMPILER_FLAGS_DEBUG} ${I_CXX_COMPILER_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE           "${I_COMPILER_FLAGS_RELEASE} ${I_CXX_COMPILER_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS            "${I_LINKER_FLAGS} ${I_EXE_LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG      "${I_LINKER_FLAGS_DEBUG} ${I_EXE_LINKER_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE    "${I_LINKER_FLAGS_RELEASE} ${I_EXE_LINKER_FLAGS_RELEASE}")

message(STATUS "Current mode:                   ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler:                       ${CMAKE_CXX_COMPILER_ID}")

message(STATUS "Compiler definitions:           ${DEFINITIONS}")

message(STATUS "C_FLAGS:                        ${CMAKE_C_FLAGS}")
message(STATUS "CXX_FLAGS:                      ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG:                ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE:              ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "EXE_LINKER_FLAGS:               ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "EXE_LINKER_FLAGS_DEBUG:         ${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
message(STATUS "EXE_LINKER_FLAGS_RELEASE:       ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

### Install paths

# Set output path for the compiled executable
set(EXE_DIR "${PROJECT_SOURCE_DIR}/${EXE_SUB_DIR}")

set(INSTALL_RESOURCES_DIR "${PROJECT_SOURCE_DIR}/bin/all")

message(STATUS "Generating Project-Files to ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS " - Resulting Executable goes to ${EXE_DIR}")

add_subdirectory(vendor)
add_subdirectory(inexor)

install(FILES ${INSTALL_SHARED_LIBS} ${INSTALL_EXES} DESTINATION ${EXE_DIR})
message(STATUS "INSTALL_RESOURCES       = ${INSTALL_RESOURCES}")
install(FILES ${INSTALL_RESOURCES} DESTINATION ${INSTALL_RESOURCES_DIR})

# shipped portable stuff on win (dirty but temporarily)
if(OS_WINDOWS)
  install(DIRECTORY ${INSTALL_PORTABLE_DIR} DESTINATION "${PROJECT_SOURCE_DIR}/bin/windows/all")
endif()
