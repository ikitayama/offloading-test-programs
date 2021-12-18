int main() {
  int a[1024*1024];
#pragma omp target parallel for
  for (int i=0;i<1024*1024;i++)
      a[i];


}
