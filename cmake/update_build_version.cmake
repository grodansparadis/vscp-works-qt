# cmake/update_build_version.cmake
#
# Derives the public CalVer version from the current calendar month and the
# number of commits on the master branch during that month. The build counter
# remains separate and is still incremented in buildnumber.h.
# Invoked automatically as a POST_BUILD step via CMakeLists.txt.

set(VERSION_FILE   "${SOURCE_DIR}/src/version.h")
set(BUILDNUM_FILE  "${SOURCE_DIR}/src/buildnumber.h")

file(READ "${VERSION_FILE}"  VERSION_CONTENTS)
file(READ "${BUILDNUM_FILE}" BUILDNUM_CONTENTS)

# Extract existing major/minor/patch macros from version.h as fallbacks.
string(REGEX MATCH "#define VSCPWORKS_MAJOR_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(MAJOR ${CMAKE_MATCH_1})

string(REGEX MATCH "#define VSCPWORKS_MINOR_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(MINOR ${CMAKE_MATCH_1})

string(REGEX MATCH "#define VSCPWORKS_PATCH_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(PATCH ${CMAKE_MATCH_1})

# Extract current BUILD from buildnumber.h
string(REGEX MATCH "#define VSCPWORKS_BUILD_VERSION[ \t]+([0-9]+)" _ "${BUILDNUM_CONTENTS}")
set(BUILD ${CMAKE_MATCH_1})

math(EXPR NEW_BUILD "${BUILD} + 1")

string(TIMESTAMP CURRENT_TIMESTAMP "%Y-%m-%d")
string(REPLACE "-" ";" CURRENT_DATE_PARTS "${CURRENT_TIMESTAMP}")
list(GET CURRENT_DATE_PARTS 0 CURRENT_YEAR)
list(GET CURRENT_DATE_PARTS 1 CURRENT_MONTH)
math(EXPR CURRENT_YEAR_MOD "${CURRENT_YEAR} % 100")

set(PUBLIC_MAJOR "${CURRENT_YEAR_MOD}")
set(PUBLIC_MINOR "${CURRENT_MONTH}")
string(REGEX REPLACE "^0" "" NUMERIC_MINOR "${PUBLIC_MINOR}")
set(PATCH_COUNT "${PATCH}")

if(CURRENT_MONTH EQUAL 12)
  math(EXPR NEXT_YEAR "${CURRENT_YEAR} + 1")
  set(NEXT_MONTH "1")
else()
  set(NEXT_YEAR "${CURRENT_YEAR}")
  math(EXPR NEXT_MONTH "${CURRENT_MONTH} + 1")
endif()

set(MONTH_START "${CURRENT_YEAR}-${CURRENT_MONTH}-01")
set(MONTH_END "${NEXT_YEAR}-${NEXT_MONTH}-01")

execute_process(
  COMMAND git -C "${SOURCE_DIR}" rev-list --count --since "${MONTH_START}" --until "${MONTH_END}" master
  OUTPUT_VARIABLE GIT_PATCH_COUNT
  ERROR_QUIET
  RESULT_VARIABLE GIT_RESULT
)

if(GIT_RESULT EQUAL 0)
  string(STRIP "${GIT_PATCH_COUNT}" GIT_PATCH_COUNT)
  if(GIT_PATCH_COUNT MATCHES "^[0-9]+$")
    set(PATCH_COUNT "${GIT_PATCH_COUNT}")
  endif()
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
file(WRITE "${VERSION_FILE}" "${VERSION_CONTENTS}")

# Update VSCPWORKS_BUILD_VERSION in buildnumber.h
string(REGEX REPLACE
  "(#define VSCPWORKS_BUILD_VERSION[ \t]+)[0-9]+"
  "\\1${NEW_BUILD}"
  BUILDNUM_CONTENTS "${BUILDNUM_CONTENTS}"
)

# Update VSCPWORKS_DISPLAY_VERSION in buildnumber.h
string(REGEX REPLACE
  "(#define VSCPWORKS_DISPLAY_VERSION[ \t]+\")[0-9]+-[0-9]+-[0-9]+"
  "\\1${PUBLIC_MAJOR}-${PUBLIC_MINOR}-${PATCH_COUNT}"
  BUILDNUM_CONTENTS "${BUILDNUM_CONTENTS}"
)

file(WRITE "${BUILDNUM_FILE}" "${BUILDNUM_CONTENTS}")

message(STATUS "Build version: ${PUBLIC_MAJOR}-${PUBLIC_MINOR}-${PATCH_COUNT} (build ${NEW_BUILD})")
