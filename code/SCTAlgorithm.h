//*****************************************************************
// SCTAlgorithm.h
//
//  Created on: Dec 9, 2011
//      Author: Christopher Wood
//              Vineeth Vijayakumaran
//
//  $Id: SCTAlgorithm.h 59 2012-01-11 04:28:36Z w463-01u1a $
//*****************************************************************

#ifndef SCTALGORITHM_H_
#define SCTALGORITHM_H_

// Module includes
#include "SchedulingAlgorithm.h"

/**
 * This class is responsible for encapsulating the
 * Shortest Completion Time (SCT) algorithm for scheduling.
 */
class SCTAlgorithm: public SchedulingAlgorithm
{
public:
	/**
	 * Default, empty constructor.
	 */
	SCTAlgorithm();

	/**
	 * Default, empty destructor.
	 */
	virtual ~SCTAlgorithm();

	/**
	 * Apply the SCT algorithm to build a list of task priorities
	 * that is then used to reschedule tasks during the schedule test.
	 *
	 * @param tasks - list of tasks under control of the schedule test
	 * @return descending priority list of tasks used for scheduling.
	 */
	vector<unsigned int> scheduleTasks(vector<Task*> tasks);
};

#endif /* SCTALGORITHM_H_ */
