#define VLEN 256

#ifdef USE_COMBINED_GEMM_EACH
void w2v_gemm2( unsigned long n, unsigned long h, unsigned long w, int *Coffsets, float *A, float *B, float *C )
{
  float cbuf[VLEN] ;
#pragma _NEC vreg(cbuf)

  if( h <= VLEN ) {
    for( int i=0; i<w; i++ ) {
      int dstBase = Coffsets[i] * h ;
#pragma _NEC shortloop
      for( int k=0; k<h ; k++ ) {
        cbuf[k] = 0.0f ;
      }
#pragma _NEC novector
      for( int j=0; j<n; j++) {
        const float a = A[w*j+i] ;
#pragma _NEC shortloop
        for( int k=0; k<h ; k++ ) {
          cbuf[k] += a * B[h*j+k] ;
        }
      }
#pragma _NEC shortloop
      for( int k=0; k<h ; k++ ) {
        C[dstBase+k] += cbuf[k] ;
      }
    }
  }
  else {
    for( int i=0; i<w; i++ ) {
      int dstBase = Coffsets[i] * h ;
      for( int j=0; j<n; j++) {
        const float a = A[w*j+i] ;
        for( int k=0; k<h ; k++ ) {
          C[dstBase+k] += a * B[h*j+k] ;
        }
      }
    }
  }
}

#else // USE_COMBINED_GEMM_EACH

void w2v_gemm2( unsigned long n, unsigned long h, unsigned long w, float *A,  float *B, float *C )
{
  float cbuf[VLEN] ;
#pragma _NEC vreg(cbuf)

  if( h <= VLEN ) {
    for( int i=0; i<w; i++ ) {
#pragma _NEC shortloop
      for( int k=0; k<h ; k++ ) {
        cbuf[k] = 0.0f ;
      }
#pragma _NEC novector
      for( int j=0; j<n; j++) {
        const float a = A[w*j+i] ;
#pragma _NEC shortloop
        for( int k=0; k<h ; k++ ) {
          cbuf[k] += a * B[h*j+k] ;
        }
      }
#pragma _NEC shortloop
      for( int k=0; k<h ; k++ ) {
        C[i*h+k] = cbuf[k] ;
      }
    }
  }
  else {
    for( int i=0; i<w; i++ ) {
      for( int k=0; k<h ; k++ ) {
        C[i*h+k] = 0.0f ;
      }

      for( int j=0; j<n; j++) {
        const float a = A[w*j+i] ;
        for( int k=0; k<h ; k++ ) {
          C[i*h+k] += a * B[h*j+k] ;
        }
      }
    }
  }
}
#endif // USE_COMBINED_GEMM_EACH
