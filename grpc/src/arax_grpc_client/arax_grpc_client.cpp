#include "arax_grpc_client.h"
#include <cstdint>
#include <grpcpp/security/credentials.h>

using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientWriter;
using grpc::CreateChannel;
using grpc::InsecureChannelCredentials;
using grpc::Status;

using namespace arax;

/*
 * Add some colors for the output
 */
#ifdef __linux__
#define ERROR_COL   "\033[1;38;5;9;1m"
#define SUCCESS_COL "\033[1;37;38;5;10m"
#define RESET_COL   "\033[0m"
#endif /* #ifdef __linux__ */

constexpr long int MAX_MSG = 524288;  
constexpr long int CHANNEL_POOL = 10;  

/*
 * Constructors
 */
AraxClient::AraxClient(const char *addr)
{
  grpc::ChannelArguments args;
  args.SetInt(GRPC_ARG_MAX_CONCURRENT_STREAMS, 200);
  main_channel = grpc::CreateCustomChannel(addr, InsecureChannelCredentials(), args);
  stub_ = Arax::NewStub(main_channel);
}

/*
 * Destructors
 */
AraxClient::~AraxClient(){ }

// -------------------- Arax Client Services --------------------

void AraxClient::set_reader_writer(){
  this->stream = stub_->Arax_task_issue_streaming(&task_ctx);
}

void AraxClient::terminate_task_issue_streaming(){
  stream->WritesDone();
  Status status = stream->Finish();

  if (!status.ok()) {
    #ifdef __linux__
    std::stringstream ss;
    ss << ERROR_COL;
    ss << "\nERROR: " << status.error_code() << "\n";
    ss << status.error_message() << "\n";
    ss << status.error_details() << "\n\n";
    ss << RESET_COL;
    std::cerr << ss.str();
    #else
    std::cerr << "\nERROR: " << status.error_code() << "\n";
    std::cerr << status.error_message() << "\n";
    std::cerr << status.error_details() << "\n\n";
    #endif /* ifdef __linux__ */

    return;
  }
}

uint64_t AraxClient::client_arax_task_issue_streaming(uint64_t accel, uint64_t proc, void *host_init, size_t host_size,
      size_t in_count,
      uint64_t *in_buffer,
      size_t out_count, uint64_t *out_buffer
){
  ResourceID res;
  TaskRequest req;

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
void AraxClient::client_arax_clean()
{
    ClientContext ctx;
    Empty req;
    Empty res;


    Status status = stub_->Arax_clean(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */

        return;
    }

    std::cout << "-- Arax was cleaned up successfully\n";

    return;
} // AraxClient::client_arax_clean

/*
 * Create an arax_buffer_s object
 *
 * @param size The desired size for the buffer
 *
 * @return The ID of the newly allocated buffer, or 0 on failure
 */
uint64_t AraxClient::client_arax_buffer(size_t size)
{
    ClientContext ctx;
    RequestBuffer req;

    req.set_buffer_size(size);
    ResourceID res;

    Status status = stub_->Arax_buffer(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */
        return res.id();
    }

    return res.id();
} // AraxClient::client_arax_buffer

/*
 * Register a new process 'func_name'
 *
 * @param func_name The name of the process
 *
 * @return The id of the arax_proc resource, or 0 on failure
 */
uint64_t AraxClient::client_arax_proc_register(const char *func_name)
{
    ClientContext ctx;
    ProcRequest req;

    req.set_func_name(func_name);
    ResourceID res;

    Status status = stub_->Arax_proc_register(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */
        return res.id();
    }

    return res.id();
} // AraxClient::client_arax_proc_register

/*
 * Retrieve a previously registered arax_process
 *
 * \Note Calls to client_arax_proc_register(..), client_arax_proc_get(..),
 * should have matching calls to arax_proc_put(..)
 *
 * @param func_name The process func name
 *
 * @return The ID of the resource or 0 on failure
 */
uint64_t AraxClient::client_arax_proc_get(const char *func_name)
{
    ClientContext ctx;
    ProcRequest req;
    ResourceID res;

    req.set_func_name(func_name);

    Status status = stub_->Arax_proc_get(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */
        return 0;
    }

    fprintf(stdout, "-- Process \'%s\' was acquired successfully\n", func_name);

    return res.id();
} // AraxClient::client_arax_proc_get

/*
 * Delete registered arax proc pointer
 *
 * @param proc The ID of the arax_proc
 *
 * @return nothing
 */
void AraxClient::client_arax_proc_put(uint64_t id)
{
    ClientContext ctx;
    AraxProc req;

    req.set_id(id);
    ProcCounter res;

    Status status = stub_->Arax_proc_put(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */
        return;
    }

    return;
} // AraxClient::client_arax_proc_put

/*
 * Acquire a virtual accelerator of the given type
 *
 * @param type The type of the accelerator
 *
 * @return The id of the acquired resource or 0 on failure
 */
uint64_t AraxClient::client_arax_accel_acquire_type(unsigned int type)
{
    AccelRequest req;

    req.set_type(type);
    ResourceID res;
    ClientContext ctx;

    Status status = stub_->Arax_accel_acquire_type(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */
        return 0;
    }

    std::cout << "-- Accelerator was acquired successfully\n";
    return res.id();
} // AraxClient::client_arax_accel_acquire_type

/*
 * Release previously acquired accelerator
 *
 * @param id The id of the accelerator
 *
 * @return nothing
 */
void AraxClient::client_arax_accel_release(uint64_t id)
{
    ClientContext ctx;
    ResourceID req;
    Empty res;

    req.set_id(id);

    Status status = stub_->Arax_accel_release(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */
        return;
    }

    std::cout << "-- Accelerator was released successfully\n";

    return;
} // AraxClient::client_arax_accel_release

/*
 * Set data to buffer
 *
 * @param buffer The ID of the buffer
 * @param accel The ID of the accelerator
 * @param data Byte sequence of the serialized data
 *
 * @return nothing
 */
void AraxClient::client_arax_data_set(uint64_t buffer, uint64_t accel, void *data, size_t size)
{
    #ifdef DEBUG
    assert(data);
    #endif

    /*
     * Google suggests that protobuf messages over 1MB are not optimal.
     * If the serialized data is larger than that, then this calls the
     * client streaming version, which fragments the data and sends them sequentially.
     * If they are not, then proceed normally
     */
    if (size > MAX_MSG) {
        large_data_set(buffer, accel, data, size);
        return;
    }

    DataSet req;
    Empty res;
    ClientContext ctx;

    if (!data) {
        fprintf(stderr, "-- Invalid data entered\n");
        return;
    }

    /* -- Plus 1 for the null terminating character -- */
    req.set_buffer(buffer);
    req.set_accel(accel);
    req.set_data_size(size);
    req.set_data(data, size);

    Status status = stub_->Arax_data_set(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */
        return;
    }

    std::cout << "-- Data was set successfully\n";
    return;
} // AraxClient::client_arax_data_set

/*
 * Return the data that was set to an arax buffer
 *
 * @param buffer The ID of the buffer
 * @param user Memory at least data_size long
 *
 * @return nothing
 */
void AraxClient::client_arax_data_get(uint64_t buffer, void *user, size_t size)
{
    ClientContext ctx;
    ResourceID req;
    DataSet res;

    if (size > MAX_MSG) {
        client_arax_large_data_get(buffer, user, size);
        return;
    }

    req.set_id(buffer);

    Status status = stub_->Arax_data_get(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */
        return;
    }

    memcpy(user, res.data().data(), size);

    return;
} // AraxClient::client_arax_data_get

/*
 * Similar to client_arax_data_get
 * This one should be used when anticipating large data
 *
 * @param the ID of the buffer holding the data
 *
 * @return The serialized data or an empty string on failure
 */
void AraxClient::client_arax_large_data_get(uint64_t buffer, void *user, size_t size)
{
    ClientContext ctx;
    ResourceID req;

    req.set_id(buffer);

    std::unique_ptr<ClientReader<DataSet> > reader(stub_->Arax_large_data_get(&ctx, req));
    std::string data("");
    DataSet d;

    /* -- Combine the segments -- */
    while (reader->Read(&d)) {
        data += d.data();
    }

    Status status = reader->Finish();

    if (!status.ok()) {
        #ifdef __linux
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */
        return;
    }

    memcpy(user, data.data(), size);

    return;
} // AraxClient::client_arax_large_data_get

/*
 * Return the size of the data of specified arax_data
 *
 * @param id The ID of the arax_buffer
 *
 * @return The size of the data
 */
size_t AraxClient::client_arax_data_size(uint64_t id)
{
    ClientContext ctx;
    ResourceID req;
    DataSet res;

    req.set_id(id);

    Status status = stub_->Arax_data_size(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */

        return 0;
    }

    return res.data_size();
} // AraxClient::client_arax_data_size

/*
 * Mark data for deletion
 *
 * @param id The id of the buffer
 *
 * @return nothing
 */
void AraxClient::client_arax_data_free(uint64_t id)
{
    ClientContext ctx;
    ResourceID req;
    Empty res;

    req.set_id(id);

    Status status = stub_->Arax_data_free(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */

        return;
    }

    return;
} // AraxClient::client_arax_data_free

/*
 * Issue a new task
 *
 * @param accel The ID of the accelerator responsible for executing the task
 * @param proc       ID of the arax_proc to be dispatched on accelerator
 * @param in_count   Size of input array (elements)
 * @param in_buffer  Input buffer
 * @param out_count  Size of output array (elements)
 * @param out_buffer Output buffer
 *
 * @return The ID of the new task of 0 on failure
 */
uint64_t AraxClient::client_arax_task_issue(uint64_t accel, uint64_t proc, void *host_init, size_t host_size,
  size_t in_count, uint64_t *in_buffer, size_t out_count, uint64_t *out_buffer)
{
    TaskRequest req;
    ResourceID res;
    ClientContext ctx;

    req.set_accel(accel);
    req.set_proc(proc);
    req.set_in_count(in_count);
    req.set_out_count(out_count);
    req.set_host_init(host_init, host_size);
    req.set_host_size(host_size);
    req.set_in_buffer(in_buffer, in_count * sizeof(uint64_t));
    req.set_out_buffer(out_buffer, out_count * sizeof(uint64_t));

    Status status = stub_->Arax_task_issue(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
        #endif // ifdef __linux__
        return 0;
    }

    return res.id();
} // AraxClient::client_arax_task_issue

/*
 * Decrease ref counter of task
 *
 * @param task The ID of the task
 *
 * @return nothing
 */
void AraxClient::client_arax_task_free(uint64_t task)
{
    ClientContext ctx;
    TaskMessage req;
    Empty res;

    req.set_task_id(task);

    Status status = stub_->Arax_task_free(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */
        return;
    }

    return;
} // AraxClient::client_arax_task_free

/*
 * Wait for an issued task to complete or fail
 *
 * @param id The ID of the task
 *
 * @return The state of the task or -1 on error
 */
int AraxClient::client_arax_task_wait(uint64_t task)
{
    ClientContext ctx;
    TaskMessage req;
    TaskMessage res;

    req.set_task_id(task);
    Status status = stub_->Arax_task_wait(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cout << "\nERROR: " << status.error_code() << "\n";
        std::cout << status.error_message() << "\n";
        std::cout << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */
        return -1;
    }

    return res.task_state();
} // AraxClient::client_arax_task_wait

/*
 * Function to fragment the data into messages
 * less than 1MB in size.
 * Uses client streaming to send them to the server
 *
 * @param buffer The buffer to assign the data
 * @param accel  The accelerator identifier
 * @param data   The data
 */
void AraxClient::large_data_set(uint64_t buffer, uint64_t accel, void *data, size_t size)
{
    ClientContext ctx;
    Empty res;

    DataSet original;

    original.set_data(data, size);
    DataSet chunk;

    // /* -- write to stream for server -- */
    std::unique_ptr<ClientWriter<DataSet> > writer(stub_->Arax_data_set_streaming(&ctx, &res));

    // /* -- Split the data into chunks of 1 MAX_MSG each-- */
    long int remaining = size;
    size_t it      = 0;
    // int iterations = 0;

    // fprintf(stderr, "It %zu, Current sent %zu, Remaining %ld Iterations %d\n", it, it, remaining, iterations);
    while (it < size) {
        /* -- Less than 1 MAX_MSG remains -- */
        if (remaining < MAX_MSG) {
            chunk.set_data(original.data().substr(it, remaining));
            it       += remaining;
            remaining = 0; /* -- no more to send -- */
        } else {
            chunk.set_data(original.data().substr(it, MAX_MSG));
            it        += MAX_MSG;
            remaining -= MAX_MSG;
        }

        DataSet d;
        d.set_data(chunk.data());
        d.set_data_size(size); // --> The original data size
        d.set_buffer(buffer);
        d.set_accel(accel);

        if (!writer->Write(d, grpc::WriteOptions().set_buffer_hint())) {
            std::cerr << "-- Stream broke\n";
            break;
        }

        // iterations += 1;
        // fprintf(stderr, "It %zu, Current sent %zu, Remaining %ld Iterations %d\n", it, it, remaining, iterations);
    }

    // std::cerr << "Loop iterations " << iterations << "\n";

    writer->WritesDone();

    // fprintf(stderr, "Finished streaming!\n");

    Status status = writer->Finish();

    if (!status.ok()) {
        #ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cout << "\nERROR: " << status.error_code() << "\n";
        std::cout << status.error_message() << "\n";
        std::cout << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */
        return;
    }
} // AraxClient::large_data_set

/*
 * Initialize arax_data_s object
 *
 * @param  size The size for the object
 *
 * @return The ID of the resource or 0 on failure
 */
uint64_t AraxClient::client_arax_data_init(size_t size)
{
    ClientContext ctx;
    AraxData req;
    ResourceID res;

    req.set_size(size);

    Status status = stub_->Arax_data_init(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cout << "\nERROR: " << status.error_code() << "\n";
        std::cout << status.error_message() << "\n";
        std::cout << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */
        return 0;
    }

    return res.id();
} // AraxClient::client_arax_data_init

/*
 * Initialize arax_data_s object with aligned buffer
 *
 * @param  size  The size for the object
 * @param  align Alignment of buffer in bytes, power of two
 *
 * @return The ID of the resource or 0 on failure
 */
uint64_t AraxClient::client_arax_data_init_aligned(size_t size, size_t align)
{
    ClientContext ctx;
    AraxData req;
    ResourceID res;

    req.set_size(size);
    req.set_alligned(align);

    Status status = stub_->Arax_data_init_aligned(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cout << "\nERROR: " << status.error_code() << "\n";
        std::cout << status.error_message() << "\n";
        std::cout << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */
        return 0;
    }

    return res.id();
} // AraxClient::client_arax_data_init_aligned

/*
 * Initialize data remote (accelerator) buffer
 *
 * @param buffer The arax buffer
 * @param accel  The accelerator
 *
 * @return void
 */
void AraxClient::client_arax_data_allocate_remote(uint64_t buffer, uint64_t accel)
{
    DataSet req;
    Empty res;
    ClientContext ctx;

    req.set_buffer(buffer);
    req.set_accel(accel);

    Status status = stub_->Arax_data_allocate_remote(&ctx, req, &res);

    if (!status.ok()) {
        #ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n";
        ss << RESET_COL;
        std::cerr << ss.str();
        #else
        std::cout << "\nERROR: " << status.error_code() << "\n";
        std::cout << status.error_message() << "\n";
        std::cout << status.error_details() << "\n\n";
        #endif /* ifdef __linux__ */
        return;
    }
}
