if(NOT UNIX OR APPLE)
  return()
endif()

if(NOT DEFINED PROJECT_NAME)
  set(PROJECT_NAME "vscp-works-qt")
endif()

if(NOT DEFINED CMAKE_SOURCE_DIR)
  message(FATAL_ERROR "CMAKE_SOURCE_DIR is not available to package_appimage.cmake")
endif()

if(NOT DEFINED CMAKE_BINARY_DIR)
  message(FATAL_ERROR "CMAKE_BINARY_DIR is not available to package_appimage.cmake")
endif()

if(NOT DEFINED CPACK_PACKAGE_VERSION)
  set(CPACK_PACKAGE_VERSION "0.0.0")
endif()

set(APP_DIR "${CMAKE_BINARY_DIR}/AppDir")
set(OUTPUT_APPIMAGE "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-x86_64.AppImage")

file(REMOVE_RECURSE "${APP_DIR}")
file(MAKE_DIRECTORY "${APP_DIR}/usr")

execute_process(
  COMMAND "${CMAKE_COMMAND}" --install "${CMAKE_BINARY_DIR}" --prefix "${APP_DIR}/usr"
  RESULT_VARIABLE install_result
  OUTPUT_VARIABLE install_output
  ERROR_VARIABLE install_error
)

if(NOT install_result EQUAL 0)
  message(FATAL_ERROR "Failed to install project into AppDir:\n${install_output}\n${install_error}")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/install/AppRun")
  file(COPY "${CMAKE_SOURCE_DIR}/install/AppRun" DESTINATION "${APP_DIR}")
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/install/vscp-works-qt.desktop")
  file(COPY "${CMAKE_SOURCE_DIR}/install/vscp-works-qt.desktop" DESTINATION "${APP_DIR}")
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/icons/vscpworks.png")
  file(COPY "${CMAKE_SOURCE_DIR}/icons/vscpworks.png" DESTINATION "${APP_DIR}")
endif()
file(MAKE_DIRECTORY "${APP_DIR}/usr/share/applications")
if(EXISTS "${CMAKE_SOURCE_DIR}/install/vscp-works-qt.desktop")
  file(COPY "${CMAKE_SOURCE_DIR}/install/vscp-works-qt.desktop" DESTINATION "${APP_DIR}/usr/share/applications")
endif()
file(MAKE_DIRECTORY "${APP_DIR}/usr/share/icons/hicolor/256x256/apps")
if(EXISTS "${CMAKE_SOURCE_DIR}/icons/vscpworks.png")
  file(COPY "${CMAKE_SOURCE_DIR}/icons/vscpworks.png" DESTINATION "${APP_DIR}/usr/share/icons/hicolor/256x256/apps")
endif()

set(ENV{QML_SOURCES_PATHS} "${CMAKE_SOURCE_DIR}/src")

find_program(LINUXDEPLOYQT_EXECUTABLE NAMES linuxdeployqt linuxdeployqt-continuous-x86_64.AppImage)
if(NOT LINUXDEPLOYQT_EXECUTABLE)
  message(STATUS "linuxdeployqt not found; skipping AppImage generation")
  return()
endif()

execute_process(
  COMMAND "${LINUXDEPLOYQT_EXECUTABLE}" "${APP_DIR}/usr/share/applications/vscp-works-qt.desktop" -appimage -bundle-non-qt-libs
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  RESULT_VARIABLE deploy_result
  OUTPUT_VARIABLE deploy_output
  ERROR_VARIABLE deploy_error
)

if(NOT deploy_result EQUAL 0)
  message(WARNING "AppImage creation failed; continuing with the archive package.\n${deploy_output}\n${deploy_error}")
  return()
endif()

file(GLOB APPIMAGE_FILES "${CMAKE_BINARY_DIR}/*.AppImage")
if(APPIMAGE_FILES)
  list(LENGTH APPIMAGE_FILES APPIMAGE_COUNT)
  if(APPIMAGE_COUNT GREATER 0)
    list(GET APPIMAGE_FILES 0 APPIMAGE_SOURCE)
    file(RENAME "${APPIMAGE_SOURCE}" "${OUTPUT_APPIMAGE}")
    message(STATUS "Created AppImage: ${OUTPUT_APPIMAGE}")
  endif()
endif()
