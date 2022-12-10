#ifndef ARAX_GRPC_CLIENT_H
#define ARAX_GRPC_CLIENT_H

#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <exception>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

// Protobuf generated files
#include "../generated/arax.grpc.pb.h"
#include "../generated/arax.pb.h"

// Arax header files
#include <arax.h>
#include <arax_pipe.h>
#include <arax_types.h>

#include <core/arax_data.h>

// -------------------- Arax Client Class --------------------

class AraxClient {
private:
    std::unique_ptr<arax::Arax::Stub> stub_; // Only member functions should have access to this

public:

    /*
     * Constructors
     *
     */
    AraxClient(std::shared_ptr<grpc::Channel> channel);

    /*
     * Destructors
     */
    ~AraxClient();

    // -------------------- Arax Client Services --------------------

    /*
     * Delete the shared segment
     */
    void client_arax_clean();

    /*
     * Get the type of accelerator, specified in the AccelDescriptor request message
     *
     * @param the request message
     *
     * @return the type of the accelerator or -1 on failure
     */
    int client_arax_accel_type(arax::AccelDescriptor req);

    /*
     * Create an arax_buffer_s object
     *
     * @param size The desired size for the buffer
     *
     * @return The ID for the newly allocated buffer, or 0 if failed
     */
    uint64_t client_arax_buffer(size_t size);

    /*
     * Register a new process 'func_name'
     *
     * @param func_name The name of the process
     *
     * @return The id of the arax_proc resource, or 0 on failure
     */
    uint64_t client_arax_proc_register(const char *func_name);

    /*
     * Retrieve a previously registered arax_process
     *
     * \Note Calls to client_arax_proc_register(..), client_arax_proc_get(..),
     * should have matching calls to arax_proc_put(..)
     *
     * @param func_name The process func name
     *
     * @return The ID of the resource, or 0 on failure
     */
    uint64_t client_arax_proc_get(const char *func_name);

    /*
     * Delete registered arax proc pointer
     *
     * @param proc The name of the process
     *
     * @return nothing
     */
    void client_arax_proc_put(uint64_t id);

    /*
     * Acquire a virtual accelerator of the given type
     *
     * @param type The type of the accelerator
     *
     * @return The id of the acquire resource or 0 on failure
     */
    uint64_t client_arax_accel_acquire_type(unsigned int type);

    /*
     * Release previously acquired accelerator
     *
     * @param id The id of the accelerator
     *
     * @return nothing
     */
    void client_arax_accel_release(uint64_t id);

    /*
     * Set data to buffer
     *
     * @param buffer The ID of the buffer
     * @param accel The ID of the accelerator
     * @param value The string value to be passed to buffer
     * TODO: Change this depending on the possible values a buffer can get
     *
     * @return nothing
     */
    void client_arax_data_set(uint64_t buffer, uint64_t accel, const char *value);

    /*
     * Return the data that was set to an arax buffer
     *
     * @param buffer The ID of the buffer
     *
     * @return The data
     */
    const char* client_arax_data_get(uint64_t buffer);

    /*
     * Issue a new task
     *
     * @param accel The ID of the accelerator responsible for executing the task
     * @param proc ID of the arax_proc to be dispatched on accelerator
     * @param in_count Size of input array (elements)
     * @param in_buffer Input buffer
     * @param out_count Size of output array (elements)
     * @param out_buffer Output buffer
     *
     * @return The id of the new task or 0 on failure
     */
    uint64_t client_arax_task_issue(uint64_t accel, uint64_t proc, size_t in_count, uint64_t in_buffer,
      size_t out_count, uint64_t out_buffer);

    /*
     * Decrease ref counter of task
     *
     * @param task The ID of the task
     *
     * @return
     */
    void client_arax_task_free(uint64_t task);

    /*
     * Wait for an issued task to complete or fail
     *
     * @param task The ID of the task
     *
     * @return The state of the task or -1 on failure
     */
    int client_arax_task_wait(uint64_t task);
}; /* class AraxClient */


#endif /* ifndef ARAX_GRPC_CLIENT_H */
