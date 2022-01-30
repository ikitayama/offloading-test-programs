#include <memory_resource>
#include <vector>
#include <iostream>
#include <omp.h>
#include <array>
#include <boost/core/demangle.hpp>
#include <deque>

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
    //auto ptr = (std::uintptr_t)llvm_omp_target_alloc_shared(bytes,0);
    auto ptr = (void*)llvm_omp_target_alloc_shared(bytes,0);

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

class A {
public:
   std::string name;
   double a_;
   float b_;
};
int main() {
  debug_resource myres{};
  std::pmr::set_default_resource(&myres);  
  std::vector<int> v(1024);
  std::pmr::vector<std::array<double,3>> va(1024);
 std::cout << v[0] << std::endl;
 std::cout << boost::core::demangle(typeid(va[0]).name()) << std::endl;

  std::pmr::deque<int> d  = {9,10};

  std::pmr::vector<int> v1 = {1,2,3};
  std::pmr::vector<int> v2 = {3,2,1};
  v1.swap(v2);
  std::cout << v1[0] << v1[1] << v1[2] << std::endl;
#pragma omp target 
{
  v[0] = 1023;
  va[0][0]=0.123;
  //std::pmr::deque<int> d1 = {10,1};
}
 std::cout << va[0][0] << std::endl;

}
