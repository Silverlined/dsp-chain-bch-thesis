INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_ITPP itpp)

FIND_PATH(
    ITPP_INCLUDE_DIRS
    NAMES itpp/itbase.h
    HINTS $ENV{ITPP_DIR}/include
        ${PC_ITPP_INCLUDEDIR}
    PATHS /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    ITPP_LIBRARIES
    NAMES itpp
    HINTS $ENV{ITPP_DIR}/lib
        ${PC_ITPP_LIBDIR}
    PATHS /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ITPP DEFAULT_MSG ITPP_LIBRARIES ITPP_INCLUDE_DIRS)
