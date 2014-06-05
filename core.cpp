/*
 * =====================================================================================
 *
 *       Filename:  core.cpp
 *
 *    Description:  Dummy core for generating requests
 *
 *        Version:  1.0
 *        Created:  05/31/2014 10:42:07 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakesh Ramesh (Doctoral Candidate), rakeshr1@stanford.edu
 *      Execution:     
 *
 * =====================================================================================
 */

#include <cstdlib>
#include <time.h>

#include "core.h"

Core::Core(Controller *controller_, float mem_intensity_, float type1_intensity_, float type2_intensity_,
		unsigned int num_ranks_, unsigned int num_banks_) {
	controller = controller_;
	mem_intensity = mem_intensity_;
	type1_intensity = type1_intensity_;
	type2_intensity = type2_intensity_;
	type12_intensity = 1 - type1_intensity - type2_intensity;

	num_ranks = num_ranks_;
	num_banks = num_banks_;

	clock = 0;
	srand(time(NULL));
}

Core::~Core() {
}

void Core::clockTick() {
	float prob = rand() / float(RAND_MAX);
	if(prob < mem_intensity) {
		Request *req = new Request;
		req->start_time = clock;

		req->rank = rand() % num_ranks;
		req->bank = rand() % num_banks;

		float type_prob = rand() / float(RAND_MAX);
		if(type_prob < type1_intensity) {
			req->type = 0;
		} else if(type_prob < (type1_intensity + type2_intensity)) {
			req->type = 1;
		} else {
			req->type = 2;
		}

		controller->addRequest(req);
	}

	clock++;
}

