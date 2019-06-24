#ifndef _DUMMY_MODEL_HPP_
#define _DUMMY_MODEL_HPP_

namespace frovedis {
struct dummy_glm {
  dummy_glm() {}
  dummy_glm(int id, short kind, size_t nftr, int ncls=2, 
            double thr=0.5) : mid(id), mkind(kind),
            numFeatures(nftr),numClasses(ncls),threshold(thr) {}
  void debug_print() {
    std::cout << "mid: " << mid
              << ", mkind: " << mkind
              << ", numFeatures: " << numFeatures
              << ", numClasses: " << numClasses
              << ", threshold: " << threshold << "\n";             
  }
  int mid;
  short mkind;
  size_t  numFeatures;
  int  numClasses;
  double threshold;
  SERIALIZE(mid, mkind, numFeatures, numClasses, threshold)
};

struct dummy_mfm {
  dummy_mfm() {}
  dummy_mfm(int id, int r, int nr, int nc): 
            mid(id),rank(r),nrow(nr),ncol(nc) {}
  void debug_print() {
    std::cout << "mid: " << mid
              << ", rank: " << rank
              << ", nrow: " << nrow
              << ", ncol: " << ncol << "\n";
  }
  int mid, rank, nrow, ncol;
  SERIALIZE(mid, rank, nrow, ncol)
};

}
#endif
