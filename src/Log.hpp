#include <iostream>

#ifdef ENABLE_LOG
#define LOG(line) std::cerr << __FUNCTION__ << ": " <<  line << std::endl;
#else
#define LOG(line) ;
#endif
