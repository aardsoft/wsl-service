# cmake --toolchain ../toolchain.cmake ..
# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

IF (NOT TOOLCHAIN)
    SET(TOOLCHAIN "x86_64-w64-mingw32-")
ENDIF()

SET(CMAKE_INCLUDE_PATH  ../gettext/x86_64/usr/include)
# which compilers to use for C and C++
SET(CMAKE_C_COMPILER ${TOOLCHAIN}gcc)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN}g++)
SET(CMAKE_RC_COMPILER ${TOOLCHAIN}windres)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
