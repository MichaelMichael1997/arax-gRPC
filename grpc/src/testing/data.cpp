#include "../arax_grpc_client/arax_grpc_client.h"

#include <string.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using namespace arax;

void something(char *str)
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

    something(test);

    // Arax init and exit are called by the constructors and destructors respectively
    AraxClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    // Request buffer
    const uint64_t input  = client.client_arax_buffer(strlen(test) + 1);
    const uint64_t output = client.client_arax_buffer(strlen(test) + 1);

    // Get registered process
    const uint64_t proc = client.client_arax_proc_get("something");

    if (proc == 0) {
        free(test);
        exit(1);
    }

    // request accelerator
    const uint64_t accel = client.client_arax_accel_acquire_type(CPU);

    client.client_arax_data_set(input, accel, test);

    const uint64_t task = client.client_arax_task_issue(accel, proc, 1, input, 1, output);

    client.client_arax_task_wait(task);

    const char *data = client.client_arax_data_get(output);

    something(test);

    fprintf(stderr, "Data received from client_arax_buffer_get (Should be %s): %s\n", test, data);

    client.client_arax_task_free(task);
    client.client_arax_proc_put(proc);
    client.client_arax_proc_put(accel);

    free(test);

    return 0;
} // main
