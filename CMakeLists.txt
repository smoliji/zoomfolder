cmake_minimum_required(VERSION 3.20)
if(APPLE)
    project(zoomfolder C OBJC)
else()
    project(zoomfolder C)
endif()
set(CMAKE_C_STANDARD 17)

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
)
FetchContent_Declare(nfd
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

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
    target_sources(zoomfolder PRIVATE src/scanner_posix.c)
elseif(WIN32)
    target_sources(zoomfolder PRIVATE src/scanner_win32.c)
else()
    target_sources(zoomfolder PRIVATE src/scanner_posix.c)
endif()

add_custom_command(TARGET zoomfolder POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:zoomfolder>/fonts
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/fonts/Inter-Regular.ttf
        $<TARGET_FILE_DIR:zoomfolder>/fonts/Inter-Regular.ttf
)

# Tests
enable_testing()

add_executable(test_tree tests/test_tree.c src/tree.c)
target_include_directories(test_tree PRIVATE src)
add_test(NAME test_tree COMMAND test_tree)

add_executable(test_scanner tests/test_scanner.c src/tree.c)
target_include_directories(test_scanner PRIVATE src)
target_link_libraries(test_scanner PRIVATE SDL3::SDL3)
if(WIN32)
    target_sources(test_scanner PRIVATE src/scanner_win32.c)
else()
    target_sources(test_scanner PRIVATE src/scanner_posix.c)
endif()
add_test(NAME test_scanner COMMAND test_scanner)
