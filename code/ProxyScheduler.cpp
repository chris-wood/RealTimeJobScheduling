//*****************************************************************
// ProxyScheduler.cpp
//
//  Created on: Dec 9, 2011
//      Author: Christopher Wood
//              Vineeth Vijayakumaran
//
//  $Id: ProxyScheduler.cpp 59 2012-01-11 04:28:36Z w463-01u1a $
//*****************************************************************

#include "ProxyScheduler.h"

// Global semaphore that is exposed for quick (minimal overhead) access by tasks
sem_t proxySem;

// Global task list that is exposed for quick (minimal overhead) access by tasks
vector<int> scheduleList;

/**
 * External (but friendly) function that is used as a callback
 * for the schedule test timer. The single parameter stores
 * a pointer to the respective ProxyScheduler object whose period has
 * just expired.
 *
 * @param arg - sigval struct containing a pointer to a ProxyScheduler object
 */
void testTimerExpired(sigval arg);

/**
 * Default constructor for the proxy scheduler, which
 * takes in a list of tasks and begins execution.
 *
 * @param taskSet - the collection of pointers to task objects.
 */
ProxyScheduler::ProxyScheduler(AlgorithmType alg, vector<TaskData> taskSet, int runtime, unsigned int id)
{
	int result;

	// Attempt to initialize the execution semaphore.
	result = sem_init(&proxySem, 0, SEM_COUNT);
	if (result != 0)
	{
		cerr << "Error initializing execution semaphore for task " <<
				id << endl;
		uid = -1; // serves as error flag for proxy scheduler
	}
	else
	{
		// Initialize the rest of the task parameters.
		this->taskData = taskSet;
		this->runtime = runtime;
		this->algorithmType = alg;
		this->uid = id;
		this->realScheduleTime = 0;
		this->numScheduleEvents = 0;

		// Configure the runtime timer.
		configureTimer();
	}
}

/**
 * Default destructor that traverses the list of tasks and
 * destroys each one by one.
 */
ProxyScheduler::~ProxyScheduler()
{
	// Traverse task list and destroy all instances.
	for (vector<Task*>::iterator itr = tasks.begin(); itr != tasks.end(); itr++)
	{
		delete(*itr);
	}
}

/**
 * The start routine that is executed when the client calls start().
 */
void* ProxyScheduler::startRoutine()
{	
	// First, create the tasks and then kick them off to run
	int taskID = 0;
	int pol = 0;
	uint64_t startCycleTime;
	uint64_t endCycleTime;
	struct sched_param schedParam;

	for (vector<TaskData>::iterator itr = taskData.begin(); itr != taskData.end(); itr++)
	{
		Task* task = new Task(taskID++, (*itr).computeTime, (*itr).periodTime);
		tasks.push_back(task);
	}

	// Construct the convenience task map.
	for (vector<Task*>::iterator itr = tasks.begin(); itr != tasks.end(); itr++)
	{
		taskMap[(*itr)->taskID()] = *itr; // drop each task pointer in the map
	}

	// Now construct the appropriate scheduler based on the algorithm type
	switch (algorithmType)
	{
	case ALGORITHM_TYPE_RMA:
		scheduler = new RMAlgorithm();
		break;
	case ALGORITHM_TYPE_EDF:
		scheduler = new EDFAlgorithm();
		break;
	case ALGORITHM_TYPE_SCT:
		scheduler = new SCTAlgorithm();
		break;
	default:
		cerr << "Invalid scheduling algorithm selection. Terminating now." << endl;
		kill();
		return NULL;
	}

	// Now run the test and then clean up
	cout << "START" << endl;
	startCycleTime = ClockCycles();
	runTest();
	endCycleTime = ClockCycles();
	realRuntime = (endCycleTime - startCycleTime);
	cout << "STOP" << endl;
	delete scheduler;

	// Log proxy scheduler data
	logData();

	// Log all task data and then kill each task
	for (vector<Task*>::iterator itr = tasks.begin(); itr != tasks.end(); itr++)
	{
		(*itr)->logData();
		(*itr)->stopTask();
	}

	// Terminate and return
	kill();
}

/**
 * Start the schedule test.
 */
void ProxyScheduler::runTest()
{
	timeExpired = false;
	uint64_t startCycleTime = 0;
	uint64_t endCycleTime = 0;

	// Determine the initial task schedules
	vector<unsigned int> priorities = scheduler->scheduleTasks(tasks);

	// Start the timer to run in the background while the test is performed
	startTimer();

	// Start each task
	for (vector<Task*>::iterator itr = tasks.begin(); itr != tasks.end(); itr++)
	{
		(*itr)->start();
	}

	// Allow each timer to start
	releaseTasks(priorities);
	for (unsigned int i = 0; i < tasks.size(); i++)
	{
		sched_yield();
		sem_wait(&proxySem);
	}

	// Finally, assign priorities and start each task
	setTaskPriorities(priorities);
	releaseTasks(priorities); // this release starts the tests

	// Run the test until the time expires
	while (!timeExpired)
	{
		// Blocks on scheduling semaphore
		sem_wait(&proxySem);
		startCycleTime = ClockCycles();

		// Pause all tasks
		for (vector<Task*>::iterator itr = tasks.begin(); itr != tasks.end(); itr++)
		{
			(*itr)->pause();
		}

		// Re-determine the new priorities
		priorities = scheduler->scheduleTasks(tasks);

		// Assign the priorities and then release all tasks again
		setTaskPriorities(priorities);
		releaseTasks(priorities);

		// Record the time for this schedule event
		endCycleTime = ClockCycles();
		realScheduleTime += (endCycleTime - startCycleTime);
		numScheduleEvents++;
	}

	// Kill all tasks
	for (vector<Task*>::iterator itr = tasks.begin(); itr != tasks.end(); itr++)
	{
		(*itr)->stopTest();
	}

	// Delete the timer - no longer needed
	timer_delete(timerID);
}

/**
 * Log all data collected at runtime to the kernel event stream and stdout.
 */
void ProxyScheduler::logData()
{
	uint64_t cps;
	float realSchedTime = 0;
	float realTime = 0;
	char data[256]; // arbitrary size big enough to fit what we need
	char stringHolder[4];
	string trace;

	// Determine the clock rate
	cps = SYSPAGE_ENTRY(qtime)->cycles_per_sec;

	// Calculate the real compute time period
	realTime = ((float)((float)realRuntime / (float)cps));
	realSchedTime = ((float)((float)realScheduleTime / (float)cps));

	// Log the schedule trace
	for (vector<int>::iterator itr = scheduleList.begin(); itr != scheduleList.end(); itr++)
	{
		trace.append(itoa(*itr, stringHolder, 10));
		trace.append(",");
	}
	TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, EVENT_SCHEDULE_TRACE, trace.c_str());
	cout << "TRACE " << trace.c_str() << endl;

	// Log the data
	sprintf(data, "PDATA %f,%f,%f", realSchedTime / numScheduleEvents, realTime, (float)(realTime - runtime) / (realTime));
	TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, EVENT_PROXY_DATA, data);
	cout << data << endl;
}

/**
 * Set the priority of each task from the context of the proxy scheduler thread.
 *
 * @param priorities - the descending list of priorities used
 *                     to assign dynamic priorities.
 */
void ProxyScheduler::setTaskPriorities(vector<unsigned int> priorities)
{
	struct sched_param* schedParam;

	// Assign priorities in descending order in order to let the QNX scheduler
	// manage the scheduling with as little overhead as possible.
	int val = 0;
	int pol;
	for (vector<unsigned int>::reverse_iterator itr = priorities.rbegin(); itr != priorities.rend(); itr++)
	{
		schedParam = taskMap[*itr]->getSchedParam();
		schedParam->sched_priority = priority + val++;
		pthread_setschedparam(taskMap[*itr]->threadID(), SCHED_RR, schedParam);
		pthread_getschedparam(taskMap[*itr]->threadID(), &pol, schedParam);
	}
}

/**
 * Release all tasks from their blocked state.
 *
 * @param priorities - list of priorities used to determine the order of release.
 */
void ProxyScheduler::releaseTasks(vector<unsigned int> priorities)
{
	for (vector<unsigned int>::iterator itr = priorities.begin(); itr != priorities.end(); itr++)
	{
		taskMap[*itr]->release();
	}
}

/**
 * Preempt all running tasks and put them in their blocked state.
 */
void ProxyScheduler::pauseTasks()
{
	for (vector<Task*>::iterator itr = tasks.begin(); itr != tasks.end(); itr++)
	{
		(*itr)->pause();
	}
}

/**
 * Configure the timer for the schedule test duration.
 */
void ProxyScheduler::configureTimer()
{
	// Initialize the daemon timer thread to invoke a function when it expires
	SIGEV_THREAD_INIT(&event, &testTimerExpired, this, 0);

	// Configure the timer parameters (period signals only)
	timerSpec.it_value.tv_sec = runtime;
	timerSpec.it_value.tv_nsec = 0;
	timerSpec.it_interval.tv_sec = runtime;
	timerSpec.it_interval.tv_nsec = 0;
}

/**
 * Start the timer for the schedule test duration.
 */
void ProxyScheduler::startTimer()
{
	timer_create(CLOCK_REALTIME, &event, &timerID);
	timer_settime(timerID, 0, &timerSpec, NULL);
}

/**
 * Signal a test completion event.
 */
void ProxyScheduler::testComplete()
{
	timeExpired = true;
	sem_post(&proxySem);
}

/**
 * Set the scheduler's priority.
 */
void ProxyScheduler::setPriority(int priority)
{
	this->priority = priority;
}

/**
 * External (but friendly) function that is used as a callback
 * for the schedule test timer. The single parameter stores
 * a pointer to the respective ProxyScheduler object whose period has
 * just expired.
 *
 * @param arg - sigval struct containing a pointer to a ProxyScheduler object
 */
void testTimerExpired(sigval arg)
{
	ProxyScheduler* scheduler = (ProxyScheduler*)arg.sival_ptr;
	scheduler->testComplete();
}
