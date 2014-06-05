/*
 * =====================================================================================
 *
 *       Filename:  core.h
 *
 *    Description:  Dummy core for generating requests
 *
 *        Version:  1.0
 *        Created:  05/31/2014 10:32:03 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakesh Ramesh (Doctoral Candidate), rakeshr1@stanford.edu
 *      Execution:     
 *
 * =====================================================================================
 */

#ifndef _CORE_H_
#define _CORE_H_

#include "request.h"
#include "controller.h"

class Core {
private:
	Controller *controller;
	unsigned long int clock;

	// Config
	float mem_intensity;
	float type1_intensity;
	float type2_intensity;
	float type12_intensity;

	unsigned int num_ranks;
	unsigned int num_banks;

	// Stats
	unsigned int num_access;

public:
	Core(Controller *controller_, float mem_intensity_, float type1_intensity_, float type2_intensity_,
			unsigned int num_ranks_, unsigned int num_banks_);
	~Core();

	void clockTick();
};

#endif
