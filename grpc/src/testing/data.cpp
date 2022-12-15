#include "../arax_grpc_client/arax_grpc_client.h"
#include <string.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

#define MAGIC 10

using namespace arax;

typedef const uint64_t Task;
typedef const uint64_t Buffer;
typedef const uint64_t Proc;
typedef const uint64_t Accel;


/* -- Serialize integer -- */
std::string serialize_int(const int num)
{
    std::ostringstream os;

    os << num;

    return os.str();
}

/* -- Deseiralize integer -- */
int deserialize_int(std::string bytes)
{
    std::istringstream is(bytes);
    int num = 0;

    is >> num;

    return num;
}

/* -- return number squared -- */
int something_op(const int number)
{
    return number * number;
}

#ifdef BUILD_MAIN
int main(int argc, char *argv[])
{
    AraxClient client("localhost:50051");

    /* -- Set the data  -- */
    int out   = 0;
    int tmp   = MAGIC;
    int magic = MAGIC;

    /* -- Serialize them, get size and request buffers accordingly -- */
    std::string in_data = serialize_int(tmp);

    /* -- Request buffer -- */
    Buffer input  = client.client_arax_buffer(sizeof(in_data));
    Buffer output = client.client_arax_buffer(sizeof(in_data));
    /* -- Request accelerator -- */
    Accel accel = client.client_arax_accel_acquire_type(CPU);

    /* -- Get registered process -- */
    Proc proc = client.client_arax_proc_get("something");

    /* -- Failed to retrieve registered process -- */
    if (proc == 0) {
        exit(EXIT_FAILURE);
    }

    client.client_arax_data_set(input, accel, in_data);

    // /* -- Issue task -- */
    Task task = client.client_arax_task_issue(accel, proc, magic, 4, 1, input, 1, output);

    int task_state = client.client_arax_task_wait(task);

    fprintf(stdout, "\n======================\n");
    fprintf(stdout, "-- -1: gRPC_failed\n--  0: task_failed\n--  1: task_issued\n--  2: task_completed\n");
    fprintf(stdout, "======================\n\n");
    fprintf(stdout, "Task state returned by client_arax_task_wait: %d\n", task_state);

    if (task_state == 0 != task_state == -1) { /* -- task failed -- */
        fprintf(stderr, "Task failed\n");
        client.client_arax_data_free(input);
        client.client_arax_data_free(output);
        client.client_arax_task_free(task);
        client.client_arax_proc_put(proc);
        client.client_arax_accel_release(accel);

        exit(EXIT_FAILURE);
    }

    /* -- Get the data from the buffer after they are processed -- */
    std::string data = client.client_arax_data_get(output);

    if (data.empty()) { /* -- failed to retrieve data from buffer -- */
        std::cout << "-- Failed to retrieve data from buffer --\n";
        client.client_arax_data_free(input);
        client.client_arax_data_free(output);
        client.client_arax_task_free(task);
        client.client_arax_proc_put(proc);
        client.client_arax_accel_release(accel);
        exit(EXIT_FAILURE);
    }

    /* -- Deserialize to get the integer value -- */
    int output_number = deserialize_int(data);

    /* -- Do the operation on the original to compare -- */
    tmp = something_op(tmp);

    fprintf(stdout, "Data received : %d\n", output_number);
    fprintf(stdout, "Should be: %d\n", tmp);

    /* -- Free the resources -- */
    client.client_arax_data_free(input);
    client.client_arax_data_free(output);
    client.client_arax_task_free(task);
    client.client_arax_proc_put(proc);
    client.client_arax_accel_release(accel);

    return 0;
} // main

#endif /* -- BUILD_MAIN -- */

#ifdef BUILD_SO

#include <core/arax_data_private.h>
#include <AraxLibUtilsCPU.h>

arax_task_state_e something(arax_task_msg_s *msg)
{
    /* -- Add Magic checking here -- */

    char *in  = (char *) arax_data_deref(msg->io[0]);
    char *out = (char *) arax_data_deref(msg->io[1]);
    int magic = *(int *) arax_task_host_data(msg, 4);

    if (magic != MAGIC) {
        throw std::runtime_error("Magic numbers don't match!\n");
    }

    /* -- Deserialize input -- */
    int input = deserialize_int(std::string(in)); // --> This is ugly

    input = something_op(input);

    /* -- Copy result to output -- */
    std::string output = serialize_int(input);

    strcpy(out, output.c_str());

    arax_task_mark_done(msg, task_completed);
    return task_completed;
}

ARAX_PROC_LIST_START()
ARAX_PROCEDURE("something", CPU, something, 0)
ARAX_PROCEDURE("something", GPU, something, 0)
ARAX_PROC_LIST_END()
#endif /* -- BUILD_SO -- */
