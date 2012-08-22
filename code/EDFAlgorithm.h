//*****************************************************************
// EDFAlgorithm.h
//
//  Created on: Dec 9, 2011
//      Author: Christopher Wood
//              Vineeth Vijayakumaran
//
//  $Id: EDFAlgorithm.h 59 2012-01-11 04:28:36Z w463-01u1a $
//*****************************************************************

#ifndef EDFALGORITHM_H_
#define EDFALGORITHM_H_

// Module Includes
#include "SchedulingAlgorithm.h"

/**
 * This class is responsible for encapsulating the
 * Earliest Deadline First (EDF) algorithm for scheduling.
 */
class EDFAlgorithm: public SchedulingAlgorithm
{
public:
	/**
	 * Default, empty constructor.
	 */
	EDFAlgorithm();

	/**
	 * Default, empty destructor.
	 */
	virtual ~EDFAlgorithm();

	/**
	 * Apply the EDF algorithm to build a list of task priorities
	 * that is then used to reschedule tasks during the schedule test.
	 *
	 * @param tasks - list of tasks under control of the schedule test
	 * @return descending priority list of tasks used for scheduling.
	 */
	vector<unsigned int> scheduleTasks(vector<Task*> tasks);
};

#endif /* EDFALGORITHM_H_ */
