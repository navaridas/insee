/**
* @file
* @brief	The circulant graph topology tools.
* @author	J. Navaridas

FSIN Functional Simulator of Interconnection Networks

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

long step;	///> The distance to the second dimension of adjacency
long rows;	///> The number of rows of the circulant graph
long twist;	///> The twist of the circulant graphs (every r_circ hops in y advances t_circ in x)

/**
* Obtains a neighbor node in a circulant graph topology (only 2D).
*
* @param ad A node address.
* @param wd A dimension (X or Y).
* @param ww A way (UP or DOWN).
* @return The address of the neighbor in that direction and way
*/
long circulant_neighbor(long ad, dim wd, way ww) {
	long res;	///> The id of the neighbour in the required dimension and direction.

	switch (wd) {
		case D_X:	//	Clockwise and counterclockwise neightbours
			if (ww == DOWN)
				res = (NUMNODES+ad-1)%NUMNODES;
			else
				res = (ad+1)%NUMNODES;
			break;
		case D_Y:	//	Neighbout#rs located at +k and -k
			if (ww == DOWN)
				res = (NUMNODES+ad-step)%NUMNODES;
			else
				res = (ad+step)%NUMNODES;
			break;
		default:
			res = 0;
			panic("Only 2-D Circulant Graphs implemented so far");
	}
	return res;
}

/**
* Generates the routing record for a circulant graph.
*
* EXPERIMENTAL:
* minimal routing ??????????.
*
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record to go from source to destination.
*/
routing_r circulant_rr (long source, long destination) {
	long A1;	///> The id difference when travelling clockwise (positive)
	long A2;	///> The id difference when travelling counterclockwise (negative)
	long x[12],y[12],d[12];	///> The six possible routes
	long t, top, i;		///> A temporal variable to compute the number of complete turn using the twists.
	long minx[12],miny[12], mind,paths;	///> for searching the shortest path
	routing_r res;	///> The resulting routing record

	//printf("\n==== kk:  %ld --> %ld ====\n",source, destination);
	res.rr=alloc(ndim*sizeof(long));

	top=4;

	if (destination>source){
		A1=destination-source;
		A2=A1-NUMNODES;
	} else {
		A2=destination-source;
		A1=A2+NUMNODES;
	}

	// Travelling clockwise
	y[0]=A1/step;
	x[0]=A1%step;
	d[0]=abs(x[0])+abs(y[0]);

	// Clockwise with an extra y hop
	y[1]=y[0]+1;
	x[1]=x[0]-step;
	d[1]=abs(x[1])+abs(y[1]);

	// Now, counterclockwise
	y[2]=A2/step;
	x[2]=A2%step;	// this should be negative, but may depend on C implementation.
	d[2]=abs(x[2])+abs(y[2]);

	// Counterclockwise with an extra Y hop
	y[3]=y[2]-1;
	x[3]=x[2]+step;
	d[3]=abs(x[3])+abs(y[3]);

	if (abs(twist)>=abs(rows)){

		// Advance in X using the twists in Y
		t=x[0]/twist;
		x[4]=x[0]%twist;
		y[4]=y[0]-(rows*t);
		d[4]=abs(x[4])+abs(y[4]);

		// Advance an extra complete turn using Y
		x[5]=x[4]+twist;
		y[5]=y[4]+rows;
		d[5]=abs(x[5])+abs(y[5]);

		// Advance in X using the twists in Y
		t=x[1]/twist;
		x[6]=x[1]%twist;
		y[6]=y[1]-(rows*t);
		d[6]=abs(x[6])+abs(y[6]);

		// Advance an extra complete turn using Y
		x[7]=x[6]-twist;
		y[7]=y[6]-rows;
		d[7]=abs(x[7])+abs(y[7]);

		// Advance in X using the twists in Y
		t=x[2]/twist;
		x[8]=x[2]%twist;
		y[8]=y[2]-(rows*t);
		d[8]=abs(x[8])+abs(y[8]);

		// Advance an extra complete turn using Y
		x[9]=x[8]-twist;
		y[9]=y[8]-rows;
		d[9]=abs(x[9])+abs(y[9]);

		// Advance in X using the twists in Y
		t=x[3]/twist;
		x[10]=x[3]%twist;
		y[10]=y[3]-(rows*t);
		d[10]=abs(x[10])+abs(y[10]);

		// Advance an extra complete turn using Y
		x[11]=x[10]+twist;
		y[11]=y[10]+rows;
		d[11]=abs(x[11])+abs(y[11]);

		top=12;
	}
	//Let's decide which way is better
	minx[0]=x[0];
	miny[0]=y[0];
	mind=d[0];
	paths=1;

	//printf("kk:  %ld, %ld (%ld)\n",x[0],y[0],d[0]);
	for (i=1; i<top; i++){
	//printf("kk:  %ld, %ld (%ld)\n",x[i],y[i],d[i]);
		if  (d[i]==mind){
			minx[paths]=x[i];
			miny[paths]=y[i];
			paths++;
		}
		if (d[i]<mind){
			minx[0]=x[i];
			miny[0]=y[i];
			paths=1;
			mind=d[i];
		}
	}
	t=rand()%paths;

	res.rr[D_X] = minx[t];
	res.rr[D_Y] = miny[t];
	res.size = mind;
	//printf("kkKKkk:  %ld, %ld (%ld)\n",minx[t],miny[t],mind);

	return res;
}


/**
* Generates the routing record for a circulant graph.
*
* EXPERIMENTAL:
* Not minimal routing implemented yet.
*
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record to go from source to destination.
*/
routing_r circulant_dummy_rr (long source, long destination) {
	long A1, x1, y1, d1; ///> The id difference, the number of hops in x and y and the total distance, when travelling clockwise
	long A2, x2, y2, d2; ///> The id difference, the number of hops in x and y and the total distance, when travelling counterclockwise
	routing_r res;	///> The resulting routing record

	res.rr=alloc(ndim*sizeof(long));

	if (destination>source){
		A1=destination-source;
		A2=A1-NUMNODES;
	} else {
		A2=destination-source;
		A1=A2+NUMNODES;
	}

	// Travelling clockwise
	y1=A1/step;
	x1=A1%step;
	if (x1>step/2){	// if k is odd and x=k/2+1 should decide at random whether take another hop in Y or not. TODO
		y1++;
		x1=x1-step;
	}
	d1=abs(x1)+abs(y1);

	// Now, counterclockwise
	y2=A2/step;
	x2=A2%step;	// this should be negative, but may depend on C implementation.
	if (abs(x2)>step/2){	// same as above
		y2--;
		x2=x2+step;
	}
	d2=abs(x2)+abs(y2);

	//Let's decide which way is better
	if (d1<d2 || (d1==d2 && (rand() >= (RAND_MAX/2)))) {
		res.rr[D_X] = x1;
		res.rr[D_Y] = y1;
		res.size = d1;
	} else {
		res.rr[D_X] = x2;
		res.rr[D_Y] = y2;
		res.size = d2;
	}

	//printf("From %ld to %ld\nA1: %ld\t%ld, %ld\t[%ld]\nA2: %ld\t%ld, %ld\t[%ld]\n",source,destination,A1,x1,y1,d1,A2,x2,y2,d2);
	return res;
}

