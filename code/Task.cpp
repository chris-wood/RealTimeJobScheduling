//*****************************************************************
// Task.cpp
//
//  Created on: Dec 9, 2011
//      Author: Christopher Wood
//              Vineeth Vijayakumaran
//
//  $Id: Task.cpp 59 2012-01-11 04:28:36Z w463-01u1a $
//*****************************************************************

#include "Task.h"
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include "ProxyScheduler.h"

/**
 * External (but friendly) function that is used as a callback
 * for each task's period timer. The single parameter stores
 * a pointer to the respective Task object whose period has
 * just expired.
 *
 * @param arg - sigval struct containing a pointer to a Task object
 */
void taskTimerExpired(sigval arg);

/**
 * Default constructor for the task that stores its unique ID and
 * compute/period time pair. All time values are assumed to be milliseconds.
 *
 * @param id - the task's unique ID
 * @param computeTime - the tasks's compute time
 * @param periodTime - the tasks's period time
 */
Task::Task(int id, int computeTime, int periodTime)
{
	int result;

	// Attempt to initialize the execution semaphore.
	result = sem_init(&sem, 0, SEM_COUNT);
	if (result != 0)
	{
		cerr << "Error initializing execution semaphore for task " <<
				id << endl;
		uid = -1;
	}
	else
	{
		// Initialize the rest of the task's variables.
		this->uid = id;
		this->computeTime = computeTime;
		this->currentComputeTime = 0;
		this->periodTime = periodTime;
		this->deadline = periodTime;
		this->deadlinesMissed = 0;
		this->deadlineEvents = 0;
		this->totalComputationTimeMissed = 0;
		this->totalComputationTime = 0;
		this->totalComputationCycles = 0;
		this->averageTaskPeriod = 0;
		this->realComputeTime = 0;
		this->computeTransitionTime = 0;

		// Initialize the burn time quantum.
		this->burnTime.tv_nsec = REAL_TIME_QUANTUM;

		// Configure the period timer.
		configureTimer();
	}
}

/**
 * Default destructor for the task (no tear-down required).
 */
Task::~Task()
{
	// Empty...
}

/**
 * The start routine that is executed when the client calls start().
 */
void* Task::startRoutine()
{
	int result;
	uint64_t preStartCycleTime = 0;
	uint64_t preEndCycleTime = 0;
	uint64_t startCycleTime = 0;
	uint64_t endCycleTime = 0;
	uint64_t postEndCycleTime = 0;

	// Set up some flags used to control task execution
	bool firstRun = true;
	computeComplete = 1;
	testRunning = true;
	preempted = false;
	firstTimerRun = true;

	// Reset the current compute time for this test
	currentComputeTime = 0;

	// Wait until we are released (a test begins)
	sem_wait(&sem);

	// Create the timer and kick it off
	timer_create(CLOCK_REALTIME, &event, &timerID);
	timer_settime(timerID, 0, &timerSpec, NULL);
	sem_post(&proxySem);

	// Intermittent wait that is used to make sure every task's timer is started
	sem_wait(&sem);

	// Jump into the test loop where the task will iteratively execute
	// compute cycles when it is scheduled
	while (testRunning)
	{
		// Block on execution semaphore (only after the first cycle)
		if (!firstRun)
		{
			sem_wait(&sem);
			preempted = false;
		}
		else
		{
			firstRun = false;
		}

		// Log pre-compute cycles
		preEndCycleTime = 0;
		preStartCycleTime = ClockCycles();

		// Log the schedule event
		TraceEvent(_NTO_TRACE_INSERTSUSEREVENT, EVENT_SCHEDULE, EVENT_SCHEDULE, uid);
		scheduleList.push_back(uid);

		// Begin/resume the compute cycle.
		while (currentComputeTime < (computeTime * NS_PER_MS))
		{
			if (!preempted)
			{
				// Record start compute time jitter
				if (preEndCycleTime == 0)
				{
					preEndCycleTime = ClockCycles();
				}

				// Burn and churn.
				startCycleTime = ClockCycles();
				result = nanospin(&burnTime);
				endCycleTime = ClockCycles();

				// Preemption means that nanospin took longer than expected, so handle that case.
				if (!preempted)
				{
					realComputeTime += (endCycleTime - startCycleTime);
				}
				else
				{
					realComputeTime += TIME_QUANTUM; // unavoidable
				}

				// Check the nanospin return, just to be safe.
				if (result == 0)
				{
					// We're okay - bump up the compute time.
					currentComputeTime += TIME_QUANTUM;
					totalComputationTime += TIME_QUANTUM;
				}
				else
				{
					cout << "Error: Task " << uid << " nanospin() returned: " << result << endl;
				}
			}
			else
			{
				break; // Drop back to the execution semaphore.
			}
		}

		// Check for deadline being hit
		if (currentComputeTime >= (computeTime * NS_PER_MS))
		{
			// Update the new deadline and reset the compute time
			currentComputeTime = 0;
			computeComplete--;
			totalComputationCycles++;
		}

		// Log post compute time cycles
		postEndCycleTime = ClockCycles();
		computeTransitionTime += ((postEndCycleTime - endCycleTime) +
				(preEndCycleTime - preStartCycleTime));

		// Give up the CPU for other tasks to execute
		sched_yield();
	}

	// Suicide
	kill();

	// Delete the timer for the period - no longer needed
	timer_delete(timerID);
}

/**
 * Configure the task's timer to fire an event at every periodic
 * interval that corresponds to the task's period.
 */
void Task::configureTimer()
{
	// Initialize the timer thread to invoke a function when it expires
	SIGEV_THREAD_INIT(&event, &taskTimerExpired, this, 0);
	
	// Configure the timer parameters (period signals only)
	timerSpec.it_value.tv_sec = 0;
	timerSpec.it_value.tv_nsec = 1;
	timerSpec.it_interval.tv_sec = 0;
	timerSpec.it_interval.tv_nsec = periodTime * NS_PER_MS;
}

/**
 * Retrieve this task's period time.
 *
 * @return period time
 */
unsigned int Task::getPeriodTime()
{
	return periodTime;
}

/**
 * Retrieve this task's compute time value.
 *
 * @return compute time
 */
unsigned int Task::getComputeTime()
{
	return computeTime;
}

/**
 * Retrieve this task's current deadline.
 *
 * @return current deadline
 */
unsigned int Task::getDeadline()
{
	return deadline;
}

/**
 * Retrieve this task's current compute time that has been
 * completed thus far in its compute cycle.
 *
 * @return current compute time
 */
unsigned int Task::getCurrentComputeTime()
{
	return currentComputeTime;
}

/**
 * Period handler event that checks the current state of the
 * task in its compute cycle and determines whether or not
 * a deadline has been met or missed.
 */
void Task::periodEvent()
{
	if (testRunning)
	{
		deadlineEvents++;

		// Check for missed deadline
		if (computeComplete > 0)
		{
			deadlinesMissed++;
			totalComputationTimeMissed += ((computeTime * NS_PER_MS) - currentComputeTime);

			// Log this event
			TraceEvent(_NTO_TRACE_INSERTSUSEREVENT, EVENT_MISSED_DEADLINE, EVENT_MISSED_DEADLINE, uid);
			cout << "MISSED " << uid << endl;
		}

		// Reset the deadline information
		deadline += periodTime;
		computeComplete++; // add on another compute cycle

		// Let the scheduler know our period has expired
		sem_post(&proxySem);
		sched_yield();
	}
}

/**
 * Retrieve the amount of time remaining in this
 * task's current compute cycle.
 *
 * @return currently remaining compute time
 */
unsigned int Task::remainingTime()
{
	return ((computeTime * NS_PER_MS) - currentComputeTime);
}

/**
 * Retrieve a pointer to this task's schedule parameter structure
 * so its priority can be modified.
 *
 * @return schedule parameter structure pointer
 */
struct sched_param* Task::getSchedParam()
{
	return &schedParam;
}

/**
 * Release this task from its blocked state on the execution semaphore.
 */
void Task::release()
{
	// Only release from the semaphore if we are still in a compute cycle.
	if (computeComplete != 0)
	{
		sem_post(&sem);
	}
}

/**
 * Pause (preempt) this task during its computation cycle and make it
 * block on its execution semaphore.
 */
void Task::pause()
{
	preempted = true;
}

/**
 * Stop the schedule test that is currently in progress.
 */
void Task::stopTest()
{
	testRunning = false;
	sem_post(&sem);
}

/**
 * Stop the task thread altogether (performed at the end of the test).
 */
void Task::stopTask()
{
	alive = false;
	sem_post(&sem);
}

/**
 * Log all of the schedule data collected at runtime to the kernel
 * event stream and stdout.
 */
void Task::logData()
{
	uint64_t cps;
	float realTime = 0;
	float realTransitionTime = 0;
	char data[256];

	// Determine the clock rate
	cps = SYSPAGE_ENTRY(qtime)->cycles_per_sec;

	// Calculate the real compute time period
	realTime = ((float)((float)realComputeTime / (float)cps) * 1000);
	realTransitionTime = (float)((float)computeTransitionTime / (float)cps) * 1000;

	// Log the data
	sprintf(data, "TDATA %d,%d,%d,%d,%d,%d,%f,%f,%f", uid, deadlineEvents,
			deadlinesMissed, totalComputationTimeMissed, totalComputationTime / NS_PER_MS,
			totalComputationCycles, realTransitionTime / realTime, realTime,
			((totalComputationTime / NS_PER_MS) - realTime) / (totalComputationTime / NS_PER_MS));
	TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, EVENT_PROXY_DATA, data);
	cout << data << endl;
}

/**
 * External (but friendly) function that is used as a callback
 * for each task's period timer. The single parameter stores
 * a pointer to the respective Task object whose period has
 * just expired.
 *
 * @param arg - sigval struct containing a pointer to a Task object
 */
void taskTimerExpired(sigval arg)
{
	Task* task = (Task*)arg.sival_ptr;
	if (!task->firstTimerRun)
	{
		task->periodEvent();
	}
	else
	{
		task->firstTimerRun = false;
	}
}
