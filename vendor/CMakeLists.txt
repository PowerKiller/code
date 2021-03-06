# TODO: Write a lib detector that does the following: Search for cmake definitions; search for pkg-cfg's; search for libs and headers; everyting may be specified multiple times

# NOTE: Locally built dependencies need to go to the bottom
# so all the others are available and the GLIBC C++11 ABI detection
# works (it needs a list of libraries to check whether they use the new ABI)

include(ExternalProject)
find_package(PkgConfig)

set(pwd "${CMAKE_CURRENT_SOURCE_DIR}")

#### Threads

if (NOT THREADAPI)
  find_package(Threads REQUIRED)
  
  if(WIN32 AND CMAKE_USE_WIN32_THREADS_INIT)
    set(THREADAPI "win32" CACHE INTERN "")
  elseif(CMAKE_USE_PTHREADS_INIT OR CMAKE_HP_PTHREADS_INIT)
    set(THREADAPI "pthreads" CACHE INTERN "")
  endif()
endif()

if (THREADAPI STREQUAL "pthreads")
  set(THREADAPI_PTHREADS 1 CACHE INTERN "")

  find_libs(PTHREADS_LIBS pthread pthreadGC2 winpthread)
  find_path(PTHREADS_INCLUDE_DIRS pthread.h)

elseif(THREADAPI STREQUAL "win32")
  set(THREADAPI_WIN32 1 CACHE INTERN "")
endif()
register_possible_dependency(${PTHREADS_LIBS})

message(STATUS "THREADAPI : ${THREADAPI}")

function(config_threads targ)
  message(STATUS "Configuring ${targ} with Threads (${THREADAPI})")
  if (";${ARGN};" MATCHES ";NOLINK;")
    set(NOLINK "NOLINK")
  endif()

  if (THREADAPI_PTHREADS)
    target_link_libs(${targ} ${PTHREADS_LIBS} ${NOLINK})
    include_directories(${PTHREADS_INCLUDE_DIRS})
  elseif(THREADAPI_WIN32)
    # pass
  else()
    message(SEND_ERROR "Unkown thread API: ${THREADAPI}")
  endif()

endfunction()
#### OpenGL

find_package(OpenGL)

set(OPENGL_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR} CACHE INTERN "")
set(OPENGL_LIBS ${OPENGL_gl_LIBRARY} CACHE INTERN "")
register_possible_dependency(${OPENGL_LIBs})

function(config_opengl targ)
  message(STATUS "Configuring ${targ} with OpenGL (${OPENGL_LIBS})")
  if (";${ARGN};" MATCHES ";NOLINK;")
    set(NOLINK "NOLINK")
  endif()

  if (OPENGL_INCLUDE_DIRS)
    include_directories("${OPENGL_INCLUDE_DIRS}")
  endif()

  target_link_libs(${targ} ${OPENGL_LIBS} ${NOLINK})
endfunction()

#### SDL

find_libs(SDL2_LIBS SDL2 LIB SDL2_image LIB SDL2_mixer)
register_possible_dependency(${SDL2_LIBS})

find_path(SDL2_BASE_INCLUDE_DIRS  SDL.h       PATH_SUFFIXES SDL2)
find_path(SDL2_IMAGE_INCLUDE_DIRS SDL_image.h PATH_SUFFIXES SDL2)
find_path(SDL2_MIXER_INCLUDE_DIRS SDL_mixer.h PATH_SUFFIXES SDL2)
set(SDL2_INCLUDE_DIRS
  ${SDL2_BASE_INCLUDE_DIRS}
  ${SDL2_IMAGE_INCLUDE_DIRS}
  ${SDL2_MIXER_INCLUDE_DIRS} CACHE INTERN "")

function(config_sdl targ)
  message(STATUS "Configuring ${targ} with SDL")
  if (";${ARGN};" MATCHES ";NOLINK;")
    set(NOLINK "NOLINK")
  endif()

  include_directories(${SDL2_INCLUDE_DIRS})
  target_link_libs(${targ} ${SDL2_LIBS} ${NOLINK})

  if(OS_WINDOWS)
    target_link_libs(${targ} winmm ${NOLINK})
    if(NOT MSVC)
      add_definitions(-mwindows) # This is GUI!
    endif()
  elseif(OS_POSIX)
    target_link_libs(${targ} dl rt ${NOLINK})
  endif()

  config_opengl(${targ} ${NOLINK})
endfunction()

#### ZLIB

find_package(ZLIB)

if (${ZLIB_FOUND})
  set(ZLIB_LIBS ${ZLIB_LIBRARY} CACHE INTERN "")
  set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR} CACHE INTERN "")
else()
  find_libs(ZLIB_LIBS z zlib1)
  find_path(ZLIB_INCLUDE_DIRS zlib.h)

  message(STATUS "ZLIB_LIBRARY2 = ${ZLIB_LIBRARY2}")
  message(STATUS "ZLIB_INCLUDE_DIR2 = ${ZLIB_INCLUDE_DIR2}")

  if (EXISTS ${ZLIB_LIBRARY} AND EXISTS ${ZLIB_INCLUDE_DIR})
    set(ZLIB_FOUND 1)
  else()
    message(ERROR "Could not find zlib")
  endif()
endif()

register_possible_dependency(${ZLIB_LIBS})

function(config_zlib targ)
  message(STATUS "Configuring ${targ} with ZLIB")
  if (";${ARGN};" MATCHES ";NOLINK;")
    set(NOLINK "NOLINK")
  endif()

  target_link_libs(${targ} ${ZLIB_LIBS} ${NOLINK})
  include_directories(${ZLIB_INCLUDE_DIRS})
endfunction()

#### ENET

find_libs(ENET_LIBS enet)
find_path(ENET_INCLUDE_DIRS enet/enet.h)

register_possible_dependency(${ENET_LIBS})

function(config_enet targ)
  message(STATUS "Configuring ${targ} with ENET")
  if (";${ARGN};" MATCHES ";NOLINK;")
    set(NOLINK "NOLINK")
  endif()

  include_directories(${ENET_INCLUDE_DIRS})
  target_link_libs(${targ} ${ENET_LIBS} ${NOLINK})
  if (OS_WINDOWS)
    # Why does enet need winmm
    target_link_libs(${targ} ws2_32 winmm ${NOLINK})
  endif()
endfunction()

#### BOOST (System)

find_libs(BOOST_SYSTEM_LIBS boost_system)
find_path(BOOST_SYSTEM_INCLUDE_DIRS boost/system/config.hpp)

register_possible_dependency(${BOOST_SYSTEM_LIBS})

function(config_boost_system targ)
  message(STATUS "Configuring ${targ} with Boost System")

  target_link_libs(${targ} ${BOOST_SYSTEM_LIBS})
  target_include_directories(${targ} PUBLIC ${BOOST_SYSTEM_INCLUDE_DIRS})
endfunction()

#### BOOST (Thread)

find_path(BOOST_THREAD_INCLUDE_DIRS boost/thread.hpp)

if (THREADAPI_WIN32)
  find_libs(BOOST_THREAD_LIBS boost_thread_win32 boost_thread)
else()
  find_libs(BOOST_THREAD_LIBS boost_thread)
endif()

register_possible_dependency(${BOOST_THREAD_LIBS})

function(config_boost_thread targ)
  message(STATUS "Configuring ${targ} with Boost Thread")
  if (";${ARGN};" MATCHES ";NOLINK;")
    set(NOLINK "NOLINK")
  endif()

  config_boost_system(${targ} ${NOLINK})
  config_threads(${targ} ${NOLINK})

  add_definitions(-DBOOST_THREAD_LIB)

  if (THREADAPI_PTHREADS)
    add_definitions(-DBOOST_THREAD_PTHREAD -DBOOST_THREAD_POSIX)
  elseif(THREADAPI_WIN32)
    #add_definitions(-DBOOST_THREAD_WIN32) # redundant
  endif()

  target_link_libs(${targ} ${BOOST_THREAD_LIBS} ${NOLINK})
  target_include_directories(${targ} PUBLIC ${BOOST_THREAD_INCLUDE_DIRS})
endfunction()

#### BOOST (Random)

find_libs(BOOST_RANDOM_LIBS boost_random)
find_path(BOOST_RANDOM_INCLUDE_DIRS boost/random.hpp)

register_possible_dependency(${BOOST_RANDOM_LIBS})

function(config_boost_random targ)
  message(STATUS "Configuring ${targ} with Boost Random")
  if (";${ARGN};" MATCHES ";NOLINK;")
    set(NOLINK "NOLINK")
  endif()

  config_boost_system(${targ} ${NOLINK})

  include_directories(${BOOST_RANDOM_INCLUDE_DIRS})
  target_link_libs(${targ} ${BOOST_RANDOM_LIBS} ${NOLINK})
endfunction()

#### BOOST (ASIO)

find_path(ASIO_INCLUDE_DIRS boost/asio.hpp)

function(config_asio targ)
  message(STATUS "Configuring ${targ} with Boost Asio")
  if (";${ARGN};" MATCHES ";NOLINK;")
    set(NOLINK "NOLINK")
  endif()

  include_directories(${ASIO_INCLUDE_DIRS})

  if (WIN32)
    target_link_libs(${targ} ws2_32 ${NOLINK})
  endif()

  config_boost_system(${targ})
endfunction()

function(config_boost_asio)
  config_asio(${ARGN})
endfunction()

#### Protobuf

find_libs(PROTOBUF_LIBRARIES protobuf)
find_path(PROTOBUF_INCLUDE_DIRS google/protobuf/service.h)

register_possible_dependency(${PROTOBUF_LIBRARIES})

if (NOT DEFINED PROTOC_EXE)
  find_program(PROTOC_EXE protoc)
endif()

function(config_protobuf)
  message(STATUS "Configuring ${targ} with protobuf")
  if (";${ARGN};" MATCHES ";NOLINK;")
    set(NOLINK "NOLINK")
  endif()

  add_definitions(-DGOOGLE_PROTOBUF_NO_RTTI)
  include_directories(${PROTOBUF_INCLUDE_DIRS} ${RPC_INCLUDE_DIRS})
  target_link_libs(${targ} ${NOLINK} ${PROTOBUF_LIBRARIES})
endfunction()
#### Googletest

set(GTEST_DIR "${pwd}/googletest" CACHE INTERNAL "")
set(GTEST_INCLUDE_DIRS "${GTEST_DIR}/include"
  CACHE INTERNAL "")

check_GLIBC_CXX11_AB1()

set(gtest_cxx_flags "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=${_GLIBCXX_USE_CXX11_ABI}")

# NOTE: For some reason compiling with pthreads fails for win32;
# can we fix this?
set(gtest_args
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
      -DCMAKE_CXX_FLAGS=${gtest_cxx_flags}
      -Dgtest_disable_pthreads=true)

if (CMAKE_TOOLCHAIN_FILE)
  list(APPEND gtest_args
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
endif()

ExternalProject_Add( build_gtest
  SOURCE_DIR "${GTEST_DIR}"
  INSTALL_COMMAND ""
  CMAKE_ARGS ${gtest_args})

# This is a bit of a hack to find the actual location of the
# built library
ExternalProject_Get_Property(build_gtest BINARY_DIR)
set(GTEST_LIB_DIR ${BINARY_DIR} CACHE INTERN "")

function(config_gtest targ)
  message(STATUS "Configuring ${targ} with google test")
  if (";${ARGN};" MATCHES ";NOLINK;")
    set(NOLINK "NOLINK")
  endif()

  config_cxx11_abi(${targ} ${NOLINK})

  add_dependencies(${targ} build_gtest)

  include_directories(${GTEST_INCLUDE_DIRS})
  target_link_libs(${targ} gtest ${NOLINK})
endfunction()
