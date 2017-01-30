#include "utils.h"

#include <mpi.h>
#include <stdlib.h>
#include <stdarg.h>

static void MPI_finalizer()
{
    MPI_Finalize();
}

void check_MPI()
{
    int initialized;
    MPI_Initialized(&initialized);

    if (!initialized)
    {
        MPI_Init(NULL, NULL);
        atexit(MPI_finalizer);
    }
}

void fprintf_once(FILE *stream, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    int initialized;
    MPI_Initialized(&initialized);
    if (initialized)
    {
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        if (rank == 0)
            vfprintf(stream, format, args);
    } else
    {
        vfprintf(stream, format, args);
    }

    va_end(args);
}