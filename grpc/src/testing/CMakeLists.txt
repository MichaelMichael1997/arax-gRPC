set(ARAX_CONTROLLER_PATH ${CMAKE_CURRENT_BINARY_DIR}/../../../../controller)

# -- Add the kernels in their own separate directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/kernels)

# -- Noop kernel -- 
add_library(nooplib SHARED noop_grpc.cpp)
target_include_directories(nooplib PRIVATE ${ARAX_CONTROLLER_PATH}/include)
target_compile_definitions(nooplib PRIVATE BUILD_SO)
target_link_libraries(nooplib arax)

add_library(datalib SHARED data.cpp)
target_include_directories(datalib PRIVATE ${ARAX_CONTROLLER_PATH}/include)
target_compile_definitions(datalib PRIVATE BUILD_SO)
target_link_libraries(datalib arax)


foreach(_target noop_grpc data)
	add_executable(${_target}
			"${_target}.cpp" 
		      )
	# include path to arax include files
	target_include_directories(${_target} PRIVATE ../../../build/include)
    target_include_directories(${_target} PRIVATE ${ARAX_CONTROLLER_PATH}/include)	
    target_compile_definitions(${_target} PRIVATE BUILD_MAIN)
	  target_link_libraries(${_target}
			      arax
			      pthread
                  nooplib
                  datalib
			      ${_REFLECTION}
			      ${_GRPC_GRPCPP}
			      ${_PROTOBUF_LIBPROTOBUF}
			      g_arax 
			     )
endforeach()
