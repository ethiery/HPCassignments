#include "Timer.h"
#include "utils.h"

#include <sys/time.h>
#include <malloc.h>
#include <mpi.h>

typedef struct Timer
{
    struct timeval start, stop;
} Timer;

Timer *start_timer()
{
    check_MPI();
    Timer *t = (Timer *) malloc(sizeof(Timer));
    MPI_Barrier(MPI_COMM_WORLD);
    gettimeofday(&(t->start), NULL);
    return t;
}

double stop_timer(Timer *t)
{
    MPI_Barrier(MPI_COMM_WORLD);
    gettimeofday(&(t->stop), NULL);
    double duration = t->stop.tv_sec - t->start.tv_sec + 0.000001 * (t->stop.tv_usec - t->start.tv_usec);
    free(t);
    return duration;
}
