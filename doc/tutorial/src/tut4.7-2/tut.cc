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
                          int c1, int c2, //conv2d channel
                          int k1, int k2, //conv2d kernel size
                          bool bn1, bool bn2, //batch normalization
                          int fc) // fully connected layer
{    
  // keras mnist tutorial cnn
  // https://github.com/keras-team/keras/blob/master/examples/mnist_cnn.py
  int conv1_out = 28 - 2 * static_cast<int>(std::floor(k1 * 0.5));
  int conv2_out = conv1_out - 2 * static_cast<int>(std::floor(k2 * 0.5));
  
  nn << convolutional_layer(28, 28, k1, 1, c1, padding::valid, true,
                            1, 1, 1, 1, backend_type)
     << relu_layer()
     << convolutional_layer(conv1_out, conv1_out, k2, c1, c2,
                            padding::valid, true, 1, 1, 1, 1, backend_type)
     << relu_layer();
  if(bn1){
    nn << batch_normalization_layer(conv2_out * conv2_out, c2);
  }
  nn << max_pooling_layer(conv2_out, conv2_out, c2, 2, backend_type)
     << fully_connected_layer(conv2_out * conv2_out * 0.25 * c2, fc, true,
                              backend_type) // (conv2_out / 2)^2
     << relu_layer();
  if(bn2){
    nn << batch_normalization_layer(fc, 1);
  }
  nn << fully_connected_layer(fc, 10, true, backend_type)
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
      /*
      std::cout << "Testing..." << std::endl;
      tiny_dnn::result res = nn->test(*test_images, *test_labels);
      std::cout << res.num_success << "/" << res.num_total 
                << " ("
                << static_cast<double>(res.num_success)
                   / static_cast<double>(res.num_total) * 100
                << "%)" << std::endl;
      */
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
                  double mu,
                  const int n_train_epochs,
                  const int n_minibatch,
                  std::string backend_type_string,
                  int c1, int c2, int k1, int k2, int fc,
                  bool bn1, bool bn2, bool shuffle) {
  std::vector<label_t> train_labels;
  std::vector<vec_t> train_images;

  parse_mnist_labels(data_dir_path + "/train-labels.idx1-ubyte", &train_labels);
  parse_mnist_images(data_dir_path + "/train-images.idx3-ubyte", &train_images,
                     -1.0, 1.0, 0, 0);
  std::cout << "mnist data initialized." << std::endl;

  dvector<tiny_dnn::label_t> dtrain_labels;
  dvector<tiny_dnn::vec_t> dtrain_images;
  if(shuffle){
    std::vector<label_t> shuffled_train_labels(train_images.size());
    std::vector<vec_t> shuffled_train_images(train_images.size());

    std::vector<int> random_list (train_images.size());
    for(int i = 0; i < train_images.size(); ++i) random_list[i] = i;  
    std::mt19937 seed(0);
    std::shuffle(random_list.begin(), random_list.end(), seed);

    for(int i = 0; i < train_images.size(); ++i) {
      shuffled_train_labels[i] = train_labels[random_list[i]];
      shuffled_train_images[i] = train_images[random_list[i]];
    }
    dtrain_labels = frovedis::make_dvector_scatter(shuffled_train_labels);
    dtrain_images = frovedis::make_dvector_scatter(shuffled_train_images);
  
  } else {
    dtrain_labels = frovedis::make_dvector_scatter(train_labels);
    dtrain_images = frovedis::make_dvector_scatter(train_images);
  }

  core::backend_t backend_type = parse_backend_name(backend_type_string);
  
  network<sequential> nn;
  construct_net(nn, backend_type, c1, c2, k1, k2, bn1, bn2, fc);

  dist_RMSprop optimizer;
  optimizer.alpha = learning_rate;
  optimizer.mu = mu;

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
            << " --backend_type ve" 
            << " --learning_rate 0.0001 (Learning Rate)"
            << " --mu 0.99 (RMSprop Parameter)"
            << " --epochs 12 (Epochs)"
            << " --local_minibatch_size 16 (Local Minibatch Size)"
            << " --c1 32 (Conv2d_1 channels)" 
            << " --c2 64 (Conv2d_2 channels)" 
            << " --k1 3 (Conv2d_1 kernel size)" 
            << " --k2 3 (Conv2d_2 kernel size)" 
            << " --fc 128 (Fully Connected Layer)" 
            << " --b1 false (BatchNormalization_1)" 
            << " --b2 false (BatchNormalization_2)" 
            << " --shuffle false (Training data shuffle)" 
            << std::endl;
}

int main(int argc, char **argv) {
  frovedis::use_frovedis use(argc, argv);

  std::string data_path           = "./mnist";
  double learning_rate            = 0.0001;
  double mu                       = 0.99;
  int epochs                      = 10;
  int minibatch_size              = 32;
  int c1                          = 32;
  int k1                          = 3;
  int c2                          = 64;
  int k2                          = 3;
  int fc                          = 128;
  bool bn1                        = false;
  bool bn2                        = false;
  bool shuffle                    = false;
 
#ifdef __ve__
  std::string backend_type_string = "ve";
#else
  std::string backend_type_string = "internal";
#endif

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
    } else if (argname == "--mu") {
      mu = atof(argv[count + 1]);
    } else if (argname == "--epochs") {
      epochs = atoi(argv[count + 1]);
    } else if (argname == "--local_minibatch_size") {
      minibatch_size = atoi(argv[count + 1]);
    } else if (argname == "--backend_type") {
      backend_type_string = std::string(argv[count + 1]);
    } else if (argname == "--data_path") {
      data_path = std::string(argv[count + 1]);
    } else if (argname == "--c1") {
      c1 = atoi(argv[count + 1]);
    } else if (argname == "--c2") {
      c2 = atoi(argv[count + 1]);
    } else if (argname == "--k1") {
      k1 = atoi(argv[count + 1]);
    } else if (argname == "--k2") {
      k2 = atoi(argv[count + 1]);
    } else if (argname == "--bn1") {
      bn1 = argv[count + 1];
    } else if (argname == "--bn2") {
      bn2 = argv[count + 1];
    } else if (argname == "--fc") {
      fc = atoi(argv[count + 1]);
    } else if (argname == "--shuffle") {
      shuffle = argv[count + 1];
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

  std::cout << "Running with the following parameters:" << std::endl
            << "Data path: " << data_path << std::endl
            << "Learning rate: " << learning_rate << std::endl
            << "Mu: " << mu << std::endl
            << "Number of epochs: " << epochs << std::endl
            << "Local Minibatch size: " << minibatch_size 
            << " (in total: " << minibatch_size * frovedis::get_nodesize()
            << ")" << std::endl
            << "Backend type: " << backend_type_string << std::endl
            << "Conv_1 channnel: " << c1 << std::endl
            << "Conv_1 kernel: " << k1 << std::endl
            << "Conv_2 channnel: " << c2 << std::endl
            << "Conv_2 kernel: " << k2 << std::endl
            << "Fully_Connected_Layer: " << fc << std::endl
            << "Batch_Norm_1: " << std::boolalpha << bn1 << std::endl
            << "Batch_Norm_2: " << std::boolalpha << bn2 << std::endl
            << "Training data shuffle: " << std::boolalpha << shuffle
            << std::endl
            << std::endl;
  try {
    train(data_path, learning_rate, mu, epochs, minibatch_size, 
          backend_type_string, c1, c2, k1, k2, fc, bn1, bn2, shuffle);
  } catch (std::exception& err) {
    std::cerr << "Exception: " << err.what() << std::endl;
  }

  return 0;
}
