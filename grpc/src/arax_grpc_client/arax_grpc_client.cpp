#include "arax_grpc_client.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using google::protobuf::Empty;

using namespace arax;

/*
 * Add some colors for the output
 */
#ifdef __linux__
  #define ERROR_COL "\033[1;38;5;9;1m"
  #define SUCCESS_COL "\033[1;37;38;5;10m"
  #define RESET_COL "\033[0m"
#endif /* #ifdef __linux__ */

/*
 * Constructors
 */
AraxClient::AraxClient(std::shared_ptr<Channel> channel) : stub_(Arax::NewStub(channel)){}

/*
 * Destructors
 */
AraxClient::~AraxClient(){}


// -------------------- Arax Client Services --------------------

/*
 * Initialize Arax
 *
 * This should be called by all applications prior
 * to using any other Arax function.
 * @return An arax_pipe_s c struct object
 */
//  void AraxClient::client_arax_init(){

//   ClientContext ctx;
//   Arax_pipe_s pipe;
//   Empty req; // google::protobuf::Empty

//   Status status = stub_->Arax_init(&ctx, req, &pipe);

//   if(!status.ok()){

//     std::cout << status.error_code() << ": ";
//     std::cout << status.error_message() << "\n";

//     return;
//   }

//   return;
// }

 /*
  * Exit and cleanup Arax
  */
 // void AraxClient::client_arax_exit(){
 //    ClientContext ctx;
 //    Empty req;
 //    Empty res;

 //    Status status = stub_->Arax_exit(&ctx, req, &req);

 //    if(!status.ok()) {
 //      std::cout << status.error_code() << ": ";
 //      std::cout << status.error_message() << "\n";

 //      return;
 //    }

 //    std::cout << "Arax has exited\n";
 //    return;
 // }

 /*
  * Delete the shared segment
  */
  void AraxClient::client_arax_clean(){
   ClientContext ctx;
   Empty req;
   Empty res;

   Status status = stub_->Arax_clean(&ctx, req, &res);

   if(!status.ok()){
#ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
     std::cerr << "\nERROR: " << status.error_code() << "\n";
     std::cerr << status.error_message() << "\n";
     std::cerr << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */

     return;
   }

   std::cout << "-- Arax was cleaned up successfully\n";

   return;
 }

  /*
   * Returns the number of accelerators of a specific type
   * TODO: Plus some other things
   *
   * @param type the type of the accelerator
   * @param physical true if physical accelerators, false if virtual
   *
   * @return AcceListResponse, with the type and a container of AccelDescriptors 
   */
    AccelListResponse AraxClient::client_arax_accel_list(arax_accel_type_e type, bool physical){

      ClientContext ctx;
      AccelListRequest req;
      AccelListResponse res;

      req.set_type((int)type);
      req.set_physical(physical);

      Status status = stub_->Arax_accel_list(&ctx, req, &res);

      if(!status.ok()){
#ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
        std::cerr << "\nERROR: " << status.error_code() << "\n" ;
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
        return res;
      }

      return res;
    }

  /*
   * Returns arax revision
   *
   * @return string with arax revision
   */
  const char* AraxClient::client_arax_pipe_revision(){

    ClientContext ctx;
    Empty req;
    RevisionResponse res;

    Status status = stub_->Arax_pipe_get_revision(&ctx, req, &res);

    if(!status.ok()){
#ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
      std::cerr << "\nERROR: " << status.error_code() << "\n";
      std::cerr << status.error_message() << "\n";
      std::cerr << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
      return 0;
    }

    const char* retval = res.revision().c_str();

    return retval;
  }
  
  /*
   * Increase process counter for arax_pipe
   *
   * @return Number of active processes before adding issuer
   */
   uint64_t AraxClient::client_arax_pipe_add_process(){

     ClientContext ctx;
     Empty req;
     ProcCounter res;

     Status status = stub_->Arax_pipe_add_process(&ctx, req, &res);

     if(!status.ok()){
#ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
       std::cerr << "\nERROR: " << status.error_code() << "\n";
       std::cerr << status.error_message() << "\n";
       std::cerr << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
       return 0;
     }

     return res.proc_counter();
   }
   
   /*
     * Decrease process counter for pipe
     *
     * @return Number of active processes before removing issuer
     */
    uint64_t AraxClient::client_arax_pipe_del_process(){

      ClientContext ctx;
      Empty req;
      ProcCounter res;

      Status status = stub_->Arax_pipe_del_process(&ctx, req, &res);

      if(!status.ok()){
#ifdef __linux
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
        return 0;
      }

      return res.proc_counter();
    }
    
    /*
     * Free memory of accelerator array returned by arax accel list
     */
    void AraxClient::client_arax_accel_list_free(){
   
      ClientContext ctx;
      Empty req;
      Empty res;

      Status status = stub_->Arax_accel_list_free(&ctx, req, &res);

      if(!status.ok()){
#ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
        return;
      }

      std::cout << "-- Accelerator array was freed successfully\n";

      return; 
    }
    
    /*
     * Get the type of accelerator, specified in the AccelDescriptor request message
     *
     * @param req The accel descriptor message request
     *
     * @return the type of the accelerator -1 on failure
     */
    int AraxClient::client_arax_accel_type(AccelDescriptor req){
      ClientContext ctx;
      AccelListRequest res;

      Status status = stub_->Arax_accel_type(&ctx, req, &res);

      if(!status.ok()){
#ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
        return -1;
      }

      return res.type();
    }
    
    /*
     * Create an arax_buffer_s object
     *
     * @param size The desired size for the buffer
     *
     * @return The ID of the newly allocated buffer, or 0 on failure
     */
      uint64_t AraxClient::client_arax_buffer(size_t size){

      ClientContext ctx;
      RequestBuffer req;
      req.set_buffer_size(size);
      ResourceID res;

      Status status = stub_->Arax_buffer(&ctx, req, &res);

      if(!status.ok()){
#ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
        return res.id();
      }

      return res.id(); 
    }
    
    /*
     * Register a new process 'func_name'
     *
     * @param func_name The name of the process
     *
     * @return The id of the arax_proc resource, or 0 on failure 
     */
     uint64_t AraxClient::client_arax_proc_register(const char* func_name){
      ClientContext ctx;
      ProcRequest req;
      req.set_func_name(func_name);
      ResourceID res;

      Status status = stub_->Arax_proc_register(&ctx, req, &res);

      if(!status.ok()){
#ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
        return res.id();
      }

      return res.id(); 
    }
    
    /*
      * Delete registered arax proc pointer
      *
      * @param proc The ID of the arax_proc      
      *
      * @return nothing
      */
      void AraxClient::client_arax_proc_put(uint64_t id){
        ClientContext ctx;
        AraxProc req;
        req.set_id(id);
        ProcCounter res;

        Status status = stub_->Arax_proc_put(&ctx, req, &res);

        if(!status.ok()){
#ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
          std::cerr << "\nERROR: " << status.error_code() << "\n";
          std::cerr << status.error_message() << "\n";
          std::cerr << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
          return;
        }
       
        return;
      }
      
      /*
     * Acquire a virtual accelerator of the given type
     *
     * @param type The type of the accelerator
     *
     * @return The id of the acquired resource or 0 on failure
     */
       uint64_t AraxClient::client_arax_accel_acquire_type(unsigned int type){
        AccelRequest req;
        req.set_type(type);
        ResourceID res;
        ClientContext ctx;

        Status status = stub_->Arax_accel_acquire_type(&ctx, req, &res);

        if(!status.ok()){
#ifdef __linux__
          std::stringstream ss;
          ss << ERROR_COL;
          ss << "\nERROR: " << status.error_code() << "\n";
          ss << status.error_message() << "\n";
          ss << status.error_details() << "\n\n"; 
          ss << RESET_COL;
          std::cerr << ss.str();
#else
          std::cerr << "\nERROR: " << status.error_code() << "\n";
          std::cerr << status.error_message() << "\n";
          std::cerr << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
          return 0;
        }

        std::cout<< "-- Accelerator was acquired successfully\n";
      	return res.id();
      }
      
      /*
     * Release previously acquired accelerator
     *
     * @param id The id of the accelerator 
     *
     * @return nothing 
     */
      void AraxClient::client_arax_accel_release(uint64_t id){
        ClientContext ctx;
        ResourceID req;
        Empty res;

        req.set_id(id);
        Status status = stub_->Arax_accel_release(&ctx, req, &res);

        if(!status.ok()){
#ifdef __linux__
          std::stringstream ss;
          ss << ERROR_COL;
          ss << "\nERROR: " << status.error_code() << "\n";
          ss << status.error_message() << "\n";
          ss << status.error_details() << "\n\n"; 
          ss << RESET_COL;
          std::cerr << ss.str();
#else
          std::cerr << "\nERROR: " << status.error_code() << "\n";
          std::cerr << status.error_message() << "\n";
          std::cerr << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
          return ;
        }

        std::cout<< "-- Accelerator was released successfully\n";

        return;      
      }

      /*
     * Set data to buffer
     *
     * @param buffer The ID of the buffer
     * @param accel The ID of the accelerator
     * @param value The string value to be passed to buffer
     * TODO: Change this depending on the possible values a buffer can get
     * 
     * @return nothing 
     */
    void AraxClient::client_arax_set_data(uint64_t buffer, uint64_t accel, const char* value){
      DataSet req;
      Empty res;
      ClientContext ctx;

      req.set_buffer(buffer);
      req.set_accel(accel);
      req.set_str_val(value);

      Status status = stub_->Arax_data_set(&ctx, req, &res);

      if(!status.ok()){
#ifdef __linux
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
        return;
      }

      std::cout << "-- Data was set successfully\n";
      return;
    }
    
    /*
     * Issue a new task
     *
     * @param accel The ID of the accelerator responsible for executing the task
     * @param proc ID of the arax_proc to be dispatched on accelerator
     * @param in_count Size of input array (elements)
     * @param in_buffer Input buffer 
     * @param out_count Size of output array (elements)
     * @param out_buffer Output buffer 
     *
     * @return The ID of the new task of 0 on failure 
     */
     uint64_t AraxClient::client_arax_task_issue(uint64_t accel, uint64_t proc, size_t in_count, uint64_t in_buffer,
                size_t out_count, uint64_t out_buffer){
               
      TaskRequest req;
      ResourceID res;
      ClientContext ctx;

      req.set_accel(accel);
      req.set_proc(proc);
      req.set_in_buffer(in_buffer);
      req.set_out_buffer(out_buffer);
      req.set_in_count(in_count);
      req.set_out_count(out_count);

      Status status = stub_->Arax_task_issue(&ctx, req, &res);

      if(!status.ok()){
#ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
#endif
        return 0;
      }

      std::cout << "-- Task was issued successfully!\n";
      return res.id();
    }
    
    /*
     * Decrease ref counter of task
     *
     * @param task The ID of the task
     *
     * @return nothing
     */
    void AraxClient::client_arax_task_free(uint64_t task){
      ClientContext ctx;
      Task req;
      Empty res;

      req.set_task_id(task);

      Status status = stub_->Arax_task_free(&ctx, req, &res);

      if(!status.ok()){
#ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
        std::cerr << "\nERROR: " << status.error_code() << "\n";
        std::cerr << status.error_message() << "\n";
        std::cerr << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
        return; 
      }

      std::cout << "-- Task was freed successfully\n";
      return;
    }
    
    /*
     * Wait for an issued task to complete or fail
     *
     * @param id The ID of the task
     *
     * @return The state of the task or -1 on error
     */
    int AraxClient::client_arax_task_wait(uint64_t task){
      ClientContext ctx;
      Task req;
      Task res;
      req.set_task_id(task);

      Status status = stub_->Arax_task_wait(&ctx, req, &res);

      if(!status.ok()){
#ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
        std::cout << "\nERROR: " << status.error_code() << "\n";
        std::cout << status.error_message() << "\n";
        std::cout << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
        return -1;
      }

      return res.task_state();
    }
    
    /*
     * Gen an orphan/Unassigned virtual accelerator of NULL
     * Function will sleep if there are no orphans exist at the time of the call
     *
     * @return The ID of the retrieved vaccel or 0 on failure
     */
    uint64_t AraxClient::client_arax_pipe_get_orphan_vaccel(){
   
        ClientContext ctx;
        Empty req;
        ResourceID res;

        Status status = stub_->Arax_pipe_get_orphan_vaccel(&ctx, req, &res); 

        if(!status.ok()){
#ifdef __linux__
          std::stringstream ss;
          ss << ERROR_COL;
          ss << "\nERROR: " << status.error_code() << "\n";
          ss << status.error_message() << "\n";
          ss << status.error_details() << "\n\n"; 
          ss << RESET_COL;
          std::cerr << ss.str();
#else
          std::cout << "\nERROR: " << status.error_code() << "\n";
          std::cout << status.error_message() << "\n";
          std::cout << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
        return 0;
      }

        return res.id();
    }
    
    /*
     * Add vac with ID \c id to the list of orphan_vacs/unassigned accels
     *
     * @param id The ID of the vaccel
     *
     * @return nothing
     */
    void AraxClient::client_arax_pipe_add_orphan_vaccel(uint64_t id){
      ClientContext ctx;
      ResourceID req;
      Empty res;

      Status status = stub_->Arax_pipe_add_orphan_vaccel(&ctx, req, &res);

      if(!status.ok()){
#ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
        std::cout << "\nERROR: " << status.error_code() << "\n";
        std::cout << status.error_message() << "\n";
        std::cout << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
        return;
      }
    }
    
        /*
     * Returns false if there are no orphan vaccels
     *
     * @return true if orphan vaccels exist, false otherwise
     */
    bool AraxClient::client_arax_pipe_have_orphan_vaccels(){
      ClientContext ctx;
      Empty req;
      Success res;

      Status status = stub_->Arax_pipe_have_orphan_vaccels(&ctx, req, &res);

      if(!status.ok()){
#ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
        std::cout << "\nERROR: " << status.error_code() << "\n";
        std::cout << status.error_message() << "\n";
        std::cout << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
        return false;
      }

      return res.success();
    }
    
    /*
     * Remove vaccel, specified by \c id, from the orphan vaccel list
     *
     * @param id The id for the vaccel
     *
     * @return void
     */
    void AraxClient::client_arax_pipe_remove_orphan_vaccel(uint64_t id){
      ClientContext ctx;
      ResourceID req;
      Empty res;

      Status status = stub_->Arax_pipe_remove_orphan_vaccel(&ctx, req, &res);

      if(!status.ok()){
#ifdef __linux__
        std::stringstream ss;
        ss << ERROR_COL;
        ss << "\nERROR: " << status.error_code() << "\n";
        ss << status.error_message() << "\n";
        ss << status.error_details() << "\n\n"; 
        ss << RESET_COL;
        std::cerr << ss.str();
#else
        std::cout << "\nERROR: " << status.error_code() << "\n";
        std::cout << status.error_message() << "\n";
        std::cout << status.error_details() << "\n\n";
#endif /* ifdef __linux__ */
      }

      return;
    }











