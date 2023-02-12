#include <iostream>
#include <map>


class SmallAllocator
{
private:
	static constexpr size_t init_free_memory_{ 1048576 };
	size_t free_memory_{ init_free_memory_ };

	char Memory[init_free_memory_]{};
	std::map<void *, size_t> used_addrs_;
	std::map<void *, size_t> free_addrs_to_sizes_;

public:
	SmallAllocator()
	{
		free_addrs_to_sizes_[Memory] = init_free_memory_;
	}
	void *Alloc(unsigned int size)
	{
		if (free_memory_ < size) {
			std::cout << "No free memory" << std::endl;
			return nullptr;
		}

		bool found_free_block = false;
		void *found_free_block_addr = nullptr;
		size_t found_free_blk_size = 0;
		for (auto &free_addr_to_size : free_addrs_to_sizes_) {
			found_free_blk_size = free_addr_to_size.second;
			if (found_free_blk_size >= size) {
				found_free_block = true;
				found_free_block_addr = free_addr_to_size.first;
				break;
			}
		}
		if (!found_free_block) {
			std::cout << "Memory is highly fragmented, can't find a block"
					  << std::endl;
			return nullptr;
		}

		used_addrs_[found_free_block_addr] = size;
		free_memory_ -= size;

		free_addrs_to_sizes_.erase(found_free_block_addr);

		const size_t next_free_blk_size = found_free_blk_size - size;
		if (next_free_blk_size > 0) {
			void *next_free_addr =
					static_cast<char *>(found_free_block_addr) + size;
			free_addrs_to_sizes_[next_free_addr] = next_free_blk_size;
		}

		return found_free_block_addr;
	};

	void *ReAlloc(void *pointer, unsigned int size)
	{
		const auto iter = used_addrs_.find(pointer);
		if (iter == used_addrs_.end()) {
			std::cout << "Incorrect pointer" << std::endl;
			return nullptr;
		}
		const size_t used_memory_bytes = iter->second;
		// less, more, eq
		if (size < used_memory_bytes) {
			used_addrs_[iter->first] = size;
			void *next_free_addr = static_cast<char *>(iter->first) + size;
			const size_t next_free_blk_size = used_memory_bytes - size;

			free_addrs_to_sizes_[next_free_addr] = next_free_blk_size;
			free_memory_ += next_free_blk_size;
		} else if (size > used_memory_bytes) {
			auto *next_free_addr = Alloc(size);
			if (!next_free_addr) {
				std::cout << "Memory is highly fragmented, can't find a block"
						  << std::endl;
				return nullptr;
			}
			// copy data
			for (int i = 0; i < used_memory_bytes; ++i) {
				auto *writtable_next_free_addr =
						static_cast<char *>(next_free_addr);
				auto *readble_pointer = static_cast<char *>(iter->first);
				*writtable_next_free_addr++ = *readble_pointer++;
			}

			Free(iter->first);

			return next_free_addr;
		}

		return iter->first;
	};

	void Free(void *pointer)
	{
		const auto iter = used_addrs_.find(pointer);
		if (iter != used_addrs_.end()) {
			const size_t used_memory_bytes = iter->second;
			free_addrs_to_sizes_[iter->first] = used_memory_bytes;
			used_addrs_.erase(iter);
			free_memory_ += used_memory_bytes;
		} else {
			std::cout << "Incorrect pointer" << std::endl;
		}
	};
};


int main()
{
	SmallAllocator A1;

	int *A1_P1 = static_cast<int *>(A1.Alloc(sizeof(int)));

	A1_P1 = static_cast<int *>(A1.ReAlloc(A1_P1, 2 * sizeof(int)));

	A1.Free(A1_P1);


		SmallAllocator A2;
		int *A2_P1 = static_cast<int *>(A2.Alloc(10 * sizeof(int)));
		for (unsigned int i = 0; i < 10; i++) {
			A2_P1[i] = i;
		}
		for (unsigned int i = 0; i < 10; i++) {
			if (A2_P1[i] != i) {
				std::cout << "ERROR 1" << std::endl;
			}
		}

		int *A2_P2 = static_cast<int *>(A2.Alloc(10 * sizeof(int)));
		for (unsigned int i = 0; i < 10; i++) {
			A2_P2[i] = -1;
		}
		for (unsigned int i = 0; i < 10; i++) {
			if (A2_P1[i] != i) {
				std::cout << "ERROR 2" << std::endl;
			}
		}
		for (unsigned int i = 0; i < 10; i++) {
			if (A2_P2[i] != -1) {
				std::cout << "ERROR 3" << std::endl;
			}
		}


		A2_P1 = (int *) A2.ReAlloc(A2_P1, 20 * sizeof(int));
		for(unsigned int i = 10; i < 20; i++) A2_P1[i] = i;
		for(unsigned int i = 0; i < 20; i++) if(A2_P1[i] != i) std::cout << "ERROR 4" << std::endl;

//		for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 5" << std::endl;
//		A2_P1 = (int *) A2.ReAlloc(A2_P1, 5 * sizeof(int));
//		for(unsigned int i = 0; i < 5; i++) if(A2_P1[i] != i) std::cout << "ERROR 6" << std::endl;
//		for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 7" << std::endl;
//		A2.Free(A2_P1);
//		A2.Free(A2_P2);
}