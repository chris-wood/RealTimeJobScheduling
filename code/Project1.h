//*****************************************************************
// Project1.h
//
//  Created on: Dec 10, 2011
//      Author: Christopher Wood
//              Vineeth Vijayakumaran
//
//  $Id: Project1.h 59 2012-01-11 04:28:36Z w463-01u1a $
//*****************************************************************

#ifndef PROJECT1_H_
#define PROJECT1_H_

// Standard C/C++ includes
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <unistd.h>
#include <assert.h>
#include <inttypes.h>

// Global QNX libraries
#include <time.h>
#include <sys/trace.h>
#include <sys/siginfo.h>
#include <sys/syspage.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <semaphore.h>

// For simplicity
using namespace std;

// Task data structure used to pass data to the proxy scheduler
typedef struct
{
	unsigned int computeTime;
	unsigned int periodTime;
} TaskData;

// Enumeration of the different scheduling algorithms available
typedef enum
{
	ALGORITHM_TYPE_RMA, // 0
	ALGORITHM_TYPE_EDF, // 1
	ALGORITHM_TYPE_SCT, // 2
	ALGORITHM_TYPE_LAST_ENTRY
} AlgorithmType;

// Enumeration for the different types of schedule test events
typedef enum
{
	EVENT_SCHEDULE,
	EVENT_MISSED_DEADLINE,
	EVENT_SCHEDULE_TRACE,
	EVENT_PROXY_DATA,
	EVENT_TASK_DATA,
	EVENT_LAST_ENTRY
} EventType;

#endif /* PROJECT1_H_ */
