/**
* @file
* @brief	k-ary n-tree topology tools.
*
*@author Javier Navaridas

FSIN Functional Simulator of Interconnection Networks
Copyright (2003-2011) J. Miguel-Alonso, J. Navaridas

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "globals.h"

/**
* Creates a fat tree topology.
*
* This function defines all the links between the elements in the network.
*/
void create_fattree(){
	long i, j, nr, np;	//neighbor router and port.
	long st, r, p;		//current stage, router and port.
	long st_first, next_st_first;	//if for this and next stage's first element(switch).
	long k, ks, ks1, sg;	//auxiliar variables for saving cpu and reducing complexity.
	long router_per_stage;	// Total Number of routers in each stage of a multistage topology.

	k=radix/2;
	router_per_stage=(long)pow(k, nstages-1);

	// Initializating processors. Only 1 transit port plus injection queues.
	for (i=0; i<nprocs; i++){
		network[i].rcoord[STAGE]=-1;
		network[i].rcoord[POSITION]=i;
		for (j=0; j<ninj; j++)
			inj_init_queue(&network[i].qi[j]);
		init_ports(i);
		nr=(i/k)+nprocs;
		np=(i%k)+k;
		network[i].nbor[0] = nr;
		network[i].nborp[0] = np;
		network[i].op_i[0] = ESCAPE;
		network[nr].nbor[np] = i;
		network[nr].nborp[np] = 0;
		network[nr].op_i[np] = ESCAPE;
		for (j=1; j<radix; j++){
			network[i].nbor[j] = NULL_PORT;
			network[i].nborp[j] = NULL_PORT;
			network[i].op_i[j] = ESCAPE;
		}
	}
	next_st_first=nprocs;
	ks=1;	// k ^ stage
	ks1=k;	// k ^ stage+1

	// Initializing switches. No injection queues needed.
	for (st=0; st< nstages-1; st++ ){
		st_first = next_st_first;						//first swith in this stage
		next_st_first = st_first + router_per_stage;	//first switch in the next stage
		for (r=0; r < router_per_stage; r++ ){
			i = r + st_first;
			network[i].rcoord[STAGE]=st;
			network[i].rcoord[POSITION]=r;
			init_ports(i);
			sg = r/(ks1);
			for (p=0; p< k; p++) {
				nr = mod(((p*ks) + (ks1*sg)+(r%ks)), router_per_stage) + next_st_first;
				np = mod(((r/ks)-(ks1*sg)), k) + k;

				network[i].nbor[p]=nr;			// neighbor router
				network[i].nborp[p]=np;			// neighbor's port
				network[i].op_i[p] = ESCAPE;
				network[nr].nbor[np]=i;			// neighbor router's neighbor
				network[nr].nborp[np]=p;		// neighbor router's neighbor's port
				network[nr].op_i[np] = ESCAPE;
			}
		}
		ks=ks1;
		ks1=ks1*k;
	}
	st_first=next_st_first;

	// last stage routers' neighbor is the same router
	for (r=0; r< router_per_stage; r++ ){
		i = r + st_first;
		network[i].rcoord[STAGE]=st;
		network[i].rcoord[POSITION]=r;
		init_ports(i);
		for (p=0; p< k; p++) {
			network[i].nbor[p] = NULL_PORT;	// neighbor router
			network[i].nborp[p] = NULL_PORT;// neighbor's port
			network[i].op_i[p] = NULL_PORT;
		}
	}
}

/**
* Creates a thin tree topology.
*
* This function defines all the links between the elements in the network.
*/
void create_thintree()
{
	long i, j, nr, np;	//neighbor router and port.
	long st, r, p;	//current stage, router and port.
	long st_first, next_st_first;	//if for this and next stage's first element(switch).
	long sgUp;		// stUp ^ st
	long router_per_stage;		// Total Number of routers in each stage of a multistage topology.

	// Processors
	for (i=0; i<nprocs; i++){
		network[i].rcoord[STAGE]=-1;
		network[i].rcoord[POSITION]=i;
		for (j=0; j<ninj; j++) inj_init_queue(&network[i].qi[j]);
		init_ports(i);
		nr=(i/stDown)+nprocs;
		np=(i%stDown)+stUp;
		network[i].nbor[0] = nr;
		network[i].nborp[0] = np;
		network[i].op_i[0] = ESCAPE;
		network[nr].nbor[np] = i;
		network[nr].nborp[np] = 0;
		network[nr].op_i[np] = ESCAPE;
		for (j=1; j<radix; j++){
			network[i].nbor[j] = NULL_PORT;
			network[i].nborp[j] = NULL_PORT;
			network[i].op_i[j] = ESCAPE;
		}
	}

	next_st_first=nprocs;
	sgUp=1;		//stUp ^ st
	router_per_stage=(long)pow(stDown,nstages-1);
		// Initializing switches. No injection queues needed.
	for (st=0; st< nstages-1; st++ ){
		st_first = next_st_first;						//first swith in this stage
		next_st_first = st_first + router_per_stage;	//first switch in the next stage
		for (r=0; r < router_per_stage; r++ ){
			i = r + st_first;
			network[i].rcoord[STAGE]=st;
			network[i].rcoord[POSITION]=r;
			init_ports(i);
			for (p=0; p< stUp; p++) {
				nr = (p*sgUp) + (r%sgUp) + ((r/(stDown*sgUp))*sgUp*stUp) + next_st_first;
				np = ((r/sgUp)%stDown)+ stUp;
				network[i].nbor[p]=nr;		// neighbor router
				network[i].nborp[p]=np;		// neighbor's port
				network[i].op_i[p] = ESCAPE;
				network[nr].nbor[np]=i;		// neighbor router's neighbor
				network[nr].nborp[np]=p;	// neighbor router's neighbor's port
				network[nr].op_i[np] = ESCAPE;
			}
		}
		sgUp = sgUp * stUp;
		router_per_stage = (stUp * router_per_stage)/stDown;
	}
	st_first=next_st_first;
	// last stage routers' neighbor is the same router
	for (r=0; r< router_per_stage; r++ ){
		i = r + st_first;
		network[i].rcoord[STAGE]=st;
		network[i].rcoord[POSITION]=r;
		init_ports(i);
		for (p=0; p< stUp; p++) {
			network[i].nbor[p] = NULL_PORT;	// neighbor router
			network[i].nborp[p] = NULL_PORT;// neighbor's port
			network[i].op_i[p] = NULL_PORT;
		}
	}
}

/**
* Creates a slim tree topology.
*
* This function defines all the links between the elements in the network.
*/
void create_slimtree(){
	long i, j, nr, np;	//neighbor router and port.
	long st, r, p;					//current stage, router and port.
	long st_first, next_st_first;	//if for this and next stage's first element(switch).
	long router_per_stage,
	     router_per_next_stage;		// Total Number of routers in each stage of a multistage topology.

	for (i=0; i<nprocs; i++){
		for (j=0; j<ninj; j++) inj_init_queue(&network[i].qi[j]);
		network[i].rcoord[STAGE]=-1;
		network[i].rcoord[POSITION]=i;
		init_ports(i);
		nr=(i/stDown)+nprocs;
		np=(i%stDown)+stUp;
		network[i].nbor[0] = nr;
		network[i].nborp[0] = np;
		network[i].op_i[0] = ESCAPE;
		network[nr].nbor[np] = i;
		network[nr].nborp[np] = 0;
		network[nr].op_i[np] = ESCAPE;
		for (j=1; j<radix; j++){
			network[i].nbor[j] = NULL_PORT;
			network[i].nborp[j] = NULL_PORT;
			network[i].op_i[j] = ESCAPE;
		}
	}

	router_per_stage=(long)pow((stDown/stUp),nstages-1)*stUp;
	router_per_next_stage=(router_per_stage*stUp)/stDown;
	st_first=nprocs;
	next_st_first=st_first+router_per_stage;

	for (st=0; st<nstages-1; st++){
		for (r=0; r<router_per_stage;r++){
			i = r + st_first;
			network[i].rcoord[STAGE]=st;
			network[i].rcoord[POSITION]=r;
			init_ports(i);

			for (p=0; p<stUp; p++){
				nr=stUp*(r/stDown)+p+next_st_first;
				np=r%stDown+stUp;
				network[i].nbor[p]=nr;
				network[i].nborp[p]=np;
				network[nr].nbor[np]=i;
				network[nr].nborp[np]=p;
			}
		}
		router_per_stage=router_per_next_stage;
		router_per_next_stage=(router_per_next_stage*stUp)/stDown;
		st_first=next_st_first;
		next_st_first=st_first + router_per_stage;
    }

	// last stage routers' neighbor is the same router
	for (r=0; r< router_per_stage; r++ ){
		i = r + st_first;
		network[i].rcoord[STAGE]=st;
		network[i].rcoord[POSITION]=r;
		init_ports(i);
		for (p=0; p< stUp; p++) {
			network[i].nbor[p] = NULL_PORT;	// neighbor router
			network[i].nborp[p] = NULL_PORT;// neighbor's port
			network[i].op_i[p] = NULL_PORT;
		}
	}
}

/**
* Generates the routing record for a k-ary n-tree.
*
* This function allows adaptive routing because no route is defined here.
*
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record needed to go from source to destination.
*/
routing_r fattree_rr_adapt (long source, long destination) {
	long nhops=1,	// Number of hops
		k=radix/2,	// Number of ports
		k_n=k;		// k ^ nhops
	routing_r res;
	if (source == destination)
		panic("Self-sent packet");

	// Search the first common ancester
	while (source / k_n != destination / k_n){
		nhops++;
		k_n=k_n*k;
	}
	res.rr=NULL;
	res.size=nhops*2;
	return res;
}

/**
* Generates the routing record for a thin tree.
*
* This function allows adaptive routing because no route is defined here.
*
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record needed to go from source to destination.
*/
routing_r thintree_rr_adapt (long source, long destination) {
	long nhops=1, sgDown=stDown;
	routing_r res;

	if (source == destination)
		panic("Self-sent packet");

	// Search the first common ancester
	while (source / (sgDown) != destination / (sgDown)){
		nhops++;
		sgDown=sgDown*stDown;
	}

	res.rr=NULL;
	res.size=nhops*2;
	return res;
}

/**
* Generates the routing record for a slimtree.
*
* This function allows adaptive routing because no route is defined here.
*
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record needed to go from source to destination.
*/
routing_r slimtree_rr_adapt (long source, long destination) {
	long nhops=1,	// Number of hops
		sgDown,		// stDown ^ nhops
		sgUp;		// StUp ^ nhops -2
	routing_r res;

	if (source == destination)
		panic("Self-sent packet");

	// Search the first common ancester
	if (source/ stDown != destination /stDown){
		nhops=2;
		sgDown=stDown*stDown;
		sgUp=1;
		while ((sgUp*source)/sgDown != ((sgUp*destination)/sgDown)){
			nhops++;
			sgDown=sgDown*stDown;
			sgUp=sgUp*stUp;
		}
	}
	res.rr=NULL;
	res.size=nhops*2;
	return res;
}

