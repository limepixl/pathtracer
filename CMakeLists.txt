cmake_minimum_required(VERSION 3.16)

project(pathtracer)

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# List of project source and header files
set(SOURCE_FILES 
	src/main.cpp
	src/loader.cpp

	src/scene/bvh.cpp
	src/scene/material.cpp
        src/scene/scene.cpp
	src/scene/triangle.cpp
	src/display/display.cpp
	src/resource/shader.cpp
	src/scene/camera.cpp

	src/math/mat3.cpp
	src/math/mat4.cpp
	src/math/math.cpp
	src/math/vec.cpp

	
	thirdparty/stb/stb_image.c
	thirdparty/pcg-c-basic-0.9/pcg_basic.c
	thirdparty/glad/src/glad.c
)

set(HEADER_FILES
	src/loader.hpp
	src/defines.hpp

	src/scene/bvh.hpp
	src/scene/material.hpp
	src/scene/ray.hpp
	src/scene/scene.hpp
	src/scene/sphere.hpp
	src/scene/triangle.hpp
	src/display/display.hpp
	src/resource/shader.hpp
	src/core/array.hpp

	src/math/mat3.hpp
	src/math/mat4.hpp
	src/math/math.hpp
	src/math/vec.hpp

)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

# C++ standard version
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

# Link-time optimization (LTO)
include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
	message("-- Enabled Link-Time Optimization (LTO)!")
	set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)

# Build flags
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
include(CMakePrintHelpers)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror -Wno-unused-result)
	if(UNIX)
#		target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
		target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl)
	endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(${PROJECT_NAME} PRIVATE -WX -W4 -wd4201 -wd4146)
	target_link_libraries(${PROJECT_NAME} PUBLIC opengl32)
endif()

cmake_print_variables(CMAKE_CXX_COMPILER_ID)
cmake_print_properties(TARGETS ${PROJECT_NAME} PROPERTIES COMPILE_OPTIONS)
cmake_print_variables(CMAKE_CXX_FLAGS)
cmake_print_variables(CMAKE_CXX_FLAGS_RELEASE)
cmake_print_variables(CMAKE_CXX_FLAGS_DEBUG)

set(SDL_JOYSTICK OFF)
set(SDL_CONTROLLER OFF)
set(SDL_HAPTIC OFF)
set(SDL_AUDIO OFF)
set(SDL_SHARED ON)
set(SDL_STATIC OFF)
add_subdirectory(thirdparty/SDL2-2.24.1/)

target_include_directories(${PROJECT_NAME} PUBLIC thirdparty/SDL2-2.24.1/include thirdparty/glad/include thirdparty/tinyobjloader thirdparty/stb)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL2 SDL2main)

if(CMAKE_BUILD_TYPE MATCHES Debug)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/thirdparty/SDL2-2.24.1/SDL2d.dll
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2d.dll)
else()
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/thirdparty/SDL2-2.24.1/SDL2.dll
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2.dll)
endif()