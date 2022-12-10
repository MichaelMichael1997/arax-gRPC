#include "arax_grpc_client.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

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
 * Constructors
 */
AraxClient::AraxClient(std::shared_ptr<Channel> channel) : stub_(Arax::NewStub(channel)){ }

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
 * @param value The string value to be passed to buffer
 * TODO: Change this depending on the possible values a buffer can get
 *
 * @return nothing
 */
void AraxClient::client_arax_data_set(uint64_t buffer, uint64_t accel, const char *value)
{
    DataSet req;
    Empty res;
    ClientContext ctx;

    req.set_buffer(buffer);
    req.set_accel(accel);
    req.set_str_val(value);

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
} // AraxClient::client_arax_set_data

/*
 * Return the data that was set to an arax buffer
 *
 * @param buffer The ID of the buffer
 *
 * @return The data
 */
const char * AraxClient::client_arax_data_get(uint64_t buffer)
{
    ClientContext ctx;
    ResourceID req;
    DataSet res;

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
        return "";
    }

    return res.str_val().c_str();
}

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
uint64_t AraxClient::client_arax_task_issue(uint64_t accel, uint64_t proc, size_t in_count, uint64_t in_buffer,
  size_t out_count, uint64_t out_buffer)
{
    TaskRequest req;
    ResourceID res;
    ClientContext ctx;

    req.set_accel(accel);
    req.set_proc(proc);
    req.set_in_buffer(in_buffer);
    req.set_out_buffer(out_buffer);
    req.set_in_count(in_count);
    req.set_out_count(out_count);

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

    std::cout << "-- Task was issued successfully!\n";
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
    Task req;
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
    Task req;
    Task res;

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
