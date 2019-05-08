#include <iostream>
#include <array>
#include <frovedis.hpp>
#include <frovedis/ml/dnn/dnn.hpp>
#include <tiny_dnn/io/mnist_parser.h>
#include <tiny_dnn/io/display.h>

using namespace tiny_dnn;
using namespace frovedis;

static void construct_net(tiny_dnn::network<tiny_dnn::sequential> &nn,
                          tiny_dnn::core::backend_t backend_type,
                          int hidden_layer) {
    nn << fully_connected_layer(784, hidden_layer, true, backend_type)
     << relu_layer()
     << dropout_layer(hidden_layer, 0.2)
     << fully_connected_layer(hidden_layer, hidden_layer, true, backend_type)
     << relu_layer()
     << dropout_layer(hidden_layer, 0.2)
     << fully_connected_layer(hidden_layer, 10, true, backend_type)
     << softmax_layer();
}

static tiny_dnn::core::backend_t parse_backend_name(const std::string &name) {
  const std::array<const std::string, 7> names = {{
      "internal", "nnpack", "libdnn", "avx", "opencl", "cblas", "ve"
  }};
  for (size_t i = 0; i < names.size(); ++i) {
    if (name.compare(names[i]) == 0) {
      return static_cast<tiny_dnn::core::backend_t>(i);
    }
  }
  return tiny_dnn::core::default_engine();
}

template <class Network>
struct on_enumerate_epoch {
  on_enumerate_epoch(){}
  on_enumerate_epoch(tiny_dnn::timer* t,
                     tiny_dnn::progress_display* disp,
                     int n_train_epochs,
                     Network* nn,
                     std::vector<label_t>* test_labels,
                     std::vector<vec_t>* test_images) :
    t(t), disp(disp), n_train_epochs(n_train_epochs), nn(nn),
    test_labels(test_labels), test_images(test_images),
    total_time(0), epoch(1) {}
  void operator()() {
    if(get_selfid() == 0) {
      total_time += t->elapsed();
      std::cout << std::endl
                << "Epoch " << epoch << "/" << n_train_epochs << " finished. "
                << t->elapsed() << "s elapsed. "
                << "Total time is " << total_time << "s." << std::endl;
      ++epoch;
      std::cout << "Testing..." << std::endl;
      tiny_dnn::result res = nn->test(*test_images, *test_labels);
      std::cout << res.num_success << "/" << res.num_total 
                << " ("
                << static_cast<double>(res.num_success)
                   / static_cast<double>(res.num_total) * 100
                << "%)" << std::endl;
      disp->restart(disp->expected_count());
      t->restart();
    }
  };
  timer* t;
  progress_display* disp;
  int n_train_epochs;
  Network* nn;
  std::vector<label_t>* test_labels;
  std::vector<vec_t>* test_images;
  double total_time;
  int epoch;
  SERIALIZE_NONE
};

struct on_enumerate_minibatch {
  on_enumerate_minibatch(){}
  on_enumerate_minibatch(tiny_dnn::progress_display* disp,
                         int n_minibatch) :
    disp(disp), n_minibatch(n_minibatch) {}
  void operator()() {
    if(get_selfid() == 0) {
      (*disp) += n_minibatch * frovedis::get_nodesize();
    }
  };
  progress_display* disp;
  int n_minibatch;
  SERIALIZE_NONE
};

static void train(const std::string &data_dir_path,
                  double learning_rate,
                  const int n_train_epochs,
                  const int n_minibatch,
                  std::string backend_type_string,
                  int hidden_layer) {
  std::vector<label_t> train_labels;
  std::vector<vec_t> train_images;

  parse_mnist_labels(data_dir_path + "/train-labels.idx1-ubyte", &train_labels);
  parse_mnist_images(data_dir_path + "/train-images.idx3-ubyte", &train_images,
                     -1.0, 1.0, 0, 0);
  std::cout << "mnist data initialized." << std::endl;
  auto dtrain_labels = frovedis::make_dvector_scatter(train_labels);
  auto dtrain_images = frovedis::make_dvector_scatter(train_images);

  core::backend_t backend_type = parse_backend_name(backend_type_string);
  
  network<sequential> nn;
  dist_RMSprop optimizer;
  construct_net(nn, backend_type, hidden_layer);

  optimizer.alpha *= learning_rate;

  std::vector<label_t> test_labels;
  std::vector<vec_t> test_images;
  parse_mnist_labels(data_dir_path + "/t10k-labels.idx1-ubyte", 
                     &test_labels);
  parse_mnist_images(data_dir_path + "/t10k-images.idx3-ubyte", 
                     &test_images, -1.0, 1.0, 0, 0);

  std::cout << "start training." << std::endl;

  tiny_dnn::progress_display disp(train_images.size());
  timer t;

  on_enumerate_epoch<network<sequential>>
    enum_epoch(&t, &disp, n_train_epochs, &nn, &test_labels, &test_images);
  on_enumerate_minibatch enum_minibatch(&disp, n_minibatch);

  dnn_train<mse>(nn, optimizer, dtrain_images, dtrain_labels, n_minibatch,
                 n_train_epochs, enum_minibatch, enum_epoch);

  std::cout << "end training." << std::endl;

  nn.test(test_images, test_labels).print_detail(std::cout);
  nn.save("model");
}

static void usage(const char *argv0) {
  std::cout << "Usage: " << argv0 << " --data_path path_to_dataset_folder"
            << " --learning_rate 1"
            << " --epochs 10"
            << " --local_minibatch_size 4"
            << " --backend_type ve" 
            << " --hidden_layer 512" << std::endl;
}

int main(int argc, char **argv) {
  frovedis::use_frovedis use(argc, argv);
 
  double learning_rate         = 1;
  int epochs                   = 10;
  std::string data_path        = "./mnist";
  int minibatch_size           = 4;
#ifdef __ve__
  std::string backend_type_string = "ve";
#else
  std::string backend_type_string = "internal";
#endif
  int hidden_layer             = 512;

  if (argc == 2) {
    std::string argname(argv[1]);
    if (argname == "--help" || argname == "-h") {
      usage(argv[0]);
      return 0;
    }
  }
  for (int count = 1; count + 1 < argc; count += 2) {
    std::string argname(argv[count]);
    if (argname == "--learning_rate") {
      learning_rate = atof(argv[count + 1]);
    } else if (argname == "--epochs") {
      epochs = atoi(argv[count + 1]);
    } else if (argname == "--local_minibatch_size") {
      minibatch_size = atoi(argv[count + 1]);
    } else if (argname == "--backend_type") {
      backend_type_string = std::string(argv[count + 1]);
    } else if (argname == "--data_path") {
      data_path = std::string(argv[count + 1]);
    } else if (argname == "--hidden_layer") {
      hidden_layer = atoi(argv[count + 1]);
    } else {
      std::cerr << "Invalid parameter specified - \"" << argname << "\""
                << std::endl;
      usage(argv[0]);
      return -1;
    }
  }
  if (data_path == "") {
    std::cerr << "Data path not specified." << std::endl;
    usage(argv[0]);
    return -1;
  }
  if (learning_rate <= 0) {
    std::cerr
      << "Invalid learning rate. The learning rate must be greater than 0."
      << std::endl;
    return -1;
  }
  if (epochs <= 0) {
    std::cerr << "Invalid number of epochs. The number of epochs must be "
                 "greater than 0."
              << std::endl;
    return -1;
  }
  if (minibatch_size <= 0 || minibatch_size > 60000) {
    std::cerr
      << "Invalid minibatch size. The minibatch size must be greater than 0"
         " and less than dataset size (60000)."
      << std::endl;
    return -1;
  }
  if (hidden_layer <= 0) {
    std::cerr
      << "Invalid number of hidden layer. The number of hidden layer must be greater than 0."
      << std::endl;
    return -1;
  }
  std::cout << "Running with the following parameters:" << std::endl
            << "Data path: " << data_path << std::endl
            << "Learning rate: " << learning_rate << std::endl
            << "Local Minibatch size: " << minibatch_size
            << " (in total: " << minibatch_size * frovedis::get_nodesize()
            << ")" << std::endl
            << "Number of epochs: " << epochs << std::endl
            << "Backend type: " << backend_type_string << std::endl
            << "Number of hidden layer: " << hidden_layer <<std::endl
            << std::endl;
  try {
    train(data_path, learning_rate, epochs, minibatch_size, 
          backend_type_string, hidden_layer);
  } catch (std::exception& err) {
    std::cerr << "Exception: " << err.what() << std::endl;
  }

  return 0;
}
