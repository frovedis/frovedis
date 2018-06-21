#include "exceptions.hpp"

namespace frovedis {

std::string get_id_tag(int id) {
  switch(id) {
  case DUMMY:           return "DUMMY";
  case MESSAGE:         return "MESSAGE";
  case FILE_ERROR:      return "FILE_ERROR";
  case USER_ERROR:      return "USER_ERROR";
  case INTERNAL_ERROR:  return "INTERNAL_ERROR";
  case MAX_ERROR:       return "MAX_ERROR";
  case WARNING_MESSAGE: return "WARNING_MESSAGE";
  default:              return "UNKNOWN";
  }
}

void report_helper(bool info, bool to_throw, int id, std::string err_msg, 
                   std::string filename, int linenum) {
  if(!info) {
    std::cerr << "\n[" << get_id_tag(id) << "]: "
              << "file: " << filename
              << ", line: " << linenum
              << std::endl;
  } else {
    std::cerr << "[INFO]: ";
  }
  std::cerr << err_msg << std::endl;
  if(to_throw) throw std::runtime_error(err_msg);
}

}
