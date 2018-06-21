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
