#include <frovedis.hpp>
#include <frovedis/dataframe.hpp>
#include <frovedis/dataframe/make_dftable_dvector.hpp>
#include <frovedis/dataframe/dftable_to_dvector.hpp>

int main(int argc, char* argv[]){
  frovedis::use_frovedis use(argc, argv);

  using namespace std;
  vector<tuple<int,double,string>>
    v = {make_tuple(1,10.0,string("a")),make_tuple(2,20.0,string("b")),
         make_tuple(3,30.0,string("c")),make_tuple(4,40.0,string("d")),
         make_tuple(5,50.0,string("e")),make_tuple(6,60.0,string("f")),
         make_tuple(7,70.0,string("g")),make_tuple(8,80.0,string("h"))};
  auto d = frovedis::make_dvector_scatter(v);
  auto t = frovedis::make_dftable_dvector(d,{"c1","c2","c3"});
  t.show();
  auto d2 = frovedis::dftable_to_dvector<int,double,string>(t);
  for(auto i: d2.gather()) {
    cout << get<0>(i) << ", " << get<1>(i) << ", " << get<2>(i) << endl;
  }
}
