#ifndef SPARSE_SVD_HPP
#define SPARSE_SVD_HPP

#include <sstream>

#include "crs_matrix.hpp"
#include "ccs_matrix.hpp"
#include "colmajor_matrix.hpp"
#include "diag_matrix.hpp"
#include "arpackdef.hpp"
#include "shrink_matrix.hpp"

namespace frovedis {

//-- These functions are workaround for ve vectorization compiler
template <class REAL>
void mysqrt(REAL* s, int nev) {
  for(int i = 0; i < nev; i++) {
    s[i] = sqrt(s[i]);
  }
}

template <class REAL>
REAL calc_sq_sum_local(REAL* ret_u_valp, int mloc, int c) {
  REAL sq_sum_local = 0;
  for(int r = 0; r < mloc; r++) {
    sq_sum_local += ret_u_valp[c * mloc + r] * ret_u_valp[c * mloc + r];
  }
  return sq_sum_local;
}

template <class REAL>
void norm_ret_u_valp(REAL* ret_u_valp, REAL norm, int mloc, int c){
#pragma _NEC ivdep
  for(size_t r = 0; r < mloc; r++) {
    ret_u_valp[c * mloc + r] *= norm;
  }
}
//--

template <class REAL, class I, class SPARSE_MATRIX_LOCAL>
void svd_mpi(SPARSE_MATRIX_LOCAL& mat,
             SPARSE_MATRIX_LOCAL& trans_mat,
             size_t num_row,
             size_t num_col,
             std::vector<I>& tbl,
             colmajor_matrix_local<REAL>& ret_u,
             diag_matrix_local<REAL>& ret_s,
             colmajor_matrix_local<REAL>& ret_v,
             int k) {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm comm = MPI_COMM_WORLD;
  MPI_Fint fcomm = MPI_Comm_c2f(MPI_COMM_WORLD);

  // we assume that mat/trans_mat is distributed by row
  // whole matrix is m x n; m > n
  int mloc = mat.local_num_row;
  int n = num_col;
  int n_each = ceil_div(n, size);
  int nloc = 0;
  if(rank * n_each < n) {
    nloc = std::min(n_each, n - rank * n_each);
  } else nloc = 0;
  
  int ido = 0;
  char bmat = 'I';
  char* which = const_cast<char*>("LM");
  int nev = k;
  REAL tol = 0.0;
  REAL* resid = new REAL[nloc];
  int ncv;
  // some heulistics
  // ncv should be large enough, but it takes a lot of time if it is too large
  if(nev * 2 < n) { 
    if(nev * 2 > 10) {
      ncv = nev * 2;
    } else {
      ncv = std::min(10, n);
    }
  } else {
    ncv = n;
  }
  REAL* v = new REAL[ncv*nloc];
  int ldv = nloc;
  int iparam[11];
  iparam[0] = 1;
  int maxitr = n;
  iparam[2] = maxitr;
  iparam[3] = 1;
  iparam[6] = 1;
  int ipntr[11];
  REAL* workd = new REAL[3*nloc];
  int lworkl = ncv * (ncv + 8);
  REAL* workl = new REAL[lworkl];
  int info = 0;

  std::vector<int> each_n(size);
  MPI_Allgather(&nloc, 1, MPI_INT, &each_n[0], 1, MPI_INT, comm);

  std::vector<I> column_partition(size+1);
  auto column_partitionp = column_partition.data();
  auto each_np = each_n.data();
  for(int i = 1; i < size+1; i++)
    column_partitionp[i] = column_partitionp[i-1] + each_np[i-1];
  auto mat_info = create_shrink_vector_info_local(tbl, column_partition);

  frovedis::time_spent t(DEBUG), t2(TRACE);
  frovedis::time_spent arpack_lap(DEBUG), mpi_lap(DEBUG), spmv_lap(DEBUG);
  int count = 0;
  while(1) {
    arpack_lap.lap_start();
    pxsaupd<REAL>(&fcomm, &ido, &bmat, &nloc, which, &nev, &tol, resid, 
                  &ncv, v, &ldv, iparam, ipntr, workd, workl,
                  &lworkl, &info);
    arpack_lap.lap_stop();
    if(ido == -1 || ido == 1) {
      REAL* start = &workd[ipntr[0]-1];
      std::vector<REAL> workv(nloc);
      auto workvp = workv.data();
      for(int i = 0; i < nloc; i++) workvp[i] = start[i];
      mpi_lap.lap_start();
      auto x = shrink_vector_bcast_local(workv, mat_info);
      mpi_lap.lap_stop();
      spmv_lap.lap_start();
      auto axloc = mat * x;
      auto yloc = trans_mat * axloc;
      spmv_lap.lap_stop();
      auto y = shrink_vector_sum_local(yloc, mat_info);
      start = &workd[ipntr[1]-1];
      auto yp = y.data();
      //for(int i = 0; i < nloc; i++) start[i] = yp[i];
      memcpy(start, yp, nloc * sizeof(REAL));
    } else break;
    count++;
    if(rank == 0) {t2.show("one iteration: ");}
  }
  if(rank == 0) {
    std::stringstream ss;
    ss << "p[ds]saupd count: " << count;
    t.show(ss.str() + ", p[ds]saupd time: ");
  }
  trans_mat.clear();

  if(info < 0) {
    RLOG(ERROR) << "error with dsaupd, info = " << info << std::endl;
  } else {
    int rvec = 1;
    char howmny = 'A';
    int* select = new int[ncv];
    ret_s.val.resize(nev);
    REAL* s = &ret_s.val[0];
    ret_v.val.resize(nloc*nev);
    ret_v.local_num_col = nev;
    ret_v.local_num_row = nloc;
    REAL* z = &ret_v.val[0];
    REAL sigma;
    arpack_lap.lap_start();
    pxseupd<REAL>(&fcomm, &rvec, &howmny, select, s, z, &ldv, &sigma, 
                  &bmat, &nloc, which, &nev, &tol, resid, &ncv, v, &ldv, 
                  iparam, ipntr, workd, workl, &lworkl, &info);
    arpack_lap.lap_stop();
    if(rank == 0) t.show("p[ds]seupd time: ");
    if(info < 0) {
      RLOG(ERROR) << "error with dseupd, info = " << info << std::endl;
    } else {
      ret_u.val.resize(mloc*nev);
      REAL* ret_u_valp = &ret_u.val[0];
      ret_u.local_num_col = nev;
      ret_u.local_num_row = mloc;
      for(int i = 0; i < nev; i++) {
        std::vector<REAL> workv(nloc);
        REAL* start = z + i * nloc;
        auto workvp = workv.data();
        for(int j = 0; j < nloc; j++) workvp[j] = start[j];
        mpi_lap.lap_start();
        auto tmpv = shrink_vector_bcast_local(workv, mat_info);
        mpi_lap.lap_stop();
        spmv_lap.lap_start();
        auto avloc = mat * tmpv;
        spmv_lap.lap_stop();
        REAL* avlocp = &avloc[0];
        memcpy(ret_u_valp + i * mloc, avlocp, mloc * sizeof(REAL));
        /*for(size_t j = 0; j < mloc; j++) {
          ret_u_valp[i * mloc + j] = avlocp[j];
        }*/
      }
      if(rank == 0) t.show("Calculate u time: ");
      mysqrt(s, nev);
      /*for(size_t i = 0; i < nev; i++) {
        s[i] = sqrt(s[i]);
      }*/
      for(int c = 0; c < nev; c++) {
        REAL sq_sum = 0;
        REAL sq_sum_local = 0;
        sq_sum_local = calc_sq_sum_local(ret_u_valp, mloc, c);
        /*for(int r = 0; r < mloc; r++) {
          sq_sum_local += ret_u_valp[c * mloc + r] * ret_u_valp[c * mloc + r];
        }*/
        mpi_lap.lap_start();
        typed_allreduce<REAL>(&sq_sum_local, &sq_sum, 1, MPI_SUM,
                              MPI_COMM_WORLD);
        mpi_lap.lap_stop();
        REAL norm = 1.0 / sqrt(sq_sum);
        norm_ret_u_valp(ret_u_valp, norm, mloc, c);
        /*for(size_t r = 0; r < mloc; r++) {
          ret_u_valp[c * mloc + r] *= norm;
          }*/
      }
      if(rank == 0) t.show("Calculate returning s, u time: ");
    }
    if(rank == 0) {
      arpack_lap.show_lap("arpack time: ");
      mpi_lap.show_lap("MPI time: ");
      spmv_lap.show_lap("SpMV time: ");
    }
    delete [] select;
  }
    
  delete [] resid;
  delete [] v;
  delete [] workd;
  delete [] workl;
}

template <class T, class I, class SPARSE_MATRIX_LOCAL>
struct calc_svd {
  calc_svd() {}
  calc_svd(int k, size_t num_row, size_t num_col) :
    k(k), num_row(num_row), num_col(num_col){}
  void operator()(SPARSE_MATRIX_LOCAL& mat,
                  colmajor_matrix_local<T>& u,
                  diag_matrix_local<T>& s,
                  colmajor_matrix_local<T>& v,
                  std::vector<I>& tbl,
                  SPARSE_MATRIX_LOCAL& trans_mat) {
    svd_mpi<T, I, SPARSE_MATRIX_LOCAL>(mat, trans_mat,
                                       num_row, num_col,
                                       tbl,
                                       u, s, v, k);
  }
  int k;
  size_t num_row, num_col;
  SERIALIZE(k, num_row, num_col)
};

template <class T, class I, class O>
void sparse_svd(crs_matrix<T,I,O>& mat,
                colmajor_matrix<T>& u,
                diag_matrix_local<T>& s,
                colmajor_matrix<T>& v,
                int k, 
                bool allow_transpose = false) {
  auto stmp = make_node_local_allocate<diag_matrix_local<T>>();
  u.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  v.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  if(mat.num_col <= mat.num_row || allow_transpose == false) {
    frovedis::time_spent t(DEBUG);
    auto tbl = shrink_column(mat);
    t.show("shrink_column: ");
    // call_tranpose is defined in crs_matrix.hpp
    auto local_trans = mat.data.map(call_transpose<T,I,O>);
    t.show("local transpose: ");
    mat.data.mapv(calc_svd<T,I,crs_matrix_local<T>>
                  (k, mat.num_row, mat.num_col),
                  u.data, stmp,v.data, tbl, local_trans);
  } else {
    frovedis::time_spent t(DEBUG);  
    auto trans_mat = mat.transpose();
    t.show("transpose: ");
    auto tbl = shrink_column(trans_mat);
    t.show("shrink_column: ");
    auto local_trans = trans_mat.data.map(call_transpose<T,I,O>);
    t.show("local transpose: ");
    trans_mat.data.mapv(calc_svd<T,I,crs_matrix_local<T>>
                        (k, trans_mat.num_row, trans_mat.num_col),
                        v.data, stmp, u.data, tbl, local_trans);
  }
  s = *stmp.get_dvid().get_selfdata();
  u.set_num(mat.num_row, k);
  v.set_num(mat.num_col, k);
}

template <class SPARSE_MATRIX_LOCAL, class T, class I, class O>
SPARSE_MATRIX_LOCAL trans_and_convert(crs_matrix_local<T,I,O>& m) {
  return SPARSE_MATRIX_LOCAL(m.transpose());
}

template <class SPARSE_MATRIX,
          class SPARSE_MATRIX_LOCAL,
          class T, class I, class O>
void sparse_svd(crs_matrix<T,I,O>& mat,
                colmajor_matrix<T>& u,
                diag_matrix_local<T>& s,
                colmajor_matrix<T>& v,
                int k,
                bool allow_transpose = false) {
  auto stmp = make_node_local_allocate<diag_matrix_local<T>>();
  u.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  v.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  if(mat.num_col <= mat.num_row || allow_transpose == false) {
    frovedis::time_spent t(DEBUG);
    auto tbl = shrink_column(mat);
    t.show("shrink_column: ");
    auto local_trans = mat.data.map(trans_and_convert
                                    <SPARSE_MATRIX_LOCAL, T, I ,O>);
    t.show("local transpose and convert: ");
    SPARSE_MATRIX new_mat(mat);
    t.show("convert matrix format: ");
    new_mat.data.mapv(calc_svd<T,I,SPARSE_MATRIX_LOCAL>
                      (k, mat.num_row, mat.num_col),
                      u.data, stmp, v.data, tbl, local_trans);
  } else {
    frovedis::time_spent t(DEBUG);  
    auto trans_mat = mat.transpose();
    t.show("transpose: ");
    auto tbl = shrink_column(trans_mat);
    t.show("shrink_column: ");
    auto local_trans = trans_mat.data.map(trans_and_convert
                                          <SPARSE_MATRIX_LOCAL, T, I ,O>);
    t.show("local transpose: ");
    SPARSE_MATRIX new_trans_mat(trans_mat);
    t.show("convert matrix format: ");
    new_trans_mat.data.mapv(calc_svd<T,I,SPARSE_MATRIX_LOCAL>
                            (k, trans_mat.num_row, trans_mat.num_col),
                            v.data, stmp, u.data, tbl, local_trans);
  }
  s = *stmp.get_dvid().get_selfdata();
  u.set_num(mat.num_row, k);
  v.set_num(mat.num_col, k);
}

template <class SPARSE_MATRIX,
          class SPARSE_MATRIX_LOCAL,
          class T, class I, class O>
void sparse_svd(crs_matrix<T,I,O>&& mat,
                colmajor_matrix<T>& u,
                diag_matrix_local<T>& s,
                colmajor_matrix<T>& v,
                int k,
                bool allow_transpose = false) {
  auto stmp = make_node_local_allocate<diag_matrix_local<T>>();
  u.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  v.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  if(mat.num_col <= mat.num_row || allow_transpose == false) {
    frovedis::time_spent t(DEBUG);
    auto tbl = shrink_column(mat);
    t.show("shrink_column: ");
    auto local_trans = mat.data.map(trans_and_convert
                                    <SPARSE_MATRIX_LOCAL, T, I ,O>);
    t.show("local transpose and convert: ");
    SPARSE_MATRIX new_mat(mat);
    t.show("convert matrix format: ");
    new_mat.data.mapv(calc_svd<T,I,SPARSE_MATRIX_LOCAL>
                      (k, mat.num_row, mat.num_col),
                      u.data, stmp, v.data, tbl, local_trans);
  } else {
    frovedis::time_spent t(DEBUG);  
    auto trans_mat = mat.transpose();
    mat.clear();
    t.show("transpose: ");
    auto tbl = shrink_column(trans_mat);
    t.show("shrink_column: ");
    auto local_trans = trans_mat.data.map(trans_and_convert
                                          <SPARSE_MATRIX_LOCAL, T, I ,O>);
    t.show("local transpose: ");
    SPARSE_MATRIX new_trans_mat(trans_mat);
    t.show("convert matrix format: ");
    new_trans_mat.data.mapv(calc_svd<T,I,SPARSE_MATRIX_LOCAL>
                            (k, trans_mat.num_row, trans_mat.num_col),
                            v.data, stmp, u.data, tbl, local_trans);
  }
  s = *stmp.get_dvid().get_selfdata();
  u.set_num(mat.num_row, k);
  v.set_num(mat.num_col, k);
}

}
#endif
