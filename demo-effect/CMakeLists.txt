cmake_minimum_required(VERSION 3.14.0)

# set the name and version of the plugin here
set(unplug_plugin_name "UnPlugDemoEffect")
set(unplug_plugin_version 1.0.0)

# unplug_SOURCE_DIR must be set to the path of your unplug local repo
set(unplug_SOURCE_DIR "${CMAKE_SOURCE_DIR}/..")

# C++ global config
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)
if(WIN32)
	add_compile_options("/EHsc")
	ADD_DEFINITIONS(-DUNICODE)
	ADD_DEFINITIONS(-D_UNICODE)
endif()

# VST3 SDK and plugin
set(vst3sdk_SOURCE_DIR ${unplug_SOURCE_DIR}/libs/vst3sdk)
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "VST3 SDK is missing. Please run: git submodule update --init --recursive in your unplug local repository")
endif()
project(${unplug_plugin_name} VERSION ${unplug_plugin_version})
set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")
add_subdirectory(${vst3sdk_SOURCE_DIR} "${PROJECT_BINARY_DIR}/vst3sdk")
smtg_enable_vst3_sdk()
file(GLOB_RECURSE src "${CMAKE_SOURCE_DIR}/source/*")
smtg_add_vst3plugin(${PROJECT_NAME} ${src})
if(SMTG_ADD_VSTGUI)
    target_link_libraries(${PROJECT_NAME} PRIVATE vstgui_support)
endif()
smtg_add_vst3_snapshot(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/resource/DB759B1D41C65B09ADAD652A8D6F776A_snapshot.png")
smtg_add_vst3_snapshot(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/resource/DB759B1D41C65B09ADAD652A8D6F776A_snapshot_2.0x.png")
target_link_libraries(${PROJECT_NAME} PRIVATE sdk)
if(SMTG_MAC)
    smtg_set_bundle(${PROJECT_NAME} INFOPLIST 
        "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" 
        PREPROCESS
    )
elseif(SMTG_WIN)
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
endif()

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

# Pugl
add_library(pugl STATIC)
include_directories("${unplug_SOURCE_DIR}/libs/pugl/include" "${unplug_SOURCE_DIR}/libs/pugl/src" "${unplug_SOURCE_DIR}/libs/pugl/bindings/cxx/include")
target_sources(pugl PRIVATE "${unplug_SOURCE_DIR}/libs/pugl/src/implementation.c")
target_compile_definitions(pugl PUBLIC PUGL_STATIC=1)
if(SMTG_WIN)
	target_sources(pugl PRIVATE "${unplug_SOURCE_DIR}/libs/pugl/src/win.c" "${unplug_SOURCE_DIR}/libs/pugl/src/win_gl.c" "${unplug_SOURCE_DIR}/libs/pugl/src/win_stub.c")
elseif(SMTG_MAC)
	#todo
else()
	target_sources(pugl PRIVATE "${unplug_SOURCE_DIR}/libs/pugl/src/x11.c")
	target_sources(pugl PRIVATE "${unplug_SOURCE_DIR}/libs/pugl/src/x11_gl.c")
	target_sources(pugl PRIVATE "${unplug_SOURCE_DIR}/libs/pugl/src/x11_stub.c")
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE pugl)

# Dear ImGui
add_library(imgui STATIC)
target_compile_definitions(imgui PUBLIC IMGUI_USER_CONFIG="imgui_user_config.h" IMGUI_DISABLE_OBSOLETE_FUNCTIONS)
include_directories("${unplug_SOURCE_DIR}/libs/imgui" "${unplug_SOURCE_DIR}/libs/imgui/backends" "${unplug_SOURCE_DIR}/unplug/imgui")
file(GLOB imgui_src "${unplug_SOURCE_DIR}/libs/imgui/*.cpp")
file(GLOB imgui_gl "${unplug_SOURCE_DIR}/libs/imgui/backends/imgui_impl_opengl2.cpp" "${unplug_SOURCE_DIR}/unplug/imgui/imgui_tls_context.cpp")
target_sources(imgui PRIVATE ${imgui_src})
target_sources(imgui PRIVATE ${imgui_gl})
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

# Unplug
include_directories("${unplug_SOURCE_DIR}/unplug/include")

