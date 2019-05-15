set (dec265_sources
  dec265.cc
)

set (hdrcopy_sources
  hdrcopy.cc
)

if(MSVC)
  set (dec265_sources
    ${dec265_sources}
    ../extra/getopt.c
    ../extra/getopt_long.c
  )
  set (hdrcopy_sources
    ${hdrcopy_sources}
    ../extra/getopt.c
    ../extra/getopt_long.c
  )
endif()

if(SDL_FOUND)
  add_definitions(-DHAVE_SDL)
  include_directories ("${SDL_INCLUDE_DIR}")
  set (dec265_sources
    ${dec265_sources}
    sdl.cc
  )
endif()

add_executable (dec265 ${dec265_sources})

target_link_libraries (dec265 ${LIBDE265_LIBRARY_NAME} ${SDL_LIBRARY})


if(NOT MSVC)
  # hdrcopy uses internal APIs that are not available when compiled for Windows
  add_executable (hdrcopy ${hdrcopy_sources})

  target_link_libraries (hdrcopy ${LIBDE265_LIBRARY_NAME})
endif()
