#include "../arax_grpc_client/arax_grpc_client.h"

// -- Arax header files --
#include <arax.h>
#include <arax_pipe.h>
#include <arax_types.h>
#include <core/arax_data.h>

#include <cstring>
#include <vector>
#include <thread>
#include <iostream>

#define MAGIC 1337

typedef const uint64_t Task;
typedef const uint64_t Buffer;
typedef const uint64_t Proc;
typedef const uint64_t Accel;


void vac_per_thread(Proc proc, size_t ops, AraxClient *client)
{
    // arax_accel *accel = arax_accel_acquire_type(CPU);
    Accel accel = client->client_arax_accel_acquire_type(CPU);

    while (ops--) {
        std::cerr << ops << "\n";

        size_t size  = strlen("Hello") + 1;
        int magic    = MAGIC;
        Buffer io[2] = {
            client->client_arax_buffer(size),
            client->client_arax_buffer(size)
        };

        client->client_arax_data_set(io[0], accel, "Hello");

        Task task = client->client_arax_task_issue(accel, proc, magic, 4, 1, io[0], 1, io[1]);

        int state = client->client_arax_task_wait(task);

        fprintf(stdout, "State return by the client_arax_task_wait: %d\n", state);

        std::string output = client->client_arax_data_get(io[0]);

        std::cout << output << "\n";

        client->client_arax_data_free(io[0]);
        client->client_arax_data_free(io[1]);
        client->client_arax_task_free(task);
    }

    client->client_arax_accel_release(accel);
} // vac_per_thread

int main(int argc, char *argv[])
{
    AraxClient client("localhost:50051");

    // arax_proc *proc = arax_proc_get("noop");
    Proc proc = client.client_arax_proc_get("noop");

    std::vector<std::thread> threads;

    for (int c = 0; c < 10; c++)
        threads.emplace_back(vac_per_thread, proc, 1000, &client);

    for (std::thread & thread : threads)
        thread.join();

    client.client_arax_proc_put(proc);

    return 0;
}
