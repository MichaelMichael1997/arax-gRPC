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
#include <thread>
#include <condition_variable>
#include <signal.h>

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

using grpc::ServerCompletionQueue;
using grpc::ServerContext;

// Protobuf generated files
#include "../generated/arax.grpc.pb.h"
#include "../generated/arax.pb.h"

// Arax headers
#include <arax.h>
#include <arax_pipe.h>
#include <core/arax_data.h>
#include <core/arax_data_private.h>

// ---------------------------------- Arax Service Class --------------------------------------

class AraxServer final : public arax::Arax::Service
{
private:
    std::unique_ptr<grpc::Server> server;
    uint64_t unique_id; 
    arax_pipe_s *pipe_s;



    std::map<uint64_t, arax_buffer_s> buffers;
    std::map<uint64_t, arax_proc *> arax_processes;
    std::map<uint64_t, arax_accel *> arax_accels;
    std::map<uint64_t, arax_task *> arax_tasks;

    /*
     * Template function to add a pair into an std::map
     *
     * @param mapping Map with key uint64_t, the ID of the Resource, and the type T as value
     * @param ID the id of the resource
     * @param t The value T to be inserted
     *
     * @return true if operation succesfull, false if the key already exists in the map
     */
    template <typename T>
    bool insert_pair(std::map<uint64_t, T> &mapping, uint64_t ID, const T t)
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
    template <typename T>
    bool check_if_exists(const std::map<uint64_t, T> map, uint64_t key)
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

    /*
     * Function to receive large data from the client
     * via streaming
     *
     * @param ctx Server Context
     * @param reader ServerReader instance to read the incoming stream of data
     *               from the client
     * @param res Empty message
     */
    grpc::Status Arax_data_set_streaming(grpc::ServerContext *ctx,
      grpc::ServerReader<arax::DataSet> *reader, arax::Empty *res) override; 

public:

    /*
     * @param the address to connect
     */
    AraxServer(const char *addr);

    /*
     * Destructosrs
     */
    ~AraxServer();

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

   /*
    * Thread polls to check for server shutdown
    */
    void server_shutdown_thread();

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
    grpc::Status Arax_clean(grpc::ServerContext *ctx, const arax::Empty *req,
      arax::Empty *res) override;

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
      const arax::ResourceID *req, arax::Empty *res) override;

    /*
     * Copy data to buffer
     *
     * @param ctx Server context
     * @param req DataSet message containing the necessary information
     * @param res Success message, contains true if operation successfull, false otherwise
     */
    grpc::Status Arax_data_set(grpc::ServerContext *ctx,
      const arax::DataSet *req, arax::Empty *res) override;

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
     * Similar to Arax_data_get
     * This one should be used for returned data that are over 1 MB in size
     *
     * @param ctx    Server Context
     * @param req    ResourceID message holding the ID of the buffer
     * @param writer ServerWriter instance to write to stream
     *
     * @return The appropriate status code
     */
    grpc::Status Arax_large_data_get(grpc::ServerContext *ctx,
      const arax::ResourceID *req, grpc::ServerWriter<arax::DataSet> *writer) override;

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
      const arax::ResourceID *req, arax::Empty *res) override;

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
      const arax::TaskMessage *req, arax::Empty *res) override;

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

    /*
     * Initialize a new arax_data_s object
     *
     * @param  ctx Server context
     * @param  req Message with the requested size for the data
     * @param  res Message with the unique ID of the resource
     *
     * @return The appropriate status code
     */
    grpc::Status Arax_data_init(grpc::ServerContext *ctx,
      const arax::AraxData *req, arax::ResourceID *res) override;

    /*
     * Initialize a new arax_data_s object with an aligned buffer
     *
     * @param  ctx ServerContext
     * @param  req Message with the requested size and allignment
     * @param  res Message with the ID of the resource
     *
     * @return The appropriate status code
     */
    grpc::Status Arax_data_init_aligned(grpc::ServerContext *ctx,
      const arax::AraxData *req, arax::ResourceID *res) override;

    /*
     * Initialize data remote (accelerator) buffer
     *
     * @param ctx Server Context
     * @param req DataSet message with the buffer and accelerator identifiers
     * @param res Empty message
     *
     * @return The appropriate status code
     */
    grpc::Status Arax_data_allocate_remote(grpc::ServerContext *ctx,
      const arax::DataSet *req, arax::Empty *res) override;

    /*
     * Method to call multiple task_issues in a single stream
     *
     * @param ctx Server Context
     * @param req stream ServerReaderWriter object to read from and
     *                   write to the client
     *
     * @return The appropriate status code
     */
    inline grpc::Status Arax_task_issue_streaming(grpc::ServerContext * ctx,
      grpc::ServerReaderWriter<arax::ResourceID, arax::TaskRequest> *stream) 
    {
      #ifdef DEBUG
      assert(ctx);
      assert(stream);
      #endif

      arax::TaskRequest req;
      arax::ResourceID res;

      if (!stream->Read(&req)) {
          std::string err_message("Stream broke");
          return grpc::Status(grpc::StatusCode::DATA_LOSS, err_message);
      }

      /* Do this once, and then get the host argument in the loop */
      uint64_t accel   = req.accel();
      uint64_t proc    = req.proc();
      size_t in_count  = req.in_count();
      size_t out_count = req.out_count();
      void *host_init  = (void *) req.host_init().data();
      size_t host_size = req.host_size();

      /* -- Fix the buffer i/o arrays -- */
      arax_buffer_s in_buffer[in_count];
      arax_buffer_s out_buffer[out_count];

      uint64_t *in  = (uint64_t *) req.in_buffer().data();
      uint64_t *out = (uint64_t *) req.out_buffer().data();

      for (auto i = 0; i < in_count; i++) {
          in_buffer[i] = buffers[in[i]];
      }

      for (auto i = 0; i < out_count; i++) {
          out_buffer[i] = buffers[out[i]];
      }

      arax_accel *exec   = arax_accels[accel];
      arax_proc *process = arax_processes[proc];

      /* Call the first arax_task_issue outside of the loop */
      arax_task *task = arax_task_issue(exec, process, host_init, host_size, in_count, in_buffer, out_count, out_buffer);

      uint64_t id = get_unique_id();
      insert_pair(arax_tasks, id, task);

      // Send the ID to the client
      res.set_id(id);

      if (!stream->Write(res)) {
          std::string err_message("Stream broke");
          return grpc::Status(grpc::StatusCode::DATA_LOSS, err_message);
      }

      while (stream->Read(&req)) {
          host_init = (void *) req.host_init().data();
          task      = arax_task_issue(exec, process, host_init, host_size, in_count, in_buffer, out_count, out_buffer);

          if(task == NULL){
            std::string err_message("-- Task failed (in Arax_task_issue_streaming)");
            return grpc::Status(grpc::StatusCode::ABORTED, err_message);
          }
          id        = get_unique_id();
          insert_pair(arax_tasks, id, task);

          res.set_id(id);

          if (!stream->Write(res)) {
              std::string err_message("Stream broke");
              return grpc::Status(grpc::StatusCode::DATA_LOSS, err_message);
          }
      }

      return grpc::Status::OK;
    } // AraxServer::Arax_task_issue_streaming
};

/*
 * Signal hanlder callback
 */
void signal_handler_callback(int signum);

extern bool shutdown_required;
extern std::mutex mutex;
extern std::condition_variable cond;

#endif /* #ifndef SERVER_H */
