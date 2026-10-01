# The port: the decompiled game (decomp/, glue/) on modern systems, over
# miniwin (port/miniwin/: the Win32 subset the game uses, implemented with
# SDL2) and a host layer (port/host/: the few things that differ per target).
#
# Normally driven by `uv run port build`, but plain CMake works too:
#
#   emcmake cmake -S port -B build/port/browser_wasm && cmake --build build/port/browser_wasm
#   cmake -S port -B build/port/macos_universal && cmake --build build/port/macos_universal
#
# A source in port/decomp/ replaces decomp/'s file of the same name (for a
# module whose portable version has to differ from the matching one); none
# does yet.

cmake_minimum_required(VERSION 3.24)
project(zoombinis C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

# The decompiled code is Borland's 32-bit dialect: its longs are 32 bits, and
# where a pointer has to live in an integer it says LONG_PTR (decomp/zoombinis.h).
# On a target whose long is 64 bits (Linux, macOS) miniwin/prelude.h makes the
# game's `long` an int, so its structures keep the original's sizes (see "The
# port" in the handbook); everywhere else (WebAssembly, 32-bit and 64-bit
# Windows) long is already 32 bits.

include(FetchContent)

# stb_truetype renders the game's TrueType font (miniwin/text.cpp).
FetchContent_Declare(stb
    URL https://github.com/nothings/stb/archive/2c980bb59875b0d32144a71867fbdebb2f77cd20.tar.gz
    URL_HASH SHA256=9a955b1b49a4410088a2e0ee2a9c057c3c907d0c1d75454144cb980aca0ba515)
FetchContent_MakeAvailable(stb)

# TinySoundFont plays the game's MIDI (miniwin/midi.cpp).
FetchContent_Declare(tsf
    URL https://github.com/schellingb/TinySoundFont/archive/853a0a171759f1ddba0de1442133a75912bbeffa.tar.gz
    URL_HASH SHA256=76c356df524f71f2a34f67fa0cb2cb946036fe7a65d2930a849569c5e742fa57)
FetchContent_MakeAvailable(tsf)

# SDL2: Emscripten's own port on the web; elsewhere the system's, or a pinned
# release built from source.
if(EMSCRIPTEN)
    add_library(SDL2::SDL2 INTERFACE IMPORTED)
    target_compile_options(SDL2::SDL2 INTERFACE -sUSE_SDL=2)
    target_link_options(SDL2::SDL2 INTERFACE -sUSE_SDL=2)
else()
    # A macOS build is universal (arm64 and x86_64), which a package manager's
    # SDL2 (for the host's architecture only) can't be: build the pinned release
    # there, unless asked to use the system's.
    if(APPLE)
        set(ZB_SYSTEM_SDL_DEFAULT OFF)
    else()
        set(ZB_SYSTEM_SDL_DEFAULT ON)
    endif()
    option(ZB_SYSTEM_SDL "Use the system's SDL2 if there is one" ${ZB_SYSTEM_SDL_DEFAULT})
    if(ZB_SYSTEM_SDL)
        find_package(SDL2 CONFIG QUIET)
    endif()
    if(NOT TARGET SDL2::SDL2)
        FetchContent_Declare(SDL2
            URL https://github.com/libsdl-org/SDL/releases/download/release-2.32.10/SDL2-2.32.10.tar.gz
            URL_HASH SHA256=5f5993c530f084535c65a6879e9b26ad441169b3e25d789d83287040a9ca5165)
        set(SDL_SHARED OFF CACHE BOOL "" FORCE)
        set(SDL_STATIC ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(SDL2)
        # (SDL 2.32 defines SDL2::SDL2 itself, as the static library when only that is built.)
        if(NOT TARGET SDL2::SDL2)
            add_library(SDL2::SDL2 ALIAS SDL2-static)
        endif()
    endif()
endif()

# The game's sources: decomp/ and glue/ (each replaced by port/decomp/'s or
# port/glue/'s file of the same name, if there is one).
file(GLOB DECOMP_SOURCES CONFIGURE_DEPENDS "${REPO_ROOT}/decomp/*.cpp")
file(GLOB PORT_DECOMP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/decomp/*.cpp")
foreach(replacement IN LISTS PORT_DECOMP_SOURCES)
    get_filename_component(name "${replacement}" NAME)
    list(REMOVE_ITEM DECOMP_SOURCES "${REPO_ROOT}/decomp/${name}")
endforeach()
file(GLOB GLUE_SOURCES CONFIGURE_DEPENDS "${REPO_ROOT}/glue/*.cpp")
file(GLOB PORT_GLUE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/glue/*.cpp")
foreach(replacement IN LISTS PORT_GLUE_SOURCES)
    get_filename_component(name "${replacement}" NAME)
    list(REMOVE_ITEM GLUE_SOURCES "${REPO_ROOT}/glue/${name}")
endforeach()
set(GAME_SOURCES ${DECOMP_SOURCES} ${PORT_DECOMP_SOURCES} ${GLUE_SOURCES} ${PORT_GLUE_SOURCES})

# The game's code is Borland C++ 4.5's dialect: miniwin/prelude.h makes clang
# read it the same way (byte packing, the Borland runtime's extras). Its
# chars are signed, as in BCC32 (they aren't by default on ARM); signed
# overflow wraps and pointers alias freely, as the code expects. Warnings are
# off (and the two that are errors by default, `register` and narrowing in
# case labels): modern C++ warns about much that BCC32 accepted, and the
# decompiled code stays as it is.
add_library(game OBJECT ${GAME_SOURCES})
target_include_directories(game PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_options(game PRIVATE
    -include miniwin/prelude.h
    -fsigned-char -fno-exceptions -fno-rtti -fno-strict-aliasing -fwrapv
    -w -Wno-register -Wno-c++11-narrowing)

# ZB_DEBUG: the debug tooling (port/debug/: text commands to jump to a scene,
# set levels, fill the party, edit the saved state; see the handbook). The
# hook in decomp/game.cpp exists only when it's defined, so the matching
# build never sees it.
option(ZB_DEBUG "Build the debug tooling" ON)
if(ZB_DEBUG)
    set(DEBUG_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/debug/zbdebug.cpp")
    # (-iquote: decomp/ has headers named like system ones, e.g. features.h.)
    # (The globals the debug commands read by name: written by `uv run port build`; without it
    # the table is empty.)
    set(ZB_DEBUG_GENERATED_DIR "${CMAKE_BINARY_DIR}/../generated" CACHE PATH "Where debug_globals.inc is")
    set_source_files_properties("${DEBUG_SOURCE}" PROPERTIES COMPILE_OPTIONS
        "-iquote;${REPO_ROOT}/decomp;-iquote;${ZB_DEBUG_GENERATED_DIR}")
    target_sources(game PRIVATE "${DEBUG_SOURCE}")
    target_compile_definitions(game PRIVATE ZB_DEBUG)
endif()

# miniwin: the Win32 API over SDL2.
file(GLOB MINIWIN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/miniwin/*.cpp")
add_library(miniwin STATIC ${MINIWIN_SOURCES})
target_include_directories(miniwin PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}" "${stb_SOURCE_DIR}")
target_include_directories(miniwin SYSTEM PRIVATE "${tsf_SOURCE_DIR}")
target_compile_options(miniwin PRIVATE -fsigned-char -fno-exceptions -fno-rtti -Wall
    -Wno-unused-function)
target_link_libraries(miniwin PRIVATE SDL2::SDL2)

# The host layer: fibers and the event loop, per target. It never sees
# miniwin's headers, so it can use a host's real Win32 API.
if(EMSCRIPTEN)
    set(HOST_SOURCES host/emscripten.cpp)
elseif(WIN32)
    set(HOST_SOURCES host/windows.cpp)
else()
    set(HOST_SOURCES host/posix.cpp)
endif()
add_library(host STATIC ${HOST_SOURCES})
target_include_directories(host PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(host PRIVATE SDL2::SDL2)

# Windows: a program with a window and no console, unless it's a debugging build
# (its debug commands and messages go to the console).
# ZB_WINDOWS_ICON: an .ico for the program (uv run port makes one from the game's).
set(ZB_RESOURCES "")
if(WIN32 AND ZB_WINDOWS_ICON)
    enable_language(RC)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/zoombinis.rc" "1 ICON \"${ZB_WINDOWS_ICON}\"\n")
    set(ZB_RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/zoombinis.rc")
endif()
if(WIN32 AND NOT ZB_DEBUG)
    add_executable(zoombinis WIN32 main.cpp ${ZB_RESOURCES} $<TARGET_OBJECTS:game>)
else()
    add_executable(zoombinis main.cpp ${ZB_RESOURCES} $<TARGET_OBJECTS:game>)
endif()
target_include_directories(zoombinis PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(zoombinis PRIVATE miniwin host SDL2::SDL2)
if(ZB_DEBUG)
    target_compile_definitions(zoombinis PRIVATE ZB_DEBUG)
endif()

# ZB_HEADLESS (Emscripten only): a build for Node, with no screen or sound,
# reading the host's files directly: for testing and debugging in a terminal.
option(ZB_HEADLESS "Build for Node, headless (Emscripten)" OFF)

if(EMSCRIPTEN AND ZB_HEADLESS)
    target_compile_definitions(miniwin PRIVATE ZB_HEADLESS)
    target_link_options(zoombinis PRIVATE
        -sASYNCIFY -sASYNCIFY_STACK_SIZE=262144
        -sSTACK_SIZE=1048576 -sINITIAL_MEMORY=134217728 -sALLOW_MEMORY_GROWTH
        -sENVIRONMENT=node -sNODERAWFS -sEXIT_RUNTIME=1)
elseif(EMSCRIPTEN)
    # Asyncify lets the game's blocking loops (and its fibers) give the
    # browser its turn: see host/emscripten.cpp.
    set_target_properties(zoombinis PROPERTIES SUFFIX ".html")
    target_link_options(zoombinis PRIVATE
        -sASYNCIFY -sASYNCIFY_STACK_SIZE=262144
        -sSTACK_SIZE=1048576 -sINITIAL_MEMORY=134217728 -sALLOW_MEMORY_GROWTH
        -sFORCE_FILESYSTEM -lidbfs.js
        -sEXPORTED_RUNTIME_METHODS=FS,IDBFS,addRunDependency,removeRunDependency
        -sEXIT_RUNTIME=1
        --shell-file "${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html"
        --pre-js "${CMAKE_CURRENT_SOURCE_DIR}/web/pre.js")
    set_property(TARGET zoombinis APPEND PROPERTY LINK_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html" "${CMAKE_CURRENT_SOURCE_DIR}/web/pre.js")
endif()
