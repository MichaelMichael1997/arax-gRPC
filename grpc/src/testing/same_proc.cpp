#include "../arax_grpc_client/arax_grpc_client.h"
#include "../server/server.h"
#include <grpcpp/security/credentials.h>

/*
 *  Test to see if we can have a server and a client running
 *  in the same process, using multithreading
 */

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

int main(int argc, char *argv[])
{
    AraxServer server("localhost:50051");

    // Start a new server thread to listen to requests
    // Otherwise it will block the rest of the program
    std::thread server_thread([&server](){
      server.start_server();
        });

    AraxClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    std::string input("helloworld");

    // Request buffer
    Buffer buffer = client.client_arax_buffer(strlen(input.c_str()) + 1);

    // Request accelerator
    Accel accel = client.client_arax_accel_acquire_type(CPU);

    client.client_arax_data_set(buffer, accel, input.c_str());

    std::string data_get(client.client_arax_data_get(buffer));

    std::cout << "Data gotten from buffer: " << data_get << "\n";

    client.client_arax_data_free(buffer);

    // The buffers has been freed, this should throw a
    // 'Buffer not found error'
    std::string data_get2(client.client_arax_data_get(buffer));

    std::cout << "Data gotten after data_free: " << data_get2 << "\n";

    client.client_arax_accel_release(accel);

    Proc proc = client.client_arax_proc_get("noop");

    if (proc == 0) {
        fprintf(stderr, "Failed to fetch process 'Random'\n");
        server.shutdown();
        server_thread.join();
        exit(EXIT_FAILURE);
    }

    client.client_arax_proc_put(proc);

    // Shutdown the server
    server.shutdown();

    // Terminate server thread
    server_thread.join();

    return 0;
} // main
