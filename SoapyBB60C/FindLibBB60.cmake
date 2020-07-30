########################################################################
# Find the library for the BB60C Hardware Driver
########################################################################

INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_BB60C libbb_api)

FIND_PATH(
    BB60C_INCLUDE_DIRS
    NAMES bb_api.h
    HINTS $ENV{BB60C_DIR}/include
        ${PC_BB60C_INCLUDEDIR}
    PATHS /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    BB60C_LIBRARIES
    NAMES bb_api
    HINTS $ENV{BB60C_DIR}/lib
        ${PC_BB60C_LIBDIR}
    PATHS /usr/local/lib
          /usr/lib
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BB60C DEFAULT_MSG BB60C_LIBRARIES BB60C_INCLUDE_DIRS)
MARK_AS_ADVANCED(BB60C_LIBRARIES BB60C_INCLUDE_DIRS)
