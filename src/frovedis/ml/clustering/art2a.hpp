#ifndef _ART2A_HPP_
#define _ART2A_HPP_

#include <cassert>

#include "../../core/log.hpp"
#include "../../matrix/rowmajor_matrix.hpp"


namespace frovedis {

template <typename V>
using art2a_result = std::tuple<rowmajor_matrix<V>, std::vector<size_t>>;

template <typename V>
void normalize_byrow(rowmajor_matrix<V>& mat) {
  mat.data.mapv(
    +[](rowmajor_matrix_local<V> lmat) {
      size_t nr = lmat.local_num_row;
      size_t nc = lmat.local_num_col;
      for (size_t ir = 0; ir < nr; ir++) {
        V s = 0;
        for (size_t ic = 0; ic < nc; ic++) s += lmat.val[ic + ir * nc] * lmat.val[ic + ir * nc]; 
        V sq = std::sqrt(s);
        for (size_t ic = 0; ic < nc; ic++) lmat.val[ic + ir * nc] /= sq;
      }
    }
  );
}

template <typename V>
art2a_result<V> art2a(
    rowmajor_matrix<V>& data0, const size_t weight_size,
    const size_t max_iter, const bool random_shuffle, const size_t random_seed,  
    const V learning_rate, const V vigilance, const bool data_inplace = true
) {
  assert(weight_size > 0);

  srand(random_seed + 1);
  const size_t dim = data0.num_col;

  rowmajor_matrix_local<V> weight0_loc(weight_size, dim);
  for (size_t j = 0; j < weight_size * dim; j++) {
    // Uniform distribution. This can be other distribution
    weight0_loc.val[j] = 2.0 * rand() / RAND_MAX - 1.0;
  }

  size_t node_size = get_nodesize();
  std::vector<size_t> num_rows(node_size);
  for (size_t node = 0; node < node_size; node++) {
    size_t len = weight_size / node_size;
    size_t rem = weight_size % node_size;
    num_rows[node] = len + size_t(node < rem);
  }
  rowmajor_matrix<V> weight0 = make_rowmajor_matrix_scatter(weight0_loc, num_rows);

  return art2a(data0, weight0, max_iter, random_shuffle, random_seed, 
               learning_rate, vigilance, data_inplace);
}

template <typename V>
art2a_result<V> art2a(
    rowmajor_matrix<V>& data0, const rowmajor_matrix<V>& weight0,  
    const size_t max_iter, const bool random_shuffle, const size_t random_seed,  
    const V learning_rate, const V vigilance, const bool data_inplace = true
) {
  /*
    Performs clustering data with art2a method and returns the result.
    
    `data0` is input data, in which each row is one sample. 
    `random_shuffle` controls shuffling order of point selection in clutering. If `random_shuffle` is false, `random_seed` is ignored.
    `learning_rate` and `vigilance` are parameters for art2a model.
    `data_inplace` is true, preprocess to `data0` is applied in-place. If you want to keep it constant, set `data_inplace` false.

    Return value `art2a_result` is tuple of two objects.
    First object is weights of cluster, which grows from `weight0`.
    Second object is cluster id assignment to each data sample. Cluster id is same to row index of cluster weight.
  */

  const size_t part_size = data0.num_row;
  const size_t dim = data0.num_col;
  LOG(DEBUG) << "data size = " << part_size << std::endl;
  LOG(DEBUG) << "dimension = " << dim << std::endl;
  
  if (dim != weight0.num_col) {
    throw std::runtime_error("Dimensions of data and weight are mismatched");
  }
  if (weight0.num_row == 0 || weight0.num_col == 0) {
    throw std::runtime_error("Dimension of weight should have non-zero size");
  }
  if (learning_rate <= 0 || 1 <= learning_rate) {
    throw std::runtime_error("Learning rate should be in exclusive range from 0 to 1");
  }
  assert(max_iter > 0);

  rowmajor_matrix<V> data;
  // Preprocess input data inplace or not
  if (data_inplace) {
    data = rowmajor_matrix<V>(std::move(data0.data));
  } else {
    data = rowmajor_matrix<V>(data0);
  }
  rowmajor_matrix<V> weight(weight0);

  normalize_byrow(data);
  normalize_byrow(weight);  

  // random permutation
  std::vector<size_t> perm_index(part_size);
  for (size_t i = 0; i < part_size; i++)  perm_index[i] = i;
  srand(random_seed);
    
  for (size_t iter = 0; iter < max_iter; iter++) {
    // LOG(DEBUG) << "iter: " << iter << std::endl;
   
    if (random_shuffle) {
      std::random_shuffle(perm_index.begin(), perm_index.end());
    }
    
    for (size_t part_i0 = 0; part_i0 < part_size; part_i0++) {
      size_t part_i = perm_index[part_i0];

      std::vector<V> part_row = data.get_row(part_i);
      auto part_bc_row = broadcast(part_row);

      auto activation = weight.data.map(
        +[](const rowmajor_matrix_local<V>& wt, const std::vector<V>& pv) {
          return wt * pv;
        },
        part_bc_row
      ).template moveto_dvector<V>().gather();

      V proximity = std::numeric_limits<V>::lowest();
      size_t target_ri = activation.size();
      for (size_t c_i = 0; c_i < activation.size(); c_i++) {
        if (proximity < activation[c_i]) {
          proximity = activation[c_i];
          target_ri = c_i;
        }
      }
      assert(target_ri < activation.size());

      if (proximity >= vigilance) {
        // update weight at the most proximal to the point
        auto num_rows = weight.get_local_num_rows();
        assert(num_rows.size() == get_nodesize());

        size_t off = 0;
        size_t target_node;
        for (size_t node = 0; node < num_rows.size(); node++) {
          size_t nr = num_rows[node];
          if (target_ri < off + nr) {
            target_node = node;
            break;
          }
          off += nr;
        }
        assert(target_node < weight.num_row);

        // modify weight at the node
        size_t target_node_ri = target_ri - off;
        weight.data.mapv(+[](
          rowmajor_matrix_local<V>& wt, size_t node, size_t ri, 
          const V learning_rate, std::vector<V>& part_row) {
            if (get_selfid() != node) return; 

            size_t nc = wt.local_num_col;
            assert(nc == part_row.size());
            V s = 0;
            for (size_t j = 0; j < nc; j++) {
              wt.val[j + ri * nc] = wt.val[j + ri * nc] * (1 - learning_rate) + part_row[j] * learning_rate;
              s += wt.val[j + ri * nc] * wt.val[j + ri * nc];
            }
            V sq = std::sqrt(s);
            for (size_t j = 0; j < nc; j++)  wt.val[j + ri * nc] /= sq;
          }, 
          broadcast(target_node), broadcast(target_node_ri), broadcast(learning_rate), part_bc_row
        );

      } else {
        // create new cluster
        auto num_rows = weight.get_local_num_rows();
        size_t min_num_row = std::numeric_limits<size_t>::max();
        size_t min_node;
        for (size_t node = 0; node < num_rows.size(); node++) {
          if (num_rows[node] < min_num_row) {
            min_num_row = num_rows[node];
            min_node = node;
          }
        }

        weight.data.mapv(+[](
          rowmajor_matrix_local<V>& wt, size_t node, std::vector<V>& part_row) {
            if (get_selfid() != node) return; 
            size_t nr = wt.local_num_row;
            size_t nc = wt.local_num_col;
            assert(nc == part_row.size());

            // append data row to node with minimal number of weight
            wt.val.resize((nr + 1) * nc);
            wt.set_local_num(nr + 1, nc);
            for (size_t j = 0; j < nc; j++)  wt.val[j + nr * nc] = part_row[j];
          }, 
          broadcast(min_node), part_bc_row
        );

        weight.set_num(weight.num_row + 1, weight.num_col);
      }

    }  // loop for part_i0
  }  // loop for iter

  // Assign cluster after fixed
  std::vector<size_t> part_cluster(part_size);

  for (size_t part_i = 0; part_i < part_size; part_i++) {
    auto activation = weight.data.map(
      +[](const rowmajor_matrix_local<V>& wt, const std::vector<V>& pv) {
        return wt * pv;
      },
      broadcast(data.get_row(part_i))
    ).template moveto_dvector<V>().gather();
    assert(activation.size() == weight.num_row);

    V proximity = std::numeric_limits<V>::lowest();
    size_t target_ri = activation.size();
    for (size_t c_i = 0; c_i < activation.size(); c_i++) {
      if (proximity < activation[c_i]) {
        proximity = activation[c_i];
        target_ri = c_i;
      }
    }
    assert(target_ri < activation.size());
    part_cluster[part_i] = target_ri;
  }
  
  return std::make_tuple(weight, part_cluster);
}

}  // namespace frovedis

#endif  // _ART2A_HPP_

