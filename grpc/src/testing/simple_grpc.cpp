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

#define BUFFER const uint64_t
#define ACCEL  const uint64_t
#define PROC   const uint64_t
#define TASK   const uint64_t


int main(int argc, char *argv[])
{
    AraxClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    std::string input("helloworld");

    // Request buffer
    BUFFER buffer = client.client_arax_buffer(strlen(input.c_str()) + 1);

    // Request accelerator
    ACCEL accel = client.client_arax_accel_acquire_type(CPU);

    client.client_arax_data_set(buffer, accel, input.c_str());

    std::string data_get(client.client_arax_data_get(buffer));

    std::cout << "Data gotten from buffer: " << data_get << "\n";

    client.client_arax_data_free(buffer);

    std::string data_get2(client.client_arax_data_get(buffer));

    std::cout << "Data gotten after data_free: " << data_get2 << "\n";

    client.client_arax_accel_release(accel);

    // const uint64_t proc = client.client_arax_proc_register("Random");

    // client.client_arax_proc_put(proc);

    // std::cout << "ID of buffer: " << buffer << "\n";
    // std::cout << "ID of proc: " << proc << "\n";

    return 0;
} // main
