#include "server.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using std::chrono::system_clock;
using grpc::Status;
using grpc::StatusCode;

using google::protobuf::Empty;

using namespace arax;

/*
 * Add some colors for the output
 */
#ifdef __linux__
  #define ERROR_COL "\033[1;38;5;9;1m"
  #define SUCCESS_COL "\033[1;37;38;5;10m"
  #define RESET_COL "\033[0m"
#endif /* #ifdef __linux__ */

#define MAX_NAME_LENGTH 1024

/*
 * Constructor
 * Initialize the arax service
 */
AraxServer::AraxServer(){
  /* initialize arax */
  std::cout << "-- Initializing Arax --\n";
try{
  pipe_s = arax_init();
 
  if(pipe_s == NULL){
#ifdef __linux__
      std::stringstream ss;
      ss << ERROR_COL << "Arax failed to initialize, please try again later"  << RESET_COL << "\n";
      throw std::runtime_error(ss.str()); 
#else
      throw std::runtime_error("Arax failed to initialize, please try again later\n");
#endif /* ifdef __linux__ */

  }else{
#ifdef __linux__ 
    std::stringstream ss;
    ss << SUCCESS_COL << "Arax was initialized successfully" << RESET_COL << "\n";
    std::cout << ss.str();
#else
    std::cout << "Arax was initialized successfully\n";
#endif /* ifdef __linux__ */
  }
}
 catch(std::runtime_error& e){
   std::cerr << e.what();
   exit(1);
 }
  accels = 0;
  unique_id = 1;
}

/*
 * Destructors
 * Exit and cleanup the arax service
 */
AraxServer::~AraxServer(){
  /* Leave arax */
  std::cout << "-- Exiting Arax --\n";
  arax_exit();
}

/*
 * -------------------- Arax Services Implementations --------------------
 */

/*
 * Initialize Arax
 *
 * Note: This function call is now not callable form the gRPC client side, but rather
 *       it's invoced in the AraxServerConstructor
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
// Status AraxServer::Arax_init(ServerContext* ctx, const Empty* req, Empty* res){
//   pipe_s = arax_init(); 

//   if(!pipe_s){
//     std::string error_msg("Arax failed to initialize ..\n");
//     std::string details("arax_init() failed to return an arax_pipe_s instance\n");
//     return Status(StatusCode::OK, error_msg, details);
//   }

//   return Status::OK;
// }

/*
  * Exit and cleanup arax
  */
// Status AraxServer::Arax_exit(ServerContext* ctx, const Empty* req, Empty* res) {

//   arax_exit();

//   return Status::OK;
// }

/*
 * Clean/Delete the shared segment
 * \note This should only be called when there are no uses of the shared segment
 * \note Behaviour undefined if called with processes accessing the shared segment
 *
 * @return The appropriate status code 
 */
Status AraxServer::Arax_clean(ServerContext* ctx, const Empty* req, Empty* res){
  int result = arax_clean();

  if(result == 0){
    std::string error_msg("-- Arax failed to delete the shared segment --");
    return Status(StatusCode::INTERNAL, error_msg);
  }

  return Status::OK;
}

/* -------------------- Accel_list and Accel_free have old implementations. If they are needed in the future, revisit them, else delete  */
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
  Status AraxServer::Arax_accel_list(ServerContext* ctx, const AccelListRequest* req, AccelListResponse* res){

    // Extract the information from the request
    int type = req->type();
    bool physical = req->physical();

    int accel_num = arax_accel_list((arax_accel_type_e)type, physical, &accels);
#ifdef DEBUG
    assert(accels);
#endif /* #ifdef DEBUG */

    res->set_accel_num(accel_num);

    // Save descriptor details
    int index = 0;
    while(*(accels+index)){
      AccelDescriptor* desc = res->add_descriptors();
      desc->set_index(index);
      desc->set_type(accel_type_string[arax_accel_type(*(accels+index))]);
     
      index++;
    }

    return Status::OK;
  }

 /*
  * Get arax revision
  *
  * @param ctx the Server context
  * @param req Empty message
  * @param res Empty message
  *
  * @return the appropriate status code
  */
  Status AraxServer::Arax_pipe_get_revision(ServerContext* ctx, const Empty* req, RevisionResponse* res){

#ifdef DEBUG
    assert(pipe_s);
#endif /* #ifdef DEBUG */

    /* 
     * There are 3 different error codes we could use for situations like this:
     * 1. FAILED_PRECONDITION
     * 2. INTERNAL
     * 3. ABORTED
     */
    if(!pipe_s){
      std::string error_msg("-- Arax failed to initialize.. --");
      res->set_revision("");
      return Status(StatusCode::FAILED_PRECONDITION, error_msg);
    }

    std::string revision(arax_pipe_get_revision(pipe_s));

    res->set_revision(revision);

    return Status::OK; 
  }

  /*
   * Increase process counter for pipe
   * 
   * @param ctx the Server Context
   * @param req Empty message
   * @param res ProcCounter message
   *
   * @return the appropriate status code
   */
  Status AraxServer::Arax_pipe_add_process(ServerContext* ctx, const Empty* req, ProcCounter* res){

#ifdef DEBUG
    assert(pipe_s);
#endif /* #ifdef DEBUG */

    if(!pipe_s){
      std::string error_msg("-- The server failed to initialize Arax, try again later --");
      return Status(StatusCode::FAILED_PRECONDITION, error_msg);
    }

    uint64_t proc_counter = arax_pipe_add_process(pipe_s);

    res->set_proc_counter(proc_counter);

    return Status::OK;
  }
  
  /*
   * Decrease process counter for pipe
   *
   * @param ctx The server context
   * @param req Empty message
   * @param res ProcCounter message to encapsulate the number of active processes
   *            before removing issuer
   */
  Status AraxServer::Arax_pipe_del_process(ServerContext* ctx, const Empty* req, ProcCounter* res) {

#ifdef DEBUG
    assert(pipe_s);
#endif /* #ifdef DEBUG */

    if(!pipe_s){
      std::string error_msg("-- The server failed to initialize Arax, try again later .. --");
      return Status(StatusCode::FAILED_PRECONDITION, error_msg);
    }

    uint64_t proc_counter = arax_pipe_del_process(pipe_s);

    res->set_proc_counter(proc_counter);

    return Status::OK;
  }
  
  /*
   * Free memory of accelerator array returned by arax_accel_list
   *
   * @param ctx The server context
   * @param req Empty request message
   * @param res Empty response message
   *
   * @return The appropriate status code
   */
  Status AraxServer::Arax_accel_list_free(ServerContext* ctx, const Empty* req, Empty* res){

#ifdef DEBUG
    assert(accels);
#endif /* #ifdef DEBUG */

    if(!accels){
      std::string msg("-- Accelerator list has not been acquired, and therefore cannot be freed --");
      return Status(StatusCode::FAILED_PRECONDITION, msg);
    }

    arax_accel_list_free(accels);

    return Status::OK;
  }
 
  /*
   * ----- This one as well is an old implementation. If we ever need this is the future, revisit it else delete ------ 
   * Return the type of accelerator specified by the client
   * 
   * @param ctx The server context
   * @param req Arax descriptor, with the index of the descriptor
   *            in the accels array
   * @param res Response object with the type of accelerator
   *
   * @return the appropriate status code
   */
  Status AraxServer::Arax_accel_type(ServerContext* ctx, const AccelDescriptor* req, AccelListRequest* res){

    /* This function, if it is to be used, needs to be changed completely */

    // get the index
    int index = req->index();

    if(*(accels+index) == 0){
      std::string error_msg("-- Invalid index given for the accelerator descriptors array --");
      return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    arax_accel_type_e type = (arax_accel_type(*(accels+index)));

    res->set_type(type);

    return Status::OK;
  } 
  
  /*
   * Create an arax_buffer_s object
   *
   * @param ctx The server context
   * @param req RequestBuffer object with the size and the ID of the buffer
   * @param res ResourceID message, with the ID of the newly created resource 
   *
   * @return The appropriate status code
   */
  grpc::Status AraxServer::Arax_buffer(grpc::ServerContext* ctx, const RequestBuffer* req, ResourceID* res) {
#ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
#endif /* ifdef DEBUG */

    size_t size = req->buffer_size();
    uint64_t id = req->id();
    arax_buffer_s buffer = ARAX_BUFFER(size);

    res->set_id(get_unique_id());
    return Status::OK;
  }
  
  /*
   * Register a new process 'func_name'
   * Processes are accelerator agnostic and initially have no 'Implementations'/functors
   * Created arax_proc* identifies given function globally
   *
   * @param ctx Server context
   * @param req ProcRequest message holding 'func_name'
   * @param res ResourceID message holding the id of the arax_proc resource
   *
   * @return The appropriate status code
   */
  Status AraxServer::Arax_proc_register(ServerContext* ctx, const ProcRequest* req, ResourceID* res){
 
/* Preconditions */
#ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
#endif /* ifdef DEBUG */

    std::string func_name = req->func_name();
    arax_proc* proc = arax_proc_register(func_name.c_str());

    if(!proc){
      std::string error_msg("Arax failed to register the process with name '" + func_name + "'");
      res->set_id(0);
      return Status(StatusCode::INTERNAL, error_msg);
    }
#ifdef DEBUG
    assert(proc);
#endif
    // insert new process in the processes mapping
    res->set_id(get_unique_id());
    arax_processes.insert(std::pair<uint64_t, arax_proc*>(res->id(), proc));

    return Status::OK;
  }
  
  /*
   * Delete registered arax_proc pointer
   * 
   * @param ctx Server Context
   * @param req AraxProc message with the ID of the arax_proc
   * @param res Response which holds the return value of the arax_proc_put function
   *
   * @return The appropriate status code
   */
  Status AraxServer::Arax_proc_put(ServerContext* ctx, const AraxProc* req, ProcCounter* res){
/* Preconditions */
#ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
#endif /* ifdef DEBUG */
  
    uint64_t id = req->id();

    // check if process with name proc exists
    if(!check_if_exists(arax_processes, id)){
      res->set_proc_counter(0);
      std::string error_msg("-- No process with name '" + std::to_string(id) + "' exists --");
      return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    res->set_proc_counter(arax_proc_put(arax_processes[id]));

    return Status::OK;
  }
  
  /*
   * Acquire a virtual accelerator of the given type
   *
   * @param ctx Server context
   * @param req AccelRequest message with the name and type
   * @param res ResourceID message with the returned id for the resource 
   *
   * @return The appropriate status code
   */
  Status AraxServer::Arax_accel_acquire_type(ServerContext* ctx, const AccelRequest* req, ResourceID* res){
/* Preconditions */
#ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
#endif /* ifdef DEBUG */

    unsigned int type = req->type();

    arax_accel* accel = arax_accel_acquire_type((arax_accel_type_e)type);

    /* INTERNAL seems to be a 'harsh' error code to return in this scenario. Maybe a more appropriate would be INVALID_ARGUMENT? */
    if(!accel){
      std::string error_msg("-- Failed to acquire accelerator of given type --");
      res->set_id(0);
      return Status(StatusCode::INTERNAL, error_msg);
    }

    uint64_t id = get_unique_id();
    res->set_id(id);

    // Insert the new accel to map
    insert_pair(arax_accels, id, accel);

    return Status::OK;
  }
  
  /*
   * Release a previously acquired accelerator 
   *
   * @param ctx Server context
   * @param req ResourceID message with the resource ID 
   * @param res Empty message 
   *
   * @return The appropriate status code
   */
  Status AraxServer::Arax_accel_release(ServerContext* ctx, const ResourceID* req, Empty* res){
/* Preconditions */
#ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
#endif /* ifdef DEBUG */

    uint64_t id = req->id();

    if(arax_accels.find(id) != arax_accels.end()){ /* accel var with ID exists*/
      arax_accel_release(&arax_accels[id]);
      arax_accels.erase(id);
    }else{
      std::string error_msg("-- Accelerator with ID '" + std::to_string(id) + "' does not exist --");
      return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    return Status::OK; 
  }

/*
   * -------- This one needs revisiting, temporary implementation to make the noop example work --------
   * Copy data to buffer
   *
   * @param ctx Server context
   * @param req DataSet message containing the data to be passed to buffer + the name of the buffer
   * @param res Empty message, contains true if operation successfull, false otherwise
   */
  grpc::Status AraxServer::Arax_data_set(grpc::ServerContext *ctx, const arax::DataSet *req, Empty *res){
#ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
#endif /* ifdef DEBUG */

    uint64_t buffer = req->buffer();
    uint64_t accel = req->accel();
    std::string data = req->str_val();

    if(!check_if_exists(buffers, buffer)){
      std::string error_msg("-- No buffer exists with ID'" + std::to_string(buffer) + "' --");
      return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    if(!check_if_exists(arax_accels, accel)){
      std::string error_msg("-- No accelerator with ID'" + std::to_string(accel) + "' exists --");
      return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    arax_data_set(buffers[buffer], arax_accels[accel], &data);
    return Status::OK;
  }
  
  /*
   * Issue a new arax task
   *
   * @param ctx The server context
   * @param req TaskRequest message, containing all the necessary info for the new task
   * @param res ResourceID message with the id of the resource
   *
   * @return The appropriate status code
   */
  Status AraxServer::Arax_task_issue(ServerContext* ctx, const TaskRequest* req, ResourceID* res){
#ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
#endif /* ifdef DEBUG */

    uint64_t accel = req->accel();
    uint64_t proc = req->proc();
    uint64_t in_buffer = req->in_buffer();
    uint64_t out_buffer = req->out_buffer();
    size_t in_count = req->in_count();
    size_t out_count = req->out_count();

    // Check if valid names for data structures 
    // Buffers
    if(!check_if_exists(buffers, in_buffer)) {
      std::string error_msg("-- No buffer with ID '" + std::to_string(in_buffer) + "' exists --");
      return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    if(!check_if_exists(buffers, out_buffer)) {
      std::string error_msg("-- No buffer with ID '" + std::to_string(out_buffer) + "' exists --");
      return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    // Arax accel
    if(!check_if_exists(arax_accels, accel)){
      std::string error_msg("-- No accelarator with ID '" + std::to_string(accel) + "' exists --");
      return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    // Arax proces
    if(!check_if_exists(arax_processes, proc)){
      std::string error_msg("-- No process with ID '" + std::to_string(proc) + "' exists --");
      return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    arax_buffer_s input = buffers[in_buffer];
    arax_buffer_s output = buffers[out_buffer];
    arax_accel* exec = arax_accels[accel];
    arax_proc* process = arax_processes[proc];

    arax_task* task = arax_task_issue(exec, process, 0, 0, in_count, &input, out_count, &output);

    if(task == NULL){
      std::string error_msg("-- Failed to issue task --");
      return Status(StatusCode::ABORTED, error_msg);
    }

    res->set_id(get_unique_id());
    // insert task to map
    arax_tasks.insert(std::pair<uint64_t, arax_task*>(res->id(), task));

    return Status::OK; 
  }
  
  /*
   * Decrease ref counter of task
   *
   * @param ctx The Server Context
   * @param req Task message holding the name of the task to be processed
   * @param res Empty message
   *
   * @return The appropriate status code
   */
  Status AraxServer::Arax_task_free(ServerContext* ctx, const Task* req, Empty* res){
#ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
#endif /* ifdef DEBUG */

    uint64_t task = req->task_id();

    // check if task exists
    if(!check_if_exists(arax_tasks, task)){
      std::string error_msg("-- Task with ID '" + std::to_string(task) + "' does not exist --");
      return Status(StatusCode::FAILED_PRECONDITION, error_msg);
    }

    arax_task_free(arax_tasks[task]);

    return Status::OK;
  }
  
  /*
   * Wait for an issued task to complete or fail
   *
   * @param ctx The server context
   * @param req Task message with the name of the task
   * @param res Task message with the state of the task
   *
   * @return The appropriate status code
   */
  Status AraxServer::Arax_task_wait(ServerContext* ctx, const Task* req, Task* res){
 
    uint64_t id = req->task_id();

    // See if task with the given ID exists
    if(!check_if_exists(arax_tasks, id)){
      std::string error_msg("-- There is no task registered with ID '" + std::to_string(id) + "' --");
      return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    unsigned int state = arax_task_wait(arax_tasks[id]);

    res->set_task_state(state);
    return Status::OK;
  }
  
  /*
   * Return an orphan/unassigned virtual accelerator or null.
   * Function will sleep if no orphans exist at the time of the call.
   *
   * @param ctx The server context
   * @param req Empty message
   * @param res ResourceID message with the id of the resource
   *
   * @return The appropriate status code 
   */
  Status AraxServer::Arax_pipe_get_orphan_vaccel(ServerContext* ctx, const Empty* req, ResourceID* res){

#ifdef DEBUG
  assert(ctx);
  assert(req);
  assert(res);
  assert(pipe_s);
#endif

    // Arax should be initialized
    if(!pipe_s){
      std::string error_msg("-- Arax has not been initialized --");
      std::string details("-- The server has failed to initialize Arax, try again later --");
      return Status(StatusCode::INTERNAL, error_msg, details);
    }

    arax_vaccel_s* v = arax_pipe_get_orphan_vaccel(pipe_s);

#ifdef DEBUG
    assert(v);
#endif

    // Check if operation successfull
    if(!v){
      std::string error_msg("-- Arax failed to return an orphan vaccel --");
      std::string details("-- Either the system faced an issue, or there is no orphan virtual accelerator --");
      return Status(StatusCode::INTERNAL, error_msg);
    } 

    // give ID
    uint64_t vaccel = get_unique_id();
    res->set_id(vaccel);
    // add pair to map
    arax_vaccels.insert(std::pair<uint64_t, arax_vaccel_s*>(vaccel, v));

    return Status::OK;
  }
  
  /*
   * Function to return an ID for a data structure
   * This ID is an unsigned integer
   * Every ID that is returns is unique
   * Resets to 0 for a new server
   *
   * @return unsigned integer Unique ID
   */
  uint64_t AraxServer::get_unique_id(){
    unique_id++;
    return unique_id-1; 
  }
  
  /*
   * Add vaccel with \c ID to the list of orphan_vacs/ unassigned accels
   *
   * @param ctx The server context
   * @param req ResourceID message containing the ID of the vaccel
   * @param res Empty message
   *
   * @return The appropriate status code
   */
  Status AraxServer::Arax_pipe_add_orphan_vaccel(grpc::ServerContext* ctx, const ResourceID* req, Empty* res){
#ifdef DEBUG
  assert(ctx);
  assert(req);
  assert(res);
  assert(pipe_s);
#endif

    if(!pipe_s){
      std::string error_msg("-- Arax has not been initialized --");
      std::string details("-- Arax failed to initialize on the start of the service. Try again later --");
      return Status(StatusCode::INTERNAL, error_msg, details);
    }

    uint64_t id = req->id();

    if(!check_if_exists(arax_vaccels, id)){
      std::string error_msg("-- No vaccel exists with ID '" + std::to_string(id) + "' --");
      return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    arax_vaccel_s* vac = arax_vaccels[id];

#ifdef DEBUG
    assert(vac);
#endif

    arax_pipe_add_orphan_vaccel(pipe_s, vac);

    return Status::OK;
  }
  
  /*
   * Function will check if there are orphan vaccels
   *
   * @param ctx The server context
   * @param req Empty message
   * @param res Success message, containing true if there are orphan vaccels, false otherwise
   *
   * @return The appropriate status code
   */
  Status AraxServer::Arax_pipe_have_orphan_vaccels(ServerContext* ctx, const Empty* req, Success* res){
#ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    assert(pipe_s);
#endif

    if(!pipe_s){
      std::string error_msg("-- Arax has not been initialized --");
      std::string details("-- Arax failed to initialize on the start of the service. Try again later --");
      return Status(StatusCode::INTERNAL, error_msg, details);
    }

    int result = arax_pipe_have_orphan_vaccels(pipe_s);

    result == 0 ? res->set_success(false) : res->set_success(true);

    return Status::OK;
  }
  
    /*
   * Remove specified vac from list of orphan vacs
   *
   * @param ctx The server context
   * @param req ResourceID message, containing the ID of the vaccel
   * @param res Empty message
   *
   * @return The appropriate Status code
   */
  Status AraxServer::Arax_pipe_remove_orphan_vaccel(ServerContext* ctx, const ResourceID* req, Empty* res){
#ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    assert(pipe_s);
#endif

    if(!pipe_s){
      std::string error_msg("-- Arax has not been initialized --");
      std::string details("-- Arax failed to initialize on the start of the service. Try again later --");
      return Status(StatusCode::INTERNAL, error_msg, details);
    }
   
    uint64_t id = req->id();

    // Check if there is an orphan vaccel with this ID
    if(!check_if_exists(arax_vaccels, id)){
      std::string error_msg("-- There is no vaccel with ID '" + std::to_string(id) + "' --");
      return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    /* Remove the orphan vaccel */
    arax_pipe_remove_orphan_vaccel(pipe_s, arax_vaccels[id]);
    /* Remove from map */
    arax_accels.erase(id);

/* Check if the orphan vaccel pair was removed successfully from the mapping */
#ifdef DEBUG
  assert(!check_if_exists(arax_accels, id));
#endif

    return Status::OK;
  }

// ---------------------------------- Utility Functions --------------------------------------


/*
 * -------------- This function was used back when we used string identifiers rather than uint64_t. --------------
 * Function to sanitize user input, when it comes to strings
 * If not sanitizable, return false
 *
 * Used in the functions where we have to access/create/process tasks/vaccels/buffers etc
 *
 * @param input Input string
 *
 * @return true if valid, false otherwise
 */
bool sanitize_input(const char* input){
#ifdef DEBUG
  assert(input);
#endif
  
  if(strlen(input) > MAX_NAME_LENGTH){
    std::cout << "String length should have max " << MAX_NAME_LENGTH << " characters long.\n";
    return false;
  }

  /* And then do more ... */

  return true;
}

/*
 * Function to begin running the server
 *
 * @return Void
 *
 * @param addr The server address, which will be represented by a std::string
 */
void RunServer(std::string address){

  // Create the service instance
  std::cout << "Starting the server ..\n";
  AraxServer service;

  ServerBuilder builder;
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << address << "\n";
  server->Wait();

  return;
}

 
int main(){

  // Stuff we still need to do:
  // 1. Safe shutdown
  // 2. Timeouts Maybe
  // 3. and more

  // RunServer("0.0.0.0:50051");
  std::string address("0.0.0.0:50051");
  std::cout << "Starting the server ..\n";
  AraxServer service;
  ServerBuilder builder;
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << address << "\n";
  server->Wait();

  return 0;
}






