#ifndef TIMER_H
#define TIMER_H

typedef struct Timer Timer;

/**
 * Initalize, start and return a new timer.
 * @return a new timer, just started
 */
Timer *start_timer();

/**
 * Stop a timer, return its measurement and free the associated resources.
 * @param t An initialize timer
 * @return The time elapsed since the timer was started, in seconds
 */
double stop_timer(Timer *t);

#endif //TIMER_H
