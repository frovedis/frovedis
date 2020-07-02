#ifndef _BATCH_CHAIN_COMMUNICATOR_
#define _BATCH_CHAIN_COMMUNICATOR_

#include <frovedis.hpp>
#include <frovedis/core/serializehelper.hpp>
#include <frovedis/core/exceptions.hpp>
#include <frovedis/matrix/crs_matrix.hpp>
#include <frovedis/dataframe/hashtable.hpp>  // used to replace std::set

#include <cassert>
#include <set>


namespace frovedis {

template <class I>
struct index_suite {
  index_suite() {}
  index_suite(size_t partition_size):
    counts(partition_size), displs(partition_size) {}
  index_suite(size_t buf_size, size_t partition_size):
    buf(buf_size), counts(partition_size), displs(partition_size) {}
  index_suite(std::vector<I>& buf, size_t partition_size):
    buf(buf), counts(partition_size), displs(partition_size) {}
  index_suite(std::vector<I>& buf, std::vector<size_t>& managed_borders):
    buf(buf), counts(managed_borders.size() - 1), displs(managed_borders.size() - 1) {
      size_t nodesize = get_nodesize();
      //assert(managed_borders.size() - 1 == nodesize && managed_borders[0] == 0);

      displs[0] = 0;
      for (size_t i_node = 1; i_node < nodesize; i_node++) {
        auto value = managed_borders[i_node];
        auto it = std::lower_bound(buf.begin(), buf.end(), value);
        displs[i_node] = it - buf.begin();
        counts[i_node-1] = displs[i_node] - displs[i_node-1];
      }
      counts[nodesize - 1] = buf.size() - displs[nodesize - 1];
    }
    
  std::vector<I> buf;
  std::vector<size_t> counts;
  std::vector<size_t> displs;
  SERIALIZE(buf, counts, displs)
};  // struct index_suite

template <class I>
std::ostream& operator<<(std::ostream& str, index_suite<I>& suite) {
  str << "[buf] ";
  for (size_t i = 0; i < suite.buf.size(); i++) str << suite.buf[i] << " ";
  str << "[displs] ";
  for (size_t i = 0; i < suite.displs.size(); i++) str << suite.displs[i] << " ";
  str << "[counts] ";
  for (size_t i = 0; i < suite.counts.size(); i++) str << suite.counts[i] << " ";
  return str;
}


template <class I>
struct chain_schedule {
  chain_schedule() {}
  chain_schedule(size_t batch_count):
    computed_index_suites(batch_count), gathered_index_suites(batch_count), gathered_index_unions(batch_count) {}
  chain_schedule(size_t batch_count, std::vector<size_t>& managed_borders):
    computed_index_suites(batch_count), gathered_index_suites(batch_count), gathered_index_unions(batch_count) {
      // managed_borders has 0 as first element, feature size as last element.
      size_t nodesize = get_nodesize();
      //assert(nodesize == managed_borders.size() - 1);
      
      managed_displs = std::vector<size_t>(nodesize);
      managed_counts = std::vector<size_t>(nodesize);
      for (size_t i = 0; i < nodesize; i++) {
        managed_displs[i] = managed_borders[i];
        managed_counts[i] = managed_borders[i+1] - managed_borders[i];
      }
    }
  
  std::vector<index_suite<I>> computed_index_suites;
  std::vector<index_suite<I>> gathered_index_suites; 
  std::vector<std::vector<I>> gathered_index_unions;
  std::vector<size_t> managed_displs;
  std::vector<size_t> managed_counts;

  SERIALIZE(computed_index_suites, gathered_index_suites, gathered_index_unions, managed_displs, managed_counts)
};  // struct chain_schedule

template <class I>
std::ostream& operator<<(std::ostream& str, chain_schedule<I>& sched) {
  str << "computed_index_suites:" << std::endl;
  for (size_t i = 0; i < sched.computed_index_suites.size(); i++) {
    if (i >= 10) break;
    str << i << ": " << sched.computed_index_suites[i] << std::endl;
  }
  str << "gathered_index_suites:" << std::endl;
  for (size_t i = 0; i < sched.gathered_index_suites.size(); i++) {
    if (i >= 10) break;
    str << i << ": " << sched.gathered_index_suites[i] << std::endl;
  }
  str << "managed_displs:" << std::endl;
  for (size_t i = 0; i < sched.managed_displs.size(); i++) {
    str << sched.managed_displs[i] << " ";
  }
  str << std::endl;
  str << "managed_counts:" << std::endl;
  for (size_t i = 0; i < sched.managed_counts.size(); i++) {
    str << sched.managed_counts[i] << " ";
  }
  str << std::endl;
  return str;
}


template <class T, class I, class O>
void assume_equal_count_of_batches(
  node_local<std::vector<crs_matrix_local<T,I,O>>>& nl_batches
) {
  std::vector<size_t> counts = nl_batches.map(
    +[](std::vector<crs_matrix_local<T,I,O>>& v){
      return v.size();
    }  
  ).gather();

  size_t c = counts[0];
  for (size_t i = 0; i < counts.size(); i++) {
    if (c != counts[i]) {
      REPORT_ERROR(USER_ERROR, "All workers should have equal number of mini batches.");
    }
  }
}

template <class T, class I, class O> 
node_local<std::vector<std::vector<I>>> 
extract_unique_index_in_each(
  node_local<std::vector<crs_matrix_local<T,I,O>>>& nl_batches
){    
  auto func = +[](std::vector<crs_matrix_local<T,I,O>>& batches) {
    std::vector<std::vector<I>> ret(batches.size());
    for (size_t i = 0; i < batches.size(); i++) {
      auto &mat = batches[i];

#if 0
      std::set<I> tmp_set(mat.idx.begin(), mat.idx.end());
      ret[i] = std::vector<I>(tmp_set.begin(), tmp_set.end());
#endif
      using dummy_t = float;
      unique_hashtable<I,dummy_t> tmp_ht(mat.idx);
      auto uq_idx = tmp_ht.all_keys(); 
      std::vector<dummy_t> dummy_val(uq_idx.size(), 0);
      radix_sort(uq_idx, dummy_val);
      ret[i] = std::move(uq_idx);

    }
    return ret;
  };  
  return nl_batches.map(func);
}

template <class I>
std::vector<size_t> count_unique_index_thru(
    node_local<std::vector<std::vector<I>>>& nl_unique_index_chain,
    size_t dim
){
  auto nl_dim = broadcast(dim);
    
  auto func_count_local = +[](std::vector<std::vector<I>>& unique_index_chain, size_t dim) {
    std::vector<size_t> idx_count(dim);
    auto* ptr_idx_count = idx_count.data();
    size_t batch_count = unique_index_chain.size();
    for (size_t ib = 0; ib < batch_count; ib++) {
      auto& unique_index = unique_index_chain[ib];
      auto* ptr_unique_index = unique_index.data();
      size_t ui_count = unique_index.size();
      for (size_t iu = 0; iu < ui_count; iu++) {
        ptr_idx_count[ptr_unique_index[iu]] += 1;
      }      
    } 
    return idx_count;
  };
  
  auto nl_count = nl_unique_index_chain.map(func_count_local, nl_dim);

  return nl_count.vector_sum();
}

std::vector<size_t> fix_managed_parameter_range(std::vector<size_t>& count);

template <class I> 
index_suite<I> make_index_suite_by_exchange(index_suite<I>& one_suite)
{
  size_t nodesize = get_nodesize();
  
  index_suite<I> another_suite(nodesize);
  MPI_Alltoall(one_suite.counts.data(), sizeof(size_t), MPI_CHAR,
               another_suite.counts.data(), sizeof(size_t), MPI_CHAR, frovedis_comm_rpc);
  size_t another_bufsize = another_suite.counts[0];
  another_suite.displs[0] = 0;
  for (size_t i_node = 1; i_node < nodesize; i_node++) {
    another_suite.displs[i_node] = another_suite.displs[i_node - 1] + another_suite.counts[i_node - 1];
    another_bufsize += another_suite.counts[i_node];
  }
  
  another_suite.buf.resize(another_bufsize);
  large_alltoallv(sizeof(I), 
                  reinterpret_cast<char*>(one_suite.buf.data()), one_suite.counts, one_suite.displs,
                  reinterpret_cast<char*>(another_suite.buf.data()), another_suite.counts, another_suite.displs,
                  frovedis_comm_rpc);

  return another_suite;    
}


template <class I> 
node_local<chain_schedule<I>>
initialize_chain_schedule(node_local<std::vector<std::vector<I>>>& nl_unique_index_chain,
                          std::vector<size_t>& managed_borders)
{
  auto nl_managed_borders = broadcast(managed_borders);

  auto func = +[](std::vector<std::vector<I>>& unique_index_chain, std::vector<size_t>& managed_borders) {
    size_t selfid = get_selfid();
    auto batch_count = unique_index_chain.size();
    //assert(managed_borders.size() - 1 == nodesize);

    chain_schedule<I> sched(batch_count, managed_borders);
        
    for (size_t i_batch = 0; i_batch < batch_count; i_batch++) {
      auto& unique_index = unique_index_chain[i_batch]; 
      index_suite<I> computed_suite(unique_index, managed_borders);
      
      auto gathered_suite = make_index_suite_by_exchange(computed_suite);
      // subtract range start 
      auto* ptr_gathered_suite = gathered_suite.buf.data();
      auto managed_displ = managed_borders[selfid];
      for (size_t pos = 0; pos < gathered_suite.buf.size(); pos++) {
        ptr_gathered_suite[pos] -= managed_displ;
      }
  
#if 0
      std::set<I> tmp_set(gathered_suite.buf.begin(), gathered_suite.buf.end());
      auto gathered_union = std::vector<I>(tmp_set.begin(), tmp_set.end());
#endif
      using dummy_t = float;
      unique_hashtable<I,dummy_t> tmp_ht(gathered_suite.buf);
      auto uq_idx = tmp_ht.all_keys(); 
      std::vector<dummy_t> dummy_val(uq_idx.size(), 0);
      radix_sort(uq_idx, dummy_val);
      std::vector<I> gathered_union(std::move(uq_idx));

      sched.computed_index_suites[i_batch] = computed_suite;
      sched.gathered_index_suites[i_batch] = gathered_suite;
      sched.gathered_index_unions[i_batch] = gathered_union;
    }
    
    return sched;
  };
  
  return nl_unique_index_chain.map(func, nl_managed_borders);
}


template <class T, class I = size_t, class O = size_t>
node_local<chain_schedule<I>>
create_schedule_from_batches(node_local<std::vector<crs_matrix_local<T,I,O>>>& nl_batches, 
                             size_t block = 1) {
  LOG(DEBUG) << "To create chain schedule from batches." << std::endl;
  
  assume_equal_count_of_batches(nl_batches);
  LOG(DEBUG) << "Assume all workers have same number of batches" << std::endl;
  
  size_t dim = nl_batches.map(
    +[](std::vector<crs_matrix_local<T,I,O>>& v){return v[0].local_num_col; }
  ).get(0);
  LOG(DEBUG) << "Fetched data dimension." << std::endl;

  auto nl_unique_index_chain = extract_unique_index_in_each(nl_batches);
  LOG(DEBUG) << "Extracted unique indices in batches." << std::endl;
  
  auto count_thru = count_unique_index_thru(nl_unique_index_chain, dim);
  LOG(DEBUG) << "Counted unique index through batches" << std::endl;
  
  auto managed_borders = fix_managed_parameter_range(count_thru);  // return object size is nodesize + 1
  LOG(DEBUG) << "Fixed range of managed parameters" << std::endl;
    
  auto nl_schedule = initialize_chain_schedule(nl_unique_index_chain, managed_borders);
  LOG(DEBUG) << "Initialized batch chained schedule" << std::endl;
  
  return nl_schedule;
}

// 
// Function to commucate and update managed parameter based on prepared schedule
// Make sure to use in map/mapv context
//

template <class T, class I>
void aggregate_to_managing_node(chain_schedule<I>& sched, size_t batch_id, 
                                std::vector<T>& local_grad, std::vector<T>& managed_grad_accumulator, 
                                size_t block = 1) 
{ 
  auto& computed_suite = sched.computed_index_suites[batch_id];
  auto& gathered_suite = sched.gathered_index_suites[batch_id];
  auto& gathered_union = sched.gathered_index_unions[batch_id];

  //assert(block * computed_suite.buf.size() == local_grad.size());
  //assert(block * sched.managed_counts[selfid] == managed_grad_accumulator.size());
  auto* ptr_managed_grad_accumulator = managed_grad_accumulator.data();

  std::vector<T> gathered_buffer(block * gathered_suite.buf.size());
  large_alltoallv(sizeof(T) * block,
                  reinterpret_cast<char*>(local_grad.data()), computed_suite.counts, computed_suite.displs,
                  reinterpret_cast<char*>(gathered_buffer.data()), gathered_suite.counts, gathered_suite.displs,  
                  frovedis_comm_rpc);
  auto* ptr_gathered_buffer = gathered_buffer.data();

  // clear accumulater in relevant position
  auto* ptr_gathered_union = gathered_union.data();
  auto gathered_union_count = gathered_union.size();

#if 0
  for (size_t pos = 0; pos < gathered_union_count; pos++) {
    auto local_id = ptr_gathered_union[pos];
    for (size_t off = 0; off < block; off++) {
      ptr_managed_grad_accumulator[block * local_id + off] = 0;
    }
  }
#endif
  for (size_t pos_off = 0; pos_off < gathered_union_count * block; pos_off++) {
    size_t pos = pos_off / block;
    size_t off = pos_off - pos * block;
    auto local_id = ptr_gathered_union[pos];
    ptr_managed_grad_accumulator[block * local_id + off] = 0;
  }

  auto* ptr_gathered_suite = gathered_suite.buf.data();
#pragma cdir novector
#pragma _NEC novector
  for (size_t i_node = 0; i_node < get_nodesize(); i_node++) {
    auto displ = gathered_suite.displs[i_node];
    auto count = gathered_suite.counts[i_node];

#if 0
#pragma cdir nodep
#pragma _NEC ivdep
    for (size_t pos = displ; pos < displ + count; pos++) {
      auto local_id = ptr_gathered_suite[pos];
      for (size_t off = 0; off < block; off++) {
        ptr_managed_grad_accumulator[block * local_id + off] += \
          ptr_gathered_buffer[block * pos + off];
      }
    }
#endif
    size_t start = displ * block;
    size_t end = (displ + count) * block;
#pragma cdir nodep
#pragma _NEC ivdep
    for (size_t pos_off = start; pos_off < end; pos_off++) {
      size_t pos = pos_off / block;
      size_t off = pos_off - pos * block;
      auto local_id = ptr_gathered_suite[pos];
      ptr_managed_grad_accumulator[block * local_id + off] +=	\
	ptr_gathered_buffer[pos_off];
    }
  }

}


template <class T, class I>
void update_managed_parameter(chain_schedule<I>& sched, size_t batch_id,
                              std::vector<T>& managed_parameter, std::vector<T>& managed_grad_accumulator,
                              T learn_rate, T regular_2, T regular_1,
                              size_t block = 1) 
{
  auto& gathered_union = sched.gathered_index_unions[batch_id];

  //assert(block * sched.managed_counts[selfid] == managed_parameter.size());
  //assert(block * sched.managed_counts[selfid] == managed_grad_accumulator.size());

  auto gathered_union_count = gathered_union.size();
  auto* ptr_gathered_union = gathered_union.data();
  auto* ptr_managed_parameter = managed_parameter.data();
  auto* ptr_managed_grad_accumulator = managed_grad_accumulator.data();
  
#pragma cdir nodep
#pragma _NEC ivdep
  for (size_t pos = 0; pos < gathered_union_count; pos++) {
    auto local_id = ptr_gathered_union[pos];
    for (size_t off = 0; off < block; off++) {
      auto t_grad = ptr_managed_grad_accumulator[local_id * block + off];
      auto t_reg2 = 2 * regular_2 * ptr_managed_parameter[local_id * block + off];
      auto t_reg1 = regular_1 * (2.0 * (ptr_managed_parameter[local_id * block + off] > 0) - 1.0);
      ptr_managed_parameter[local_id * block + off] -= learn_rate * (t_grad + t_reg2 + t_reg1);
    }
  }
}


template <class T, class I>
std::vector<T>
broadcast_from_managing_node(chain_schedule<I>& sched, size_t batch_id,
                             std::vector<T>& managed_parameter, size_t block = 1) 
{
  auto& computed_suite = sched.computed_index_suites[batch_id];
  auto& gathered_suite = sched.gathered_index_suites[batch_id];

  //assert(block * sched.managed_counts[selfid] == managed_parameter.size());

  std::vector<T> param_buffer(block * gathered_suite.buf.size());
  auto* ptr_param_buffer = param_buffer.data();
  
  auto* ptr_gathered_suite = gathered_suite.buf.data();
  auto* ptr_managed_parameter = managed_parameter.data();
  for (size_t i_node = 0; i_node < get_nodesize(); i_node++) {
    size_t count = gathered_suite.counts[i_node];
    size_t displ = gathered_suite.displs[i_node];

#if 0
#pragma cdir nodep
#pragma _NEC ivdep
    for (size_t pos = displ; pos < displ + count; pos++) {
      auto local_id = ptr_gathered_suite[pos];
      for (size_t off = 0; off < block; off++) {
        ptr_param_buffer[pos * block + off] = ptr_managed_parameter[local_id * block + off];
      }
    }
#endif
    size_t start = displ * block;
    size_t end = (displ + count) * block;
    for (size_t pos_off = start; pos_off < end; pos_off++) {
      size_t pos = pos_off / block;
      size_t off = pos_off - pos * block;
      auto local_id = ptr_gathered_suite[pos];
      ptr_param_buffer[pos * block + off] = ptr_managed_parameter[local_id * block + off];
    }

  }

  std::vector<T> computed_parameter(block * computed_suite.buf.size());
  large_alltoallv(sizeof(T) * block,
                  reinterpret_cast<char*>(param_buffer.data()), gathered_suite.counts, gathered_suite.displs,  
                  reinterpret_cast<char*>(computed_parameter.data()), computed_suite.counts, computed_suite.displs,
                  frovedis_comm_rpc);
  
  return computed_parameter;
}
  
}  // namespace frovedis
#endif  // _BATCH_CHAIN_COMMUNICATOR_
