#define VLEN 256

#ifdef USE_COMBINED_GEMM_EACH
void w2v_gemm1( unsigned long n, unsigned long h, unsigned long w, int *Boffsets, int *Coffsets, float *A, float *B, float *C )
{
  float cbuf[VLEN] ;
#pragma _NEC vreg(cbuf)

  if( h <= VLEN ) {
    for( int i=0; i<n; i++ ) {
      int dstBase = Coffsets[i] * h ;
#pragma _NEC shortloop
      for( int k=0; k<h ; k++ ) {
        cbuf[k] = 0.0f ;
      }
#pragma _NEC novector
      for( int j=0; j<w; j++) {
        const float a = A[w*i+j] ;
        int baseB = Boffsets[j] * h;
#pragma _NEC shortloop
        for( int k=0; k<h ; k++ ) {
          cbuf[k] += a * B[baseB+k] ;
        }
      }
#pragma _NEC shortloop
      for( int k=0; k<h ; k++ ) {
        C[dstBase+k] += cbuf[k] ;
      }
    }
  }
  else {
    for( int i=0; i<n; i++ ) {
      int dstBase = Coffsets[i] * h ;
      for( int j=0; j<w; j++) {
        const float a = A[w*i+j] ;
        int baseB = Boffsets[j] * h;
        for( int k=0; k<h ; k++ ) {
          C[dstBase+k] += a * B[baseB+k] ;
        }
      }
    }
  }
}

#else // USE_COMBINED_GEMM_EACH

void w2v_gemm1( unsigned long n, unsigned long h, unsigned long w, float *A,  float *B, float *C )
{
  float cbuf[VLEN] ;
#pragma _NEC vreg(cbuf)

  if( h <= VLEN ) {
    for( int i=0; i<n; i++ ) {
#pragma _NEC shortloop
      for( int k=0; k<h ; k++ ) {
        cbuf[k] = 0.0f ;
      }
#pragma _NEC novector
      for( int j=0; j<w; j++) {
        const float a = A[w*i+j] ;
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

    for( int i=0; i<n; i++ ) {
      for( int k=0; k<h ; k++ ) {
        C[i*h+k] = 0.0f ;
      }

      for( int j=0; j<w; j++) {
        const float a = A[w*i+j] ;
        for( int k=0; k<h ; k++ ) {
          C[i*h+k] += a * B[h*j+k] ;
        }
      }
    }
  }
}
#endif // USE_COMBINED_GEMM_EACH
