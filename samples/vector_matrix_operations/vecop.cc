#include <frovedis.hpp>
#include <frovedis/core/vector_operations.hpp>

using namespace frovedis;

template <class T>
void show(const std::string& msg, const T& val) {
  std::cout << msg << val << std::endl;
}

template <class T>
void show(const std::string& msg, 
          const std::vector<T>& vec,
          const int& limit = 10) {
  std::cout << msg; debug_print_vector(vec, limit);
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);
  try {
    if (argc < 2) 
      throw std::runtime_error("Use syntax: mpirun -np <nproc> ./vecop <input-file>");

    auto vec = make_dvector_loadline<double>(argv[1]).gather();

    time_spent t(INFO);
    auto vsum = vec + vec; // calls vector_add(vec, vec)
    t.show("vector_add: ");

    auto vsub = vec - vec; // calls vector_subtract(vec, vec)
    t.show("vector_subtract: ");

    auto vmul = vec * vec; // calls vector_multiply(vec, vec)
    t.show("vector_multiply: ");

    auto vdiv = vec / vec; // calls vector_divide(vec, vec)
    t.show("vector_divide: ");

    auto vneg = -vec; // calls vector_negative(vec)
    t.show("vector_negative: ");

    auto vsq = vector_square(vec);
    t.show("vector_square: ");

    auto vreducesum = vector_sum(vec);
    t.show("vector_sum: ");

    auto vmean = vector_mean(vec);
    t.show("vector_mean: ");

    auto vscalsum = vector_scaled_sum(vec, 2.0);
    t.show("vector_scaled_sum: ");

    auto vsqsum = vector_squared_sum(vec);
    t.show("vector_squared_sum: ");

    auto vnorm = vector_norm(vec);
    t.show("vector_norm: ");

    auto vdot = vector_dot(vec,  vec);
    t.show("vector_dot: ");

    auto vssd = vector_ssd(vec, vec * 2.0);
    t.show("vector_ssd: ");

    auto vssmd = vector_ssmd(vec);
    t.show("vector_ssmd: ");

    auto vaxpy = vector_axpy(vec, vec, 2.0);
    t.show("vector_axpy: ");

    auto vsort = vector_sort(vec);
    t.show("vector_sort: ");

    std::vector<size_t> uind, uinv, ucnt;
    auto vunq = vector_unique(vec, uind, uinv, ucnt); // for numpy-like unique
    //auto vunq = vector_unique(vec); // for only unique
    t.show("vector_unique: ");
    /*
    show("unique-sample: ", vunq);
    show("unique-count: ", ucnt);
    show("unique-index: ", uind);
    show("unique-inverse: ", uinv);
    */

    // only for vector<int>
    //auto vbincnt = vector_bincount<size_t>(vec);
    //t.show("vector_bincount: ");

    auto vlog = vector_log(vec);
    t.show("vector_log: ");

    auto vnlog = vector_negative_log(vec); // -vector_log(vec)
    t.show("vector_negative_log: ");

    auto vexp = vector_exp(vec);
    t.show("vector_exp: ");

    auto vlogsumexp = vector_logsumexp(vec);
    t.show("vector_logsumexp: ");

    auto vminid = vector_argmin(vec); 
    t.show("vector_argmin: ");

    auto vmin = vector_amin(vec); 
    t.show("vector_amin: ");

    auto vmaxid = vector_argmax(vec); 
    t.show("vector_argmax: ");

    auto vmax = vector_amax(vec); 
    t.show("vector_amax: ");

    auto vnnz = vector_count_nonzero(vec); 
    t.show("vector_count_nonzero: ");

    auto index = vector_find_nonzero(vec);
    t.show("vector_find_nonzero: ");

    auto vbin = vector_binarize(vec, 4.0);
    t.show("vector_binarize: ");

    auto vclip = vector_clip(vec, 2.0, 5.0);
    t.show("vector_clip: ");

    auto vtake = vector_take<double>(vec, vector_arrange<size_t>(vec.size()));
    t.show("vector_take: ");

    auto vcast = vector_astype<int>(vec); 
    t.show("vector_astype: ");

    auto vzeros = vector_zeros<int>(1000);
    t.show("vector_zeros: ");

    auto vones = vector_ones<int>(1000);
    t.show("vector_ones: ");

    auto vfull = vector_full<int>(1000, 5.0);
    t.show("vector_full: ");

    std::cout << "-------- results -------- \n";   
    show("data: ", vec);
    show("data + data: ", vsum);
    show("data - data: ", vsub);
    show("data * data: ", vmul);
    show("data / data: ", vdiv);
    show("-data: ", vneg);
    show("square(data): ", vsq);
    show("sum(data): ", vreducesum);
    show("mean(data): ", vmean);
    show("scaled-sum(data, 2): ", vscalsum);
    show("squared_sum(data): ", vsqsum);
    show("euclidean-norm(data): ", vnorm);
    show("(dot) data . data: ", vdot);
    show("sum-squared-diff(data, 2*data): ", vssd);
    show("sum-squared-mean-diff(data): ", vssmd);
    show("(axpy) 2 * data + data: ", vaxpy);
    show("sort(data): ", vsort);
    show("unique(data): ", vunq);
    //show("bincount(data): ", vbincnt);
    show("log(data): ", vlog);
    show("-log(data): ", vnlog);
    show("exp(data): ", vexp);
    show("logsumexp(data): ", vlogsumexp);
    show("argmin(data): ", vminid);
    show("amin(data): ", vmin);
    show("argmax(data): ", vmaxid);
    show("amax(data): ", vmax);
    show("count_nonzero(data): ", vnnz);
    show("vector_find nonzero index: ", index); 
    show("binarize(data, 4): ", vbin);
    show("clip(data, 2, 5): ", vclip);
    show("take(data[0 : N): ", vtake);
    show("astype<double>(data): ", vcast);
    show("zeros(1000): ", vzeros);
    show("ones(1000): ", vones);
    show("full(1000, 5): ", vfull);
  } 
  catch(std::exception& e) {
    std::cout <<  "exception caught: " << e.what() << std::endl;
  }
  return 0;
}
