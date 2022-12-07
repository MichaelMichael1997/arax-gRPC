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

class AraxClient{
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
   * Initialize Arax
   *
   * This should be called by all applications prior
   * to using any other Arax function.
   *
   * @return An arax_pipe_s c struct object
   */
   // void client_arax_init();
   
   /*
    * Exit and cleanup Arax
    */
   // void client_arax_exit();
   
   /*
    * Delete the shared segment
    */
    void client_arax_clean();

  /*
   * Returns the number of accelerators of a specific type
   * TODO: Plus some other things
   */
    arax::AccelListResponse client_arax_accel_list(arax_accel_type_e type, bool physical);

    /*
     * Returns arax revision
     *
     * @return string with arax revision
     */
    const char* client_arax_pipe_revision();
    
    /*
     * Increase process counter for arax_pipe
     *
     * @return Number of active processes before adding issuer
     */
    uint64_t client_arax_pipe_add_process();

    /*
     * Decrease process counter for pipe
     *
     * @return Number of active processes before removing issuer
     */
    uint64_t client_arax_pipe_del_process();

    /*
     * Free memory of accelerator array returned by arax accel list
     */
    void client_arax_accel_list_free();

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
      uint64_t client_arax_proc_register(const char* func_name);

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
    void client_arax_set_data(uint64_t buffer, uint64_t accel, const char* value);

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

    /*
     * Gen an orphan/Unassigned virtual accelerator of NULL
     * Function will sleep if there are no orphans exist at the time of the call
     *
     * @return The ID of the retrieved vaccel or 0 on failure
     */
    uint64_t client_arax_pipe_get_orphan_vaccel();

    /*
     * Add vac with ID \c id to the list of orphan_vacs/unassigned accels
     *
     * @param id The ID of the vaccel
     *
     * @return nothing
     */
    void client_arax_pipe_add_orphan_vaccel(uint64_t id);

    /*
     * Returns false if there are no orphan vaccels
     *
     * @return true if orphan vaccels exist, false otherwise
     */
    bool client_arax_pipe_have_orphan_vaccels();

    /*
     * Remove vaccel, specified by \c id, from the orphan vaccel list
     *
     * @param id The id for the vaccel
     *
     * @return void
     */
    void client_arax_pipe_remove_orphan_vaccel(uint64_t id);

}; /* class AraxClient */


#endif /* ifndef ARAX_GRPC_CLIENT_H */




















