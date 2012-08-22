//*****************************************************************
// Thread.cpp
//
//  Created on: Oct 15, 2011
//      Author: Christopher Wood
//              Vineeth Vijayakumaran
//
//  $Id: Thread.cpp 59 2012-01-11 04:28:36Z w463-01u1a $
//*****************************************************************

// Module includes
#include "Thread.h"
#include <stdexcept>
#include <unistd.h>

/**
 * Default constructor for a thread (does nothing until user starts the thread manually)
 */
Thread::Thread()
{
	// Empty constructor
}

/**
 * Default destructor for the thread (no tear-down required)
 */
Thread::~Thread()
{
	// Empty...
}

/**
 * Start the thread by invoking its start routine.
 *
 * @param - none
 * @returns - nothing
 */
void Thread::start()
{
	// Flag the thread as being alive
	alive = true;

	if (pthread_create(&m_id, NULL, &(this->startRoutineTrampoline), this) != 0)
	{
		alive = false;
		throw std::runtime_error("Thread was not created");
	}
}

/**
 * Perform a timed microsecond sleep on this thread
 *
 * @param - ms - microsecond count for sleep
 * @return - nothing
 */
void Thread::sleep(unsigned long ms)
{
	usleep(ms);
}

/**
 * Kill this thread by flagging it as not alive and invoking
 * the pthread exit routine.
 *
 * @param - none
 * @return - nothing
 */
void Thread::kill()
{
	alive = false;
	pthread_exit(NULL);
}

/**
 * Join this thread on the calling thread context.
 *
 * @param - none
 * @return - nothing
 */
void Thread::join()
{
	pthread_join(m_id, NULL);
}

/**
 * Determine if this thread is alive or not.
 *
 * @param - none
 * @return - nothing
 */
bool Thread::isAlive()
{
	return alive;
}

/**
 * Retrieve the ID of this active thread.
 *
 * @return thread ID
 */
unsigned int Thread::taskID()
{
	return uid;
}

/**
 * Return the POSIX thread ID of this active thread.
 *
 * @return POSIX thread ID
 */
pthread_t Thread::threadID()
{
	return m_id;
}
