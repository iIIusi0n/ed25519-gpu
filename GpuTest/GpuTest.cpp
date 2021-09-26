#define BOOST_COMPUTE_DEBUG_KERNEL_COMPILATION
#define BOOST_COMPUTE_USE_OFFLINE_CACHE

#include <boost/chrono.hpp>
#include <boost/compute.hpp>
#include <future>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace compute = boost::compute;
namespace chrono = boost::chrono;

typedef uint8_t public_key_t[32];
typedef uint8_t private_key_t[64];

std::string read_file(const std::string& file_name) {
  std::ifstream ifs(file_name.c_str(),
                    std::ios::in | std::ios::binary | std::ios::ate);

  std::ifstream::pos_type size = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  std::vector<char> bytes(size);
  ifs.read(bytes.data(), size);

  return std::string(bytes.data(), size);
}

int main() {
  try {
    assert(sizeof(public_key_t) == 32);
    assert(sizeof(private_key_t) == 64);

    auto hash_count = 1024 * 1024;

    assert(hash_count < 1024 * 1024 * 16);

    compute::device device = compute::system::default_device();
    compute::context context(device);
    compute::command_queue queue(context, device);

    std::default_random_engine generator(
        chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<short> rand(0, 255);
    std::vector<uint8_t> random_number_host(hash_count * 32);
    std::generate(random_number_host.begin(), random_number_host.end(),
                  [generator, rand]() mutable { return rand(generator); });

    assert(random_number_host[0] != random_number_host[1] !=
           random_number_host[2] != random_number_host[3]);

    auto public_keys = std::vector<public_key_t>(hash_count);
    auto private_keys = std::vector<private_key_t>(hash_count);

    compute::buffer buffer_a(context, hash_count * 32);
    compute::buffer buffer_b(context, hash_count * 64);
    compute::buffer buffer_c(context, hash_count * 32);

    compute::program program =
        compute::program::create_with_source_file("kernel.cl", context);
    program.build();

    compute::kernel kernel(program, "ed25519_create_keypair");
    kernel.set_arg(0, buffer_a);
    kernel.set_arg(1, buffer_b);
    kernel.set_arg(2, buffer_c);

    queue.enqueue_write_buffer(buffer_c, 0, hash_count * 32,
                               random_number_host.data());

    auto start_time = chrono::high_resolution_clock::now();

    queue.enqueue_1d_range_kernel(kernel, 0, hash_count, 0);

    auto end_time = chrono::high_resolution_clock::now();

    queue.enqueue_read_buffer(buffer_a, 0, hash_count * sizeof(public_key_t),
                              public_keys.data());
    queue.enqueue_read_buffer(buffer_b, 0, hash_count * sizeof(private_key_t),
                              private_keys.data());

    std::cout << "--gpu test (ed25519)--" << std::endl;
    std::cout << "key count: " << hash_count << std::endl;
    std::cout << "device: " << device.name() << std::endl;
    std::cout << "time: "
              << chrono::duration_cast<chrono::nanoseconds>(end_time -
                                                            start_time)
                     .count()
              << " ns" << std::endl;
  } catch (const std::exception& exc) {
    std::cerr << exc.what();
  }
  return 0;
}
