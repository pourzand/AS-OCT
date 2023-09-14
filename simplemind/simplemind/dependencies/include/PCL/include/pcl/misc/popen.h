#include <pcl/os.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(WINDOWS)

#include <io.h>
#include <fcntl.h>
#ifndef PIPE_BUFFER_SIZE
#define PIPE_BUFFER_SIZE 512
#endif
#define pcl_popen _popen
#define pcl_pclose _pclose

#else

#define pcl_popen popen
#define pcl_pclose pclose

#endif