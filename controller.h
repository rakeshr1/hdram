/*
 * =====================================================================================
 *
 *       Filename:  controller.h
 *
 *    Description:  DRAM Controller for Heterogeneous Memory
 *
 *        Version:  1.0
 *        Created:  05/31/2014 08:17:26 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakesh Ramesh (Doctoral Candidate), rakeshr1@stanford.edu
 *
 * =====================================================================================
 */

#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <list>
#include <vector>

#include "request.h"
#include "dram.h"

using namespace std;

const unsigned int NUM_TYPES = 2;

enum SchedPolicy {
	FIFO=0,
	PD_AWARE,
	BACKLOG
};

enum PDPolicy {
	NONE=0,
	CONSERVATIVE,
	WATERMARK
};

const unsigned int PD_WM = 10; // Watermark for power-down
const unsigned int TIMEOUT = 1000;

class Controller {
private:
	list<Request *> request_queue; // list for traversal
	DRAM **ranks[NUM_TYPES];

	vector<unsigned int> request_counter[NUM_TYPES];
	vector<unsigned int> mutual_request_counter;
	vector<bool> power_down_status[NUM_TYPES];

	unsigned int clock;

	// Config
	unsigned int num_ranks;
	SchedPolicy sched_policy;
	PDPolicy pd_policy;

public:
	Controller(unsigned int num_ranks_, unsigned int num_banks_, SchedPolicy sched_policy_, PDPolicy pd_policy_);
	~Controller();

	void clockTick();

	void addRequest(Request *req);
	void scheduleRequests();
	void schedPowerDown();

	unsigned int totalAccess();
	float avgLatency();
	float avgEnergy();
};

#endif
