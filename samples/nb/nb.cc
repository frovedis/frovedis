#include <frovedis.hpp>
#include <frovedis/matrix/crs_matrix.hpp>
#include <frovedis/ml/nb/naive_bayes.hpp>
#include <boost/program_options.hpp>

using namespace boost;
using namespace frovedis;
using namespace std;

template <class T, class MATRIX>
void do_nb_train(MATRIX& mat, 
                 dvector<T>& lbl,
                 const std::string& output,
                 const std::string& model_type,
                 double lambda, 
                 bool binary) {
  naive_bayes_model<T> model;
  time_spent train_t(DEBUG);
  train_t.lap_start(); 
  if(model_type == "multinomial") 
    model = multinomial_nb(mat, lbl, lambda);
  else if(model_type == "bernoulli") 
    model = bernoulli_nb(mat, lbl, lambda);
  else REPORT_ERROR(USER_ERROR, "Currently frovedis supports only multinomial and bernoulli naive bayes!\n");
  train_t.lap_stop(); 
  train_t.show_lap("total train time: ");

  binary ? model.savebinary(output) : model.save(output);
}

template <class T, class MATRIX>
void do_nb_predict(MATRIX& mat,
                   const std::string& modelfile,
                   const std::string& output,
                   bool is_prob,
                   bool binary) {
  time_spent pred_t(DEBUG);
  naive_bayes_model<T> model;
  binary ? model.loadbinary(modelfile) : model.load(modelfile);

  pred_t.lap_start();
  auto plbl =  is_prob? model.predict_probability(mat) : model.predict(mat);
  pred_t.lap_stop();
  pred_t.show_lap("total prediction time: ");

  binary ? make_dvector_scatter(plbl).savebinary(output)
         : make_dvector_scatter(plbl).saveline(output);
}


int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;
  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("predict,p", "predict mode")
    ("predict-probability,y", "probability-predict mode")
    ("input,i", value<string>(), "input matrix")
    ("label,l", value<string>(), "input label (for train)")
    ("model,m", value<string>(), "input model (for predict)")
    ("output,o", value<string>(), "output model or predict result")
    ("lambda,s", value<double>(), "smoothing parameter (0 for no smoothing) (default: 1.0) ")
    ("model_type,t", value<string>(), "target naive bayes model type (bernoulli or multinomial) (default: multinomial)")
    ("dtype", value<string>(), "target data type for input (float or double) (default: double)")
    ("mtype", value<string>(), "target matrix type for input (dense or sparse) (default: sparse)")
    ("verbose", "set loglevel to DEBUG")
    ("verbose2", "set loglevel to TRACE")
    ("binary,b", "use binary input/output");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  bool ispredict = false;
  bool predict_probability = false;
  string input, label, model, output;
  string dtype = "double";
  string mtype = "sparse";
  string model_type = "multinomial";
  double lambda = 1.0;
  bool binary = false;

  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("predict")){
    ispredict = true;
  }

  if(argmap.count("predict-probability")){
    ispredict = true;
    predict_probability = true;
  }

  if(argmap.count("input")){
    input = argmap["input"].as<string>();
  } else {
    cerr << "input is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(ispredict) {
    if(argmap.count("model")){
      model = argmap["model"].as<string>();
    } else {
      cerr << "model file is not specified" << endl;
      cerr << opt << endl;
      exit(1);
    }
  }
  else {
    if(argmap.count("label")){
      label = argmap["label"].as<string>();
    } else {
      cerr << "label file is not specified" << endl;
      cerr << opt << endl;
      exit(1);
    }
  }

  if(argmap.count("output")){
    output = argmap["output"].as<string>();
  } else {
    cerr << "output is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("lambda")){
    lambda = argmap["lambda"].as<double>();
  }

  if(argmap.count("dtype")){
    dtype = argmap["dtype"].as<string>();
  } 

  if(argmap.count("mtype")){
    mtype = argmap["mtype"].as<string>();
  } 

  if(argmap.count("model_type")){
    model_type = argmap["model_type"].as<string>();
  } 

  if(argmap.count("verbose")){
    set_loglevel(DEBUG);
  }

  if(argmap.count("verbose2")){
    set_loglevel(TRACE);
  }

  if(argmap.count("binary")){
    binary = true;
  }

  try {
    if(ispredict) {
      if(mtype == "dense") {
        if(dtype == "float") {
          auto mat = binary ? make_rowmajor_matrix_local_loadbinary<float>(input)
                            : make_rowmajor_matrix_local_load<float>(input);
          do_nb_predict<float>(mat, model, output, predict_probability, binary);
        }
        else if(dtype == "double") { 
          auto mat = binary ? make_rowmajor_matrix_local_loadbinary<double>(input)
                            : make_rowmajor_matrix_local_load<double>(input);
          do_nb_predict<double>(mat, model, output, predict_probability, binary);
        }
        else REPORT_ERROR(USER_ERROR, "supported dtypes are float or double!\n");
      }
      else if(mtype == "sparse") {
        if(dtype == "float") {
          auto mat = binary ? make_crs_matrix_local_loadbinary<float>(input)
                            : make_crs_matrix_local_load<float>(input);
          do_nb_predict<float>(mat, model, output, predict_probability, binary);
        }
        else if(dtype == "double") { 
          auto mat = binary ? make_crs_matrix_local_loadbinary<double>(input)
                            : make_crs_matrix_local_load<double>(input);
          do_nb_predict<double>(mat, model, output, predict_probability, binary);
        }
        else REPORT_ERROR(USER_ERROR, "supported dtypes are float or double!\n");
      }
      else REPORT_ERROR(USER_ERROR, "supported mtypes are dense or sparse!\n");
    }
    else {
      if(mtype == "dense") {
        if(dtype == "float") { 
          auto mat = binary ? make_rowmajor_matrix_loadbinary<float>(input)
                            : make_rowmajor_matrix_load<float>(input);
          auto lbl = binary ? make_dvector_loadbinary<float>(label)
                            : make_dvector_loadline<float>(label);
          do_nb_train(mat, lbl, output, model_type, lambda, binary);
        }
        else if(dtype == "double") {
          auto mat = binary ? make_rowmajor_matrix_loadbinary<double>(input)
                            : make_rowmajor_matrix_load<double>(input);
          auto lbl = binary ? make_dvector_loadbinary<double>(label)
                            : make_dvector_loadline<double>(label);
          do_nb_train(mat, lbl, output, model_type, lambda, binary);
        }
        else REPORT_ERROR(USER_ERROR, "supported dtypes are float or double!\n");
      }
      else if(mtype == "sparse") {
        if (dtype == "float") {
          auto mat = binary ? make_crs_matrix_loadbinary<float>(input)
                            : make_crs_matrix_load<float>(input);
          auto lbl = binary ? make_dvector_loadbinary<float>(label)
                            : make_dvector_loadline<float>(label);
          do_nb_train(mat, lbl, output, model_type, lambda, binary);
        }
        else if(dtype == "double") {
          auto mat = binary ? make_crs_matrix_loadbinary<double>(input)
                            : make_crs_matrix_load<double>(input);
          auto lbl = binary ? make_dvector_loadbinary<double>(label)
                            : make_dvector_loadline<double>(label);
          do_nb_train(mat, lbl, output, model_type, lambda, binary);
        }
        else REPORT_ERROR(USER_ERROR, "supported dtypes are float or double!\n");
      }
      else REPORT_ERROR(USER_ERROR, "supported mtypes are dense or sparse!\n");
    }
  }
  catch (std::exception& e) {
    std::cout << "exception caught: " << e.what() << std::endl;
  }
  return 0;
}
