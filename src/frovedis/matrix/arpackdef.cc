#include "arpackdef.hpp"

template <>
void pxsaupd<double>(int* comm,
                     int *ido,
                     char *bmat,
                     int *nloc,
                     char *which,
                     int *nev,
                     double *tol,
                     double *resid,
                     int *ncv,
                     double *v,
                     int *ldv,
                     int *iparam,
                     int *ipntr,
                     double *workd,
                     double *workl,
                     int *lworkl,
                     int *info) {
  pdsaupd_(comm, ido, bmat, nloc, which, nev, tol, resid, ncv, v, ldv,
           iparam, ipntr, workd, workl, lworkl, info);
}

template <>
void pxsaupd<float>(int* comm,
                    int *ido,
                    char *bmat,
                    int *nloc,
                    char *which,
                    int *nev,
                    float *tol,
                    float *resid,
                    int *ncv,
                    float *v,
                    int *ldv,
                    int *iparam,
                    int *ipntr,
                    float *workd,
                    float *workl,
                    int *lworkl,
                    int *info) {
  pssaupd_(comm, ido, bmat, nloc, which, nev, tol, resid, ncv, v, ldv,
           iparam, ipntr, workd, workl, lworkl, info);
}

template <>
void pxseupd<double>(int* comm,
                     int *rvec,
                     char *howmny,
                     int *select,
                     double *d,
                     double *z,
                     int *ldz,
                     double *sigma,
                     char *bmat,
                     int *n,
                     char *which,
                     int *nev,
                     double *tol,
                     double *resid,
                     int *ncv,
                     double *v,
                     int *ldv,
                     int *iparam,
                     int *ipntr,
                     double *workd,
                     double *workl,
                     int *lworkl,
                     int *info) {
  pdseupd_(comm, rvec, howmny, select, d, z, ldz, sigma, bmat, n, which,
           nev, tol, resid, ncv, v, ldv, iparam, ipntr, workd, workl,
           lworkl, info);  
}

template <>
void pxseupd<float>(int* comm,
                    int *rvec,
                    char *howmny,
                    int *select,
                    float *d,
                    float *z,
                    int *ldz,
                    float *sigma,
                    char *bmat,
                    int *n,
                    char *which,
                    int *nev,
                    float *tol,
                    float *resid,
                    int *ncv,
                    float *v,
                    int *ldv,
                    int *iparam,
                    int *ipntr,
                    float *workd,
                    float *workl,
                    int *lworkl,
                    int *info) {
  psseupd_(comm, rvec, howmny, select, d, z, ldz, sigma, bmat, n, which,
           nev, tol, resid, ncv, v, ldv, iparam, ipntr, workd, workl,
           lworkl, info);  
}

// ----- non-symmetric -----
template <>
void pxnaupd<double>(int* comm,
                     int *ido,
                     char *bmat,
                     int *nloc,
                     char *which,
                     int *nev,
                     double *tol,
                     double *resid,
                     int *ncv,
                     double *v,
                     int *ldv,
                     int *iparam,
                     int *ipntr,
                     double *workd,
                     double *workl,
                     int *lworkl,
                     int *info) {
  pdnaupd_(comm, ido, bmat, nloc, which, nev, tol, resid, ncv, v, ldv,
           iparam, ipntr, workd, workl, lworkl, info);
}

template <>
void pxnaupd<float>(int* comm,
                    int *ido,
                    char *bmat,
                    int *nloc,
                    char *which,
                    int *nev,
                    float *tol,
                    float *resid,
                    int *ncv,
                    float *v,
                    int *ldv,
                    int *iparam,
                    int *ipntr,
                    float *workd,
                    float *workl,
                    int *lworkl,
                    int *info) {
  psnaupd_(comm, ido, bmat, nloc, which, nev, tol, resid, ncv, v, ldv,
           iparam, ipntr, workd, workl, lworkl, info);
}

template <>
void pxneupd<double>(int* comm,
                     int *rvec,
                     char *howmny,
                     int *select,
                     double *dr,
                     double *di,
                     double *z,
                     int *ldz,
                     double *sigmar,
                     double *sigmai,
                     double *workv,
                     char *bmat,
                     int *n,
                     char *which,
                     int *nev,
                     double *tol,
                     double *resid,
                     int *ncv,
                     double *v,
                     int *ldv,
                     int *iparam,
                     int *ipntr,
                     double *workd,
                     double *workl,
                     int *lworkl,
                     int *info) {
  pdneupd_(comm, rvec, howmny, select, dr, di, z, ldz, sigmar, sigmai, workv,
           bmat, n, which, nev, tol, resid, ncv, v, ldv, iparam, ipntr,
           workd, workl, lworkl, info);  
}

template <>
void pxneupd<float>(int* comm,
                    int *rvec,
                    char *howmny,
                    int *select,
                    float *dr,
                    float *di,
                    float *z,
                    int *ldz,
                    float *sigmar,
                    float *sigmai,
                    float *workv,
                    char *bmat,
                    int *n,
                    char *which,
                    int *nev,
                    float *tol,
                    float *resid,
                    int *ncv,
                    float *v,
                    int *ldv,
                    int *iparam,
                    int *ipntr,
                    float *workd,
                    float *workl,
                    int *lworkl,
                    int *info) {
  psneupd_(comm, rvec, howmny, select, dr, di, z, ldz, sigmar, sigmai, workv,
           bmat, n, which, nev, tol, resid, ncv, v, ldv, iparam, ipntr,
           workd, workl, lworkl, info);  
}
