cmake_minimum_required(VERSION 3.16)

project(Zombos LANGUAGES CXX C)

# ------------------------------------------------------------
# General compiler settings
# ------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------
# Defaults for flags that should always be enabled  
# ------------------------------------------------------------
if (APPLE AND NOT CMAKE_PREFIX_PATH)
	set(CMAKE_PREFIX_PATH "/opt/homebrew/")
endif()

# ------------------------------------------------------------
# Find dependencies
# ------------------------------------------------------------
find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

# ------------------------------------------------------------
# Gather all source files
# ------------------------------------------------------------
file(GLOB_RECURSE GAME_FILES "${CMAKE_SOURCE_DIR}/dat/*.cpp")
file(GLOB_RECURSE ANTIBOX_FILES "${CMAKE_SOURCE_DIR}/Libraries/include/antibox/*.cpp")

# ------------------------------------------------------------
# Build the executable
# ------------------------------------------------------------

add_executable(Zombos main.cpp)
target_sources(Zombos PRIVATE ${GAME_FILES})
target_include_directories(Zombos PRIVATE
	# ${CMAKE_SOURCE_DIR}/Libraries/include
  ${CMAKE_SOURCE_DIR}/dat/
)
#target_sources(Zombos PRIVATE
# ${CMAKE_SOURCE_DIR}/Libraries/src/glad.c
#)
# antibox.h

set(ASSET_SOURCE_DIR ${CMAKE_SOURCE_DIR}/dat)
set(ASSET_DEST_DIR   ${CMAKE_BINARY_DIR}/bin/dat)

add_custom_command(
    TARGET Zombos POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${ASSET_SOURCE_DIR}
            ${ASSET_DEST_DIR}
)

add_custom_command(
    TARGET Zombos POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
			${CMAKE_SOURCE_DIR}/imgui.ini
			${CMAKE_BINARY_DIR}/bin
)
add_library(miniaudio INTERFACE 
	${CMAKE_SOURCE_DIR}/Libraries/include/miniaudio/miniaudio.h
)

add_library(antibox STATIC ${ANTIBOX_FILES})

target_include_directories(antibox
  PUBLIC
    ${CMAKE_SOURCE_DIR}/Libraries/include
)

add_library(ImGui STATIC
	${CMAKE_SOURCE_DIR}/Libraries/include/imgui/imgui.cpp
	${CMAKE_SOURCE_DIR}/Libraries/include/imgui/imgui_widgets.cpp
	${CMAKE_SOURCE_DIR}/Libraries/include/imgui/imgui_draw.cpp
	${CMAKE_SOURCE_DIR}/Libraries/include/imgui/imgui_tables.cpp
	${CMAKE_SOURCE_DIR}/Libraries/include/imgui/imgui_impl_opengl3.cpp
	${CMAKE_SOURCE_DIR}/Libraries/include/imgui/imgui_impl_glfw.cpp
)

target_include_directories(ImGui
  PUBLIC
    ${CMAKE_SOURCE_DIR}/Libraries/include
)

add_library(glad STATIC
	${CMAKE_SOURCE_DIR}/Libraries/include/glad/glad.c
)
set_property(TARGET glad PROPERTY LINKER_LANGUAGE C)

target_include_directories(glad
  PUBLIC
    ${CMAKE_SOURCE_DIR}/Libraries/include
)

# ------------------------------------------------------------
# Linker and compiler flags
# ------------------------------------------------------------
target_compile_definitions(Zombos PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

target_link_libraries(glad PUBLIC OpenGL::GL)
target_link_libraries(ImGui PUBLIC glfw OpenGL::GL)
target_link_libraries(antibox PUBLIC ImGui glad)
target_link_libraries(Zombos PRIVATE antibox)

# ------------------------------------------------------------
# Platform specific linking
# ------------------------------------------------------------
if(APPLE)
  target_link_libraries(glad PUBLIC OpenGL::GL)
  target_link_libraries(ImGui PUBLIC glfw OpenGL::GL)
  target_link_libraries(Zombos PRIVATE "-framework OpenGL")
endif()

if(WIN32)
  target_link_libraries(Zombos PRIVATE opengl32)
endif()

if(UNIX AND NOT APPLE)
  target_link_libraries(glad PUBLIC OpenGL::GL)
  target_link_libraries(ImGui PUBLIC glfw OpenGL::GL)
 target_link_libraries(glad PUBLIC dl)
 target_link_libraries(Zombos PRIVATE pthread)
endif()


target_link_libraries(antibox PUBLIC ImGui glad)
target_link_libraries(Zombos PRIVATE antibox)
# ------------------------------------------------------------
# Install / runtime configuration
# ------------------------------------------------------------
set_target_properties(Zombos PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
