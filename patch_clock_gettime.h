
/* Hack support for clock_gettime into OS X */

#define CLOCK_MONOTONIC 1
typedef int clockid_t;
int clock_gettime(clockid_t clk_id, struct timespec *res) {
  struct timeval tv;
  int ret = 0;
  ret = gettimeofday(&tv, NULL);
  if (ret == 0) {
    res->tv_sec = tv.tv_sec;
    res->tv_nsec = tv.tv_usec * 1000;
  }
  return ret;
}
