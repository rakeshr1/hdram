/*
 * =====================================================================================
 *
 *       Filename:  dram.cpp
 *
 *    Description:  Crude DRAM model
 *
 *        Version:  1.0
 *        Created:  05/31/2014 09:33:09 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakesh Ramesh (Doctoral Candidate), rakeshr1@stanford.edu
 *      Execution:     
 *
 * =====================================================================================
 */

#include "dram.h"

DRAM::DRAM(unsigned int num_banks_, unsigned int type) {
	status = IDLE;

	num_banks = num_banks_;

	command_queue.resize(num_banks);
	now_serving.resize(num_banks);

	req_timer.resize(num_banks);
	for(int i=0; i<num_banks; i++) {
		req_timer[i] = 0;
		now_serving[i] = NULL;
	}
	power_up_timer = 0;

	if(type == 0) {
		// Assign GDDR5 parameters
		param.latency = 20;
		param.dynamic_power = 1630;
		param.static_power = 620;
		param.power_down_power = 280;
		param.power_up_latency = 400;

		// Assign RLDRAM3 parameters
		// param.latency = 16.5;
		// param.dynamic_power = 1175;
		// param.static_power = 725;
		// param.power_down_power = 125;
		// param.power_up_latency = 200;
	} else if(type == 1) {
		// Assign DDR3 parameters
		param.latency = 47;
		param.dynamic_power = 270;
		param.static_power = 45;
		param.power_down_power = 40;
		param.power_up_latency = 600;

		// Assign LPDDR2 parameters
		// param.latency = 60;
		// param.dynamic_power = 5;
		// param.static_power = 1.2;
		// param.power_down_power = 0.5;
		// param.power_up_latency = 760;
	}

	next_bank = 0;

	// Init stats
	num_access = 0;
	average_latency = 0;
	num_idle_cycles = 0;
	num_power_down_cycles = 0;
}

DRAM::~DRAM() {
	for(int i=0; i < num_banks; i++) {
		while(!command_queue[i].empty()) {
			Request *req = command_queue[i].front();
			command_queue[i].pop();
			delete req;
		}
	}
}

void DRAM::clockTick() {
	if(status == POWER_DOWN) {
		clock++;
		num_power_down_cycles++;
		return;
	} else if(status == IDLE) {
		num_idle_cycles++;

		// Powering up
		if(power_up_timer != 0) {
			power_up_timer--;
			clock++;
			return;
		}
	}

	// Check for serving
	if(req_timer[next_bank] == 0) {
		if(!command_queue[next_bank].empty()) {
			now_serving[next_bank] = command_queue[next_bank].front();
			command_queue[next_bank].pop();

			req_timer[next_bank] = param.latency;
			status = ACTIVE;
		}
	} else {
		req_timer[next_bank]--;
		if(req_timer[next_bank] == 0) {
			now_serving[next_bank]->end_time = clock;
			now_serving[next_bank]->latency = now_serving[next_bank]->end_time - now_serving[next_bank]->start_time;
			// cout << "Request ptr : " << now_serving[next_bank] << " served : " << *now_serving[next_bank] << " End : " << clock << endl;

			average_latency = (average_latency * num_access + now_serving[next_bank]->latency) / float(num_access + 1);
			num_access++;

			delete now_serving[next_bank];
			now_serving[next_bank] = NULL;
			status = IDLE;
		}
	}

	// Round-robin
	unsigned int prev_bank = next_bank;
	do {
		next_bank = (next_bank + 1) % num_banks;
	} while(prev_bank != next_bank && command_queue[next_bank].empty());

	clock++;
}

void DRAM::addRequest(Request *req) {
	// cout << "Adding request : " << *req << endl;
	command_queue[req->bank].push(req);
}

void DRAM::powerDown() {
	status = POWER_DOWN;
}

void DRAM::powerUp() {
	status = IDLE;
	power_up_timer = param.power_up_latency;
}

unsigned int DRAM::backlog(unsigned int bank) {
	return command_queue[bank].size();
}

unsigned int DRAM::totalBacklog() {
	unsigned int total_backlog = 0;
	for(int i=0; i < num_banks; i++) {
		total_backlog += command_queue[i].size();
	}
	return total_backlog;
}

unsigned int DRAM::numAccess() {
	return num_access;
}

float DRAM::avgLatency() {
	if(num_access == 0) {
		return 0;
	}

	return average_latency;
}

float DRAM::avgEnergy() {
	if(num_access == 0) {
		return 0;
	}

	float total_dynamic_energy = param.dynamic_power * num_access;
	float total_idle_energy = param.static_power * num_idle_cycles;
	float total_pd_energy = param.power_down_power * num_power_down_cycles;

	float total_energy = total_dynamic_energy + total_idle_energy + total_pd_energy;
	float average_energy = total_energy / num_access;

	return average_energy;
}

