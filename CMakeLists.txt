cmake_minimum_required(VERSION 3.9)
project("jackal-vulkan")

include(${CMAKE_CURRENT_LIST_DIR}/ext/common-cmake/common.cmake)

# Set the default install location.
SetDefaultInstallPrefix("${CMAKE_SOURCE_DIR}/bin")

# Load the project version from a file 'VERSION'
SetProjectVersionFromFile(JackalVK ${CMAKE_CURRENT_LIST_DIR}/meta/version/VERSION)

# Enable target groups for the Visual Studio IDE
EnableGroups()

# Generate Documentation
add_subdirectory("${CMAKE_SOURCE_DIR}/meta/doxy")

# Main Library
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src/jkvk")

# Installs the project's targets
SetupExport("JackalVK")
InstallExport("JackalVK"
	NAMESPACE "JackalVK")