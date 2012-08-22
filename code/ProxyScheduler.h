//*****************************************************************
// ProxyScheduler.h
//
//  Created on: Dec 9, 2011
//      Author: Christopher Wood
//              Vineeth Vijayakumaran
//
//  $Id: ProxyScheduler.h 59 2012-01-11 04:28:36Z w463-01u1a $
//*****************************************************************

#ifndef PROXYSCHEDULER_H_
#define PROXYSCHEDULER_H_

// Module includes
#include "Thread.h"
#include "Project1.h"
#include "Task.h"
#include "RMAlgorithm.h"
#include "SCTAlgorithm.h"
#include "EDFAlgorithm.h"
#include "SchedulingAlgorithm.h"

// Forward declaration due to bidirection association
class Task;

// Global semaphore that is exposed for quick (minimal overhead) access by tasks
extern sem_t proxySem;

// Global task list that is exposed for quick (minimal overhead) access by tasks
extern vector<int> scheduleList;

/**
 * This class is responsible for acting as a proxy to the main QNX scheduler
 * by using provided scheduling algorithms to determine task priorities for
 * scheduling and handling all scheduling events generated from its set of
 * active tasks.
 */
class ProxyScheduler : public Thread
{
public:
	/**
	 * Default constructor for the proxy scheduler, which
	 * takes in a list of tasks and begins execution.
	 *
	 * @param taskSet - the collection of pointers to task objects.
	 */
	ProxyScheduler(AlgorithmType alg, vector<TaskData> taskSet, int runtime, unsigned int id);

	/**
	 * Default destructor that traverses the list of tasks and
	 * destroys each one by one.
	 */
	virtual ~ProxyScheduler();

	/**
	 * Configure the timer for the schedule test duration.
	 */
	void configureTimer();

	/**
	 * Start the timer for the schedule test duration.
	 */
	void startTimer();

	/**
	 * Signal a test completion event.
	 */
	void testComplete();

	/**
	 * Set the scheduler's priority.
	 */
	void setPriority(int priority);

protected:
	/**
	 * The start routine that is executed when the client calls start().
	 */
	void* startRoutine();

private:
	
	/**
	 * Log all data collected at runtime to the kernel event stream and stdout.
	 */
	void logData();

	/**
	 * Set the priority of each task from the context of the proxy scheduler thread.
	 *
	 * @param priorities - the descending list of priorities used
	 *                     to assign dynamic priorities.
	 */
	void setTaskPriorities(vector<unsigned int> priorities);

	/**
	 * Release all tasks from their blocked state.
	 *
	 * @param priorities - list of priorities used to determine the order of release.
	 */
	void releaseTasks(vector<unsigned int> priorities);

	/**
	 * Preempt all running tasks and put them in their blocked state.
	 */
	void pauseTasks();

	/**
	 * Start the schedule test.
	 */
	void runTest();

	// Structures used by the underlying task timer thread
	struct itimerspec timerSpec;
	struct sigevent event;

	// The scheduler's unique timer ID
	timer_t timerID;

	// The scheduler's base priority used as a limit when determining
	// all active task priorities.
	int priority;
	
	// Boolean flag indicating whether or not the schedule test is complete.
	bool timeExpired;

	// Runtime for each schedule test (repeated for the different algorithms).
	uint64_t runtime;

	// The real (based on clock cycles) runtime for the test.
	uint64_t realRuntime;

	// Total schedule event time and number of events
	uint64_t realScheduleTime;
	uint64_t numScheduleEvents;

	// The type of algorithm being used for this specific test.
	AlgorithmType algorithmType;

	// Internal collection of tasks that are managed by the scheduler.
	vector<Task*> tasks;

	// Temporary list of task data used to construct the main list of tasks.
	vector<TaskData> taskData;

	// Convenience map that associates task ID's with task objects
	// (used when assigning priorities).
	map<int, Task*> taskMap;
	
	// The current scheduling algorithm object used to determine task priorities.
	SchedulingAlgorithm* scheduler;

	// Constant for the initial semaphore value.
	static const int SEM_COUNT = 0; // Binary semaphore
};

#endif /* PROXYSCHEDULER_H_ */
