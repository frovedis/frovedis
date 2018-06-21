#include <frovedis.hpp>

void mpi_func(std::vector<int>& v) {
  int rank = frovedis::get_selfid();
  //int size = frovedis::get_nodesize();

  int a;
  if(rank == 0) {a = 123;}
  MPI_Bcast(&a, 1, MPI_INT, 0, MPI_COMM_WORLD);
  for(size_t i = 0; i < v.size(); i++) v[i] *= a;
}


int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);
  
  std::vector<int> v;
  for(size_t i = 1; i <= 8; i++) v.push_back(i);
  auto d1 = frovedis::make_dvector_scatter(v);
  d1.mapv_partitions(mpi_func);
  auto r = d1.gather();
  for(auto i: r) std::cout << i << std::endl;
}
