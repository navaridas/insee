/**
* @file
* The spinnaker topology tools.
*/

/*
FSIN Functional Simulator of Interconnection Networks
Copyright (2003-2005) J. Miguel-Alonso, A. Gonzalez

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
* Obtains a neighbor node in the spinnaker topology.
*
* Given a node address "ad", a direction "wd" (U, V or W) and a way "ww" (+ or -)
* returns the address of the neighbor in that direction and way; only valid for torus
* but usable also for mesh.
* @param ad A node address.
* @param wd A dimension (X,Y or Z).
* @param ww A way (UP or DOWN).
* @return The address of the neighbor in that direction.
*/
long spinnaker_neighbor (long ad, dim wd, way ww) {
	long ox, oy;
	long nx, ny;
	long w;

	if (ww == UP)
		w = 1;
	else
		w = -1;

	ox=network[ad].rcoord[D_X];
	oy=network[ad].rcoord[D_Y];

	switch (wd) {
		case D_X:		// U :: x
			nx=mod(ox+w,nodes_x);
			ny=oy;
			break;
		case D_Y:		// V :: y
			nx=ox;
			ny=mod(oy+w,nodes_y);
			break;
		case D_Z:		// W :: x & y
			nx=mod(ox+w,nodes_x);
			ny=mod(oy+w,nodes_y);
			break;
		case INJ:
		case CON:
		default:
		    nx=ny=-1;
	}
	return address(nx,ny,0);
}

/**
* Generates the routing record for the spinnaker topology.
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record needed to go from source to destination.
*/
routing_r spinnaker_rr (long source, long destination) {
    static long sx, sy, dx, dy, Ax1, Ax2, Ay1, Ay2;
    static long rx, ry, rz, dist;
    routing_r res;

    res.rr=alloc(ndim*sizeof(long));

    if (source == destination)
       panic("Self-sent packet");

    sx=network[source].rcoord[D_X];
    sy=network[source].rcoord[D_Y];

    dx=network[destination].rcoord[D_X];
    dy=network[destination].rcoord[D_Y];

    // distance in each axis
    Ax1=dx-sx;
    Ax2=-1*sign(Ax1)*(nodes_x-abs(Ax1));

    Ay1=dy-sy;
    Ay2=-1*sign(Ay1)*(nodes_y-abs(Ay1));

    // all routing possibilities are calculated here. the best one is selected.

    res.rr[D_Z]=Ay1;
    res.rr[D_X]=Ax1-Ay1;
    res.rr[D_Y]=0;
    res.size = abs(res.rr[D_X]) + abs(res.rr[D_Y]) + abs(res.rr[D_Z]);

    rz=Ax1;
    ry=Ay1-Ax1;
    rx=0;
    dist = abs(rx)+abs(ry)+abs(rz);
    if (dist<res.size){
        res.rr[D_X]=rx;
        res.rr[D_Y]=ry;
        res.rr[D_Z]=rz;
        res.size=dist;
    }

    rx=Ax1;
    ry=Ay1;
    rz=0;
    dist = abs(rx)+abs(ry)+abs(rz);
    if (dist<res.size){
        res.rr[D_X]=rx;
        res.rr[D_Y]=ry;
        res.rr[D_Z]=rz;
        res.size=dist;
    }

    rz=Ay2;
    rx=Ax1-Ay2;
    ry=0;
    dist = abs(rx)+abs(ry)+abs(rz);
    if (dist<res.size){
        res.rr[D_X]=rx;
        res.rr[D_Y]=ry;
        res.rr[D_Z]=rz;
        res.size=dist;
    }

    rz=Ax1;
    ry=Ay2-Ax1;
    rx=0;
    dist = abs(rx)+abs(ry)+abs(rz);
    if (dist<res.size){
        res.rr[D_X]=rx;
        res.rr[D_Y]=ry;
        res.rr[D_Z]=rz;
        res.size=dist;
    }

    rx=Ax1;
    ry=Ay2;
    rz=0;
    dist = abs(rx)+abs(ry)+abs(rz);
    if (dist<res.size){
        res.rr[D_X]=rx;
        res.rr[D_Y]=ry;
        res.rr[D_Z]=rz;
        res.size=dist;
    }

    rz=Ay1;
    rx=Ax2-Ay1;
    ry=0;
    dist = abs(rx)+abs(ry)+abs(rz);
    if (dist<res.size){
        res.rr[D_X]=rx;
        res.rr[D_Y]=ry;
        res.rr[D_Z]=rz;
        res.size=dist;
    }

    rz=Ax2;
    ry=Ay1-Ax2;
    rx=0;
    dist = abs(rx)+abs(ry)+abs(rz);
    if (dist<res.size){
        res.rr[D_X]=rx;
        res.rr[D_Y]=ry;
        res.rr[D_Z]=rz;
        res.size=dist;
    }

    rx=Ax2;
    ry=Ay1;
    rz=0;
    dist = abs(rx)+abs(ry)+abs(rz);
    if (dist<res.size){
        res.rr[D_X]=rx;
        res.rr[D_Y]=ry;
        res.rr[D_Z]=rz;
        res.size=dist;
    }

    rz=Ay2;
    rx=Ax2-Ay2;
    ry=0;
    dist = abs(rx)+abs(ry)+abs(rz);
    if (dist<res.size){
        res.rr[D_X]=rx;
        res.rr[D_Y]=ry;
        res.rr[D_Z]=rz;
        res.size=dist;
    }

    rz=Ax2;
    ry=Ay2-Ax2;
    rx=0;
    dist = abs(rx)+abs(ry)+abs(rz);
    if (dist<res.size){
        res.rr[D_X]=rx;
        res.rr[D_Y]=ry;
        res.rr[D_Z]=rz;
        res.size=dist;
    }

    rx=Ax2;
    ry=Ay2;
    rz=0;
    dist = abs(rx)+abs(ry)+abs(rz);
    if (dist<res.size){
        res.rr[D_X]=rx;
        res.rr[D_Y]=ry;
        res.rr[D_Z]=rz;
        res.size=dist;
    }
    return res;
}
