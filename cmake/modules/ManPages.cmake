# ManPages.cmake
#
# Man pages from Markdown with pandoc.
#
#   include(ManPages)
#   vscp_add_man_pages(${PROJECT_SOURCE_DIR}/man)              # all pages
#   vscp_add_man_pages(${PROJECT_SOURCE_DIR}/man prog.1 progd.8)  # these only
#
# Every <name>.<section>.md in the folder (or the pages named) (pandoc's man format, starting with
# a "% NAME(SECTION) ..." title block) is converted with
#   pandoc -s -t man <name>.<section>.md -o <name>.<section>
# when the project is built and installed in ${CMAKE_INSTALL_MANDIR}/man<section>
# (share/man/man1 for commands, man8 for daemons). Without pandoc the man
# pages are left out with a warning. Turn them off with -DBUILD_MAN_PAGES=OFF.
#
# Man pages are made on Unix-like systems only.
#

include(GNUInstallDirs)

option(BUILD_MAN_PAGES "Build and install the man pages (needs pandoc)" ${UNIX})

function(vscp_add_man_pages dir)
  if(NOT BUILD_MAN_PAGES OR NOT UNIX)
    return()
  endif()

  find_program(PANDOC_EXECUTABLE pandoc DOC "pandoc, converts the Markdown man pages")
  if(NOT PANDOC_EXECUTABLE)
    message(WARNING "pandoc was not found, the man pages are not built. "
                    "Install pandoc (e.g. apt install pandoc) or set -DBUILD_MAN_PAGES=OFF.")
    return()
  endif()

  if(ARGN)
    set(_man_sources "")
    foreach(_p IN LISTS ARGN)
      list(APPEND _man_sources "${dir}/${_p}.md")
    endforeach()
  else()
    file(GLOB _man_sources CONFIGURE_DEPENDS "${dir}/*.[1-8].md")
  endif()
  set(_man_pages "")
  foreach(_src IN LISTS _man_sources)
    get_filename_component(_name "${_src}" NAME)
    string(REGEX REPLACE "\\.md$" "" _page "${_name}")
    string(REGEX MATCH "[1-8]$" _section "${_page}")
    set(_out "${CMAKE_CURRENT_BINARY_DIR}/man/${_page}")
    add_custom_command(
      OUTPUT "${_out}"
      COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/man"
      COMMAND "${PANDOC_EXECUTABLE}" -s -t man "${_src}" -o "${_out}"
      DEPENDS "${_src}"
      COMMENT "Man page ${_page}"
      VERBATIM)
    list(APPEND _man_pages "${_out}")
    install(FILES "${_out}" DESTINATION "${CMAKE_INSTALL_MANDIR}/man${_section}")
  endforeach()

  if(_man_pages)
    add_custom_target(man-pages ALL DEPENDS ${_man_pages})
    list(LENGTH _man_pages _count)
    message(STATUS "Man pages: ${_count} (pandoc ${PANDOC_EXECUTABLE})")
  endif()
endfunction()
