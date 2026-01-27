cmake_minimum_required(VERSION "3.19")

project("ASCII renderer")
set(CMAKE_CXX_STANDARD 17)

#--------------------------------------------------------------
# resources subdirectory
#--------------------------------------------------------------
add_compile_definitions(RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources")

#--------------------------------------------------------------
# GLFW
#--------------------------------------------------------------
#set(GLFW_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/glfw/include)
#set(GLFW_LIBRARY ${CMAKE_SOURCE_DIR}/external/glfw/lib/glfw3.lib)


#--------------------------------------------------------------
# STB Image
#--------------------------------------------------------------
set(STB_IMAGE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/stb_img")

#--------------------------------------------------------------
# Executable
#--------------------------------------------------------------
add_executable(Main "src/Mains/main.cpp" "src/Mains/helpers.h"  "src/Mains/CustomException.h")


# Includes
target_include_directories(Main PRIVATE 
	${STB_IMAGE_INCLUDE_DIR}
)

#Link libraries
target_link_libraries(Main PRIVATE 
	Kernel32.lib
)