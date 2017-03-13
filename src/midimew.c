/**
* @file
* @brief	The midimew topology tools.

FSIN Functional Simulator of Interconnection Networks
Copyright (2003-2011) J. Miguel-Alonso, A. Gonzalez, J. Navaridas

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
* Obtains a neighbor node in midimew topology (only 2D).
* 
* @param ad A node address.
* @param wd A dimension (X or Y).
* @param ww A way (UP or DOWN).
* @return The address of the neighbor in that direction and way; only valid for midimew
*/
long midimew_neighbor(long ad, dim wd, way ww) {
	long res;
	long b = (long)ceil(sqrt(((double)NUMNODES/(double)2)));

	switch (wd) {
		case D_X:
			if (ww == DOWN)
				b = -b;
			res = (ad+b)%NUMNODES;
			if (res < 0)
				res += NUMNODES;
			break;
		case D_Y:
			b = b-1;
			if (ww == DOWN)
				b = -b;
			res = (ad+b)%NUMNODES;
			if (res < 0)
				res += NUMNODES;
			break;
		default:
			res = 0;
			panic("Only 2-D Midimew");
	}
	return res;
}

/**
* Generates the routing record for a midimew.
* 
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record needed to go from source to destination.
*/
routing_r midimew_rr (long source, long destination) {
	long b, m, sign;
	long x0, x1, y0, y1, q, r;
	routing_r res;

	res.rr=alloc(ndim*sizeof(long));

	b = (long)ceil(sqrt(((double)NUMNODES/(double)2)));

	if (source == destination)
		panic("Self-sent packet");
	m = labs(destination-source);
	if (destination <= source) sign = -1; else sign = 1;
	if (m > NUMNODES/2) {
		sign = -sign;
		m = NUMNODES - m;
	}
	q = m/b;
	r = m-(q*b);
	y0 = -r;
	x0 = q + r;
	y1 = y0 + b;
	x1 = x0 - (b-1);

	if ((y0 == 0)||(x0 < y1)) {
		res.rr[D_X] = x0*sign;
		res.rr[D_Y] = y0*sign;
	}
	else {
		res.rr[D_X] = x1*sign;
		res.rr[D_Y] = y1*sign;
	}

	res.size = abs(res.rr[D_X]) + abs(res.rr[D_Y]);
	return res;
}

