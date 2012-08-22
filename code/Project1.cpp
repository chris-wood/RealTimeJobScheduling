//*****************************************************************
// Project1.cpp
//
//  Created on: Dec 9, 2011
//      Author: Christopher Wood
//              Vineeth Vijayakumaran
//
//  $Id: Project1.cpp 59 2012-01-11 04:28:36Z w463-01u1a $
//*****************************************************************

// Module includes
#include "Project1.h"
#include "Task.h"
#include "ProxyScheduler.h"

// Private constants
#define CLOCK_RESOLUTION (50000)
#define PRIORITY_OFFSET  (5)

/**
 * The main entry point into the application.
 */
int main(int argc, char *argv[])
{
	int testRuntime = 0;
	int numTasks = 0;
	int taskID = 0;
	int computeTime = 0;
	int periodTime = 0;
	int pol = 0;
	int algorithm = 0;
	struct _clockperiod period;
	vector<TaskData> tasks;
	ProxyScheduler* scheduler;
	struct sched_param schedParam;

	// Read in the algorithm selection from stdin and do a quick validation
	cout << "Algorithm choice: ";
	cin >> algorithm;
	assert(algorithm >= ALGORITHM_TYPE_RMA && algorithm < ALGORITHM_TYPE_LAST_ENTRY);

	// Read in text fixture parameters from stdin
	cout << "Test runtime: ";
	cin >> testRuntime;
	cout << "Number of tasks: ";
	cin >> numTasks;

	// Read in task parameters from stdin
	cout << "Task data ([c,p] pairs):" << endl;
	for (int count = 0; count < numTasks; count++)
	{
		// Read in this individual task's parameters (compute-period pair).
		cin >> computeTime;
		cin >> periodTime;
		assert(computeTime <= periodTime); // just to be safe

		// Push a new task object into the list.
		TaskData data;
		data.computeTime = computeTime;
		data.periodTime = periodTime;
		tasks.push_back(data);
	}

	// Calibrate timing for nanospin
	ThreadCtl(_NTO_TCTL_IO, NULL); // Get I/O privileges first
	nanospin_calibrate(1); // EINTR = 4 -> too many interrupts during calibration

	// Set the clock resolution to 0.5ms (sufficient for our tests).
	period.fract = 0;
	period.nsec = CLOCK_RESOLUTION;
	ClockPeriod(CLOCK_REALTIME, &period, NULL, 0);

	// Give the proxy scheduler the highest priority and then start it.
	scheduler = new ProxyScheduler((AlgorithmType)algorithm, tasks, testRuntime, taskID++);
	pthread_getschedparam(scheduler->threadID(), &pol, &schedParam);
	scheduler->setPriority(schedParam.sched_priority);
	schedParam.sched_priority = schedParam.sched_priority + tasks.size() + PRIORITY_OFFSET;
	pthread_setschedparam(scheduler->threadID(), SCHED_RR, &schedParam);
	scheduler->start();

	// Wait until the proxy scheduler terminates before cleaning up.
	scheduler->join();

	delete scheduler;
	return EXIT_SUCCESS;
}
