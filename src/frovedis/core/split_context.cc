#include "split_context.hpp"
#include "dvector.hpp"

namespace frovedis {

void push_context() {
  frovedis_comm_rpc_stack.push_back(frovedis_comm_rpc);
  frovedis_self_rank_stack.push_back(frovedis_self_rank);
  frovedis_comm_size_stack.push_back(frovedis_comm_size);
  
  frovedis_shm_init_stack.push_back(frovedis_shm_init);
  frovedis_shm_init = false;
  frovedis_shm_comm_stack.push_back(frovedis_shm_comm);
  frovedis_shm_self_rank_stack.push_back(frovedis_shm_self_rank);
  frovedis_shm_comm_size_stack.push_back(frovedis_shm_comm_size);
  frovedis_shmroot_comm_stack.push_back(frovedis_shmroot_comm);
  frovedis_shmroot_self_rank_stack.push_back(frovedis_shmroot_self_rank);
  frovedis_shmroot_comm_size_stack.push_back(frovedis_shmroot_comm_size);

  dvid_table_stack.push_back(dvid_table);
  dvid_table.clear();
  current_dvid_stack.push_back(current_dvid);
  current_dvid = 1;
}

void pop_context() {
  int r = MPI_Comm_free(&frovedis_comm_rpc);
  if(r != 0) throw std::runtime_error("failed to call MPI_Comm_free");
  frovedis_comm_rpc = frovedis_comm_rpc_stack.back();
  frovedis_comm_rpc_stack.pop_back();
  frovedis_self_rank = frovedis_self_rank_stack.back();
  frovedis_self_rank_stack.pop_back();
  frovedis_comm_size = frovedis_comm_size_stack.back();
  frovedis_comm_size_stack.pop_back();
  
  frovedis_shm_init = frovedis_shm_init_stack.back();
  frovedis_shm_init_stack.pop_back();
  frovedis_shm_comm = frovedis_shm_comm_stack.back();
  frovedis_shm_comm_stack.pop_back();
  frovedis_shm_self_rank = frovedis_shm_self_rank_stack.back();
  frovedis_shm_self_rank_stack.pop_back();
  frovedis_shm_comm_size = frovedis_shm_comm_size_stack.back();
  frovedis_shm_comm_size_stack.pop_back();
  frovedis_shmroot_comm = frovedis_shmroot_comm_stack.back();
  frovedis_shmroot_comm_stack.pop_back();
  frovedis_shmroot_self_rank = frovedis_shmroot_self_rank_stack.back();
  frovedis_shmroot_self_rank_stack.pop_back();
  frovedis_shmroot_comm_size = frovedis_shmroot_comm_size_stack.back();
  frovedis_shmroot_comm_size_stack.pop_back();

  dvid_table = dvid_table_stack.back();
  dvid_table_stack.pop_back();
  current_dvid = current_dvid_stack.back();
  current_dvid_stack.pop_back();
}

void init_split_context_execution(int color) {
  push_context();
  int r  = MPI_Comm_split(frovedis_comm_rpc, color, 0, &frovedis_comm_rpc);
  if(r != 0) throw std::runtime_error("failed to call MPI_Comm_split");
  MPI_Comm_rank(frovedis_comm_rpc, &frovedis_self_rank);
  MPI_Comm_size(frovedis_comm_rpc, &frovedis_comm_size);
}

void finalize_split_context_execution() {
  if(frovedis_self_rank == 0) {
    std::vector<std::string> dummy;
    send_bcast_rpcreq(rpc_type::finalize_type, 0, 0, "", dummy);
  }
  pop_context();
}

std::vector<int> get_split_rank_stack() {
  return frovedis_self_rank_stack;
}

void split_context_execution(const std::vector<int>& color, void(*fp)()) {
  split_context_execution(color, make_serfunc(fp));
}

void split_context_execution(void(*fp)()) {
  split_context_execution(make_serfunc(fp));
}

std::vector<int> make_color(int size) {
  auto node_size = get_nodesize();
  if(node_size < size)
    throw std::runtime_error("split size is larger than node size");
  auto each = node_size / size;
  std::vector<int> color(node_size);
  auto colorp = color.data();
  auto crnt_color = 0;
  for(int i = 0; i < size; i++) {
    for(int j = 0; j < each; j++) {
      colorp[i * each + j] = crnt_color;
    }
    crnt_color++;
  }
  crnt_color--;
  for(int i = size * each; i < node_size; i++) {
    colorp[i] = crnt_color;
  }
  return color;
}

}
