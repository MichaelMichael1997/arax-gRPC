#include "../arax_grpc_client/arax_grpc_client.h"

// -- Arax header files --
#include <arax.h>
#include <arax_pipe.h>
#include <arax_types.h>
#include <core/arax_data.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using namespace arax;

#define MAGIC 1337

typedef uint64_t Task;
typedef uint64_t Buffer;
typedef uint64_t Proc;
typedef uint64_t Accel;

void noop_op(char *in, char *out, int l)
{
    int c;

    l -= 2;
    for (c = 0; l >= 0; l--, c++)
        out[c] = in[l];
    out[c] = 0;
}

#ifdef BUILD_MAIN

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage:\n\t%s <string>\n\n", argv[0]);
        return 0;
    }

    AraxClient client("localhost:50051");

    Accel accel = client.client_arax_accel_acquire_type(CPU);
    Proc proc   = client.client_arax_proc_get("noop");

    /* -- Failed to retrieve registered process -- */
    if (proc == 0) {
        exit(EXIT_FAILURE);
    }

    std::string input(argv[1]);

    for (int i = 0; i < 20000006; i++) {
        input += "t";
    }

    size_t size = input.size();

    fprintf(stderr, "-- Input string size in bytes: %zu\n", size);
    fprintf(stderr, "-- Input string size in megabytes: %zu\n", size >> 20);

    // size_t size = strlen(argv[1]) + 1;
    int magic  = MAGIC;
    char *temp = (char *) calloc(size, 1);

    Buffer io[2] = {
        client.client_arax_buffer(size),
        client.client_arax_buffer(size)
    };

    client.client_arax_data_set(io[0], accel, input);

    Task task = client.client_arax_task_issue(accel, proc, 0, 0, 1, io, 1, io + 1);

    uint64_t state = client.client_arax_task_wait(task);

    fprintf(stdout, "\n======================\n");
    fprintf(stdout, "-- 0: task_failed\n-- 1: task_issued\n-- 2: task_completed\n");
    fprintf(stdout, "======================\n\n");
    fprintf(stdout, "Task state returned by client_arax_task_wait: %zu\n", state);

    if (state == 0) { /* -- task failed -- */
        fprintf(stderr, "Task failed\n");
    }

    std::string out = client.client_arax_data_get(io[1]);

    // std::string out = client.client_arax_large_data_get(io[1]);

    noop_op((char *) input.c_str(), temp, size);
    if (strcmp(out.c_str(), temp) != 0) {
        fprintf(stderr, "-- Strings are not equal\n");
    } else {
        fprintf(stderr, "-- Strings are equal! --\n");
    }
    // fprintf(stderr, "Noop is   \'%s\'\n", out.c_str());
    // fprintf(stderr, "Should be \'%s\'\n", temp);
    client.client_arax_data_free(io[0]);
    client.client_arax_data_free(io[1]);
    client.client_arax_task_free(task);
    client.client_arax_proc_put(proc);
    client.client_arax_accel_release(accel);

    if (strcmp(out.c_str(), temp) == 0) {
        fprintf(stdout, "-- The two string are equal --\n");
    }

    return 0;
} // main

#endif /* -- ifdef BUILD_MAIN -- */


#ifdef BUILD_SO

#include <core/arax_data_private.h>
#include <AraxLibUtilsCPU.h>

arax_task_state_e noop(arax_task_msg_s *msg)
{
    int l = arax_data_size(msg->io[0]);

    char *in  = (char *) arax_data_deref(msg->io[0]);
    char *out = (char *) arax_data_deref(msg->io[1]);

    noop_op(in, out, l);
    arax_task_mark_done(msg, task_completed);

    return task_completed;
}

ARAX_PROC_LIST_START()
ARAX_PROCEDURE("noop", CPU, noop, 0)
ARAX_PROCEDURE("noop", GPU, noop, 0)
ARAX_PROC_LIST_END()

#endif /* -- ifdef BUILD_SO -- */
