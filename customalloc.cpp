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

typedef size_t index; // one of the NEST types.

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
      report(p, n);
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

template < unsigned int num_channels >
class MultiChannelInputBuffer {
public:
  MultiChannelInputBuffer();

  void add_value( const index slot, const index channel, const double value ); 

  void reset_values_all_channels( const index slot );
  void clear();
  
  void resize();
  double get() { return buffer_[1][0]; };
private:
  std::vector< std::array< double, num_channels >, Mallocator<std::array<double, num_channels>>> buffer_;
     //std::vector<std::array<double, 3>> buffer_;
};

template < unsigned int num_channels >
MultiChannelInputBuffer< num_channels >::MultiChannelInputBuffer()
  //: buffer_( kernel().connection_manager.get_min_delay() + kernel().connection_manager.get_max_delay() )
  : buffer_( 30 )
{

}

template < unsigned int num_channels >
inline void
MultiChannelInputBuffer< num_channels >::reset_values_all_channels( const index slot )
{
  //assert( 0 <= slot and slot < buffer_.size() );
  buffer_[ slot ].fill( 0.0 );
}


#pragma omp declare target
template < unsigned int num_channels >
inline void
MultiChannelInputBuffer< num_channels >::add_value( const index slot, const index channel, const double value )
{
  //buffer_[ slot ][ channel ] += value; // original
  // above works perfct on host, but not on device.
  // 'buffer_' is a std::vector<std::array<double, num_channels>>
  auto tmp = reinterpret_cast<std::array<double, num_channels>*>(buffer_.data());
 auto tmp1 = &(tmp[slot]); // tmp1 is a reference to the object of whose type is std::array<double, num_channels>
 //auto tmp2 = &(tmp1[channel]);//) += value;
 //*(tmp2) += value ;
 (*tmp1)[channel]+=value;

}

template < unsigned int num_channels >
void
MultiChannelInputBuffer< num_channels >::resize()
{
  const size_t size = 30;//kernel().connection_manager.get_min_delay() + kernel().connection_manager.get_max_delay();
  if ( buffer_.size() != size )
  {
    //buffer_.resize( size, std::array< double, num_channels >() );
  }
}


template < unsigned int num_channels >
void
MultiChannelInputBuffer< num_channels >::clear()
{
  //resize(); // does nothing if size is fine
  // set all elements to 0.0
  for ( index slot = 0; slot < buffer_.size(); ++slot )
  {
    reset_values_all_channels( slot );
  }
}

#pragma omp end declare target

class Node {
public:
     Node();
     void init();
#pragma omp declare target
     void set() {
       B_.input_buffer_.add_value(1, 0, 10.0);
     };
     double get() {
       return B_.input_buffer_.get();
     }
#pragma omp end declare target
private:
     void init_buffers_();
     struct Buffers_ {
       Buffers_( Node& );
       Buffers_( const Buffers_&, Node& );  

       enum
       {
          SYN_IN = 0,
          SYN_EX,
          I0,
          NUM_INPUT_CHANNELS
       };
       MultiChannelInputBuffer< NUM_INPUT_CHANNELS > input_buffer_;

     };  
     Buffers_& getB() {
       return B_;
     }
   Buffers_ B_;
};

Node::Buffers_::Buffers_( Node& n ) {
}

Node::Node() : B_( *this ) {
}
void
Node::init()
{
 init_buffers_();
}
void
Node::init_buffers_()
{
B_.input_buffer_.clear(); // includes resize

}

int main()
{
   auto node = (std::uintptr_t)llvm_omp_target_alloc_shared(sizeof(Node),0);
   //auto node = (char*)llvm_omp_target_alloc_shared(sizeof(Node),0);
   new (reinterpret_cast<Node*>(node)) Node();
   reinterpret_cast<Node*>(node)->init();

#pragma omp target parallel for
  for (int i=0;i < 1;i++) {
    reinterpret_cast<Node*>(node)->set();
#if (!__CUDA_ARCH__) && (___CUDA__)
   const char *name = typeid(xxx).name();
   printf("%s\n", name);
#endif
  }
  //auto p3 = reinterpret_cast<Node*>(p);
  std::cout << "after " << reinterpret_cast<Node*>(node)->get() << std::endl;
  return 0;
}
