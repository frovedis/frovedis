#define VLEN 256

#if defined(USE_COMBINED_GEMM_12) || defined(USE_COMBINED_GEMM_EACH)
void w2v_gemm0( unsigned long n, unsigned long h, unsigned long w, int *Aoffsets, int *Boffsets, float *A, float *B, float *Ad, float *C )
{
  float abuf[VLEN] ;
#pragma _NEC vreg(abuf)

  if( h <= VLEN ) {
    for( int i=0; i<n; i++ ) {
      int baseA = h * Aoffsets[i] ;
#pragma _NEC shortloop
      for( int k=0; k<h ; k++ ) {
        abuf[k] = A[baseA+k] ;
#ifdef USE_COMBINED_GEMM_EACH
        Ad[h*i+k] = abuf[k];
#endif
      }
#pragma _NEC novector
      for( int j=0; j<w; j++) {
        float sum = 0 ;
        int baseB = h * Boffsets[j] ;
#pragma _NEC shortloop
        for( int k=0; k<h ; k++ ) {
          sum += abuf[k] * B[baseB+k] ;
        }
        C[w*i+j] = sum ;
      }
    }
  }
  else {
    for( int i=0; i<n; i++ ) {
      for( int k=0; k<h ; k++ ) {
        Ad[h*i+k] = A[h*Aoffsets[i]+k] ;
      }
#pragma _NEC novector
      for( int j=0; j<w; j++) {
        float sum = 0 ;
        for( int k=0; k<h ; k++ ) {
          sum += A[h*Aoffsets[i]+k] * B[h*Boffsets[j]+k] ;
        }
        C[w*i+j] = sum ;
      }
    }
  }
}
#else
void w2v_gemm0( unsigned long n, unsigned long h, unsigned long w, float *A, float *B, float *C )
{
  float abuf[VLEN] ;
#pragma _NEC vreg(abuf)

  if( h <= VLEN ) {
    for( int i=0; i<n; i++ ) {
#pragma _NEC shortloop
      for( int k=0; k<h ; k++ ) {
        abuf[k] = A[h*i+k] ;
      }
#pragma _NEC novector
      for( int j=0; j<w; j++) {
        float sum = 0 ;
#pragma _NEC shortloop
        for( int k=0; k<h ; k++ ) {
          sum += abuf[k] * B[h*j+k] ;
        }
        C[w*i+j] = sum ;
      }
    }
  }
  else {
    for( int i=0; i<n; i++ ) {
#pragma _NEC novector
      for( int j=0; j<w; j++) {
        float sum = 0 ;
        for( int k=0; k<h ; k++ ) {
          sum += A[h*i+k] * B[h*j+k] ;
        }
        C[w*i+j] = sum ;
      }
    }
  }
}
#endif
