#include <math.h>
#ifdef W2V_USE_INTRINSIC
#include "veintrin.h"
#endif

// #include "w2v_common.hpp"
// import MAX_EXP from w2v_common.hpp
#define MAX_EXP 6


void w2v_gemm0_associated(unsigned long n, unsigned long w, const float alpha, float *corrM)
{
#ifndef W2V_USE_INTRINSIC
#pragma _NEC shortloop
  for (int ij = 0; ij < n*w; ij++) {
    float f = corrM[ij];
    // int label = (ij < w ? 0 : 1);
    // corrM[ij] = (label - 1.0f / (1.0f + expf(-1.0f * f))) * alpha;
    float label = 1.0f;
    if (ij < w) {
      label = 0.0f;
    }

    float r = (label - 1.0f / (1.0f + expf(-1.0f * f))) * alpha;
    if (MAX_EXP < f) {
      r = (label - 1.0f) * alpha;
    }
    if (f < -MAX_EXP) {
      r = label * alpha;
    }
    corrM[ij] = r;
  }
#else
  const unsigned long nw = n * w ;
  _ve_lvl(nw) ;

  // float label = 1.0f;
  // if (0 < w - ij) {
  //   label = 0.0f;
  // }
  __vr vij = _ve_vseq_v() ;
  __vr vlabel = _ve_vbrdu_vs_f32(1.0f);
  __vm256 vm_ij_g_w = _ve_vfmkw_mcv(VECC_G, _ve_vsubswsx_vsv(w, vij));
  vlabel = _ve_vbrdu_vsmv_f32(0.0f, vm_ij_g_w, vlabel);

  // float f = corrM[ij];
  __vr vf = _ve_vldu_vss(4, corrM) ;    // load corrM

  // float r = (label - 1.0f / (1.0f + expf(-1.0f * f))) * alpha;
  __vr vr;
  vr = _ve_vfmuls_vsv(-1.0f, vf) ;   // (-1.0 * f)
  vr = _ve_vec_expf_vv(vr) ;         // expf(-1.0 * f)
  vr = _ve_vfadds_vsv(1.0f, vr) ;    // (1.0 + expf(-1. * f))
  vr = _ve_vfdivs_vsv(1.0f, vr) ;    // 1.0 / (1.0 + expf(-1. * f))

  // if (MAX_EXP - f < 0) {
  //   // r = (label - 1.0f) * alpha;
  //   (1.0 + expf(-1. * f)) = 1.0f;
  // }
  __vm256 vm_f_gt_exp = _ve_vfmkw_mcv(VECC_L, _ve_vfsubs_vsv((float)MAX_EXP, vf));
  vr = _ve_vbrdu_vsmv_f32(1.0f, vm_f_gt_exp, vr);

  // if (0 < -MAX_EXP - f) {
  //   // r = label * alpha;
  //   (1.0 + expf(-1. * f)) = 0.0f;
  // }
  __vm256 vm_f_lt_exp = _ve_vfmkw_mcv(VECC_G, _ve_vfsubs_vsv((float)-MAX_EXP, vf));
  vr = _ve_vbrdu_vsmv_f32(0.0f, vm_f_lt_exp, vr);

  vr = _ve_vfsubs_vvv(vlabel, vr) ;  // (label - 1. / (1. + expf(-1. * f)))
  vr = _ve_vfmuls_vsv(alpha, vr) ;   // (label - 1. / (1. + expf(-1. * f))) * alpha

  _ve_vstu_vss(vr, 4, corrM) ;
#endif
}
