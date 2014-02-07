/**
* @file
* @brief	The indirect cube topology tools.
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
#include "misc.h"
#include "router.h"

long nodes_per_switch;		///< The number of nodes attached to each switch.
long links_per_direction;	///< The number of parallel links in each direction.


/**
* Generates the routing record for an indirect cube  using adaptive routing.
* 
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record needed to go from source to destination.
*/
routing_r icube_rr (long source, long destination) {
	long i, j, k, nhops=1;
	long sx,sy,sz, dx,dy,dz, numhops;
	routing_r res;
	
	res.rr=alloc(ndim*sizeof(long));
	res.size=2;	// 2 hops: From NIC to first switch + From last switch to NIC.

	sx=network[source].rcoord[D_X];
	dx=network[destination].rcoord[D_X];

	if (ndim>1){
		sy=network[source].rcoord[D_Y];
		dy=network[destination].rcoord[D_Y];
	}
	if (ndim>2){
		sz=network[source].rcoord[D_Z];
		dz=network[destination].rcoord[D_Z];
	}

	res.rr[D_X] = (dx-sx)%nodes_x;
	if (res.rr[D_X] < 0)
		res.rr[D_X] += nodes_x;
	if (res.rr[D_X] > nodes_x/2)
		res.rr[D_X] = (nodes_x-res.rr[D_X])*(-1);
	if ((double)res.rr[D_X] == nodes_x/2.0)
		if (rand() >= (RAND_MAX/2))
			res.rr[D_X] = (nodes_x-res.rr[D_X])*(-1);
	res.size+=abs(res.rr[D_X]);

	if (ndim>1) {
		res.rr[D_Y] = (dy-sy)%nodes_y;
		if (res.rr[D_Y] < 0)
			res.rr[D_Y] += nodes_y;
		if (res.rr[D_Y] > nodes_y/2)
			res.rr[D_Y] = (nodes_y-res.rr[D_Y])*(-1);
		if ((double)res.rr[D_Y] == nodes_y/2.0)
			if (rand() >= (RAND_MAX/2))
				res.rr[D_Y] = (nodes_y-res.rr[D_Y])*(-1);
		res.size+=abs(res.rr[D_Y]);
	}

	if (ndim>2) {
		res.rr[D_Z] = (dz-sz)%nodes_z;
		if (res.rr[D_Z] < 0)
			res.rr[D_Z] += nodes_z;
		if (res.rr[D_Z] > nodes_z/2)
			res.rr[D_Z] = (nodes_z-res.rr[D_Z])*(-1);
		if ((double)res.rr[D_Z] == nodes_z/2.0)
			if (rand() >= (RAND_MAX/2))
				res.rr[D_Z] = (nodes_z-res.rr[D_Z])*(-1);
		res.size+=abs(res.rr[D_Z]);
	}

	return res;
}

/**
* Generates the routing record for an indirect cube using static routing (no Bubble).
* Deadlock is avoided by considering four isolated meshes and using DOR in each
* of them. Origins are in nodes (0,0) (X/2, 0) (0, Y/2) and (X/2, Y/2) repectively
* for each of the meshes. Routing selects the mesh with shorter distance.
* 
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record needed to go from source to destination.
*/
routing_r icube_4mesh_rr (long source, long destination) {
	long i, j, k, nhops=1;
	long sx,sy,sz, dx,dy,dz, p, numhops;
	routing_r res;

	res.rr=alloc(ndim+1*sizeof(long));
	res.rr[ndim]=0;
	
	res.size=2;	// 2 hops: From NIC to first switch + From last switch to NIC.

	sx=network[source].rcoord[D_X];
	dx=network[destination].rcoord[D_X];

	p= (source % nodes_per_switch) % links_per_direction;
	
	if (ndim>1){
		sy=network[source].rcoord[D_Y];
		dy=network[destination].rcoord[D_Y];
	}
	if (ndim>2){
		panic("4mesh only defined for 2D icubes");
	}

	res.rr[D_X] = (dx-sx)%nodes_x;
	if (res.rr[D_X] < 0)
		res.rr[D_X] += nodes_x;
	if (res.rr[D_X] > nodes_x/2)
		res.rr[D_X] = (nodes_x-res.rr[D_X])*(-1);
	if ((double)res.rr[D_X] == nodes_x/2.0)
		if (p%2)
			res.rr[D_X] = (nodes_x-res.rr[D_X])*(-1);
 	res.size+=abs(res.rr[D_X]);
 	
	if ((sx+res.rr[D_X]>=nodes_x)||(sx+res.rr[D_X]<0))
		res.rr[ndim]=1;
	else if ((2*sx/nodes_x)==(2*dx/nodes_x))
	    res.rr[ndim]=p%2;
	else
	    res.rr[ndim]=0;
	if (ndim>1) {
		res.rr[D_Y] = (dy-sy)%nodes_y;
		if (res.rr[D_Y] < 0)
			res.rr[D_Y] += nodes_y;
		if (res.rr[D_Y] > nodes_y/2)
			res.rr[D_Y] = (nodes_y-res.rr[D_Y])*(-1);
		if ((double)res.rr[D_Y] == nodes_y/2.0)
			if ((p/2)%2)
				res.rr[D_Y] = (nodes_y-res.rr[D_Y])*(-1);
		res.size+=abs(res.rr[D_Y]);
		
		if ((sy+res.rr[D_Y]>=nodes_y)||(sy+res.rr[D_Y]<0))
			res.rr[ndim]+=2;
		else if ((2*sy/nodes_y)==(2*dy/nodes_y))
		    res.rr[ndim]+=((p/2)%2)*2;
		else
		    res.rr[ndim]+=0;
	}
	return res;
}

/**
* Generates the routing record for an indirect cube using static routing (no Bubble).
* Deadlock is avoided by considering a collection of isolated meshes and using
* DOR in each of them. All the  meshes have their origin in node (0,0).
* 
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record needed to go from source to destination.
*/
routing_r icube_1mesh_rr (long source, long destination) {
	long i, j, k, nhops=1;
	long sx,sy,sz, dx,dy,dz, numhops;
	routing_r res;

	res.rr=alloc(ndim+1*sizeof(long)); // the last dimension is the number of parallel mesh to be used.
	res.size=2;	// 2 hops: From NIC to first switch + From last switch to NIC.

	sx=network[source].rcoord[D_X];
	dx=network[destination].rcoord[D_X];

	if (ndim>1){
		sy=network[source].rcoord[D_Y];
		dy=network[destination].rcoord[D_Y];
	}
	if (ndim>2){
		panic("1mesh only defined for 2D indirect cube");
	}

	res.rr[D_X] = (dx-sx)%nodes_x;
	res.size+=abs(res.rr[D_X]);

	if (ndim>1) {
		res.rr[D_Y] = (dy-sy)%nodes_y;
		res.size+=abs(res.rr[D_Y]);
	}

	if (ndim>2) {
		res.rr[D_Z] = (dz-sz)%nodes_z;
		res.size+=abs(res.rr[D_Z]);
	}

	res.rr[ndim] = (source % nodes_per_switch) % links_per_direction; // Last dimension. Stores the number of parallel mesh.
	return res;
}

/**
* Creates an indirect cube topology.
*
* This function links all the elements within the network.
*/
void create_icube(){
	long i, j, nr, np, p, k, n,m, i_np;

	for (i=0; i<nprocs; i++){
		for (j=0; j<ninj; j++)
			inj_init_queue(&network[i].qi[j]);
		init_ports(i);
		nr=(i/nodes_per_switch)+nprocs;
		np=i%nodes_per_switch;
		network[i].nbor[0] = nr;
		network[i].nborp[0] = np;
		network[i].op_i[0] = ESCAPE;
		network[nr].nbor[np] = i;
		network[nr].nborp[np] = 0;
		network[nr].op_i[np] = ESCAPE;

		for (j=1; j<radix; j++) {
			network[i].nbor[j] = NULL_PORT;
			network[i].nborp[j] = NULL_PORT;
			network[i].op_i[j] = ESCAPE;
		}
	}

	for (i=0; i < (nodes_x*nodes_y*nodes_z); i++){
		i_np=i+nprocs;
		init_ports(i_np);
		for (j=0; j<ndim; j++) { // X, Y, Z
			for (k=0; k<nways; k++){ // +, -
				nr=neighbor(i_np, j, k)+ nprocs;
				for (n=0; n<links_per_direction; n++){ // 0 1 2
					p  = nodes_per_switch + (links_per_direction*((2*j)+k)) +n;
					np = nodes_per_switch + (links_per_direction*((2*j)+1-k)) +n;
					network[i_np].nborp[p] = np;
					network[i_np].nbor[p] = nr;
					network[i_np].op_i[p] = ESCAPE;
				}
			}
		}
	}
}
