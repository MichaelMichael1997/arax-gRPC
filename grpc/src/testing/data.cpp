#include "../arax_grpc_client/arax_grpc_client.h"

#include <string.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using namespace arax;

typedef const uint64_t Task;
typedef const uint64_t Buffer;
typedef const uint64_t Proc;
typedef const uint64_t Accel;

void something_op(char *str)
{
    for (int i = 0; i < strlen(str); i++) {
        char tmp = '-';
        i % 2 == 0 ? tmp = '-' : tmp = '_';
        *(str + i)       = tmp;
    }
}

int main(int argc, char *argv[])
{
    char *test = (char *) malloc(sizeof(char) * 5);

    strcpy(test, "test");

    AraxClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    /* -- Request buffer -- */
    Buffer input  = client.client_arax_buffer(strlen(test) + 1);
    Buffer output = client.client_arax_buffer(strlen(test) + 1);

    /* -- Get registered process -- */
    Proc proc = client.client_arax_proc_get("noop");

    if (proc == 0) {
        free(test);
        exit(1);
    }

    /* -- Request accelerator -- */
    Accel accel = client.client_arax_accel_acquire_type(CPU);

    /* -- Set the data -- */
    client.client_arax_data_set(input, accel, test);

    /* -- Issue task -- */
    Task task = client.client_arax_task_issue(accel, proc, 1, input, 1, output);

    int task_state = client.client_arax_task_wait(task);

    fprintf(stdout, "-- Task state: %d\n", task_state);

    /* -- Get the data from the buffer after they are processed -- */
    const char *data = client.client_arax_data_get(output);

    fprintf(stdout, "Data received from client_arax_buffer_get : %s\n", data);

    /* -- Free the resources -- */
    client.client_arax_data_free(input);
    client.client_arax_data_free(output);
    client.client_arax_task_free(task);
    client.client_arax_proc_put(proc);
    client.client_arax_proc_put(accel);

    free(test);

    return 0;
} // main
