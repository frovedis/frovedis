#include "stdlib.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options.hpp>

#include <frovedis.hpp>
#include "frovedis/matrix/rowmajor_matrix.hpp"
#include "frovedis/matrix/crs_matrix.hpp"



#define DOC_COUNT 2000

void append_crs_matrix(frovedis::crs_matrix_local<int>& m1, frovedis::crs_matrix_local<int>& m2){
    for (int i=0; i<m2.idx.size(); i++){
        m1.idx.push_back(m2.idx[i]);
        m1.val.push_back(m2.val[i]);
    } 
    int curr_element = m1.off.back();
    for(int r=1; r<m2.off.size(); r++){
        m1.off.push_back(m2.off[r]+curr_element);
    }
}

int main(int argc, char* argv[]){
    frovedis::use_frovedis use(argc, argv);

    using namespace boost::program_options;
    
    options_description opt("option");
    opt.add_options()
        ("help,h", "print help message")
        ("input,i", value<std::string>(), "path for input data with bag-of-words format")  // see https://archive.ics.uci.edu/ml/datasets/bag+of+words
        ("split_size", value<size_t>()->default_value(0), "size of test split")
        ("train", value<std::string>(), "path for output data for training")
        ("test", value<std::string>(), "path for output data for testing. if the number of test samples is 0, this will not be made.")
        ("binary", "save/load file with binary")
        ;

    variables_map argmap;
    store(command_line_parser(argc,argv).options(opt).allow_unregistered().run(), argmap);
    notify(argmap);                

    std::string input_path;
    std::string train_path;
    std::string test_path;

    uint32_t test_doc = 0, train_doc = 0, num_voc, num_doc;

    test_doc = argmap["split_size"].as<size_t>();

    if(argmap.count("input")){
      input_path = argmap["input"].as<std::string>();
    } else {
      std::cerr << "input is not specified" << std::endl;
      std::cerr << opt << std::endl;
      exit(1);
    }
    if(argmap.count("train")){
      train_path = argmap["train"].as<std::string>();
    } else {
      std::cerr << "train is not specified" << std::endl;
      std::cerr << opt << std::endl;
      exit(1);
    }
    if(argmap.count("test")){
      test_path = argmap["test"].as<std::string>();
    } else if (test_doc > 0) {
      std::cerr << "test is not specified" << std::endl;
      std::cerr << opt << std::endl;
      exit(1);
    }

    bool binary = argmap.count("binary") ? true : false;
    std::ifstream fin(input_path);

    frovedis::rowmajor_matrix_local<int> dataset_train;
    frovedis::rowmajor_matrix_local<int> dataset_test;
    frovedis::crs_matrix_local<int> dataset_train_sparse;
    frovedis::crs_matrix_local<int> dataset_test_sparse;
    
    std::string line;
    std::getline(fin, line);
    std::istringstream ss1(line);
    ss1 >> num_doc;
    assert(test_doc<num_doc);
    train_doc = num_doc-test_doc;
    std::getline(fin, line);
    std::istringstream ss2(line);
    ss2 >> num_voc;    
    std::getline(fin, line);
    
    int left_train = train_doc, left_test = test_doc;
    left_train = left_train < DOC_COUNT? left_train : DOC_COUNT;
    left_test = left_test < DOC_COUNT? left_test : DOC_COUNT;
    dataset_train.set_local_num(left_train, num_voc);
    dataset_train.val.resize(left_train*num_voc);
    if(test_doc!=0){
        dataset_test.set_local_num(left_test, num_voc);
        dataset_test.val.resize(left_train*num_voc);
    }
    
    std::cout<<"training document:" << train_doc << std::endl;
    std::cout<<"test document:" << test_doc << std::endl;
    std::cout<<"words: " << num_voc << std::endl;
    
    int loop_count_test = 1, loop_count_train = 1;
    while (std::getline(fin, line)) {
        std::istringstream ss3(line);
        int word_id, count, doc_id;
        ss3 >> doc_id >> word_id >> count;
        doc_id --; word_id --;
        if(doc_id < test_doc){
            if(doc_id < DOC_COUNT*loop_count_test){
                int doc_idx = doc_id - (loop_count_test-1)*DOC_COUNT;
                dataset_test.val[doc_idx*num_voc+word_id] = count;
            } else {
                if(loop_count_test==1) dataset_test_sparse = dataset_test.to_crs();
                else {
                    auto tmp_test_sparse = dataset_test.to_crs();
                    append_crs_matrix(dataset_test_sparse,tmp_test_sparse);
                }                
                left_test = test_doc - DOC_COUNT*loop_count_test;
                left_test = left_test < DOC_COUNT? left_test:DOC_COUNT;
                dataset_test.local_num_row = left_test;
                dataset_test.val.assign(left_test*num_voc,0);
                int doc_idx = doc_id - loop_count_test*DOC_COUNT;
                dataset_test.val[doc_idx*num_voc+word_id] = count;
                loop_count_test ++;
            }
        } else {
            if(doc_id - test_doc < DOC_COUNT*loop_count_train){
                int doc_idx = doc_id - test_doc - (loop_count_train-1)*DOC_COUNT;
                assert(doc_idx>=0 && doc_idx<DOC_COUNT);
                dataset_train.val[doc_idx*num_voc+word_id] = count;
            } else {
                if(loop_count_train==1) dataset_train_sparse = dataset_train.to_crs();
                else {
                    auto tmp_train_sparse = dataset_train.to_crs();
                    append_crs_matrix(dataset_train_sparse,tmp_train_sparse);
                }   
                left_train = train_doc - DOC_COUNT*loop_count_train;
                left_train = left_train < DOC_COUNT? left_train:DOC_COUNT;
                dataset_train.local_num_row = left_train;
                dataset_train.val.assign(left_train*num_voc,0);
                int doc_idx = doc_id - test_doc - loop_count_train*DOC_COUNT;
                dataset_train.val[doc_idx*num_voc+word_id] = count;
                loop_count_train ++; 
            }
        }
    }
    
    if(loop_count_train==1) dataset_train_sparse = dataset_train.to_crs();    
    else {
        auto tmp_train_sparse = dataset_train.to_crs();
        append_crs_matrix(dataset_train_sparse,tmp_train_sparse);
    }
    
    dataset_train_sparse.set_local_num(num_voc);
    if (binary) {
        dataset_train_sparse.savebinary(train_path);
    } else {
        make_crs_matrix_scatter(dataset_train_sparse).save(train_path);
    }
    assert(dataset_train_sparse.local_num_row==train_doc);
    
    if(test_doc != 0) {    
        if(loop_count_test==1)  dataset_test_sparse = dataset_test.to_crs();
        else {
            auto tmp_test_sparse = dataset_test.to_crs();
            append_crs_matrix(dataset_test_sparse,tmp_test_sparse);
        }
        dataset_test_sparse.set_local_num(num_voc);
        if (binary) {
            dataset_test_sparse.savebinary(test_path);
        } else {
            make_crs_matrix_scatter(dataset_test_sparse).save(test_path);
        }
        assert(dataset_test_sparse.local_num_row==test_doc);
    }
    
}
