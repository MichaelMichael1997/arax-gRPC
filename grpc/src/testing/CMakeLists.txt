# Build the tests and arax_controller dynamic libs (kernel ops)

# Set the path to the Arax controller include directory
set(ARAX_CONTROLLER_PATH ${CMAKE_CURRENT_BINARY_DIR}/../../../../controller)

# -- Add the kernels in their own separate directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/kernels)

# -- Noop kernel -- 
add_library(nooplib SHARED noop_grpc.cpp)
target_include_directories(nooplib PRIVATE ${ARAX_CONTROLLER_PATH}/include)
target_compile_definitions(nooplib PRIVATE BUILD_SO)
target_link_libraries(nooplib arax)

# -- Create another test kernel --
add_library(datalib SHARED data.cpp)
target_include_directories(datalib PRIVATE ${ARAX_CONTROLLER_PATH}/include)
target_compile_definitions(datalib PRIVATE BUILD_SO)
target_link_libraries(datalib arax)

# -- Create a test kernel for a vector of integers --
add_library(vectorlib SHARED vector.cpp)
target_include_directories(vectorlib PRIVATE ${ARAX_CONTROLLER_PATH}/include)
target_compile_definitions(vectorlib PRIVATE BUILD_SO)
target_link_libraries(vectorlib arax)


# Create the executable and create link the library
foreach(_target vector data noop_grpc)
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
                  vectorlib
                  nooplib
                  datalib
			      ${_REFLECTION}
			      ${_GRPC_GRPCPP}
			      ${_PROTOBUF_LIBPROTOBUF}
			      ${ARAX_API}
			     )
endforeach()
