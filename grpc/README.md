# Computing acceleration as a Service using gRPC over the Arax Heterogeneous Accelerator Runtime Framework
## Author: Michael Michaelou



# Directory Layout
- **protos** - Protocol buffer definitions 
- **src**
  - **arax_grpc_client** - gRPC Arax API
  - **server** - gRPC Server implementation.
  - **testing** - Some tests/examples 

# Building

## **Client,Protos and Tests**
  To build the gRPC Arax library and examples: 

  ```
  rm -rf build
  mkdir build
  cd build
  cmake ..
  make -j
  ``` 
## **Server**
  To build the server go to src/server and:
  ```
  rm -rf build
  mkdir build
  cd build
  cmake ..
  make -j
  ```
