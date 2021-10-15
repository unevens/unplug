cmake_minimum_required(VERSION 3.14.0)

# unplug_SOURCE_DIR must be set to the path of your unplug local repo
set(unplug_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../..")

# set the name and version of the plugin here
set(unplug_plugin_name "UnPlugGainExample")

#set this to true to use the Oversimple oversampling library
set(unplug_use_oversimple_oversampling TRUE)

#set the cpp standard you want to use for the plugin (must be at least c++20)
set(unplug_cpp_standard 20)

# sets the version of openGL to use: supported values are 2 and 3
set(unplug_opengl_version 3)

# on MacOS, by default unplug will build an universal binary. you can set unplug_override_macos_arch to the architecture
# you want to build by uncommenting one of the next two lines. It can be useful because Compiler Explorer does not support
# universal binaries.
# Note: CMake sometimes does not update this when you reload the project without deleting the build folder manually.
set(unplug_override_macos_arch "")
#set(unplug_override_macos_arch "arm64")
#set(unplug_override_macos_arch "x85_64")

# set this to TRUE to make public some function to query the parameters that return false on failure. See Vst3ParameterAccess.hpp
set(unplug_expose_vst3style_parameter_api FALSE)

#set this to TRUE to build with the address sanitizer enabled - it can be a good idea to enable it for running the validator or other testing suites.
set(unplug_use_asan FALSE)


# C++ global config
if (WIN32)
    add_compile_options("/EHsc" "/Zc:__cplusplus")
    ADD_DEFINITIONS(-DUNICODE)
    ADD_DEFINITIONS(-D_UNICODE)
else ()
    if (${unplug_use_asan} STREQUAL TRUE)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
        set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    endif ()
endif (WIN32)

#universal binary if building on arm64
if (APPLE)
    if (unplug_override_macos_arch STREQUAL "arm64")
        set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "")
        message(STATUS "Supported architectures: arm64 (forced by user)")
    elseif (unplug_override_macos_arch STREQUAL "x86_64")
        set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "")
        message(STATUS "Supported architectures: x86_64 (forced by user)")
    else ()
        execute_process(
                COMMAND uname -m
                RESULT_VARIABLE result
                OUTPUT_VARIABLE MACHINE_ARCHITECTURE
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if (MACHINE_ARCHITECTURE STREQUAL "arm64")
            set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "")
            message(STATUS "Supported architectures: x86_64, arm64")
        else ()
            message(STATUS "Supported architectures: ${MACHINE_ARCHITECTURE}")
        endif ()
    endif ()
endif (APPLE)

# VST3 SDK and plugin
set(vst3sdk_SOURCE_DIR ${unplug_SOURCE_DIR}/libs/vst3sdk)
if (NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "VST3 SDK is missing. Please run: git submodule update --init --recursive in your unplug local repository")
endif ()
project(${unplug_plugin_name})
set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")
cmake_policy(SET CMP0077 NEW)
set(SMTG_ADD_VSTGUI OFF)
add_subdirectory(${vst3sdk_SOURCE_DIR} "${PROJECT_BINARY_DIR}/vst3sdk")
smtg_enable_vst3_sdk()

if(unplug_use_oversimple_oversampling)
    add_subdirectory("${unplug_SOURCE_DIR}/libs/oversimple" "${PROJECT_BINARY_DIR}/oversimple")
endif()

# reset c++ standard to the desired one (vst sdk set it to 14, oversimple to 17)
set(CMAKE_CXX_STANDARD ${unplug_cpp_standard})
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "")

set(plug-src-path "${CMAKE_SOURCE_DIR}/source")
include_directories(${plug-src-path})
file(GLOB_RECURSE plug-src "${plug-src-path}/*")
file(GLOB_RECURSE unplug-src "${unplug_SOURCE_DIR}/unplug/source/unplug/*")
set(src ${plug-src} ${unplug-src})
smtg_add_vst3plugin(${PROJECT_NAME} ${src})
if (SMTG_ADD_VSTGUI)
    target_link_libraries(${PROJECT_NAME} PRIVATE vstgui_support)
endif ()
smtg_add_vst3_snapshot(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/resource/DB759B1D41C65B09ADAD652A8D6F776A_snapshot.png")
smtg_add_vst3_snapshot(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/resource/DB759B1D41C65B09ADAD652A8D6F776A_snapshot_2.0x.png")
target_link_libraries(${PROJECT_NAME} PRIVATE sdk)
if (SMTG_MAC)
    smtg_set_bundle(${PROJECT_NAME} INFOPLIST
            "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist"
            PREPROCESS
            )
elseif (SMTG_WIN)
    target_sources(${PROJECT_NAME} PRIVATE
            resource/win32resource.rc
            )
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

    set_target_properties(${PROJECT_NAME} PROPERTIES
            VS_DEBUGGER_COMMAND
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            VS_DEBUGGER_COMMAND_ARGUMENTS
            " --pluginfolder \"$(OutDir)\" "
            )
endif ()

target_compile_definitions(${PROJECT_NAME} PUBLIC UNPLUG_VST3=1)
if (SMTG_WIN)
    target_compile_definitions(${PROJECT_NAME} PUBLIC _USE_MATH_DEFINES=1)
endif ()

if(unplug_use_oversimple_oversampling)
    target_link_libraries(${PROJECT_NAME} PRIVATE oversimple)
endif()

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

# Pugl
add_library(pugl STATIC)
include_directories("${unplug_SOURCE_DIR}/libs/pugl/include" "${unplug_SOURCE_DIR}/libs/pugl/src" "${unplug_SOURCE_DIR}/libs/pugl/bindings/cpp/include")
target_sources(pugl PRIVATE "${unplug_SOURCE_DIR}/libs/pugl/src/implementation.c")
target_compile_definitions(pugl PUBLIC PUGL_STATIC PUGL_NO_INCLUDE_GL_H PUGL_NO_INCLUDE_GLU_H)
if (SMTG_WIN)
    target_sources(pugl PRIVATE "${unplug_SOURCE_DIR}/libs/pugl/src/win.c" "${unplug_SOURCE_DIR}/libs/pugl/src/win_gl.c" "${unplug_SOURCE_DIR}/libs/pugl/src/win_stub.c")
elseif (SMTG_MAC)
    target_sources(pugl PRIVATE "${unplug_SOURCE_DIR}/libs/pugl/src/mac.m" "${unplug_SOURCE_DIR}/libs/pugl/src/mac_gl.m" "${unplug_SOURCE_DIR}/libs/pugl/src/mac_stub.m")
    find_library(COCOA_LIBRARY Cocoa)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COCOA_LIBRARY})
    find_library(COREVIDEO_LIBRARY CoreVideo)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COREVIDEO_LIBRARY})
else ()
    target_compile_definitions(pugl PUBLIC HAVE_XSYNC HAVE_XRANDR HAVE_XCURSOR)
    target_sources(pugl PRIVATE "${unplug_SOURCE_DIR}/libs/pugl/src/x11.c" "${unplug_SOURCE_DIR}/libs/pugl/src/x11_gl.c" "${unplug_SOURCE_DIR}/libs/pugl/src/x11_stub.c")
endif ()
target_link_libraries(${PROJECT_NAME} PRIVATE pugl)

# Dear ImGui & ImPlot
add_library(imgui STATIC)
target_compile_definitions(imgui PUBLIC IMGUI_USER_CONFIG="imgui_user_config.h" IMGUI_DISABLE_OBSOLETE_FUNCTIONS IMGUI_DEFINE_MATH_OPERATORS)
include_directories("${unplug_SOURCE_DIR}/libs/imgui" "${unplug_SOURCE_DIR}/libs/imgui/backends" "${unplug_SOURCE_DIR}/unplug/imgui")
file(GLOB imgui_src "${unplug_SOURCE_DIR}/libs/imgui/*.cpp")
target_sources(imgui PRIVATE ${imgui_src} "${unplug_SOURCE_DIR}/unplug/imgui/imgui_tls_context.cpp" "${unplug_SOURCE_DIR}/libs/imgui/backends/imgui_impl_opengl2.cpp")
if (${unplug_opengl_version} STREQUAL 3)
    target_sources(imgui PRIVATE "${unplug_SOURCE_DIR}/libs/imgui/backends/imgui_impl_opengl3.cpp")
    target_compile_definitions(${PROJECT_NAME} PUBLIC UNPLUG_OPENGL_VERSION=3)
elseif (${unplug_opengl_version} STREQUAL 2)
    target_sources(imgui PRIVATE "${unplug_SOURCE_DIR}/libs/imgui/backends/imgui_impl_opengl2.cpp")
    target_compile_definitions(${PROJECT_NAME} PUBLIC UNPLUG_OPENGL_VERSION=2)
else ()
    message(FATAL_ERROR "Error: unplug_opengl_version should be 2 or 3. current value is ${unplug_opengl_version}")
endif ()
# and ImPlot
include_directories("${unplug_SOURCE_DIR}/libs/implot")
file(GLOB_RECURSE implot_src "${unplug_SOURCE_DIR}/libs/implot/*.cpp" "${unplug_SOURCE_DIR}/libs/implot/*.h")
target_sources(imgui PRIVATE ${implot_src})
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

# Unplug
include_directories("${unplug_SOURCE_DIR}/unplug/include")
include_directories("${unplug_SOURCE_DIR}/libs/lockfree-async")
add_library(unplug-opaque-gl STATIC)
target_sources(unplug-opaque-gl PRIVATE "${unplug_SOURCE_DIR}/unplug/source/gl/OpaqueGl.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE unplug-opaque-gl)

if (${unplug_expose_vst3style_parameter_api} STREQUAL TRUE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC UNPLUG_EXPOSE_VST3STYLE_PARAMETER_API=1)
endif ()