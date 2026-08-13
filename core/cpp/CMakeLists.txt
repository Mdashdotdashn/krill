cmake_minimum_required(VERSION 2.8)


PROJECT(KrillEmbed)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
	# Ensure __cplusplus reflects the configured language standard on MSVC.
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
endif()

add_subdirectory(tests)
