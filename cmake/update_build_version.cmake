# cmake/update_build_version.cmake
#
# Reads src/version.h for MAJOR/MINOR/RELEASE, reads src/buildnumber.h for the
# current BUILD counter, increments BUILD by 1, and rewrites src/buildnumber.h.
# version.h is never modified, so only the two .cpp files that include
# buildnumber.h are recompiled on the next build.
# Invoked automatically as a POST_BUILD step via CMakeLists.txt.

set(VERSION_FILE   "${SOURCE_DIR}/src/version.h")
set(BUILDNUM_FILE  "${SOURCE_DIR}/src/buildnumber.h")

file(READ "${VERSION_FILE}"  VERSION_CONTENTS)
file(READ "${BUILDNUM_FILE}" BUILDNUM_CONTENTS)

# Extract MAJOR / MINOR / RELEASE from version.h
string(REGEX MATCH "#define VSCPWORKS_MAJOR_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(MAJOR ${CMAKE_MATCH_1})

string(REGEX MATCH "#define VSCPWORKS_MINOR_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(MINOR ${CMAKE_MATCH_1})

string(REGEX MATCH "#define VSCPWORKS_RELEASE_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(RELEASE ${CMAKE_MATCH_1})

# Extract current BUILD from buildnumber.h
string(REGEX MATCH "#define VSCPWORKS_BUILD_VERSION[ \t]+([0-9]+)" _ "${BUILDNUM_CONTENTS}")
set(BUILD ${CMAKE_MATCH_1})

math(EXPR NEW_BUILD "${BUILD} + 1")

# Update VSCPWORKS_BUILD_VERSION in buildnumber.h
string(REGEX REPLACE
  "(#define VSCPWORKS_BUILD_VERSION[ \t]+)[0-9]+"
  "\\1${NEW_BUILD}"
  BUILDNUM_CONTENTS "${BUILDNUM_CONTENTS}"
)

# Update VSCPWORKS_DISPLAY_VERSION in buildnumber.h
string(REGEX REPLACE
  "(#define VSCPWORKS_DISPLAY_VERSION[ \t]+\")[0-9]+\\.[0-9]+\\.[0-9]+-[0-9]+"
  "\\1${MAJOR}.${MINOR}.${RELEASE}-${NEW_BUILD}"
  BUILDNUM_CONTENTS "${BUILDNUM_CONTENTS}"
)

file(WRITE "${BUILDNUM_FILE}" "${BUILDNUM_CONTENTS}")

message(STATUS "Build version: ${MAJOR}.${MINOR}.${RELEASE}-${NEW_BUILD}")
