#include "../arax_grpc_client/arax_grpc_client.h"
#include "../server/server.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using namespace arax;

#define MAGIC 1337

typedef const uint64_t Task;
typedef const uint64_t Buffer;
typedef const uint64_t Proc;
typedef const uint64_t Accel;

void noop_op(char *in, char *out, int l)
{
    int c;

    l -= 2;
    for (c = 0; l >= 0; l--, c++)
        out[c] = in[l];
    out[c] = 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage:\n\t%s <string>\n\n", argv[0]);
        return 0;
    }

    /* -- Server and client will run on the same process -- */
    // AraxServer server("localhost:50051");

    /* -- Create separate thread for server to run -- */
    // std::thread server_thread([&server](){
    //   server.start_server();
    //     });

    AraxClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    Accel accel = client.client_arax_accel_acquire_type(CPU);
    Proc proc   = client.client_arax_proc_get("noop");

    /* -- Failed to retrieve registered process -- */
    if (proc == 0) {
        // server.shutdown();
        // server_thread.join();
        exit(EXIT_FAILURE);
    }

    size_t size = strlen(argv[1]) + 1;
    char *temp  = (char *) calloc(size, 1);

    Buffer io[2] = {
        client.client_arax_buffer(size),
        client.client_arax_buffer(size)
    };

    client.client_arax_data_set(io[0], accel, argv[1]);

    Task task = client.client_arax_task_issue(accel, proc, 1, io[0], 1, io[1]);

    uint64_t state = client.client_arax_task_wait(task);

    fprintf(stdout, "Task state: %zu\n", state);

    std::string out(client.client_arax_data_get(io[1]));

    fprintf(stderr, "Noop is   \'%s\'\n", out.c_str());
    noop_op(argv[1], temp, size);
    fprintf(stderr, "Should be \'%s\'\n", temp);
    client.client_arax_data_free(io[0]);
    client.client_arax_data_free(io[1]);
    client.client_arax_task_free(task);
    client.client_arax_proc_put(proc);
    client.client_arax_accel_release(accel);

    // server.shutdown();
    // server_thread.join();

    return strcmp(out.c_str(), temp);
} // main
