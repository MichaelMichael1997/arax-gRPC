#include "../arax_grpc_client/arax_grpc_client.h"
#include <string>

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using namespace arax;

#define MAGIC 1337

typedef const uint64_t Task;
typedef const uint64_t Buffer;
typedef const uint64_t Proc;
typedef const uint64_t Accel;

/* -- Double every integer in the vector -- */
void vector_op(std::vector<int>& vec)
{
    for (auto& i : vec) {
        i *= 2;
    }

    return;
}

/* -- Serialize vector into a string sequence of bytes -- */
std::string serialize_vector(const std::vector<int> vec)
{
    std::ostringstream ss;

    for (auto i : vec) {
        ss << i << " ";
    }

    return ss.str();
}

std::vector<int> deserialize_vector(const std::string bytes)
{
    std::vector<int> retval;

    std::istringstream ss(bytes);

    int i = 0;

    while (ss >> i) {
        retval.push_back(i);
    }

    return retval;
}

#ifdef BUILD_MAIN

int main(int argc, char *argv[])
{
    AraxClient client("localhost:50051");

    std::vector<int> input_vec{ 1, 2, 3, 4, 5 };

    /* -- To test if large inputs work -- */
    for (int i = 0; i < 2600000; i++) {
        input_vec.push_back(i);
    }

    Accel accel = client.client_arax_accel_acquire_type(CPU);
    Proc proc   = client.client_arax_proc_get("vectorop");

    /* -- Failed to retrieve registered process -- */
    if (proc == 0) {
        exit(EXIT_FAILURE);
    }

    /* -- Serialize the vector input and get size -- */
    std::string vector_in = serialize_vector(input_vec);

    size_t bytes     = vector_in.size();
    size_t megabytes = bytes >> 20;

    std::cout << "Size of input in bytes: " << bytes << "\n";
    std::cout << "Size of input in megabytes: " << megabytes << "\n";

    size_t size = vector_in.size();
    int magic   = MAGIC;

    Buffer io[2] = {
        client.client_arax_buffer(size),
        client.client_arax_buffer(size)
    };

    client.client_arax_data_set(io[0], accel, vector_in);

    Task task = client.client_arax_task_issue(accel, proc, magic, 4, 1, io[0], 1, io[1]);

    uint64_t state = client.client_arax_task_wait(task);

    fprintf(stdout, "\n======================\n");
    fprintf(stdout, "-- -1: grpc_failed\n--  0: task_failed\n--  1: task_issued\n--  2: task_completed\n");
    fprintf(stdout, "======================\n\n");
    fprintf(stdout, "Task state returned by client_arax_task_wait: %zu\n", state);

    if (state == 0) { /* -- task failed -- */
        fprintf(stderr, "Task failed\n");
        exit(EXIT_FAILURE);
    }

    std::string vector_out = client.client_arax_data_get(io[1]);

    if (vector_out.empty()) {
        fprintf(stderr, "-- Failed to get data from buffer --\n");
        exit(EXIT_FAILURE);
    }

    std::vector<int> output_vec = deserialize_vector(vector_out);

    assert(input_vec.size() == output_vec.size());

    vector_op(input_vec);
    for (int i = 0; i < input_vec.size(); i++) {
        assert(input_vec.at(i) == output_vec.at(i));
    }

    // fprintf(stderr, "Initial vector: \n");
    // for (const auto i : input_vec) {
    //     std::cout << i << " ";
    // }
    // std::cout << "\n";

    // fprintf(stderr, "Output vector: \n");
    // for (const auto i : output_vec) {
    //     std::cout << i << " ";
    // }
    // std::cout << "\n";

    // vector_op(input_vec);
    // fprintf(stderr, "What should have been returned: \n");
    // for (const auto i : input_vec) {
    //     std::cout << i << " ";
    // }
    // std::cout << "\n";

    /* -- Free the resources -- */
    client.client_arax_data_free(io[0]);
    client.client_arax_data_free(io[1]);
    client.client_arax_task_free(task);
    client.client_arax_proc_put(proc);
    client.client_arax_accel_release(accel);

    return 0;
} // main

#endif /* -- ifdef BUILD_MAIN -- */


#ifdef BUILD_SO

#include <core/arax_data_private.h>
#include <AraxLibUtilsCPU.h>

arax_task_state_e vectorop(arax_task_msg_s *msg)
{
    char *in  = (char *) arax_data_deref(msg->io[0]);
    char *out = (char *) arax_data_deref(msg->io[1]);
    int magic = *(int *) arax_task_host_data(msg, 4);

    if (magic != MAGIC) {
        throw std::runtime_error("Magic numbers dont match! (" + std::to_string(magic) + ") vs ("
                + std::to_string(MAGIC) + ")\n");
    }

    std::vector<int> vec = deserialize_vector(std::string(in)); // <-- Super ugly

    vector_op(vec);

    std::string output = serialize_vector(vec);

    strcpy(out, output.c_str());

    arax_task_mark_done(msg, task_completed);

    return task_completed;
}

ARAX_PROC_LIST_START()
ARAX_PROCEDURE("vectorop", CPU, vectorop, 0)
ARAX_PROCEDURE("vectorop", GPU, vectorop, 0)
ARAX_PROC_LIST_END()

#endif /* -- ifdef BUILD_SO -- */
