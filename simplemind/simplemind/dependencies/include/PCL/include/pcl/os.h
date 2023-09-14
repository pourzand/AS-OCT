//Checking of mutual exclusiveness
#if defined(WINDOWS) && defined(LINUX)
#undef LINUX
#endif

//Checking the current OS
#if defined(WINDOWS) || defined(LINUX)
#else

#if defined(_MSC_VER) || defined(WIN32)  || defined(_WIN32) || defined(__WIN32__) \
|| defined(WIN64)     || defined(_WIN64) || defined(__WIN64__)
#define WINDOWS
#elif defined(sun)       || defined(__sun)      || defined(linux)       || defined(__linux) \
 || defined(__linux__)   || defined(__CYGWIN__) || defined(BSD)         || defined(__FreeBSD__) \
 || defined(__OPENBSD__) || defined(__MACOSX__) || defined(__APPLE__)   || defined(sgi) \
 || defined(__sgi)
#define LINUX
#endif

#endif
