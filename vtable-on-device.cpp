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

class Base {
public:
   virtual void send() = 0;
};

class Derived : public Base {
public:
  void send() { printf("executing send() on device\n"); };
};

int main() {

#pragma omp target
{
  Base *p = new Derived();
  p->send(); // ok as new is called in the target region
}

  auto pBase = (Base*)llvm_omp_target_alloc_shared(sizeof(Derived), 0);
#pragma omp target
  new (pBase) Derived();

#pragma omp target is_device_ptr(pBase)
{
  pBase->send(); // ok pointer pBase can be used both on host and device
}

  auto p2 = (void*)llvm_omp_target_alloc_shared(sizeof(Derived), 0);
#pragma omp target
  new (p2) Derived();

#pragma omp target
{
  reinterpret_cast<Derived*>(p2)->send(); // ok by casting note there's no is_device_prt()

}

  return 0;
}
