#include "../arax_grpc_client/arax_grpc_client.h"
#include <grpcpp/security/credentials.h>

/*
 *  Simple test file to see if the functions were as intended,
 *  but in isolation. This is NOT an actual demo
 */

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using namespace arax;

int main(int argc, char *argv[])
{
    AraxClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    // Request buffer
    const uint64_t buffer = client.client_arax_buffer(5);

    // Request accelerator
    const uint64_t accel1 = client.client_arax_accel_acquire_type(CPU);

    client.client_arax_accel_release(accel1);

    const uint64_t proc = client.client_arax_proc_register("Random");

    client.client_arax_proc_put(proc);

    std::cout << "ID of buffer: " << buffer << "\n";
    std::cout << "ID of proc: " << proc << "\n";

    return 0;
}
