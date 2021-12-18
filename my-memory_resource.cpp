#include <memory_resource>
#include <vector>
#include <iostream>
#include <omp.h>

extern "C" {
  void* llvm_omp_target_alloc_shared(size_t, int);
}

class debug_resource : public std::pmr::memory_resource {
public: 

  void* do_allocate(
    std::size_t bytes, 
        std::size_t alignment
    )                             override;

  void do_deallocate(
        void*  p, 
        size_t bytes, 
        size_t alignment
    )                             override;
    bool do_is_equal(
        const memory_resource& other
    ) const noexcept              override;

};

void* debug_resource::do_allocate(
    std::size_t bytes,
    std::size_t alignment [[maybe_unused]]
)
{
    auto ptr = (std::uintptr_t)llvm_omp_target_alloc_shared(bytes,0);

    std::cout << "Allocating " << bytes << " bytes ";
    std::cout << "at address " << ptr << std::endl;
    return reinterpret_cast<void*>(ptr);
}

void debug_resource::do_deallocate(
    void* p,
    std::size_t bytes     [[maybe_unused]],
    std::size_t alignment [[maybe_unused]]
)
{
    //free(p);
    omp_target_free(reinterpret_cast<void*>(p), 0);
}


bool debug_resource::do_is_equal(
    const memory_resource& other [[maybe_unused]]
) const noexcept
{
    return true;
}

class Test : public std::pmr::memory_resource {

};

int main() {
  debug_resource myres{};
  
  std::pmr::vector<int> v(1024, &myres);




}
