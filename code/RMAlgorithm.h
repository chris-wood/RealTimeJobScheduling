//*****************************************************************
// RMAlgorithm.h
//
//  Created on: Dec 9, 2011
//      Author: Christopher Wood
//              Vineeth Vijayakumaran
//
//  $Id: RMAlgorithm.h 59 2012-01-11 04:28:36Z w463-01u1a $
//*****************************************************************

#ifndef RMALGORITHM_H_
#define RMALGORITHM_H_

// Module includes
#include "SchedulingAlgorithm.h"

/**
 * This class is responsible for encapsulating the
 * Rate Monotonic Analysis (RMA) algorithm for scheduling.
 */
class RMAlgorithm: public SchedulingAlgorithm
{
public:
	/**
	 * Default, empty constructor.
	 */
	RMAlgorithm();

	/**
	 * Default, empty destructor.
	 */
	virtual ~RMAlgorithm();

	/**
	 * Apply the RMA algorithm to build a list of task priorities
	 * that is then used to reschedule tasks during the schedule test.
	 *
	 * @param tasks - list of tasks under control of the schedule test
	 * @return descending priority list of tasks used for scheduling.
	 */
	vector<unsigned int> scheduleTasks(vector<Task*> tasks);
};

#endif /* RMALGORITHM_H_ */
