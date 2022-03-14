#include "threads.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include "threads_win32.cpp"
#endif

