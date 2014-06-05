/*
 * =====================================================================================
 *
 *       Filename:  hdram.cpp
 *
 *    Description:  Energy efficient scheduler simulation
 *
 *        Version:  1.0
 *        Created:  05/31/2014 09:34:36 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakesh Ramesh (Doctoral Candidate), rakeshr1@stanford.edu
 *      Execution:     
 *
 * =====================================================================================
 */

#include <iostream>
#include <cstdlib>
#include <cstring>
#include "controller.h"
#include "core.h"

using namespace std;

void print_help() {
	cout << "** Execution: ./hdram <Options>" << endl
		<< endl
		<< "** Options:" << endl
		<< "\t-t <Simulation time> (Default : 10000)" << endl
		<< "\t-r <Ranks> (Default : 4)" << endl
		<< "\t-b <Banks> (Default : 4)" << endl
		<< "\t-c <Cores> (Default : 4)" << endl
		<< "\t-x <Memory Intensity (MPKI)> (Default : 50)" << endl
		<< "\t-y <Type1 %> (Default : 50)" << endl
		<< "\t-z <Type2 %> (Default : 50)" << endl
		<< "\t-s <Sched Policy> (Default : 0)" << endl
		<< "\t-p <Power-Down Policy> (Default : 0)" << endl
		<< "\t-h or --help : Help screen" << endl
		<< endl
		;
}

void sim_need_argument(int argc, char **argv, int argi)
{
	
    if (argi == argc - 1)
        cerr << "Option '" << argv[argi] << "' requires one argument\n" << 
			"Please type './hdram --help' for help screen\n\n";
}

int main(int argc, char *argv[]) {
	cout << "\t\tHDRAM Simulator" << endl << endl;

	unsigned long int sim_time = 10000;
	unsigned int num_ranks=4, num_banks=4, num_cores=4;
	float mem_intensity=0.05, type1_intensity=0.5, type2_intensity=0.5;
	SchedPolicy sched_policy = FIFO;
	PDPolicy pd_policy = NONE;

	// Command line parsing
	for(int argi=1; argi < argc; argi++) {
		if(!strcmp(argv[argi], "-h") || !strcmp(argv[argi], "--help")) {
			print_help();
			return 1;
		}

		if(!strcmp(argv[argi], "-t")) {
			sim_need_argument(argc, argv, argi);
			argi++;
			sim_time = atoi(argv[argi]);
			continue;
		}

		if(!strcmp(argv[argi], "-r")) {
			sim_need_argument(argc, argv, argi);
			argi++;
			num_ranks = atoi(argv[argi]);
			continue;
		}

		if(!strcmp(argv[argi], "-b")) {
			sim_need_argument(argc, argv, argi);
			argi++;
			num_banks = atoi(argv[argi]);
			continue;
		}

		if(!strcmp(argv[argi], "-c")) {
			sim_need_argument(argc, argv, argi);
			argi++;
			num_cores = atoi(argv[argi]);
			continue;
		}

		if(!strcmp(argv[argi], "-x")) {
			sim_need_argument(argc, argv, argi);
			argi++;
			mem_intensity = atoi(argv[argi])/1000.0;
			continue;
		}

		if(!strcmp(argv[argi], "-y")) {
			sim_need_argument(argc, argv, argi);
			argi++;
			type1_intensity = atoi(argv[argi])/100.0;
			continue;
		}

		if(!strcmp(argv[argi], "-z")) {
			sim_need_argument(argc, argv, argi);
			argi++;
			type2_intensity = atoi(argv[argi])/100.0;
			continue;
		}

		if(!strcmp(argv[argi], "-s")) {
			sim_need_argument(argc, argv, argi);
			argi++;
			sched_policy = (SchedPolicy) atoi(argv[argi]);
			continue;
		}

		if(!strcmp(argv[argi], "-p")) {
			sim_need_argument(argc, argv, argi);
			argi++;
			pd_policy = (PDPolicy) atoi(argv[argi]);
			continue;
		}

        /*  Invalid option */
		if (argv[argi][0] == '-') {
			cout << "'" << argv[argi] << "' is not a valid command-line option.\n"
				<< "Please type './hdram --help' for help screen\n\n";
		}
	}

	// Simulator initialization
	Controller *controller = new Controller(num_ranks, num_banks, sched_policy, pd_policy);

	Core **cores = new Core *[num_cores];
	for(int i=0; i < num_cores; i++) {
		cores[i] = new Core(controller, mem_intensity, type1_intensity, type2_intensity,
				num_cores, num_banks);
	}

	// Simulation Loop
	for(unsigned long int cycle=0; cycle < sim_time; cycle++) {
		for(int i=0; i < num_cores; i++) {
			cores[i]->clockTick();
		}

		controller->clockTick();

		// Heartbeat
		if(cycle % (sim_time / 10) == 0) {
			cout << "cycle : " << cycle << endl;
		}
	}

	for(unsigned long int cycle=sim_time; cycle < 3*sim_time; cycle++) {
		controller->clockTick();

		// Heartbeat
		if(cycle % (sim_time/10) == 0) {
			cout << "cycle : " << cycle << endl;
		}
	}

	unsigned int total_access = controller->totalAccess();
	float avg_latency = controller->avgLatency();
	float avg_energy = controller->avgEnergy();
	cout << "Total Access : " << total_access << endl;
	cout << "Average Latency : " << avg_latency << endl;
	cout << "Average Energy : " << avg_energy << endl;
	cout << "E-D Product : " << (avg_latency * avg_energy) << endl;

	// Free heap
	for(int i=0; i < num_cores; i++) {
		delete cores[i];
	}
	delete [] cores;
	delete controller;

	return 0;
}
