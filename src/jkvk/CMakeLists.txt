# Create a library
AddLibrary(JackalVK STATIC)
AddTargetToGroup(JackalVK JackalVK)

install(TARGETS JackalVK EXPORT JackalVK
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
	INCLUDES DESTINATION include)

AddIncludeDirectoryToTarget(JackalVK "${CMAKE_CURRENT_SOURCE_DIR}/.." INSTALL_INTERFACE "include")

target_compile_features(JackalVK INTERFACE
	cxx_std_17)

find_package(Vulkan REQUIRED)
find_package(JKUtil REQUIRED)

target_link_libraries(JackalVK PUBLIC Vulkan::Vulkan)
target_link_libraries(JackalVK PUBLIC JKUtil::JKUtil)

# Header Files

	AddSourcesToTarget(JackalVK
		GROUP "Header Files\\Utility"
		SCOPE PRIVATE
		INSTALL_DESTINATION "include/jkvk"
		FILES
			"${CMAKE_CURRENT_LIST_DIR}/structure_chain.h"
			"${CMAKE_CURRENT_LIST_DIR}/utility.h"

# Source Files

	AddSourcesToTarget(JackalVK
		GROUP "Source Files\\Utility"
		SCOPE PRIVATE
		FILES
			"${CMAKE_CURRENT_LIST_DIR}/structure_chain.cpp")