/*
 * =====================================================================================
 *
 *       Filename:  request.cpp
 *
 *    Description:  Request structure
 *
 *        Version:  1.0
 *        Created:  06/01/2014 12:17:51 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakesh Ramesh (Doctoral Candidate), rakeshr1@stanford.edu
 *      Execution:     
 *
 * =====================================================================================
 */

#include "request.h"

ostream &operator<<(ostream &out, Request &req) {
	out << "Type: " << req.type 
		<< " Rank: " << req.rank
		<< " Bank: " << req.bank
		<< " Start_time: " << req.start_time;
	return out;
}


