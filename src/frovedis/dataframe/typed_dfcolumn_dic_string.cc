#include "dfcolumn.hpp"

namespace frovedis {

using namespace std;

void typed_dfcolumn<dic_string>::init(node_local<words>& ws,
                                      bool allocate_nulls) {
  auto cws = ws.map(make_compressed_words);
  init_compressed(cws, allocate_nulls);
}

void typed_dfcolumn<dic_string>::init_compressed
(node_local<compressed_words>& cws, bool allocate_nulls) {
  time_spent t(DEBUG);
  if(allocate_nulls) nulls = make_node_local_allocate<vector<size_t>>();
  auto nl_dict = cws.map(make_dict_from_compressed);
  t.show("init_compressed, create dict locally: ");
  auto local_dict = nl_dict.reduce(merge_dict);
  t.show("init_compressed, merge_dict: ");
  // broadcasting dic; heavy operation if dic is large
  // (when loading text, loaded separately to save memory...)
  auto bdic = broadcast(local_dict);
  t.show("init_compressed, broadcast dict: ");
  dic = make_shared<dict>(std::move(local_dict));
  val = bdic.map(+[](const dict& d, const compressed_words& c)
                 {return d.lookup(c);}, cws);
  t.show("init_compressed, lookup: ");
}

template <>
std::shared_ptr<dfcolumn>
create_null_column<dic_string>(const std::vector<size_t>& sizes) {
  auto nlsizes = make_node_local_scatter(sizes);
  auto val = make_node_local_allocate<std::vector<size_t>>();
  auto nulls = val.map(+[](std::vector<size_t>& val, size_t size) {
      val.resize(size);
      auto valp = val.data();
      auto max = std::numeric_limits<size_t>::max();
      std::vector<size_t> nulls(size);
      auto nullsp = nulls.data();
      for(size_t i = 0; i < size; i++) {
        valp[i] = max;
        nullsp[i] = i;
      }
      return nulls;
    }, nlsizes);
  auto ret = make_shared<typed_dfcolumn<dic_string>>();
  ret->val = std::move(val);
  ret->nulls = std::move(nulls);
  ret->dic = make_shared<dict>();
  ret->contain_nulls_check();
  return ret;
}

}
