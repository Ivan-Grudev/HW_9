#define BOOST_DATE_TIME_NO_LIB

#include <iostream>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/container/scoped_allocator.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/string.hpp>

using namespace boost::interprocess;

typedef managed_shared_memory::segment_manager segment_manager_t;
typedef boost::container::scoped_allocator_adaptor<allocator<void, segment_manager_t> > void_allocator;
typedef void_allocator::rebind<std::string>::other string_allocator;
typedef vector<std::string, string_allocator> string_vector;


class complex_data
{
public:
    string_vector string_vector;

    typedef void_allocator allocator_type;

    complex_data(complex_data const& other, const allocator_type& void_alloc)
        : string_vector(other.string_vector, void_alloc)
    {}
    complex_data(const allocator_type& void_alloc)
        : string_vector(void_alloc)
    {}

};

typedef void_allocator::rebind<complex_data>::other    complex_data_allocator;
typedef vector<complex_data, complex_data_allocator>   complex_data_vector;

int main()
{
    struct shared_memory_remove {
        shared_memory_remove() { shared_memory_object::remove("MySharedMemory"); }
        ~shared_memory_remove() { shared_memory_object::remove("MySharedMemory"); }
    } remover;

    managed_shared_memory segment(open_or_create, "MySharedMemory", 65536);
    void_allocator alloc_inst(segment.get_segment_manager());
    complex_data* complex_data0_ = segment.construct<complex_data>("MyComplexData")(alloc_inst);

    std::string message;
    size_t counter = 0;

    while (message != "end") {
        std::cin >> message;
        complex_data0_->string_vector.push_back(message);
        counter++;
    }

    std::cout << std::endl;

    for (size_t i = 0; i < counter - 1; i++) {
        std::cout << complex_data0_->string_vector[i] << std::endl;
    }

    return EXIT_SUCCESS;
}
