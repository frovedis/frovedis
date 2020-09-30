#include "find.hpp"
#include <stdexcept>

using namespace std;

namespace frovedis {

void find_one_helper(const int* vp,
                     size_t* retidx,
                     size_t size,
                     size_t* bufidx,
                     size_t& nextsize,
                     int to_find) {
  if(size == 0) {
    nextsize = 0;
    return;
  }
  size_t each = size / FIND_VLEN; // maybe 0
  auto aligned_each = each * sizeof(int) / FIND_ALIGN_SIZE;
  if(aligned_each % 2 == 0 && aligned_each != 0)
    aligned_each -= 1;
  each = aligned_each * FIND_ALIGN_SIZE / sizeof(int);
  size_t rest = size - each * FIND_VLEN;

  if(each == 0) {
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(vp[i] == to_find) {
        bufidx[current++] = i;
      }
    }
    nextsize = current;
    for(size_t i = 0; i < current; i++) {
      retidx[i] = bufidx[i];
    }
  } else {
    size_t out_ridx[FIND_VLEN];
    size_t out_ridx_0[FIND_VLEN_EACH];
    size_t out_ridx_1[FIND_VLEN_EACH];
    size_t out_ridx_2[FIND_VLEN_EACH];
    size_t out_ridx_3[FIND_VLEN_EACH];
// never remove this vreg! this is needed following vovertake
#pragma _NEC vreg(out_ridx_0)
#pragma _NEC vreg(out_ridx_1)
#pragma _NEC vreg(out_ridx_2)
#pragma _NEC vreg(out_ridx_3)
    for(size_t i = 0; i < FIND_VLEN_EACH; i++) {
      auto v0 = i;
      auto v1 = i + FIND_VLEN_EACH;
      auto v2 = i + FIND_VLEN_EACH * 2;
      auto v3 = i + FIND_VLEN_EACH * 3;
      out_ridx_0[i] = each * v0;
      out_ridx_1[i] = each * v1;
      out_ridx_2[i] = each * v2;
      out_ridx_3[i] = each * v3;
    }
#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t i = 0; i < FIND_VLEN_EACH; i++) {
        auto idx0 = j + each * i;
        auto idx1 = j + each * (i + FIND_VLEN_EACH);
        auto idx2 = j + each * (i + FIND_VLEN_EACH * 2);
        auto idx3 = j + each * (i + FIND_VLEN_EACH * 3);
        auto loaded_v0 = vp[idx0];
        auto loaded_v1 = vp[idx1];
        auto loaded_v2 = vp[idx2];
        auto loaded_v3 = vp[idx3];
        if(loaded_v0 == to_find) {
          bufidx[out_ridx_0[i]++] = idx0;
        }
        if(loaded_v1 == to_find) {
          bufidx[out_ridx_1[i]++] = idx1;
        }
        if(loaded_v2 == to_find) {
          bufidx[out_ridx_2[i]++] = idx2;
        }
        if(loaded_v3 == to_find) {
          bufidx[out_ridx_3[i]++] = idx3;
        }
      }
    }
    size_t rest_idx_start = each * FIND_VLEN;
    size_t rest_idx = rest_idx_start;
    for(size_t j = 0; j < rest; j++) {
      auto loaded_v = vp[j + rest_idx_start];
      if(loaded_v == to_find) {
        bufidx[rest_idx++] = j + rest_idx_start;
      }
    }
    for(size_t i = 0; i < FIND_VLEN_EACH; i++) {
      auto v0 = i;
      auto v1 = i + FIND_VLEN_EACH;
      auto v2 = i + FIND_VLEN_EACH * 2;
      auto v3 = i + FIND_VLEN_EACH * 3;
      out_ridx[v0] = out_ridx_0[i];
      out_ridx[v1] = out_ridx_1[i];
      out_ridx[v2] = out_ridx_2[i];
      out_ridx[v3] = out_ridx_3[i];
    }
    size_t sizes[FIND_VLEN];
    for(size_t i = 0; i < FIND_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < FIND_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - rest_idx_start;
    total += rest_size;
    nextsize = total;
    size_t current = 0;
    for(size_t i = 0; i < FIND_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        retidx[current + j] = bufidx[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      retidx[current + j] = bufidx[rest_idx_start + j];
    }
  }
}

void find_helper(const int* vp,
                 size_t* crntidx,
                 size_t size,
                 size_t* bufidx,
                 size_t& nextsize,
                 int to_find,
                 size_t off) {
  if(size == 0) {
    nextsize = 0;
    return;
  }
  size_t each = size / FIND_VLEN; // maybe 0
  auto aligned_each = each * sizeof(int) / FIND_ALIGN_SIZE;
  if(aligned_each % 2 == 0 && aligned_each != 0)
    aligned_each -= 1;
  each = aligned_each * FIND_ALIGN_SIZE / sizeof(int);
  size_t rest = size - each * FIND_VLEN;

  if(each == 0) {
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(vp[crntidx[i]+off] == to_find) {
        bufidx[current++] = crntidx[i];
      }
    }
    nextsize = current;
    for(size_t i = 0; i < current; i++) {
      crntidx[i] = bufidx[i];
    }
  } else {
    size_t out_ridx[FIND_VLEN];
    size_t out_ridx_0[FIND_VLEN_EACH];
    size_t out_ridx_1[FIND_VLEN_EACH];
    size_t out_ridx_2[FIND_VLEN_EACH];
    size_t out_ridx_3[FIND_VLEN_EACH];
// never remove this vreg! this is needed following vovertake
#pragma _NEC vreg(out_ridx_0)
#pragma _NEC vreg(out_ridx_1)
#pragma _NEC vreg(out_ridx_2)
#pragma _NEC vreg(out_ridx_3)
    for(size_t i = 0; i < FIND_VLEN_EACH; i++) {
      auto v0 = i;
      auto v1 = i + FIND_VLEN_EACH;
      auto v2 = i + FIND_VLEN_EACH * 2;
      auto v3 = i + FIND_VLEN_EACH * 3;
      out_ridx_0[i] = each * v0;
      out_ridx_1[i] = each * v1;
      out_ridx_2[i] = each * v2;
      out_ridx_3[i] = each * v3;
    }
#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t i = 0; i < FIND_VLEN_EACH; i++) {
        auto idx0 = crntidx[j + each * i];
        auto idx1 = crntidx[j + each * (i + FIND_VLEN_EACH)];
        auto idx2 = crntidx[j + each * (i + FIND_VLEN_EACH * 2)];
        auto idx3 = crntidx[j + each * (i + FIND_VLEN_EACH * 3)];
        auto loaded_v0 = vp[idx0+off];
        auto loaded_v1 = vp[idx1+off];
        auto loaded_v2 = vp[idx2+off];
        auto loaded_v3 = vp[idx3+off];
        if(loaded_v0 == to_find) {
          bufidx[out_ridx_0[i]++] = idx0;
        }
        if(loaded_v1 == to_find) {
          bufidx[out_ridx_1[i]++] = idx1;
        }
        if(loaded_v2 == to_find) {
          bufidx[out_ridx_2[i]++] = idx2;
        }
        if(loaded_v3 == to_find) {
          bufidx[out_ridx_3[i]++] = idx3;
        }
      }
    }
    size_t rest_idx_start = each * FIND_VLEN;
    size_t rest_idx = rest_idx_start;
    for(size_t j = 0; j < rest; j++) {
      auto idx = crntidx[j + rest_idx_start];
      auto loaded_v = vp[idx+off];
      if(loaded_v == to_find) {
        bufidx[rest_idx++] = idx;
      }
    }
    for(size_t i = 0; i < FIND_VLEN_EACH; i++) {
      auto v0 = i;
      auto v1 = i + FIND_VLEN_EACH;
      auto v2 = i + FIND_VLEN_EACH * 2;
      auto v3 = i + FIND_VLEN_EACH * 3;
      out_ridx[v0] = out_ridx_0[i];
      out_ridx[v1] = out_ridx_1[i];
      out_ridx[v2] = out_ridx_2[i];
      out_ridx[v3] = out_ridx_3[i];
    }
    size_t sizes[FIND_VLEN];
    for(size_t i = 0; i < FIND_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < FIND_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - rest_idx_start;
    total += rest_size;
    nextsize = total;
    size_t current = 0;
    for(size_t i = 0; i < FIND_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        crntidx[current + j] = bufidx[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      crntidx[current + j] = bufidx[rest_idx_start + j];
    }
  }
}

vector<size_t> find_impl(const int* vp, size_t size,
                         const string& to_find) {
  auto to_find_size = to_find.size();
  if(to_find_size == 0 || size == 0 || size < to_find_size)
    return vector<size_t>();
  auto idxsize = size - (to_find_size - 1);
  std::vector<size_t> crntidx(idxsize);
  std::vector<size_t> bufidx(idxsize);
  auto crntidxp = crntidx.data();
  size_t retsize;
  auto crntsize = idxsize;
  find_one_helper(vp, crntidxp, crntsize, bufidx.data(), retsize,
                  static_cast<int>(static_cast<unsigned char>(to_find[0])));
  crntsize = retsize;
  for(size_t i = 1; i < to_find_size; i++) {
    find_helper(vp, crntidxp, crntsize, bufidx.data(), retsize,
                static_cast<int>(static_cast<unsigned char>(to_find[i])), i);
    crntsize = retsize;
  }
  {std::vector<size_t> tmp; bufidx.swap(tmp);}
  std::vector<size_t> ret(retsize);
  auto retp = ret.data();
  for(size_t i = 0; i < retsize; i++) retp[i] = crntidxp[i];
  return ret;
}

std::vector<size_t> find_impl(const std::vector<int>& sv,
                              const std::string& to_find) {
  return find_impl(sv.data(), sv.size(), to_find);
}


vector<size_t> find(const int* vp, size_t size,
                    const string& to_find,
                    size_t block_size_mb) {
  if(block_size_mb == 0) throw runtime_error("block_size is 0");
  auto block_size = block_size_mb * 1024 * 1024 / 4;
  auto to_find_size = to_find.size();
  if(block_size < to_find_size)
    throw runtime_error("to_find size is larger than block_size");
  auto num_block = size / block_size;
  auto rest_size = size - num_block * block_size;
  if(num_block == 0) return find_impl(vp, size, to_find);
  std::vector<std::vector<size_t>> retvec(num_block + 1);
  auto retvecp = retvec.data();
  for(size_t i = 0; i < num_block; i++) {
    if(i == 0) {
      retvecp[i] = find_impl(vp, block_size, to_find);
    } else {
      auto start_pos = block_size * i - (to_find_size - 1);
      auto find_size = block_size + (to_find_size - 1);
      retvecp[i] = find_impl(vp + start_pos, find_size, to_find);
      auto retvecip = retvecp[i].data();
      auto retveci_size = retvecp[i].size();
      for(size_t j = 0; j < retveci_size; j++) {
        retvecip[j] += start_pos;
      }
    }
  }
  // at least one block
  auto start_pos = block_size * num_block - (to_find_size - 1);
  auto find_size = rest_size + (to_find_size - 1);
  retvecp[num_block] = find_impl(vp + start_pos, find_size, to_find);
  auto retvecip = retvecp[num_block].data();
  auto retveci_size = retvecp[num_block].size();
  for(size_t j = 0; j < retveci_size; j++) {
    retvecip[j] += start_pos;
  }
  size_t total = 0;
  for(size_t i = 0; i < num_block + 1; i++) {
    total += retvecp[i].size();
  }
  vector<size_t> ret(total);
  auto retp = ret.data();
  auto crnt_retp = retp;
  for(size_t i = 0; i < num_block + 1; i++) {
    auto crnt_size = retvecp[i].size();
    auto retvecip = retvecp[i].data();
    for(size_t j = 0; j < crnt_size; j++) {
      crnt_retp[j] = retvecip[j];
    }
    crnt_retp += crnt_size;
  }
  return ret;
}

std::vector<size_t> find(const std::vector<int>& sv,
                         const std::string& to_find,
                         size_t block_size_mb) {
  return find(sv.data(), sv.size(), to_find, block_size_mb);
}

}
