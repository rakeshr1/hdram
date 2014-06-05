/*
 * =====================================================================================
 *
 *       Filename:  Request.h
 *
 *    Description:  Memory Request Structure
 *
 *        Version:  1.0
 *        Created:  05/31/2014 08:23:56 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakesh Ramesh (Doctoral Candidate), rakeshr1@stanford.edu
 *
 * =====================================================================================
 */

#ifndef _REQUEST_H_
#define _REQUEST_H_

#include <iostream>
using namespace std;

struct Request {
	// Address map
	unsigned int type; // NOTE: Supports only two types for now
	unsigned int rank;
	unsigned int bank;

	// Latency book keep
	unsigned long int start_time;
	unsigned long int end_time;
	unsigned long int latency;

	// Other counters
};

ostream &operator<<(ostream &out, Request &req);

#endif
