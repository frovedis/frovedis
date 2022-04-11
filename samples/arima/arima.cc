#include <frovedis.hpp>
#include <frovedis/ml/tsa/arima.hpp>
#include <boost/program_options.hpp>

// usage:
// mpirun -np 1 arima_updated -i ./shampoo -p 1 -d 1 -q 1 -s 0 --auto-arima 0 --solver lapack -o out.txt --start-index 32 --stop-index 35 --step 1

using namespace boost;
using namespace frovedis;
using namespace std;

template <class T>
void do_train(const string& input, 
              size_t ar_lag,
              size_t diff_order,
              size_t ma_lag,
              size_t seasonality,
              bool auto_arima,
              const string& solver,
              size_t start_step,
              size_t stop_step,
              size_t step) {

  time_spent t(DEBUG);
  auto data = make_dvector_load<T>(input,"\n" ).moveto_node_local();
  t.show("load sample data: ");
  t.lap_start();
  auto ar_obj = Arima<T>(data, ar_lag, diff_order, ma_lag, seasonality, auto_arima, solver);  
  ar_obj.fit();  
  t.lap_stop();  
  t.show("train time: ");
  t.lap_start();
  auto x = ar_obj.predict(start_step, stop_step);
  t.lap_stop();
  t.show("time spent on prediction: ");
  std::cout << "Predicted values: " << x << std::endl;
  t.lap_start();
  x = ar_obj.forecast(step); //Param:size_t -> default 1 value forecast
  t.lap_stop();
  t.show("time spent on forecasting: ");
  std::cout << "Forecasted values: " << x << std::endl;  
    
}

int main(int argc, char* argv[]) {
  use_frovedis use(argc, argv);

  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("help,h", "print help")
    ("input,i", value<string>(), "input time series data")
    ("start-index,f", value<size_t>(), "start index for predicting future points")
    ("stop-index,e", value<size_t>(), "stop index for predicting future points")
    ("step", value<size_t>(), "steps used for forecasting future points")
    ("dtype", value<string>(), "target data type for input (double or float) (default: double)")
    ("auto-regressive-order,p", value<size_t>(), "auto regressive order")
    ("differencing-order,d", value<size_t>(), "differencing order")
    ("moving-average-order,q", value<size_t>(), "moving average order")
    ("seasonal,s", value<size_t>(), "seasonality")
    ("solver", value<string>(), "solver used to perform linear regression (default: lapack)")
    ("auto_arima,a", "use auto_arima or not (default: False)")
    ("verbose", "set loglevel to DEBUG")
    ("verbose2", "set loglevel to TRACE");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  string input;
  size_t ar_lag = 1;
  size_t diff_order = 1;
  size_t ma_lag = 1;
  size_t seasonality = 0;
  size_t start_step;
  size_t stop_step;
  size_t step = 1;
  bool auto_arima = false;
  string solver = "lapack";
  string dtype = "double";
  
  if(argmap.count("help")){
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("input")){
    input = argmap["input"].as<string>();
  } else {
    cerr << "input is not specified" << endl;
    cerr << opt << endl;
    exit(1);
  }

  if(argmap.count("start-index")){
    start_step = argmap["start-index"].as<size_t>();
  } else {
    cerr << "start step was not provided for prediction" << endl;
    cerr << opt << endl;
    exit(1);
  }
  if(argmap.count("stop-index")){
    stop_step = argmap["stop-index"].as<size_t>();
  } else {
    cerr << "stop step was not provided for prediction" << endl;
    cerr << opt << endl;
    exit(1);
  }

if(argmap.count("step")){
    step = argmap["step"].as<size_t>();
  } else {
    cerr << "step was not provided for forecasting" << endl;
    cerr << opt << endl;
    exit(1);
  }    
    
  if(argmap.count("auto-regressive-order")){
    ar_lag = argmap["auto-regressive-order"].as<size_t>();
  }

  if(argmap.count("differencing-order")){
    diff_order = argmap["differencing-order"].as<size_t>();
  }

  if(argmap.count("moving-average-order")){
    ma_lag = argmap["moving-average-order"].as<size_t>();
  }

  if(argmap.count("seasonal")){
    seasonality = argmap["seasonal"].as<size_t>();
  }    
    
  if(argmap.count("auto_arima")){
    auto_arima = true;
  }

  if(argmap.count("solver")){
    solver = argmap["solver"].as<string>();
  } 

  if(argmap.count("dtype")){
    dtype = argmap["dtype"].as<string>();
  } 
        
  if(argmap.count("verbose")){
    set_loglevel(DEBUG);
  }

  if(argmap.count("verbose2")){
    set_loglevel(TRACE);
  }
    
  if (dtype == "float") {
    do_train<float>(input, ar_lag, diff_order, ma_lag, seasonality, 
                    auto_arima, solver, start_step, stop_step, step);
  }
  else if (dtype == "double") {
    do_train<double>(input, ar_lag, diff_order, ma_lag, seasonality, 
                     auto_arima, solver, start_step, stop_step, step);
  }
  else REPORT_ERROR(USER_ERROR, "supported dtype is either float or double!\n");
}
