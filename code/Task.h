//*****************************************************************
// Task.h
//
//  Created on: Dec 9, 2011
//      Author: Christopher Wood
//              Vineeth Vijayakumaran
//
//  $Id: Task.h 59 2012-01-11 04:28:36Z w463-01u1a $
//*****************************************************************

#ifndef TASK_H_
#define TASK_H_

// Module includes
#include "Thread.h"
#include "Project1.h"
#include <pthread.h>

// Forward declaration due to bidirectional association
class ProxyScheduler;

/**
 * This class is responsible for managing the execution logic for a task
 * for the scheduler. It is assigned a compute/period time pair that is
 * then used to determine when the task should be scheduled to execute.
 */
class Task : public Thread
{
public:
	/**
	 * Default constructor for the task that stores its unique ID and
	 * compute/period time pair. All time values are assumed to be milliseconds.
	 *
	 * @param id - the task's unique ID
	 * @param computeTime - the tasks's compute time
	 * @param periodTime - the tasks's period time
	 */
	Task(int id, int computeTime, int periodTime);

	/**
	 * Default destructor for the task (no tear-down required).
	 */
	virtual ~Task();

	/**
	 * Retrieve this task's period time.
	 *
	 * @return period time
	 */
	unsigned int getPeriodTime();

	/**
	 * Retrieve this task's current deadline.
	 *
	 * @return current deadline
	 */
	unsigned int getDeadline();

	/**
	 * Retrieve this task's compute time value.
	 *
	 * @return compute time
	 */
	unsigned int getComputeTime();

	/**
	 * Retrieve this task's current compute time that has been
	 * completed thus far in its compute cycle.
	 *
	 * @return current compute time
	 */
	unsigned int getCurrentComputeTime();

	/**
	 * Period handler event that checks the current state of the
	 * task in its compute cycle and determines whether or not
	 * a deadline has been met or missed.
	 */
	void periodEvent();

	/**
	 * Retrieve the amount of time remaining in this
	 * task's current compute cycle.
	 *
	 * @return currently remaining compute time
	 */
	unsigned int remainingTime();

	/**
	 * Retrieve a pointer to this task's schedule parameter structure
	 * so its priority can be modified.
	 *
	 * @return schedule parameter structure pointer
	 */
	struct sched_param* getSchedParam();

	/**
	 * Release this task from its blocked state on the execution semaphore.
	 */
	void release();

	/**
	 * Pause (preempt) this task during its computation cycle and make it
	 * block on its execution semaphore.
	 */
	void pause();

	/**
	 * Stop the schedule test that is currently in progress.
	 */
	void stopTest();

	/**
	 * Stop the task thread altogether (performed at the end of the test).
	 */
	void stopTask();

	/**
	 * Log all of the schedule data collected at runtime to the kernel
	 * event stream and stdout.
	 */
	void logData();

	// Boolean primer for the timer
	volatile bool firstTimerRun;

protected:

	/**
	 * The start routine that is executed when the client calls start().
	 */
	void* startRoutine();

private:

	// Structures used by the underlying task timer thread
	struct itimerspec timerSpec;
	struct sigevent event;

	// The ID of this task's period timer
	timer_t timerID;

	// The task's execution semaphore (controlled by the proxy scheduler)
	sem_t sem;

	// Boolean flag indicating whether or not the task has been preempted.
	volatile bool preempted;

	// Boolean flag indicating whether or not a test is still being conducted.
	volatile bool testRunning;

	// Task compute and period time values.
	unsigned int computeTime;
	unsigned int periodTime;
	unsigned int currentComputeTime;
	unsigned int deadline;

	// The time quantum struct used to burn CPU cycles.
	struct timespec burnTime;

	// Counter indicating how many compute cycles this task has left to complete
	volatile int computeComplete;

	// Schedule data that is collected at runtime
	unsigned int deadlineEvents;
	unsigned int deadlinesMissed;
	unsigned long totalComputationTimeMissed;
	unsigned long totalComputationTime;
	unsigned int totalComputationCycles;
	unsigned int averageTaskPeriod;
	uint64_t realComputeTime;
	uint64_t computeTransitionTime;

	// The task's schedule parameter structure
	struct sched_param schedParam;

	// Constants used during the task lifetime
	static const int SEM_COUNT = 0; // binary semaphore initial value
	static const long TIME_QUANTUM = 100000; // .1ms time quantum (needs to be calibrated)
	static const long REAL_TIME_QUANTUM = 80000; // adjusted time quantum

	/**
	 * Configure the task's timer to fire an event at every periodic
	 * interval that corresponds to the task's period.
	 */
	void configureTimer();
};

#endif /* TASK_H_ */
