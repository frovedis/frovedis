#include <frovedis/ml/tsa/arima.hpp>
using namespace frovedis;

//usage:
//  mpirun -np 2  arima ~/swf28mar/trunk/samples/arima/shampoo 1 1 1 0 0

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  std::string data = argv[1];     //sample data
  int ar_lag = atoi(argv[2]);     //auto regressive order
  int diff_lag = atoi(argv[3]);   //differencing order
  int ma_lag = atoi(argv[4]);     //moving average order
  int seasonality = atoi(argv[5]);//seasonal differencing order
  bool auto_arima = atoi(argv[6]);//auto arima flag
  //uncomment below line to load from a csv 
  auto n = make_dvector_load<double>(data,"\n" ).moveto_node_local();
  
  auto ar_obj = Arima<double>(n, ar_lag, diff_lag, ma_lag, seasonality, auto_arima);  
  ar_obj.fit();
  std::cout << "Order: " << ar_obj.ar_lag << ar_obj.diff_order << ar_obj.ma_lag << std::endl;
  auto ft = ar_obj.fitted_values.gather();
  std::cout << "fitted values: " << std::endl;
  for(auto i : ft){
    for(auto j : i) {
      std::cout << j << std::endl;
    }
  }
  time_spent t;
  auto x = ar_obj.predict(ar_obj.sample_size, ar_obj.sample_size);
  t.show("time spent");
  std::cout << "Predicted values: " << x << std::endl;
  x = ar_obj.forecast(); //Param:size_t -> default 1 value forcast
  std::cout << "Forecasted values: " << x << std::endl;
}
