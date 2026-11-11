# cmake/update_build_version.cmake
#
# Derives the public CalVer version from the current calendar month and
# increments the build counter. The patch version is the resulting build number.
# Invoked automatically as a POST_BUILD step via CMakeLists.txt.

set(VERSION_FILE   "${SOURCE_DIR}/src/version.h")
set(DOC_VARS_FILE  "${SOURCE_DIR}/docs/variables.xml")

file(READ "${VERSION_FILE}"  VERSION_CONTENTS)
file(READ "${DOC_VARS_FILE}" DOC_VARS_CONTENTS)

# The patch component is also the monotonically increasing build number.
string(REGEX MATCH "#define VSCPWORKS_PATCH_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(BUILD ${CMAKE_MATCH_1})

math(EXPR NEW_BUILD "${BUILD} + 1")
set(PATCH_COUNT "${NEW_BUILD}")

string(TIMESTAMP CURRENT_TIMESTAMP "%Y-%m-%d")
string(REPLACE "-" ";" CURRENT_DATE_PARTS "${CURRENT_TIMESTAMP}")
list(GET CURRENT_DATE_PARTS 0 CURRENT_YEAR)
list(GET CURRENT_DATE_PARTS 1 CURRENT_MONTH)
math(EXPR CURRENT_YEAR_MOD "${CURRENT_YEAR} % 100")

set(PUBLIC_MAJOR "${CURRENT_YEAR_MOD}")
set(PUBLIC_MINOR "${CURRENT_MONTH}")
string(REGEX REPLACE "^0" "" NUMERIC_MINOR "${PUBLIC_MINOR}")

# Update version.h with the current calendar-version macros.
string(REGEX REPLACE
  "(#define VSCPWORKS_MAJOR_VERSION[ \t]+)[0-9]+"
  "\\1${PUBLIC_MAJOR}"
  VERSION_CONTENTS "${VERSION_CONTENTS}"
)
string(REGEX REPLACE
  "(#define VSCPWORKS_MINOR_VERSION[ \t]+)[0-9]+"
  "\\1${NUMERIC_MINOR}"
  VERSION_CONTENTS "${VERSION_CONTENTS}"
)
string(REGEX REPLACE
  "(#define VSCPWORKS_PATCH_VERSION[ \t]+)[0-9]+"
  "\\1${PATCH_COUNT}"
  VERSION_CONTENTS "${VERSION_CONTENTS}"
)

# Update the display version in the same header.
string(REGEX REPLACE
  "(#define VSCPWORKS_DISPLAY_VERSION[ \t]+\")[0-9]+-[0-9]+-[0-9]+"
  "\\1${PUBLIC_MAJOR}-${PUBLIC_MINOR}-${PATCH_COUNT}"
  VERSION_CONTENTS "${VERSION_CONTENTS}"
)
file(WRITE "${VERSION_FILE}" "${VERSION_CONTENTS}")

string(REGEX REPLACE
  "(<document-version>)[^<]+(</document-version>)"
  "\\1${PUBLIC_MAJOR}.${NUMERIC_MINOR}.${PATCH_COUNT}\\2"
  DOC_VARS_CONTENTS "${DOC_VARS_CONTENTS}"
)
file(WRITE "${DOC_VARS_FILE}" "${DOC_VARS_CONTENTS}")

message(STATUS "Build version: ${PUBLIC_MAJOR}-${PUBLIC_MINOR}-${PATCH_COUNT}")
