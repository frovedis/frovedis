#ifndef GRAPH_BINARY_SEARCH_HPP
#define GRAPH_BINARY_SEARCH_HPP

#include <vector>
#include <algorithm>
#include <cmath>
namespace frovedis {

template <class T>
std::vector<size_t> lower_bound(std::vector<T>& sorted, std::vector<T>& values){ //Vectorized binary search for lowerbound. 
    size_t num_values = values.size();
    size_t num_elem = sorted.size();

    
    std::vector<size_t> ret(values.size(),0);
    auto* retp = &ret[0];
    auto* valuesp= &values[0];
    auto* sortedp= &sorted[0];
    std::vector<size_t> left(num_values,0); //range identifier for each value
    std::vector<size_t> right(num_values,num_elem - 1);
    auto* leftp = &left[0];
    auto* rightp = &right[0];

    std::vector<int> isFinished(num_values,0);
    bool isFinished_all = false;
    auto* isFinishedp = &isFinished[0];

       for(size_t j=0; j<num_values; j++){
            if(isFinishedp[j] == 0 && valuesp[j] > sortedp[num_elem - 1]){
                isFinishedp[j] = 1;
                retp[j] = num_elem;
            }
       }
       for(size_t j=0; j<num_values; j++){
            if(isFinishedp[j] == 0 && valuesp[j] <= sortedp[0]){
                isFinishedp[j] = 1;
                retp[j] = 0;
            }
       }  
    while(1){

    #pragma cdir on_adb(leftp)
    #pragma cdir on_adb(rightp)
    #pragma cdir on_adb(valuesp)
    #pragma cdir on_adb(retp)  
    #pragma cdir on_adb(isFinishedp)       
        for(size_t j=0; j<num_values; j++){
            if(isFinishedp[j] == 0 && leftp[j] != rightp[j]){
                size_t mid = (leftp[j] + rightp[j])/2;
                if(valuesp[j] < sortedp[mid]){
                    rightp[j] = mid;
                }
                else if(valuesp[j] >  sortedp[mid]){
                    leftp[j] = mid + 1;
                }
                else{//equal
                    retp[j] = mid;
                    isFinishedp[j] = 1;
                }
            }
            else if(isFinishedp[j] == 0 && leftp[j] == rightp[j]) {
                retp[j] = leftp[j];
                isFinishedp[j] = 1;
            }          
        }
        isFinished_all = true;
        for(size_t k=0; k<num_values; k++){
            if(isFinishedp[k] == 0){
                isFinished_all = 0;
                break;
            }
        }
        if(isFinished_all == true){
            break;
        }
    }
    return ret;
}

template <class T>
std::vector<size_t> upper_bound(std::vector<T> sorted, std::vector<T> values);

//#endif



template <class T>
void lower_bound_HMerge(std::vector<T>& buff,
        size_t num_pairs,
        T left_idx[], 
        T left_idx_stop[],
        T right_idx[], 
        T right_idx_stop[],
        T new_left_idx[],
        T new_right_idx[]
        ){
    
    auto* buffp = &buff[0];
    //first, find lower_bound for right_idx in left vectors
    
    size_t smallest_in_left[num_pairs]; //range identifier for each value
    size_t largest_in_left[num_pairs];  

    //second, find lower_bound for right vectors in left vectors
    size_t smallest_in_right[num_pairs]; //range identifier for each value
    size_t largest_in_right[num_pairs];


    
    for(size_t i=0; i<num_pairs; i++){
        smallest_in_right[i] = right_idx[i];
        largest_in_right[i] = right_idx_stop[i];
        
        smallest_in_left[i] = left_idx[i];
        largest_in_left[i] = left_idx_stop[i];
    }

    std::vector<int> isFinished_in_left(num_pairs,0);
    std::vector<int> isFinished_in_right(num_pairs,0);    
    bool isFinished_all_in_left = false;
    auto* isFinished_in_left_p = &isFinished_in_left[0];
    auto* isFinished_in_right_p = &isFinished_in_right[0];
    bool isFinished_all_in_right = false;
    
    

    #pragma cdir on_adb(isFinished_in_left_p)
    #pragma cdir on_adb(isFinished_in_right_p)
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(largest_in_left)
    #pragma cdir on_adb(smallest_in_right)
    #pragma cdir on_adb(smallest_in_left)
    #pragma cdir on_adb(largest_in_left)
    #pragma cdir on_adb(new_left_idx)
    #pragma cdir on_adb(new_right_idx)
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(buffp)
    for(size_t j=0; j<num_pairs; j++){
         if(isFinished_in_left_p[j] == 0 && buffp[right_idx[j]] > buffp[largest_in_left[j] - 1]){
             isFinished_in_left_p[j] = 1;
             new_left_idx[j] = largest_in_left[j];
         }

         else if(isFinished_in_left_p[j] == 0 && buffp[right_idx[j]] <= buffp[smallest_in_left[j]]){
             isFinished_in_left_p[j] = 1;
             new_left_idx[j] = smallest_in_left[j];
         }
         
         
         if(isFinished_in_right_p[j] == 0 && buffp[left_idx[j]] > buffp[largest_in_right[j] - 1]){
             isFinished_in_right_p[j] = 1;
             new_right_idx[j] = largest_in_right[j];
         }

         else if(isFinished_in_right_p[j] == 0 && buffp[left_idx[j]] <= buffp[smallest_in_right[j]]){
             isFinished_in_right_p[j] = 1;
             new_right_idx[j] = smallest_in_right[j];
         }         
    }  
    
    
    while(1){
    #pragma cdir on_adb(buffp)
    #pragma cdir on_adb(left_idx)
    #pragma cdir on_adb(new_left_idx)
    #pragma cdir on_adb(smallest_in_left)
    #pragma cdir on_adb(largest_in_left) 
    #pragma cdir on_adb(isFinished_in_left_p)       
        for(size_t j=0; j<num_pairs; j++){
            if(isFinished_in_left_p[j] == 0 && smallest_in_left[j] != largest_in_left[j]){
                size_t mid = (smallest_in_left[j] + largest_in_left[j])/2;
                if(buffp[right_idx[j]] < buffp[mid]){
                    largest_in_left[j] = mid;
                }
                else if(buffp[right_idx[j]] >  buffp[mid]){
                    smallest_in_left[j] = mid + 1;
                }
                else{//equal
                    new_left_idx[j] = mid;
                    isFinished_in_left_p[j] = 1;
                }
            }
            else if(isFinished_in_left_p[j] == 0 && smallest_in_left[j] == largest_in_left[j]) {
                new_left_idx[j] = smallest_in_left[j];
                isFinished_in_left_p[j] = 1;
            }          
        }
        isFinished_all_in_left = true;
   #pragma cdir on_adb(isFinished_in_left_p)         
        for(size_t k=0; k<num_pairs; k++){
            if(isFinished_in_left_p[k] == 0){
                isFinished_all_in_left = 0;
                break;
            }
        }
        if(isFinished_all_in_left == true){
            break;
        }
    }
  


    while(1){
    #pragma cdir on_adb(buffp)
    #pragma cdir on_adb(right_idx)
    #pragma cdir on_adb(new_right_idx)
    #pragma cdir on_adb(smallest_in_right)
    #pragma cdir on_adb(largest_in_right)
    #pragma cdir on_adb(isFinished_in_right_p)       
        for(size_t j=0; j<num_pairs; j++){
            if(isFinished_in_right_p[j] == 0 && smallest_in_right[j] != largest_in_right[j]){
                size_t mid = (smallest_in_right[j] + largest_in_right[j])/2;
                if(buffp[right_idx[j]] < buffp[mid]){
                    largest_in_right[j] = mid;
                }
                else if(buffp[right_idx[j]] >  buffp[mid]){
                    smallest_in_right[j] = mid + 1;
                }
                else{//equal
                    new_right_idx[j] = mid;
                    isFinished_in_right_p[j] = 1;
                }
            }
            else if(isFinished_in_right_p[j] == 0 && smallest_in_right[j] == largest_in_right[j]) {
                new_right_idx[j] = smallest_in_right[j];
                isFinished_in_right_p[j] = 1;
            }          
        }
        isFinished_all_in_right = true;
    #pragma cdir on_adb(isFinished_in_right_p)           
        for(size_t k=0; k<num_pairs; k++){
            if(isFinished_in_right_p[k] == 0){
                isFinished_all_in_right = 0;
                break;
            }
        }
        if(isFinished_all_in_right == true){
            break;
        }
    }   
}

}
#endif  // GRAPH_BINARY_SEARCH_HPP
