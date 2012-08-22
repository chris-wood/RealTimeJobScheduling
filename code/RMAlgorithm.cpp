//*****************************************************************
// RMAlgorithm.cpp
//
//  Created on: Dec 9, 2011
//      Author: Christopher Wood
//              Vineeth Vijayakumaran
//
//  $Id: RMAlgorithm.cpp 59 2012-01-11 04:28:36Z w463-01u1a $
//*****************************************************************

// Module includes
#include "RMAlgorithm.h"

/**
 * Default, empty constructor.
 */
RMAlgorithm::RMAlgorithm()
{
}

/**
 * Default, empty destructor.
 */
RMAlgorithm::~RMAlgorithm()
{
}

/**
 * Apply the RMA algorithm to build a list of task priorities
 * that is then used to reschedule tasks during the schedule test.
 *
 * @param tasks - list of tasks under control of the schedule test
 * @return descending priority list of tasks used for scheduling.
 */
vector<unsigned int> RMAlgorithm::scheduleTasks(vector<Task*> tasks)
{
	vector<Task*> sorted;
	vector<unsigned int> priorities;

	// Sort the tasks based on the frequency of their period (lower period, higher priority)
	for (vector<Task*>::iterator itr = tasks.begin(); itr != tasks.end(); itr++)
	{
		if (sorted.empty())
		{
			// Insert something to start
			sorted.push_back(*itr);
		}
		else
		{
			bool inserted = false;
			for (vector<Task*>::iterator pitr = sorted.begin(); pitr != sorted.end(); pitr++)
			{
				if ((*itr)->getPeriodTime() <= (*pitr)->getPeriodTime())
				{
					sorted.insert(pitr, *itr);
					inserted = true;
					break;
				}
			}
			if (!inserted)
			{
				sorted.push_back(*itr);
			}
		}
	}

	// Populate the priorities vector based on the now sorted list
	for (vector<Task*>::iterator itr = sorted.begin(); itr != sorted.end(); itr++)
	{
		priorities.push_back((*itr)->taskID());
	}

	return priorities;
}
