# ======================================================================
# Nintendo Switch (libnx / devkitA64) build configuration.
#
# Entered from the top-level CMakeLists.txt when NINTENDO_SWITCH is set
# (i.e. configured with $DEVKITPRO/cmake/Switch.cmake as toolchain).
# The KisakBlack target already exists with the full Windows source list;
# here we swap platform sources and apply GCC/AArch64 options.
# ======================================================================

# 1300+ object files exceed the Windows command-line length limit at link
# time; make cmake pass them via @response files.
set(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS 1)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS 1)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES 1)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES 1)

set(NX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/nx")

file(GLOB NX_SOURCES
    "${NX_DIR}/*.cpp"
    "${NX_DIR}/*.c"
    "${NX_DIR}/*.h"
)

get_target_property(KB_SOURCES ${BIN_NAME} SOURCES)

# Sources that must not build on Switch (replaced by src/nx/ equivalents).
set(NX_EXCLUDED_SOURCES
    ${TRACY_FILES}                                # profiler: x86/OS-specific, Debug-only on PC
    "${SRC_DIR}/win32/win_mini_dumper.cpp"        # dbghelp minidumps -> nx_platform_stubs
    "${SRC_DIR}/win32/win_splash.cpp"             # splash window -> nx_platform_stubs
    "${SRC_DIR}/win32/win_syscon.cpp"             # Win32 console dialog -> nx_platform_stubs
    "${SRC_DIR}/win32/win_voice.cpp"              # waveIn/mixer voice chat -> nx_platform_stubs
    "${SRC_DIR}/binklib/dx9rad3d.cpp"             # Bink RAD3D-over-D3D9 (unreferenced)
    "${SRC_DIR}/binklib/binktextures.cpp"         # Bink texture helpers (unreferenced; Bink stubbed)
    "${SRC_DIR}/sound/snd_driver_xaudio2.cpp"     # XAudio2 driver -> nx_snd_null
    "${SRC_DIR}/sound/snd_driver_xaudio2_dsp.cpp" # XAPO DSP effects -> nx_snd_null
    "${SRC_DIR}/groupvoice/play_dsound.cpp"       # DirectSound playback -> stubs
    "${SRC_DIR}/groupvoice/record_dsound.cpp"     # DirectSound capture -> stubs
    "${SRC_DIR}/vpx/vpx.cpp"                      # VP8 clip encoder -> nx_platform_stubs
)
list(REMOVE_ITEM KB_SOURCES ${NX_EXCLUDED_SOURCES})
list(APPEND KB_SOURCES ${NX_SOURCES})
set_target_properties(${BIN_NAME} PROPERTIES SOURCES "${KB_SOURCES}")

# ----- Preprocessor defines -----
target_compile_definitions(${BIN_NAME} PUBLIC
    KISAK_MP
    KISAK_NX
    _CRT_SECURE_NO_WARNINGS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# ----- Include directories -----
# Compat headers (windows.h, d3d9.h, ...) must be found before anything else.
target_include_directories(${BIN_NAME} BEFORE PUBLIC "${NX_DIR}/compat")
target_include_directories(${BIN_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/libs"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/libs/libtomcrypt-1.17/src/headers"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/libs/libtommath-1.0"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/libs/libvpx-1.5.0/include"
)

# ----- Compile options -----
# -fsigned-char        : MSVC/x86 char is signed; AArch64 defaults to unsigned.
# -fno-strict-aliasing : decompiled code type-puns heavily.
# -fwrapv              : rely on wrapping signed overflow like MSVC/x86 in practice.
# -fms-extensions      : anonymous structs/unions and other MSVC-isms.
# -include nx_prefix.h : maps MSVC keywords/intrinsics for every TU.
set(NX_COMMON_FLAGS
    -fsigned-char
    -fno-strict-aliasing
    -fwrapv
    -fms-extensions
    -ffunction-sections
    -fdata-sections
    "SHELL:-include ${NX_DIR}/compat/nx_prefix.h"
)
target_compile_options(${BIN_NAME} PRIVATE
    ${NX_COMMON_FLAGS}
)

# ----- Warning policy: sanitised sources build with warnings on -----
#
# The decompiled tree casts pointers through 32-bit ints in thousands of places.
# On x86 that was lossless; on LP64 every one of them truncates. The compiler
# diagnoses all of them, and -w has been hiding them.
#
# -w cannot be undone by a later flag, so it is applied PER FILE to everything
# that has not been cleaned yet, rather than to the whole target. A file
# graduates by being listed in NX_SANITIZED_SOURCES; from then on it compiles
# with warnings on, and a pointer truncation is a hard error:
#
#   C++  the cast is ill-formed without -fpermissive, so it errors on its own
#        ("cast from 'const char*' to 'int' loses precision")
#   C    -Wpointer-to-int-cast / -Wint-to-pointer-cast are C-only options, so
#        they are requested explicitly as errors
#
# Keep the list additive. Removing a file from it is a regression.
# See docs/lp64-sweeps/README.md, "Closing the class one subsystem at a time".
set(NX_SANITIZED_SOURCES ${NX_SOURCES})
list(FILTER NX_SANITIZED_SOURCES INCLUDE REGEX "\.(c|cpp)$")

# Graduated decompiled sources, newest last.
list(APPEND NX_SANITIZED_SOURCES
    "${SRC_DIR}/universal/com_expressions_eval.cpp"  # menu expression evaluator
    "${SRC_DIR}/gfx_d3d/r_material.cpp"              # material registry / duplication
)

get_target_property(NX_UNSANITIZED_SOURCES ${BIN_NAME} SOURCES)
list(REMOVE_ITEM NX_UNSANITIZED_SOURCES ${NX_SANITIZED_SOURCES})

set_source_files_properties(${NX_UNSANITIZED_SOURCES} PROPERTIES
    COMPILE_OPTIONS "-w;-fpermissive;-Wno-narrowing")

set(NX_SANITIZED_C_SOURCES ${NX_SANITIZED_SOURCES})
list(FILTER NX_SANITIZED_C_SOURCES INCLUDE REGEX "\.c$")
if(NX_SANITIZED_C_SOURCES)
    set_source_files_properties(${NX_SANITIZED_C_SOURCES} PROPERTIES
        COMPILE_OPTIONS "-Werror=pointer-to-int-cast;-Werror=int-to-pointer-cast")
endif()

list(LENGTH NX_SANITIZED_SOURCES NX_SANITIZED_COUNT)
list(LENGTH NX_UNSANITIZED_SOURCES NX_UNSANITIZED_COUNT)
message(STATUS
    "LP64 warning policy: ${NX_SANITIZED_COUNT} sanitised sources build with "
    "warnings on, ${NX_UNSANITIZED_COUNT} still build with -w")

# ----- Link options / libs -----
target_link_options(${BIN_NAME} PRIVATE
    -Wl,--gc-sections
    # path-normalizing file wrappers (see src/nx/nx_wincompat.cpp)
    -Wl,--wrap=fopen
    -Wl,--wrap=remove
    -Wl,--wrap=rename
)

target_link_libraries(${BIN_NAME} PRIVATE nx)

# ----- NRO packaging -----
nx_create_nro(${BIN_NAME}
    NAME    "KisakBlack"
    AUTHOR  "SwagSoftware"
    VERSION "0.1.0"
)
