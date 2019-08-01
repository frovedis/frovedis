
#ifndef _LDA_UTILITY_HPP_
#define _LDA_UTILITY_HPP_

namespace frovedis {
namespace lda {
 
template <class T>
T sum(T& a, T& b){return a + b;}

template <typename TC>
crs_matrix_local<TC> rowmajor_to_crs(rowmajor_matrix_local<TC>& a){    
    size_t num_col = a.local_num_col, num_row = a.local_num_row;    
    crs_matrix_local<TC> res(num_row,num_col);    
    res.off.resize(num_row+1);
    res.idx.resize(num_row*num_col);
    res.val.resize(num_row*num_col);
    res.off[0] = 0;
    size_t c_x_v = num_col*LDA_CGS_VLEN;
    size_t i_x_v = 0;  
    int32_t sub_count = (num_row-1)/LDA_CGS_VLEN + 1; // avoid too much temporary memory requirement
    int32_t loop_count = LDA_CGS_VLEN;  
    
    std::vector<TC> tmp_val_dense(c_x_v); // Careful!!! too much memory required for really large num_col
    std::vector<TC> tmp_idx_dense(c_x_v);
    size_t pos_res[LDA_CGS_VLEN];
    TC nonzero_res[LDA_CGS_VLEN];
    TC idx_offset[LDA_CGS_VLEN];
#if defined(LDA_CGS_USE_VREG)
#pragma _NEC vreg(pos_res)
#pragma _NEC vreg(nonzero_res)   
#pragma _NEC vreg(idx_offset)   
#endif
    
    for(int w=0; w<LDA_CGS_VLEN; w++){
        idx_offset[w] = w*num_col;
    }            
    int finished_vec = 0;
    for(int i = 0; i < sub_count; i++) {
        if(i == sub_count-1) loop_count = num_row % LDA_CGS_VLEN; 
        loop_count = loop_count==0? LDA_CGS_VLEN:loop_count;
#pragma _NEC shortloop
#pragma _NEC ivdep
        for(int w=0; w<loop_count; w++){  
            pos_res[w] = 0;
            nonzero_res[w] = 0;
        }       
        for(size_t t=0; t<num_col; t++){
#pragma _NEC shortloop
#pragma _NEC ivdep
            for(int w=0; w<loop_count; w++){
                size_t word_id = i_x_v+w;
                TC val = a.val[word_id * num_col + t];
                if(val!=0) {
                    TC dense_idx = idx_offset[w]+nonzero_res[w];
                    tmp_val_dense[dense_idx] = val;
                    tmp_idx_dense[dense_idx] = t;
                    nonzero_res[w] ++;
                }
            }
        }        
        for(size_t w=i_x_v; w<i_x_v+loop_count; w++){
            res.off[w+1] = res.off[w] + nonzero_res[w-i_x_v];
        }   
        finished_vec = 0;
        while (finished_vec < loop_count) {  
            finished_vec = 0;
#pragma _NEC ivdep
#pragma _NEC shortloop
            for(int w=0; w<loop_count; w++){
                if(nonzero_res[w]==0) finished_vec++;
                else {
                    TC sparse_idx = res.off[w+i_x_v] + pos_res[w];
                    TC dense_idx = idx_offset[w]+pos_res[w];
                    res.idx[sparse_idx] = tmp_idx_dense[dense_idx];
                    res.val[sparse_idx] = tmp_val_dense[dense_idx];
                    pos_res[w] ++; nonzero_res[w]--;
                }
            }
        }
        i_x_v += loop_count;
    }
    res.idx.resize(res.off.back());
    res.val.resize(res.off.back());
    return res;
}

#if (defined(_SX) || defined(__ve__))
template <typename TC>
crs_matrix_local<TC> sum_crs(crs_matrix_local<TC>& a, crs_matrix_local<TC>& b){     
    size_t num_topics = a.local_num_col, num_voc = a.local_num_row;    
    crs_matrix_local<TC> res(num_voc,num_topics);
    res.off.resize(num_voc+1);
    res.idx.resize(num_voc*num_topics);
    res.val.resize(num_voc*num_topics);
    res.off[0] = 0;
    
    size_t k_x_v = num_topics*LDA_CGS_VLEN;
    size_t i_x_v = 0;
    int32_t loop_count = LDA_CGS_VLEN, sub_count = (num_voc-1)/LDA_CGS_VLEN + 1;   // avoid too much temporary memory requirement
    if(sub_count == 1) loop_count = num_voc;
    
    std::vector<TC> tmp_val_dense(k_x_v);
    std::vector<TC> tmp_idx_dense(k_x_v);
    size_t pos_a[LDA_CGS_VLEN];
    size_t pos_b[LDA_CGS_VLEN];
    size_t pos_res[LDA_CGS_VLEN];
    TC nonzero_a[LDA_CGS_VLEN];
    TC nonzero_b[LDA_CGS_VLEN];
    TC nonzero_res[LDA_CGS_VLEN];
    TC idx_offset[LDA_CGS_VLEN];    
#if defined(LDA_CGS_USE_VREG)
#pragma _NEC vreg(pos_a)
#pragma _NEC vreg(pos_b)
#pragma _NEC vreg(pos_res)
#pragma _NEC vreg(nonzero_a)
#pragma _NEC vreg(nonzero_b)
#pragma _NEC vreg(nonzero_res) 
#pragma _NEC vreg(idx_offset)   
#endif
    for(int w=0; w<LDA_CGS_VLEN; w++){
        idx_offset[w] = w*num_topics;
    }      
            
    int finished_vec = 0;
    for(int i = 0; i < sub_count; i++) {
        if(i == sub_count-1 && sub_count > 1) loop_count = num_voc % LDA_CGS_VLEN; 
        loop_count = loop_count==0? LDA_CGS_VLEN:loop_count;
#pragma _NEC shortloop
#pragma _NEC ivdep
        for(int w=0; w<loop_count; w++){  
            size_t row_idx = i_x_v + w;
            pos_a[w] = 0; pos_b[w] = 0; pos_res[w] = 0;
            nonzero_a[w] = a.off[row_idx+1] - a.off[row_idx];
            nonzero_b[w] = b.off[row_idx+1] - b.off[row_idx];
            nonzero_res[w] = 0;
        }       
        finished_vec = 0;
        while (finished_vec < loop_count) {  
            finished_vec = 0;
#pragma _NEC ivdep
#pragma _NEC shortloop
            for(int w=0; w<loop_count; w++){
                size_t row_idx = i_x_v+w;
                TC idx_a, idx_b, val_a, val_b;
                size_t col_a, col_b;
                if (nonzero_a[w] == 0 && nonzero_b[w] == 0) {
                    finished_vec ++;
                } else if (nonzero_a[w] != 0 && nonzero_b[w] != 0) {                  
                    idx_a = a.off[row_idx] + pos_a[w], idx_b = b.off[row_idx] + pos_b[w];
                    col_a = a.idx[idx_a], col_b = b.idx[idx_b];
                    if(col_a == col_b) {
                        val_a = a.val[idx_a], val_b = b.val[idx_b];         
                        tmp_val_dense[nonzero_res[w]+idx_offset[w]] = val_a + val_b;
                        tmp_idx_dense[nonzero_res[w]+idx_offset[w]] = col_a; 
                        pos_a[w] ++; pos_b[w] ++; nonzero_res[w] ++; nonzero_a[w] --; nonzero_b[w] --;
                    } else if (col_a < col_b) {                        
                        val_a = a.val[idx_a];
                        tmp_val_dense[nonzero_res[w]+idx_offset[w]] = val_a;
                        tmp_idx_dense[nonzero_res[w]+idx_offset[w]] = col_a; 
                        pos_a[w] ++; nonzero_res[w] ++; nonzero_a[w] --; 
                    } else {  
                        val_b = b.val[idx_b];
                        tmp_val_dense[nonzero_res[w]+idx_offset[w]] = val_b;
                        tmp_idx_dense[nonzero_res[w]+idx_offset[w]] = col_b; 
                        pos_b[w] ++; nonzero_res[w] ++; nonzero_b[w] --; 
                    }
                } else if (nonzero_b[w] == 0) {
                    idx_a = a.off[row_idx] + pos_a[w];
                    col_a = a.idx[idx_a]; val_a = a.val[idx_a];
                    tmp_val_dense[nonzero_res[w]+idx_offset[w]] = val_a;
                    tmp_idx_dense[nonzero_res[w]+idx_offset[w]] = col_a;  
                    pos_a[w] ++; nonzero_res[w] ++; nonzero_a[w] --;
                } else {
                    idx_b = b.off[row_idx] + pos_b[w];
                    col_b = b.idx[idx_b]; val_b = b.val[idx_b];
                    tmp_val_dense[nonzero_res[w]+idx_offset[w]] = val_b;
                    tmp_idx_dense[nonzero_res[w]+idx_offset[w]] = col_b; 
                    pos_b[w] ++; nonzero_res[w] ++; nonzero_b[w] --;
                } 
            }
        }   
        for(int w=i_x_v; w<i_x_v+loop_count; w++){
            auto idx = w-i_x_v;
            res.off[w+1] = res.off[w] + nonzero_res[idx];
        }   
        finished_vec = 0;
        while (finished_vec < loop_count) {  
            finished_vec = 0;
#pragma _NEC ivdep
#pragma _NEC shortloop
            for(int w=0; w<loop_count; w++){
                if(nonzero_res[w]==0) finished_vec++;
                else {
                    int idx_res = res.off[w+i_x_v] + pos_res[w];
                    res.idx[idx_res] = tmp_idx_dense[idx_offset[w]+pos_res[w]];
                    res.val[idx_res] = tmp_val_dense[idx_offset[w]+pos_res[w]];
                    pos_res[w] ++; nonzero_res[w]--;
                }
            }
        }
        i_x_v += loop_count;
    }
    res.idx.resize(res.off.back());
    res.val.resize(res.off.back());
    return res;
}
#else
template <typename TC>
crs_matrix_local<TC> sum_crs(crs_matrix_local<TC>& a, crs_matrix_local<TC>& b){
    crs_matrix_local<TC> res(a.local_num_row,a.local_num_col);
    size_t pos1=0, pos2=0, pos3=0; 
    for(size_t r=0; r<a.local_num_row; r++){
        while(pos1<a.off[r+1] && pos2<b.off[r+1]){
            if(a.idx[pos1] == b.idx[pos2]){
                res.val.push_back(a.val[pos1]+b.val[pos2]);
                res.idx.push_back(a.idx[pos1]);
                pos1++;pos2++;pos3++;
            } else if(a.idx[pos1] < b.idx[pos2]) {
                res.val.push_back(a.val[pos1]);
                res.idx.push_back(a.idx[pos1]);
                pos1++;pos3++;
            } else if(a.idx[pos1] > b.idx[pos2]) {
                res.val.push_back(b.val[pos2]);
                res.idx.push_back(b.idx[pos2]);
                pos2++;pos3++;
            }
        }
        while(pos1<a.off[r+1]){
            res.val.push_back(a.val[pos1]);
            res.idx.push_back(a.idx[pos1]);
            pos1++;pos3++;
        }
        while(pos2<b.off[r+1]){
            res.val.push_back(b.val[pos2]);
            res.idx.push_back(b.idx[pos2]);
            pos2++;pos3++;
        }
        res.off.push_back(pos3);
    }    
    return res;
}
#endif

}
}


#endif /* _LDA_UTILITY_HPP_ */

