/*
 * =====================================================================================
 *
 *       Filename:  dram.h
 *
 *    Description:  Crude DRAM model
 *
 *        Version:  1.0
 *        Created:  05/31/2014 08:43:15 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakesh Ramesh (Doctoral Candidate), rakeshr1@stanford.edu
 *      Execution:     
 *
 * =====================================================================================
 */

#ifndef _DRAM_H_
#define _DRAM_H_

#include <queue>
#include <vector>

#include "request.h"

using namespace std;

enum Status {
	IDLE=0,
	ACTIVE,
	POWER_DOWN
};

struct Parameters {
	unsigned long int latency;
	unsigned long int power_up_latency;
	float dynamic_power;
	float static_power;
	float power_down_power;
};

class DRAM {
private:
	vector< queue<Request*> > command_queue; // Command Q per bank
	Status status;

	vector< Request * > now_serving;
	vector<unsigned long int> req_timer;
	unsigned long int power_up_timer;

	int next_bank; // Round-robin for banks
	unsigned long int clock;

	// Config
	unsigned int num_banks;
	Parameters param;

	// Stats
	unsigned int num_access;
	float average_latency;
	unsigned long int num_idle_cycles;
	unsigned long int num_power_down_cycles;

public:
	DRAM(unsigned int num_banks_, unsigned int type);
	~DRAM();

	void clockTick();

	void addRequest(Request *req);
	void powerDown();
	void powerUp();

	unsigned int backlog(unsigned int bank);
	unsigned int totalBacklog();
	unsigned int numAccess();
	float avgLatency();
	float avgEnergy();
};

#endif
