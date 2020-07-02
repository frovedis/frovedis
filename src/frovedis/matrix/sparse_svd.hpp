#ifndef SPARSE_SVD_HPP
#define SPARSE_SVD_HPP

#include <sstream>

#include "crs_matrix.hpp"
#include "ccs_matrix.hpp"
#include "colmajor_matrix.hpp"
#include "diag_matrix.hpp"
#include "arpackdef.hpp"

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

template <class REAL, class SPARSE_MATRIX_LOCAL>
void svd_mpi(SPARSE_MATRIX_LOCAL& mat,
             SPARSE_MATRIX_LOCAL& trans_mat,
             colmajor_matrix_local<REAL>& ret_u,
             diag_matrix_local<REAL>& ret_s,
             colmajor_matrix_local<REAL>& ret_v,
             int k) {
  int rank, size;
  MPI_Comm_rank(frovedis_comm_rpc, &rank);
  MPI_Comm_size(frovedis_comm_rpc, &size);
  MPI_Comm comm = frovedis_comm_rpc;
  MPI_Fint fcomm = MPI_Comm_c2f(frovedis_comm_rpc);

  // we assume that mat/trans_mat is distributed by row
  // whole matrix is m x n; m > n
  int mloc = mat.local_num_row;
  int n = mat.local_num_col;
  int n_each = ceil_div(n, size);
  int nloc = 0;
  if(rank * n_each < n) {
    nloc = std::min(n_each, n - rank * n_each);
  } else {
    throw std::runtime_error("nloc became 0. try another number of procs.");
    //nloc = 0;
  }

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

  int* recvcount_n = new int[size];
  MPI_Allgather(&nloc, 1, MPI_INT, recvcount_n, 1, MPI_INT, comm);
  int* displs_n = new int[size];
  displs_n[0] = 0;
  for(int i = 1; i < size; i++) {
    displs_n[i] = displs_n[i-1] + recvcount_n[i-1];
  }

  frovedis::time_spent t(DEBUG), t2(TRACE);
  frovedis::time_spent arpack_lap(DEBUG), mpi_lap(DEBUG), spmv_lap(DEBUG);
  int count = 0;
  std::vector<REAL> x(n);
  std::vector<REAL> y(n);
  while(1) {
    arpack_lap.lap_start();
    pxsaupd<REAL>(&fcomm, &ido, &bmat, &nloc, which, &nev, &tol, resid, 
                  &ncv, v, &ldv, iparam, ipntr, workd, workl,
                  &lworkl, &info);
    arpack_lap.lap_stop();
    if(ido == -1 || ido == 1) {
      REAL* start = &workd[ipntr[0]-1];
      mpi_lap.lap_start();
      typed_allgatherv<REAL>(start, nloc, &x[0], recvcount_n,
                             displs_n, frovedis_comm_rpc);
      mpi_lap.lap_stop();
      spmv_lap.lap_start();
      auto axloc = mat * x;
      auto yloc = trans_mat * axloc;
      spmv_lap.lap_stop();
      mpi_lap.lap_start();
      typed_allreduce(&yloc[0], &y[0], n, MPI_SUM, frovedis_comm_rpc);
      mpi_lap.lap_stop();
      start = &workd[ipntr[1]-1];
      REAL* yptr = &y[0] + n_each * rank;
      //for(int i = 0; i < nloc; i++) start[i] = yptr[i];
      memcpy(start, yptr, nloc * sizeof(REAL));
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
      std::vector<REAL> tmpv(n);
      REAL* tmpvp = &tmpv[0];
      ret_u.val.resize(mloc*nev);
      REAL* ret_u_valp = &ret_u.val[0];
      ret_u.local_num_col = nev;
      ret_u.local_num_row = mloc;
      for(int i = 0; i < nev; i++) {
        mpi_lap.lap_start();
        typed_allgatherv<REAL>(z + i * nloc, nloc, tmpvp, recvcount_n,
                               displs_n, frovedis_comm_rpc);
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
                              frovedis_comm_rpc);
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
  delete [] recvcount_n;
  delete [] displs_n;
}

template <class T, class SPARSE_MATRIX_LOCAL>
struct calc_svd {
  calc_svd() {}
  calc_svd(int k) : k(k) {}
  void operator()(SPARSE_MATRIX_LOCAL& mat,
                  colmajor_matrix_local<T>& u,
                  diag_matrix_local<T>& s,
                  colmajor_matrix_local<T>& v,
                  SPARSE_MATRIX_LOCAL& trans_mat) {
    svd_mpi<T, SPARSE_MATRIX_LOCAL>(mat, trans_mat, u, s, v, k);
  }
  int k;
  SERIALIZE(k)
};

template <class SPARSE_MATRIX_LOCAL, class T, class I, class O>
SPARSE_MATRIX_LOCAL trans_and_convert(crs_matrix_local<T,I,O>& m) {
  return SPARSE_MATRIX_LOCAL(m.transpose());
}

template <class SPARSE_MATRIX, class SPARSE_MATRIX_LOCAL,
          class T, class I, class O>
void sparse_svd(crs_matrix<T,I,O>& mat,
                colmajor_matrix<T>& u,
                diag_matrix_local<T>& s,
                colmajor_matrix<T>& v,
                int k,
                bool allow_transpose = true) {
  auto stmp = make_node_local_allocate<diag_matrix_local<T>>();
  u.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  v.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  if(mat.num_col <= mat.num_row || allow_transpose == false) {
    frovedis::time_spent t(DEBUG);  
    auto local_trans = mat.data.map(trans_and_convert
                                    <SPARSE_MATRIX_LOCAL, T, I ,O>);
    t.show("local transpose and convert matrix format: ");
    SPARSE_MATRIX new_mat(mat);
    t.show("convert matrix format: ");
    new_mat.data.mapv(calc_svd<T, SPARSE_MATRIX_LOCAL>(k), 
                      u.data, stmp, v.data, local_trans);
  } else {
    frovedis::time_spent t(DEBUG);  
    auto trans_mat = mat.transpose();
    t.show("transpose: ");
    auto local_trans = trans_mat.data.map(trans_and_convert
                                          <SPARSE_MATRIX_LOCAL, T, I ,O>);
    t.show("local transpose and convert matrix format: ");
    SPARSE_MATRIX new_trans_mat(trans_mat);
    t.show("convert matrix format: ");
    new_trans_mat.data.mapv(calc_svd<T, SPARSE_MATRIX_LOCAL>(k), 
                            v.data, stmp, u.data, local_trans);
  }
  s = *stmp.get_dvid().get_selfdata();
  u.set_num(mat.num_row, k);
  v.set_num(mat.num_col, k);
}

template <class SPARSE_MATRIX, class SPARSE_MATRIX_LOCAL,
          class T, class I, class O>
void sparse_svd(crs_matrix<T,I,O>&& mat,
                colmajor_matrix<T>& u,
                diag_matrix_local<T>& s,
                colmajor_matrix<T>& v,
                int k,
                bool allow_transpose = true) {
  auto stmp = make_node_local_allocate<diag_matrix_local<T>>();
  u.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  v.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  auto mat_num_row = mat.num_row;
  auto mat_num_col = mat.num_col;
  if(mat_num_col <= mat_num_row || allow_transpose == false) {
    frovedis::time_spent t(DEBUG);  
    auto local_trans = mat.data.map(trans_and_convert
                                    <SPARSE_MATRIX_LOCAL, T, I ,O>);
    t.show("local transpose and convert matrix format: ");
    SPARSE_MATRIX new_mat(mat);
    t.show("convert matrix format: ");
    new_mat.data.mapv(calc_svd<T, SPARSE_MATRIX_LOCAL>(k), 
                      u.data, stmp, v.data, local_trans);
  } else {
    frovedis::time_spent t(DEBUG);  
    auto trans_mat = mat.transpose();
    mat.clear();
    t.show("transpose: ");
    auto local_trans = trans_mat.data.map(trans_and_convert
                                          <SPARSE_MATRIX_LOCAL, T, I ,O>);
    t.show("local transpose and convert matrix format: ");
    SPARSE_MATRIX new_trans_mat(trans_mat);
    t.show("convert matrix format: ");
    new_trans_mat.data.mapv(calc_svd<T, SPARSE_MATRIX_LOCAL>(k), 
                            v.data, stmp, u.data, local_trans);
  }
  s = *stmp.get_dvid().get_selfdata();
  u.set_num(mat_num_row, k);
  v.set_num(mat_num_col, k);
}

template <class T, class I, class O>
void sparse_svd(crs_matrix<T,I,O>& mat,
                colmajor_matrix<T>& u,
                diag_matrix_local<T>& s,
                colmajor_matrix<T>& v,
                int k,
                bool allow_transpose = true) {
  auto stmp = make_node_local_allocate<diag_matrix_local<T>>();
  u.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  v.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  if(mat.num_col <= mat.num_row || allow_transpose == false) {
    frovedis::time_spent t(DEBUG);
    // call_tranpose is defined in crs_matrix.hpp
    auto local_trans = mat.data.map(call_transpose<T,I,O>);
    t.show("local transpose: ");
    mat.data.mapv(calc_svd<T,crs_matrix_local<T,I,O>>(k), u.data, stmp, v.data,
                  local_trans);
  } else {
    frovedis::time_spent t(DEBUG);  
    auto trans_mat = mat.transpose();
    t.show("transpose: ");
    auto local_trans = trans_mat.data.map(call_transpose<T,I,O>);
    t.show("local transpose: ");
    trans_mat.data.mapv(calc_svd<T,crs_matrix_local<T,I,O>>(k),
                        v.data, stmp, u.data, local_trans);
  }
  s = *stmp.get_dvid().get_selfdata();
  u.set_num(mat.num_row, k);
  v.set_num(mat.num_col, k);
}

}
#endif
