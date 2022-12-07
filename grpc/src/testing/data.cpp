#include "../arax_grpc_client/arax_grpc_client.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using namespace arax;

void something(const char* str){
  std::cout << "String length modulo 2: " << strlen(str) << "\n";

  return;
}

int main(int argc, char* argv[]){

  if(argc != 2){
    std::cerr << "Correct usage: <executable> <string_argument>\n";
    exit(1);
  }

  something(argv[1]);

  // Arax init and exit are called by the constructors and destructors respectively
  AraxClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

  // Request buffer
  const uint64_t buffer1 = client.client_arax_buffer(strlen(argv[1])+1);
  const uint64_t buffer2 = client.client_arax_buffer(strlen(argv[1])+1);

  // request accelerator
  const uint64_t accel1 = client.client_arax_accel_acquire_type(CPU);

  client.client_arax_set_data(buffer1, accel1, argv[1]);

  const uint64_t proc = client.client_arax_proc_register("something");

  const uint64_t task1 = client.client_arax_task_issue(accel1, proc, 1, buffer1, 2, buffer2);

  client.client_arax_proc_put(proc);

  return 0;
}
