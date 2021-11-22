#include <cstdlib>
#include <new>
#include <limits>
#include <iostream>
#include <vector>
#include <omp.h>
#include <array>
#include <memory_resource>
#include <cxxabi.h>
#include <boost/core/demangle.hpp>

extern "C" {
  void* llvm_omp_target_alloc_shared(size_t, int);
} 

template <class T>
struct Mallocator
{
  typedef T value_type;
 
  Mallocator () = default;
  template <class U> constexpr Mallocator (const Mallocator <U>&) noexcept {}
 
  [[nodiscard]] T* allocate(std::size_t n) {
    if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
      throw std::bad_array_new_length();
 
    //if (auto p = static_cast<T*>(std::malloc(n*sizeof(T)))) {
    //if (T* p = static_cast<T*>(llvm_omp_target_alloc_shared(n*sizeof(T), 0))) {
    if (std::uintptr_t p = (std::uintptr_t)llvm_omp_target_alloc_shared(n*sizeof(T), 0)) {
      //report(p, n);
      return reinterpret_cast<T*>(p);
    }
 
    throw std::bad_alloc();
  }
 
  void deallocate(T* p, std::size_t n) noexcept {
    //report(p, n, 0);
    //std::free(p);
    //omp_target_free(p, 0);
  }
 
private:
  void report(std::uintptr_t p, std::size_t n, bool alloc = true) const {
    std::cout << (alloc ? "Alloc: " : "Dealloc: ") << sizeof(T)*n
      << " bytes at " << std::hex << std::showbase
      << reinterpret_cast<void*>(p) << std::dec << '\n';
    std::cout << "Type " << boost::core::demangle(typeid(T).name()) << std::endl;
  }
};
 
template <class T, class U>
bool operator==(const Mallocator <T>&, const Mallocator <U>&) { return true; }
template <class T, class U>
bool operator!=(const Mallocator <T>&, const Mallocator <U>&) { return false; }
 
//typedef std::vector<int, Mallocator<int> > vector<int>;
class MultiChannelInputBuffer {
public:
     MultiChannelInputBuffer();
     void add(int slot, int channel) {
#pragma omp critical
         buffer_[slot][channel] += 1.0;
     };
     double get(int slot, int channel) {
         return buffer_[slot][channel];
     };
     std::array<double, 3>& get1() {
         return buffer_[0];
     };
     size_t size() { return buffer_.size(); };
private:
     std::vector<std::array<double, 3>, Mallocator<std::array<double, 3>>> buffer_;
     //std::vector<std::array<double, 3>> buffer_;
};
MultiChannelInputBuffer::MultiChannelInputBuffer() {
    buffer_.reserve(1024);
    for (int i=0;i<1024;i++)
       buffer_[i].fill(0.4);
}
class A {
public:
     A();
     void set() {
       reinterpret_cast<MultiChannelInputBuffer*>(&(B_.input_buffer_))->add(1,1);
     };
     void seta() {
#pragma omp critical   
       a_ += 1;
     }
     double get() {
       return reinterpret_cast<MultiChannelInputBuffer*>(&(B_.input_buffer_))->get(1,1);
     }
     int geta() {
        return a_;
    }
     int& refa() {
        return a_;
     }
     struct Buffers_ {
       Buffers_( A& );
       Buffers_( const Buffers_&, A& );  
       MultiChannelInputBuffer input_buffer_;

     };  
     Buffers_& getB() {
       return B_;
     }
     std::vector<int, Mallocator<int> > v1_;
private:
   Buffers_ B_;
   int a_;
};

A::Buffers_::Buffers_( A&) {
  //input_buffer_ = MultiChannelInputBuffer();
}
A::A() : B_( *this ) {
//A::A() {
}

int main()
{
  int a1;
  int xy = 0;
  int a, b, c;
  std::vector<std::array<double, 3>, Mallocator<std::array<double,3>>> vOfArray(10);
  
#pragma omp target parallel for
  for (int i=0;i < 1024;i++) {
    auto p = reinterpret_cast<std::array<double,3>*>(vOfArray.data());
    //((*p)[1])[0] =3;
    auto p1 = &((*p)[0]);
    *(p1 + 1) = 104;
#if (!__CUDA_ARCH__) && (___CUDA__)
   const char *name = typeid(xxx).name();
   printf("%s\n", name);
#endif
  }
  std::cout << "after " << vOfArray[0][1] << std::endl;
  return 0;
}
