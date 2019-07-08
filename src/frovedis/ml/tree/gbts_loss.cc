#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "gbts_loss.hpp"

namespace frovedis {
namespace tree {

std::unordered_map<std::string, loss_type> get_loss_map() {
  std::unordered_map<std::string, loss_type> m;
  m.emplace("default", loss_type::Default);
  m.emplace("logloss", loss_type::LogLoss);
  m.emplace("log", loss_type::LogLoss);
  m.emplace("leastsquareserror", loss_type::LeastSquaresError);
  m.emplace("lse", loss_type::LeastSquaresError);
  m.emplace("leastabsoluteerror", loss_type::LeastAbsoluteError);
  m.emplace("lae", loss_type::LeastAbsoluteError);
  return m;
}

loss_type get_loss_type(const std::string& arg) {
  static auto imap = get_loss_map();
  std::string str;
  str.resize(arg.length());
  std::transform(arg.cbegin(), arg.cend(), str.begin(), tolower);

  try {
    return imap.at(str);
  } catch (const std::out_of_range&) {
    throw std::runtime_error(std::string("invalid loss type: ") + arg);
  }
}

} // end namespace tree
} // end namespace frovedis
