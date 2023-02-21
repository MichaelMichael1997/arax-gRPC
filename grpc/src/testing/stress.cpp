#include "../arax_grpc_client/arax_grpc_client.h"
#include <arax.h>
#include <arax_pipe.h>
#include <arax_types.h>
#include <core/arax_data.h>
#include <chrono>
#include <unistd.h>
#include <thread>

typedef uint64_t Task;
typedef uint64_t Buffer;
typedef uint64_t Proc;
typedef uint64_t Accel;

#define NUM_TASKS 2048

typedef struct test{
  int i;
}Test;

#ifdef BUILD_MAIN

int main(int argc, char *argv[])
{
    using std::chrono::high_resolution_clock;
    using std::chrono::duration;

    AraxClient client("localhost:50051");

    Accel accel = client.client_arax_accel_acquire_type(CPU);
    Proc proc   = client.client_arax_proc_get("stress");

    Test test;
    test.i = 0;

    /* -- Failed to retrieve registered process -- */
    if (proc == 0) {
        exit(EXIT_FAILURE);
    }

    std::vector<Task> tasks;
    tasks.reserve(NUM_TASKS);

    auto start = high_resolution_clock::now();
    client.set_reader_writer();
    for (int i = 0; i < NUM_TASKS; i++) {
        test.i = i+1;
        Task task = client.client_arax_task_issue_streaming(accel, proc, &test, sizeof(Test), 0, 0, 0, 0);
        tasks.push_back(task);
    }
    client.terminate_task_issue_streaming();
    auto end = high_resolution_clock::now();

    duration<double, std::milli> dur = end - start;
    std::cerr << "Loop time for streaming: " << dur.count() << " ms\n";

    std::cerr << "Tasks to be freed: " << tasks.size() << '\n';
    for (auto& task : tasks) {
      client.client_arax_task_free(task);
    }

    client.client_arax_accel_release(accel);

    return 0;
} // main

#endif /* -- ifdef BUILD_MAIN -- */


#ifdef BUILD_SO

#include <core/arax_data_private.h>
#include <AraxLibUtilsCPU.h>

arax_task_state_e stress(arax_task_msg_s *msg)
{
    arax_task_mark_done(msg, task_completed);

    return task_completed;
}

ARAX_PROC_LIST_START()
ARAX_PROCEDURE("stress", CPU, stress, 0)
ARAX_PROC_LIST_END()

#endif /* -- ifdef BUILD_SO -- */
