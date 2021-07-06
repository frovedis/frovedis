#ifndef _LSQR_
#define _LSQR_
#include <frovedis/core/vector_operations.hpp>
#include <frovedis/matrix/jds_crs_hybrid.hpp>
#if !defined(LSQR_JDS) && !defined(LSQR_CRS) && !defined(LSQR_HYBRID)
#if defined(_SX) || defined(__ve__)
#define LSQR_HYBRID
#else
#define LSQR_CRS
#endif
#endif

namespace frovedis {
  template <class T>
  struct sparse_lsqr_result{
    std::vector<T> x;
    int istop,itn;
    float r1norm, r2norm, anorm, acond, xnorm;
    double arnorm;
    std::vector<T> var;
    sparse_lsqr_result(std::vector<T> x, int itn, 
                       int istop, float r1norm, 
                       float r2norm, double arnorm, 
                       float anorm, float acond, 
                       float xnorm, std::vector<T> var){
      this -> x = x;
      this -> itn = itn;
      this -> istop = istop;
      this -> r1norm = r1norm;
      this -> r2norm = r2norm;
      this -> arnorm = arnorm;
      this -> anorm = anorm;
      this -> acond = acond;
      this -> xnorm = xnorm;
      this -> var = var;
    }
    SERIALIZE(x, istop, itn, r1norm, r2norm, anorm, acond, arnorm, xnorm, var);
  };
  template <class T>
  void sym_ortho(T a, T b, T& c, T& s, T& r){
    if(b == 0){
      c = ((a == 0) ? 0 : (a > 0) ? 1 : -1);
      s = 0;
      r = std::fabs(a);
    }
    else if(a == 0){
      c = 0;
      s = ((b == 0) ? 0 : (b > 0) ? 1 : -1);
      r = std::fabs(b);
    }
    else if(std::fabs(b) > std::fabs(a)){
      auto tau = a / b;
      s = ((b == 0) ? 0 : (b > 0) ? 1 : -1) / std::sqrt(1 + tau * tau);
      c = s * tau;
      r = b / s;
    }
    else{
      auto tau = b / a;
      c = ((a == 0) ? 0 : (a > 0) ? 1 : -1) / std::sqrt(1 + tau * tau);
      s = c * tau;
      r = a / c;
    }
  }
  
  template <class T, class I, class O>
  std::vector<T>
  operator*(crs_matrix<T,I,O>& mat, std::vector<T>& vec){
    auto res = mat.data.map(call_crs_mv<T,I,O>, broadcast(vec))
                       .template moveto_dvector<T>().gather();
    return res;
  }

  template <class T, class I, class O>
  std::vector<T>
  operator*(jds_crs_hybrid<T,I,O>& mat, std::vector<T>& vec){
    auto res = mat.data.map(call_hyb_mv<T,I,O>, broadcast(vec))
                       .template moveto_dvector<T>().gather();
    return res;
  }

  template <class T, class I, class O>
  std::vector<T>
  operator*(jds_matrix<T,I,O>& mat, std::vector<T>& vec){
    auto res = mat.data.map(call_jds_mv<T,I,O>, broadcast(vec))
                       .template moveto_dvector<T>().gather();
    return res;
  }


  struct simple_matvec {
    simple_matvec() {}
    template <class MATRIX, class T>
    std::vector<T> operator()(MATRIX& m, std::vector<T>& v) { return m * v; }
    SERIALIZE_NONE
  };
  
  template <class T, class I, class O>
  sparse_lsqr_result<T> 
  sparse_lsqr(crs_matrix<T, I, O>& mat, dvector<T>& b_dv,
              int iter_lim = std::numeric_limits<int>::max(),
              bool show = false,
              float damp = 0.0,
              double atol = 1e-8,
              double btol = 1e-8,
              double conlim = 1e8,
              bool calc_var = false,
              const std::vector<T>& x0 = {}) {
    auto linop = simple_matvec();
    return sparse_lsqr(mat, b_dv, linop, linop, iter_lim, show, damp, atol, 
                       btol, conlim, calc_var, x0);
  }

  template <class T, class I, class O, 
            class LINOP1, class LINOP2>
  sparse_lsqr_result<T> 
  sparse_lsqr(crs_matrix<T, I, O>& mat, dvector<T>& b_dv,
              LINOP1& func1, LINOP2& func2,
              int iter_lim = std::numeric_limits<int>::max(),
              bool show = false,
              float damp = 0.0,
              double atol = 1e-8,
              double btol = 1e-8,
              double conlim = 1e8,
              bool calc_var = false,
              const std::vector<T>& x0 = {}){
    auto mat_trans = mat.transpose();
   #ifdef LSQR_HYBRID
     jds_crs_hybrid<T,I,O> A(mat);
     jds_crs_hybrid<T,I,O> A_trans(mat_trans);
   #elif defined LSQR_JD
     jds_matrix<T,I,O> A(mat);
     jds_matrix<T,I,O> A_trans(mat_trans);
   #elif defined LSQR_CRS
     crs_matrix<T,I,O>& A = mat;
     crs_matrix<T,I,O>& A_trans = mat_trans;
   #else
   #error specify matrix format by defining JDS, CRS, or HYBRID
   #endif
    auto b = b_dv.gather();
    int m = A.num_row;
    int n = A.num_col;
    require(m == b.size(), 
    "number of samples differ in input matrix and label");
    auto eps = std::numeric_limits<double>::epsilon();
    if(iter_lim == std::numeric_limits<int>::max()){
      iter_lim = 2 * n;
    }
    std::string msg[8] = {"The exact solution is  x = 0",
                          "Ax - b is small enough, given atol, btol",
                          "The least-squares solution is good enough, given atol",
                          "The estimate of cond(Abar) has exceeded conlim",
                          "Ax - b is small enough for this machine",
                          "The least-squares solution is good enough for this machine",
                          "Cond(Abar) seems to be too large for this machine",
                          "The iteration limit has been reached"};
    if(show){
      std::cout << " " << std::endl;
      std::cout << "LSQR\tLeast-squares solution of  Ax = b" << std::endl;
      std::cout << "The A matrix has " << m << " rows";
      std::cout << " and " << n << " columns" << std::endl;
      std::cout << std::setprecision(15) << std::scientific;
      std::cout << "damp = " << damp;
      std::cout << "   cal_var = " << int(calc_var) <<std::endl;
      std::cout << std::setprecision(2) << std::scientific;
      std::cout << "atol = " << atol;
      std::cout << std::setprecision(2) << std::scientific;
      std::cout << "\tconlim = " << conlim;
      std::cout << std::setw(8) << std::setprecision(2) << std::scientific;
      std::cout << "\nbtol = " << btol;
      std::cout << "\titer_lim = " << iter_lim << std::endl;
    }
    std::vector<T> var(n);
    std::vector<T> x;
    int itn = 0, istop = 0;
    float ctol = 0, dampsq = damp * damp;
    if(conlim > 0) ctol = 1 / conlim;
    float anorm = 0, acond = 0, xnorm = 0;
    T alfa = 0, beta = 0, test1 = 0;
    T test2 = 0, ddnorm = 0, res2 = 0;
    T xxnorm = 0, z = 0, cs2 = -1;
    int sn2 = 0;
    std::vector<T> u = b;
    std::vector<T> v;
    auto bnorm = vector_norm(b);
    if(x0.empty()){
      x.resize(n);
      beta = bnorm;
    }
    else{
      require(x0.size() == n, 
              std::string("size of initial solution vector, x0,") +
              std::string("doesn't match the number of features in matrix"));
      x = x0;
      u = u - func1(A, x);
      beta = vector_norm(u);
    }
    if(beta > 0){
      u = u * (1 / beta);
      v = func2(A_trans, u);
      alfa = vector_norm(v);
    }
    else{
      v = x;
      alfa = 0;
    }
    if(alfa > 0){
      v = v * (1 / alfa);
    }
    std::vector<T> w = v;
    auto rhobar = alfa;
    auto phibar = beta;
    auto rnorm = beta;
    auto r1norm = rnorm;
    auto r2norm = rnorm;
    auto arnorm = alfa * beta;
    if(arnorm == 0) {
      if(show) {
        RLOG(INFO) << msg[0] << std::endl;
      }
      sparse_lsqr_result<T> ret_val(x, itn, istop, r1norm,
                                    r2norm, arnorm, anorm,
                                    acond, xnorm, var);
      return ret_val;
    }
    if(show){
        std::cout << "\nItn\t  x[0]\t\t   r1norm\t   r2norm\t";
        std::cout << " Compatible\t  LS\t     Norm A\t    Cond A" << std::endl;
        test1 = 1;
        test2 = alfa / beta;
        std::cout << itn << " ";
        std::cout << std::setw(5) << std::setprecision(5) << std::scientific;
        std::cout << " ";
        std::cout << x[0] << " ";
        std::cout << std::setw(6) << std::setprecision(3) << std::scientific;
        std::cout << " ";
        std::cout << r1norm << " ";
        std::cout << std::setw(6) << std::setprecision(3) << std::scientific;
        std::cout << " ";
        std::cout << r2norm << " ";
        std::cout << std::setw(6) << std::setprecision(1) << std::scientific;
        std::cout << " ";
        std::cout << test1 << " ";
        std::cout << std::setw(6) << std::setprecision(1) << std::scientific;
        std::cout << " ";
        std::cout << test2 << " ";
      }
    //Main iteration loop.
    while(itn < iter_lim){
      ++itn;
      u = func1(A, v) - (u * alfa);
      beta = vector_norm(u);
      if(beta > 0){
        u = u * (1 / beta);
        anorm = std::sqrt(anorm * anorm  + alfa * alfa 
                          + beta * beta + damp * damp);
        v = func2(A_trans, u) - (v * beta);
        alfa = vector_norm(v);
        if(alfa > 0)v = v * (1 / alfa);
      }
      auto rhobar1 = std::sqrt(rhobar * rhobar + damp * damp);
      auto cs1 = rhobar / rhobar1;
      auto sn1 = damp / rhobar1;
      auto psi = sn1 * phibar;
      phibar = cs1 * phibar;
      T cs = 0, sn = 0, rho = 0;
      sym_ortho(rhobar1, beta, cs, sn, rho);
      auto theta = sn * alfa;
      rhobar = -1 * cs * alfa;
      auto phi = cs * phibar;
      phibar = sn * phibar;
      auto tau = sn * phi;

      //Update x and w.
      auto t1 = phi / rho;
      auto t2 = -1 * (theta / rho);
      std::vector<T> dk = w * (1 / rho);
      x = x + (w * t1);
      w = v + (w * t2);
      ddnorm = ddnorm + (vector_norm(dk) * vector_norm(dk));
      if(calc_var)var = var + (dk * dk);
      auto delta = sn2 * rho;
      auto gambar = -1 * cs2 * rho;
      auto rhs = phi - delta * z;
      auto zbar = rhs / gambar;
      xnorm = std::sqrt(xxnorm + zbar * zbar);
      auto gamma = std::sqrt(gambar * gambar + theta * theta);
      cs2 = gambar / gamma;
      sn2 = theta / gamma;
      z = rhs / gamma;
      xxnorm = xxnorm + z * z;

      //Test for convergence..
      acond = anorm * std::sqrt(ddnorm);
      auto res1 = phibar * phibar;
      res2 = res2 + psi * psi;
      rnorm = std::sqrt(res1 + res2);
      arnorm = alfa * std::fabs(tau);
      auto r1sq = rnorm * rnorm - dampsq * xxnorm;
      r1norm = std::sqrt(std::fabs(r1sq));
      if(r1sq < 0) r1norm = -1 * r1norm;
      r2norm = rnorm;

      // Now use these norms to estimate certain other quantities,
      // some of which will be small near a solution.
      test1 = rnorm / bnorm;
      test2 = arnorm / (anorm * rnorm + eps);
      auto test3 = 1 / (acond + eps);
      t1 = test1 / (1 + anorm * xnorm / bnorm);
      auto rtol = btol + atol * anorm * xnorm / bnorm;
      
      // The following tests guard against extremely small values of
      // atol, btol  or  ctol.  (The user may have set any or all of
      // the parameters  atol, btol, conlim  to 0.)
      if(itn >= iter_lim) istop = 7;
      if(1 + test3 <= 1) istop = 6;
      if(1 + test2 <= 1) istop = 5;
      if(1 + t1 <= 1) istop = 4;
      
      //Allow for tolerances set by the user.
      if(test3 <= ctol) istop = 3;
      if(test2 <= atol) istop = 2;
      if(test1 <= rtol) istop = 1;

      // See if it is time to print something.
      bool prnt = false;
      if(n <= 40) prnt = true;
      if(itn <= 10) prnt = true;
      if(itn >= iter_lim - 10) prnt = true;
      if(test3 <= 2 * ctol) prnt = true;
      if(test2 <= 10 * atol) prnt = true;
      if(test1 <= 10 * rtol) prnt = true;
      if(istop != 0) prnt = true;
      if(prnt){
        if(show){
          std::cout << " " <<std::endl;
          std::cout << itn << " ";
          std::cout << std::setw(5) << std::setprecision(5) << std::scientific << " ";
          std::cout << x[0] << " ";
          std::cout << std::setw(6) << std::setprecision(3) << std::scientific << " ";
          std::cout << r1norm << " ";
          std::cout << std::setw(6) << std::setprecision(3) << std::scientific << " ";
          std::cout << r2norm << " ";
          std::cout << std::setw(6) << std::setprecision(1) << std::scientific << " ";
          std::cout << test1 << " ";
          std::cout << std::setw(6) << std::setprecision(1) << std::scientific << " ";
          std::cout << test2 << " ";
          std::cout << std::setw(6) << std::setprecision(1) << std::scientific << " ";
          std::cout << anorm << " ";
          std::cout << std::setw(6) << std::setprecision(1) << std::scientific << " ";
          std::cout << acond << " ";
        }
      }
      if(istop != 0) break;
    }
    //End of iteration loop.
    //Print the stopping condition.
    if(show){
      std::cout << "\nLSQR finished" << std::endl;
      RLOG(INFO) << msg[istop] << std::endl;
      std::cout << "istop = " << istop;
      std::cout << std::setprecision(1) << std::scientific;
      std::cout << "   r1norm = " << r1norm;
      std::cout << std::setprecision(1) << std::scientific;
      std::cout << "   anorm = " << anorm;
      std::cout << std::setprecision(1) << std::scientific;
      std::cout << "   arnorm = " << arnorm << std::endl;
      std::cout << "itn = " << itn;
      std::cout << std::setprecision(1) << std::scientific;
      std::cout << "   r2norm = " << r2norm;
      std::cout << std::setprecision(1) << std::scientific;
      std::cout << "   acond = " << acond;
      std::cout << std::setprecision(1) << std::scientific;
      std::cout << "   xnorm = " << xnorm << std::endl;
    }
    sparse_lsqr_result<T> ret_val(x, itn, istop, r1norm, 
                                  r2norm, arnorm, anorm,
                                  acond, xnorm, var);
    return ret_val;
  }
}
#endif
