#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <stdlib.h>
#include <sstream>
#include <typeinfo>
#include <string.h>
#include <map>

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <utils/config.h>
#include <utils/system.h>
#include <utils/timer.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// Protobuf generated files
#include "../generated/arax.grpc.pb.h"
#include "../generated/arax.pb.h"

// Arax headers
#include <arax.h>
#include <arax_pipe.h>
#include <core/arax_data.h>


// ---------------------------------- Arax Service Class --------------------------------------

class AraxServer final : public arax::Arax::Service
{
private:
    std::unique_ptr<grpc::Server> server; // Unique pointer to the Server service

    uint64_t unique_id;  // Current value for the unique id to be given to a resource. Starts from 1.
    arax_pipe_s *pipe_s; // Arax pipe_s instance. Arax initialized in the Constructor

    /* Mappings to store resources, with key a unique uint64_t ID */
    std::map<uint64_t, arax_buffer_s> buffers;
    std::map<uint64_t, arax_proc *> arax_processes;
    std::map<uint64_t, arax_accel *> arax_accels;
    std::map<uint64_t, arax_task *> arax_tasks;
    std::map<uint64_t, arax_vaccel_s *> arax_vaccels;

    /*
     * Template function to add a pair into an std::map
     *
     * @param mapping Map with key uint64_t, the ID of the Resource, and the type T as value
     * @param ID the id of the resource
     * @param t The value T to be inserted
     *
     * @return true if operation succesfull, false if the key already exists in the map
     */
    template <typename T> bool insert_pair(std::map<uint64_t, T> &mapping, uint64_t ID, const T t)
    {
        // check if name in mapping
        if (mapping.find(ID) == mapping.end()) { /* key does not exist in the mapping */
            mapping.insert(std::pair<uint64_t, T>(ID, t));
            return true;
        }
        /* Key already exists in the map */
        return false;
    }

    /*
     * If pair with key 'ID' exists, return true,
     * else return false
     */
    template <typename T> bool check_if_exists(const std::map<uint64_t, T> map, uint64_t key)
    {
        return map.find(key) != map.end();
    }

    /*
     * Function to return an uint64_t ID for a data structure
     * Every ID that is returned is unique
     * 0 is never returned as an ID.
     *
     *
     * @return uint64_t Unique ID
     */
    uint64_t get_unique_id();


    /* ----- Server Start/Shutdown ------ */

    /*
     * Function to start the server
     *
     * @return void
     */
    void start_server();

    /*
     * Function to shutdown the server
     *
     * @return void
     */
    void shutdown_server();


public:

    /*
     * @param the address to connect
     */
    AraxServer(const char *addr);

    /*
     * Destructosrs
     */
    ~AraxServer();

    /*
     * -------------------- Arax Services Implementations --------------------
     */

    /*
     * Clean/Delete the shared segment
     * \note This should only be called when there are no uses of the shared segment
     * \note Behaviour undefined if called with processes accessing the shared segment
     *
     * @return The appropriate status code
     */
    grpc::Status Arax_clean(grpc::ServerContext *ctx, const google::protobuf::Empty *req,
      google::protobuf::Empty *res) override;

    /*
     * Create an arax_buffer_s object
     *
     * @param ctx The server context
     * @param req RequestBuffer object with the size of the buffer
     * @param res ResourceID message, with the ID of the newly created resource
     *
     * @return The appropriate status code
     */
    grpc::Status Arax_buffer(grpc::ServerContext *ctx,
      const arax::RequestBuffer *req, arax::ResourceID *res) override;

    /*
     * Register a new process 'func_name'
     * Processes are accelerator agnostic and initially have no 'Implementations'/functors
     * Created arax_proc* identifies given function globally
     *
     * @param ctx Server context
     * @param req ProcRequest message holding the name of the process
     * @param res ResourceID message holding the id of the arax_proc resource
     *
     * @return The appropriate status code
     */
    grpc::Status Arax_proc_register(grpc::ServerContext *ctx,
      const arax::ProcRequest *req, arax::ResourceID *res) override;

    /*
     * Retrieve a previously registerd process
     * Calls to Arax_proc_register(..), Arax_proc_get(..) should have matching
     * calls to Arax_proc_put(..)
     *
     * @param ctx The server context
     * @param req ProcRequest message with the functor name
     * @param res ResourceID message with the ID of the resource
     *
     * @return The appropriate status code
     */
    grpc::Status Arax_proc_get(grpc::ServerContext *ctx,
      const arax::ProcRequest *req, arax::ResourceID *res) override;

    /*
     * Delete registered arax_proc pointer
     *
     * @param ctx Server Context
     * @param req AraxProc message with the func_name of the process
     * @param res Response which holds the return value of the arax_proc_put function
     *
     * @return The appropriate status code
     */
    grpc::Status Arax_proc_put(grpc::ServerContext *ctx,
      const arax::AraxProc *req, arax::ProcCounter *res) override;

    /*
     * Acquire a virtual accelerator of the given type
     *
     * @param ctx Server context
     * @param req AccelRequest with the type
     * @param res ResourceID message with the returned id for the resource
     *
     * @return The appropriate status code
     */
    grpc::Status Arax_accel_acquire_type(grpc::ServerContext *ctx,
      const arax::AccelRequest *req, arax::ResourceID *res) override;

    /*
     * Release a previously acquired accelerator
     *
     * @param ctx Server context
     * @param req ResourceID message with the ID of the resource
     * @param res Empty message
     *
     * @return The appropriate status code
     */
    grpc::Status Arax_accel_release(grpc::ServerContext *ctx,
      const arax::ResourceID *req, google::protobuf::Empty *res) override;

    /*
     * Copy data to buffer
     *
     * @param ctx Server context
     * @param req DataSet message containing the necessary information
     * @param res Success message, contains true if operation successfull, false otherwise
     */
    grpc::Status Arax_data_set(grpc::ServerContext *ctx,
      const arax::DataSet *req, google::protobuf::Empty *res) override;

    /*
     * Get data from buffer and return them to user
     *
     * @param ctx The server context
     * @param req ResourceID message holding the ID of the buffer
     * @param res DataSet message holding the data to be returned
     *
     * @return The appropriate status code
     */
    grpc::Status Arax_data_get(grpc::ServerContext *ctx,
      const arax::ResourceID *req, arax::DataSet *res) override;

    /*
     * Get size of the specified data
     *
     * @param ctx The server context
     * @param req ResourceID message holding the ID of the buffer
     * @param res DataSet message holding the size of the data
     *
     * @return The appropriate status code
     */
    grpc::Status Arax_data_size(grpc::ServerContext *ctx,
      const arax::ResourceID *req, arax::DataSet *res) override;

    /*
     * Mark data for deletion
     *
     * @param ctx The server context
     * @param req ResourceID messsage holding the ID of the buffer
     * @param res Empty message
     */
    grpc::Status Arax_data_free(grpc::ServerContext *ctx,
      const arax::ResourceID *req, google::protobuf::Empty *res) override;

    /*
     * Issue a new arax task
     *
     * @param ctx The server context
     * @param req TaskRequest message, containing all the necessary info for the new task
     * @param res Resource message with the id of the resource
     *
     * @return The appropriate status code
     */
    grpc::Status Arax_task_issue(grpc::ServerContext *ctx,
      const arax::TaskRequest *req, arax::ResourceID *res) override;

    /*
     * Decrease ref counter of task
     *
     * @param ctx The Server Context
     * @param req TaskMessage message holding the ID
     * @param res Empty message
     *
     * @return The appropriate status code
     */
    grpc::Status Arax_task_free(grpc::ServerContext *ctx,
      const arax::TaskMessage *req, google::protobuf::Empty *res) override;

    /*
     * Wait for an issued task to complete or fail
     *
     * @param ctx The server context
     * @param req TaskMessage message with the name of the task
     * @param res TaskMessage message with the state of the task
     *
     * @return The appropriate status code
     */
    grpc::Status Arax_task_wait(grpc::ServerContext *ctx,
      const arax::TaskMessage *req, arax::TaskMessage *res) override;
};


#endif /* #ifndef SERVER_H */
