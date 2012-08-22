//*****************************************************************
// Thread.h
//
//  Created on: Oct 15, 2011
//      Author: Christopher Wood
//              Vineeth Vijayakumaran
//
//  $Id: Thread.h 59 2012-01-11 04:28:36Z w463-01u1a $
//*****************************************************************

#ifndef THREAD_H_
#define THREAD_H_

// Module includes
#include "pthread.h"

/**
 * This class represents a wrapper for the POSIX thread implementation
 * provided by QNX. It is intended to simplify thread construction
 * and operation by other modules in the system.
 *
 * NOTE: THIS IS AN ABSTRACT CLASS
 */
class Thread
{
public:
	/**
	 * Default constructor for a thread (does nothing until user starts the thread manually)
	 */
	Thread();

	/**
	 * Default destructor for the thread (no tear-down required)
	 */
	virtual ~Thread();

	/**
	 * Start the thread by invoking its start routine.
	 *
	 * @param - none
	 * @returns - nothing
	 */
	void start();

	/**
	 * Perform a timed microsecond sleep on this thread
	 *
	 * @param - ms - microsecond count for sleep
	 * @return - nothing
	 */
	void sleep(unsigned long ms);

	/**
	 * Kill this thread by flagging it as not alive and invoking
	 * the pthread exit routine.
	 *
	 * @param - none
	 * @return - nothing
	 */
	void kill();

	/**
	 * Join this thread on the calling thread context.
	 *
	 * @param - none
	 * @return - nothing
	 */
	void join();

	/**
	 * Determine if this thread is alive or not.
	 *
	 * @param - none
	 * @return - nothing
	 */
	bool isAlive();

	/**
	 * Retrieve the ID of this active thread.
	 *
	 * @return thread ID
	 */
	unsigned int taskID();

	/**
	 * Return the POSIX thread ID of this active thread.
	 *
	 * @return POSIX thread ID
	 */
	pthread_t threadID();

protected:
	// Boolean flag indicating if this thread is still running.
	volatile bool alive;

	/**
	 * Pure virtual method that subclasses must override.
	 */
	virtual void *startRoutine() = 0;

	// This task's unique ID.
	unsigned int uid;

	// Some useful constants used by all threads for timing.
	static const unsigned int NS_PER_MS = 1000000;
	static const unsigned int NS_PER_SEC = 1000000000;

private:
	// This thread's task ID
    pthread_t m_id;

	/**
	 * In-line function implementation due to its simplicity.
	 */
    static void *startRoutineTrampoline(void *p)
    {
        Thread* pThis = (Thread*)p;
        return pThis->startRoutine();
    }
};

#endif /* THREAD_H_ */
