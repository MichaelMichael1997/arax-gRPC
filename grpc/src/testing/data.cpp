#include "../arax_grpc_client/arax_grpc_client.h"

// -- Arax header files --
#include <arax.h>
#include <arax_pipe.h>
#include <arax_types.h>
#include <core/arax_data.h>

#define ARR_SIZE 4000000

using namespace arax;

typedef uint64_t Task;
typedef uint64_t Buffer;
typedef uint64_t Proc;
typedef uint64_t Accel;

void process_arr(float *arr)
{
    for (int i = 0; i < ARR_SIZE; i++) {
        arr[i] *= 2;
    }
}

#ifdef BUILD_MAIN
int main(int argc, char *argv[])
{
    AraxClient client("localhost:50051");

    size_t size = ARR_SIZE * sizeof(float);
    float *p    = (float *) malloc(size);
    float *init = (float *) malloc(size);

    for (int i = 0; i < ARR_SIZE; i++) {
        p[i]    = i / 2.0f;
        init[i] = i / 2.0f;
        // printf("%f ", p[i]);
    }
    printf("\n");

    /* -- Request buffer -- */
    Buffer io[1] = {
        client.client_arax_buffer(size)
    };

    /* -- Request accelerator -- */
    Accel accel = client.client_arax_accel_acquire_type(CPU);

    /* -- Get registered process -- */
    Proc proc = client.client_arax_proc_get("float_array");

    /* -- Failed to retrieve registered process -- */
    if (proc == 0) {
        exit(EXIT_FAILURE);
    }

    client.client_arax_data_set(io[0], accel, p, size);

    // /* -- Issue task -- */
    Task task = client.client_arax_task_issue(accel, proc, 0, 0, 1, io, 1, io);

    int task_state = client.client_arax_task_wait(task);

    fprintf(stdout, "\n======================\n");
    fprintf(stdout, "-- -1: gRPC_failed\n--  0: task_failed\n--  1: task_issued\n--  2: task_completed\n");
    fprintf(stdout, "======================\n\n");
    fprintf(stdout, "Task state returned by client_arax_task_wait: %d\n", task_state);

    if (task_state == 0 || task_state == -1) { /* -- task failed -- */
        fprintf(stderr, "Task failed\n");
        client.client_arax_data_free(io[0]);
        client.client_arax_task_free(task);
        client.client_arax_proc_put(proc);
        client.client_arax_accel_release(accel);

        exit(EXIT_FAILURE);
    }

    // client.client_arax_data_get(io[0], p);
    client.client_arax_data_get(io[0], p, size);

    printf("After data_get\n");
    for (int i = 0; i < ARR_SIZE; i++) {
        // printf("%f ", p[i]);
        assert(p[i] == 2 * init[i]);
    }
    printf("\n");

    /* -- Free the resources -- */
    client.client_arax_data_free(io[0]);
    client.client_arax_task_free(task);
    client.client_arax_proc_put(proc);
    client.client_arax_accel_release(accel);

    free(p);
    free(init);

    return 0;
} // main

#endif /* -- BUILD_MAIN -- */

#ifdef BUILD_SO

#include <core/arax_data_private.h>
#include <AraxLibUtilsCPU.h>

arax_task_state_e float_array(arax_task_msg_s *msg)
{
    // proc_t proc  = *(proc_t *) arax_data_deref(msg->io[0]);
    float *arr = (float *) arax_data_deref(msg->io[0]);

    // block_process(proc);
    process_arr(arr);

    arax_task_mark_done(msg, task_completed);
    return task_completed;
}

ARAX_PROC_LIST_START()
ARAX_PROCEDURE("float_array", CPU, float_array, 0)
ARAX_PROCEDURE("float_array", GPU, float_array, 0)
ARAX_PROC_LIST_END()
#endif /* -- BUILD_SO -- */
