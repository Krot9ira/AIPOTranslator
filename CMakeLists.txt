cmake_minimum_required(VERSION 3.10)

project(AIPOTranslator)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source - exclude ThirdParty and UI directories to avoid compiling unnecessary files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "source/*.cpp" "source/*.h" "source/*.hpp" "source/*.inl" "source/*.tpp"
)

# Filter out ThirdParty and UI directories - we'll add them separately
list(FILTER SOURCES EXCLUDE REGEX "source/ThirdParty/.*")
list(FILTER SOURCES EXCLUDE REGEX "source/UI/.*")

# GLAD
add_library(glad source/ThirdParty/glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/source/ThirdParty/glad/include)

# GLFW
add_subdirectory(source/ThirdParty/glfw EXCLUDE_FROM_ALL)

# ImGui
set(IMGUI_SRC
    source/ThirdParty/imgui/imgui.cpp
    source/ThirdParty/imgui/imgui_draw.cpp
    source/ThirdParty/imgui/imgui_tables.cpp
    source/ThirdParty/imgui/imgui_widgets.cpp
    source/ThirdParty/imgui/backends/imgui_impl_glfw.cpp
    source/ThirdParty/imgui/backends/imgui_impl_opengl3.cpp
    source/ThirdParty/imgui/misc/cpp/imgui_stdlib.cpp
)

# ImGuiManager
set(IMGUI_MANAGER_SRC
    source/UI/ImGuiManager.cpp
)

# Third-party include directories
set(THIRDPARTY_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/source/ThirdParty)
set(NLOHMANN_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/source/ThirdParty)
set(GLAD_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/source/ThirdParty/glad/include)
set(IMGUI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/source/ThirdParty/imgui)
set(IMGUI_BACKENDS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/source/ThirdParty/imgui/backends)
set(GLFW_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/source/ThirdParty/glfw/include)

# lib
add_library(core STATIC ${SOURCES})
target_include_directories(core PUBLIC 
    ${CMAKE_SOURCE_DIR}/source
    ${NLOHMANN_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}
    ${IMGUI_BACKENDS_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
)
#curl
find_package(CURL REQUIRED)
#xlnt
find_package(xlnt CONFIG REQUIRED)
#libzip (used to read the theme palette from .xlsx for cell-color detection)
find_package(libzip CONFIG REQUIRED)
target_link_libraries(core PUBLIC CURL::libcurl xlnt::xlnt libzip::zip)

add_library(imgui ${IMGUI_SRC})

target_include_directories(imgui PUBLIC
    ${IMGUI_INCLUDE_DIR}
    ${IMGUI_BACKENDS_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
)
target_link_libraries(imgui PUBLIC glfw)

add_library(imguimanager STATIC ${IMGUI_MANAGER_SRC})
target_include_directories(imguimanager PUBLIC 
    ${CMAKE_SOURCE_DIR}/source
    ${NLOHMANN_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}
    ${IMGUI_BACKENDS_INCLUDE_DIR}
)
target_link_libraries(imguimanager PUBLIC core imgui glad)

# Main
add_executable(${PROJECT_NAME} source/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/source
    ${NLOHMANN_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}
    ${IMGUI_BACKENDS_INCLUDE_DIR}
)
target_link_libraries(${PROJECT_NAME} PRIVATE core imguimanager glad)



# Grouping files for VS
foreach(FILE ${SOURCES})
    file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}/source" "${FILE}")
    get_filename_component(FOLDER "${REL_PATH}" PATH)
    string(REPLACE "/" "\\" GROUP "${FOLDER}")
    if(NOT GROUP STREQUAL "")
        source_group("source\\${GROUP}" FILES "${FILE}")
    else()
        source_group("source" FILES "${FILE}")
    endif()
endforeach()



# Tests
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "tests/*.cpp" "tests/*.h" "tests/*.hpp")
add_executable(unit_tests ${TEST_SOURCES})
target_include_directories(unit_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/source
    ${NLOHMANN_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}
    ${IMGUI_BACKENDS_INCLUDE_DIR}
)
target_link_libraries(unit_tests PRIVATE core)

file(COPY ${CMAKE_SOURCE_DIR}/poForTests
     DESTINATION ${CMAKE_BINARY_DIR})

add_custom_command(TARGET unit_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/poForTests
            $<TARGET_FILE_DIR:unit_tests>/poForTests
)

