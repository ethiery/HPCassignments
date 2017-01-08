#include "benchtools.h"

#define _GNU_SOURCE
#include <asm/unistd.h>
#include <assert.h>
#include <linux/perf_event.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>



struct benchTimer
{
  long nbFlops;
  long nbLoops;
  struct timeval start;
  struct timeval stop;
  int instrCountFd;
  int l1CacheMissFd;
  int llCacheMissFd;
  long long instrCount;
  long long l1CacheMissCount;
  long long llCacheMissCount;
};

long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
  int cpu, int group_fd, unsigned long flags)
{
  return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

benchTimer *timer_init(const long nbFlops, const long nbLoops)
{
  benchTimer *t = (benchTimer *) malloc(sizeof(benchTimer));
  t->nbFlops = nbFlops;
  t->nbLoops = nbLoops;
  t->instrCount = -1;
  t->l1CacheMissCount = -1;

  struct perf_event_attr pe;
  memset(&pe, 0, sizeof(struct perf_event_attr));
  pe.type = PERF_TYPE_HARDWARE;
  pe.size = sizeof(struct perf_event_attr);
  pe.config = PERF_COUNT_HW_INSTRUCTIONS;
  pe.disabled = 1;
  pe.exclude_kernel = 1;
  pe.exclude_hv = 1;
  t->instrCountFd = perf_event_open(&pe, 0, -1, -1, 0);

  memset(&pe, 0, sizeof(struct perf_event_attr));
  pe.type = PERF_TYPE_HW_CACHE;
  pe.size = sizeof(struct perf_event_attr);
  pe.config = PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
  pe.disabled = 1;
  pe.exclude_kernel = 1;
  pe.exclude_hv = 1;
  t->l1CacheMissFd = perf_event_open(&pe, 0, -1, -1, 0);

  pe.config = PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
  t->llCacheMissFd = perf_event_open(&pe, 0, -1, -1, 0);


  return t;
}

void timer_start(benchTimer *t)
{
  if (t->instrCountFd != -1) {
    ioctl(t->instrCountFd, PERF_EVENT_IOC_RESET, 0);
    ioctl(t->instrCountFd, PERF_EVENT_IOC_ENABLE, 0);
  }
  if (t->l1CacheMissFd != -1) {
    ioctl(t->l1CacheMissFd, PERF_EVENT_IOC_RESET, 0);
    ioctl(t->l1CacheMissFd, PERF_EVENT_IOC_ENABLE, 0);
  }
  if (t->llCacheMissFd != -1) {
    ioctl(t->llCacheMissFd, PERF_EVENT_IOC_RESET, 0);
    ioctl(t->llCacheMissFd, PERF_EVENT_IOC_ENABLE, 0);
  }
  gettimeofday(&t->start, NULL);
}

void timer_stop(benchTimer *t)
{
  gettimeofday(&t->stop, NULL);
  if (t->instrCountFd != -1)
    ioctl(t->instrCountFd, PERF_EVENT_IOC_DISABLE, 0);
  if (t->l1CacheMissFd != -1)
    ioctl(t->l1CacheMissFd, PERF_EVENT_IOC_DISABLE, 0);
  if (t->llCacheMissFd != -1)
    ioctl(t->llCacheMissFd, PERF_EVENT_IOC_DISABLE, 0);
  if (t->instrCountFd != -1)
    assert(read(t->instrCountFd, &(t->instrCount), 8) == 8);
  if (t->l1CacheMissFd != -1)
    assert(read(t->l1CacheMissFd, &(t->l1CacheMissCount), 8) == 8);
  if (t->llCacheMissFd != -1)
    assert(read(t->llCacheMissFd, &(t->llCacheMissCount), 8) == 8);
}

double timer_getDuration(benchTimer *t)
{
  return (t->stop.tv_sec - t->start.tv_sec +
    0.000001 * (t->stop.tv_usec - t->start.tv_usec)) / t->nbLoops;
}

double timer_getFlopPerS(benchTimer *t)
{
  return t->nbFlops / timer_getDuration(t);
}

long long timer_getInstructionCount(benchTimer *t)
{
  return t->instrCount;
}

long long timer_getL1CacheMissCount(benchTimer *t)
{
  return t->l1CacheMissCount;
}

long long timer_getLlCacheMissCount(benchTimer *t)
{
  return t->llCacheMissCount;
}

void timer_release(benchTimer *t)
{
  close(t->instrCountFd);
  close(t->l1CacheMissFd);
  free(t);
}


char cacheTypes[][20] = {
  "", "Data cache", "Instruction cache", "Unified cache", "Unknown cache"
};

/*
* Returns the total cache size of the specified caches
* (use CACHE_TYPE and CACHE_LEVEL, the differents values can be OR'd)
*
*/
long getCacheSize(const int cacheType, const int cacheLevel, const int verbose)
{
  int totalSize = 0;
  int type, level;
  int nbSets, coherencyLineSize, physicalLinePartitions, waysOfAssociativity;
  long cacheSize;
  unsigned int eax, ebx, ecx, edx;

  if (verbose)
  printf("Detecting CPU caches for accurate benchmarking\n");

  // with EAX=4 and ECX=i the CPUID instruction returns cache info of cache i
  for (int i = 0; i < 32; i++) {
    eax = 4;
    ecx = i; // cache id
    __asm__ (
      "cpuid" // call i386 cpuid instruction
      : "+a" (eax) // "+a" means register EAX is an input and an output
      , "=b" (ebx) // "=b" means register EBX is an output
      , "+c" (ecx) // "+c" means register ECX is an input and an output
      , "=d" (edx) // "=d" means register EDX is an output
    );

    // results' content is explained in https://software.intel.com/sites/default/files/m/9/2/3/41604 page 33
    // bits 1-4 of resulting EAX are the cache_type
    type = eax & 0x1F;
    if (type == 0)
    break;
    level = (eax >> 5) & 0x7;
    nbSets = ecx + 1;
    coherencyLineSize = (ebx & 0xFFF) + 1;
    physicalLinePartitions = ((ebx >> 12) & 0x3FF) + 1;
    waysOfAssociativity = ((ebx >> 22) & 0x3FF) + 1;
    cacheSize = nbSets * waysOfAssociativity * physicalLinePartitions * coherencyLineSize;

    if (verbose)
    printf("Level %d %s - %ld kB\n", level, cacheTypes[type], cacheSize/1024);

    if (!((cacheType >> (type-1)) & 0x1) || !((cacheLevel >> (level-1)) & 0x1))
    continue;

    totalSize += cacheSize;
  }
  return totalSize;
}
