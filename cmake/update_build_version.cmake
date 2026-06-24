# cmake/update_build_version.cmake
#
# Reads src/version.h, increments VSCPWORKS_BUILD_VERSION by 1,
# and updates VSCPWORKS_DISPLAY_VERSION to match.
# Invoked automatically as a pre-build step via CMakeLists.txt.

set(VERSION_FILE "${SOURCE_DIR}/src/version.h")

file(READ "${VERSION_FILE}" VERSION_CONTENTS)

# Extract current values
string(REGEX MATCH "#define VSCPWORKS_MAJOR_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(MAJOR ${CMAKE_MATCH_1})

string(REGEX MATCH "#define VSCPWORKS_MINOR_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(MINOR ${CMAKE_MATCH_1})

string(REGEX MATCH "#define VSCPWORKS_RELEASE_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(RELEASE ${CMAKE_MATCH_1})

string(REGEX MATCH "#define VSCPWORKS_BUILD_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(BUILD ${CMAKE_MATCH_1})

math(EXPR NEW_BUILD "${BUILD} + 1")

# Update VSCPWORKS_BUILD_VERSION
string(REGEX REPLACE
  "(#define VSCPWORKS_BUILD_VERSION[ \t]+)[0-9]+"
  "\\1${NEW_BUILD}"
  VERSION_CONTENTS "${VERSION_CONTENTS}"
)

# Update VSCPWORKS_DISPLAY_VERSION
string(REGEX REPLACE
  "(#define VSCPWORKS_DISPLAY_VERSION[ \t]+\")[0-9]+\\.[0-9]+\\.[0-9]+-[0-9]+"
  "\\1${MAJOR}.${MINOR}.${RELEASE}-${NEW_BUILD}"
  VERSION_CONTENTS "${VERSION_CONTENTS}"
)

file(WRITE "${VERSION_FILE}" "${VERSION_CONTENTS}")

message(STATUS "Build version: ${MAJOR}.${MINOR}.${RELEASE}-${NEW_BUILD}")
