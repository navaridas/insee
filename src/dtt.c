/**
* @file
* @brief	The Twisted Torus topology tools.

FSIN Functional Simulator of Interconnection Networks
Copyright (2003-2011) J. Miguel-Alonso, A. Gonzalez, J. Navaridas, J.M. Camara

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

long sk_xy;	///< Skew from X to Y
long sk_xz;	///< Skew from X to Z
long sk_yx;	///< Skew from Y to X
long sk_yz;	///< Skew from Y to Z
long sk_zx;	///< Skew from Z to X
long sk_zy;	///< Skew from Z to Y


/**
* Gets a neighbor in a given direction.
*
* @param ad A node address.
* @param wd A dimension (X,Y or Z).
* @param ww A way (UP or DOWN).
* @return The address of the neighbor in that dimension and way; only valid for ttorus
*/
long dtt_neighbor (long ad, dim wd, way ww){
	long ox, oy, oz;
	long nx, ny, nz;
	long w;

	nx = ny = nz = 0;

	if (ww == UP)
		w = 1;
	else
		w = -1;

	ox=network[ad].rcoord[D_X];
	oy=network[ad].rcoord[D_Y];
	oz=network[ad].rcoord[D_Z];

	switch (wd) {
	case D_X:
		nx = (ox+w);
		if (nx >= nodes_x){
			nx = (ox+w)%(nodes_x);
			ny = (oy + sk_xy)%(nodes_y);
			nz = (oz + sk_xz)%(nodes_z);
		}
		else if (nx < 0){
			nx += nodes_x;
			ny = oy - sk_xy;
			if (ny < 0) ny += nodes_y;
			nz = oz - sk_xz;
			if (nz < 0) nz += nodes_z;
		}
		else{
			ny = oy;
			nz = oz;
		}
		break;

	case D_Y:
		ny = (oy+w);
		if (ny >= nodes_y){
			ny = (oy+w)%(nodes_y);
			nx = (ox + sk_yx)%(nodes_x);
			nz = (oz + sk_yz)%(nodes_z);
		}
		else if (ny < 0){
			ny += nodes_y;
			nx = ox - sk_yx;
			if (nx < 0) nx += nodes_x;
			nz = oz - sk_yz;
			if (nz < 0) nz += nodes_z;
		}
		else{
			nx = ox;
			nz = oz;
		}
		break;

	case D_Z:
		nz = (oz+w);
		if (nz >= nodes_z){
			nz = (oz+w)%(nodes_z);
			nx = (ox + sk_zx)%(nodes_x);
			ny = (oy + sk_zy)%(nodes_y);
		}
		else if (nz < 0){
			nz += nodes_z;
			nx = ox - sk_zx;
			if (nx < 0) nx += nodes_x;
			ny = oy - sk_zy;
			if (ny < 0) ny += nodes_y;
		}
		else{
			nx = ox;
			ny = oy;
		}
		break;

	default:;

	}
	return address(nx, ny, nz);
}

/**
* Generates the routing record for a twisted torus.
*
* This function considers three cases according to the
* different options of the target dimension of the skews.
* Z dimension will act as default in case no skew (torus) is considered.
* 
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record needed to go from source to destination.
*/
routing_r dtt_rr (long source, long destination) {
	long sx, sy, sz, dx, dy, dz;
	static long rr_x[6], rr_y[6], rr_z[6], min, num, bet;
	long mesh_x, mesh_y, mesh_z, wrapx_x, wrapx_y, wrapx_z,wrapy_x, wrapy_y, wrapy_z,wrapz_x, wrapz_y, wrapz_z;
	routing_r res;

	res.rr=alloc(ndim*sizeof(long));

	if (source == destination)
		panic("Self-sent message");

	sx=network[source].rcoord[D_X];
	sy=network[source].rcoord[D_Y];
	sz=network[source].rcoord[D_Z];

	dx=network[destination].rcoord[D_X];
	dy=network[destination].rcoord[D_Y];
	dz=network[destination].rcoord[D_Z];

	if ((sk_yx !=0) || (sk_zx !=0)){
		// mesh
		mesh_x = (dx-sx)%nodes_x; if (mesh_x < 0) mesh_x += nodes_x;
		if (mesh_x > nodes_x/2) mesh_x = (nodes_x-mesh_x)*(-1);
		if ((double)mesh_x == nodes_x/2.0)
			if (rand() >= (RAND_MAX/2)) mesh_x = (nodes_x-mesh_x)*(-1);
		mesh_y = dy - sy;
		mesh_z = dz - sz;

		rr_x[0] = mesh_x;
		rr_y[0] = mesh_y;
		rr_z[0] = mesh_z;
		num=1;
		min=(labs(rr_x[0]) + labs(rr_y[0]) + labs(rr_z[0]));

		// wraparround y
		wrapy_y = -sign(mesh_y)*(nodes_y - labs(mesh_y));
		wrapy_x = (sx - sign(mesh_y)*sk_yx)%nodes_x;
		if (wrapy_x < 0) wrapy_x += nodes_x;

		wrapy_x = (dx-wrapy_x)%nodes_x;
		if (wrapy_x < 0) wrapy_x += nodes_x;
		if (wrapy_x > nodes_x/2) wrapy_x = (nodes_x-wrapy_x)*(-1);
		if ((double)wrapy_x == nodes_x/2.0)
			if (rand() >= (RAND_MAX/2)) wrapy_x = (nodes_x-wrapy_x)*(-1);

		wrapy_z = mesh_z;

		if ((labs(wrapy_x) + labs(wrapy_y) + labs(wrapy_z)) < min){
			rr_x[0] = wrapy_x;
			rr_y[0] = wrapy_y;
			rr_z[0] = wrapy_z;
			num=1;
			min=(labs(rr_x[0]) + labs(rr_y[0]) + labs(rr_z[0]));
		}
		else if ((labs(wrapy_x) + labs(wrapy_y) + labs(wrapy_z)) == min){
			rr_x[num] = wrapy_x;
			rr_y[num] = wrapy_y;
			rr_z[num] = wrapy_z;
			num++;
		}

		// wraparround z
		wrapz_z = -sign(mesh_z)*(nodes_z - labs(mesh_z));
		wrapz_x = (sx - sign(mesh_z)*sk_zx)%nodes_x;
		if (wrapz_x < 0) wrapz_x += nodes_x;

		wrapz_x = (dx-wrapz_x)%nodes_x;
		if (wrapz_x < 0) wrapz_x += nodes_x;
		if (wrapz_x > nodes_x/2) wrapz_x = (nodes_x-wrapz_x)*(-1);
		if ((double)wrapz_x == nodes_x/2.0)
			if (rand() >= (RAND_MAX/2)) wrapz_x = (nodes_x-wrapz_x)*(-1);

		wrapz_y = mesh_y;

		if ((labs(wrapz_x) + labs(wrapz_y) + labs(wrapz_z)) < min){
			rr_x[0] = wrapz_x;
			rr_y[0] = wrapz_y;
			rr_z[0] = wrapz_z;
			num=1;
			min=(labs(rr_x[0]) + labs(rr_y[0]) + labs(rr_z[0]));
		}
		else if ((labs(wrapz_x) + labs(wrapz_y) + labs(wrapz_z)) == min){
			rr_x[num] = wrapz_x;
			rr_y[num] = wrapz_y;
			rr_z[num] = wrapz_z;
			num++;
		}

		// wraparround y + wraparround z
		wrapy_y = -sign(mesh_y)*(nodes_y - labs(mesh_y));
		wrapz_z = -sign(mesh_z)*(nodes_z - labs(mesh_z));
		wrapx_x = (sx - sign(mesh_y)*sk_yx)%nodes_x;
		if (wrapx_x < 0) wrapx_x += nodes_x;
		wrapx_x = (wrapx_x -sign(mesh_z)*sk_zx)%nodes_x;
		if (wrapx_x < 0) wrapx_x += nodes_x;

		wrapx_x = (dx - wrapx_x)%nodes_x;
		if (wrapx_x < 0) wrapx_x += nodes_x;
		if (wrapx_x > nodes_x/2) wrapx_x = (nodes_x-wrapx_x)*(-1);
		if ((double)wrapx_x == nodes_x/2.0)
			if (rand() >= (RAND_MAX/2)) wrapx_x = (nodes_x-wrapx_x)*(-1);

		if ((labs(wrapx_x) + labs(wrapy_y) + labs(wrapz_z)) < min){
			rr_x[0] = wrapx_x;
			rr_y[0] = wrapy_y;
			rr_z[0] = wrapz_z;
			num=1;
			min=(labs(rr_x[0]) + labs(rr_y[0]) + labs(rr_z[0]));
		}
		else if ((labs(wrapx_x) + labs(wrapy_y) + labs(wrapz_z)) == min){
			rr_x[num] = wrapx_x;
			rr_y[num] = wrapy_y;
			rr_z[num] = wrapz_z;
			num++;
		}
	}
	else if ((sk_xy !=0) || (sk_zy !=0)){
		// mesh
		mesh_x = dx - sx;
		mesh_y = (dy-sy)%nodes_y; if (mesh_y < 0) mesh_y += nodes_y;
		if (mesh_y > nodes_y/2) mesh_y = (nodes_y-mesh_y)*(-1);
		if ((double)mesh_y == nodes_y/2.0)
			if (rand() >= (RAND_MAX/2)) mesh_y = (nodes_y-mesh_y)*(-1);
		mesh_z = dz - sz;

		rr_x[0] = mesh_x;
		rr_y[0] = mesh_y;
		rr_z[0] = mesh_z;
		num=1;
		min=(labs(rr_x[0]) + labs(rr_y[0]) + labs(rr_z[0]));

		// wraparround x
		wrapx_x = -sign(mesh_x)*(nodes_x - labs(mesh_x));
		wrapx_y = (sy - sign(mesh_x)*sk_xy)%nodes_y;
		if (wrapx_y < 0) wrapx_y += nodes_y;

		wrapx_y = (dy-wrapx_y)%nodes_y;
		if (wrapx_y < 0) wrapx_y += nodes_y;
		if (wrapx_y > nodes_y/2) wrapx_y = (nodes_y-wrapx_y)*(-1);
		if ((double)wrapx_y == nodes_y/2.0)
			if (rand() >= (RAND_MAX/2)) wrapx_y = (nodes_y-wrapx_y)*(-1);

		wrapx_z = mesh_z;

		if ((labs(wrapx_x) + labs(wrapx_y) + labs(wrapx_z)) < min){
			rr_x[0] = wrapx_x;
			rr_y[0] = wrapx_y;
			rr_z[0] = wrapx_z;
			num=1;
			min=(labs(rr_x[0]) + labs(rr_y[0]) + labs(rr_z[0]));
		}
		else if ((labs(wrapx_x) + labs(wrapx_y) + labs(wrapx_z)) == min){
			rr_x[num] = wrapx_x;
			rr_y[num] = wrapx_y;
			rr_z[num] = wrapx_z;
			num++;
		}

		// wraparround z
		wrapz_z = -sign(mesh_z)*(nodes_z - labs(mesh_z));
		wrapz_y = (sy - sign(mesh_z)*sk_zy)%nodes_y;
		if (wrapz_y < 0) wrapz_y += nodes_y;

		wrapz_y = (dy-wrapz_y)%nodes_y;
		if (wrapz_y < 0) wrapz_y += nodes_y;
		if (wrapz_y > nodes_y/2) wrapz_y = (nodes_y-wrapz_y)*(-1);
		if ((double)wrapz_y == nodes_y/2.0)
			if (rand() >= (RAND_MAX/2)) wrapz_y = (nodes_y-wrapz_y)*(-1);

		wrapz_x = mesh_x;

		if ((labs(wrapz_x) + labs(wrapz_y) + labs(wrapz_z)) < min){
			rr_x[0] = wrapz_x;
			rr_y[0] = wrapz_y;
			rr_z[0] = wrapz_z;
			num=1;
			min=(labs(rr_x[0]) + labs(rr_y[0]) + labs(rr_z[0]));
		}
		else if ((labs(wrapz_x) + labs(wrapz_y) + labs(wrapz_z)) == min){
			rr_x[num] = wrapz_x;
			rr_y[num] = wrapz_y;
			rr_z[num] = wrapz_z;
			num++;
		}

		// wraparround x + wraparround z
		wrapx_x = -sign(mesh_x)*(nodes_x - labs(mesh_x));
		wrapz_z = -sign(mesh_z)*(nodes_z - labs(mesh_z));
		wrapy_y = (sy - sign(mesh_x)*sk_xy)%nodes_y;
		if (wrapy_y < 0 ) wrapy_y += nodes_y;
		wrapy_y = (wrapy_y -sign(mesh_z)*sk_zy)%nodes_y;
		if (wrapy_y < 0 ) wrapy_y += nodes_y;
		wrapy_y = (dy-wrapy_y)%nodes_y;
		if (wrapy_y < 0) wrapy_y += nodes_y;
		if (wrapy_y > nodes_y/2) wrapy_y = (nodes_y-wrapy_y)*(-1);
		if ((double)wrapy_y == nodes_y/2.0)
			if (rand() >= (RAND_MAX/2)) wrapy_y = (nodes_y-wrapy_y)*(-1);

		if ((labs(wrapx_x) + labs(wrapy_y) + labs(wrapz_z)) < min){
			rr_x[0] = wrapx_x;
			rr_y[0] = wrapy_y;
			rr_z[0] = wrapz_z;
			num=1;
			min=(labs(rr_x[0]) + labs(rr_y[0]) + labs(rr_z[0]));
		}
		else if ((labs(wrapx_x) + labs(wrapy_y) + labs(wrapz_z)) == min){
			rr_x[num] = wrapx_x;
			rr_y[num] = wrapy_y;
			rr_z[num] = wrapz_z;
			num++;
		}
	}
	else {
		// mesh
		mesh_z = (dz-sz)%nodes_z; if (mesh_z < 0) mesh_z += nodes_z;
		if (mesh_z > nodes_z/2) mesh_z = (nodes_z-mesh_z)*(-1);
		if ((double)mesh_z == nodes_z/2.0)
			if (rand() >= (RAND_MAX/2)) mesh_z = (nodes_z-mesh_z)*(-1);
		mesh_x = dx - sx;
		mesh_y = dy - sy;

		rr_x[0] = mesh_x;
		rr_y[0] = mesh_y;
		rr_z[0] = mesh_z;
		num=1;
		min=(labs(rr_x[0]) + labs(rr_y[0]) + labs(rr_z[0]));

		// wraparround y
		wrapy_y = -sign(mesh_y)*(nodes_y - labs(mesh_y));
		wrapy_z = (sz - sign(mesh_y)*sk_yz)%nodes_z;
		if (wrapy_z < 0) wrapy_z += nodes_z;

		wrapy_z = (dz-wrapy_z)%nodes_z;
		if (wrapy_z < 0) wrapy_z += nodes_z;
		if (wrapy_z > nodes_z/2) wrapy_z = (nodes_z-wrapy_z)*(-1);
		if ((double)wrapy_z == nodes_z/2.0)
			if (rand() >= (RAND_MAX/2)) wrapy_z = (nodes_z-wrapy_z)*(-1);

		wrapy_x = mesh_x;

		if ((labs(wrapy_x) + labs(wrapy_y) + labs(wrapy_z)) < min){
			rr_x[0] = wrapy_x;
			rr_y[0] = wrapy_y;
			rr_z[0] = wrapy_z;
			num=1;
			min=(labs(rr_x[0]) + labs(rr_y[0]) + labs(rr_z[0]));
		}
		else if ((labs(wrapy_x) + labs(wrapy_y) + labs(wrapy_z)) == min){
			rr_x[num] = wrapy_x;
			rr_y[num] = wrapy_y;
			rr_z[num] = wrapy_z;
			num++;
		}

		// wraparround x
		wrapx_x = -sign(mesh_x)*(nodes_x - labs(mesh_x));
		wrapx_z = (sz - sign(mesh_z)*sk_xz)%nodes_z;
		if (wrapx_z < 0) wrapx_z += nodes_z;

		wrapx_z = (dz-wrapx_z)%nodes_z;
		if (wrapx_z < 0) wrapx_z += nodes_z;
		if (wrapx_z > nodes_z/2) wrapx_z = (nodes_z-wrapx_z)*(-1);
		if ((double)wrapx_z == nodes_z/2.0)
			if (rand() >= (RAND_MAX/2)) wrapx_z = (nodes_z-wrapx_z)*(-1);

		wrapx_y = mesh_y;

		if ((labs(wrapx_x) + labs(wrapx_y) + labs(wrapx_z)) < min){
			rr_x[0] = wrapx_x;
			rr_y[0] = wrapx_y;
			rr_z[0] = wrapx_z;
			num=1;
			min=(labs(rr_x[0]) + labs(rr_y[0]) + labs(rr_z[0]));
		}
		else if ((labs(wrapx_x) + labs(wrapx_y) + labs(wrapx_z)) == min){
			rr_x[num] = wrapx_x;
			rr_y[num] = wrapx_y;
			rr_z[num] = wrapx_z;
			num++;
		}

		// wraparround y + wraparround x
		wrapy_y = -sign(mesh_y)*(nodes_y - labs(mesh_y));
		wrapx_x = -sign(mesh_x)*(nodes_x - labs(mesh_x));
		wrapz_z = (sz - sign(mesh_y)*sk_yz)%nodes_z;
		if (wrapz_z < 0) wrapz_z += nodes_z;
		wrapz_z = (wrapz_z -sign(mesh_x)*sk_xz)%nodes_z;
		if (wrapz_z < 0) wrapz_z += nodes_z;

		wrapz_z = (dz - wrapz_z)%nodes_z;
		if (wrapz_z < 0) wrapz_z += nodes_z;
		if (wrapz_z > nodes_z/2) wrapz_z = (nodes_z-wrapz_z)*(-1);
		if ((double)wrapz_z == nodes_z/2.0)
			if (rand() >= (RAND_MAX/2)) wrapz_z = (nodes_z-wrapz_z)*(-1);

		if ((labs(wrapx_x) + labs(wrapy_y) + labs(wrapz_z)) < min){
			rr_x[0] = wrapx_x;
			rr_y[0] = wrapy_y;
			rr_z[0] = wrapz_z;
			num=1;
			min=(labs(rr_x[0]) + labs(rr_y[0]) + labs(rr_z[0]));
		}
		else if ((labs(wrapx_x) + labs(wrapy_y) + labs(wrapz_z)) == min){
			rr_x[num] = wrapx_x;
			rr_y[num] = wrapy_y;
			rr_z[num] = wrapz_z;
			num++;
		}
	}

	bet=rand()%num;
	res.rr[D_X] = rr_x[bet];
	if (ndim >= 2)
		res.rr[D_Y] = rr_y[bet];
	if (ndim == 3)
		res.rr[D_Z] = rr_z[bet];
	res.size = res.rr[D_X] + res.rr[D_Y] + res.rr[D_Z];
	return res;
}

/**
* Generates the routing record for an unidirectional twisted torus.
* 
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record needed to go from source to destination.
*/
routing_r dtt_rr_unidir (long source, long destination) {
	routing_r res;
	panic("Not ready to work yet with unidirectional twisted torus");
/*
	res.rr=alloc(ndim*sizeof(long));
	res.rr[D_X] = 0;
	if (ndim >= 2)
		res.rr[D_Y] = 0;
	if (ndim == 3)
		res.rr[D_Z] = 0;
*/
    return res;
}


