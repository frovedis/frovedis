#include "dfcolumn.hpp"
#include "../text/words.hpp"

#include "dfcolumn_helper.hpp"

namespace frovedis {

using namespace std;

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_format(const std::string& fmt,
                                          const std::string& type) {
  auto ws = as_words(6, fmt, false, "NULL");
  auto newnulls = nulls;
  if(type == "string") {
    auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(newnulls));
    return ret;
  } else if (type == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(ws), std::move(newnulls));
    return ret;
  } else if (type == "dic_string") {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(ws), std::move(newnulls));
    return ret;
  } else {
    throw std::runtime_error("unsupported type: " + type);
  }
}

template <>
std::shared_ptr<dfcolumn>
create_null_column<datetime>(const std::vector<size_t>& sizes) {
  auto nlsizes = make_node_local_scatter(sizes);
  auto val = make_node_local_allocate<std::vector<datetime_t>>();
  auto nulls = val.map(+[](std::vector<datetime_t>& val, size_t size) {
      val.resize(size);
      auto valp = val.data();
      auto max = std::numeric_limits<datetime_t>::max();
      std::vector<size_t> nulls(size);
      auto nullsp = nulls.data();
      for(size_t i = 0; i < size; i++) {
        valp[i] = max;
        nullsp[i] = i;
      }
      return nulls;
    }, nlsizes);
  auto ret = make_shared<typed_dfcolumn<datetime>>();
  ret->val = std::move(val);
  ret->nulls = std::move(nulls);
  ret->contain_nulls_check();
  return ret;
}

// datetime operations are defined in this file, not dfcolumn_arith_op.cc
struct datetime_extract_helper {
  datetime_extract_helper(){}
  datetime_extract_helper(datetime_type type) : type(type) {}
  std::vector<long> operator()(const std::vector<datetime_t>& d) {
    auto size = d.size();
    std::vector<long> ret(size);
    auto dp = d.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++) retp[i] = year_from_datetime(dp[i]);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) retp[i] = month_from_datetime(dp[i]);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) retp[i] = day_from_datetime(dp[i]);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) retp[i] = hour_from_datetime(dp[i]);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++) retp[i] = minute_from_datetime(dp[i]);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++) retp[i] = second_from_datetime(dp[i]);
    } else if(type == datetime_type::nanosecond) {
      for(size_t i = 0; i < size; i++)
        retp[i] = nanosecond_from_datetime(dp[i]);
    } else if(type == datetime_type::quarter) {
      for(size_t i = 0; i < size; i++) retp[i] = quarter_from_datetime(dp[i]);
    } else if(type == datetime_type::dayofweek) {
      for(size_t i = 0; i < size; i++) retp[i] = dayofweek_from_datetime(dp[i]);
    } else if(type == datetime_type::dayofyear) {
      for(size_t i = 0; i < size; i++) retp[i] = dayofyear_from_datetime(dp[i]);
    } else if(type == datetime_type::weekofyear) {
      for(size_t i = 0; i < size; i++)
        retp[i] = weekofyear_from_datetime(dp[i]);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  SERIALIZE(type)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_extract(datetime_type type) {
  auto ex = val.map(datetime_extract_helper(type));
  ex.mapv(reset_null<long>, nulls);
  return std::make_shared<typed_dfcolumn<long>>(std::move(ex), nulls);
}

struct datetime_diff_helper {
  datetime_diff_helper(){}
  datetime_diff_helper(datetime_type type) : type(type) {}
  std::vector<long> operator()(const std::vector<datetime_t>& d1,
                               const std::vector<datetime_t>& d2) {
    auto size = d1.size();
    std::vector<long> ret(size);
    auto d1p = d1.data();
    auto d2p = d2.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_diff_year(d1p[i], d2p[i]);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_month(d1p[i], d2p[i]);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_day(d1p[i], d2p[i]);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_hour(d1p[i], d2p[i]);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_minute(d1p[i], d2p[i]);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_second(d1p[i], d2p[i]);
    } else if(type == datetime_type::nanosecond) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_diff_nanosecond(d1p[i], d2p[i]);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  SERIALIZE(type)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_diff(const std::shared_ptr<dfcolumn>& right,
                                        datetime_type type) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("datetime_diff: type mismatch");
  auto newval = val.map(datetime_diff_helper(type), right2->val);
  if(contain_nulls || right2->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right2->nulls);
    newval.mapv(reset_null<long>, newnulls);
    return std::make_shared<typed_dfcolumn<long>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<long>();
    return std::make_shared<typed_dfcolumn<long>>(std::move(dvval));
  }
}

struct datetime_diff_im_helper {
  datetime_diff_im_helper(){}
  datetime_diff_im_helper(datetime_type type, datetime_t im)
    : type(type), im(im) {}
  std::vector<long> operator()(const std::vector<datetime_t>& d) {
    auto size = d.size();
    std::vector<long> ret(size);
    auto dp = d.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_diff_year(dp[i], im);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_month(dp[i], im);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_day(dp[i], im);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_hour(dp[i], im);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_minute(dp[i], im);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_second(dp[i], im);
    } else if(type == datetime_type::nanosecond) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_diff_nanosecond(dp[i], im);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  datetime_t im;
  SERIALIZE(type, im)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_diff_im(datetime_t right,
                                           datetime_type type) {
  auto newval = val.map(datetime_diff_im_helper(type, right));
  if(contain_nulls) {
    newval.mapv(reset_null<long>, nulls);
    return std::make_shared<typed_dfcolumn<long>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<long>();
    return std::make_shared<typed_dfcolumn<long>>(std::move(dvval));
  }
}

struct rdatetime_diff_im_helper {
  rdatetime_diff_im_helper(){}
  rdatetime_diff_im_helper(datetime_type type, datetime_t im)
    : type(type), im(im) {}
  std::vector<long> operator()(const std::vector<datetime_t>& d) {
    auto size = d.size();
    std::vector<long> ret(size);
    auto dp = d.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_diff_year(im, dp[i]);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_month(im, dp[i]);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_day(im, dp[i]);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_hour(im, dp[i]);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_minute(im, dp[i]);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_diff_second(im, dp[i]);
    } else if(type == datetime_type::nanosecond) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_diff_nanosecond(im, dp[i]);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  datetime_t im;
  SERIALIZE(type, im)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::rdatetime_diff_im(datetime_t right,
                                            datetime_type type) {
  auto newval = val.map(rdatetime_diff_im_helper(type, right));
  if(contain_nulls) {
    newval.mapv(reset_null<long>, nulls);
    return std::make_shared<typed_dfcolumn<long>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<long>();
    return std::make_shared<typed_dfcolumn<long>>(std::move(dvval));
  }
}


struct datetime_add_helper {
  datetime_add_helper(){}
  datetime_add_helper(datetime_type type) : type(type) {}
  std::vector<datetime_t> operator()(const std::vector<datetime_t>& d1,
                                     const std::vector<long>& d2) {
    auto size = d1.size();
    std::vector<datetime_t> ret(size);
    auto d1p = d1.data();
    auto d2p = d2.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_add_year(d1p[i], d2p[i]);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_month(d1p[i], d2p[i]);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_day(d1p[i], d2p[i]);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_hour(d1p[i], d2p[i]);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_minute(d1p[i], d2p[i]);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_second(d1p[i], d2p[i]);
    } else if(type == datetime_type::nanosecond) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_add_nanosecond(d1p[i], d2p[i]);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  SERIALIZE(type)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_add(const std::shared_ptr<dfcolumn>& right,
                                       datetime_type type) {
  auto right2 =
    std::dynamic_pointer_cast<typed_dfcolumn<long>>(right->type_cast("long"));
  auto newval = val.map(datetime_add_helper(type), right2->val);
  if(contain_nulls || right2->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right2->nulls);
    newval.mapv(reset_null<datetime_t>, newnulls);
    return std::make_shared<typed_dfcolumn<datetime>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<datetime_t>();
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(dvval));
  }
}

struct datetime_add_im_helper {
  datetime_add_im_helper(){}
  datetime_add_im_helper(datetime_type type, long val)
    : type(type), val(val) {}
  std::vector<datetime_t> operator()(const std::vector<datetime_t>& d) {
    auto size = d.size();
    std::vector<datetime_t> ret(size);
    auto dp = d.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_add_year(dp[i], val);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_month(dp[i], val);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_day(dp[i], val);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_hour(dp[i], val);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_minute(dp[i], val);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_add_second(dp[i], val);
    } else if(type == datetime_type::nanosecond) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_add_nanosecond(dp[i], val);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  long val;
  SERIALIZE(type, val)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_add_im(long right,
                                          datetime_type type) {
  auto newval = val.map(datetime_add_im_helper(type, right));
  if(contain_nulls) {
    newval.mapv(reset_null<datetime_t>, nulls);
    auto newnulls = nulls;
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(newval),
                                                      std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<datetime_t>();
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(dvval));
  }
}


struct datetime_sub_helper {
  datetime_sub_helper(){}
  datetime_sub_helper(datetime_type type) : type(type) {}
  std::vector<datetime_t> operator()(const std::vector<datetime_t>& d1,
                                     const std::vector<long>& d2) {
    auto size = d1.size();
    std::vector<datetime_t> ret(size);
    auto d1p = d1.data();
    auto d2p = d2.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_sub_year(d1p[i], d2p[i]);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_month(d1p[i], d2p[i]);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_day(d1p[i], d2p[i]);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_hour(d1p[i], d2p[i]);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_minute(d1p[i], d2p[i]);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_second(d1p[i], d2p[i]);
    } else if(type == datetime_type::nanosecond) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_sub_nanosecond(d1p[i], d2p[i]);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  SERIALIZE(type)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_sub(const std::shared_ptr<dfcolumn>& right,
                                       datetime_type type) {
  auto right2 =
    std::dynamic_pointer_cast<typed_dfcolumn<long>>(right->type_cast("long"));
  auto newval = val.map(datetime_sub_helper(type), right2->val);
  if(contain_nulls || right2->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right2->nulls);
    newval.mapv(reset_null<datetime_t>, newnulls);
    return std::make_shared<typed_dfcolumn<datetime>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<datetime_t>();
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(dvval));
  }
}

struct datetime_sub_im_helper {
  datetime_sub_im_helper(){}
  datetime_sub_im_helper(datetime_type type, long val)
    : type(type), val(val) {}
  std::vector<datetime_t> operator()(const std::vector<datetime_t>& d) {
    auto size = d.size();
    std::vector<datetime_t> ret(size);
    auto dp = d.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_sub_year(dp[i], val);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_month(dp[i], val);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_day(dp[i], val);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_hour(dp[i], val);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_minute(dp[i], val);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++) 
        retp[i] = datetime_sub_second(dp[i], val);
    } else if(type == datetime_type::nanosecond) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_sub_nanosecond(dp[i], val);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  long val;
  SERIALIZE(type, val)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_sub_im(long right,
                                          datetime_type type) {
  auto newval = val.map(datetime_sub_im_helper(type, right));
  if(contain_nulls) {
    newval.mapv(reset_null<datetime_t>, nulls);
    auto newnulls = nulls;
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(newval),
                                                      std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<datetime_t>();
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(dvval));
  }
}


struct datetime_truncate_helper {
  datetime_truncate_helper(){}
  datetime_truncate_helper(datetime_type type) : type(type) {}
  std::vector<datetime_t> operator()(const std::vector<datetime_t>& d) {
    auto size = d.size();
    std::vector<datetime_t> ret(size);
    auto dp = d.data();
    auto retp = ret.data();
    if(type == datetime_type::year) {
      for(size_t i = 0; i < size; i++) retp[i] = datetime_truncate_year(dp[i]);
    } else if(type == datetime_type::month) {
      for(size_t i = 0; i < size; i++) retp[i] = datetime_truncate_month(dp[i]);
    } else if(type == datetime_type::day) {
      for(size_t i = 0; i < size; i++) retp[i] = datetime_truncate_day(dp[i]);
    } else if(type == datetime_type::hour) {
      for(size_t i = 0; i < size; i++) retp[i] = datetime_truncate_hour(dp[i]);
    } else if(type == datetime_type::minute) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_truncate_minute(dp[i]);
    } else if(type == datetime_type::second) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_truncate_second(dp[i]);
    } else if(type == datetime_type::quarter) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_truncate_quarter(dp[i]);
    } else if(type == datetime_type::weekofyear) {
      for(size_t i = 0; i < size; i++)
        retp[i] = datetime_truncate_weekofyear(dp[i]);
    } else throw std::runtime_error("unsupported datetime_type");
    return ret;
  }
  
  datetime_type type;
  SERIALIZE(type)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_truncate(datetime_type type) {
  auto ex = val.map(datetime_truncate_helper(type)).
    mapv(+[](std::vector<datetime_t>& v, std::vector<size_t>& nulls) {
        auto vp = v.data();
        auto nullsp = nulls.data();
        auto nulls_size = nulls.size();
        auto max = std::numeric_limits<datetime_t>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < nulls_size; i++) {
          vp[nullsp[i]] = max;
        }
      }, nulls);
  auto newnulls = nulls;
  return std::make_shared<typed_dfcolumn<datetime>>(std::move(ex),
                                                    std::move(newnulls));
}


struct datetime_months_between_helper {
  datetime_months_between_helper(){}
  std::vector<double> operator()(const std::vector<datetime_t>& d1,
                                 const std::vector<datetime_t>& d2) {
    auto size = d1.size();
    std::vector<double> ret(size);
    auto d1p = d1.data();
    auto d2p = d2.data();
    auto retp = ret.data();
    for(size_t i = 0; i < size; i++)
      retp[i] = datetime_months_between(d1p[i], d2p[i]);
    return ret;
  }
  SERIALIZE_NONE
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_months_between
(const std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("datetime_months_between: type mismatch");
  auto newval = val.map(datetime_months_between_helper(), right2->val);
  if(contain_nulls || right2->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right2->nulls);
    newval.mapv(reset_null<double>, newnulls);
    return std::make_shared<typed_dfcolumn<double>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<double>();
    return std::make_shared<typed_dfcolumn<double>>(std::move(dvval));
  }
}


struct datetime_next_day_helper {
  datetime_next_day_helper(){}
  std::vector<datetime_t> operator()(const std::vector<datetime_t>& d1,
                                     const std::vector<int>& d2) {
    auto size = d1.size();
    std::vector<datetime_t> ret(size);
    auto d1p = d1.data();
    auto d2p = d2.data();
    auto retp = ret.data();
    for(size_t i = 0; i < size; i++)
      retp[i] = datetime_next_day(d1p[i], d2p[i]);
    return ret;
  }
  
  SERIALIZE_NONE
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_next_day
(const std::shared_ptr<dfcolumn>& right) {
  auto right2 =
    std::dynamic_pointer_cast<typed_dfcolumn<int>>(right->type_cast("int"));
  auto newval = val.map(datetime_next_day_helper(), right2->val);
  if(contain_nulls || right2->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right2->nulls);
    newval.mapv(reset_null<datetime_t>, newnulls);
    return std::make_shared<typed_dfcolumn<datetime>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<datetime_t>();
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(dvval));
  }
}


struct datetime_next_day_im_helper {
  datetime_next_day_im_helper(){}
  datetime_next_day_im_helper(int val) : val(val) {}
  std::vector<datetime_t> operator()(const std::vector<datetime_t>& d) {
    auto size = d.size();
    std::vector<datetime_t> ret(size);
    auto dp = d.data();
    auto retp = ret.data();
    for(size_t i = 0; i < size; i++)
      retp[i] = datetime_next_day(dp[i], val);
    return ret;
  }
  
  int val;
  SERIALIZE(val)
};

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::datetime_next_day_im(int right) {
  auto newval = val.map(datetime_next_day_im_helper(right));
  if(contain_nulls) {
    newval.mapv(reset_null<datetime_t>, nulls);
    auto newnulls = nulls;
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(newval),
                                                      std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<datetime_t>();
    return std::make_shared<typed_dfcolumn<datetime>>(std::move(dvval));
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::add(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<dfcolumn> ret = NULL;
  if (right_type == "long") ret = datetime_add(right, datetime_type_for_add_sub_op);
  else if (right_type == "int" || right_type == "unsigned int" || 
           right_type == "unsigned long") {
    std::shared_ptr<dfcolumn> tmp = 
      std::dynamic_pointer_cast<typed_dfcolumn<long>>(right->type_cast("long"));
    ret = datetime_add(tmp, datetime_type_for_add_sub_op);
  } else throw std::runtime_error(
      "datetime + " + right_type + ": invalid operation!");
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::add_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<dfcolumn> ret = NULL;
  if (right_type == "long") {
    auto tmp = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    ret = datetime_add_im(tmp->val, datetime_type_for_add_sub_op);
  } else if (right_type == "int" || right_type == "unsigned int" || 
             right_type == "unsigned long") {
    auto tmp = std::dynamic_pointer_cast<typed_dfscalar<long>>(
                                         right->type_cast("long"));
    ret = datetime_add_im(tmp->val, datetime_type_for_add_sub_op);
  } else throw std::runtime_error(
      "datetime + " + right_type + ": invalid operation!");
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::sub(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<dfcolumn> ret = NULL;
  if (right_type == "long") ret = datetime_sub(right, datetime_type_for_add_sub_op);
  else if (right_type == "datetime") ret = datetime_diff(right, datetime_type_for_add_sub_op);
  else if (right_type == "int" || right_type == "unsigned int" ||
           right_type == "unsigned long") {
    std::shared_ptr<dfcolumn> tmp =
      std::dynamic_pointer_cast<typed_dfcolumn<long>>(right->type_cast("long"));
    ret = datetime_sub(tmp, datetime_type_for_add_sub_op);
  } else if (right_type == "string" ||
             right_type == "raw_string" || right_type == "dic_string") {
    std::shared_ptr<dfcolumn> tmp =
      std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(
        right->type_cast("datetime")); // as per default format: %Y-%m-%d
    ret = datetime_diff(tmp, datetime_type_for_add_sub_op);
  } else throw std::runtime_error(
      "datetime - " + right_type + ": invalid operation!");
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::sub_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<dfcolumn> ret = NULL;
  if (right_type == "long") {
    auto tmp = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    ret = datetime_sub_im(tmp->val, datetime_type_for_add_sub_op);
  } else if (right_type == "datetime") {
    auto tmp = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(right);
    ret = datetime_diff_im(tmp->val, datetime_type_for_add_sub_op);
  } else if (right_type == "int" || right_type == "unsigned int" ||
             right_type == "unsigned long") {
    auto tmp = std::dynamic_pointer_cast<typed_dfscalar<long>>(
                                         right->type_cast("long"));
    ret = datetime_sub_im(tmp->val, datetime_type_for_add_sub_op);
  } else if (right_type == "string" ||
             right_type == "raw_string" || right_type == "dic_string") {
    auto tmp = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(
      right->type_cast("datetime")); // as per default format: %Y-%m-%d
    ret = datetime_diff_im(tmp->val, datetime_type_for_add_sub_op);
  } else throw std::runtime_error(
      "datetime - " + right_type + ": invalid operation!");
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::rsub_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<dfcolumn> ret = NULL;
  if (right_type == "long") {
    auto tmp = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    ret = rdatetime_diff_im(tmp->val, datetime_type_for_add_sub_op);
  } else if (right_type == "datetime") {
    auto tmp = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(right);
    ret = rdatetime_diff_im(tmp->val, datetime_type_for_add_sub_op);
  } else if (right_type == "string" ||
             right_type == "raw_string" || right_type == "dic_string") {
    auto tmp = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(
      right->type_cast("datetime")); // as per default format: %Y-%m-%d
    ret = rdatetime_diff_im(tmp->val, datetime_type_for_add_sub_op);
  } else throw std::runtime_error(
      right_type + " - datetime: invalid operation!");
  return ret;
}

}
