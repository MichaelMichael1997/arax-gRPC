#include "../arax_grpc_client/arax_grpc_client.h"
#include "arax.h"

/*
 * This file is used to observe how the arax api library functions
 * work. NOT a demo, just function testing in isolation, irrelevant from gRPC
 */

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using namespace arax;


int main(int argc, char* argv[]){

  AraxClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

  arax_pipe_s* pipe_s = arax_init();

  std::cout << "-- Size of pipe_s: " << sizeof(*pipe_s) << "\n";

  arax_accel **accels = 0;
  
  int accel_num = arax_accel_list(CPU, 1, &accels);

  std::cout << "-- Number of physical CPU accelerators: " << accel_num << "\n";

  arax_accel* accel = arax_accel_acquire_type(CPU);

  std::cout << "-- Size of arax accelerator structure: " << sizeof(accel) << std::endl;

  std::cout << "-- Acquire physical accelerator";

  // uint64_t proc_counter = arax_pipe_add_process(pipe_s);
  // std::cout << "-- Number of active processes before adding issuer: " << proc_counter << std::endl;

  // proc_counter = arax_pipe_del_process(pipe_s);
  // std::cout << "-- Number of active processes before removing issuer: " << proc_counter << std::endl;

  // arax_accel_list_free(accels);
  
  arax_exit();

  return 0;
}
