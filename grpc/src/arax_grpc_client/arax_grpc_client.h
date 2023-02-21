#ifndef ARAX_GRPC_CLIENT_H
#define ARAX_GRPC_CLIENT_H

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <chrono>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <exception>
#include <cstddef>
#include <cmath>
#include <vector>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

// Protobuf generated files
#include "../generated/arax.grpc.pb.h"
#include "../generated/arax.pb.h"

#define INIT_TASK_STREAMING(client) client.set_reader_writer();
#define TERM_TASK_STREAMING(client) client.terminate_task_issue_streaming();

// -------------------- Arax Client Class --------------------
class AraxClient {
private:
    std::unique_ptr<arax::Arax::Stub> stub_;
    std::shared_ptr<grpc::Channel> main_channel;

    /* ReaderWriter for the client. Used for task_issues streaming */
    std::shared_ptr<grpc::ClientReaderWriter<arax::TaskRequest, arax::ResourceID> > stream;
    grpc::ClientContext* task_ctx = 0;

    /*
     * Function to fragment the data into messages
     * less than 2MB in size.
     * Uses client streaming to send them to the server
     *
     * @param buffer The buffer to assign the data
     * @param accel  The accelerator identifier
     * @param data   The data
     */
    void large_data_set(uint64_t buffer, uint64_t accel, void *data, size_t size);

public:

    /*
     * Method to set up Client Reader/Writer for bidirectional
     * streaming. Should be called before using arax_task_issue_stream.
     *
     * Note: For each call to task_issue_streaming, a prior matching call to
     *       this must be made
     */
    void set_reader_writer();

    /*
     * Method to terminate bidirectional streaming used in task_issue_streaming.
     * Call this after every call to arax_task_issue_stream is made.
     */
    void terminate_task_issue_streaming();

    /*
     * Constructors
     *
     */
    AraxClient(const char *addr);

    /*
     * Destructors
     */
    ~AraxClient();

    // -------------------- Arax Client Services --------------------

    /*
     * This method should be used when task_issue is to be called multiple
     * times sequentially (e.g. Inside a loop)
     *
     * Each call to this should get a prior matching call to set_reader_writer
     * and one matching call to terminate_task_issue_streaming after
     *
     * @ same params as task_issue
     * @return The task ID
     */
    inline uint64_t client_arax_task_issue_streaming(const uint64_t accel, const uint64_t proc, 
      const void *host_init, const size_t host_size,
      const size_t in_count,
      const uint64_t *in_buffer,
      const size_t out_count, const uint64_t *out_buffer){
      arax::TaskRequest req;
      arax::ResourceID res;

      req.set_accel(accel);
      req.set_proc(proc);
      req.set_in_count(in_count);
      req.set_out_count(out_count);
      req.set_host_init(host_init, host_size);
      req.set_host_size(host_size);
      req.set_in_buffer(in_buffer, in_count * sizeof(uint64_t));
      req.set_out_buffer(out_buffer, out_count * sizeof(uint64_t));

      stream->Write(req);
      stream->Read(&res);

      return res.id();
    }

    /*
     * Delete the shared segment
     */
    void client_arax_clean();

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
     * @param data Byte sequence of serialized data
     *
     * @return nothing
     */
    void client_arax_data_set(uint64_t buffer, uint64_t accel, void *data, size_t size);

    /*
     * Return the data that was set to an arax buffer
     *
     * @param buffer The ID of the buffer
     *
     * @return The serialized data or empty string on failure
     */
    void client_arax_data_get(uint64_t buffer, void *user, size_t size);

    /*
     * Similar to client_arax_data_get
     * This one should be used when anticipating large data
     *
     * @param the ID of the buffer holding the data
     *
     * @return The serialized data or an empty string on failure
     */
    void client_arax_large_data_get(uint64_t buffer, void *user, size_t size);

    /*
     * Return the size of the data of specified arax_data
     *
     * @param id The ID of the arax_buffer
     *
     * @return The size of the data
     */
    size_t client_arax_data_size(uint64_t id);

    /*
     * Mark data for deletion
     *
     * @param id The id of the buffer
     *
     * @return nothing
     */
    void client_arax_data_free(uint64_t id);

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
    uint64_t client_arax_task_issue(uint64_t accel, uint64_t proc, void *host_init, size_t host_size,
      size_t in_count,
      uint64_t *in_buffer,
      size_t out_count, uint64_t *out_buffer);

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

    /*
     * Initialize arax_data_s object
     *
     * @param  size The size for the object
     *
     * @return The ID of the resource or 0 on failure
     */
    uint64_t client_arax_data_init(size_t size);

    /*
     * Initialize arax_data_s object with aligned buffer
     *
     * @param  size  The size for the object
     * @param  align Alignment of buffer in bytes, power of two
     *
     * @return The ID of the resource or 0 on failure
     */
    uint64_t client_arax_data_init_aligned(size_t size, size_t align);

    /*
     * Initialize data remote (accelerator) buffer
     *
     * @param buffer The arax buffer
     * @param accel  The accelerator
     *
     * @return void
     */
    void client_arax_data_allocate_remote(uint64_t buffer, uint64_t accel);
}; /* class AraxClient */


#endif /* ifndef ARAX_GRPC_CLIENT_H */
