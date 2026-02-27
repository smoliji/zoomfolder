cmake_minimum_required(VERSION 3.20)
if(APPLE)
    project(zoomfolder C OBJC)
else()
    project(zoomfolder C)
endif()
set(CMAKE_C_STANDARD 17)

if(APPLE)
    set(BUILD_SHARED_LIBS OFF)
endif()

include(FetchContent)

FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)
FetchContent_Declare(SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG main
    GIT_SHALLOW TRUE
    GIT_SUBMODULES external/freetype external/harfbuzz external/plutovg external/plutosvg
)
FetchContent_Declare(nfd
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

set(SDLTTF_VENDORED ON CACHE BOOL "")
FetchContent_MakeAvailable(SDL3 SDL3_ttf nfd)

add_executable(zoomfolder
    src/main.c
    src/tree.c
    src/renderer.c
    src/input.c
    src/font_cache.c
)

target_include_directories(zoomfolder PRIVATE src)
target_link_libraries(zoomfolder PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf nfd)

if(APPLE)
    set_target_properties(zoomfolder PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Zoomfolder"
        MACOSX_BUNDLE_BUNDLE_VERSION "0.1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1.0"
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/Info.plist.in
        OUTPUT_NAME "Zoomfolder"
    )

    set_source_files_properties(${CMAKE_SOURCE_DIR}/fonts/Inter-Regular.ttf
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/fonts"
    )
    set_source_files_properties(${CMAKE_SOURCE_DIR}/assets/generated/icon.icns
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(zoomfolder PRIVATE
        ${CMAKE_SOURCE_DIR}/fonts/Inter-Regular.ttf
        ${CMAKE_SOURCE_DIR}/assets/generated/icon.icns
    )

    install(TARGETS zoomfolder BUNDLE DESTINATION .)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "Zoomfolder")
    set(CPACK_PACKAGE_FILE_NAME "Zoomfolder-macos")
    include(CPack)
    add_custom_target(dmg COMMAND cpack WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

if(APPLE)
    target_sources(zoomfolder PRIVATE src/scanner_posix.c)
elseif(WIN32)
    target_sources(zoomfolder PRIVATE src/scanner_win32.c ${CMAKE_SOURCE_DIR}/assets/icon.rc)
else()
    target_sources(zoomfolder PRIVATE src/scanner_posix.c)
endif()

if(NOT APPLE)
    add_custom_command(TARGET zoomfolder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:zoomfolder>/fonts
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/fonts/Inter-Regular.ttf
            $<TARGET_FILE_DIR:zoomfolder>/fonts/Inter-Regular.ttf
    )
endif()

# Tests
enable_testing()

add_executable(test_tree tests/test_tree.c src/tree.c)
target_include_directories(test_tree PRIVATE src)
add_test(NAME test_tree COMMAND test_tree)

if(NOT WIN32)
    add_executable(test_scanner tests/test_scanner.c src/tree.c src/scanner_posix.c)
    target_include_directories(test_scanner PRIVATE src)
    target_link_libraries(test_scanner PRIVATE SDL3::SDL3)
    add_test(NAME test_scanner COMMAND test_scanner)
endif()
