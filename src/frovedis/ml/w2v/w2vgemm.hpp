#ifndef _W2VGEMM_HPP_
#define _W2VGEMM_HPP_

#if defined(__ve__)

#if defined(USE_COMBINED_GEMM_ALL)
typedef void (*pFuncW2VKernel)(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
#elif defined(USE_COMBINED_GEMM_12)
typedef void (*pFuncVGemm0)(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
typedef void (*pFuncVGemm12)(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
#elif defined(USE_COMBINED_GEMM_EACH)
typedef void (*pFuncVGemm0)(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
typedef void (*pFuncVGemm2)(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
#else
typedef void (*pFuncVGemm0)(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
typedef void (*pFuncVGemm2)(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
#endif

extern "C" {
// w2v_kernel(gemm0,1,2)
#ifdef USE_COMBINED_GEMM_ALL
void w2v_kernel_N6_HU512_2X_W01(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_kernel_N6_HU512_2X_W02(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_kernel_N6_HU512_2X_W03(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_kernel_N6_HU512_2X_W04(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_kernel_N6_HU512_2X_W05(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_kernel_N6_HU512_2X_W06(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_kernel_N6_HU512_2X_W07(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_kernel_N6_HU512_2X_W08(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_kernel_N6_HU512_2X_W09(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_kernel_N6_HU512_2X_W10(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_kernel_N6_HU512_2X_W11(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_kernel_N6_HU512_2X_W12(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_kernel_N6_HU512_2X_W13(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_kernel_N6_HU512_2X_W14(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_kernel_N6_HU512_2X_W15(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_kernel_N6_HU512_2X_W16(unsigned long n, unsigned long h, unsigned long w, float alpha, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
#endif

// for fallback
#if defined(USE_COMBINED_GEMM_ALL)
void w2v_gemm0( unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C );
void w2v_gemm0_associated(unsigned long n, unsigned long w, float alpha, float *corrM);
void w2v_gemm1            (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2             (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
#endif

// gemm0
#if defined(USE_COMBINED_GEMM_ALL)
// nothing
#elif defined(USE_COMBINED_GEMM_12) || defined(USE_COMBINED_GEMM_EACH)
void w2v_gemm0             (unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU256       (unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X    (unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X_W01(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X_W02(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X_W03(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X_W04(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X_W05(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X_W06(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X_W07(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X_W08(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X_W09(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X_W10(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X_W11(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X_W12(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X_W13(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X_W14(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X_W15(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
void w2v_gemm0_HU512_2X_W16(unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C);
#else
void w2v_gemm0             (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU256       (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X    (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X_W01(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X_W02(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X_W03(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X_W04(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X_W05(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X_W06(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X_W07(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X_W08(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X_W09(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X_W10(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X_W11(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X_W12(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X_W13(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X_W14(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X_W15(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm0_HU512_2X_W16(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
#endif

// gemm0_associated
#if !defined(USE_COMBINED_GEMM_ALL)
void w2v_gemm0_associated(unsigned long n, unsigned long w, float alpha, float *corrM);
#endif

#if defined(USE_COMBINED_GEMM_ALL)
// nothing
#elif defined(USE_COMBINED_GEMM_12)
// gemm1,2
void w2v_gemm12_N6_HU512_2X_W01(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_gemm12_N6_HU512_2X_W02(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_gemm12_N6_HU512_2X_W03(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_gemm12_N6_HU512_2X_W04(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_gemm12_N6_HU512_2X_W05(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_gemm12_N6_HU512_2X_W06(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_gemm12_N6_HU512_2X_W07(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_gemm12_N6_HU512_2X_W08(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_gemm12_N6_HU512_2X_W09(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_gemm12_N6_HU512_2X_W10(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_gemm12_N6_HU512_2X_W11(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_gemm12_N6_HU512_2X_W12(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_gemm12_N6_HU512_2X_W13(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_gemm12_N6_HU512_2X_W14(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_gemm12_N6_HU512_2X_W15(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
void w2v_gemm12_N6_HU512_2X_W16(unsigned long n, unsigned long h, unsigned long w, int *pInputOffsets, int *pOutputOffsets, float *corrM, float *Woh, float *Wih);
#elif defined(USE_COMBINED_GEMM_EACH)
// gemm1
void w2v_gemm1            (unsigned long n, unsigned long h, unsigned long w, int *Boffsets, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm1_HU256      (unsigned long n, unsigned long h, unsigned long w, int *Boffsets, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm1_HU512_2X   (unsigned long n, unsigned long h, unsigned long w, int *Boffsets, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm1_N6_HU256   (unsigned long n, unsigned long h, unsigned long w, int *Boffsets, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm1_N6_HU512_2X(unsigned long n, unsigned long h, unsigned long w, int *Boffsets, int *Coffsets, float *A, float *B, float *C);

// gemm2
void w2v_gemm2             (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256       (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X    (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256_W01   (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256_W02   (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256_W03   (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256_W04   (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256_W05   (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256_W06   (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256_W07   (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256_W08   (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256_W09   (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256_W10   (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256_W11   (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256_W12   (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256_W13   (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256_W14   (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256_W15   (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU256_W16   (unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W01(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W02(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W03(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W04(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W05(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W06(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W07(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W08(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W09(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W10(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W11(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W12(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W13(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W14(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W15(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W16(unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C);
#else
// gemm1
void w2v_gemm1            (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm1_HU256      (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm1_HU512_2X   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm1_N6_HU256   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm1_N6_HU512_2X(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);

// gemm2
void w2v_gemm2             (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256       (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X    (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256_W01   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256_W02   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256_W03   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256_W04   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256_W05   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256_W06   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256_W07   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256_W08   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256_W09   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256_W10   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256_W11   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256_W12   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256_W13   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256_W14   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256_W15   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU256_W16   (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W01(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W02(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W03(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W04(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W05(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W06(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W07(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W08(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W09(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W10(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W11(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W12(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W13(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W14(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W15(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W16(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2_HU512_2X_W17(unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
#endif
}

#else  // __ve__

extern "C" {
void w2v_gemm0( unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C );
void w2v_gemm0_associated(unsigned long n, unsigned long w, float alpha, float *corrM);
void w2v_gemm1            (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
void w2v_gemm2             (unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C);
}

#endif  // __ve__

#endif // _W2VGEMM_
