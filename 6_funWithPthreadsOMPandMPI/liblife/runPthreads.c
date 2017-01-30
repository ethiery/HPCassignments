#include "Board.h"
#include "DistributedBoard.h"
#include "runPthreads.h"

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

/* Private */

typedef struct Task
{
    int nbIters;
    int blockSize;
    Board *b;
    int threadNo;
    int nbThreads;
    sem_t *semReadLeft;
    sem_t *semReadRight;
    sem_t *semWrittenLeft;
    sem_t *semWrittenRight;
} Task;

/**
 * Code executed by each pthread
 * @param args Pointer on the task to be done by the pthread
 * @return NULL
 */
static void *work(void *args)
{
    Task *task = (Task *) args;
    Board *b = task->b;

    int colsPerThread = b->Nx / task->nbThreads;
    int prevThread = (0 < task->threadNo) ? task->threadNo - 1 : task->nbThreads - 1;
    int nextThread = (task->threadNo < task->nbThreads - 1) ? task->threadNo + 1 : 0;
    int start = task->threadNo * colsPerThread;
    int end = (task->threadNo + 1) * colsPerThread;

    for (int iter = 0; iter < task->nbIters; iter++)
    {
        // Count neighbours of the border cells
        assert(sem_wait(task->semWrittenRight + prevThread) == 0);
        count_neighbours_col(b, start);
        assert(sem_post(task->semReadLeft + task->threadNo) == 0);

        assert(sem_wait(task->semWrittenLeft + nextThread) == 0);
        count_neighbours_col(b, end - 1);
        assert(sem_post(task->semReadRight + task->threadNo) == 0);

        count_neighbours_block_col(b, start + 1, end - 1, task->blockSize);

        // Update the state of the internal cells + the corresponding ghost cells
        update_state_block_col(b, start + 1, end -1);                       // includes top and bottom ghost cells

        // Update the state of the border cells + the corresponding ghost cells
        assert(sem_wait(task->semReadRight + prevThread) == 0);
        update_state_col(b, start);                                         // includes top and bottom ghost cells
        if (task->threadNo == 0)
            copy_state_col(b, b->Nx, 0);                                    // right ghost cells (including corners)
        assert(sem_post(task->semWrittenLeft + task->threadNo) == 0);

        assert(sem_wait(task->semReadLeft + nextThread) == 0);
        update_state_col(b, end - 1);
        if (task->threadNo == task->nbThreads - 1)
            copy_state_col(b, -1, b->Nx - 1);                               // left ghost cells (including corners)
        assert(sem_post(task->semWrittenRight + task->threadNo) == 0);
    }
    return NULL;
}

/* Public */

void run_pthreads(DistributedBoard *db, int nbIterations)
{
    int blockSize = (getenv("BLOCK_SIZE") == NULL) ? 4096 : atoi(getenv("BLOCK_SIZE"));
    int nbThreads = (getenv("NUM_THREADS") == NULL) ? 1: atoi(getenv("NUM_THREADS"));
    Board *b = db->b;
    assert(db->N % nbThreads == 0);

    pthread_t threads[nbThreads];
    Task tasks[nbThreads];
    sem_t semReadLeft[nbThreads];
    sem_t semReadRight[nbThreads];
    sem_t semWrittenLeft[nbThreads];
    sem_t semWrittenRight[nbThreads];

    for (int i = 0; i < nbThreads; i++)
    {
        tasks[i].nbIters = nbIterations;
        tasks[i].blockSize = blockSize;
        tasks[i].b = b;
        tasks[i].threadNo = i;
        tasks[i].nbThreads = nbThreads;
        sem_init(semReadLeft + i, 0, 0);
        sem_init(semReadRight + i, 0, 0);
        sem_init(semWrittenLeft + i, 0, 1);
        sem_init(semWrittenRight + i, 0, 1);
        tasks[i].semReadLeft = (sem_t *)semReadLeft;
        tasks[i].semReadRight = (sem_t *)semReadRight;
        tasks[i].semWrittenLeft = (sem_t *)semWrittenLeft;
        tasks[i].semWrittenRight = (sem_t *)semWrittenRight;
    }

    for (int i = 0; i < nbThreads; i++)
        assert(pthread_create(threads+i, NULL, work, (void *)(tasks + i)) == 0);

    for (int i = 0; i < nbThreads; i++)
        assert(pthread_join(threads[i], NULL) == 0);

    for (int i = 0; i < nbThreads; i++)
    {
        sem_destroy(semReadLeft + i);
        sem_destroy(semReadRight + i);
        sem_destroy(semWrittenLeft + i);
        sem_destroy(semWrittenRight + i);
    }
}