/*
 * =====================================================================================
 *
 *       Filename:  controller.cpp
 *
 *    Description:  DRAM Controller for Heterogeneous Memory
 *
 *        Version:  1.0
 *        Created:  05/31/2014 09:31:14 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakesh Ramesh (Doctoral Candidate), rakeshr1@stanford.edu
 *
 * =====================================================================================
 */

#include "controller.h"
#include <cstdlib>

Controller::Controller(unsigned int num_ranks_, unsigned int num_banks_, SchedPolicy sched_policy_, PDPolicy pd_policy_) : num_ranks(num_ranks_) {
	sched_policy = sched_policy_;
	pd_policy = pd_policy_;

	for(int i=0; i < NUM_TYPES; i++) {
		ranks[i] = new DRAM* [num_ranks];
		request_counter[i].resize(num_ranks);
		power_down_status[i].resize(num_ranks);

		for(int j=0; j<num_ranks; j++) {
			ranks[i][j] = new DRAM(num_banks_, i);
			request_counter[i][j] = 0;
			power_down_status[i][j] = false;
		}
	}

	mutual_request_counter.resize(num_ranks);

	clock = 0;
}

Controller::~Controller() {
	while(!request_queue.empty()) {
		Request *req = request_queue.front();
		request_queue.pop_front();
		delete req;
	}

	for(int i=0; i < NUM_TYPES; i++) {
		for(int j=0; j<num_ranks; j++) {
			delete ranks[i][j];
		}
		delete [] ranks[i];
	}
}

void Controller::clockTick() {
	for(int i=0; i < NUM_TYPES; i++) {
		for(int j=0; j<num_ranks; j++) {
			ranks[i][j]->clockTick();
		}
	}

	scheduleRequests();

	schedPowerDown();

	clock++;
}

void Controller::addRequest(Request *req) {
	// cout << "Adding request : " << *req << endl;
	request_queue.push_back(req);

	if(req->type != 2) {
		request_counter[req->type][req->rank]++;
	} else {
		mutual_request_counter[req->rank]++;
	}
}

void Controller::scheduleRequests() {
	// FIFO as starting point
	if(sched_policy == FIFO) {
		if(!request_queue.empty()) {
			Request *req = request_queue.front();
			request_queue.pop_front();

			if(req->type == 2) { 
				unsigned int type1_backlog = ranks[0][req->rank]->backlog(req->bank);
				unsigned int type2_backlog = ranks[1][req->rank]->backlog(req->bank);

				if(type1_backlog < type2_backlog) {
					ranks[0][req->rank]->addRequest(req);
					if(power_down_status[0][req->rank] == true) {
						ranks[0][req->rank]->powerUp();
					}
				} else {
					ranks[1][req->rank]->addRequest(req);
					if(power_down_status[1][req->rank] == true) {
						ranks[1][req->rank]->powerUp();
					}
				}
				mutual_request_counter[req->rank]--;
			} else {
				// cout << "Adding request at clock : " << clock << " to type : " << req->type << " rank : " << req->rank << " req : " << req << endl;
				ranks[req->type][req->rank]->addRequest(req);
				request_counter[req->type][req->rank] -= 1;
				if(power_down_status[req->type][req->rank] == true) {
					ranks[req->type][req->rank]->powerUp();
				}
			}
		}
	} else if(sched_policy == PD_AWARE) {
		if(!request_queue.empty()) {
			Request *req = request_queue.front();
			request_queue.pop_front();

			if(req->type == 2) { 
				if(power_down_status[0][req->rank] == false && power_down_status[1][req->rank] == true) {
					ranks[0][req->rank]->addRequest(req);
				} else if(power_down_status[0][req->rank] == true && power_down_status[1][req->rank] == false) {
					ranks[1][req->rank]->addRequest(req);
				}

				unsigned int type1_backlog = ranks[0][req->rank]->backlog(req->bank);
				unsigned int type2_backlog = ranks[1][req->rank]->backlog(req->bank);

				if(type1_backlog < type2_backlog) {
					ranks[0][req->rank]->addRequest(req);
				} else {
					ranks[1][req->rank]->addRequest(req);
				}
				mutual_request_counter[req->rank]--;
			} else {
				// cout << "Adding request at clock : " << clock << " to type : " << req->type << " rank : " << req->rank << " req : " << req << endl;
				ranks[req->type][req->rank]->addRequest(req);
				request_counter[req->type][req->rank]--;
				if(power_down_status[req->type][req->rank] == true) {
					ranks[req->type][req->rank]->powerUp();
					// cout << "Clock : " << clock << " powering up type : " << req->type << " rank : " << req->rank << endl;
				}
			}
		}
	} else if(sched_policy == BACKLOG) {
		list<Request *>::iterator it = request_queue.begin();
		for(; it != request_queue.end(); it++) {
			Request *req = *it;

			// Check if its blocked too long
			// If YES schedule it instantly
			/* if((clock - req->start_time) > TIMEOUT) {
				if(req->type == 2) { 
					unsigned int type1_backlog = ranks[0][req->rank]->backlog(req->bank);
					unsigned int type2_backlog = ranks[1][req->rank]->backlog(req->bank);

					if(type1_backlog < type2_backlog) {
						ranks[0][req->rank]->addRequest(req);
						if(power_down_status[0][req->rank] == true) {
							ranks[0][req->rank]->powerUp();
						}
					} else {
						ranks[1][req->rank]->addRequest(req);
						if(power_down_status[1][req->rank] == true) {
							ranks[1][req->rank]->powerUp();
						}
					}
					mutual_request_counter[req->rank]--;
				} else {
					// cout << "Adding request at clock : " << clock << " to type : " << req->type << " rank : " << req->rank << " req : " << req << endl;
					ranks[req->type][req->rank]->addRequest(req);
					request_counter[req->type][req->rank] -= 1;
					if(power_down_status[req->type][req->rank] == true) {
						ranks[req->type][req->rank]->powerUp();
					}
				}
				request_queue.erase(it);
				break;
			} */

			if(req->type == 2) { 
				if(power_down_status[0][req->rank] == false && power_down_status[1][req->rank] == true) {
					ranks[0][req->rank]->addRequest(req);
					request_queue.erase(it);
					mutual_request_counter[req->rank]--;
					break;
				} else if(power_down_status[0][req->rank] == true && power_down_status[1][req->rank] == false) {
					ranks[1][req->rank]->addRequest(req);
					request_queue.erase(it);
					mutual_request_counter[req->rank]--;
					break;
				} else if(power_down_status[0][req->rank] == false && power_down_status[1][req->rank] == false) {
					unsigned int type1_backlog = ranks[0][req->rank]->backlog(req->bank);
					unsigned int type2_backlog = ranks[1][req->rank]->backlog(req->bank);

					if(type1_backlog < type2_backlog) {
						ranks[0][req->rank]->addRequest(req);
					} else {
						ranks[1][req->rank]->addRequest(req);
					}
					request_queue.erase(it);
					mutual_request_counter[req->rank]--;
					break;
				} else {
					// Both power down
					// Check for watermarks before scheduling
					if((ranks[0][req->rank]->totalBacklog() + request_counter[0][req->rank] + mutual_request_counter[req->rank]) >= PD_WM) {
						ranks[0][req->rank]->addRequest(req);
						ranks[0][req->rank]->powerUp();
						power_down_status[0][req->rank] = false;
						request_queue.erase(it);
						mutual_request_counter[req->rank]--;
						break;
					} else if((ranks[1][req->rank]->totalBacklog() + request_counter[1][req->rank] + mutual_request_counter[req->rank]) >= PD_WM) {
						ranks[1][req->rank]->addRequest(req);
						ranks[1][req->rank]->powerUp();
						power_down_status[1][req->rank] = false;
						request_queue.erase(it);
						mutual_request_counter[req->rank]--;
						break;
					}
				}
			} else { // Req is type 1 or 2
				// cout << "Clock : " << clock << " PD : " << power_down_status[req->type][req->rank] << " Backlog : " << (ranks[req->type][req->rank]->totalBacklog() + request_counter[req->type][req->rank]) << endl;
				if(power_down_status[req->type][req->rank] == false) {
					ranks[req->type][req->rank]->addRequest(req);
					request_counter[req->type][req->rank]--;
					request_queue.erase(it);
					break;
				} else if(power_down_status[req->type][req->rank] == true && (ranks[req->type][req->rank]->totalBacklog() + request_counter[req->type][req->rank]) >= PD_WM) {
					// cout << "Adding request at clock : " << clock << " to type : " << req->type << " rank : " << req->rank << " req : " << req << endl;
					ranks[req->type][req->rank]->addRequest(req);
					request_counter[req->type][req->rank]--;
					ranks[req->type][req->rank]->powerUp();
					power_down_status[req->type][req->rank] = false;
					// cout << "Clock : " << clock << " powering up type : " << req->type << " rank : " << req->rank << endl;
					request_queue.erase(it);
					break;
				}
			}
		}
	} else {
		cerr << "Incompatible scheduling policy\n\n";
		exit(1);
	}
}

void Controller::schedPowerDown() {
	if(pd_policy == NONE) {
	} else if(pd_policy == CONSERVATIVE) {
		for(int i=0; i < NUM_TYPES; i++) {
			for(int j=0; j < num_ranks; j++) {
				if(ranks[i][j]->totalBacklog() == 0 && request_counter[i][j] == 0 && mutual_request_counter[j] == 0) {
					ranks[i][j]->powerDown();
					// cout << "Clock : " << clock << " powering down type : " << i << " rank : " << j << endl;
					power_down_status[i][j] = true;
				}
			}
		}
	} else if(pd_policy == WATERMARK) {
		for(int i=0; i < NUM_TYPES; i++) {
			for(int j=0; j < num_ranks; j++) {
				if((ranks[i][j]->totalBacklog() + request_counter[i][j] + mutual_request_counter[j]) < PD_WM) {
					ranks[i][j]->powerDown();
					// cout << "Clock : " << clock << " powering down type : " << i << " rank : " << j << endl;
					power_down_status[i][j] = true;
				}
			}
		}
	} else {
		cerr << "Incompatible scheduling policy\n\n";
		exit(1);

	}
}

unsigned int Controller::totalAccess() {
	unsigned int total_access = 0;

	for(int i=0; i < NUM_TYPES; i++) {
		for(int j=0; j < num_ranks; j++) {
			unsigned int access_count = ranks[i][j]->numAccess();
			total_access += access_count;
		}
	}

	return total_access;
}

float Controller::avgLatency() {
	float total_latency = 0;
	unsigned int total_access = 0;

	for(int i=0; i < NUM_TYPES; i++) {
		for(int j=0; j < num_ranks; j++) {
			unsigned int access_count = ranks[i][j]->numAccess();
			total_access += access_count;
			total_latency += ranks[i][j]->avgLatency() * access_count;
		}
	}

	if(total_access == 0) return 0;

	float average_latency = total_latency / total_access;
	return average_latency;
}

float Controller::avgEnergy() {
	float total_energy = 0;
	unsigned int total_access = 0;

	for(int i=0; i < NUM_TYPES; i++) {
		for(int j=0; j < num_ranks; j++) {
			unsigned int access_count = ranks[i][j]->numAccess();
			total_access += access_count;
			total_energy += ranks[i][j]->avgEnergy() * access_count;
		}
	}

	if(total_access == 0) return 0;

	float average_energy = total_energy / total_access;
	return average_energy;
}

