# FindSDL2_ttf.cmake
# Find the SDL2_ttf library
#
# This will define the following variables:
#
# SDL2_TTF_FOUND - SDL2_ttf library was found
# SDL2_TTF_INCLUDE_DIRS - the SDL2_ttf include directory
# SDL2_TTF_LIBRARIES - the SDL2_ttf library

find_package(PkgConfig)
pkg_check_modules(SDL2_TTF QUIET SDL2_ttf)

if (NOT SDL2_TTF_FOUND)
    find_path(SDL2_TTF_INCLUDE_DIR SDL2/SDL_ttf.h)
    find_library(SDL2_TTF_LIBRARY NAMES SDL2_ttf)
    if (SDL2_TTF_INCLUDE_DIR AND SDL2_TTF_LIBRARY)
        set(SDL2_TTF_FOUND TRUE)
        set(SDL2_TTF_INCLUDE_DIRS ${SDL2_TTF_INCLUDE_DIR})
        set(SDL2_TTF_LIBRARIES ${SDL2_TTF_LIBRARY})
    endif ()
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_TTF
        REQUIRED_VARS SDL2_TTF_LIBRARIES SDL2_TTF_INCLUDE_DIRS
        VERSION_VAR SDL2_TTF_VERSION)
mark_as_advanced(SDL2_TTF_LIBRARIES SDL2_TTF_INCLUDE_DIRS)
