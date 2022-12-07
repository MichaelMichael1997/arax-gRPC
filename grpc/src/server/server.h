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
  uint64_t unique_id; // Current value for the unique id to be given to a resource. Starts from 1.
  arax_pipe_s *pipe_s; // Arax pipe_s instance. Arax initialized in the Constructor
  arax_accel **accels; // Not used anymore. Was used for the functions accel_list, accel_list_free

  // String arrays to map enumarations to strings. This is not used anymore. 
  std::string accel_type_string[10] = {"ANY", "GPU", "GPU_SOFT", "CPU", "SDA",
                                       "NANO_ARM", "NANO_CORE", "OPEN_CL", "HIP"};
 
  /* Mappings to store resources, with key a unique uint64_t ID */
  std::map<uint64_t, arax_buffer_s> buffers;
  std::map<uint64_t, arax_proc*> arax_processes;
  std::map<uint64_t, arax_accel*> arax_accels;
  std::map<uint64_t, arax_task*> arax_tasks;
  std::map<uint64_t, arax_vaccel_s*> arax_vaccels;


  /*
   * Template function to add a pair into an std::map
   *
   * @param mapping Map with key uint64_t, the ID of the Resource, and the type T as value
   * @param ID the id of the resource 
   * @param t The value T to be inserted
   *
   * @return true if operation succesfull, false if the key already exists in the map
   */
  template <typename T> bool insert_pair(std::map<uint64_t, T> &mapping, uint64_t ID, const T t){
    // check if name in mapping
    if (mapping.find(ID) == mapping.end()){ /* key does not exist in the mapping */
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
  template<typename T> bool check_if_exists(const std::map<uint64_t,T> map, uint64_t key){
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

public:
  /*
   * Constructors
   */
  AraxServer();

  /*
   * Destructosrs
   */
  ~AraxServer();

  /*
   * -------------------- Arax Services Implementations --------------------
   */

  /* ------ IMPORTANT: arax_init and arax_exit are called automatically by the server. The client does not need to make calls to these */ 

  /*
   * Initialize Arax
   *
   * This should be called by all applications prior to
   * using any other Arax function
   *
   * @return the appropriate status code depending on if the operation was successfull or not
   *
   * @param ctx The server context
   * @param empty Empty object, used for gRPC functions that require no parameters
   * @param pipe Arax_pipe_s object
   */
  // grpc::Status Arax_init(grpc::ServerContext* ctx, const google::protobuf::Empty* req,
  //                        google::protobuf::Empty* res) override;

  /*
   * Exit and cleanup arax
   */
  // grpc::Status Arax_exit(grpc::ServerContext* ctx, const google::protobuf::Empty* req,
  //                        google::protobuf::Empty* res) override;

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
   * Get the number of accelerators of provided type
   * If 0 is the answer, no matching devices were found
   * If accels is not NULL, an array with all matching accelerator
   * descriptors is allocated and passed to the user
   * TODO: See how we can represent this array of descriptors using protos
   *
   * @param ctx Server context
   * @parram req The request message with the necessary data fields
   * @param res The response with all the data to be sent back to the user
   *
   * @return The appropriate status code, based on the success of the operation
   */
  grpc::Status Arax_accel_list(grpc::ServerContext *ctx, const arax::AccelListRequest *req,
                               arax::AccelListResponse *res) override;

  /*
   * Get arax revision
   *
   * @param ctx the Server context
   * @param req Empty message
   * @param res RevisionReponse message
   *
   * @return the appropriate status code
   */
  grpc::Status Arax_pipe_get_revision(grpc::ServerContext *ctx,
                                      const google::protobuf::Empty *req, arax::RevisionResponse *res) override;

  /*
   * Increase process counter for pipe
   *
   * @param ctx the Server Context
   * @param req Empty message
   * @param res Response message
   *
   * @return the appropriate status code
   */
  grpc::Status Arax_pipe_add_process(grpc::ServerContext *ctx,
                                     const google::protobuf::Empty *req, arax::ProcCounter *res) override;

  /*
   * Decrease process counter for pipe
   *
   * @param ctx The server context
   * @param req Empty message
   * @param res Response message to encapsulate the number of active processes
   *            before removing issuer
   *
   * @return The appropriate status code
   */
  grpc::Status Arax_pipe_del_process(grpc::ServerContext *ctx,
                                     const google::protobuf::Empty *req, arax::ProcCounter *res) override;

  /*
   * Free memory of accelerator array returned by arax_accel_list
   *
   * @param ctx The server context
   * @param req Empty request message
   * @param res Empty response message
   *
   * @return The appropriate status code
   */
  grpc::Status Arax_accel_list_free(grpc::ServerContext *ctx,
                                    const google::protobuf::Empty *req, google::protobuf::Empty *res) override;

  // TODO: request/response messages are all over the place right now, fix them

  /*
   * Return the type of accelerator specified by the client
   *
   * @param ctx The server context
   * @param req Arax descriptor, with the index of the descriptor
   *            in the accels array
   * @param res Response object with the type of accelerator
   *
   * @return the appropriate status code
   */
  grpc::Status Arax_accel_type(grpc::ServerContext *ctx,
                               const arax::AccelDescriptor *req, arax::AccelListRequest *res) override;

  /*
   * Create an arax_buffer_s object
   *
   * @param ctx The server context
   * @param req RequestBuffer object with the size and the ID of the buffer
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
   * Issue a new arax task
   *
   * @param ctx The server context
   * @param req TaskRequest message, containing all the necessary info for the new task
   * @param res Resource message with the id of the resource
   *
   * @return The appropriate status code
   */
  grpc::Status Arax_task_issue(grpc::ServerContext* ctx, 
          const arax::TaskRequest* req, arax::ResourceID* res) override;

  /*
   * Decrease ref counter of task
   *
   * @param ctx The Server Context
   * @param req Task message holding the ID 
   * @param res Empty message 
   *
   * @return The appropriate status code
   */
  grpc::Status Arax_task_free(grpc::ServerContext* ctx,
          const arax::Task* req, google::protobuf::Empty* res) override;

  /*
   * Wait for an issued task to complete or fail
   *
   * @param ctx The server context
   * @param req Task message with the name of the task
   * @param res Task message with the state of the task
   *
   * @return The appropriate status code
   */
  grpc::Status Arax_task_wait(grpc::ServerContext* ctx,
          const arax::Task* req, arax::Task* res) override;

  /*
   * Return an orphan/unassigned virtual accelerator or null.
   * Function will sleep if no orphans exist at the time of the call.
   *
   * @param ctx The server context
   * @param req Empty message
   * @param res ResourceID message with the ID of the resource 
   *
   * @return The appropriate status code 
   */
  grpc::Status Arax_pipe_get_orphan_vaccel(grpc::ServerContext* ctx,
          const google::protobuf::Empty* req, arax::ResourceID* res) override;

  /*
   * Add vaccel with \c ID to the list of orphan_vacs/ unassigned accels
   *
   * @param ctx The server context
   * @param req ResourceID message containing the ID of the vaccel
   * @param res Empty message
   *
   * @return The appropriate status code
   */
  grpc::Status Arax_pipe_add_orphan_vaccel(grpc::ServerContext* ctx, 
          const arax::ResourceID* req, google::protobuf::Empty* res) override;
  
  /*
   * Function will check if there are orphan vaccels
   *
   * @param ctx The server context
   * @param req Empty message
   * @param res Success message, containing true if there are orphan vaccels, false otherwise
   *
   * @return The appropriate status code
   */
  grpc::Status Arax_pipe_have_orphan_vaccels(grpc::ServerContext* ctx,
          const google::protobuf::Empty* req, arax::Success* res) override;

  /*
   * Remove specified vac from list of orphan vacs
   *
   * @param ctx The server context
   * @param req ResourceID message, containing the ID of the vaccel
   * @param res Empty message
   *
   * @return The appropriate Status code
   */
  grpc::Status Arax_pipe_remove_orphan_vaccel(grpc::ServerContext* ctx,
          const arax::ResourceID* req, google::protobuf::Empty* res) override;
};

// ---------------------------------- Utility Functions --------------------------------------


/*
 * Function to sanitize user input, when it comes to strings
 * If not sanitizable, return false
 *
 * Used in the functions where we have to access/create/process tasks/vaccels/buffers etc
 *
 * @param input Input string
 *
 * @return true if valid, false otherwise
 *
 * ------- This function is not used anymore, unless we ever need to go back to string identifiers, instead of uint64_t -------- 
 */
bool sanitize_input(const char *input);

/*
 * Function to begin running the server
 *
 * @return Void
 *
 * @param addr The server address, which will be represented by a std::string
 */
void RunServer(std::string address);

#endif /* #ifndef SERVER_H */
