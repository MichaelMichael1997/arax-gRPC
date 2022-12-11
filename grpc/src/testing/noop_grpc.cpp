#include "../arax_grpc_client/arax_grpc_client.h"
#include "../server/server.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using namespace arax;

#define MAGIC  1337

#define BUFFER const uint64_t
#define ACCEL  const uint64_t
#define PROC   const uint64_t
#define TASK   const uint64_t

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

    AraxClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    ACCEL accel = client.client_arax_accel_acquire_type(CPU);
    PROC proc   = client.client_arax_proc_get("noop");

    size_t size = strlen(argv[1]) + 1;
    char *temp  = (char *) calloc(size, 1);

    BUFFER io[2] = {
        client.client_arax_buffer(size),
        client.client_arax_buffer(size)
    };

    client.client_arax_data_set(io[0], accel, argv[1]);

    TASK task = client.client_arax_task_issue(accel, proc, 1, io[0], 1, io[1]);

    client.client_arax_task_wait(task);

    std::string out(client.client_arax_data_get(io[1]));

    fprintf(stderr, "Noop is   \'%s\'\n", out);
    noop_op(argv[1], temp, size);
    fprintf(stderr, "Should be \'%s\'\n", temp);
    client.client_arax_data_free(io[0]);
    client.client_arax_data_free(io[1]);
    client.client_arax_task_free(task);
    client.client_arax_proc_put(proc);
    client.client_arax_accel_release(accel);

    return strcmp(out.c_str(), temp);
} // main

#endif // BUILD_MAIN

#ifdef BUILD_SO
arax_task_state_e noop(arax_task_msg_s *msg)
{
    int l     = arax_data_size(msg->io[0]);
    char *in  = (char *) arax_data_deref(msg->io[0]);
    char *out = (char *) arax_data_deref(msg->io[1]);

    // int magic = *(int *) arax_task_host_data(msg, 4);

    // if (magic != MAGIC) {
    //     throw std::runtime_error(std::string("Magic does not match ") + std::to_string(magic) + " != "
    //             + std::to_string(MAGIC));
    // }
    noop_op(in, out, l);
    arax_task_mark_done(msg, task_completed);
    return task_completed;
}

ARAX_PROC_LIST_START()
ARAX_PROCEDURE("noop", CPU, noop, 0)
ARAX_PROCEDURE("noop", GPU, noop, 0)
ARAX_PROC_LIST_END()
#endif // ifdef BUILD_SO
