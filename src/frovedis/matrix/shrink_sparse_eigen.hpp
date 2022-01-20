#ifndef SHRINK_SPARSE_EIGEN_HPP
#define SHRINK_SPARSE_EIGEN_HPP

#include <sstream>

#include "crs_matrix.hpp"
#include "ccs_matrix.hpp"
#include "colmajor_matrix.hpp"
#include "diag_matrix.hpp"
#include "arpackdef.hpp"
#include "shrink_matrix.hpp"

namespace frovedis {
namespace shrink {

template <class REAL, class I, class SPARSE_MATRIX_LOCAL>
void eigen_sym_mpi(SPARSE_MATRIX_LOCAL& mat,
                   size_t mat_size,
                   std::vector<I>& tbl,
                   diag_matrix_local<REAL>& ret_d,
                   colmajor_matrix_local<REAL>& ret_v,
                   std::string order,
                   int k, int maxiter, REAL tol) {
  int rank, size;
  MPI_Comm_rank(frovedis_comm_rpc, &rank);
  MPI_Comm_size(frovedis_comm_rpc, &size);
  MPI_Comm comm = frovedis_comm_rpc;
  MPI_Fint fcomm = MPI_Comm_c2f(frovedis_comm_rpc);

  int mloc = mat.local_num_row;
  int n = mat_size;
  int n_each = ceil_div(n, size); 
  // here nloc is block distributed size
  int nloc = 0; 
  if(rank * n_each < n) {
    nloc = std::min(n_each, n - rank * n_each);
  } else nloc = 0;

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
  iparam[6] = 1;
  int ipntr[11];
  REAL* workd = new REAL[3*nloc];
  int lworkl = ncv * (ncv + 8);
  REAL* workl = new REAL[lworkl];
  int info = 0;

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

  std::vector<I> column_partition(size+1);
  auto column_partitionp = column_partition.data();
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
      spmv_lap.lap_stop();
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
    REAL sigma;
    arpack_lap.lap_start();
    pxseupd<REAL>(&fcomm, &rvec, &howmny, select, s, z, &ldv, &sigma, 
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
struct calc_eigen_sym {
  calc_eigen_sym() {}
  calc_eigen_sym(std::string order, int k, size_t mat_size, 
                 int maxiter, float tol) :
                 order(order), k(k), mat_size(mat_size), 
                 maxiter(maxiter), tol(tol) {}
  void operator()(SPARSE_MATRIX_LOCAL& mat,
                  diag_matrix_local<T>& d,
                  colmajor_matrix_local<T>& v,
                  std::vector<I>& tbl) {
    eigen_sym_mpi<T, I, SPARSE_MATRIX_LOCAL>(mat, mat_size,
                                             tbl, d, v, order, k, 
                                             maxiter, tol);
  }
  std::string order;
  int k;
  size_t mat_size;
  int maxiter;
  float tol;
  SERIALIZE(order, k, mat_size, maxiter, tol)
};

template <class T, class I, class O>
void sparse_eigen_sym(crs_matrix<T,I,O>& mat,
                      diag_matrix_local<T>& d,
                      colmajor_matrix<T>& v,
                      std::string order,
                      int k, int maxiter = std::numeric_limits<int>::max(),
                      float tol = 0.0) { 
  auto dtmp = make_node_local_allocate<diag_matrix_local<T>>();
  v.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  if(maxiter == std::numeric_limits<int>::max()) maxiter = mat.num_row;
  require(mat.num_col == mat.num_row, "sparse_eigen_sym: matrix is not square");
  frovedis::time_spent t(DEBUG);
  auto tbl = shrink_column(mat);
  t.show("shrink_column: ");
  mat.data.mapv(calc_eigen_sym<T,I,crs_matrix_local<T,I,O>>
                (order, k, mat.num_row, maxiter, tol),
                 dtmp, v.data, tbl);
  d = *dtmp.get_dvid().get_selfdata();
  v.set_num(mat.num_row, k);
}

template <class SPARSE_MATRIX,
          class SPARSE_MATRIX_LOCAL,
          class T, class I, class O>
void sparse_eigen_sym(crs_matrix<T,I,O>& mat,
                      diag_matrix_local<T>& d,
                      colmajor_matrix<T>& v,
                      std::string order,
                      int k, int maxiter = std::numeric_limits<int>::max(),
                      float tol = 0.0) {
  auto dtmp = make_node_local_allocate<diag_matrix_local<T>>();
  v.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  if(maxiter == std::numeric_limits<int>::max()) maxiter = mat.num_row;
  require(mat.num_col == mat.num_row, "sparse_eigen_sym: matrix is not square");
  frovedis::time_spent t(DEBUG);
  auto tbl = shrink_column(mat);
  t.show("shrink_column: ");
  SPARSE_MATRIX new_mat(mat);
  t.show("convert matrix format: ");
  new_mat.data.mapv(calc_eigen_sym<T,I,SPARSE_MATRIX_LOCAL>
                   (order, k, mat.num_row, maxiter, tol),
                    dtmp, v.data, tbl);
  d = *dtmp.get_dvid().get_selfdata();
  v.set_num(mat.num_row, k);
}


// for non symmetric matrix
template <class REAL, class I, class SPARSE_MATRIX_LOCAL>
void eigen_mpi(SPARSE_MATRIX_LOCAL& mat,
               size_t mat_size,
               std::vector<I>& tbl,
               diag_matrix_local<REAL>& ret_dr,
               diag_matrix_local<REAL>& ret_di,
               colmajor_matrix_local<REAL>& ret_vr,
               colmajor_matrix_local<REAL>& ret_vi,
               std::string order,
               int k, int maxiter, REAL tol) {
  int rank, size;
  MPI_Comm_rank(frovedis_comm_rpc, &rank);
  MPI_Comm_size(frovedis_comm_rpc, &size);
  MPI_Comm comm = frovedis_comm_rpc;
  MPI_Fint fcomm = MPI_Comm_c2f(frovedis_comm_rpc);

  int mloc = mat.local_num_row;
  int n = mat_size;
  int n_each = ceil_div(n, size); 
  // here nloc is block distributed size
  int nloc = 0; 
  if(rank * n_each < n) {
    nloc = std::min(n_each, n - rank * n_each);
  } else nloc = 0;

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
  iparam[6] = 1;
  int ipntr[14];
  REAL* workd = new REAL[3*nloc];
  int lworkl = 3 * ncv * ncv + 6 * ncv;
  REAL* workl = new REAL[lworkl];
  int info = 0;

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

  std::vector<I> column_partition(size+1);
  auto column_partitionp = column_partition.data();
  for(int i = 1; i < size+1; i++)
    column_partitionp[i] = column_partitionp[i-1] + each_np[i-1];
  auto mat_info = create_shrink_vector_info_local(tbl, column_partition);

  frovedis::time_spent t(DEBUG), t2(TRACE);
  frovedis::time_spent arpack_lap(DEBUG), mpi_lap(DEBUG), spmv_lap(DEBUG);
  int count = 0;
  while(1) {
    arpack_lap.lap_start();
    pxnaupd<REAL>(&fcomm, &ido, &bmat, &nloc, which, &nev, &tol, resid, 
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
      spmv_lap.lap_stop();
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
    ss << "p[ds]naupd count: " << count;
    t.show(ss.str() + ", p[ds]naupd time: ");
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
    ret_dr.val.resize(nev);
    ret_di.val.resize(nev);
    REAL* dr = new REAL[nev+1];
    REAL* di = new REAL[nev+1];
    ret_vr.val.resize(nloc*nev);
    ret_vi.val.resize(nloc*nev);
    ret_vr.local_num_col = nev;
    ret_vr.local_num_row = nloc;
    ret_vi.local_num_col = nev;
    ret_vi.local_num_row = nloc;
    REAL* z = new REAL[nloc*(nev+1)];
    REAL sigmar, sigmai;
    REAL* workv = new REAL[3*ncv];
    arpack_lap.lap_start();
    pxneupd<REAL>(&fcomm, &rvec, &howmny, select, dr, di, z, &ldv,
                  &sigmar, &sigmai, workv,
                  &bmat, &nloc, which, &nev, &tol, resid, &ncv, v, &ldv, 
                  iparam, ipntr, workd, workl, &lworkl, &info);
    arpack_lap.lap_stop();
    auto ret_drp = ret_dr.val.data();
    auto ret_dip = ret_di.val.data();
    for(size_t i = 0; i < nev; i++) {
      ret_drp[i] = dr[i];
      ret_dip[i] = di[i];
    }
    auto ret_vrp = ret_vr.val.data();
    auto ret_vip = ret_vi.val.data();
    for(size_t i = 0; i < nev; i++) {
      if(std::abs(di[i]) > 0) {
        for(size_t j = 0; j < nloc; j++) {
          ret_vrp[i * nloc + j] = z[i * nloc + j];
          ret_vrp[(i + 1) * nloc + j] = z[i * nloc + j];
          ret_vip[i * nloc + j] = z[(i + 1) * nloc + j];
          ret_vip[(i + 1) * nloc + j] = -z[(i + 1) * nloc + j];
        }
        i++;
      } else {
        for(size_t j = 0; j < nloc; j++) {
          ret_vrp[i * nloc + j] = z[i * nloc + j];
        }
      }
    }
    if(rank == 0) t.show("p[ds]neupd time: ");
    if(info < 0) {
      REPORT_ERROR(INTERNAL_ERROR, 
      "error with dseupd, info = " + STR(info) + "\n");
    }
    if(rank == 0) {
      arpack_lap.show_lap("arpack time: ");
      mpi_lap.show_lap("MPI time: ");
      spmv_lap.show_lap("SpMV time: ");
    }
    delete [] select;
    delete [] workv;
    delete [] dr;
    delete [] di;
    delete [] z;
  }
    
  delete [] resid;
  delete [] v;
  delete [] workd;
  delete [] workl;
}

template <class T, class I, class SPARSE_MATRIX_LOCAL>
struct calc_eigen {
  calc_eigen() {}
  calc_eigen(std::string order, int k, size_t mat_size, 
                 int maxiter, float tol) :
                 order(order), k(k), mat_size(mat_size), 
                 maxiter(maxiter), tol(tol) {}
  void operator()(SPARSE_MATRIX_LOCAL& mat,
                  diag_matrix_local<T>& dr,
                  diag_matrix_local<T>& di,
                  colmajor_matrix_local<T>& vr,
                  colmajor_matrix_local<T>& vi,
                  std::vector<I>& tbl) {
    eigen_mpi<T, I, SPARSE_MATRIX_LOCAL>(mat, mat_size,
                                         tbl, dr, di, vr, vi, order, k, 
                                         maxiter, tol);
  }
  std::string order;
  int k;
  size_t mat_size;
  int maxiter;
  float tol;
  SERIALIZE(order, k, mat_size, maxiter, tol)
};

template <class T, class I, class O>
void sparse_eigen(crs_matrix<T,I,O>& mat,
                  diag_matrix_local<T>& dr,
                  diag_matrix_local<T>& di,
                  colmajor_matrix<T>& vr,
                  colmajor_matrix<T>& vi,
                  std::string order,
                  int k, int maxiter = std::numeric_limits<int>::max(),
                  float tol = 0.0) { 
  auto drtmp = make_node_local_allocate<diag_matrix_local<T>>();
  auto ditmp = make_node_local_allocate<diag_matrix_local<T>>();
  vr.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  vi.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  if(maxiter == std::numeric_limits<int>::max()) maxiter = mat.num_row;
  require(mat.num_col == mat.num_row, "sparse_eigen: matrix is not square");
  frovedis::time_spent t(DEBUG);
  auto tbl = shrink_column(mat);
  t.show("shrink_column: ");
  mat.data.mapv(calc_eigen<T,I,crs_matrix_local<T,I,O>>
                (order, k, mat.num_row, maxiter, tol),
                drtmp, ditmp, vr.data, vi.data, tbl);
  dr = *drtmp.get_dvid().get_selfdata();
  di = *ditmp.get_dvid().get_selfdata();
  vr.set_num(mat.num_row, k);
  vi.set_num(mat.num_row, k);
}

template <class SPARSE_MATRIX,
          class SPARSE_MATRIX_LOCAL,
          class T, class I, class O>
void sparse_eigen(crs_matrix<T,I,O>& mat,
                  diag_matrix_local<T>& dr,
                  diag_matrix_local<T>& di,
                  colmajor_matrix<T>& vr,
                  colmajor_matrix<T>& vi,
                  std::string order,
                  int k, int maxiter = std::numeric_limits<int>::max(),
                  float tol = 0.0) {
  auto drtmp = make_node_local_allocate<diag_matrix_local<T>>();
  auto ditmp = make_node_local_allocate<diag_matrix_local<T>>();
  vr.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  vi.data = make_node_local_allocate<colmajor_matrix_local<T>>();
  if(maxiter == std::numeric_limits<int>::max()) maxiter = mat.num_row;
  require(mat.num_col == mat.num_row, "sparse_eigen: matrix is not square");
  frovedis::time_spent t(DEBUG);
  auto tbl = shrink_column(mat);
  t.show("shrink_column: ");
  SPARSE_MATRIX new_mat(mat);
  t.show("convert matrix format: ");
  new_mat.data.mapv(calc_eigen<T,I,SPARSE_MATRIX_LOCAL>
                   (order, k, mat.num_row, maxiter, tol),
                    drtmp, ditmp, vr.data, vi.data, tbl);
  dr = *drtmp.get_dvid().get_selfdata();
  di = *ditmp.get_dvid().get_selfdata();
  vr.set_num(mat.num_row, k);
  vi.set_num(mat.num_row, k);
}

}  //shrink namepace
}  // frovedis namespace
#endif
