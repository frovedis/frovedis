#ifndef _EXCEPTIONS_H_
#define _EXCEPTIONS_H_

#include <exception>
#include <stdexcept>
#include <iostream>
#include <string>

namespace frovedis {

enum errorID {
  DUMMY=0,
  MESSAGE,
  FILE_ERROR,
  USER_ERROR,
  INTERNAL_ERROR,
  MAX_ERROR,
  WARNING_MESSAGE,
};


void report_helper(bool info, bool to_throw, int id, std::string err_msg, 
                   std::string filename, int linenum);

#define REPORT_INFO(MSG)                                        \
  frovedis::report_helper(true, false, 0, MSG, __FILE__, __LINE__ )

#define REPORT_WARNING(ID,MSG)                                  \
  frovedis::report_helper(false, false, ID, MSG, __FILE__, __LINE__ )

#define REPORT_ERROR(ID,MSG)                                    \
  frovedis::report_helper(false, true, ID, MSG, __FILE__, __LINE__ )

#define REPORT_FATAL(ID,MSG)                                    \
  frovedis::report_helper(false, true, ID, MSG, __FILE__, __LINE__ )

#define ASSERT_PTR(PTR) if ((!(PTR)))                           \
    REPORT_FATAL(frovedis::INTERNAL_ERROR,"Expecting Non Null Value!");

#define checkAssumption(var) if (!(var))                        \
    REPORT_FATAL(frovedis::INTERNAL_ERROR,"Assumption Violated !!");

#define require(COND, MSG) if(!(COND)) REPORT_ERROR(USER_ERROR, MSG);

}
#endif
