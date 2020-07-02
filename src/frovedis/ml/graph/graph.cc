
#include <frovedis/matrix/jds_crs_hybrid.hpp>
#include <frovedis/matrix/jds_matrix.hpp>
#include <frovedis/core/utility.hpp>
#include "graph.hpp"

namespace frovedis {
    

const double hyb_th_perc = 0.01;
const size_t hyb_th = 500000;

std::vector<size_t>
get_vsize(std::vector<TYPE_IDXV>& v_local) {
    std::vector<size_t> vsize(1);
    vsize[0] = v_local.size();
    return vsize;
}


size_t
collect_local_num_row (frovedis::crs_matrix_local<TYPE_MATRIX>& m){
    return m.local_num_row;
}




graph::graph():t_all(TIME_RECORD_LOG_LEVEL), 
        t_load(TIME_RECORD_LOG_LEVEL), 
        t_compute_spmv(TIME_RECORD_LOG_LEVEL),
        t_compute_other(TIME_RECORD_LOG_LEVEL),
        t_comm(TIME_RECORD_LOG_LEVEL)
  {
    if_fout=false;
    if_cout=false;
}

graph::graph(const graph& orig) {
}

graph::~graph() {
}


#if 0
void graph::cc_bfs_spmv(std::string matformat){    //Breath-first level-sync algorithm
    
    //1D partition
    //For undirect graph matrix(symmetric)
    //BitMap for current_nodes, next_nodes and visited_nodes to enable parallel computation
    //Dense vector for bitmap vectors
    
    frovedis::jds_crs_hybrid<TYPE_MATRIX> A_hyb;
    frovedis::jds_matrix<TYPE_MATRIX> A_jds;
    if(matformat == "HYB"){
        RLOG(TIME_RECORD_LOG_LEVEL)<<"Converting CRS Matrix to HYB...\n";    
        A_hyb = frovedis::jds_crs_hybrid<TYPE_MATRIX>(A);       
    }
    else if(matformat == "JDS"){
        RLOG(TIME_RECORD_LOG_LEVEL)<<"Converting CRS Matrix to JDS...\n";    
        A_jds = frovedis::jds_matrix<TYPE_MATRIX>(A);  
    }
    t_all.lap_start();
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Running BFS_spMV..."<<std::endl;    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"---------------------------"<<std::endl;     
    this->nodes_ifVisited.assign(num_nodes,0);
    this->num_cc = 0;    
    this->num_nodes_in_each_cc.assign(num_nodes,0);
    this->nodes_in_which_cc.assign(num_nodes,0);
    this->nodes_dist.assign(num_nodes,0);
    this->nodes_ifCurrent.assign(num_nodes,0);
    this->nodes_ifNext.assign(num_nodes,0);      
    auto* nodes_in_cc_ptr = &this->nodes_in_which_cc[0];
    auto* nodes_dist_ptr = &this->nodes_dist[0];

    for(size_t m = 0;m < num_nodes; m++){
        if(this->nodes_ifVisited[m] == 0){           
            this->num_nodes_in_each_cc[num_cc++] = 0;       
            this->source_nodeID = 1;
                     
            RLOG(TIME_RECORD_LOG_LEVEL)<<"\nccID = "<<this->num_cc<<std::endl;
            RLOG(TIME_RECORD_LOG_LEVEL)<<"source_vertexD = "<<this->source_nodeID<<std::endl;            


            for(size_t i=0; i<num_nodes; i++){
                nodes_ifNext[i] = 0; 
            }
            
            size_t num_nodes_next = 1;
            auto* nodes_ifCurrent_ptr = &this->nodes_ifCurrent[0];
            
            this->nodes_ifNext[source_nodeID] = 1;
            this->current_level = 0;

            while( num_nodes_next > 0){
       
                RLOG(TIME_RECORD_LOG_LEVEL)<<"**** level = "<<this->current_level<<" ****"<<std::endl;
                RLOG(TIME_RECORD_LOG_LEVEL)<<"num of nodes in this level= "<<num_nodes_next<<std::endl;

                this->nodes_ifCurrent = this->nodes_ifNext;
                
                auto lnodes_current = frovedis::make_node_local_broadcast(this->nodes_ifCurrent); //broadcast B 
                frovedis::node_local<std::vector<TYPE_BITMAP>> nodes_next_temp_local;
                
                if(matformat == "CRS"){
                    nodes_next_temp_local = A.data.map(spMV_crs<TYPE_MATRIX,TYPE_BITMAP>,lnodes_current);
                }else if (matformat == "JDS"){
                    nodes_next_temp_local = A_jds.data.map(spMV_jds<TYPE_MATRIX,TYPE_BITMAP>,lnodes_current);
                }else if (matformat == "HYB"){
                    nodes_next_temp_local = A_hyb.data.map(spMV_hyb<TYPE_MATRIX,TYPE_BITMAP>,lnodes_current);                    
                }
                
                
                auto nodes_next_temp = nodes_next_temp_local.moveto_dvector<TYPE_BITMAP>().gather();

                update_ifVisited(this->nodes_ifCurrent, this->nodes_ifVisited);                
                this->nodes_ifNext = check_ifVisited_bitmap(nodes_next_temp,this->nodes_ifVisited);
              
                //assign current node the cc id
                for(size_t i=0;i<num_nodes;i++){
                    nodes_in_cc_ptr[i] += nodes_ifCurrent_ptr[i]*source_nodeID;
                }
                //assign current node the distance to source node;
                for(size_t i=0;i<num_nodes;i++){
                    nodes_dist_ptr[i] += nodes_ifCurrent_ptr[i]*this->current_level;
                }                
                
                this->num_nodes_in_each_cc[num_cc] += num_nodes_next;
                num_nodes_next=0;
                for(size_t i=0; i<num_nodes; i++){
                    num_nodes_next += nodes_ifNext[i];
                }
                this->current_level ++;
            }
        }
    }
    RLOG(TIME_RECORD_LOG_LEVEL)<<"---------------------------"<<std::endl;   
    t_all.lap_stop();
    this->print_summary_cc();
    this->print_exectime();
    
}
#endif



void graph::read_graph(std::string gp, bool if_binary){
;
    graph_path = gp;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Reading CRS graph matrix from   "<<this->graph_path;    
    
    t_load.lap_start();
    if(if_binary == false){
        RLOG(TIME_RECORD_LOG_LEVEL)<<" (string file)"<<std::endl;

        A = frovedis::make_crs_matrix_load<TYPE_MATRIX>(graph_path); //load distributed graph
    }
    else{
        RLOG(TIME_RECORD_LOG_LEVEL)<<" (binary files)"<<std::endl;
        A = frovedis::make_crs_matrix_loadbinary<TYPE_MATRIX>(graph_path); //load distributed graph        
    }
    t_load.lap_stop();
    
    num_nodes = A.num_row;

    RLOG(TIME_RECORD_LOG_LEVEL)<<"num_row= "<<A.num_row<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"num_col= "<<A.num_col<<std::endl;
    
    assert(A.num_col==A.num_row); //make sure it's a square matrix
 
    std::string ftrace_path = graph_path +"_trace";
    if(this->if_fout == true){
        fout.open(ftrace_path,std::ios::out);
        if(fout.is_open() == true){
            RLOG(TIME_RECORD_LOG_LEVEL)<<"Trace file opened: "<<ftrace_path<<std::endl;
            fout<<"cc_idx\t"
                <<"level_idx\t"
                <<"num_current_nodes\t"
                <<"num_nodes_in_this_cc_acc"
                <<std::endl;
        }
    }       
    
}

void graph::read_graph_pagerank(std::string gp, bool if_binary){

    graph_path = gp;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Reading CRS graph matrix from    "<<this->graph_path;    
    t_load.lap_start(); 
    if(if_binary == false){
        RLOG(TIME_RECORD_LOG_LEVEL)<<" (string file)"<<std::endl;
        A_pg = frovedis::make_crs_matrix_load<TYPE_MATRIX_PAGERANK>(graph_path); //load distributed graph
    }
    else{
        RLOG(TIME_RECORD_LOG_LEVEL)<<" (binary files)"<<std::endl;
        A_pg = frovedis::make_crs_matrix_loadbinary<TYPE_MATRIX_PAGERANK>(graph_path); //load distributed graph        
    }  
    t_load.lap_stop();    
    
    num_nodes = A_pg.num_row;


    RLOG(TIME_RECORD_LOG_LEVEL)<<"num_row= "<<A_pg.num_row<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"num_col= "<<A_pg.num_col<<std::endl;
    
    assert(A_pg.num_col==A_pg.num_row); //make sure it's a square matrix
 
    std::string ftrace_path = graph_path +"_trace";
    if(this->if_fout == true){
        fout.open(ftrace_path,std::ios::out);
        if(fout.is_open() == true){
            RLOG(TIME_RECORD_LOG_LEVEL)<<"Trace file opened: "<<ftrace_path<<std::endl;
            fout<<"cc_idx\t"
                <<"level_idx\t"
                <<"num_current_nodes\t"
                <<"num_nodes_in_this_cc_acc"
                <<std::endl;
        }
    }       
    
}


void graph::cc_bfs_scatter(){    //Breath-first level-sync algorithm
    //only gather idx for nodes_next_temp to reduce comm overhead

    RLOG(TIME_RECORD_LOG_LEVEL)<<"converting matrix from crs to ccs.."<<std::endl;
    frovedis::ccs_matrix<TYPE_MATRIX> A_ccs(A);
    t_all.lap_start();
    auto local_num_row = A.data.map(collect_local_num_row).gather();
    
    mat_offset.resize(local_num_row.size());
    mat_offset[0] = 0;
    for(size_t i=1; i< local_num_row.size();i++){
        mat_offset[i] = mat_offset[i-1] + local_num_row[i-1];
    }
    
    t_comm.lap_start();
    auto lmat_offset = frovedis::make_node_local_broadcast(mat_offset);
    t_comm.lap_stop();

    RLOG(TIME_RECORD_LOG_LEVEL)<<"Running BFS_Scatter..."<<std::endl; 
    
    this->nodes_ifVisited.assign(num_nodes,0);
    this->num_cc = 0;    
    this->num_nodes_in_each_cc.assign(num_nodes,0);
    this->nodes_in_which_cc.assign(num_nodes,0);
    this->nodes_dist.assign(num_nodes,0);
    
    auto* nodes_in_which_cc_ptr = &this->nodes_in_which_cc[0];
    auto* nodes_dist_ptr = &this->nodes_dist[0];
    
    for(size_t m = 0;m < num_nodes; m++){
        if(this->nodes_ifVisited[m] == 0){
                          
            this->source_nodeID = m;          
            RLOG(TIME_RECORD_LOG_LEVEL)<<"\nccID = "<<this->num_cc<<std::endl;
            RLOG(TIME_RECORD_LOG_LEVEL)<<"source_vertexD = "<<this->source_nodeID<<std::endl;            

            size_t num_nodes_next = 1;
            size_t num_nodes_current;

            nodes_Next_sp.idx.assign(1,source_nodeID);
            nodes_Next_sp.val.assign(1,1);
            
            this->current_level = 0;

            while( num_nodes_next > 0){
                
                num_nodes_current = num_nodes_next;

                RLOG(TIME_RECORD_LOG_LEVEL)<<"**** level = "<<this->current_level<<" ****"<<std::endl;
                RLOG(TIME_RECORD_LOG_LEVEL)<<"num of nodes in this level= "<<num_nodes_current<<std::endl;

                this->nodes_Current_sp = this->nodes_Next_sp;
                
                t_comm.lap_start();
                auto lnodes_current_sp = frovedis::make_node_local_broadcast(this->nodes_Current_sp); //broadcast B 
                t_comm.lap_stop();
                
                t_compute_spmv.lap_start();
                auto nodes_next_idx_temp_local = A_ccs.data.map(spMspV_scatter_spv_idx<TYPE_MATRIX, TYPE_IDXV>,lnodes_current_sp,lmat_offset); 
                t_compute_spmv.lap_stop();
                
                t_comm.lap_start();
                auto nodes_next_idx_temp = nodes_next_idx_temp_local.moveto_dvector<TYPE_IDXV>().gather();
                t_comm.lap_stop();
                
                t_compute_other.lap_start();
                update_ifVisited_sp(this->nodes_Current_sp, this->nodes_ifVisited);    
                this->nodes_Next_sp = check_ifVisited_spV(nodes_next_idx_temp,this->nodes_ifVisited);          
                //assign current node the cc id
                for(size_t i=0;i<num_nodes_current;i++){
                    nodes_in_which_cc_ptr[nodes_Current_sp.idx[i]] = source_nodeID;
                }
//                //assign current node the distance to source node;
                for(size_t i=0;i<num_nodes_next;i++){
                    nodes_dist_ptr[nodes_Current_sp.idx[i]] = this->current_level;
                }                
                
                this->num_nodes_in_each_cc[num_cc] += num_nodes_current;
                num_nodes_next = this->nodes_Next_sp.idx.size();                
                t_compute_other.lap_stop();
                
                this->current_level ++;
            }
            
            this->num_cc ++;    
        }
    }
    t_all.lap_stop();
    this->print_summary_cc();
    this->print_exectime();
    
    
}


void graph::cc_bfs_idxsort(size_t threshold, bool if_hyb){    //Breath-first level-sync algorithm

    RLOG(TIME_RECORD_LOG_LEVEL)<<"converting matrix from crs to ccs.."<<std::endl;
    frovedis::ccs_matrix<TYPE_MATRIX> A_ccs(A);
    t_all.lap_start();
    
    this->hyb_threshold = threshold;

    auto local_num_row = A.data.map(collect_local_num_row).gather();

    mat_offset.resize(local_num_row.size());
     mat_offset[0] = 0;
    for(size_t i=1; i< local_num_row.size();i++){
        mat_offset[i] = mat_offset[i-1] + local_num_row[i-1];
    }
    t_comm.lap_start();
    auto lmat_offset = frovedis::make_node_local_broadcast(mat_offset);
    t_comm.lap_stop();
    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Running BFS_idxSort..."<<std::endl;    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"if_hyb (scatter + idxSort) = "<<if_hyb<<std::endl;
    if(if_hyb == 1){
        RLOG(TIME_RECORD_LOG_LEVEL)<<"hybrid threshold = "<<this->hyb_threshold<<" vertices"<<std::endl;
    }
    this->nodes_ifVisited.assign(num_nodes,0);
    this->num_cc = 0;    
    this->num_nodes_in_each_cc.assign(num_nodes,0);
    this->nodes_in_which_cc.assign(num_nodes,0);
    this->nodes_dist.assign(num_nodes,0);
    
    auto* nodes_in_which_cc_ptr = &this->nodes_in_which_cc[0];
    auto* nodes_dist_ptr = &this->nodes_dist[0];
    
    for(size_t m = 0;m < num_nodes; m++){
        if(this->nodes_ifVisited[m] == 0){
                    
            this->num_nodes_in_each_cc[num_cc] = 0;
            
            this->source_nodeID = m;
            
            RLOG(TIME_RECORD_LOG_LEVEL)<<"\nccID = "<<this->num_cc<<std::endl;
            RLOG(TIME_RECORD_LOG_LEVEL)<<"source_vertexD = "<<this->source_nodeID<<std::endl;            
             
            size_t num_nodes_next = 1;
            size_t num_nodes_current;
            this->nodes_Next_sp.idx.assign(1,source_nodeID);    
            this->nodes_Next_sp.val.assign(1,1);

            
            this->current_level = 0;

            while( num_nodes_next > 0){
                num_nodes_current = num_nodes_next;
                RLOG(TIME_RECORD_LOG_LEVEL)<<"**** level = "<<this->current_level<<" ****"<<std::endl;
                RLOG(TIME_RECORD_LOG_LEVEL)<<"num of nodes in this level= "<<num_nodes_current<<std::endl;
                this->nodes_Current_sp = this->nodes_Next_sp;
                
                t_comm.lap_start();              
                auto lnodes_current_sp = frovedis::make_node_local_broadcast(this->nodes_Current_sp); //broadcast B 
                t_comm.lap_stop();
                
                frovedis::node_local< std::vector<TYPE_IDXV>> nodes_next_idx_temp_local;
                t_compute_spmv.lap_start();
                if(if_hyb == 1 && num_nodes_next > hyb_threshold){
                     nodes_next_idx_temp_local = A_ccs.data.map(spMspV_scatter_spv_idx<TYPE_MATRIX, TYPE_IDXV>,lnodes_current_sp,lmat_offset);
                }
                else{
                     nodes_next_idx_temp_local = A_ccs.data.map(spMspV_idxSort<TYPE_MATRIX, TYPE_IDXV>,lnodes_current_sp,lmat_offset);
                }
                t_compute_spmv.lap_stop();          
                
                t_comm.lap_start();
                auto nodes_next_idx_temp = nodes_next_idx_temp_local.moveto_dvector<TYPE_IDXV>().gather();
                t_comm.lap_stop();
                
                t_compute_other.lap_start();
                update_ifVisited_sp(this->nodes_Current_sp, this->nodes_ifVisited);                
                this->nodes_Next_sp = check_ifVisited_spV(nodes_next_idx_temp,this->nodes_ifVisited);
             
                //assign current node the cc id
                for(size_t i=0;i<num_nodes_current;i++){
                    nodes_in_which_cc_ptr[nodes_Current_sp.idx[i]] = m;
                }
//                //assign current node the distance to source node;
                for(size_t i=0;i<num_nodes_next;i++){
                    nodes_dist_ptr[nodes_Current_sp.idx[i]] = this->current_level;
                }                
                
                this->num_nodes_in_each_cc[num_cc] += num_nodes_current;
                num_nodes_next = this->nodes_Next_sp.idx.size();                
                t_compute_other.lap_stop();
                
                this->current_level ++;
            }
            this->num_cc ++;    
        }
    }
    t_all.lap_stop();
    this->print_summary_cc();
    this->print_exectime();
    
    
}


void graph::cc_bfs_2dmerge_gather(size_t merge_th, bool if_scale_free, bool if_hyb){    //Breath-first level-sync algorithm


    RLOG(TIME_RECORD_LOG_LEVEL)<<"converting matrix from crs to ccs.."<<std::endl;
    frovedis::ccs_matrix<TYPE_MATRIX> A_ccs(A);
    t_all.lap_start();
    auto local_num_row = A.data.map(collect_local_num_row).gather();
    auto* local_num_row_ptr = &local_num_row[0];

    size_t num_worker = local_num_row.size();
    mat_offset.resize(num_worker);
    auto* mat_offset_ptr = &mat_offset[0];
    mat_offset_ptr[0] = 0;

    for(size_t i=1; i< num_worker;i++){
        mat_offset_ptr[i] = mat_offset_ptr[i-1] + local_num_row_ptr[i-1];
    }
    t_comm.lap_start();
    auto lmat_offset = frovedis::make_node_local_broadcast(mat_offset);
    auto lmerge_th = frovedis::make_node_local_broadcast(merge_th);
    t_comm.lap_stop();
    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Running cc_BFS_2DMerge_Gather... "<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"If scale-free graphs = "<<if_scale_free<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"If hybrid strategy = "<<if_hyb<<std::endl;   
    
    this->nodes_ifVisited.assign(num_nodes,0);
    auto* nodes_ifVisited_ptr=&nodes_ifVisited[0];
    this->num_cc = 0;    
    this->num_nodes_in_each_cc.assign(num_nodes,0);
    this->nodes_in_which_cc.assign(num_nodes,0);
    this->nodes_dist.assign(num_nodes,0);
    

    auto* nodes_in_which_cc_ptr = &this->nodes_in_which_cc[0];
    auto* nodes_dist_ptr = &this->nodes_dist[0];
  
    for(size_t m = 0;m < num_nodes; m++){
        if(nodes_ifVisited_ptr[m] == 0){
                    
            this->num_nodes_in_each_cc[num_cc] = 0;
            
            this->source_nodeID = m; 
           
            RLOG(TIME_RECORD_LOG_LEVEL)<<"\nccID = "<<this->num_cc<<std::endl;
            RLOG(TIME_RECORD_LOG_LEVEL)<<"source_vertexD = "<<this->source_nodeID<<std::endl;            
     
            size_t num_nodes_next = 1;
            size_t num_nodes_current;

            this->nodes_Next_idx.assign(1,m);
            this->current_level = 0;

            while( num_nodes_next > 0){
                num_nodes_current = num_nodes_next;

                RLOG(TIME_RECORD_LOG_LEVEL)<<"**** level = "<<this->current_level<<" ****"<<std::endl;
                RLOG(TIME_RECORD_LOG_LEVEL)<<"num of nodes in this level= "<<num_nodes_current<<std::endl;

                this->nodes_Current_idx = this->nodes_Next_idx;
                
                t_comm.lap_start();
                auto lnodes_current_idx = frovedis::make_node_local_broadcast(this->nodes_Current_idx); //broadcast Frontiers 
                t_comm.lap_stop();

                frovedis::node_local< std::vector<TYPE_IDXV>> nodes_next_idx_temp_local;                
                
                t_compute_spmv.lap_start();
                if(if_scale_free == 1){ //For low-diameter graphs
                    if(if_hyb == 0){
                     nodes_next_idx_temp_local = A_ccs.data.map(spMspV_2DMerge_idx_scalefree<TYPE_MATRIX, TYPE_IDXV>,lnodes_current_idx,lmat_offset,lmerge_th);
                    }
                    else{
                     nodes_next_idx_temp_local = A_ccs.data.map(spMspV_2DMerge_idx_Hyb<TYPE_MATRIX, TYPE_IDXV>,lnodes_current_idx,lmat_offset,lmerge_th); 
                    }
                }
                else{  //For high-diameter graphs
                    nodes_next_idx_temp_local = A_ccs.data.map(spMspV_2DMerge_idx<TYPE_MATRIX, TYPE_IDXV>,lnodes_current_idx,lmat_offset,lmerge_th);
                }
                t_compute_spmv.lap_stop();
                
                t_comm.lap_start();
                auto nodes_next_idx_temp = nodes_next_idx_temp_local.moveto_dvector<TYPE_IDXV>().gather();
                t_comm.lap_stop();
                
                t_compute_other.lap_start();
                update_ifVisited_idx(this->nodes_Current_idx, this->nodes_ifVisited);                
                this->nodes_Next_idx = check_ifVisited_idx(nodes_next_idx_temp,this->nodes_ifVisited);
              
                //assign current node the cc id
                auto* idxp = &nodes_Current_idx[0];
                #pragma cdir nodep
                for(size_t i=0;i<num_nodes_current;i++){
                    nodes_in_which_cc_ptr[idxp[i]] = m;
                }
                //assign current node the distance to source node;
                #pragma cdir nodep              
                for(size_t i=0;i<num_nodes_next;i++){
                    nodes_dist_ptr[idxp[i]] = this->current_level;
                }                
                
                this->num_nodes_in_each_cc[num_cc] += num_nodes_current;
                num_nodes_next = this->nodes_Next_idx.size();                   
                t_compute_other.lap_stop();
                
                this->current_level ++;

            }
            this->num_cc ++;    
        }
    }
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Finished!\n";
    t_all.lap_stop();
    this->print_summary_cc();
    this->print_exectime();
}

#if 0
void graph::cc_bfs_2dmerge_allgather(size_t merge_th){    //Breath-first level-sync algorithm
    //Update and check ifVisited locally
    //Using Allgather to reduce communication latency.

    RLOG(TIME_RECORD_LOG_LEVEL)<<"converting matrix from crs to ccs.."<<std::endl;
    frovedis::ccs_matrix<TYPE_MATRIX> A_ccs(A);
    t_all.lap_start();
    auto local_num_row = A.data.map(collect_local_num_row).gather();

    mat_offset.resize(local_num_row.size());
    mat_offset[0] = 0;
    for(size_t i=1; i< local_num_row.size();i++){
        mat_offset[i] = mat_offset[i-1] + local_num_row[i-1];
    }
    this->nodes_ifVisited.assign(num_nodes,0);

    t_comm.lap_start();
    auto lmat_offset = frovedis::make_node_local_broadcast(mat_offset);
    auto lmerge_th = frovedis::make_node_local_broadcast(merge_th);
    t_comm.lap_stop();
    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Running cc_BFS_2DMerge_AllGather.."<<std::endl;    

    this->num_cc = 0;    
    this->num_nodes_in_each_cc.assign(num_nodes,0);
    this->nodes_in_which_cc.assign(num_nodes,0);
    this->nodes_dist.assign(num_nodes,0);
    
    
    for(size_t m = 0;m < num_nodes; m++){
        
        if(this->nodes_ifVisited[m] == 0){
            t_comm.lap_start();
            auto nodes_ifVisited_dv = frovedis::make_dvector_scatter(this->nodes_ifVisited,local_num_row);
            auto lnodes_ifVisited = nodes_ifVisited_dv.as_node_local();              
            t_comm.lap_stop();
            
            this->num_nodes_in_each_cc[num_cc] = 0;
            this->source_nodeID = m;
            
            RLOG(TIME_RECORD_LOG_LEVEL)<<"\nccID = "<<this->num_cc<<std::endl;
            RLOG(TIME_RECORD_LOG_LEVEL)<<"source_vertexD = "<<this->source_nodeID<<std::endl;            
                  
            size_t num_nodes_next = 1;
            size_t num_nodes_current;

            this->nodes_Next_idx.resize(1);            
            this->nodes_Next_idx[0] = m;           
            this->current_level = 0;
            
            t_comm.lap_start();
            auto nodes_next_idx_ag = frovedis::make_node_local_broadcast(this->nodes_Next_idx); //broadcast B 
            t_comm.lap_stop();
            
            while( num_nodes_next > 0){
                num_nodes_current = num_nodes_next;
                RLOG(TIME_RECORD_LOG_LEVEL)<<"**** level = "<<this->current_level<<" ****"<<std::endl;
                RLOG(TIME_RECORD_LOG_LEVEL)<<"num of vectors-to-merge in this level= "<<num_nodes_current<<std::endl;
                t_compute_spmv.lap_start();
                auto nodes_next_idx_local = A_ccs.data.map(spMspV_2DMerge_unvisited<TYPE_MATRIX, TYPE_IDXV, TYPE_BITMAP>,nodes_next_idx_ag,lmat_offset,lmerge_th,lnodes_ifVisited);
                t_compute_spmv.lap_stop();
                
                t_compute_other.lap_start();
                nodes_next_idx_ag = nodes_next_idx_local.map(Allgather_next_idx<TYPE_IDXV>);
                t_compute_other.lap_stop();
                
                t_comm.lap_start();
                auto num_next_idx_each = nodes_next_idx_ag.map(get_vsize).moveto_dvector<size_t>().gather();
                num_nodes_next = num_next_idx_each[0];                  
                t_comm.lap_stop();
                
                this->current_level ++;
            }           
            
            t_comm.lap_start();
            nodes_ifVisited = lnodes_ifVisited.moveto_dvector<TYPE_BITMAP>().gather(); 
            t_comm.lap_stop();
            
            this->num_cc ++;    
        }
    }
    t_all.lap_stop();
    this->print_summary_cc();
    this->print_exectime();
}
#endif

void graph::print_summary_cc(){
    RLOG(TIME_RECORD_LOG_LEVEL)<<"*************SUMMARY***************"<<std::endl;

    RLOG(TIME_RECORD_LOG_LEVEL)<<"num of cc= "<<this->num_cc;
    
    size_t num_cc_printed = 20;
    if(num_cc < num_cc_printed) num_cc_printed = num_cc;
    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"\nnum_nodes_in_each_cc: (printing the first "<<num_cc_printed<<")"<<std::endl;
    for(size_t i=0;i<num_cc_printed;i++){
            RLOG(TIME_RECORD_LOG_LEVEL)<<i<<":"<<this->num_nodes_in_each_cc[i]<<"\t";
    }

    if(if_cout == true){
        RLOG(TIME_RECORD_LOG_LEVEL)<<"\nnodes_in_which_cc: "<<std::endl;    
        for(size_t i=0;i<num_nodes;i++){
                RLOG(TIME_RECORD_LOG_LEVEL)<<i<<":"<<this->nodes_in_which_cc[i]<<"\t";
        }
        RLOG(TIME_RECORD_LOG_LEVEL)<<"\nnodes_dist: "<<std::endl;
        for(size_t i=0;i<num_nodes;i++){
                RLOG(TIME_RECORD_LOG_LEVEL)<<i<<":"<<this->nodes_dist[i]<<"\t";
        }    
        RLOG(TIME_RECORD_LOG_LEVEL)<<std::endl;
    }
    if(this->if_fout == true && fout.is_open()){
        fout<<"\nnum_nodes_in_each_cc:"<<num_cc<<std::endl;
        fout<<"cc_idx\t"
            <<"num_nodes"
            <<std::endl;
        for(size_t i=0;i<num_cc;i++){
                fout<<i<<"\t"<<this->num_nodes_in_each_cc[i]<<"\n";
        }
    } 
    RLOG(TIME_RECORD_LOG_LEVEL)<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"***********************************"<<std::endl;    
}



void graph::print_exectime(){
    
    RLOG(TIME_RECORD_LOG_LEVEL) <<"\n---------------- Execution Time Recorder ----------------"<< std::endl;  

    RLOG(TIME_RECORD_LOG_LEVEL)<<"time_load\t"
            <<"time_compute_spmv\t"
            <<"time_compute_other\t"
            <<"time_comm\t"
            <<"time_other\t"
            <<"time_all\t"
            <<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<t_load.show_lap_value()<<"\t"
            <<t_compute_spmv.show_lap_value()<<"\t"
            <<t_compute_other.show_lap_value()<<"\t"
            <<t_comm.show_lap_value()<<"\t"
            <<t_all.show_lap_value()-t_compute_spmv.show_lap_value()-t_compute_other.show_lap_value()-t_comm.show_lap_value()<<"\t"
            <<t_all.show_lap_value()<<"\t"
            <<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL) <<"\n---------------------------------------------------------"<< std::endl;             
    std::string fout_path_exectime = graph_path +"_exectime";
    fout_exectime.open(fout_path_exectime,std::ios::app);
    if(fout_exectime.is_open()){
    fout_exectime<<t_load.show_lap_value()<<"\t"
            <<t_compute_spmv.show_lap_value()<<"\t"
            <<t_compute_other.show_lap_value()<<"\t"
            <<t_comm.show_lap_value()<<"\t"
            <<t_all.show_lap_value()-t_load.show_lap_value()-t_compute_spmv.show_lap_value()-t_compute_other.show_lap_value()-t_comm.show_lap_value()<<"\t"
            <<t_all.show_lap_value()-t_load.show_lap_value()<<"\t"
            <<t_all.show_lap_value()<<"\t"
            <<std::endl;
    }   
}

//void graph::print_exectime_cc(){
//    time_stop_s=frovedis::get_dtime();    
//    this->time_all = time_stop_s - time_start_s;
//    this->time_other = this->time_all - this->time_load - this->time_comm - this->time_compute;
//
//    RLOG(TIME_RECORD_LOG_LEVEL) <<"\n*****CC Execution Time Decomposition****"<< std::endl;  
//
//    RLOG(TIME_RECORD_LOG_LEVEL)<<"time_load\t"
//            <<"time_compute_spmv\t"
//            <<"time_compute_findNodesNext\t"
//            <<"time_compute_assignID\t"
////            <<"time_compute_other\t"
//            <<"time_comm_bcast\t"
//            <<"time_comm_collect\t"
//            <<"time_other\t"
//            <<"time_all\t"
//            <<"time_all_excl_load\t"
//            <<"time_compute\t"
//            <<"time_comm\t"
//            <<std::endl;
//    RLOG(TIME_RECORD_LOG_LEVEL)<<time_load<<"\t"
//            <<time_compute_spmv<<"\t"
//            <<time_compute_findNodesNext<<"\t"
//            <<time_compute_assignID<<"\t"            
////            <<time_compute_other<<"\t"
//            <<time_comm_bcast<<"\t"
//            <<time_comm_collect<<"\t"
//            <<time_other<<"\t"
//            <<time_all<<"\t"
//            <<time_all-time_load<<"\t"
//            <<time_compute<<"\t"
//            <<time_comm<<"\t"
//            <<std::endl;
//    
//    std::string fout_path_exectime = graph_path +"_exectime";
//    fout_exectime.open(fout_path_exectime,std::ios::app);
//    if(fout_exectime.is_open()){
//    fout_exectime<<time_load<<"\t"
//            <<time_compute_spmv<<"\t"
//            <<time_compute_findNodesNext<<"\t"
//            <<time_compute_assignID<<"\t"               
////            <<time_compute_other<<"\t"
//            <<time_comm_bcast<<"\t"
//            <<time_comm_collect<<"\t"
//            <<time_other<<"\t"
//            <<time_all<<"\t"
//            <<time_all-time_load<<"\t"
//            <<time_compute<<"\t"
//            <<time_comm<<"\t"            
//            <<std::endl;    }
//}
//
//void graph::print_exectime_pagerank(){
//    time_stop_s=frovedis::get_dtime();    
//    this->time_all = time_stop_s - time_start_s;
//    this->time_other = this->time_all - this->time_load - this->time_comm - this->time_compute;
//
//
//    RLOG(TIME_RECORD_LOG_LEVEL) <<"\n*****CC Execution Time Decomposition****"<< std::endl;  
//
//    RLOG(TIME_RECORD_LOG_LEVEL)<<"time_load\t"
//            <<"time_compute_spmv\t"
//            <<"time_compute_diff\t"
//            <<"time_comm_bcast\t"
//            <<"time_comm_collect\t"
//            <<"time_other\t"
//            <<"time_all\t"
//            <<"time_all_excl_load\t"
//            <<"time_compute\t"
//            <<"time_comm\t"
//            <<std::endl;
//    RLOG(TIME_RECORD_LOG_LEVEL)<<time_load<<"\t"
//            <<time_compute_spmv<<"\t"           
//            <<time_compute_diff<<"\t"
//            <<time_comm_bcast<<"\t"
//            <<time_comm_collect<<"\t"
//            <<time_other<<"\t"
//            <<time_all<<"\t"
//            <<time_all- time_load<<"\t"
//            <<time_compute<<"\t"
//            <<time_comm<<"\t"
//            <<std::endl;
//    
//    std::string fout_path_exectime = graph_path +"_exectime";
//    fout_exectime.open(fout_path_exectime,std::ios::app);
//    if(fout_exectime.is_open()){
//    fout_exectime<<time_load<<"\t"
//            <<time_compute_spmv<<"\t"           
//            <<time_compute_diff<<"\t"
//            <<time_comm_bcast<<"\t"
//            <<time_comm_collect<<"\t"
//            <<time_other<<"\t"
//            <<time_all<<"\t"
//            <<time_all-time_load<<"\t"
//            <<time_compute<<"\t"
//            <<time_comm<<"\t"
//            <<std::endl;    }
//}
//
//void graph::print_exectime_sssp(){
//    time_stop_s=frovedis::get_dtime();    
//    this->time_all = time_stop_s - time_start_s;
//    this->time_other = this->time_all - this->time_load - this->time_comm - this->time_compute;
//
//    RLOG(TIME_RECORD_LOG_LEVEL) <<"\n*****CC Execution Time Decomposition****"<< std::endl;  
//
//    RLOG(TIME_RECORD_LOG_LEVEL)<<"time_load\t"
//            <<"time_compute\t"
////            <<"time_compute_findNodesNext\t"
////            <<"time_compute_assignID\t"
////            <<"time_compute_other\t"
//            <<"time_comm_bcast\t"
//            <<"time_comm_collect\t"
//            <<"time_other\t"
//            <<"time_all\t"
//            <<"time_all_excl_load\t"
//            <<"time_compute\t"
//            <<"time_comm\t"
//            <<std::endl;
//    RLOG(TIME_RECORD_LOG_LEVEL)<<time_load<<"\t"
//            <<time_compute<<"\t"         
////            <<time_compute_other<<"\t"
//            <<time_comm_bcast<<"\t"
//            <<time_comm_collect<<"\t"
//            <<time_other<<"\t"
//            <<time_all<<"\t"
//            <<time_all-time_load<<"\t"
//            <<time_compute<<"\t"
//            <<time_comm<<"\t"
//            <<std::endl;
//    
//    std::string fout_path_exectime = graph_path +"_exectime";
//    fout_exectime.open(fout_path_exectime,std::ios::app);
//    if(fout_exectime.is_open()){
//    fout_exectime<<time_load<<"\t"
//            <<time_compute_sssp<<"\t"           
////            <<time_compute_other<<"\t"
//            <<time_comm_bcast<<"\t"
//            <<time_comm_collect<<"\t"
//            <<time_other<<"\t"
//            <<time_all<<"\t"
//            <<time_all-time_load<<"\t"
//            <<time_compute<<"\t"
//            <<time_comm<<"\t"            
//            <<std::endl;    }
//}

void graph::sssp_query(){
        std::string node_dest_str;
        RLOG(TIME_RECORD_LOG_LEVEL)<<"**********sssp query*************\n";
        RLOG(TIME_RECORD_LOG_LEVEL)<<"total num of nodes = "<<num_nodes<<"\n";
        RLOG(TIME_RECORD_LOG_LEVEL)<<"source node = "<<source_nodeID<<"\n";
        RLOG(TIME_RECORD_LOG_LEVEL)<<"input destination node: ";

        while(std::cin >> node_dest_str){

            size_t  node_dest;    
            std::stringstream(node_dest_str)>>node_dest;
            if(node_dest > num_nodes - 1){
                RLOG(TIME_RECORD_LOG_LEVEL)<<"ERROR: node not existed!\n";
            }
            else{
                if(nodes_dist[node_dest] == MAX_MATRIX){
                    //unreachable
                    RLOG(TIME_RECORD_LOG_LEVEL)<<"Unreachable node! Path not found!\n";
                }
                else{
                    RLOG(TIME_RECORD_LOG_LEVEL)<<"distance = "<<nodes_dist[node_dest]<<"\n";
                    RLOG(TIME_RECORD_LOG_LEVEL)<<"path:\n";
                    RLOG(TIME_RECORD_LOG_LEVEL)<<node_dest<<" <= ";    
                    size_t node_pred = nodes_pred[node_dest]; 
                    while(node_pred != source_nodeID && node_pred != nodes_pred[node_pred]){
                        RLOG(TIME_RECORD_LOG_LEVEL)<<node_pred<<" <= ";
                        node_pred = nodes_pred[node_pred];
                    }
                    RLOG(TIME_RECORD_LOG_LEVEL)<<source_nodeID;
                    RLOG(TIME_RECORD_LOG_LEVEL)<<"\n";
                }
            }
            RLOG(TIME_RECORD_LOG_LEVEL)<<"input destination node: ";
        }
        RLOG(TIME_RECORD_LOG_LEVEL)<<"*****************************\n";    
}

void graph::sssp_bf_spmv(bool if_sssp_enq, size_t source_vertex) {
#ifdef SPMV 
    //spMV, CCS
    RLOG(TIME_RECORD_LOG_LEVEL)<<"converting matrix from crs to ccs.."<<std::endl;
    frovedis::ccs_matrix<TYPE_MATRIX> A_ccs(A);
    t_all.lap_start();
    this->nodes_pred.resize(num_nodes);
    auto* nodes_pred_ptr = &nodes_pred[0];
    for(size_t i= 0; i<num_nodes;i++){
        nodes_pred_ptr[i] = i;
    }
    this->nodes_dist.assign(num_nodes,MAX_MATRIX);

    //setting offset for idx in each worker node
    auto local_num_row = A.data.map(collect_local_num_row).gather();

    size_t num_worker = local_num_row.size();
    mat_offset.resize(num_worker); 
    mat_offset.at(0) = 0;
    for(size_t i=1; i< num_worker;i++){
        mat_offset.at(i) = mat_offset.at(i-1) + local_num_row.at(i-1);
    }

    std::vector<int> isConverged(mat_offset.size(),1);
    int isConverged_all = 0;

    this->source_nodeID = source_vertex; //TODO: User can change the source node id
    this->nodes_dist[source_nodeID] = 0;
    this->nodes_pred[source_nodeID] = source_nodeID;
    t_comm.lap_start();
    auto lmat_offset = frovedis::make_node_local_broadcast(mat_offset);
    auto isConverged_dv = frovedis::make_dvector_scatter(isConverged);     
    auto nodes_pred_dv = frovedis::make_dvector_scatter(this->nodes_pred,local_num_row);
    t_comm.lap_stop();
    auto lnodes_pred = nodes_pred_dv.as_node_local();    

    RLOG(TIME_RECORD_LOG_LEVEL)<<"source_vertex ID = "<<this->source_nodeID<<std::endl;            
    size_t iter = 0;
    while(isConverged_all != num_worker && iter < num_nodes){

        RLOG(TIME_RECORD_LOG_LEVEL)<<"**** iteration = "<<iter<<" ****"<<std::endl;
        t_comm.lap_start();
        auto lnodes_dist = frovedis::make_node_local_broadcast(this->nodes_dist); //broadcast nodes_dist
        t_comm.lap_stop();
        auto lisConverged = isConverged_dv.as_node_local();  
        
        t_compute_spmv.lap_start();
        auto nodes_dist_new_local = A_ccs.data.map(bellmanford_spmv<TYPE_MATRIX>,lnodes_dist,lnodes_pred,lmat_offset,lisConverged);
        t_compute_spmv.lap_stop();
        
        t_comm.lap_start();
        auto nodes_dist_new = nodes_dist_new_local.moveto_dvector<TYPE_MATRIX>().gather();
        t_comm.lap_stop();
        
        this->nodes_dist = nodes_dist_new;
        isConverged = lisConverged.moveto_dvector<TYPE_MATRIX>().gather();
        isConverged_all = 0;
        auto* isConverged_ptr = &isConverged[0];
        for(size_t i=0; i<isConverged.size();i++){
          isConverged_all += isConverged_ptr[i];
        }
        iter ++;
    }
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Finished!"<<std::endl;
    t_all.lap_stop();
    nodes_pred = lnodes_pred.moveto_dvector<size_t>().gather(); 
    this->print_exectime();
#else
  std::vector<TYPE_MATRIX> dist;
  std::vector<size_t> pred;
  sssp_bf_impl(A, source_vertex, dist, pred);
  this->nodes_dist.swap(dist);
  this->nodes_pred.swap(pred);
#endif
  //shortest path query
  if(if_sssp_enq == true) sssp_query();
}

void graph::sssp_bf_spmspv_2dmerge(bool if_sssp_enq, size_t source_vertex, size_t upperbound){ 
    //Using sparse vector for frontiers and focus on only recently updated vertices

    RLOG(TIME_RECORD_LOG_LEVEL)<<"converting matrix from crs to ccs.."<<std::endl;
    frovedis::ccs_matrix<TYPE_MATRIX> A_ccs(A);
    t_all.lap_start();
    this->nodes_pred.resize(num_nodes);

    auto* nodes_pred_ptr = &nodes_pred[0];
    for(size_t i= 0; i<num_nodes;i++){
        nodes_pred_ptr[i] = i;
    }
    this->nodes_dist.assign(num_nodes,MAX_MATRIX);
    
    //setting offset for idx in each worker node
    auto local_num_row = A.data.map(collect_local_num_row).gather();
    
    size_t num_worker = local_num_row.size();

    mat_offset.resize(num_worker); 
    mat_offset.at(0) = 0;
    for(size_t i=1; i< num_worker;i++){
        mat_offset.at(i) = mat_offset.at(i-1) + local_num_row.at(i-1);
    }
    
    //source node
    this->source_nodeID = source_vertex;
    this->nodes_dist_current.idx.push_back(source_vertex);
    this->nodes_dist_current.val.push_back(0);
    this->nodes_dist_current.pred.push_back(source_vertex);

    t_comm.lap_start();
    auto lmat_offset = frovedis::make_node_local_broadcast(mat_offset);
    auto lnodes_dist = frovedis::make_node_local_broadcast(nodes_dist);
    auto lup = frovedis::make_node_local_broadcast(upperbound);
    auto nodes_pred_dv = frovedis::make_dvector_scatter(this->nodes_pred,local_num_row);
    t_comm.lap_stop();
    auto lnodes_pred = nodes_pred_dv.as_node_local();    
    
    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"source_vertex ID = "<<this->nodes_dist_current.idx[0]<<std::endl;            
    size_t iter = 0;

    while(nodes_dist_current.idx.size() != 0 && iter < num_nodes){

        RLOG(TIME_RECORD_LOG_LEVEL)<<"**** iteration = "<<iter<<" ****"<<std::endl;
        RLOG(TIME_RECORD_LOG_LEVEL)<<"# of active frontiers = "<<nodes_dist_current.idx.size()<<std::endl;
        
        t_comm.lap_start();
        auto lnodes_dist_current = frovedis::make_node_local_broadcast(this->nodes_dist_current); //broadcast nodes_dist
        t_comm.lap_stop();
        
        frovedis::node_local< sparse_vector_tri<TYPE_MATRIX>> nodes_dist_new_local;
        
        t_compute_spmv.lap_start();
        if(nodes_dist_current.idx.size() < hyb_th){
            nodes_dist_new_local = A_ccs.data.map(bellmanford_spmspv_merge<TYPE_MATRIX>,lnodes_dist_current,lnodes_dist, lnodes_pred,lmat_offset,lup);
        }
        else{
            RLOG(TIME_RECORD_LOG_LEVEL)<<"using sort and sep...\n";
            nodes_dist_new_local = A_ccs.data.map(bellmanford_spmspv_sort<TYPE_MATRIX>,lnodes_dist_current,lnodes_dist, lnodes_pred,lmat_offset);
        }      
        t_compute_spmv.lap_stop();
        
        t_comm.lap_start();
        auto nodes_dist_new_multiVec = nodes_dist_new_local.gather();
        t_comm.lap_stop();
        
        size_t num_elem_nodes_dist_new=0;
        std::vector< size_t> nodes_dist_offset(num_worker,0);
        for(size_t i=0;i<nodes_dist_new_multiVec.size();i++){
            nodes_dist_offset[i] = num_elem_nodes_dist_new;
            num_elem_nodes_dist_new += nodes_dist_new_multiVec[i].idx.size();
        }        
        
        sparse_vector_tri<TYPE_MATRIX> nodes_dist_new(num_elem_nodes_dist_new, 0);
//        nodes_dist_new.idx.resize(num_elem_nodes_dist_new);
//        nodes_dist_new.val.resize(num_elem_nodes_dist_new);
//        nodes_dist_new.pred.resize(num_elem_nodes_dist_new);
        
        
        //unroll;
        #pragma cdir nodep  
        for(size_t i=0; i<num_worker;i++){
            #pragma cdir nodep
            for(size_t j=0;j<nodes_dist_new_multiVec[i].idx.size();j++){
                nodes_dist_new.idx[nodes_dist_offset[i] + j] = nodes_dist_new_multiVec[i].idx[j];
                nodes_dist_new.val[nodes_dist_offset[i] + j] = nodes_dist_new_multiVec[i].val[j];
                nodes_dist_new.pred[nodes_dist_offset[i] + j] = nodes_dist_new_multiVec[i].pred[j];
            }
        }
        this->nodes_dist_current = nodes_dist_new;
        iter ++;
    }
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Finished!"<<std::endl;
    t_all.lap_stop();
    print_exectime();
    auto nodes_dist_multiVec = lnodes_dist.gather();
    nodes_dist = nodes_dist_multiVec[0];
    nodes_pred = lnodes_pred.moveto_dvector<size_t>().gather();

//    this->print_exetcime_sssp();
  
    //shortest path query
    if(if_sssp_enq == true){
        sssp_query();
    }
}


void graph::sssp_bf_spmspv_sort(bool if_sssp_enq, size_t source_vertex){ 
    //Using sparse vector for frontiers and focus on only recently updated vertices
    //sort instead of merge
    RLOG(TIME_RECORD_LOG_LEVEL)<<"converting matrix from crs to ccs.."<<std::endl;
    frovedis::ccs_matrix<TYPE_MATRIX> A_ccs(A);
    t_all.lap_start();
    this->nodes_pred.resize(num_nodes);
    
    auto* nodes_pred_ptr = &nodes_pred[0];
    for(size_t i= 0; i<num_nodes;i++){
        nodes_pred_ptr[i] = i;
    }
    this->nodes_dist.assign(num_nodes,MAX_MATRIX);
    auto local_num_row = A.data.map(collect_local_num_row).gather();
    size_t num_worker = local_num_row.size();
    
    
    mat_offset.resize(num_worker); 
    mat_offset.at(0) = 0;
    for(size_t i=1; i< num_worker;i++){
        mat_offset.at(i) = mat_offset.at(i-1) + local_num_row.at(i-1);
    }
    
    //source node
    this->source_nodeID = source_vertex;
    this->nodes_dist_current.idx.push_back(source_vertex);
    this->nodes_dist_current.val.push_back(0);
    this->nodes_dist_current.pred.push_back(source_vertex);
    
    t_comm.lap_start();
    auto lmat_offset = frovedis::make_node_local_broadcast(mat_offset);
    auto lnodes_dist = frovedis::make_node_local_broadcast(nodes_dist);
    auto nodes_pred_dv = frovedis::make_dvector_scatter(this->nodes_pred,local_num_row);
    t_comm.lap_stop();
    auto lnodes_pred = nodes_pred_dv.as_node_local();    
    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"source_vertex ID = "<<this->nodes_dist_current.idx[0]<<std::endl;            
    size_t iter = 0;
//    for(size_t current_level = 0; current_level < num_nodes; current_level ++) {
    while(nodes_dist_current.idx.size() != 0 && iter < num_nodes){

        RLOG(TIME_RECORD_LOG_LEVEL)<<"**** iteration = "<<iter<<" ****"<<std::endl;
        RLOG(TIME_RECORD_LOG_LEVEL)<<"# of active frontiers = "<<nodes_dist_current.idx.size()<<std::endl;
        t_comm.lap_start();
        auto lnodes_dist_current = frovedis::make_node_local_broadcast(this->nodes_dist_current); //broadcast nodes_dist
        t_comm.lap_stop();
        
        frovedis::node_local< sparse_vector_tri<TYPE_MATRIX>> nodes_dist_new_local;
        
        t_compute_spmv.lap_start();
        nodes_dist_new_local = A_ccs.data.map(bellmanford_spmspv_sort<TYPE_MATRIX>,lnodes_dist_current,lnodes_dist, lnodes_pred,lmat_offset);
        t_compute_spmv.lap_stop();
        
        t_comm.lap_start();
        auto nodes_dist_new_multiVec = nodes_dist_new_local.gather();
        t_comm.lap_stop();
        
        size_t num_elem_nodes_dist_new=0;
        std::vector< size_t> nodes_dist_offset(num_worker,0);
        for(size_t i=0;i<nodes_dist_new_multiVec.size();i++){
            nodes_dist_offset[i] = num_elem_nodes_dist_new;
            num_elem_nodes_dist_new += nodes_dist_new_multiVec[i].idx.size();
        }        
        
        sparse_vector_tri<TYPE_MATRIX> nodes_dist_new;
        nodes_dist_new.idx.resize(num_elem_nodes_dist_new);
        nodes_dist_new.val.resize(num_elem_nodes_dist_new);
        nodes_dist_new.pred.resize(num_elem_nodes_dist_new);
        
        
        //unroll;
     #pragma cdir nodep 
        for(size_t i=0; i<num_worker;i++){
    #pragma cdir nodep
            for(size_t j=0;j<nodes_dist_new_multiVec[i].idx.size();j++){
                nodes_dist_new.idx[nodes_dist_offset[i] + j] = nodes_dist_new_multiVec[i].idx[j];
                nodes_dist_new.val[nodes_dist_offset[i] + j] = nodes_dist_new_multiVec[i].val[j];
                nodes_dist_new.pred[nodes_dist_offset[i] + j] = nodes_dist_new_multiVec[i].pred[j];
            }
            
        }
        this->nodes_dist_current = nodes_dist_new;
        iter ++;
    }
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Finished!"<<std::endl;

    t_all.lap_stop();
    print_exectime();
    auto nodes_dist_multiVec = lnodes_dist.gather();
    nodes_dist = nodes_dist_multiVec[0];
    nodes_pred = lnodes_pred.moveto_dvector<size_t>().gather();
  
    //shortest path query
    if(if_sssp_enq == true){
        sssp_query();
    }
}

template <class T>
void gather_helper(std::vector<T>& v, std::vector<T>& gathered) {
  int self = get_selfid();
  int nodes = get_nodesize();
  std::vector<size_t> recvcounts(nodes);
  size_t vsize = v.size();
  MPI_Gather(&vsize, sizeof(size_t), MPI_CHAR, 
             reinterpret_cast<char*>(&recvcounts[0]),
             sizeof(size_t), MPI_CHAR, 0, frovedis_comm_rpc);
  size_t total = 0;
  for(size_t i = 0; i < nodes; i++) total += recvcounts[i];
  std::vector<size_t> displs(nodes);
  if(self == 0) {
    for(size_t i = 1; i < nodes; i++) 
      displs[i] = displs[i-1] + recvcounts[i-1];
  }
  gathered.resize(total);
  auto gatheredp = gathered.data();
  large_gatherv(sizeof(T), reinterpret_cast<char*>(&v[0]), vsize, 
                reinterpret_cast<char*>(gatheredp), recvcounts,
                displs, 0, frovedis_comm_rpc);
}

template <class T>
void bcast_helper(std::vector<T>& v) {
  size_t vsize = v.size();
  MPI_Bcast(&vsize, sizeof(size_t), MPI_CHAR, 0, frovedis_comm_rpc);
  if(get_selfid() != 0) v.resize(vsize);
  large_bcast(sizeof(T), reinterpret_cast<char*>(v.data()), vsize, 0,
              frovedis_comm_rpc);
}

void pagerank_v1_helper_crs(crs_matrix_local<TYPE_MATRIX_PAGERANK>& A,
                            std::vector<TYPE_MATRIX_PAGERANK>& prank,
                            double bias, double dfactor, double epsilon,
                            size_t iter_max) {
  double diff = epsilon + 1;
  size_t iter_index = 0;

  time_spent spmv(TIME_RECORD_LOG_LEVEL);
  time_spent comm(TIME_RECORD_LOG_LEVEL);
  time_spent other(TIME_RECORD_LOG_LEVEL);
  
  RLOG(TIME_RECORD_LOG_LEVEL)<<"iter_idx\t"<<"diff(abs)"<<std::endl;
  while(diff > epsilon && iter_index < iter_max){
    iter_index ++;

    spmv.lap_start();
    auto res = PRmat_crs<TYPE_MATRIX_PAGERANK>(A, prank, bias, dfactor);
    spmv.lap_stop();

    comm.lap_start();
    std::vector<TYPE_MATRIX_PAGERANK> prank_new;
    gather_helper<TYPE_MATRIX_PAGERANK>(res, prank_new);
    bcast_helper<TYPE_MATRIX_PAGERANK>(prank_new);
    comm.lap_stop();
        
    other.lap_start();        
    diff = cal_abs_diff_vec<TYPE_MATRIX_PAGERANK>(prank_new,prank);
    prank = prank_new;
    other.lap_stop();  

    RLOG(TIME_RECORD_LOG_LEVEL)<<iter_index<<"\t";        
    RLOG(TIME_RECORD_LOG_LEVEL)<<diff<<std::endl;
  }

  RLOG(TIME_RECORD_LOG_LEVEL)<<"****Converged or reached maximum iteration. Stopped.****"<<std::endl;

  spmv.show_lap("spmv: ");
  comm.show_lap("comm: ");
  other.show_lap("other: ");
}

void pagerank_v1_helper_hyb(jds_crs_hybrid_local<TYPE_MATRIX_PAGERANK>& A,
                            std::vector<TYPE_MATRIX_PAGERANK>& prank,
                            double bias, double dfactor, double epsilon,
                            size_t iter_max) {
  double diff = epsilon + 1;
  size_t iter_index = 0;

  time_spent spmv(TIME_RECORD_LOG_LEVEL);
  time_spent comm(TIME_RECORD_LOG_LEVEL);
  time_spent other(TIME_RECORD_LOG_LEVEL);
  
  RLOG(TIME_RECORD_LOG_LEVEL)<<"iter_idx\t"<<"diff(abs)"<<std::endl;
  while(diff > epsilon && iter_index < iter_max){
    iter_index ++;

    spmv.lap_start();
    auto res = PRmat_hyb<TYPE_MATRIX_PAGERANK>(A, prank, bias, dfactor);
    spmv.lap_stop();

    comm.lap_start();
    std::vector<TYPE_MATRIX_PAGERANK> prank_new;
    gather_helper<TYPE_MATRIX_PAGERANK>(res, prank_new);
    bcast_helper<TYPE_MATRIX_PAGERANK>(prank_new);
    comm.lap_stop();
        
    other.lap_start();        
    diff = cal_abs_diff_vec<TYPE_MATRIX_PAGERANK>(prank_new,prank);
    prank = prank_new;
    other.lap_stop();  

    RLOG(TIME_RECORD_LOG_LEVEL)<<iter_index<<"\t";        
    RLOG(TIME_RECORD_LOG_LEVEL)<<diff<<std::endl;
  }

  RLOG(TIME_RECORD_LOG_LEVEL)<<"****Converged or reached maximum iteration. Stopped.****"<<std::endl;

  spmv.show_lap("spmv: ");
  comm.show_lap("comm: ");
  other.show_lap("other: ");
}

void graph::pagerank_v1(std::string matformat, double dfactor, double epsilon, size_t iter_max){ 
    //V1: prank_new = (1-df)*A*prank + df*V, V is unit vector 
    t_all.lap_start();
    time_spent t(TIME_RECORD_LOG_LEVEL);
  
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Initializing PageRank..."<<std::endl;
    double bias = 1/(double)num_nodes;
    double prank_init = 1/(double)num_nodes;
    //this->prank.assign(num_nodes,prank_init);
    prank.resize(num_nodes);
    auto prankp = prank.data();
    for(size_t i = 0; i < num_nodes; i++) prankp[i] = prank_init;
    t.show("init: ");

    frovedis::jds_crs_hybrid<TYPE_MATRIX_PAGERANK> A_hyb;
    
    if(matformat == "HYB"){
        RLOG(TIME_RECORD_LOG_LEVEL)<<"Converting CRS Matrix to HYB...\n";    
        A_hyb = frovedis::jds_crs_hybrid<TYPE_MATRIX_PAGERANK>(A_pg);       
        t.show("conversion to HYB: ");
    }
    RLOG(TIME_RECORD_LOG_LEVEL)<<"-------------------------"<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Damping factor= "<<dfactor<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Convergence Threshold= "<<epsilon<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Maximum Iteration= "<<iter_max<<std::endl;   
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Matrix Format= "<<matformat<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"-------------------------"<<std::endl; 
    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Computing PageRank(V1)..."<<std::endl;
    

    auto lb = frovedis::make_node_local_broadcast(bias);
    auto ldf = frovedis::make_node_local_broadcast(dfactor);
    auto leps = frovedis::make_node_local_broadcast(epsilon);
    auto liter_max = frovedis::make_node_local_broadcast(iter_max);
    auto lprank = frovedis::make_node_local_broadcast(prank);
    
    if(matformat == "HYB") {
      A_hyb.data.mapv(pagerank_v1_helper_hyb, lprank, lb, ldf, leps, liter_max);
    } else {
      A_pg.data.mapv(pagerank_v1_helper_crs, lprank, lb, ldf, leps, liter_max);
    }
    prank = lprank.get(0);
    t.show("pagerank: ");
    t_all.lap_stop();
    t_all.show_lap("all: ");
    rel_prank = cal_rel_prank<TYPE_MATRIX_PAGERANK>(prank);    
    print_sort_prank<TYPE_MATRIX_PAGERANK>(prank);
}


void graph::pagerank_v1_shrink(std::string matformat, double dfactor, double epsilon, size_t iter_max){ 
    //V1: prank_new = (1-df)*A*prank + df*V, V is unit vector 
  
    double diff = 1000;
    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Initializing PageRank..."<<std::endl;
    double bias = 1/(double)num_nodes;
    double prank_init = 1/(double)num_nodes;
    this->prank.assign(num_nodes,prank_init);
    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Shrinking matrix..."<<std::endl;
    auto tbl = frovedis::shrink_column(A_pg);
    auto info = prepare_shrink_comm_pagerank(tbl, A_pg);

    frovedis::jds_crs_hybrid<TYPE_MATRIX_PAGERANK> A_hyb;
    frovedis::jds_matrix<TYPE_MATRIX_PAGERANK> A_jds;
 
    
    if(matformat == "HYB"){
        RLOG(TIME_RECORD_LOG_LEVEL)<<"Converting CRS Matrix to HYB...\n";    
        A_hyb = frovedis::jds_crs_hybrid<TYPE_MATRIX_PAGERANK>(A_pg);       
    }
    else if(matformat == "JDS"){
        RLOG(TIME_RECORD_LOG_LEVEL)<<"Converting CRS Matrix to JDS...\n";    
        A_jds = frovedis::jds_matrix<TYPE_MATRIX_PAGERANK>(A_pg);  
    }
    

    t_all.lap_start();
    RLOG(TIME_RECORD_LOG_LEVEL)<<"-------------------------"<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Damping factor= "<<dfactor<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Convergence Threshold= "<<epsilon<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Maximum Iteration= "<<iter_max<<std::endl;   
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Matrix Format= "<<matformat<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"-------------------------"<<std::endl;   
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Computing PageRank(V1-Shrink)..."<<std::endl;
    
    t_comm.lap_start();
    auto lb = frovedis::make_node_local_broadcast(bias); //broadcast B
    auto ldf = frovedis::make_node_local_broadcast(dfactor); //broadcast dfactor
    auto prank_dv = frovedis::make_dvector_scatter(prank);
    auto lprank = frovedis::make_node_local_broadcast(prank); //broadcast pagerank 
    t_comm.lap_stop();
    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"iter_idx\t"<<"diff(abs)"<<std::endl;
    
    size_t iter_index = 0;    
    while(diff > epsilon && iter_index < iter_max){
        iter_index ++; 

        diff = 0;
         
        frovedis::node_local<std::vector<TYPE_MATRIX_PAGERANK>> lprank_res;
        t_compute_spmv.lap_start();         
        if(matformat == "CRS"){
            lprank_res = A_pg.data.map(PRmat_crs<TYPE_MATRIX_PAGERANK>,lprank,lb,ldf);    //add damping factor, using only crs matrix
        }
        else if(matformat == "HYB"){
            lprank_res = A_hyb.data.map(PRmat_hyb<TYPE_MATRIX_PAGERANK>,lprank,lb,ldf);   
        }
        t_compute_spmv.lap_stop();
        
        auto prank_dv_new = lprank_res.moveto_dvector<TYPE_MATRIX_PAGERANK>();
        
        t_comm.lap_start();        
        auto lprank_shrink_new = frovedis::shrink_vector_bcast(prank_dv_new,info);
        t_comm.lap_stop();    
//        diff = cal_abs_diff_vec<type_Matrix_PRank>(prank_new,prank);
        
        t_compute_other.lap_start();        
        diff = frovedis::zip(prank_dv_new,prank_dv).map(cal_diff_abs<TYPE_MATRIX_PAGERANK>).reduce(sum<TYPE_MATRIX_PAGERANK>);
        t_compute_other.lap_stop();
        
        
        if(if_cout == true){
            auto prank_new = prank_dv_new.gather();
            RLOG(TIME_RECORD_LOG_LEVEL)<<"absolute rank:"<<std::endl;
            for(auto i: prank_new) RLOG(TIME_RECORD_LOG_LEVEL) << i << std::endl;   
        }
        
        RLOG(TIME_RECORD_LOG_LEVEL)<<iter_index<<"\t";        
        RLOG(TIME_RECORD_LOG_LEVEL)<<diff<<std::endl;
        
        lprank = lprank_shrink_new;
        prank_dv = prank_dv_new;        
        
        if(if_fout == true && fout.is_open()){
            fout<<iter_index<<"\t";
            for(auto i: rel_prank) fout << i << "\t";
            fout<<diff;
            fout<<std::endl;
        }
    }
    RLOG(TIME_RECORD_LOG_LEVEL)<<"****Converged or reached maximum iteration. Stopped.****"<<std::endl;
    t_all.lap_stop();
    prank = prank_dv.gather();
    rel_prank = cal_rel_prank<TYPE_MATRIX_PAGERANK>(prank);    
    print_sort_prank<TYPE_MATRIX_PAGERANK>(prank);
    this->print_exectime();
}


void graph::pagerank_v2(std::string matformat, double dfactor, double epsilon, size_t iter_max){
    //V2: prank_new = (1-df)*A*prank + (|(1-df)*A*prank|-|prank|) *V 
    
    double diff = 1000;
    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Initializing PageRank..."<<std::endl;
    double bias = 1/(double)num_nodes;
    double prank_init = 1/(double)num_nodes;
    this->prank.assign(num_nodes,prank_init);
    
    frovedis::jds_crs_hybrid<TYPE_MATRIX_PAGERANK> A_hyb;
    frovedis::jds_matrix<TYPE_MATRIX_PAGERANK> A_jds;
    
    if(matformat == "HYB"){
        RLOG(TIME_RECORD_LOG_LEVEL)<<"Converting CRS Matrix to HYB...\n";    
        A_hyb = frovedis::jds_crs_hybrid<TYPE_MATRIX_PAGERANK>(A_pg);       
    }
    else if(matformat == "JDS"){
        RLOG(TIME_RECORD_LOG_LEVEL)<<"Converting CRS Matrix to JDS...\n";    
        A_jds = frovedis::jds_matrix<TYPE_MATRIX_PAGERANK>(A_pg);  
    }
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Damping factor= "<<dfactor<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Convergence Threshold= "<<epsilon<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Maximum Iteration= "<<iter_max<<std::endl; 
    RLOG(TIME_RECORD_LOG_LEVEL)<<std::endl;   
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Computing PageRank(V2)..."<<std::endl;
    t_all.lap_start();
    t_comm.lap_start();
    auto lb = frovedis::make_node_local_broadcast(bias); //broadcast B
    auto ldf = frovedis::make_node_local_broadcast(dfactor); //broadcast dfactor
    auto prank_dv = frovedis::make_dvector_scatter(prank);
    t_comm.lap_stop();
    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"iter_idx\t"<<"diff(abs)"<<std::endl;
    size_t iter_index = 0;
    
    while(diff > epsilon && iter_index < iter_max){
        iter_index ++; 
        
        t_comm.lap_start();
        auto lprank = frovedis::make_node_local_broadcast(prank);  //broadcast prank
        t_comm.lap_stop();
        
        frovedis::node_local<std::vector<TYPE_MATRIX_PAGERANK>> temp_prank_lres; 
        t_compute_spmv.lap_start();
        if(matformat == "CRS"){
            temp_prank_lres = A_pg.data.map(PRmat_tempprank_crs<TYPE_MATRIX_PAGERANK>,lprank,ldf);
        }else if(matformat == "HYB"){
            temp_prank_lres = A_hyb.data.map(PRmat_tempprank_hyb<TYPE_MATRIX_PAGERANK>,lprank,ldf);
        }
        t_compute_spmv.lap_stop();
        
        t_compute_other.lap_start();
        auto temp_prank_dv = temp_prank_lres.moveto_dvector<TYPE_MATRIX_PAGERANK>();
        auto temp_prank_l1 = temp_prank_dv.map(abs_d<TYPE_MATRIX_PAGERANK>).reduce(sum<TYPE_MATRIX_PAGERANK>); 
        auto prank_l1 = prank_dv.map(abs_d<TYPE_MATRIX_PAGERANK>).reduce(sum<TYPE_MATRIX_PAGERANK>);
        auto prank_diff = prank_l1 - temp_prank_l1;
        t_compute_other.lap_stop();
        
        t_comm.lap_start();
        auto lprank_diff = frovedis::make_node_local_broadcast(prank_diff);
        t_comm.lap_stop();
        
        t_compute_other.lap_start();
        auto prank_dv_new = temp_prank_dv.map(PRmat_newprank_elem<TYPE_MATRIX_PAGERANK>, lprank_diff, lb);
        t_compute_other.lap_stop();
        
        t_comm.lap_start();
        auto prank_new = prank_dv_new.gather();
        t_comm.lap_stop();
        
        t_compute_other.lap_start();
        diff = frovedis::zip(prank_dv_new,prank_dv).map(cal_diff_abs<TYPE_MATRIX_PAGERANK>).reduce(sum<TYPE_MATRIX_PAGERANK>);
        t_compute_other.lap_stop();
        
        if(if_cout == true){
            RLOG(TIME_RECORD_LOG_LEVEL)<<"absolute rank:"<<std::endl;
            for(auto i: prank_new) RLOG(TIME_RECORD_LOG_LEVEL) << i << std::endl; 
        }
        RLOG(TIME_RECORD_LOG_LEVEL)<<iter_index<<"\t";        
        RLOG(TIME_RECORD_LOG_LEVEL)<<diff<<std::endl;          
        prank = prank_new;
        prank_dv = prank_dv_new;        

        if(if_fout == true && fout.is_open()){
            fout<<iter_index<<"\t";
            for(auto i: rel_prank) fout << i << "\t";
            fout<<diff;
            fout<<std::endl;
        }
    }
    RLOG(TIME_RECORD_LOG_LEVEL)<<"****Converged or reached maximum iteration. Stopped.****"<<std::endl;
    t_all.lap_stop();
    //computing relative importance 
    rel_prank = cal_rel_prank<TYPE_MATRIX_PAGERANK>(prank);
    print_sort_prank<TYPE_MATRIX_PAGERANK>(rel_prank);
    this->print_exectime();
}

void graph::pagerank_v2_shrink(std::string matformat, double dfactor, double epsilon, size_t iter_max){
    //V2: prank_new = (1-df)*A*prank + (|(1-df)*A*prank|-|prank|) *V 
    
    double diff = 1000;
    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Initializing PageRank..."<<std::endl;
    double bias = 1/(double)num_nodes;
    double prank_init = 1/(double)num_nodes;
    this->prank.assign(num_nodes,prank_init);
    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Shrinking matrix..."<<std::endl;
    auto tbl = frovedis::shrink_column(A_pg);
    auto info = prepare_shrink_comm_pagerank(tbl, A_pg);    
    frovedis::jds_crs_hybrid<TYPE_MATRIX_PAGERANK> A_hyb;
    frovedis::jds_matrix<TYPE_MATRIX_PAGERANK> A_jds;
    
    if(matformat == "HYB"){
        RLOG(TIME_RECORD_LOG_LEVEL)<<"Converting CRS Matrix to HYB...\n";    
        A_hyb = frovedis::jds_crs_hybrid<TYPE_MATRIX_PAGERANK>(A_pg);       
    }
    else if(matformat == "JDS"){
        RLOG(TIME_RECORD_LOG_LEVEL)<<"Converting CRS Matrix to JDS...\n";    
        A_jds = frovedis::jds_matrix<TYPE_MATRIX_PAGERANK>(A_pg);  
    }

    
    RLOG(TIME_RECORD_LOG_LEVEL)<<"-------------------------"<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Damping factor= "<<dfactor<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Convergence Threshold= "<<epsilon<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Maximum Iteration= "<<iter_max<<std::endl;   
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Matrix Format= "<<matformat<<std::endl;
    RLOG(TIME_RECORD_LOG_LEVEL)<<"-------------------------"<<std::endl; 
    RLOG(TIME_RECORD_LOG_LEVEL)<<"Computing PageRank(v2_shrink)..."<<std::endl;
    t_all.lap_start();
    t_comm.lap_start();
    auto lb = frovedis::make_node_local_broadcast(bias); //broadcast B
    auto ldf = frovedis::make_node_local_broadcast(dfactor); //broadcast dfactor
    auto lprank = frovedis::make_node_local_broadcast(prank); //broadcast pagerank      
    auto prank_dv = frovedis::make_dvector_scatter(prank);
    t_comm.lap_stop();

    RLOG(TIME_RECORD_LOG_LEVEL)<<"iter_idx\t"<<"diff(abs)"<<std::endl;
    size_t iter_index = 0;
    
    while(diff > epsilon && iter_index < iter_max){
        iter_index ++; 

//        auto lprank = frovedis::make_node_local_broadcast(prank);  //broadcast prank
        frovedis::node_local<std::vector<TYPE_MATRIX_PAGERANK>> temp_prank_lres; 
        
        t_compute_spmv.lap_start();
        if(matformat == "CRS"){
            temp_prank_lres = A_pg.data.map(PRmat_tempprank_crs<TYPE_MATRIX_PAGERANK>,lprank,ldf);
        }else if(matformat == "HYB"){
            temp_prank_lres = A_hyb.data.map(PRmat_tempprank_hyb<TYPE_MATRIX_PAGERANK>,lprank,ldf);
        }
        t_compute_spmv.lap_stop();
        
        t_compute_other.lap_start();
        auto temp_prank_dv = temp_prank_lres.moveto_dvector<TYPE_MATRIX_PAGERANK>();
        auto temp_prank_l1 = temp_prank_dv.map(abs_d<TYPE_MATRIX_PAGERANK>).reduce(sum<TYPE_MATRIX_PAGERANK>); 
        auto prank_l1 = prank_dv.map(abs_d<TYPE_MATRIX_PAGERANK>).reduce(sum<TYPE_MATRIX_PAGERANK>);
        auto prank_diff = prank_l1 - temp_prank_l1;
        t_compute_other.lap_stop();
        
        t_comm.lap_start();
        auto lprank_diff = frovedis::make_node_local_broadcast(prank_diff);
        t_comm.lap_stop();
        
        t_compute_other.lap_start();
        auto prank_dv_new = temp_prank_dv.map(PRmat_newprank_elem<TYPE_MATRIX_PAGERANK>, lprank_diff, lb);
        t_compute_other.lap_stop();
        
        t_comm.lap_start();        
        auto lprank_shrink_new = frovedis::shrink_vector_bcast(prank_dv_new,info);  
        t_comm.lap_stop();
        
         t_compute_other.lap_start();       
        diff = frovedis::zip(prank_dv_new,prank_dv).map(cal_diff_abs<TYPE_MATRIX_PAGERANK>).reduce(sum<TYPE_MATRIX_PAGERANK>);
        t_compute_other.lap_stop();
        
        if(if_cout == true){
            auto prank_new = prank_dv_new.gather();
            RLOG(TIME_RECORD_LOG_LEVEL)<<"absolute rank:"<<std::endl;
            for(auto i: prank_new) RLOG(TIME_RECORD_LOG_LEVEL) << i << std::endl; 
        }
        RLOG(TIME_RECORD_LOG_LEVEL)<<iter_index<<"\t";        
        RLOG(TIME_RECORD_LOG_LEVEL)<<diff<<std::endl;          
        
        prank_dv = prank_dv_new;        
        lprank = lprank_shrink_new;
        
        if(if_fout == true && fout.is_open()){
            fout<<iter_index<<"\t";
            for(auto i: rel_prank) fout << i << "\t";
            fout<<diff;
            fout<<std::endl;
        }
    }
    RLOG(TIME_RECORD_LOG_LEVEL)<<"****Converged or reached maximum iteration. Stopped.****"<<std::endl;
    t_all.lap_stop();
    prank = prank_dv.gather();
    //computing relative importance 
    rel_prank = cal_rel_prank<TYPE_MATRIX_PAGERANK>(prank);
    print_sort_prank<TYPE_MATRIX_PAGERANK>(rel_prank);
    this->print_exectime();
    
}


void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

template <class T, class I, class O>
frovedis::node_local<std::vector<size_t>> my_partition_shrink_column(frovedis::crs_matrix<T,I,O>& m) {
    auto tmp = m.data.map(frovedis::crs_get_local_num_row<T>).gather();
    std::vector<size_t> ret(tmp.size()+1,0);
    for(size_t i = 1; i < ret.size(); i++){
        ret[i] = ret[i-1] + tmp[i-1];
    }
    return frovedis::make_node_local_broadcast(ret);
}


template <class Vector, class T>
bool graph::insert_into_vector(Vector& v, const T& t) {
    typename Vector::iterator i = std::lower_bound(v.begin(), v.end(), t);
    if (i == v.end() || t < *i) {
        v.insert(i, t);
        return true;
    } else {
        return false;
    }
}
bool graph::add_arc(size_t from, size_t to) {

    bool ret = false;
    size_t max_dim = std::max(from, to);
    if (if_cout == true) {
        RLOG(TIME_RECORD_LOG_LEVEL) << "checking to add " << from << " => " << to << std::endl;
    }
    if (rows.size() <= max_dim) {
        max_dim = max_dim + 1;
        rows.resize(max_dim);
        if (num_outgoing.size() <= max_dim) {
            num_outgoing.resize(max_dim);
        }
    }
    ret = insert_into_vector(rows[to], from);
    if (ret) {
        num_outgoing[from]++;
        if (if_cout == true) {
            RLOG(TIME_RECORD_LOG_LEVEL) << "added " << from << " => " << to << std::endl;
        }
    }

    return ret;
}



size_t graph::insert_mapping(const std::string &key) {

    size_t index = 0;
    std::map<std::string, size_t>::const_iterator i = nodes_to_idx.find(key);
    if (i != nodes_to_idx.end()) {
        index = i->second;
    } else {
        index = nodes_to_idx.size();
        nodes_to_idx.insert(std::pair<std::string, size_t>(key, index));
        idx_to_nodes.insert(std::pair<size_t, std::string>(index, key));;
    }

    return index;
}


void graph::prep_graph_crs(const std::string &filename_edge, bool if_direct){
    bool file_numeric = true;
    std::pair<std::map<std::string, size_t>::iterator, bool> ret;
    char delim = ' ';
    char commentline_identifier = '%';
    reset();
    
    std::istream *infile;

    // dealing with edges
    infile = new std::ifstream(filename_edge.c_str());
    if (!infile) {
          std::cerr<<"ERROR: Cannot open file "<<filename_edge.c_str()<<std::endl;
    }
    else{
        RLOG(TIME_RECORD_LOG_LEVEL)<<"Reading raw data file from "<<filename_edge.c_str()<<std::endl;
    }

    size_t linenum = 0;
    std::string line; // current line
    while (std::getline(*infile, line)) {
        if (line.find(commentline_identifier) != std::string::npos){//skip comment line
            
            continue;
        }

        std::string from, to; // from and to fields        
        size_t from_idx, to_idx; // indices of from and to nodes

        std::vector<std::string> raw_idx;
        split(line, delim, raw_idx);
        
        if(raw_idx.size() > 2){
            std::string k;
            RLOG(TIME_RECORD_LOG_LEVEL)<<"Reading the first line..."<<std::endl;
            for(auto i:raw_idx) RLOG(TIME_RECORD_LOG_LEVEL)<<i<<"\t";
            continue;
        }
        
        //finding from node
        from = raw_idx[0];

        if(from.find(' ')!= std::string::npos){
            std::cerr<<"Reading node error!"<<std::endl; //If parsing nodes has errors
            abort();
        }

        if (file_numeric == false) {
            from_idx = insert_mapping(from);
        } else {
            from_idx = strtoull(from.c_str(), NULL, 10);
        }

        to = raw_idx[1];
        
        if(to.find(' ')!= std::string::npos){
            std::cerr<<"Reading node error!"<<std::endl;
            abort();
        }

        if (file_numeric == false) {
            to_idx = insert_mapping(to);
        } else {
            to_idx = strtoull(to.c_str(), NULL, 10);
        }
        add_arc(from_idx, to_idx);
        if(if_direct == false){
            
            add_arc(to_idx, from_idx);
        }
        linenum++;
        if (linenum && ((linenum % 100000) == 0)) {
            std::cerr << "read " << linenum << " lines, "
                 << rows.size() << " nodes" << std::endl;
        }

        from.clear();
        to.clear();
        line.clear();
    }

    RLOG(TIME_RECORD_LOG_LEVEL) << "read " << linenum << " lines, "
         << rows.size() << " nodes" << std::endl;
    nodes_to_idx.clear();
    

//  Generating graph file in CRS format
    graph_path = filename_edge + "_graph_in_crs";
    fgraph_out.open(graph_path,std::ios::out);
    
    if(fgraph_out.is_open()){
       RLOG(TIME_RECORD_LOG_LEVEL)<<"Generating graph file in CRS format to  "<<graph_path<<std::endl;

       int start_idx = 0;
       //save to graph_file in CRS format
        for(size_t i=0; i<rows.size();i++){ //Ignore the first line
            size_t num_non_zero = 0;
            for(size_t j=0; j<rows.at(i).size();j++){

                    fgraph_out<<rows.at(i).at(j)-start_idx;
                    fgraph_out<<":";
                    fgraph_out<<1<<" ";
                   num_non_zero ++;         
            }
            if(i == start_idx && num_non_zero < rows.size()-start_idx){
                fgraph_out<<rows.size()-1-start_idx<<":0"; //make sure the CRS matrix keeps the number of row and coloumn the same
            }
            fgraph_out<<std::endl;
        }
     RLOG(TIME_RECORD_LOG_LEVEL)<<"Done!"<<graph_path<<std::endl;
    }
    else{
        std::cerr<<"Cannot open target graph file"<<std::endl;
    }

    fgraph_out.close(); 
}


void graph::prep_graph_crs_pagerank(const std::string &filename_edge,
                                    const std::string &output_file){ 
    //For pagerank graphs. Weight is not unity.
    //Prepare CRS graph from original file(SNAP web-graphs or Florida Sparse Matrix Collection, ignore lines with commentline_identifier)
    bool file_numeric = true;
    std::pair<std::map<std::string, size_t>::iterator, bool> ret;
    char delim = ' ';
    char commentline_identifier = '%';
    reset();
    
    std::istream *infile;

    // dealing with edges
    infile = new std::ifstream(filename_edge.c_str());
    if (!infile) {
          std::cerr<<"ERROR: Cannot open file "<<filename_edge.c_str()<<std::endl;
    }
    else{
        RLOG(TIME_RECORD_LOG_LEVEL)<<"Reading raw data file from "<<filename_edge.c_str()<<std::endl;
    }

    size_t linenum = 0;
    std::string line; // current line
    while (std::getline(*infile, line)) {
        if (line.find(commentline_identifier) != std::string::npos){//skip comment line
            
            continue;
        }

        std::string from, to; // from and to fields        
        size_t from_idx, to_idx; // indices of from and to nodes

        std::vector<std::string> raw_idx;
        split(line, delim, raw_idx);
        
        if(raw_idx.size() > 2){
            std::string k;
            RLOG(TIME_RECORD_LOG_LEVEL)<<"Reading the first line..."<<std::endl;
            for(auto i:raw_idx) RLOG(TIME_RECORD_LOG_LEVEL)<<i<<"\t";
            continue;
        }
        
        //finding from node
        from = raw_idx[0];

        if(from.find(' ')!= std::string::npos){
            std::cerr<<"Reading node error!"<<std::endl; //If parsing nodes has errors
            abort();
        }

        if (file_numeric == false) {
            from_idx = insert_mapping(from);
        } else {
          from_idx = strtoull(from.c_str(), NULL, 10) - 1; // 1-based to 0-based
        }

        to = raw_idx[1];
        
        if(to.find(' ')!= std::string::npos){
            std::cerr<<"Reading node error!"<<std::endl;
            abort();
        }

        if (file_numeric == false) {
            to_idx = insert_mapping(to);
        } else {
            to_idx = strtoull(to.c_str(), NULL, 10) - 1; // 1-based to 0-based
        }
        add_arc(from_idx, to_idx);

        linenum++;
        if (linenum && ((linenum % 100000) == 0)) {
            std::cerr << "read " << linenum << " lines, "
                 << rows.size() << " nodes" << std::endl;
        }

        from.clear();
        to.clear();
        line.clear();
    }

    RLOG(TIME_RECORD_LOG_LEVEL) << "read " << linenum << " lines, "
         << rows.size() << " nodes" << std::endl;
    nodes_to_idx.clear();
    

//  Generating graph file in CRS format
    graph_path = output_file;
    fgraph_out.open(graph_path,std::ios::out);
    if(fgraph_out.is_open()){
       RLOG(TIME_RECORD_LOG_LEVEL)<<"Generating graph file in CRS format to  "<<graph_path<<std::endl;

       //save to graph_file in CRS format
        for(size_t i=0; i<rows.size();i++){
            size_t num_non_zero = 0;
            for(size_t j=0; j<rows.at(i).size();j++){

                    fgraph_out<<rows.at(i).at(j);
                    fgraph_out<<":";
                    fgraph_out<<1/(double)num_outgoing[rows.at(i).at(j)]<<" ";
                   num_non_zero ++;         
            }
            if(num_non_zero < rows.size() && i == 0){
                fgraph_out<<rows.size()-1<<":0"; //make sure the CRS matrix keeps the number of row and coloumn the same
            }
            fgraph_out<<std::endl;
        }
     RLOG(TIME_RECORD_LOG_LEVEL)<<"Done!"<<graph_path<<std::endl;
    }
    else{
        std::cerr<<"Cannot open target graph file"<<std::endl;
    }

    fgraph_out.close();
}

void graph::reset() {
    num_outgoing.clear();
    rows.clear();
    nodes_to_idx.clear();
    idx_to_nodes.clear();

}

void graph::_load_pagerank(const std::string& path, bool binary) {
    if (binary) {
        prank = make_dvector_loadbinary<TYPE_MATRIX_PAGERANK>(path).gather();
    } else {
        prank = make_dvector_loadline<TYPE_MATRIX_PAGERANK>(path).gather();
    }

    num_nodes = prank.size();
}

void graph::_load_cc(const std::string& path, bool binary) {
    if (!directory_exists(path)) {
      throw std::runtime_error("no such directory: " + path);
    }
    std::string path_nodes_dist = path + "/" + FNAME_NODES_DIST;
    std::string path_nodes_in_which_cc = path + "/" + FNAME_NODES_IN_WHICH_CC;
    std::string path_num_nodes_in_each_cc = path + "/" + FNAME_NUM_NODES_IN_EACH_CC;
    if (binary) {
        nodes_dist = make_dvector_loadbinary<TYPE_MATRIX>(path_nodes_dist).gather();        
        nodes_in_which_cc = make_dvector_loadbinary<size_t>(path_nodes_in_which_cc).gather();
        num_nodes_in_each_cc = make_dvector_loadbinary<size_t>(path_num_nodes_in_each_cc).gather();
    } else {
        nodes_dist = make_dvector_loadline<TYPE_MATRIX>(path_nodes_dist).gather();        
        nodes_in_which_cc = make_dvector_loadline<size_t>(path_nodes_in_which_cc).gather();
        num_nodes_in_each_cc = make_dvector_loadline<size_t>(path_num_nodes_in_each_cc).gather();
    }
    
    num_nodes = nodes_dist.size();
    num_cc = 0;
    for(size_t i = 0; i < num_nodes_in_each_cc.size(); i++) {
        if (num_nodes_in_each_cc[i] == 0) break;
        num_cc ++;
    }
}

void graph::_load_sssp(const std::string& path, bool binary) {
    if (!directory_exists(path)) {
      throw std::runtime_error("no such directory: " + path);
    }
    std::string path_nodes_dist = path + "/" + FNAME_NODES_DIST;
    std::string path_nodes_pred = path + "/" + FNAME_NODES_PRED;
    if (binary) {
        nodes_dist = make_dvector_loadbinary<TYPE_MATRIX>(path_nodes_dist).gather();        
        nodes_pred = make_dvector_loadbinary<size_t>(path_nodes_pred).gather();
    }
    else {
        nodes_dist = make_dvector_loadline<TYPE_MATRIX>(path_nodes_dist).gather();        
        nodes_pred = make_dvector_loadline<size_t>(path_nodes_pred).gather();        
    }
    
    num_nodes = nodes_dist.size();
    for(size_t i = 0; i < nodes_pred.size(); i++) {
        bool is_source = nodes_dist[i] != MAX_MATRIX && i == nodes_pred[i];
        if (is_source) source_nodeID = i;
    }    
}

void graph::_save_pagerank(const std::string& path, bool binary) {
    if (binary) {
        make_dvector_scatter(prank).savebinary(path);
    } else {
        make_dvector_scatter(prank).saveline(path);
    }   
}

void graph::_save_cc(const std::string& path, bool binary) {
    make_directory(path);
    std::string path_nodes_dist = path + "/" + FNAME_NODES_DIST;
    std::string path_nodes_in_which_cc = path + "/" + FNAME_NODES_IN_WHICH_CC;
    std::string path_num_nodes_in_each_cc = path + "/" + FNAME_NUM_NODES_IN_EACH_CC;
    if (binary) {
        make_dvector_scatter(nodes_dist).savebinary(path_nodes_dist);
        make_dvector_scatter(nodes_in_which_cc).savebinary(path_nodes_in_which_cc);
        make_dvector_scatter(num_nodes_in_each_cc).savebinary(path_num_nodes_in_each_cc);
    } else {
        make_dvector_scatter(nodes_dist).saveline(path_nodes_dist);
        make_dvector_scatter(nodes_in_which_cc).saveline(path_nodes_in_which_cc);
        make_dvector_scatter(num_nodes_in_each_cc).saveline(path_num_nodes_in_each_cc);
    }
}

void graph::_save_sssp(const std::string& path, bool binary) {
    make_directory(path);
    std::string path_nodes_dist = path + "/" + FNAME_NODES_DIST;
    std::string path_nodes_pred = path + "/" + FNAME_NODES_PRED;
    if (binary) {
        make_dvector_scatter(nodes_dist).savebinary(path_nodes_dist);
        make_dvector_scatter(nodes_pred).savebinary(path_nodes_pred);
    } else {
        make_dvector_scatter(nodes_dist).saveline(path_nodes_dist);
        make_dvector_scatter(nodes_pred).saveline(path_nodes_pred);
    }   
}

}
