#include "lda_document.hpp"
#include "lda_cgs_sampler.hpp"
#include "lda_utility.hpp"

#ifndef _LDA_CGS_HPP_
#define _LDA_CGS_HPP_

namespace frovedis {

// TC: number of tokens in training corpus;
// TD: number of tokens in each training documents;
// TW: number of words;
// TK: number of topics;
// TA: when number of topics is smaller than the range of int16_t, we use compressed alias table format
template <typename TC, typename TD = int32_t, typename TW = int32_t, typename TK = int32_t, typename TA = int16_t>
lda_model<TC> lda_train(
    crs_matrix<TD>& data_train, const double alpha, const double beta, 
    const int num_topics, const int num_iter, const std::string& algorithm, 
    const int num_explore_iter, const int num_eval_cycle) {

    // NOTE: delay_update=false causes error in some case. So we removed this from input params.
    const bool delay_update = true;  

    srand(time(NULL));     
    size_t nsize = get_nodesize(); //MPI_Comm_size
        
#ifdef LDA_CGS_TIMING
    std::chrono::duration<double> t_sampling(0),t_model(0),t_evaluate(0),t_lda(0);
    std::chrono::duration<double> t_generate_corpus(0),t_doc_model(0),t_global_model(0),t_balance(0);
    auto tt2 = std::chrono::system_clock::now();
#endif       
    // ----------------------------------- generate corpus ----------------------------------- //
    size_t num_voc = data_train.num_col;
    auto doc_token_count = make_node_local_broadcast(std::vector<TD>());
    auto local_num_tokens = make_node_local_broadcast(size_t()); 
    lda::lda_config config(beta,alpha,num_voc,num_topics,0,delay_update,algorithm,num_explore_iter,true,num_iter);  
    auto config_l = make_node_local_broadcast(config);
    auto corpus_d = data_train.data.map(lda::lda_document<TD,TW,TK>::gen_corpus_from_crs, config_l, doc_token_count,local_num_tokens).template as_dvector< lda::lda_document<TD,TW,TK> >();
    config_l.mapv(
        +[](lda::lda_config& config, size_t& num_tokens){
            TC tmp_total, local_total = num_tokens; 
            TD tmp_max, local_max = config.max_token;
            typed_allreduce<TC>(&local_total,&tmp_total,1,MPI_SUM,MPI_COMM_WORLD);
            typed_allreduce<TD>(&local_max,&tmp_max,1,MPI_MAX,MPI_COMM_WORLD);
            num_tokens = tmp_total; config.max_token = tmp_max;
        } , local_num_tokens
    ); 
    config = config_l.get(0); auto num_tokens = local_num_tokens.get(0);
    RLOG(TRACE)<<"Finish generating corpus ..."<<std::endl;
    std::vector<size_t> doc_per_thread;
    std::vector<TC> token_per_thread;
    auto token_per_doc = doc_token_count.template moveto_dvector<TD>().gather();
    if(nsize > 1) {
        lda::lda_document<TD,TW,TK>::template distribute_doc<TC>(token_per_doc,num_tokens,doc_per_thread,token_per_thread);
        corpus_d.align_as(doc_per_thread);     
    } else {
        doc_per_thread.push_back(corpus_d.size());
        token_per_thread.push_back(num_tokens);
    } 
    RLOG(TRACE)<<"Finish distributing corpus ..."<<std::endl;
#ifdef LDA_CGS_TIMING
    auto tt1 = std::chrono::system_clock::now();
    t_generate_corpus = tt1 - tt2;
#endif    
    
    RLOG(DEBUG) << "number of documents:  " << corpus_d.size()  << std::endl
               << "number of vocabulary: " << config.num_voc    << std::endl
               << "number of topics:     " << config.num_topics << std::endl
               << "number of tokens:     " << num_tokens  << std::endl
               << "number of tokens max: " << config.max_token  << std::endl;
    for (size_t i=0; i<corpus_d.sizes().size();i++) {RLOG(DEBUG)<<"number of documents on process "<<i<<": "<<corpus_d.sizes()[i]<<std::endl;}
    for (size_t i=0; i<corpus_d.sizes().size();i++) {RLOG(DEBUG)<<"number of tokens on process "<<i<<": "<<token_per_thread[i]<<std::endl;}
    
    // ------------------------------------ compute doc-topic count ------------------------------------ //
    auto corpus_l = corpus_d.viewas_node_local();
    corpus_l.mapv(
        +[](lda::lda_corpus<TD,TW,TK>& corpus, lda::lda_config& config){
            for(size_t i=0; i<corpus.size(); i++) 
                corpus[i].initilize(config.num_topics);
        },
        config_l
    );
#ifdef LDA_CGS_TIMING    
    tt2 = std::chrono::system_clock::now();
    t_doc_model = tt2 - tt1;
#endif        
    RLOG(TRACE)<<"Finish calculating document topic count ..."<<std::endl;   
    
    // ------------------------ get the global model and balance workload -------------------------------- //
    auto model_l = make_node_local_broadcast(lda_model<TC>()); 
    auto model_g = make_node_local_broadcast(lda_model<TC>()); 
    model_l.mapv(
        +[](lda_model<TC>& model1, lda_model<TC>& model2, lda::lda_config& config){
            model1.initialize(config.num_voc,config.num_topics);
            model2.initialize(config.num_voc,config.num_topics);
        },
        model_g, config_l
    );
    corpus_l.mapv(lda::lda_document<TD,TW,TK>::template get_local_model<TC>,model_l);
#ifdef LDA_CGS_SPARSE_MODEL
    auto sparse_model = model_l.map(+[](lda_model<TC>& model){return lda::template rowmajor_to_crs<TC>(model.word_topic_count);});
    sparse_model = sparse_model.allreduce(lda::template sum_crs<TC>); 
    model_l.mapv(lda_model<TC>::mpi_gather_model_sparse,model_g,sparse_model);
#else    
    model_l.mapv(lda_model<TC>::mpi_gather_model,model_g);    
#endif
    
#ifdef LDA_CGS_TIMING        
    tt1 = std::chrono::system_clock::now();
    t_global_model = tt1 - tt2;
#endif    
    RLOG(TRACE)<<"Finish calculating global model ..."<<std::endl;   
    
    // ------------------------------------- balance workload ------------------------------------------- //
    auto sampler_l = make_node_local_broadcast(lda::lda_sampler<TC,TD,TW,TK,TA>());
    sampler_l.mapv(+[](lda::lda_sampler<TC,TD,TW,TK,TA>& sampler, lda::lda_corpus<TD,TW,TK>& corpus, std::vector<TC>& token_per_thread, lda::lda_config& config)
        {sampler.initialize(corpus,token_per_thread,config);},corpus_l,broadcast(token_per_thread),config_l);  
    RLOG(TRACE)<<"Finish balancing workload ..."<<std::endl; 

#ifdef LDA_CGS_TIMING   
    tt2 = std::chrono::system_clock::now();
    t_balance = tt2 - tt1;      
    RLOG(DEBUG) << std::endl
               << "generate corpus:    "<<t_generate_corpus.count()<<std::endl
               << "generate doc model: "<<t_doc_model.count()      <<std::endl
               << "get global model:   "<<t_global_model.count()   <<std::endl
               << "balance workload:   "<<t_balance.count()        <<std::endl
               << "------------------------------------------------------------" <<std::endl;
    auto ws = std::chrono::system_clock::now();  
#endif    
        
    // int eval_times = config.num_itr/10+1;
    int eval_times = 0;
    if (num_eval_cycle > 0) {
        eval_times = config.num_itr/ num_eval_cycle;
    }
    std::vector<double> perplexity(eval_times);  
    std::vector<double> likelihood(eval_times);    
    std::vector<double> lld(eval_times);   
    std::vector<double> timestamp(eval_times);  
    eval_times = 0;
        
    for(int i=1; i<=config.num_itr; i++){        
        RLOG(DEBUG)<<"iteration = "<<i - 1<<"\n";
                
        // ====================================== sampling ====================================== //
#ifdef LDA_CGS_TIMING
        auto w5 = std::chrono::system_clock::now();
#endif                  
#if !(defined(_SX) || defined(__ve__))
        corpus_l.mapv(
            +[](lda::lda_corpus<TD,TW,TK>& corpus,lda::lda_sampler<TC,TD,TW,TK,TA>& sampler,lda_model<TC>& model_g, lda_model<TC>& model_l){
                sampler.sample_for_corpus_unvec(corpus,model_g,model_l);
            },
            sampler_l,model_g,model_l
        );
#else
        corpus_l.mapv(
            +[](lda::lda_corpus<TD,TW,TK>& corpus,lda::lda_sampler<TC,TD,TW,TK,TA>& sampler,lda_model<TC>& model_g, lda_model<TC>& model_l){
                sampler.sample_for_corpus_vec(corpus,model_g,model_l);
            },
            sampler_l,model_g,model_l
        );
#endif
#ifdef LDA_CGS_TIMING
        auto w6 = std::chrono::system_clock::now(); t_sampling += w6-w5;
#endif
        
        // ================================ aggregate model ====================================== //   
#ifdef LDA_CGS_SPARSE_MODEL
        sparse_model = model_l.map(+[](lda_model<TC>& model){return lda::template rowmajor_to_crs<TC>(model.word_topic_count);});
        sparse_model = sparse_model.allreduce(lda::template sum_crs<TC>); 
        model_l.mapv(lda_model<TC>::mpi_gather_model_sparse,model_g,sparse_model);
#else    
        model_l.mapv(lda_model<TC>::mpi_gather_model,model_g);    
#endif
    
#ifdef LDA_CGS_TIMING
        auto w7 = std::chrono::system_clock::now(); t_model += w7-w6; t_lda += w7-w5;
#endif
        
        // ============================ calculate the perplexity =================================== //
        // if(eval && (i%10==0 || i==1)){
        bool to_evaluate = num_eval_cycle > 0 && (i % num_eval_cycle == 0); 
        if (to_evaluate) {
#ifdef LDA_CGS_TIMING
            auto w8 = std::chrono::system_clock::now(); 
            timestamp[eval_times]  = (t_lda.count());      
#endif
            likelihood[eval_times] = (corpus_l.map(lda::template cal_perplexity<TC,TD,TW,TK>,model_g,config_l).reduce(lda::sum<double>)/num_tokens);
            perplexity[eval_times] = (exp(-likelihood[eval_times]));
            double word_ll = model_g.map(lda::template cal_word_likelihood<TC>,config_l).reduce(lda::sum<double>);
            double doc_ll  = corpus_l.map(lda::template cal_doc_likelihood<TD,TW,TK>,config_l).reduce(lda::sum<double>);
            lld[eval_times] = word_ll + doc_ll;
#ifdef LDA_CGS_TIMING 
            auto w9 = std::chrono::system_clock::now();  t_evaluate += w9-w8;
#endif  
            RLOG(DEBUG)<<"perplexity: "<<perplexity[eval_times]<<" || train ll: "<< lld[eval_times] 
#ifdef LDA_CGS_TIMING
            <<" @ "<<timestamp[eval_times]<<" s"
#endif
            <<std::endl;
            eval_times ++;
        } 
    }
    
#ifdef LDA_CGS_TIMING
    RLOG(DEBUG)<< "--------------------timing information-----------------------" << std::endl
              << "sampling:   " << t_sampling.count()  <<std::endl
              << "model_sum:  " << t_model.count()     <<std::endl
              << "CGS-lda:    " << t_lda.count()       <<std::endl
              << "evaluation: " << t_evaluate.count()/(double)eval_times*1000 << std::endl
              << "-------------------------------------------------------------" << std::endl << std::endl;
    auto we = std::chrono::system_clock::now();
    std::chrono::duration<double> total=we-ws;
    RLOG(DEBUG)<< "LDA execution time per iteration: " << total.count()/config.num_itr << std::endl;
    RLOG(DEBUG)<< "-------------------------------------------------------------" << std::endl;
        
# ifdef LDA_CGS_TIMING_SAVE
    std::string str_mhstep = config.algorithm==lda::lda_config::original_cgs? "":"-"+std::to_string(LDA_MH_STEP);
    std::string str_delay_update = config.delay_update? "-delay":"";
    std::string filename = std::to_string(config.num_topics)+"-"+std::to_string(config.num_itr)
        +"-"+std::to_string(nsize)+"-"+std::to_string((int)(config.algorithm))+str_mhstep+str_delay_update;
    std::ofstream fout("results/"+filename+".txt");  
    fout<<"sampling:   "<<t_sampling.count()  <<std::endl;
    fout<<"model:      "<<t_model.count()     <<std::endl;   
    fout<<"CGS-lda:    "<<t_lda.count()       <<std::endl;
    fout<<"evaluation: "<<t_evaluate.count()/(double)eval_times*1000 <<std::endl; 
    fout<<"-----------------------------------------------------------"<<std::endl<<std::endl;
    fout<<"timestamp || likelihood || perplexity || log-likelihood"<<std::endl;
    if(num_eval_cycle > 0){
        for(auto& i:timestamp) fout<<i<<" ";  fout<<std::endl;
        for(auto& i:likelihood) fout<<i<<" "; fout<<std::endl;
        for(auto& i:perplexity) fout<<i<<" "; fout<<std::endl;
        for(auto& i:lld) fout<<i<<" "; fout<<std::endl<<std::endl;
    }
    fout<<"-----------------------------------------------------------"<<std::endl<<std::endl;    
# endif
#endif    
    // ======================== save doc topic count ======================== //
    auto model = model_g.get(0);    
    return model;    
}


template <typename TC, typename TD = int32_t, typename TW = int32_t, typename TK = int32_t, typename TA = int16_t>
void lda_test(crs_matrix<TD>& data_test , const double alpha, const double beta, const int num_iter, std::string& algorithm, 
              const int num_explore_iter, lda_model<TC>& model, 
              std::vector<double>& perplexity, std::vector<double>& likelihood) {
            
    RLOG(TRACE)<<"=================== START TESTING ========================"<<std::endl;    

    // NOTE: delay_update=false causes error in some case. So we removed this from input params.
    const bool delay_update = true;

    srand(time(NULL));  
    size_t nsize = get_nodesize(); //MPI_Comm_size   
    
    size_t num_voc = model.word_topic_count.local_num_row;
    size_t num_topics = model.word_topic_count.local_num_col;
    lda::lda_config config(beta,alpha,num_voc,num_topics,0,delay_update,algorithm,num_explore_iter,false,num_iter); 
    auto config_l = make_node_local_broadcast(config);
    
    RLOG(TRACE)<<"Finish loading dataset ..."<<std::endl;
    
    // ----------------------------------- generate corpus ----------------------------------- //
    auto doc_token_count = make_node_local_broadcast(std::vector<TD>()); 
    auto local_num_tokens = make_node_local_broadcast(size_t()); 
    auto corpus_d = data_test.data.map(lda::lda_document<TD,TW,TK>::gen_corpus_from_crs, config_l, doc_token_count, local_num_tokens).template as_dvector< lda::lda_document<TD,TW,TK> >();
    config_l.mapv(
        +[](lda::lda_config& config, size_t& num_tokens){
            TC tmp_total, local_total = num_tokens; 
            TD tmp_max, local_max = config.max_token;
            typed_allreduce<TC>(&local_total,&tmp_total,1,MPI_SUM,MPI_COMM_WORLD);
            typed_allreduce<TD>(&local_max,&tmp_max,1,MPI_MAX,MPI_COMM_WORLD);
            num_tokens = tmp_total; config.max_token = tmp_max;
        }, local_num_tokens
    );     
    config = config_l.get(0); auto num_tokens = local_num_tokens.get(0);
    RLOG(TRACE)<<"Finish generating corpus ..."<<std::endl;
    std::vector<size_t> doc_per_thread;
    std::vector<TC> token_per_thread;
    auto token_per_doc = doc_token_count.template moveto_dvector<TD>().gather();
    if(nsize > 1) {
        lda::lda_document<TD,TW,TK>::template distribute_doc<TC>(token_per_doc,num_tokens,doc_per_thread,token_per_thread);
        corpus_d.align_as(doc_per_thread);     
    } else {
        doc_per_thread.push_back(corpus_d.size());
        token_per_thread.push_back(num_tokens);
    } 
    
    RLOG(DEBUG) << "number of documents:  " << corpus_d.size()  << std::endl
               << "number of vocabulary: " << config.num_voc    << std::endl
               << "number of topics:     " << config.num_topics << std::endl
               << "number of tokens:     " << num_tokens  << std::endl
               << "number of tokens max: " << config.max_token  << std::endl;
    RLOG(TRACE)<<"Finish distributing corpus ..."<<std::endl;
    for (size_t i=0; i<corpus_d.sizes().size();i++) {RLOG(DEBUG)<<"number of documents on process "<<i<<": "<<corpus_d.sizes()[i]<<std::endl;}
    for (size_t i=0; i<corpus_d.sizes().size();i++) {RLOG(DEBUG)<<"number of tokens on process "<<i<<": "<<token_per_thread[i]<<std::endl;}
    
    // ------------------------------------ compute doc-topic count ------------------------------------ //
    auto corpus_l = corpus_d.viewas_node_local();
    corpus_l.mapv(
        +[](lda::lda_corpus<TD,TW,TK>& corpus, lda::lda_config& config){
            for(size_t i=0; i<corpus.size(); i++) corpus[i].initilize(config.num_topics);
        },
        config_l
    );
    RLOG(TRACE)<<"Finish calculating document topic count ..."<<std::endl;   

    // ------------ balance workload, initialize sampler  ------------ //  
    auto sampler_l=make_node_local_broadcast(lda::lda_sampler<TC,TD,TW,TK,TA>());
    sampler_l.mapv(
        +[](lda::lda_sampler<TC,TD,TW,TK,TA>& sampler, lda::lda_corpus<TD,TW,TK>& corpus, std::vector<TC>& token_per_thread, lda::lda_config& config){
            sampler.initialize(corpus,token_per_thread,config);
        },
        corpus_l,broadcast(token_per_thread),config_l
    );  
        
    // std::vector<double> perplexity;  
    // std::vector<double> likelihood;  
    perplexity = std::vector<double>();
    likelihood = std::vector<double>();
    auto model_l = make_node_local_broadcast(model);
    model.clear();
        
    for(int i=1; i<=num_iter; i++){      
        
        RLOG(TRACE)<<"iteration-"<<i<<"\n";                
        // ====================================== sampling ====================================== //    
#if !(defined(_SX) || defined(__ve__))
        corpus_l.mapv(
            +[](lda::lda_corpus<TD,TW,TK>& corpus,lda::lda_sampler<TC,TD,TW,TK,TA>& sampler,lda_model<TC>& model_g, lda_model<TC>& model_l){
                sampler.sample_for_corpus_unvec(corpus,model_g,model_l);
            },
            sampler_l,model_l,model_l
        );
#else
        corpus_l.mapv(
            +[](lda::lda_corpus<TD,TW,TK>& corpus,lda::lda_sampler<TC,TD,TW,TK,TA>& sampler,lda_model<TC>& model_g, lda_model<TC>& model_l){
                sampler.sample_for_corpus_vec(corpus,model_g,model_l);
            },
            sampler_l,model_l,model_l
        );
#endif      
        // ============================ calculate the perplexity =================================== //
        if(true){
            likelihood.push_back(corpus_l.map(lda::template cal_perplexity<TC,TD,TW,TK>,model_l,config_l).reduce(lda::sum<double>)/num_tokens);
            perplexity.push_back(exp(-likelihood.back()));
            double word_ll = model_l.map(lda::template cal_word_likelihood<TC>,config_l).reduce(lda::sum<double>);
            double doc_ll = corpus_l.map(lda::template cal_doc_likelihood<TD,TW,TK>,config_l).reduce(lda::sum<double>);
            double lld = word_ll + doc_ll;
            RLOG(DEBUG)<<likelihood.back()<<" "<<perplexity.back()<<" "<<lld<<std::endl;
        }       
    }
       
    RLOG(DEBUG)<<"-----------------------------------------------------------"<<std::endl;
    for(auto i:perplexity) RLOG(DEBUG)<<i<<" "; RLOG(DEBUG)<<std::endl;
    for(auto i:likelihood) RLOG(DEBUG)<<i<<" "; RLOG(DEBUG)<<std::endl;
    RLOG(DEBUG)<<"-----------------------------------------------------------"<<std::endl;
      
    // if(save_model) corpus_l.mapv(lda::lda_document<TD,TW,TK>::dump_document,broadcast(dataset_name),broadcast(doc_per_thread),broadcast(false));
}

}  // namespace frovedis
#endif  // _LDA_CGS_HPP_
