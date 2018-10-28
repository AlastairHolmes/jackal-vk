# Create a library
AddLibrary(JackalVK HEADERONLY)
AddTargetToGroup(JackalVK JackalVK)

install(TARGETS JackalVK EXPORT JackalVK
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
	INCLUDES DESTINATION include)
	
AddIncludeDirectoryToTarget(JackalVK "${CMAKE_CURRENT_SOURCE_DIR}/.." INSTALL_INTERFACE "include")

target_compile_features(JackalVK INTERFACE
	cxx_std_17)
	
# Header Files

	AddSourcesToTarget(JackalVK
		GROUP "Header Files\\Utility"
		SCOPE PRIVATE
		INSTALL_DESTINATION "include/jkvk"
		FILES
			"${CMAKE_CURRENT_LIST_DIR}/structure_chain.h")