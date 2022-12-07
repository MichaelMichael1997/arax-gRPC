#include "../arax_grpc_client/arax_grpc_client.h"
#include <grpcpp/security/credentials.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using namespace arax;

/*
   This one contains only a clean arax call, in case it's needed
   at some point, between testings
 */

int main(int argc, char* argv[]){

  AraxClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

  client.client_arax_clean();

  return 0;
}
