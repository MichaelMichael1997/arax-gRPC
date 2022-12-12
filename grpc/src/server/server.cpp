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
#define ERROR_COL   "\033[1;38;5;9;1m"
#define SUCCESS_COL "\033[1;37;38;5;10m"
#define RESET_COL   "\033[0m"
#endif /* #ifdef __linux__ */

/*
 * Constructor to start the server and init arax
 *
 * @param addr The address to connect
 */
AraxServer::AraxServer(const char *addr)
{
    /*----- initialize arax -----*/
    std::cout << "-- Initializing Arax --\n";
    try{
        pipe_s = arax_init();

        if (pipe_s == NULL) {
            #ifdef __linux__
            std::stringstream ss;
            ss << ERROR_COL << "Arax failed to initialize, please try again later" << RESET_COL << "\n";
            throw std::runtime_error(ss.str());
            #else
            throw std::runtime_error("Arax failed to initialize, please try again later\n");
            #endif /* ifdef __linux__ */
        } else {
            #ifdef __linux__
            std::stringstream ss;
            ss << SUCCESS_COL << "Arax was initialized successfully" << RESET_COL << "\n";
            std::cout << ss.str();
            #else
            std::cout << "Arax was initialized successfully\n";
            #endif /* ifdef __linux__ */
        }
    }
    catch (std::runtime_error& e) {
        std::cerr << e.what();
        exit(EXIT_FAILURE);
    }

    /* -- Unique ID counter -- */
    unique_id = 1;

    /* ----- Init the server ------ */

    ServerBuilder builder;

    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(this);
    server = std::unique_ptr<Server>(builder.BuildAndStart());

    /* -- Start the server -- */
    start_server();
}

/*
 * Destructors
 * Exit and cleanup the arax service
 */
AraxServer::~AraxServer()
{
    /* -- Exit arax -- */
    std::cout << "-- Exiting Arax --\n";
    arax_exit();

    /* -- Shutdown Server -- */
    std::cout << "-- Shuting down --\n";
    shutdown_server();
}

/* ----- Server Start/Shutdown ------ */

/*
 * Function to start the server
 *
 * @return void
 */
void AraxServer::start_server()
{
    server->Wait();
}

/*
 * Function to shutdown the server
 *
 * @return void
 */
void AraxServer::shutdown_server()
{
    server->Shutdown();
}

/*
 * Function to return an ID for a data structure
 * This ID is an unsigned integer
 * Every ID that is returns is unique
 * Resets to 0 for a new server
 *
 * @return unsigned integer Unique ID
 */
uint64_t AraxServer::get_unique_id()
{
    unique_id++;
    return unique_id - 1;
}

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
Status AraxServer::Arax_clean(ServerContext *ctx, const Empty *req, Empty *res)
{
    int result = arax_clean();

    if (result == 0) {
        std::string error_msg("-- Arax failed to delete the shared segment --");
        return Status(StatusCode::INTERNAL, error_msg);
    }

    return Status::OK;
}

/*
 * Create an arax_buffer_s object
 *
 * @param ctx The server context
 * @param req RequestBuffer object with the size of the buffer
 * @param res ResourceID message, with the ID of the newly created resource
 *
 * @return The appropriate status code
 */
grpc::Status AraxServer::Arax_buffer(grpc::ServerContext *ctx, const RequestBuffer *req, ResourceID *res)
{
    #ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    #endif /* ifdef DEBUG */

    size_t size = req->buffer_size();
    arax_buffer_s buffer = ARAX_BUFFER(size);
    uint64_t id = get_unique_id();

    // Insert buffer to mapping
    insert_pair(buffers, id, buffer);

    res->set_id(id);
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
Status AraxServer::Arax_proc_register(ServerContext *ctx, const ProcRequest *req, ResourceID *res)
{
    /* Preconditions */
    #ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    #endif /* ifdef DEBUG */

    std::string func_name = req->func_name();
    arax_proc *proc       = arax_proc_register(func_name.c_str());

    if (!proc) {
        std::string error_msg("Arax failed to register the process with name '" + func_name + "'");
        res->set_id(0);
        return Status(StatusCode::INTERNAL, error_msg);
    }
    #ifdef DEBUG
    assert(proc);
    #endif
    // insert new process in the processes mapping
    res->set_id(get_unique_id());
    arax_processes.insert(std::pair<uint64_t, arax_proc *>(res->id(), proc));

    return Status::OK;
}

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
Status AraxServer::Arax_proc_get(ServerContext *ctx, const ProcRequest *req, ResourceID *res)
{
    #ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    #endif

    std::string func(req->func_name());

    arax_proc *proc = arax_proc_get(func.c_str());

    if (!proc) {
        std::string error("-- Could not retrieve process '" + func
          + "'. Maybe it has not been registered, or arax failed to retrieve it --");
        return Status(StatusCode::INVALID_ARGUMENT, error);
    }

    // Check if the retrieved process is already in the mapping
    for (auto i: arax_processes) {
        if (i.second == proc) { // Process already exists in the mapping
            res->set_id(i.first);
            return Status::OK;
        }
    }

    // Add the retrieved process to the arax_proc mapping, if it's not already there
    // Also return the ID of the resource
    res->set_id(get_unique_id());
    insert_pair(arax_processes, res->id(), proc);

    #ifdef DEBUG
    assert(check_if_exists(arax_processes, res->id()));
    #endif

    return Status::OK;
} // AraxServer::Arax_proc_get

/*
 * Delete registered arax_proc pointer
 *
 * @param ctx Server Context
 * @param req AraxProc message with the ID of the arax_proc
 * @param res Response which holds the return value of the arax_proc_put function
 *
 * @return The appropriate status code
 */
Status AraxServer::Arax_proc_put(ServerContext *ctx, const AraxProc *req, ProcCounter *res)
{
    /* Preconditions */
    #ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    #endif /* ifdef DEBUG */

    uint64_t id = req->id();

    // check if process with name proc exists
    if (!check_if_exists(arax_processes, id)) {
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
Status AraxServer::Arax_accel_acquire_type(ServerContext *ctx, const AccelRequest *req, ResourceID *res)
{
    /* Preconditions */
    #ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    #endif /* ifdef DEBUG */

    unsigned int type = req->type();

    arax_accel *accel = arax_accel_acquire_type((arax_accel_type_e) type);

    /* INTERNAL seems to be a 'harsh' error code to return in this scenario. Maybe a more appropriate would be INVALID_ARGUMENT? */
    if (!accel) {
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
Status AraxServer::Arax_accel_release(ServerContext *ctx, const ResourceID *req, Empty *res)
{
    /* Preconditions */
    #ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    #endif /* ifdef DEBUG */

    uint64_t id = req->id();

    if (arax_accels.find(id) != arax_accels.end()) { /* accel var with ID exists*/
        arax_accel_release(&arax_accels[id]);
        auto it = arax_accels.find(id);
        arax_accels.erase(it);
    } else {
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
grpc::Status AraxServer::Arax_data_set(grpc::ServerContext *ctx, const arax::DataSet *req, Empty *res)
{
    #ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    #endif /* ifdef DEBUG */

    uint64_t buffer = req->buffer();
    uint64_t accel  = req->accel();

    if (!check_if_exists(buffers, buffer)) {
        std::string error_msg("-- No buffer exists with ID'" + std::to_string(buffer) + "' --");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    if (!check_if_exists(arax_accels, accel)) {
        std::string error_msg("-- No accelerator with ID'" + std::to_string(accel) + "' exists --");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    arax_data_set(buffers[buffer], arax_accels[accel], &req->str_val());
    return Status::OK;
} // AraxServer::Arax_data_set

/*
 * Get data from buffer and return them to user
 *
 * @param ctx The server context
 * @param req ResourceID message holding the ID of the buffer
 * @param res DataSet message holding the data to be returned
 *
 * @return The appropriate status code
 */
Status AraxServer::Arax_data_get(ServerContext *ctx, const ResourceID *req, DataSet *res)
{
    #ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    #endif /* ifdef DEBUG */

    uint64_t id = req->id();

    /* Check if buffer with that ID exists */
    if (!check_if_exists(buffers, id)) {
        std::string error_msg("-- No buffer with ID '" + std::to_string(id) + "' exists --");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    size_t size = arax_data_size(buffers[id]);

    // Alocate memory for the data to be copied
    char *data = (char *) calloc(size, 1);

    #ifdef DEBUG
    assert(data);
    #endif

    if (!data) {
        std::string error("-- The system failed to allocate memory --");
        return Status(StatusCode::INTERNAL, error);
    }

    /* -- Get the data from the buffer -- */
    arax_data_get(buffers[id], data);

    if (!data) {
        std::string error_msg("-- Failed to fetch the data from the buffer --\n");
        return Status(StatusCode::INTERNAL, error_msg);
    }

    res->set_str_val("hello");

    return Status::OK;
} // AraxServer::Arax_data_get

/*
 * Get size of the specified data
 *
 * @param ctx The server context
 * @param req ResourceID message holding the ID of the buffer
 * @param res DataSet message holding the size of the data
 *
 * @return The appropriate status code
 */
Status AraxServer::Arax_data_size(ServerContext *ctx, const ResourceID *req, DataSet *res)
{
    #ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    #endif

    uint64_t id = req->id();

    // Check if buffer exists
    if (!check_if_exists(buffers, id)) {
        std::string error("-- Buffer with ID '" + std::to_string(id) + "' does not exist --");
        return Status(StatusCode::INVALID_ARGUMENT, error);
    }

    size_t size = arax_data_size(buffers[id]);

    res->set_data_size(size);

    return Status::OK;
}

/*
 * Mark data for deletion
 *
 * @param ctx The server context
 * @param req ResourceID messsage holding the ID of the buffer
 * @param res Empty message
 */
Status AraxServer::Arax_data_free(ServerContext *ctx, const ResourceID *req, Empty *res)
{
    #ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    #endif

    uint64_t id = req->id();

    // Check if buffer with ID exists
    if (!check_if_exists(buffers, id)) {
        std::string error("-- No buffer with ID '" + std::to_string(id) + "' exists --");
        return Status(StatusCode::INVALID_ARGUMENT, error);
    }

    arax_data_free(buffers[id]);

    // Remove the buffer from the mapping
    auto it = buffers.find(id);
    buffers.erase(it);

    #ifdef DEBUG
    assert(!check_if_exists(buffers, id));
    #endif

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
Status AraxServer::Arax_task_issue(ServerContext *ctx, const TaskRequest *req, ResourceID *res)
{
    #ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    #endif /* ifdef DEBUG */

    uint64_t accel      = req->accel();
    uint64_t proc       = req->proc();
    uint64_t in_buffer  = req->in_buffer();
    uint64_t out_buffer = req->out_buffer();
    size_t in_count     = req->in_count();
    size_t out_count    = req->out_count();

    // Check if valid names for data structures
    // Buffers
    if (!check_if_exists(buffers, in_buffer)) {
        std::string error_msg("-- No buffer with ID '" + std::to_string(in_buffer) + "' exists --");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    if (!check_if_exists(buffers, out_buffer)) {
        std::string error_msg("-- No buffer with ID '" + std::to_string(out_buffer) + "' exists --");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    // Arax accel
    if (!check_if_exists(arax_accels, accel)) {
        std::string error_msg("-- No accelarator with ID '" + std::to_string(accel) + "' exists --");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    // Arax proces
    if (!check_if_exists(arax_processes, proc)) {
        std::string error_msg("-- No process with ID '" + std::to_string(proc) + "' exists --");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    arax_buffer_s input  = buffers[in_buffer];
    arax_buffer_s output = buffers[out_buffer];
    arax_accel *exec     = arax_accels[accel];
    arax_proc *process   = arax_processes[proc];

    arax_task *task = arax_task_issue(exec, process, 0, 0, in_count, &input, out_count, &output);

    if (task == NULL) {
        std::string error_msg("-- Failed to issue task --");
        return Status(StatusCode::ABORTED, error_msg);
    }

    res->set_id(get_unique_id());
    // insert task to map
    arax_tasks.insert(std::pair<uint64_t, arax_task *>(res->id(), task));

    return Status::OK;
} // AraxServer::Arax_task_issue

/*
 * Decrease ref counter of task
 *
 * @param ctx The Server Context
 * @param req TaskMessage message holding the name of the task to be processed
 * @param res Empty message
 *
 * @return The appropriate status code
 */
Status AraxServer::Arax_task_free(ServerContext *ctx, const TaskMessage *req, Empty *res)
{
    #ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    #endif /* ifdef DEBUG */

    uint64_t task = req->task_id();

    // check if task exists
    if (!check_if_exists(arax_tasks, task)) {
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
 * @param req TaskMessage message with the name of the task
 * @param res TaskMessage message with the state of the task
 *
 * @return The appropriate status code
 */
Status AraxServer::Arax_task_wait(ServerContext *ctx, const TaskMessage *req, TaskMessage *res)
{
    uint64_t id = req->task_id();

    // See if task with the given ID exists
    if (!check_if_exists(arax_tasks, id)) {
        std::string error_msg("-- There is no task registered with ID '" + std::to_string(id) + "' --");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    unsigned int state = arax_task_wait(arax_tasks[id]);

    res->set_task_state(state);
    return Status::OK;
}

int main()
{
    /* -- The server starts in the constructor -- */
    AraxServer server("localhost:50051");

    /* -- Server shutdown in the destructor -- */

    return 0;
}
