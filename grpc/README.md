# Computing acceleration as a Service using gRPC over the Arax Heterogeneous Accelerator Runtime Framework
## Author: Michael Michaelou



# Directory Layout
- **protos** - The directory with the protocol buffers. The main file is **arax.proto**
- **src**
  - **arax_grpc_client** - Contains the code for the Client API. Note that the Client API, as well as the proto files are built from the build directory in the src directory
  - **server** - Contains the Server Class implementation.
  - **testing** - Contains the tests

# Building

## **Client,Protos and Tests**
  You can build the client API, generate the *.pb.h, *.pb.cc, files from the protocol buffers, as well as the tests/kernel ops by doing the following, from inside the grpc/src directory:
  ```
  rm -rf build
  mkdir build
  cd build
  cmake ..
  make
  ``` 
## **Server**
To build the server go to the grpc/src/server directory and do the following:
  ```
  rm -rf build
  mkdir build
  cd build
  cmake ..
  make
  ```
