#ifndef DENSE_EIGEN_HPP
#define DENSE_EIGEN_HPP

#include <sstream>
#include "colmajor_matrix.hpp"
#include "diag_matrix.hpp"
#include "arpackdef.hpp"
#include "scalapack_wrapper.hpp"

namespace frovedis {
  template <class REAL, class DENSE_MATRIX_LOCAL>
  void dense_eigen_sym_mpi(DENSE_MATRIX_LOCAL& mat,
                           size_t mat_size,
                           diag_matrix_local<REAL>& ret_d,
                           colmajor_matrix_local<REAL>& ret_v,
                           const std::string& order,
                           int k, int maxiter, REAL tol,
                           REAL sigma,
                           int mode) {
    if (mode != 1 && mode != 3)
      REPORT_ERROR(USER_ERROR, "dense_eigen_sym: Unsupported mode is encountered!\n");

    int rank, size;
    MPI_Comm_rank(frovedis_comm_rpc, &rank);
    MPI_Comm_size(frovedis_comm_rpc, &size);
    MPI_Comm comm = frovedis_comm_rpc;
    MPI_Fint fcomm = MPI_Comm_c2f(frovedis_comm_rpc);

    // assumed that mat is distributed by row
    int mloc = mat.local_num_row;
    int n = mat_size;
    int n_each = ceil_div(n, size);
    int nloc = 0;
    if(rank * n_each < n) {
      nloc = std::min(n_each, n - rank * n_each);
    } else {
      //throw std::runtime_error("nloc became 0. try another number of procs.");
      nloc = 0;
    }

    int ido = 0;
    char bmat = 'I';
    char* which = const_cast<char*>(order.c_str());
    int nev = k;
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
    iparam[2] = maxiter;
    iparam[3] = 1;
    iparam[6] = mode;
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
    std::vector<int> each_m(size);
    MPI_Allgather(&mloc, 1, MPI_INT, &each_m[0], 1, MPI_INT, comm);
    
    std::vector<int> each_n(size); // block distributed size
    MPI_Allgather(&nloc, 1, MPI_INT, &each_n[0], 1, MPI_INT, comm);
    
    std::vector<size_t> each_m2(size), each_n2(size);
    auto each_m2p = each_m2.data();
    auto each_mp = each_m.data();
    auto each_n2p = each_n2.data();
    auto each_np = each_n.data();
    for(size_t i = 0; i < size; i++) {
      each_m2p[i] = each_mp[i]; each_n2p[i] = each_np[i];
    }
    auto alltoall_size = align_as_calc_alltoall_sizes(each_m2, each_n2);
    std::vector<size_t> alltoall_sizes_tmp(size * size);
    MPI_Allgather(&alltoall_size[0], sizeof(size_t) * size, MPI_CHAR,
                  &alltoall_sizes_tmp[0], sizeof(size_t) * size, MPI_CHAR,
                  comm);
    std::vector<std::vector<size_t>> alltoall_sizes(size);
    for(size_t i = 0; i < size; i++) {
      alltoall_sizes[i].resize(size);
      for(size_t j = 0; j < size; j++) {
        alltoall_sizes[i][j] = alltoall_sizes_tmp[size * i + j];
      }
    }

    frovedis::time_spent t(DEBUG), t2(TRACE);
    frovedis::time_spent arpack_lap(DEBUG), mpi_lap(DEBUG), mv_lap(DEBUG);
    int count = 0;
    std::vector<REAL> x(n);
    while(1) {
      arpack_lap.lap_start();
      pxsaupd<REAL>(&fcomm, &ido, &bmat, &nloc, which, &nev, &tol, resid, 
                    &ncv, v, &ldv, iparam, ipntr, workd, workl,
                    &lworkl, &info);
      arpack_lap.lap_stop();
      REAL* start;
      if(ido == -1 || ido == 1) {
        if (mode == 3) start = &workd[ipntr[2]-1];
        else start = &workd[ipntr[0]-1];
        mpi_lap.lap_start();
        typed_allgatherv<REAL>(start, nloc, &x[0], recvcount_n,
                               displs_n, frovedis_comm_rpc);
        mpi_lap.lap_stop();
        mv_lap.lap_start();
        auto axloc = mat * x;
        mv_lap.lap_stop();
        std::vector<REAL> y(nloc);
        mpi_lap.lap_start();
        align_as_align<REAL>(axloc, y, alltoall_sizes);
        mpi_lap.lap_stop();
        start = &workd[ipntr[1]-1];
        auto yp = y.data();
        for(int i = 0; i < nloc; i++) start[i] = yp[i];
      } else break;
      count++;
      if(rank == 0) {t2.show("one iteration: ");}
    }
    if(rank == 0) {
      std::stringstream ss;
      ss << "p[ds]saupd count: " << count;
      t.show(ss.str() + ", p[ds]saupd time: ");
    }
    if(info < 0) {
      REPORT_ERROR(INTERNAL_ERROR, 
      "error with dsaupd, info = " + STR(info) + "\n");
    }
    else if(info == 1) {
      REPORT_ERROR(INTERNAL_ERROR, "ARPACK error: No convergence (" + STR(iparam[2]) + 
                   " iterations, " + STR(iparam[4]) + "/" + STR(k) +
                   " eigenvectors converged)\n");
    }
    else {
      int rvec = 1;
      char howmny = 'A';
      int* select = new int[ncv];
      ret_d.val.resize(nev);
      REAL* s = &ret_d.val[0];
      ret_v.val.resize(nloc*nev);
      ret_v.local_num_col = nev;
      ret_v.local_num_row = nloc;
      REAL* z = &ret_v.val[0];
      REAL sig = sigma;
      arpack_lap.lap_start();
      pxseupd<REAL>(&fcomm, &rvec, &howmny, select, s, z, &ldv, &sig, 
                    &bmat, &nloc, which, &nev, &tol, resid, &ncv, v, &ldv, 
                    iparam, ipntr, workd, workl, &lworkl, &info);
      arpack_lap.lap_stop();
      if(rank == 0) t.show("p[ds]seupd time: ");
      if(info < 0) {
        REPORT_ERROR(INTERNAL_ERROR, 
        "error with dseupd, info = " + STR(info) + "\n");
      }
      if(rank == 0) {
        arpack_lap.show_lap("arpack time: ");
        mpi_lap.show_lap("MPI time: ");
        mv_lap.show_lap("MV time: ");
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

  template <class T, class DENSE_MATRIX_LOCAL>
  struct calc_dense_eigen_sym {
    calc_dense_eigen_sym() {}
    calc_dense_eigen_sym(const std::string& order, int k, 
                         size_t mat_size, int maxiter,
                         float tol, T sigma = 0.0, int mode = 1) :
                         order(order), k(k), mat_size(mat_size),
                         maxiter(maxiter), tol(tol),
                         sigma(sigma), mode(mode) {}
    void operator()(DENSE_MATRIX_LOCAL& mat,
                    diag_matrix_local<T>& d,
                    colmajor_matrix_local<T>& v) {
      dense_eigen_sym_mpi<T, DENSE_MATRIX_LOCAL>(mat,mat_size,d,v,
                                                 order,k,maxiter,
                                                 tol,sigma,mode);
    }
    std::string order;
    int k;
    size_t mat_size;
    int maxiter;
    float tol;
    T sigma;
    int mode;
    SERIALIZE(order, k, mat_size, maxiter, tol, sigma, mode)
  };

  template <class DENSE_MATRIX, class DENSE_MATRIX_LOCAL, class T>
  void dense_eigen_sym(rowmajor_matrix<T>& mat,
                       diag_matrix_local<T>& d,
                       colmajor_matrix<T>& v,
                       const std::string& order,
                       int k,
                       int maxiter = std::numeric_limits<int>::max(),
                       float tol = 0.0) {
    auto dtmp = make_node_local_allocate<diag_matrix_local<T>>();
    v.data = make_node_local_allocate<colmajor_matrix_local<T>>();
    if(maxiter == std::numeric_limits<int>::max()) maxiter = mat.num_row;
    require(mat.num_col == mat.num_row, "dense_eigen_sym: matrix is not square");
    frovedis::time_spent t(INFO);  
    DENSE_MATRIX new_mat(mat);
    t.show("convert matrix format: ");
    new_mat.data.mapv(calc_dense_eigen_sym<T, DENSE_MATRIX_LOCAL>
                     (order, k, mat.num_row, maxiter, tol), 
                      dtmp, v.data);
    d = *dtmp.get_dvid().get_selfdata();
    v.set_num(mat.num_row, k);
  }

  template <class T>
  void dense_eigen_sym(rowmajor_matrix<T>& mat,
                       diag_matrix_local<T>& d,
                       colmajor_matrix<T>& v,
                       const std::string& order,
                       int k,
                       int maxiter = std::numeric_limits<int>::max(),
                       float tol = 0.0) {
    auto dtmp = make_node_local_allocate<diag_matrix_local<T>>();
    v.data = make_node_local_allocate<colmajor_matrix_local<T>>();
    if(maxiter == std::numeric_limits<int>::max()) maxiter = mat.num_row;
    require(mat.num_col == mat.num_row, "dense_eigen_sym: matrix is not square");
    mat.data.mapv(calc_dense_eigen_sym<T, rowmajor_matrix_local<T>>
                 (order, k, mat.num_row, maxiter, tol), dtmp, v.data);
    d = *dtmp.get_dvid().get_selfdata();
    v.set_num(mat.num_row, k);
  }

  template <class T>
  void compute_A_minus_sigma(rowmajor_matrix_local<T>& mat,
                             T sigma, size_t myst) {
    auto nrow = mat.local_num_row;
    auto ncol = mat.local_num_col;
    auto mptr = mat.val.data();
    for(size_t i = 0; i < nrow; ++i) mptr[i*ncol+(myst+i)] -= sigma;
  }

  template <class T>
  rowmajor_matrix<T>
  compute_Inv_AminusSig(rowmajor_matrix<T>& mat, T sigma, bool allow_redist) {
    auto nrows = mat.data.map(rowmajor_get_local_num_row<T>).gather();
    std::vector<size_t> sidx(nrows.size(),0);
    for(size_t i=1; i<nrows.size(); ++i) sidx[i] = sidx[i-1] + nrows[i-1];
    auto myst = make_node_local_scatter(sidx);
    mat.data.mapv(compute_A_minus_sigma<T>, broadcast(sigma), myst); 
    auto cmat = colmajor_matrix<T>(mat);
    size_t type;
    if(allow_redist) type = 2;
    else type = 1;
    auto bmat = blockcyclic_matrix<T>(cmat, type);
    auto inv_mat = inv(std::move(bmat));
    return inv_mat.to_rowmajor();
  }

  template <class T>
  void dense_eigen_sym(rowmajor_matrix<T>& mat,
                       diag_matrix_local<T>& d,
                       colmajor_matrix<T>& v,
                       const std::string& order,
                       int k, T sigma,
                       bool allow_redist = true,
                       int maxiter = std::numeric_limits<int>::max(),
                       float tol = 0.0) {
    auto dtmp = make_node_local_allocate<diag_matrix_local<T>>();
    v.data = make_node_local_allocate<colmajor_matrix_local<T>>();
    if(maxiter == std::numeric_limits<int>::max()) maxiter = mat.num_row;
    require(mat.num_col == mat.num_row, "dense_eigen_sym: matrix is not square");
    auto inv_mat = compute_Inv_AminusSig(mat, sigma, allow_redist);
    inv_mat.data.mapv(calc_dense_eigen_sym<T, rowmajor_matrix_local<T>>
                     (order, k, mat.num_row, maxiter, tol, sigma, 3), // mode:3 is for shift-invert
                     dtmp, v.data);
  
    d = *dtmp.get_dvid().get_selfdata();
    v.set_num(mat.num_row, k);
  }
}
#endif
