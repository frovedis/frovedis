#include "dfcolumn.hpp"

namespace frovedis {

using namespace std;

vector<compressed_words>
compressed_words_align_as_helper(compressed_words& cws,
                                 vector<size_t>& alltoall_sizes) {
  auto nodesize = alltoall_sizes.size();
  vector<compressed_words> ret(nodesize);
  size_t current = 0;
  for(size_t i = 0; i < nodesize; i++) {
    size_t extract_size = alltoall_sizes[i];
    vector<size_t> to_extract(extract_size);
    auto to_extractp = to_extract.data();
    for(size_t j = 0; j < extract_size; j++) {
      to_extractp[j] = current + j;
    }
    ret[i] = cws.extract(to_extract);
    current += extract_size;
  }
  return ret;
}

void compressed_words_align_as(node_local<compressed_words>& comp_words,
                               const vector<size_t>& mysizes,
                               const vector<size_t>& dst) {
  if(dst.size() != get_nodesize()) 
    throw std::runtime_error
      ("align_as: size of dst is not equal to node size");
  size_t dsttotal = 0;
  size_t selftotal = 0;
  for(size_t i = 0; i < dst.size(); i++) dsttotal += dst[i];
  for(size_t i = 0; i < mysizes.size(); i++) selftotal += mysizes[i];
  if(dsttotal != selftotal)
    throw std::runtime_error
      ("align_as: total size of src and dst does not match");
  bool is_same = true;
  for(size_t i = 0; i < dst.size(); i++) {
    if(dst[i] != mysizes[i]) {
      is_same = false;
      break;
    }
  }
  if(is_same) return;
  // align_as_calc_alltoall_sizes is in dvector.hpp
  auto alltoall_sizes = broadcast(mysizes).map(align_as_calc_alltoall_sizes,
                                               broadcast(dst));
  auto comp_words_toex = comp_words.map(compressed_words_align_as_helper,
                                        alltoall_sizes);
  comp_words.mapv(+[](compressed_words& cw){cw.clear();});
  auto comp_words_exchanged = alltoall_exchange(comp_words_toex);
  comp_words_toex.mapv(+[](vector<compressed_words>& vcw)
                       {for(auto& cw: vcw) cw.clear();});
  comp_words = comp_words_exchanged.map(merge_multi_compressed_words);
}

void typed_dfcolumn<raw_string>::align_as(const vector<size_t>& dst) {
  // overwrite
  compressed_words_align_as(comp_words, sizes(), dst);
}


void typed_dfcolumn<raw_string>::init(node_local<words>& ws,
                                      bool allocate_nulls) {
  auto cws = ws.map(make_compressed_words);
  init_compressed(std::move(cws), allocate_nulls);
}

void typed_dfcolumn<raw_string>::init_compressed
(node_local<compressed_words>&& cws, bool allocate_nulls) {
  if(allocate_nulls) nulls = make_node_local_allocate<vector<size_t>>();
  comp_words = std::move(cws);
}

void typed_dfcolumn<raw_string>::init_compressed
(node_local<compressed_words>& cws, bool allocate_nulls) {
  if(allocate_nulls) nulls = make_node_local_allocate<vector<size_t>>();
  comp_words = cws;
}


template <>
std::shared_ptr<dfcolumn>
create_null_column<raw_string>(const std::vector<size_t>& sizes) {
  auto nlsizes = make_node_local_scatter(sizes);
  auto nulls = make_node_local_allocate<std::vector<size_t>>();
  auto ws = nlsizes.map(+[](size_t size, std::vector<size_t>& nulls) {
      nulls.resize(size);
      auto nullsp = nulls.data();
      for(size_t i = 0; i < size; i++) nullsp[i] = i;
      words ws;
      ws.chars = char_to_int("NULL");
      ws.starts.resize(size);
      ws.lens.resize(size);
      auto startsp = ws.starts.data();
      auto lensp = ws.lens.data();
      for(size_t i = 0; i < size; i++) {
        startsp[i] = 0;
        lensp[i] = 4;
      }
      return ws;
    }, nulls);
  return std::make_shared<typed_dfcolumn<raw_string>>
    (std::move(ws), std::move(nulls));
}

}
