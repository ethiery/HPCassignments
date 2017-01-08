#ifndef BENCHTOOLS_H
#define BENCHTOOLS_H

#define FAIL(...) \
do {\
  printf(__VA_ARGS__); \
  return 0;\
} while(0)

#define SUCCESS(...) \
do {\
  printf(__VA_ARGS__);\
  return 1;\
} while(0)


/*
 * Timer object using the wall clock time.
 * Allows timing repetitions of a same operation
 */
typedef struct benchTimer benchTimer;

/*
 * Initialize a new timer to time the specfied number of repetitions
 * of an operation of the specified number of floating point operations
 */
benchTimer *timer_init(const long nbFlops, const long nbLoops);

/*
 * Starts the specified timer.
 * Cancels previous measure (whether it is finished or not)
 */
void timer_start(benchTimer *t);

/*
 * Stops the specified timer.
 * Must be use only if the latest operation performed on the timer was timer_start
 */
void timer_stop(benchTimer *t);

/*
 * Returns the duration in seconds of 1 repetition of the latest timed operation
 */
double timer_getDuration(benchTimer *t);

/*
 * Returns the speed of the latest timed operation in flops per second.
 */
double timer_getFlopPerS(benchTimer *t);

/*
 * Returns the number of instructions performed by the latest timed operation.
 */
long long timer_getInstructionCount(benchTimer *t);

/*
 * Returns the number of L1 cache miss during the latest timed operation.
 */
long long timer_getL1CacheMissCount(benchTimer *t);

/*
 * Returns the number of LL cache miss during the latest timed operation.
 */
long long timer_getLlCacheMissCount(benchTimer *t);


/*
 * Releases the resources related to the specified timer.
 */
void timer_release(benchTimer *t);



enum CACHE_TYPE {
  DATA_CACHE = 0x1,
  INSTRUCTION_CACHE = 0x2,
  UNIFIED_CACHE = 0x4,
  ANY_CACHE = 0x7
};

enum CACHE_LEVEL {
  L1 = 0x1,
  L2 = 0x2,
  L3 = 0x4,
  ANY_LEVEL = 0x7
};

/*
* Returns the total cache size of the specified caches
* (use CACHE_TYPE and CACHE_LEVEL, the differents values can be OR'd)
*/
long getCacheSize(const int cacheType, const int cacheLevel, const int verbose);


#endif
