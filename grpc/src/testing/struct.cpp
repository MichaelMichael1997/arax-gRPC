#include "../arax_grpc_client/arax_grpc_client.h"

// -- Arax header files --
#include <arax.h>
#include <arax_pipe.h>
#include <arax_types.h>
#include <core/arax_data.h>

#define MAGIC 1337

using namespace arax;

typedef uint64_t Task;
typedef uint64_t Buffer;
typedef uint64_t Proc;
typedef uint64_t Accel;

typedef struct
{
    int running;
    int blocked;
    int ready;
} proc_stats;

typedef struct
{
    unsigned int proc_id;
    unsigned int state;
    proc_stats   stats;
} proc_t;

void block_proc(proc_t * const proc)
{
    proc->state         = 0;
    proc->stats.running = -1;
    proc->stats.blocked = 1;
    proc->stats.ready   = -1;
}

void print_proc(const proc_t& proc)
{
    fprintf(stderr, "Process ID %u\nProcess State %d\n", proc.proc_id, proc.state);
    fprintf(stderr, "--- STATS ---\n");
    fprintf(stderr, "Running %d\nBlocked %d\nReady %d\n", proc.stats.running,
      proc.stats.blocked,
      proc.stats.ready);
}

#ifdef BUILD_MAIN
int main(int argc, char *argv[])
{
    AraxClient client("localhost:50051");

    proc_t p;

    p.proc_id       = 1;
    p.state         = 2;
    p.stats.running = 1000;
    p.stats.blocked = -1000;
    p.stats.ready   = 1000;

    fprintf(stderr, "-- Before blocking --\n");
    print_proc(p);

    size_t size = sizeof(p);
    int magic   = MAGIC;

    /* -- Request buffer -- */
    Buffer io[1] = {
        client.client_arax_buffer(size)
    };

    /* -- Request accelerator -- */
    Accel accel = client.client_arax_accel_acquire_type(CPU);

    /* -- Get registered process -- */
    Proc proc = client.client_arax_proc_get("proc_sched");

    /* -- Failed to retrieve registered process -- */
    if (proc == 0) {
        exit(EXIT_FAILURE);
    }

    client.client_arax_data_set(io[0], accel, &p, size);

    // /* -- Issue task -- */
    Task task = client.client_arax_task_issue(accel, proc, &magic, 4, 1, io, 1, io);

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

    client.client_arax_data_get(io[0], &p);

    fprintf(stderr, "-- After blocking --\n");
    print_proc(p);

    /* -- Free the resources -- */
    client.client_arax_data_free(io[0]);
    client.client_arax_task_free(task);
    client.client_arax_proc_put(proc);
    client.client_arax_accel_release(accel);

    return 0;
} // main

#endif /* -- BUILD_MAIN -- */

#ifdef BUILD_SO

#include <core/arax_data_private.h>
#include <AraxLibUtilsCPU.h>

arax_task_state_e proc_sched(arax_task_msg_s *msg)
{
    // proc_t proc  = *(proc_t *) arax_data_deref(msg->io[0]);
    proc_t *p = (proc_t *) arax_data_deref(msg->io[0]);
    int magic = *(int *) arax_task_host_data(msg, 4);

    if (magic != MAGIC) {
        throw std::runtime_error("Magic numbers don't match! \'" + std::to_string(magic) + " != "
                + " \'" + std::to_string(MAGIC) + "\'\n");
    }

    // block_process(proc);
    block_proc(p);

    arax_task_mark_done(msg, task_completed);
    return task_completed;
}

ARAX_PROC_LIST_START()
ARAX_PROCEDURE("proc_sched", CPU, proc_sched, 0)
ARAX_PROCEDURE("proc_sched", GPU, proc_sched, 0)
ARAX_PROC_LIST_END()
#endif /* -- BUILD_SO -- */
