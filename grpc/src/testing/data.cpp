#include "../arax_grpc_client/arax_grpc_client.h"
#include "../server/server.h"

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

    return;
}

int main(int argc, char *argv[])
{
    char *test = (char *) malloc(sizeof(char) * 5);

    strcpy(test, "test");

    something_op(test);
    AraxClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    // Request buffer
    Buffer input  = client.client_arax_buffer(strlen(test) + 1);
    Buffer output = client.client_arax_buffer(strlen(test) + 1);

    // Get registered process
    Proc proc = client.client_arax_proc_get("something");

    if (proc == 0) {
        free(test);
        exit(1);
    }

    // request accelerator
    Accel accel = client.client_arax_accel_acquire_type(CPU);

    // set the data
    client.client_arax_data_set(input, accel, test);

    Task task = client.client_arax_task_issue(accel, proc, 1, input, 1, output);

    client.client_arax_task_wait(task);

    const char *data = client.client_arax_data_get(output);

    something_op(test);

    fprintf(stdout, "Data received from client_arax_buffer_get (Should be %s): %s\n", test, data);

    client.client_arax_data_free(input);
    client.client_arax_data_free(output);
    client.client_arax_task_free(task);
    client.client_arax_proc_put(proc);
    client.client_arax_proc_put(accel);

    free(test);

    return 0;
} // main
