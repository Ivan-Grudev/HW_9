#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <mutex>

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/container/scoped_allocator.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/string.hpp>

using namespace boost::interprocess;
std::mutex m_mutex;

const int Char_Number_Max = 10000;

typedef managed_shared_memory::segment_manager segment_manager_t;
typedef boost::container::scoped_allocator_adaptor<allocator<void, segment_manager_t> > void_allocator;
typedef void_allocator::rebind<int>::other int_allocator;
typedef void_allocator::rebind<std::string>::other string_allocator;
typedef vector<std::string, string_allocator> string_vector;
typedef vector<int, int_allocator> int_vector;

class complex_data {
public:
	size_t indicator;
	size_t users; //кол-во пользователей
	size_t vec_size;
	int_vector ID_vector;
	string_vector string_vector;

	typedef void_allocator allocator_type;

	complex_data(complex_data const& other, const allocator_type& void_alloc)
		: vec_size(other.vec_size), indicator(other.indicator), ID_vector(other.ID_vector), users(other.users), string_vector(other.string_vector, void_alloc)
	{}
	complex_data(const allocator_type& void_alloc)
		: vec_size(0), indicator(0), ID_vector(void_alloc), users(0), string_vector(void_alloc)
	{}
};

typedef void_allocator::rebind<complex_data>::other complex_data_allocator;
typedef vector<complex_data, complex_data_allocator> complex_data_vector;

void Write(size_t ID, complex_data* complex_data_1) {

	std::string message;

	while (message != "end") {
		getline(std::cin, message);
		complex_data_1->ID_vector.push_back(ID);
		complex_data_1->string_vector.push_back(message);
		complex_data_1->indicator = 1;
		complex_data_1->vec_size++;
		std::cout << std::endl;
	}
	if (message == "end") {
		complex_data_1->indicator = 2;
	}
}
void Read(size_t ID, complex_data* complex_data_1)
{
	while (complex_data_1->indicator != 2)
	{
		if ((complex_data_1->indicator == 1) && (complex_data_1->ID_vector.back() != ID))
		{
			std::lock_guard < std::mutex > lock(m_mutex);
			std::cout << "Person №" << complex_data_1->ID_vector.back() << ": " << complex_data_1->string_vector.back() << std::endl;
			complex_data_1->indicator = 0;
		}
	}
}

int main()
{
	struct shared_memory_remove {
		shared_memory_remove() { shared_memory_object::remove("MySharedMemory"); }
		~shared_memory_remove() { shared_memory_object::remove("MySharedMemory"); }
	};
	managed_shared_memory segment(open_or_create, "MySharedMemory", sizeof(char) * Char_Number_Max);
	void_allocator alloc_inst(segment.get_segment_manager());
	complex_data* complex_data_1 = segment.find_or_construct<complex_data>("MyComplexData")(alloc_inst);
	if (complex_data_1->users == 0)
        complex_data_1->vec_size = 0;

	if (complex_data_1->vec_size != 0) {
		std::cout << "Message collection:" << std::endl;
		for (size_t i = 0; i < complex_data_1->vec_size; i++) {
			std::cout << "Person №" << complex_data_1->ID_vector[i] << ": " << complex_data_1->string_vector[i] << std::endl;
		}
		std::cout << std::endl;
	}
	size_t ID = complex_data_1->users + 1;
	complex_data_1->users++;
	std::thread writer(Write, ID, complex_data_1);
	std::thread reader(Read, ID, complex_data_1);
	writer.join();
	reader.join();
	system("pause");
	return EXIT_SUCCESS;
}
