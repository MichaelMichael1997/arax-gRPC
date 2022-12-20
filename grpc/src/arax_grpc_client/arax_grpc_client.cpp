#include "arax_grpc_client.h"
#include <cstdint>

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::CreateChannel;
using grpc::InsecureChannelCredentials;
using grpc::Status;

using google::protobuf::Arena;
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

/* -- Max size for a protocol buffer message -- */
// #define MAX_MSG 1048576 // --> 1 MB
#define MAX_MSG 524288 // --> 0.5 MB

/*
 * Constructors
 */
AraxClient::AraxClient(const char *addr)
{
    stub_ = Arax::NewStub(CreateChannel(addr, InsecureChannelCredentials()));
}

/*
 * Destructors
 */
AraxClient::~AraxClient(){ }

// -------------------- Arax Client Services --------------------

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
}

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
}

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
}

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
}

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
}

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
}

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
}

/*
 * Set data to buffer
 *
 * @param buffer The ID of the buffer
 * @param accel The ID of the accelerator
 * @param data Byte sequence of the serialized data
 *
 * @return nothing
 */
void AraxClient::client_arax_data_set(uint64_t buffer, uint64_t accel, std::string data)
{
    /*
     * Google suggests that protobuf messages over 1MB are not optimal.
     * If the serialized data is larger than that, then this calls the
     * client streaming version, which fragments the data and sends them sequentially.
     * If they are not, then proceed normally
     */
    size_t bytes = data.size();

    if (bytes > MAX_MSG) {
        large_data_set(buffer, accel, data);
        return;
    }

    DataSet req;
    Empty res;
    ClientContext ctx;

    /* -- Set a deadline (10s) -- */
    std::chrono::time_point<std::chrono::system_clock> deadline = std::chrono::system_clock::now()
      + std::chrono::milliseconds(10000);

    ctx.set_deadline(deadline);

    req.set_buffer(buffer);
    req.set_accel(accel);
    req.set_data(data.data(), bytes);
    req.set_data_size(bytes);

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
 *
 * @return The serialized data or NULL on failure
 */
std::string AraxClient::client_arax_data_get(uint64_t buffer)
{
    ClientContext ctx;
    ResourceID req;
    DataSet res;

    /* -- Set deadline -- */
    std::chrono::time_point<std::chrono::system_clock> deadline = std::chrono::system_clock::now()
      + std::chrono::milliseconds(10000); // -> 10s

    ctx.set_deadline(deadline);

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
        return std::string("");
    }

    return res.data();
} // AraxClient::client_arax_data_get

/*
 * Similar to client_arax_data_get
 * This one should be used when anticipating large data
 *
 * @param the ID of the buffer holding the data
 *
 * @return The serialized data or an empty string on failure
 */
std::string AraxClient::client_arax_large_data_get(uint64_t buffer)
{
    ClientContext ctx;
    ResourceID req;

    req.set_id(buffer);

    /* -- TODO: Set deadline here -- */

    std::unique_ptr<ClientReader<DataSet> > reader(stub_->Arax_large_data_get(&ctx, req));
    std::string data("");
    DataSet d;

    /* -- Combine the segments -- */
    while (reader->Read(&d)) {
        data += d.data();
    }

    #ifdef DEBUG
    assert(original_size == data.size());
    #endif

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
        return std::string("");
    }

    return data;
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
}

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
}

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
 * @return The ID of the new task of 0 on failure
 */
uint64_t AraxClient::client_arax_task_issue(uint64_t accel, uint64_t proc, char *host_init, size_t host_size,
  size_t in_count, uint64_t *in_buffer, size_t out_count, uint64_t *out_buffer)
{
    TaskRequest req;
    ResourceID res;
    ClientContext ctx;

    if (in_buffer == 0 || out_buffer == 0) {
        fprintf(stderr, "-- Pass valid arrays of Buffer identifiers --\n");
        return 0;
    }

    req.set_accel(accel);
    req.set_proc(proc);
    req.set_in_count(in_count);
    req.set_out_count(out_count);
    /* -- Throws error when initilizing bytes with null -- */
    if (host_init == 0) {
        req.set_host_init("");

        /* -- Check for invalid input -- */
        if (host_size != 0) {
            fprintf(stderr, "-- Host data are NULL, but host size != 0 --\n");
            return 0;
        }
    } else {
        req.set_host_init(host_init);
        /* -- Check for invalid input -- */
        if (host_size == 0) {
            fprintf(stderr, "-- Host data not NULL, but host size == 0 --\n");
            return 0;
        }
    }
    req.set_host_size(host_size);

    /* pass the in buffers */
    for (size_t i = 0; i < in_count; i++) {
        req.add_in_buffer(*(in_buffer + i));
    }

    /* pass the out buffers */
    for (size_t i = 0; i < out_count; i++) {
        req.add_out_buffer(*(out_buffer + i));
    }

    for (const auto& i : req.in_buffer()) {
        std::cout << i << " ";
    }
    std::cout << '\n';

    for (const auto& i : req.out_buffer()) {
        std::cout << i << " ";
    }
    std::cout << '\n';

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

    std::cout << "-- Task was freed successfully\n";
    return;
}

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
}

/*
 * Function to fragment the data into messages
 * less than 1MB in size.
 * Uses client streaming to send them to the server
 *
 * @param buffer The buffer to assign the data
 * @param accel  The accelerator identifier
 * @param data   The data
 */
void AraxClient::large_data_set(uint64_t buffer, uint64_t accel, std::string data)
{
    ClientContext ctx;
    Empty res;
    size_t size = data.size();

    /* -- Set a deadline  relative to the size of input in kilobytes -- */
    std::chrono::time_point<std::chrono::system_clock> deadline = std::chrono::system_clock::now()
      + std::chrono::milliseconds(5000 + (size >> 10)); // --> 5 seconds plus extra for larger data

    ctx.set_deadline(deadline);

    /* -- Get the number of chunks, for debugging purposes -- */
    unsigned int chunks_num = ceil(float(size) / float(MAX_MSG));

    std::cout << "-- Number of chunks to send: " << chunks_num << "--\n";
    std::cout << "-- Data size in megabytes: " << (size >> 20) << "\n";

    /* -- write to stream for server -- */
    std::unique_ptr<ClientWriter<DataSet> > writer(stub_->Arax_data_set_streaming(&ctx, &res));

    /* -- Split the data into chunks of 1 MAX_MSG each-- */
    long int remaining = size;
    size_t it      = 0;
    int iterations = 0;

    fprintf(stderr, "It %zu, Current sent %zu, Remaining %ld Iterations %d\n", it, it, remaining, iterations);
    while (it < size) {
        std::string chunk;
        /* -- Less than 1 MAX_MSG remains -- */
        if (remaining < MAX_MSG) {
            chunk     = data.substr(it, remaining);
            it       += remaining;
            remaining = 0; /* -- no more to send -- */
        } else {
            chunk      = data.substr(it, MAX_MSG);
            it        += MAX_MSG;
            remaining -= MAX_MSG;
        }

        DataSet d;
        d.set_data(chunk);
        d.set_data_size(data.size()); // --> The original data size
        d.set_buffer(buffer);
        d.set_accel(accel);

        if (!writer->Write(d)) {
            std::cerr << "-- Stream broke\n";
            break;
        }

        iterations += 1;
        fprintf(stderr, "It %zu, Current sent %zu, Remaining %ld Iterations %d\n", it, it, remaining, iterations);
    }

    // std::cerr << "Loop iterations " << iterations << "\n";

    writer->WritesDone();

    fprintf(stderr, "Finished streaming!\n");

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
}

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
}
