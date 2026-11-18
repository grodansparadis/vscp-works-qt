# cmake/update_build_version.cmake
#
# Derives the public CalVer version from the current calendar month. The build
# counter increments within a month and resets when the month changes.
# Invoked automatically by the version-update workflow on pushes to master.

set(VERSION_FILE   "${SOURCE_DIR}/src/version.h")
set(DOC_VARS_FILE  "${SOURCE_DIR}/docs/variables.xml")

file(READ "${VERSION_FILE}"  VERSION_CONTENTS)
file(READ "${DOC_VARS_FILE}" DOC_VARS_CONTENTS)

string(REGEX MATCH "#define VSCPWORKS_MAJOR_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(BUILD_MAJOR ${CMAKE_MATCH_1})
string(REGEX MATCH "#define VSCPWORKS_MINOR_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(BUILD_MINOR ${CMAKE_MATCH_1})
string(REGEX MATCH "#define VSCPWORKS_PATCH_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(BUILD ${CMAKE_MATCH_1})

string(TIMESTAMP CURRENT_TIMESTAMP "%Y-%m-%d")
string(REPLACE "-" ";" CURRENT_DATE_PARTS "${CURRENT_TIMESTAMP}")
list(GET CURRENT_DATE_PARTS 0 CURRENT_YEAR)
list(GET CURRENT_DATE_PARTS 1 CURRENT_MONTH)
math(EXPR CURRENT_YEAR_MOD "${CURRENT_YEAR} % 100")

set(PUBLIC_MAJOR "${CURRENT_YEAR_MOD}")
set(PUBLIC_MINOR "${CURRENT_MONTH}")
string(REGEX REPLACE "^0" "" NUMERIC_MINOR "${PUBLIC_MINOR}")

if(BUILD_MAJOR EQUAL PUBLIC_MAJOR AND BUILD_MINOR EQUAL NUMERIC_MINOR)
  math(EXPR PATCH_COUNT "${BUILD} + 1")
else()
  set(PATCH_COUNT 0)
endif()

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
  "(#define VSCPWORKS_DISPLAY_VERSION[ \t]+\")[0-9]+[-.][0-9]+[-.][0-9]+"
  "\\1${PUBLIC_MAJOR}.${PUBLIC_MINOR}.${PATCH_COUNT}"
  VERSION_CONTENTS "${VERSION_CONTENTS}"
)
file(WRITE "${VERSION_FILE}" "${VERSION_CONTENTS}")

string(REGEX REPLACE
  "(<document-version>)[^<]+(</document-version>)"
  "\\1${PUBLIC_MAJOR}.${NUMERIC_MINOR}.${PATCH_COUNT}\\2"
  DOC_VARS_CONTENTS "${DOC_VARS_CONTENTS}"
)
file(WRITE "${DOC_VARS_FILE}" "${DOC_VARS_CONTENTS}")

message(STATUS "Build version: ${PUBLIC_MAJOR}.${PUBLIC_MINOR}.${PATCH_COUNT}")
