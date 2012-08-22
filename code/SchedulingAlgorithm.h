//*****************************************************************
// SchedulingAlgorithm.h
//
//  Created on: Dec 9, 2011
//      Author: Christopher Wood
//              Vineeth Vijayakumaran
//
//  $Id: SchedulingAlgorithm.h 59 2012-01-11 04:28:36Z w463-01u1a $
//*****************************************************************

#ifndef SCHEDULINGALGORITHM_H_
#define SCHEDULINGALGORITHM_H_

// Module includes
#include "Task.h"
#include "Project1.h"

// Resolve forward dependency
class Task;

/**
 * This is the interface/abstract class for all scheduling algorithms that
 * are used in the schedule test. It provides a default scheduler method
 * that assigns builds a priority list of tasks based on the current
 * set of tasks under control by the proxy scheduler.
 */
class SchedulingAlgorithm
{
public:
	/**
	 * Default, empty constructor.
	 */
	SchedulingAlgorithm();

	/**
	 * Default, empty destructor.
	 */
	virtual ~SchedulingAlgorithm();

	/**
	 * This is the abstract method that is invoked by the proxy
	 * scheduler to reschedule tasks during the schedule test.
	 *
	 * @param tasks - list of tasks under control of the schedule test
	 * @return descending priority list of tasks used for scheduling.
	 */
	virtual vector<unsigned int> scheduleTasks(vector<Task*> tasks) = 0;
};

#endif /* SCHEDULINGALGORITHM_H_ */
