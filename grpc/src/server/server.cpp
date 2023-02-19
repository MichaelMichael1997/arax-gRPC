#include "server.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerWriter;
using grpc::ServerReaderWriter;
using grpc::Status;
using grpc::StatusCode;

using namespace arax;

#ifdef __linux__
#define ERROR_COL   "\033[1;38;5;9;1m"
#define SUCCESS_COL "\033[1;37;38;5;10m"
#define RESET_COL   "\033[0m"
#endif /* #ifdef __linux__ */

constexpr long int MAX_PAYLOAD = 524288;

/*
 * Constructor to start the server and init arax
 *
 * @param addr The address to connect
 */
AraxServer::AraxServer(const char *addr)
{
    /*----- initialize arax -----*/
    std::cout << "-- Initializing Arax --\n";
    try
    {
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
    catch (std::runtime_error &e)
    {
        std::cerr << e.what();
        exit(EXIT_FAILURE);
    }

    unique_id = 1;
    ServerBuilder builder;

    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(this);
    server = std::unique_ptr<Server>(builder.BuildAndStart());

    /* -- Start the server -- */
    server_thread = std::thread([this](){
        this->start_server();
    });
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
    server_thread.join();
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
        std::string error_msg("-- Arax failed to delete the shared segment (in arax clean)--");
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
    if (!insert_pair(buffers, id, buffer)) {
        std::string error("-- A buffer with ID '" + std::to_string(id) + "' already exists (in arax buffer)--");
        return Status(StatusCode::INVALID_ARGUMENT, error);
    }

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
        std::string error_msg("-- Arax failed to register the process with name '" + func_name
          + "'(in proc register)--");
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
          + "'. Maybe it has not been registered, or arax failed to retrieve it (in proc get)--");
        return Status(StatusCode::INVALID_ARGUMENT, error);
    }

    // Check if the retrieved process is already in the mapping
    for (auto i : arax_processes) {
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
        std::string error_msg("-- No process with name '" + std::to_string(id) + "' exists (in proc put)--");
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
        std::string error_msg("-- Failed to acquire accelerator of given type (in accel acquire type)--");
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
        std::string error_msg("-- Accelerator with ID '" + std::to_string(id)
          + "' does not exist (in accel release)--");
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
        std::string error_msg("-- No buffer/data_s exists with ID'" + std::to_string(buffer) + "' (in data set)--");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    if (!check_if_exists(arax_accels, accel)) {
        std::string error_msg("-- No accelerator with ID'" + std::to_string(accel) + "' exists (in data set)--");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    /* -- Pass the data as a string to arax, do the rest in the kernel -- */
    arax_data_set(buffers[buffer], arax_accels[accel], req->data().data());
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

    /* Check if buffer/data_s with that ID exists */
    if (!check_if_exists(buffers, id)) {
        std::string error_msg("-- No buffer with ID '" + std::to_string(id) + "' exists (in data get)--");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    size_t size = arax_data_size(buffers[id]);
    void *mem   = malloc(size);

    arax_data_get(buffers[id], mem);

    res->set_data(mem, size);
    res->set_data_size(size);

    if (ctx->IsCancelled()) {
        std::string error_msg("-- Deadline exceeded, or Client cancelled. Abandoning (in data get)--");
        return Status(StatusCode::CANCELLED, error_msg);
    }

    return Status::OK;
} // AraxServer::Arax_data_get

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
Status AraxServer::Arax_large_data_get(ServerContext *ctx, const ResourceID *req, ServerWriter<DataSet> *writer)
{
    #ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(writer);
    #endif

    uint64_t id = req->id();

    /* Check if buffer with that ID exists */
    if (!check_if_exists(buffers, id)) {
        std::string error_msg("-- No buffer with ID '" + std::to_string(id) + "' exists (in data get)--");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    size_t size = arax_data_size(buffers[id]);
    // std::cerr << "Size in large data get: " << size << '\n';

    // Alocate memory for the data to be copied
    void *data = malloc(size);

    #ifdef DEBUG
    assert(data);
    #endif

    if (!data) {
        std::string error("-- The system failed to allocate memory (in data get)--");
        return Status(StatusCode::INTERNAL, error);
    }

    /* -- Get the data from the buffer -- */
    arax_data_get(buffers[id], data);

    if (!data) {
        std::string error_msg("-- Failed to fetch the data from the buffer (in data get)--\n");
        return Status(StatusCode::INTERNAL, error_msg);
    }

    if (ctx->IsCancelled()) {
        std::string error_msg("-- Deadline exceeded, or Client cancelled. Abandoning (in data get)--");
        return Status(StatusCode::CANCELLED, error_msg);
    }

    DataSet original;
    original.set_data(data, size);
    DataSet chunk;

    /* -- Split the data into chunks of 1 MAX_PAYLOAD each-- */
    long int remaining = size;
    size_t it = 0;
    // int iterations     = 0;

    // fprintf(stderr, "It %zu, Current sent %zu, Remaining %ld Iterations %d\n", it, it, remaining, iterations);
    while (it < size) {
        /* -- Less than 1 MAX_PAYLOAD remains -- */
        if (remaining < MAX_PAYLOAD) {
            chunk.set_data(original.data().substr(it, remaining));
            it       += remaining;
            remaining = 0; /* -- no more to send -- */
        } else {
            chunk.set_data(original.data().substr(it, MAX_PAYLOAD));
            it        += MAX_PAYLOAD;
            remaining -= MAX_PAYLOAD;
        }

        DataSet d;
        d.set_data(chunk.data());
        d.set_data_size(size); // --> The original data size

        if (!writer->Write(d, grpc::WriteOptions().set_buffer_hint())) {
            std::cerr << "-- Stream broke\n";
            std::string error_msg("-- Stream broke (in data get)--");
            return Status(StatusCode::DATA_LOSS, error_msg);
        }

        // iterations += 1;
        // fprintf(stderr, "It %zu, Current sent %zu, Remaining %ld Iterations %d\n", it, it, remaining, iterations);
    }

    // fprintf(stderr, "Total iterations %d\n", iterations);

    return Status::OK;
} // AraxServer::Arax_large_data_get

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
        std::string error("-- Buffer with ID '" + std::to_string(id) + "' does not exist (in data size)--");
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
        std::string error("-- No buffer with ID '" + std::to_string(id) + "' exists (in data free)--");
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

    uint64_t accel   = req->accel();
    uint64_t proc    = req->proc();
    size_t in_count  = req->in_count();
    size_t out_count = req->out_count();
    void *host_init  = (void *) req->host_init().data();
    size_t host_size = req->host_size();

    /* -- Fix the buffer i/o arrays -- */
    arax_buffer_s in_buffer[in_count];
    arax_buffer_s out_buffer[out_count];

    uint64_t *in  = (uint64_t *) req->in_buffer().data();
    uint64_t *out = (uint64_t *) req->out_buffer().data();

    for (auto i = 0; i < in_count; i++) {
        in_buffer[i] = buffers[in[i]];
    }

    for (auto i = 0; i < out_count; i++) {
        out_buffer[i] = buffers[out[i]];
    }

    arax_accel *exec   = arax_accels[accel];
    arax_proc *process = arax_processes[proc];

    uint64_t id = get_unique_id();

    arax_task *task = NULL;
    task = arax_task_issue(exec, process, host_init, host_size, in_count, in_buffer, out_count, out_buffer);

    insert_pair(arax_tasks, id, task);

    if (task == NULL) {
        std::string error_msg("-- Failed to issue task (in arax task issue) --");
        return Status(StatusCode::ABORTED, error_msg);
    }

    res->set_id(id);

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
        std::string error_msg("-- Task with ID '" + std::to_string(task) + "' does not exist (in task free)--");
        return Status(StatusCode::FAILED_PRECONDITION, error_msg);
    }

    arax_task_free(arax_tasks[task]);
    auto it = arax_tasks.find(task);
    arax_tasks.erase(it);

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
        std::string error_msg("-- There is no task registered with ID '" + std::to_string(id) + "' (in task wait)--");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    unsigned int state = arax_task_wait(arax_tasks[id]);

    res->set_task_state(state);
    return Status::OK;
}

/* -- gRPC methods the client should not be able to call directly -- */

/*
 * Function to receive large data from the client
 * via streaming
 *
 * @param ctx Server Context
 * @param reader ServerReader instance to read the incoming stream of data
 *               from the client
 * @param res Empty message
 */
Status AraxServer::Arax_data_set_streaming(ServerContext *ctx, ServerReader<DataSet> *reader, Empty *res)
{
    #ifdef DEBUG
    assert(ctx);
    assert(reader);
    assert(res);
    #endif

    std::string client_data("");
    DataSet data;

    /* -- Read the incoming data form the client -- */
    while (reader->Read(&data)) {
        client_data += data.data();
    }

    uint64_t buffer = data.buffer();
    uint64_t accel  = data.accel();

    if (!check_if_exists(buffers, buffer)) {
        std::string error_msg("-- No buffer exists with ID'" + std::to_string(buffer) + "' (in data set)--");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    if (!check_if_exists(arax_accels, accel)) {
        std::string error_msg("-- No accelerator with ID'" + std::to_string(accel) + "' exists (in data set)--");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }
    size_t data_size = data.data_size();
    size_t megabytes = data_size >> 20;

    fprintf(stderr, "Buffer %zu, Accel %zu, Data size %zu Data size in megabytes %zu\n", buffer, accel, data_size,
      megabytes);

    /* -- Check if all data arrived -- */
    if (data_size != client_data.size()) {
        std::string error_msg("-- Possible data loss (in data set)--");
        return Status(StatusCode::DATA_LOSS, error_msg);
    }


    arax_data_set(buffers[buffer], arax_accels[accel], client_data.c_str());

    return Status::OK;
} // AraxServer::Arax_data_set_streaming

/*
 * Initialize a new arax_data_s object
 *
 * @param  ctx Server context
 * @param  req Message with the requested size for the data
 * @param  res Message with the unique ID of the resource
 *
 * @return The appropriate status code
 */
Status AraxServer::Arax_data_init(ServerContext *ctx, const AraxData *req, ResourceID *res)
{
    #ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    #endif

    size_t size       = req->size();
    arax_data_s *data = arax_data_init(pipe_s, size);

    if (!data) {
        std::string error_msg("-- Failed to initialize arax_data_s object (in data init)--");
        res->set_id(0);
        return Status(StatusCode::INTERNAL, error_msg);
    }

    arax_buffer_s buffer = (arax_buffer_s *) data;

    res->set_id(get_unique_id());
    insert_pair(buffers, res->id(), buffer);

    return Status::OK;
}

/*
 * Initialize a new arax_data_s object with an aligned buffer
 *
 * @param  ctx ServerContext
 * @param  req Message with the requested size and allignment
 * @param  res Message with the ID of the resource
 *
 * @return The appropriate status code
 */
Status AraxServer::Arax_data_init_aligned(ServerContext *ctx, const AraxData *req, ResourceID *res)
{
    #ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    #endif

    size_t size        = req->size();
    size_t alignment   = req->alligned();
    arax_buffer_s data = (arax_buffer_s *) arax_data_init_aligned(pipe_s, size, alignment);

    res->set_id(get_unique_id());
    insert_pair(buffers, res->id(), data);

    return Status::OK;
}

/*
 * Initialize data remote (accelerator) buffer
 *
 * @param ctx Server Context
 * @param req DataSet message with the buffer and accelerator identifiers
 * @param res Empty message
 *
 * @return The appropriate status code
 */
Status AraxServer::Arax_data_allocate_remote(ServerContext *ctx, const DataSet *req, Empty *res)
{
    #ifdef DEBUG
    assert(ctx);
    assert(req);
    assert(res);
    #endif

    uint64_t buffer = req->buffer();
    uint64_t accel  = req->accel();

    if (!check_if_exists(buffers, buffer)) {
        std::string error_msg("-- No buffer exists with ID'" + std::to_string(buffer)
          + "' (in data allocate remote)--");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    if (!check_if_exists(arax_accels, accel)) {
        std::string error_msg("-- No accelerator with ID'" + std::to_string(accel)
          + "' exists (in data allocate remote)--");
        return Status(StatusCode::INVALID_ARGUMENT, error_msg);
    }

    arax_buffer_s buffer_s  = buffers[buffer];
    arax_accel *accelerator = arax_accels[accel];
    arax_data_allocate_remote((arax_data_s *) buffer_s, accelerator);

    return Status::OK;
}

Status AraxServer::Arax_task_issue_streaming(ServerContext *ctx, ServerReaderWriter<ResourceID, TaskRequest> *stream)
{
    #ifdef DEBUG
    assert(ctx);
    assert(stream);
    #endif

    TaskRequest req;
    ResourceID res;

    if (!stream->Read(&req)) {
        std::string err_message("Stream broke");
        return Status(StatusCode::DATA_LOSS, err_message);
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
        return Status(StatusCode::DATA_LOSS, err_message);
    }

    while (stream->Read(&req)) {
        host_init = (void *) req.host_init().data();
        task      = arax_task_issue(exec, process, host_init, host_size, in_count, in_buffer, out_count, out_buffer);

        if(task == NULL){
          std::string err_message("-- Task failed (in Arax_task_issue_streaming)");
          return Status(StatusCode::ABORTED, err_message);
        }
        id        = get_unique_id();
        insert_pair(arax_tasks, id, task);

        res.set_id(id);

        if (!stream->Write(res)) {
            std::string err_message("Stream broke");
            return Status(StatusCode::DATA_LOSS, err_message);
        }
    }

    return Status::OK;
} // AraxServer::Arax_task_issue_streaming

int main()
{
    /* -- The server starts in the constructor -- */
    AraxServer server("localhost:50051");

    fprintf(stderr, "-- Type \'exit\' to shutdown the server\n");
    std::string input("");

    while (std::cin >> input) {
        if (input == "exit") {
            server.shutdown_server();
            break;
        }
    }

    /* -- Arax exit in the destructor -- */

    return 0;
}
