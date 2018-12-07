#include <vector>
#include <tuple>
#include <iostream>
#include <cmath>
#include <cassert>
namespace frovedis {

template <class KeyType>
void
find_partitions(std::vector<KeyType>& left, std::vector<KeyType>& right, 
        size_t num_partition, 
        std::vector<size_t>& left_partitions, 
        std::vector<size_t>& right_partitions)
{
  size_t left_size = left.size();
  size_t right_size = right.size();

  auto* lparp = &left_partitions[0];
  auto* rparp = &right_partitions[0]; 
  size_t ptr_lower[num_partition];
  size_t ptr_upper[num_partition];
  size_t left_lower[num_partition];
  size_t right_lower[num_partition];
  size_t diag_len[num_partition];

  size_t total_size = left_size + right_size;
  size_t partition_size = ceil( 1.0 * total_size / num_partition );
  int ifFinished[num_partition];
  for(size_t i=0; i<num_partition; i++){
    ifFinished[i] = false;
  }
    #pragma cdir nodep
    #pragma _NEC ivdep
    #pragma cdir on_adb(lparp)
    #pragma cdir on_adb(rparp)
    #pragma cdir on_adb(diag_len)
    #pragma cdir on_adb(ifFinished)
    #pragma cdir on_adb(left_lower)
    #pragma cdir on_adb(right_lower)
    for (size_t i_part = 0; i_part < num_partition ; i_part++) {
        size_t diag_offset = partition_size * (i_part + 1);
        if (diag_offset >= total_size ) {
          lparp[i_part] = left_size;
          rparp[i_part] = right_size;
          ifFinished[i_part] = true;
        }
        else{
            // diagonal lower bound
            if(diag_offset > right_size){
              left_lower[i_part] = diag_offset -  right_size;
            }
            else{
                left_lower[i_part] = 0;
            }
            if(diag_offset > left_size){
              right_lower[i_part] = diag_offset - left_size;
            }
            else{
                right_lower[i_part] = 0;
            }
        }
        diag_len[i_part] = diag_offset - left_lower[i_part] - right_lower[i_part];
        ptr_lower[i_part] = 0;
        ptr_upper[i_part] = diag_len[i_part];
  }

  while (1) {
    #pragma _NEC ivdep 
    #pragma cdir nodep      
    #pragma cdir on_adb(left)
    #pragma cdir on_adb(right)
    #pragma cdir on_adb(ifFinished)
    #pragma cdir on_adb(ptr_lower)
    #pragma cdir on_adb(ptr_upper)
    #pragma cdir on_adb(left_lower)
    #pragma cdir on_adb(right_lower)
    #pragma cdir on_adb(diag_len)
    #pragma cdir on_adb(lparp)
    #pragma cdir on_adb(rparp)
    #pragma cdir on_adb(diag_len)
    for (size_t i_part = 0; i_part < num_partition ; i_part++) {
      if(ifFinished[i_part] == false){
        size_t ptr_mid = (ptr_lower[i_part] + ptr_upper[i_part]) / 2;
        size_t id_left = left_lower[i_part] + ptr_mid;
        size_t id_right = right_lower[i_part] + (diag_len[i_part] - ptr_mid - 1);

        int left_lt_right = (left[id_left] > right[id_right]);

        if (left_lt_right) {
          ptr_upper[i_part] = ptr_mid;
        } else {
          ptr_lower[i_part] = ptr_mid + 1;
        }

        if (ptr_lower[i_part] == ptr_upper[i_part]) {
          lparp[i_part] = left_lower[i_part] + ptr_lower[i_part];
          rparp[i_part] = right_lower[i_part] + (diag_len[i_part] - ptr_lower[i_part]);
          ifFinished[i_part] = true;
        }
      }
    }
    bool ifFinished_all = true;
    for(size_t i=0; i<num_partition; i++){
      if(ifFinished[i] == false){
        ifFinished_all = false;
        break;
      }
    }
    if(ifFinished_all == true){
      break;
    }

  }

    for (size_t i_part = 0; i_part < num_partition ; i_part++) {//check boundary to avoid dups btw partitions
        if(rparp[i_part] <right_size &&  lparp[i_part] > 0 && right[rparp[i_part]] == left[lparp[i_part]-1] ) {
          rparp[i_part]++;
        }
    }
}

template <class T>
void
AdaptPartition_2D_EachPair(std::vector<T>& buff, 
                size_t num_par_each,
                size_t left_idx, 
                size_t left_idx_stop, 
                size_t right_idx, 
                size_t right_idx_stop,
                size_t left_idx_each[], 
                size_t left_idx_stop_each[], 
                size_t right_idx_each[], 
                size_t right_idx_stop_each[]){
    
    std::vector<size_t> left_par(num_par_each);
    std::vector<size_t> right_par(num_par_each);
    auto* buffp = &buff[0];
    auto* left_par_p = &left_par[0];
    auto* right_par_p = &right_par[0];    

    std::vector<size_t> left(left_idx_stop - left_idx);
    std::vector<size_t> right(right_idx_stop - right_idx);
    size_t left_size = left_idx_stop - left_idx;
    size_t right_size = right_idx_stop - right_idx;
    auto* lp = &left[0];
    auto* rp = &right[0];          
        
     #pragma cdir on_adb(lp)   
     #pragma cdir on_adb(rp)  
     #pragma cdir on_adb(buffp)
    for(size_t k = 0; k < left_size;k++){
        lp[k] = buffp[left_idx + k];
    }

    for(size_t k = 0; k < right_size;k++){
        rp[k] = buffp[right_idx + k];
    }

    find_partitions(left, 
            right, 
            num_par_each, 
            left_par, 
            right_par);

    left_idx_each[0] = left_idx;
    right_idx_each[0] = right_idx;
    
     #pragma cdir on_adb(left_par_p)   
     #pragma cdir on_adb(right_par_p)  
     #pragma cdir on_adb(left_idx_each)
     #pragma cdir on_adb(right_idx_each)
    for(size_t j = 1; j<num_par_each;j++){
        if(left_par_p[j-1]+left_idx > left_idx_stop ){
            left_idx_each[j] = left_idx_stop;
        }else{
            left_idx_each[j] = left_par_p[j-1] + left_idx;
        }
        if(right_par_p[j-1] + right_idx > right_idx_stop){
            right_idx_each[j] = right_idx_stop;
        }else{
            right_idx_each[j] = right_par_p[j-1] + right_idx;
        } 
    }
     #pragma cdir on_adb(left_idx_stop_each)   
     #pragma cdir on_adb(right_idx_stop_each)
     #pragma cdir on_adb(left_idx_each)   
     #pragma cdir on_adb(right_idx_each)
      for(int j = 0; j < num_par_each - 1; j++) {
        left_idx_stop_each[j] = left_idx_each[j + 1];
        right_idx_stop_each[j] = right_idx_each[j + 1];
    }
    left_idx_stop_each[ num_par_each - 1] = left_idx_stop;
    right_idx_stop_each[ num_par_each - 1] = right_idx_stop;        
     
}

template <class T>
void
AdaptPartition_2D(std::vector<T>& buff, 
                size_t num_pairs_vec,
                size_t num_par_each,
                std::vector<size_t>& left_idx, 
                std::vector<size_t>& left_idx_stop, 
                std::vector<size_t>& right_idx, 
                std::vector<size_t>& right_idx_stop,
                size_t left_idx_each[], 
                size_t left_idx_stop_each[], 
                size_t right_idx_each[], 
                size_t right_idx_stop_each[]){
    
    // Vectorize for all pairs
    size_t num_par_total = num_pairs_vec*num_par_each;
    size_t left_size[num_pairs_vec];
    size_t right_size[num_pairs_vec];
    size_t partition_size[num_pairs_vec];
    
    int ifFinished[num_par_total];
    size_t diag_offset[num_par_total];
    
    #pragma cdir on_adb(ifFinished)    
    for(size_t i=0; i<num_par_total; i++){
        ifFinished[i] = false;
    }        
    #pragma _NEC ivdep 
    #pragma cdir nodep    
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_stop)
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_stop)
    #pragma cdir on_adb(left_size)
    #pragma cdir on_adb(right_size)
    #pragma cdir on_adb(partition_size)
    for(size_t i=0; i<num_pairs_vec; i++){
        left_size[i] = left_idx_stop[i] - left_idx[i];
        right_size[i] = right_idx_stop[i] - right_idx[i];
    }
    #pragma _NEC ivdep 
    #pragma cdir nodep
    #pragma cdir on_adb(left_size)
    #pragma cdir on_adb(right_size)
    #pragma cdir on_adb(partition_size)    
    for(size_t i=0; i<num_pairs_vec; i++){
        partition_size[i] = (left_size[i]+right_size[i]) / num_par_each + 1; 
    }
    auto* buffp = &buff[0];
  size_t lparp[num_par_total];
  size_t rparp[num_par_total];  
  size_t ptr_lower[num_par_total];
  size_t ptr_upper[num_par_total];
  size_t left_lower[num_par_total];
  size_t right_lower[num_par_total];
  size_t diag_len[num_par_total];

    #pragma cdir nodep
    #pragma _NEC ivdep
    #pragma cdir on_adb(lparp)
    #pragma cdir on_adb(rparp)
    #pragma cdir on_adb(diag_len)
    #pragma cdir on_adb(ifFinished)
    #pragma cdir on_adb(left_lower)
    #pragma cdir on_adb(right_lower)
    #pragma cdir on_adb(ptr_lower)
    #pragma cdir on_adb(ptr_upper)  
    #pragma cdir on_adb(diag_len)
    #pragma cdir on_adb(diag_offset)  
    #pragma cdir on_adb(partition_size)
    for (size_t i_part = 0; i_part < num_par_total ; i_part++) {
        size_t i_pair = i_part/num_par_each;

        diag_offset[i_part] = partition_size[i_pair] * (i_part-(i_pair)*num_par_each + 1);
    if (diag_offset[i_part] >= (left_size[i_pair] + right_size[i_pair]) 
            ||  right_size[i_pair] == 0 
            || left_size[i_pair] == 0) {
      lparp[i_part] = left_size[i_pair];
      rparp[i_part] = right_size[i_pair];
      ifFinished[i_part] = true;
    }
    // diagonal lower bound
    else {
        
        if(diag_offset[i_part]  > right_size[i_pair] ){
          left_lower[i_part] = diag_offset[i_part] - right_size[i_pair];
        }
            else{
                left_lower[i_part] = 0;
            }
        if(diag_offset[i_part] > left_size[i_pair]){
          right_lower[i_part] = diag_offset[i_part] - left_size[i_pair];
        }
        else{
            right_lower[i_part] = 0;
        }
    }
    diag_len[i_part] = diag_offset[i_part] - left_lower[i_part] - right_lower[i_part];

    ptr_lower[i_part] = 0;
    ptr_upper[i_part] = diag_len[i_part];
  }

  while (1) {
    #pragma cdir nodep
    #pragma _NEC ivdep      
    #pragma cdir on_adb(buffp)
    #pragma cdir on_adb(ifFinished)
    #pragma cdir on_adb(ptr_lower)
    #pragma cdir on_adb(ptr_upper)
    #pragma cdir on_adb(left_lower)
    #pragma cdir on_adb(right_lower)
    #pragma cdir on_adb(diag_len)
    #pragma cdir on_adb(lparp)
    #pragma cdir on_adb(rparp)
    #pragma cdir on_adb(diag_len)
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(right_idx)
    for (size_t i_part = 0; i_part < num_par_total ; i_part++) {
        size_t i_pair = i_part/num_par_each;
        size_t left_offset = left_idx[i_pair];
        size_t right_offset = right_idx[i_pair];
        
      if(ifFinished[i_part] == false){
        size_t ptr_mid = (ptr_lower[i_part] + ptr_upper[i_part]) / 2;
        size_t id_left = left_lower[i_part] + ptr_mid;
        size_t id_right = right_lower[i_part] + (diag_len[i_part] - ptr_mid - 1);

        if (buffp[id_left + left_offset] > buffp[right_offset + id_right]){
          ptr_upper[i_part] = ptr_mid;
        } else {
          ptr_lower[i_part] = ptr_mid + 1;
        }

        if (ptr_lower[i_part] == ptr_upper[i_part]) {
          lparp[i_part] = left_lower[i_part] + ptr_lower[i_part];
          rparp[i_part] = right_lower[i_part] + (diag_len[i_part] - ptr_lower[i_part]);
          ifFinished[i_part] = true;
        }
      }
    }
    bool ifFinished_all = true;
    #pragma _NEC ivdep 
    #pragma cdir nodep     
    #pragma cdir on_adb(ifFinished)      
    for(size_t i=0; i<num_par_total; i++){
      if(ifFinished[i] == false){
        ifFinished_all = false;
        break;
      }
    }
    if(ifFinished_all == true){
      break;
    }

  } 
    #pragma cdir on_adb(buffp)
    #pragma cdir on_adb(rparp)
    #pragma cdir on_adb(lparp)
    #pragma cdir on_adb(right_size)
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(left_idx)
    #pragma cdir nodep
    #pragma _NEC ivdep
  for (size_t i_part = 0; i_part < num_par_total ; i_part++) {//check boundary to avoid dups btw partitions
      size_t i_pair = i_part/num_par_each;
      if(rparp[i_part] <right_size[i_pair] &&  
              lparp[i_part] > 0 && 
              (buffp[rparp[i_part] + right_idx[i_pair]] == buffp[lparp[i_part] + left_idx[i_pair]-1]) ) {
        rparp[i_part]++;
      }
  }
  
  
    #pragma cdir on_adb(left_idx_each)   
    #pragma cdir on_adb(right_idx_each)
    #pragma cdir on_adb(left_idx)   
    #pragma cdir on_adb(right_idx) 
    for(size_t i=0;i<num_pairs_vec;i++){
      left_idx_each[i*num_par_each+0] = left_idx[i];
      right_idx_each[i*num_par_each+0] = right_idx[i];
    }


  if(num_pairs_vec < num_par_each){
    #pragma cdir on_adb(left_par_p)   
    #pragma cdir on_adb(right_par_p)  
    #pragma cdir on_adb(left_idx_each)
    #pragma cdir on_adb(right_idx_each)
    #pragma cdir nodep
    #pragma _NEC ivdep
    for(size_t i=0;i<num_pairs_vec;i++){
        for(size_t j = 1; j<num_par_each;j++){

                #pragma cdir nodep
                #pragma _NEC ivdep

                size_t idx = i*num_par_each + j;
                if(lparp[idx-1] + left_idx[i]>= left_idx_stop[i] ){
                    left_idx_each[idx] = left_idx_stop[i];
                }
                else{
                    left_idx_each[idx] = lparp[idx-1] + left_idx[i];
                }
                if(rparp[idx-1] + right_idx[i]>= right_idx_stop[i]){
                    right_idx_each[idx] = right_idx_stop[i];
                }
                else{
                    right_idx_each[idx] = rparp[idx-1] + right_idx[i];
                } 
        }
    }
    for(int i=0;i<num_pairs_vec;i++){
     #pragma cdir on_adb(left_idx_stop_each)   
     #pragma cdir on_adb(right_idx_stop_each)
     #pragma cdir on_adb(left_idx_each)   
     #pragma cdir on_adb(right_idx_each)
        for(int j = 0; j < num_par_each - 1; j++) {
          size_t idx = num_par_each*i + j;
          left_idx_stop_each[idx] = left_idx_each[idx + 1];
          right_idx_stop_each[idx] = right_idx_each[idx + 1];
      }
    }    
  }else{
    for(size_t j = 1; j<num_par_each;j++){      
        for(size_t i=0;i<num_pairs_vec;i++){


                #pragma cdir nodep
                #pragma _NEC ivdep

                size_t idx = i*num_par_each + j;
                if(lparp[idx-1] + left_idx[i]>= left_idx_stop[i] ){
                    left_idx_each[idx] = left_idx_stop[i];
                }
                else{
                    left_idx_each[idx] = lparp[idx-1] + left_idx[i];
                }
                if(rparp[idx-1] + right_idx[i]>= right_idx_stop[i]){
                    right_idx_each[idx] = right_idx_stop[i];
                }
                else{
                    right_idx_each[idx] = rparp[idx-1] + right_idx[i];
                } 
        }
    }
    for(int j = 0; j < num_par_each - 1; j++) {    
        for(int i=0;i<num_pairs_vec;i++){
     #pragma cdir on_adb(left_idx_stop_each)   
     #pragma cdir on_adb(right_idx_stop_each)
     #pragma cdir on_adb(left_idx_each)   
     #pragma cdir on_adb(right_idx_each)

          size_t idx = num_par_each*i + j;
          left_idx_stop_each[idx] = left_idx_each[idx + 1];
          right_idx_stop_each[idx] = right_idx_each[idx + 1];
      }
    }    
  }

    for(int i=0;i<num_pairs_vec;i++){
        left_idx_stop_each[i*num_par_each + num_par_each - 1] = left_idx_stop[i];
        right_idx_stop_each[i*num_par_each + num_par_each - 1] = right_idx_stop[i];   
    }
}

template <class T>
void
AdaptPartition_2D_valid(std::vector<T>& buff, 
                size_t num_pairs_vec,
                size_t num_par_each,
                std::vector<size_t>& valid_vec_idx,
                std::vector<size_t>& left_idx, 
                std::vector<size_t>& left_idx_stop, 
                std::vector<size_t>& right_idx, 
                std::vector<size_t>& right_idx_stop,
                size_t left_idx_each[], 
                size_t left_idx_stop_each[], 
                size_t right_idx_each[], 
                size_t right_idx_stop_each[]){ //Only partition vectors that are not finished comparison yet
    
    // Vectorize for all pairs
    size_t num_par_total = num_pairs_vec*num_par_each;
    size_t left_size[num_pairs_vec];
    size_t right_size[num_pairs_vec];
    size_t partition_size[num_pairs_vec];
    
    int ifFinished[num_par_total];
    size_t diag_offset[num_par_total];
    
    #pragma cdir on_adb(ifFinished)    
    for(size_t i=0; i<num_par_total; i++){
        ifFinished[i] = false;
    }        
    #pragma _NEC ivdep 
    #pragma cdir nodep    
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(left_idx_stop)
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(right_idx_stop)
    #pragma cdir on_adb(left_size)
    #pragma cdir on_adb(right_size)
    #pragma cdir on_adb(partition_size)
    for(size_t i=0; i<num_pairs_vec; i++){
        left_size[i] = left_idx_stop[valid_vec_idx[i]] - left_idx[valid_vec_idx[i]];
        right_size[i] = right_idx_stop[valid_vec_idx[i]] - right_idx[valid_vec_idx[i]];     
    }
    #pragma _NEC ivdep 
    #pragma cdir nodep
    #pragma cdir on_adb(left_size)
    #pragma cdir on_adb(right_size)
    #pragma cdir on_adb(partition_size)    
    for(size_t i=0; i<num_pairs_vec; i++){
        partition_size[i] = (left_size[i]+right_size[i]) / num_par_each + 1; 
    }
    auto* buffp = &buff[0];
    size_t lparp[num_par_total];
    size_t rparp[num_par_total];  
    size_t ptr_lower[num_par_total];
    size_t ptr_upper[num_par_total];
    size_t left_lower[num_par_total];
    size_t right_lower[num_par_total];
    size_t diag_len[num_par_total];

    #pragma cdir nodep
    #pragma _NEC ivdep
    #pragma cdir on_adb(lparp)
    #pragma cdir on_adb(rparp)
    #pragma cdir on_adb(diag_len)
    #pragma cdir on_adb(ifFinished)
    #pragma cdir on_adb(left_lower)
    #pragma cdir on_adb(right_lower)
    #pragma cdir on_adb(ptr_lower)
    #pragma cdir on_adb(ptr_upper)  
    #pragma cdir on_adb(diag_len)
    #pragma cdir on_adb(diag_offset)  
    #pragma cdir on_adb(partition_size)
    for (size_t i_part = 0; i_part < num_par_total ; i_part++) {
        size_t i_pair = i_part/num_par_each;

        diag_offset[i_part] = partition_size[i_pair] * (i_part-(i_pair)*num_par_each + 1);
    if (diag_offset[i_part] >= (left_size[i_pair] + right_size[i_pair]) 
            ||  right_size[i_pair] == 0 
            || left_size[i_pair] == 0) {
      lparp[i_part] = left_size[i_pair];
      rparp[i_part] = right_size[i_pair];
      ifFinished[i_part] = true;
    }
    // diagonal lower bound
    else {
        
        if(diag_offset[i_part]  > right_size[i_pair] ){
          left_lower[i_part] = diag_offset[i_part] - right_size[i_pair];
        }
            else{
                left_lower[i_part] = 0;
            }
        if(diag_offset[i_part] > left_size[i_pair]){
          right_lower[i_part] = diag_offset[i_part] - left_size[i_pair];
        }
        else{
            right_lower[i_part] = 0;
        }
    }
    diag_len[i_part] = diag_offset[i_part] - left_lower[i_part] - right_lower[i_part];
    ptr_lower[i_part] = 0;
    ptr_upper[i_part] = diag_len[i_part];
  } 
  while (1) {
    #pragma cdir nodep
    #pragma _NEC ivdep      
    #pragma cdir on_adb(buffp)
    #pragma cdir on_adb(ifFinished)
    #pragma cdir on_adb(ptr_lower)
    #pragma cdir on_adb(ptr_upper)
    #pragma cdir on_adb(left_lower)
    #pragma cdir on_adb(right_lower)
    #pragma cdir on_adb(diag_len)
    #pragma cdir on_adb(lparp)
    #pragma cdir on_adb(rparp)
    #pragma cdir on_adb(diag_len)
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(right_idx)
    for (size_t i_part = 0; i_part < num_par_total ; i_part++) {
        size_t i_pair = valid_vec_idx[i_part/num_par_each];
        size_t left_offset = left_idx[i_pair];
        size_t right_offset = right_idx[i_pair];
        
      if(ifFinished[i_part] == false){
        size_t ptr_mid = (ptr_lower[i_part] + ptr_upper[i_part]) / 2;
        size_t id_left = left_lower[i_part] + ptr_mid;
        size_t id_right = right_lower[i_part] + (diag_len[i_part] - ptr_mid - 1);
        if (buffp[id_left + left_offset] > buffp[right_offset + id_right]){
          ptr_upper[i_part] = ptr_mid;
        } else {
          ptr_lower[i_part] = ptr_mid + 1;
        }

        if (ptr_lower[i_part] == ptr_upper[i_part]) {
          lparp[i_part] = left_lower[i_part] + ptr_lower[i_part];
          rparp[i_part] = right_lower[i_part] + (diag_len[i_part] - ptr_lower[i_part]);
          ifFinished[i_part] = true;
        }
      }
    }
    bool ifFinished_all = true;
    #pragma _NEC ivdep 
    #pragma cdir nodep     
    #pragma cdir on_adb(ifFinished)      
    for(size_t i=0; i<num_par_total; i++){
      if(ifFinished[i] == false){
        ifFinished_all = false;
        break;
      }
    }
    if(ifFinished_all == true){
      break;
    }

  } 
    #pragma cdir on_adb(buffp)
    #pragma cdir on_adb(rparp)
    #pragma cdir on_adb(lparp)
    #pragma cdir on_adb(right_size)
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(left_idx)
    #pragma cdir nodep
    #pragma _NEC ivdep
  for (size_t i_part = 0; i_part < num_par_total ; i_part++) {//check boundary to avoid dups btw partitions
      size_t i_pair = valid_vec_idx[i_part/num_par_each];
      if(rparp[i_part] <right_size[i_part/num_par_each] &&  
              lparp[i_part] > 0 && 
              (buffp[rparp[i_part] + right_idx[i_pair]] == buffp[lparp[i_part] + left_idx[i_pair]-1]) ) {
        rparp[i_part]++;
      }
  }
  
  
    #pragma cdir on_adb(left_idx_each)   
    #pragma cdir on_adb(right_idx_each)
    #pragma cdir on_adb(left_idx)   
    #pragma cdir on_adb(right_idx) 
    for(size_t i=0;i<num_pairs_vec;i++){
      left_idx_each[i*num_par_each+0] = left_idx[valid_vec_idx[i]];
      right_idx_each[i*num_par_each+0] = right_idx[valid_vec_idx[i]];
    }
    #pragma cdir on_adb(left_par_p)   
    #pragma cdir on_adb(right_par_p)  
    #pragma cdir on_adb(left_idx_each)
    #pragma cdir on_adb(right_idx_each)
    #pragma cdir nodep
    #pragma _NEC ivdep

    if(num_pairs_vec < num_par_each){
        for(size_t i=0;i<num_pairs_vec;i++){


            #pragma cdir nodep
            #pragma _NEC ivdep
            for(size_t j = 1; j<num_par_each;j++){

                size_t idx = i*num_par_each + j;
                if(lparp[idx-1] + left_idx[valid_vec_idx[i]]>= left_idx_stop[valid_vec_idx[i]] ){
                    left_idx_each[idx] = left_idx_stop[valid_vec_idx[i]];
                }
                else{
                    left_idx_each[idx] = lparp[idx-1] + left_idx[valid_vec_idx[i]];
                }
                if(rparp[idx-1] + right_idx[valid_vec_idx[i]]>= right_idx_stop[valid_vec_idx[i]]){
                    right_idx_each[idx] = right_idx_stop[valid_vec_idx[i]];
                }
                else{
                    right_idx_each[idx] = rparp[idx-1] + right_idx[valid_vec_idx[i]];
                } 
            }
        }
    for(int i=0;i<num_pairs_vec;i++){
     #pragma cdir on_adb(left_idx_stop_each)   
     #pragma cdir on_adb(right_idx_stop_each)
     #pragma cdir on_adb(left_idx_each)   
     #pragma cdir on_adb(right_idx_each)
        for(int j = 0; j < num_par_each - 1; j++) {
          size_t idx = num_par_each*i + j;
          left_idx_stop_each[idx] = left_idx_each[idx + 1];
          right_idx_stop_each[idx] = right_idx_each[idx + 1];
      }
    }    
  }else{
    for(size_t j = 1; j<num_par_each;j++){  
        #pragma cdir nodep
        #pragma _NEC ivdep
        for(size_t i=0;i<num_pairs_vec;i++){
                #pragma cdir nodep
                #pragma _NEC ivdep
                size_t idx = i*num_par_each + j;
                if(lparp[idx-1] + left_idx[valid_vec_idx[i]]>= left_idx_stop[valid_vec_idx[i]] ){
                    left_idx_each[idx] = left_idx_stop[valid_vec_idx[i]];
                }
                else{
                    left_idx_each[idx] = lparp[idx-1] + left_idx[valid_vec_idx[i]];
                }
                if(rparp[idx-1] + right_idx[valid_vec_idx[i]]>= right_idx_stop[valid_vec_idx[i]]){
                    right_idx_each[idx] = right_idx_stop[valid_vec_idx[i]];
                }
                else{
                    right_idx_each[idx] = rparp[idx-1] + right_idx[valid_vec_idx[i]];
                } 
        }
    }
    for(int j = 0; j < num_par_each - 1; j++) {    
        for(int i=0;i<num_pairs_vec;i++){
     #pragma cdir on_adb(left_idx_stop_each)   
     #pragma cdir on_adb(right_idx_stop_each)
     #pragma cdir on_adb(left_idx_each)   
     #pragma cdir on_adb(right_idx_each)

          size_t idx = num_par_each*i + j;
          left_idx_stop_each[idx] = left_idx_each[idx + 1];
          right_idx_stop_each[idx] = right_idx_each[idx + 1];
      }
    }    
  }

    for(int i=0;i<num_pairs_vec;i++){
        left_idx_stop_each[i*num_par_each + num_par_each - 1] = left_idx_stop[valid_vec_idx[i]];
        right_idx_stop_each[i*num_par_each + num_par_each - 1] = right_idx_stop[valid_vec_idx[i]];   
    }
}

}