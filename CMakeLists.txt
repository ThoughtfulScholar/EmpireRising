cmake_minimum_required(VERSION 3.26)

# Numele executabilului (trebuie să fie același cu EXECUTABLE_NAME din cmake.yml)
set(MAIN_PROJECT_NAME "oop")
set(MAIN_EXECUTABLE_NAME "${MAIN_PROJECT_NAME}")

project(${MAIN_PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Scripturile interne ale template-ului (PĂSTREAZĂ-LE!)
include(cmake/Options.cmake)
include(cmake/CompilerFlags.cmake)
include(cmake/CopyHelper.cmake)

###############################################################################
# 1. Secțiunea FetchContent pentru Raylib și Raylib-cpp
include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        5.0
        GIT_SHALLOW    1
)

FetchContent_Declare(
        raylib_cpp
        GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git
        GIT_TAG        v5.0.0
        GIT_SHALLOW    1
)

FetchContent_MakeAvailable(raylib raylib_cpp)

###############################################################################
# 2. Definirea executabilului
add_executable(${MAIN_EXECUTABLE_NAME}
        main.cpp
)

# Dacă ai și alte fișiere .cpp în folderul src, adaugă-le aici:
target_sources(${MAIN_EXECUTABLE_NAME}
        PRIVATE
        src/Example.cpp
)

# Flag-urile de compilare ale profesorului (PĂSTREAZĂ-LE!)
set_compiler_flags(RUN_SANITIZERS ${USE_ASAN} TARGET_NAMES ${MAIN_EXECUTABLE_NAME})

###############################################################################
# 3. Include-uri și Linkare (Aici e magia pentru Raylib)

# Folosim SYSTEM ca să nu ne dea erori de la codul lor intern
target_include_directories(${MAIN_EXECUTABLE_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

target_include_directories(${MAIN_EXECUTABLE_NAME} SYSTEM PRIVATE
        ext
        ${raylib_SOURCE_DIR}/src
        ${raylib_cpp_SOURCE_DIR}/include
)

# Linkăm librăriile
target_link_libraries(${MAIN_EXECUTABLE_NAME} PRIVATE raylib raylib-cpp)

# Rezolvăm eroarea de Linux pe GitHub (RandR headers etc.)
if(UNIX AND NOT APPLE)
    target_link_libraries(${MAIN_EXECUTABLE_NAME} PRIVATE
            X11 Xcursor Xinerama Xi Xrandr GL m pthread dl rt
    )
endif()

###############################################################################
# 4. Instalare și Copiere fișiere (PĂSTREAZĂ-LE!)
install(TARGETS ${MAIN_EXECUTABLE_NAME} DESTINATION ${DESTINATION_DIR})

if(APPLE)
    install(FILES launcher.command DESTINATION ${DESTINATION_DIR})
endif()

# Copiază assets-urile (imagini, sunete) în folderul unde rulează jocul
copy_files(FILES tastatura.txt DIRECTORY assets COPY_TO_DESTINATION TARGET_NAME ${MAIN_EXECUTABLE_NAME})