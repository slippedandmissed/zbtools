# Cross-compiling for Windows with llvm-mingw (clang and mingw-w64), which has
# builds for Linux, macOS and Windows hosts. `uv run port build windows_x86|windows_x64`
# puts its bin/ directory on the PATH and passes the architecture:
#
#   cmake -S port -B build/port/windows_x64 -DCMAKE_TOOLCHAIN_FILE=port/toolchains/mingw.cmake \
#         -DZB_MINGW_ARCH=x86_64
#
# ZB_MINGW_ARCH is i686 (32-bit Windows, which runs on 64-bit Windows too) or
# x86_64. The compilers are the architecture-prefixed ones llvm-mingw installs
# (i686-w64-mingw32-clang++ ...), so any mingw-w64 clang on the PATH will do.

set(CMAKE_SYSTEM_NAME Windows)
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES ZB_MINGW_ARCH)
if(NOT ZB_MINGW_ARCH)
    message(FATAL_ERROR "set ZB_MINGW_ARCH to i686 or x86_64")
endif()
set(CMAKE_SYSTEM_PROCESSOR ${ZB_MINGW_ARCH})
set(triple ${ZB_MINGW_ARCH}-w64-mingw32)
set(CMAKE_C_COMPILER ${triple}-clang)
set(CMAKE_CXX_COMPILER ${triple}-clang++)
set(CMAKE_RC_COMPILER ${triple}-windres)
# The runtime goes into the program: nothing but the system's DLLs to ship.
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
