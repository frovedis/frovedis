#ifndef _MACRO_
#define _MACRO_

#define STR(X) std::to_string(X)
#define require(COND, MSG) if(!(COND)) REPORT_ERROR(USER_ERROR, MSG);

#endif
