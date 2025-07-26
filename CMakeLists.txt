cmake_minimum_required(VERSION 3.10)

project(AIPOTranslator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Исходники
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "source/*.cpp" "source/*.h" "source/*.hpp" "source/*.inl" "source/*.tpp"
)

# Библиотека
add_library(core STATIC ${SOURCES})
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/source)

# Основное приложение
add_executable(${PROJECT_NAME} source/main.cpp)  # или просто ${SOURCES}
target_link_libraries(${PROJECT_NAME} PRIVATE core)

# Группы файлов для VS
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

# Тесты
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "tests/*.cpp" "tests/*.h" "tests/*.hpp")
add_executable(unit_tests ${TEST_SOURCES})
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/source)
target_link_libraries(unit_tests PRIVATE core)

file(COPY ${CMAKE_SOURCE_DIR}/poForTests
     DESTINATION ${CMAKE_BINARY_DIR})

add_custom_command(TARGET unit_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/poForTests
            $<TARGET_FILE_DIR:unit_tests>/poForTests
)