cmake_minimum_required(VERSION 3.16)

project(Zombos LANGUAGES CXX C)

# ------------------------------------------------------------
# General compiler settings
# ------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------
# Gather all source files (shared across all platforms)
# ------------------------------------------------------------
file(GLOB_RECURSE GAME_FILES "${CMAKE_SOURCE_DIR}/dat/*.cpp")
file(GLOB_RECURSE ANTIBOX_FILES "${CMAKE_SOURCE_DIR}/Libraries/include/antibox/*.cpp")

# ============================================================
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# !!                                                       !!
# !!   ANDROID BUILD BLOCK  --  NEW FOR ANDROID EXPORT     !!
# !!                                                       !!
# !!   When building with the Android NDK CMake toolchain, !!
# !!   ANDROID is automatically defined. This block        !!
# !!   replaces GLFW + desktop OpenGL with:                !!
# !!     - android_native_app_glue (app lifecycle)         !!
# !!     - EGL + GLESv3 (OpenGL ES instead of desktop GL)  !!
# !!     - imgui_impl_android (instead of imgui_impl_glfw) !!
# !!   The output is a SHARED library (.so) loaded by      !!
# !!   NativeActivity in your AndroidManifest.xml.         !!
# !!                                                       !!
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# ============================================================
if(ANDROID)

  # ----------------------------------------------------------
  # !! ANDROID: Find NDK system libraries
  # ----------------------------------------------------------
  find_library(ANDROID_LIB android)
  find_library(EGL_LIB EGL)
  find_library(GLES3_LIB GLESv3)
  find_library(LOG_LIB log)

  # ----------------------------------------------------------
  # !! ANDROID: Build android_native_app_glue as a static lib
  # !! This provides the android_main() entry point bridge.
  # ----------------------------------------------------------
  add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
  )
  target_include_directories(native_app_glue PUBLIC
    ${ANDROID_NDK}/sources/android/native_app_glue
  )

  # ----------------------------------------------------------
  # !! ANDROID: ImGui -- swap imgui_impl_glfw -> imgui_impl_android
  # !!          and use "#version 300 es" for OpenGL ES 3.0
  # ----------------------------------------------------------
  add_library(ImGui STATIC
    ${CMAKE_SOURCE_DIR}/Libraries/include/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/Libraries/include/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/Libraries/include/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/Libraries/include/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/Libraries/include/imgui/imgui_impl_opengl3.cpp
    ${CMAKE_SOURCE_DIR}/Libraries/include/imgui/imgui_impl_android.cpp  # !! ANDROID: replaces imgui_impl_glfw.cpp
  )
  target_include_directories(ImGui PUBLIC
    ${CMAKE_SOURCE_DIR}/Libraries/include
    ${ANDROID_NDK}/sources/android/native_app_glue
  )
  target_compile_definitions(ImGui PRIVATE IMGUI_IMPL_OPENGL_LOADER_CUSTOM)

  # ----------------------------------------------------------
  # !! ANDROID: antibox engine as a static lib (no change to
  # !!          sources; platform guards live inside the .cpp files)
  # ----------------------------------------------------------
  add_library(antibox STATIC ${ANTIBOX_FILES})
  target_include_directories(antibox PUBLIC
    ${CMAKE_SOURCE_DIR}/Libraries/include
    ${ANDROID_NDK}/sources/android/native_app_glue
  )
  target_link_libraries(antibox PUBLIC ImGui)

  # ----------------------------------------------------------
  # !! ANDROID: Main output is a SHARED library (.so)
  # !!          NativeActivity in AndroidManifest.xml loads it.
  # !!          No separate glad target -- EGL/GLESv3 replace it.
  # ----------------------------------------------------------
  add_library(btf_game SHARED
    main.cpp
    ${GAME_FILES}
  )
  target_include_directories(btf_game PRIVATE
    ${CMAKE_SOURCE_DIR}/dat/
    ${ANDROID_NDK}/sources/android/native_app_glue
  )
  target_compile_definitions(btf_game PRIVATE IMGUI_IMPL_OPENGL_LOADER_CUSTOM)

  target_link_libraries(btf_game PRIVATE
    antibox
    native_app_glue
    ${ANDROID_LIB}    # android
    ${EGL_LIB}        # EGL (window surface)
    ${GLES3_LIB}      # GLESv3 (OpenGL ES 3.0)
    ${LOG_LIB}        # log (logcat)
  )

# ============================================================
# !!                                                       !!
# !!   DESKTOP BUILD (Windows / macOS / Linux)             !!
# !!   -- unchanged from before --                         !!
# !!                                                       !!
# ============================================================
else()

  # ------------------------------------------------------------
  # Defaults for flags that should always be enabled
  # ------------------------------------------------------------
  if (APPLE AND NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "/opt/homebrew/")
  endif()

  # ------------------------------------------------------------
  # Find dependencies
  # ------------------------------------------------------------
  if(APPLE)
    find_package(glfw3 3.3 REQUIRED)
  endif()

  if(WIN32)
    add_library(glfw STATIC IMPORTED)
    set_target_properties(glfw PROPERTIES
      IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Libraries/lib/glfw3.lib"
      INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/Libraries/include/GLFW"
    )
  endif()
  find_package(OpenGL REQUIRED)

  # I NEED MY INTELLISENSE
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

  # ------------------------------------------------------------
  # Build the executable
  # ------------------------------------------------------------
  add_executable(Zombos main.cpp)
  set_target_properties(Zombos PROPERTIES OUTPUT_NAME "btf_game")

  target_sources(Zombos PRIVATE ${GAME_FILES})
  target_include_directories(Zombos PRIVATE
    # ${CMAKE_SOURCE_DIR}/Libraries/include
    ${CMAKE_SOURCE_DIR}/dat/
  )

  set(ASSET_SOURCE_DIR ${CMAKE_SOURCE_DIR}/dat)
  set(ASSET_DEST_DIR   ${CMAKE_BINARY_DIR}/bin/dat)

  add_custom_command(
      TARGET Zombos POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
              ${ASSET_SOURCE_DIR}
              ${ASSET_DEST_DIR}
  )

  set(RESOURCE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/res)
  set(RESOURCE_DEST_DIR   ${CMAKE_BINARY_DIR}/bin/res)

  add_custom_command(
      TARGET Zombos POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
              ${RESOURCE_SOURCE_DIR}
              ${RESOURCE_DEST_DIR}
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
  target_include_directories(antibox PUBLIC
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
  target_include_directories(ImGui PUBLIC
    ${CMAKE_SOURCE_DIR}/Libraries/include
  )

  add_library(glad STATIC
    ${CMAKE_SOURCE_DIR}/Libraries/include/glad/glad.c
  )
  set_property(TARGET glad PROPERTY LINKER_LANGUAGE C)
  target_include_directories(glad PUBLIC
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

endif() # end if(ANDROID) / else desktop
