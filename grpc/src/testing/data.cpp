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
        i % 2 == 0 ? *(str + i) = '*' : *(str + i) = '_';
    }
}

#ifdef BUILD_MAIN

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Correct usage: %s <string_argument>\n\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    AraxClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    size_t size = strlen(argv[1] + 1);
    /* -- Request buffer -- */
    Buffer input  = client.client_arax_buffer(size);
    Buffer output = client.client_arax_buffer(size);
    /* -- Request accelerator -- */
    Accel accel = client.client_arax_accel_acquire_type(CPU);

    /* -- Get registered process -- */
    Proc proc = client.client_arax_proc_get("something");

    /* -- Failed to retrieve registered process -- */
    if (proc == 0) {
        exit(EXIT_FAILURE);
    }

    /* -- Set the data -- */
    client.client_arax_data_set(input, accel, argv[1]);

    /* -- Issue task -- */
    Task task = client.client_arax_task_issue(accel, proc, 1, input, 1, output);

    int task_state = client.client_arax_task_wait(task);

    fprintf(stdout, "\n======================\n");
    fprintf(stdout, "-- 0: task_failed\n-- 1: task_issued\n-- 2: task_completed\n");
    fprintf(stdout, "======================\n\n");
    fprintf(stdout, "Task state returned by client_arax_task_wait: %d\n", task_state);

    if (task_state == 0) { /* -- task failed -- */
        fprintf(stderr, "Task failed\n");
        client.client_arax_data_free(input);
        client.client_arax_data_free(output);
        client.client_arax_task_free(task);
        client.client_arax_proc_put(proc);
        client.client_arax_accel_release(accel);

        exit(EXIT_FAILURE);
    }


    /* -- Get the data from the buffer after they are processed -- */
    char *data = client.client_arax_data_get(output);
    char *tmp  = (char *) calloc(size, 1);

    strcpy(tmp, argv[1]);
    something_op(tmp);

    fprintf(stdout, "Data received : %s\n", data);
    fprintf(stdout, "Should be: %s\n", tmp);

    /* -- Free the resources -- */
    client.client_arax_data_free(input);
    client.client_arax_data_free(output);
    client.client_arax_task_free(task);
    client.client_arax_proc_put(proc);
    client.client_arax_accel_release(accel);

    free(tmp);

    return 0;
} // main

#endif /* -- BUILD_MAIN -- */

#ifdef BUILD_SO

#include <core/arax_data_private.h>
#include <AraxLibUtilsCPU.h>

arax_task_state_e something(arax_task_msg_s *msg)
{
    int l1 = arax_data_size(msg->io[0]);
    int l2 = arax_data_size(msg->io[1]);

    if (l1 != l2) {
        return task_failed;
    }

    char *in  = (char *) arax_data_deref(msg->io[0]);
    char *out = (char *) arax_data_deref(msg->io[1]);

    strcpy(out, in);
    something_op(out);

    arax_task_mark_done(msg, task_completed);
    return task_completed;
}

ARAX_PROC_LIST_START()
ARAX_PROCEDURE("something", CPU, something, 0)
ARAX_PROCEDURE("something", GPU, something, 0)
ARAX_PROC_LIST_END()
#endif /* -- BUILD_SO -- */
