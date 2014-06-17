/**
* @file
* @brief	Circulant $C_{2a^2}(1, 2ka-1)$, gcd(k,a) = 1 and 1 <= k <= (a-1)/2
* @author	Pranava K. Jha, J. Navaridas

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
#include <math.h>
#include <stdlib.h>

long a, k;		///> the two parameters of the topology. 
long k_inv; 	///> the inverse of k used for routing
long s1, s2;	///> the steps. s1=1, s2=2*k*a-1;

//++++++++++++++
/**
* Blueprint for the ordered pair
*/
struct Double{
	long x;
	long y;
};

/**
* Blueprint for the ordered triple in respect of the Euclid's algorithm
*/
struct Triple{
	long d;
	long k;
	long m;
};

/**
* Euclid's algorithm
* Recursive method to calculate the greatest common divisor of two integers
* Also used to calculate the modular multiplicative inverse
*
* @param i The smaller of the integers
* @param n The largest of the integers (i<=n)
* @return A Triple containing the GCD(i,n), the MMI(i,n) and the MMI(n,i)
*/
struct Triple Euclid(long m, long n) {
	struct Triple t, v, w;
	long q, r;
	long d1, k1, m1;

	if (m == 0)	{
		t.d = n;
		t.k = 1;
		t.m = 0;
		return t;
	}
	
	q = n % m;
	r = n/m;
	v = Euclid(q,m);
	
	d1 = v.d;
	k1 = v.k;
	m1 = v.m;

	w.d = d1;
	w.k = m1;
	w.m = k1 - m1*r;
	
	return w;
}

/**
* Calculates the greatest common divisor of two integers, uses the Euclidean method.
*
* @param m One integer
* @param n One integer
* @return An integer with the greatest common divisor of i and n Triple containing the 
*/
long gcd(long m, long n){
	struct Triple z;
	
	if (m==n)
		return m;
		
	if (m>n)
		z = Euclid(n, m);
	else
		z = Euclid(m, n);
		
	return z.d;
}

/**
* Calculates the modular multiplicative inverse of an integer i modulo n
* GCD(i,n) must be 1
* @param i The smaller of the integers
* @param n The largest of the integers (i<=n)
* @return An integer with the inverse of i modulo n
*/
long inverse(long i, long n){
	struct Triple z;

	z = Euclid(i, n);
	if(z.m < 0)
		z.m += n;
	return z.m;
}

/**
* Locates the coordinates of an integer in the minimum distance diagram
*
* @param m An integer
* @return A Double with the coordinates
*/
struct Double map(long m){
	struct Double v;
	long q, r, c;
	
	if(m <= a-1){
		v.x = m;
		v.y = 0;
	}
	else if(NUMNODES-a <= m){
		v.x = m-NUMNODES; v.y = 0;
	}
	else{
		q = m/a; r = m%a; 
		if(q%2 == 0){
			q = q-1; r = r+a;
		}
		// At this point, m = qa +r, where q is odd and 0 <= r <= 2a-1.
		c = ((q+1)/2)*k_inv % a;
		if(0 <= r && r <= 2*(a-c)-1){
			v.x = r-a+c; v.y = -c;
		}
		else{
			v.x = r-2*a+c; v.y = a-c;
		}
	}
	
	return v;
}

/**
* Obtains a neighbor node.
* 
* @param ad A node address.
* @param wd A dimension (X or Y).
* @param ww A way (UP or DOWN).
* @return The address of the neighbor in that direction and way; 
*/
long  circ_pk_neighbor(long ad, dim wd, way ww) 
{
	long res;

	switch (wd) {
		case D_X:	//	Clockwise and counterclockwise neightbours 
			if (ww == DOWN)
				res = (NUMNODES+ad-s1)%NUMNODES;
			else
				res = (ad+s1)%NUMNODES;
			break;
		case D_Y:	//	Neighbours located at step2
			if (ww == DOWN)
				res = (NUMNODES+ad-s2)%NUMNODES;
			else
				res = (ad+s2)%NUMNODES;
			break;
		default:
			res = 0;
			panic("Only 2-D");
	}
	return res;
}

/**
* Generates the routing record.
* 
* @param source The source node of the packet.
* @param destination The destination node of the packet.
* @return The routing record needed to go from source to destination.
*/
routing_r circ_pk_rr (long source, long destination){
	long x0, x1, y0, y1, dx, dy;
	routing_r res;
	struct Double v0, v1;
	long A[9], B[9];
	long weight[9];
	
	long minA[9];
	long minB[9];
	long minw;
	long paths;

	long i,t;
	
	//printf("\n==== kk:  %d --> %d ====\n",source, destination);
	res.rr=alloc(ndim*sizeof(long));

	if (source == destination)
		panic("Self-sent packet");

	v0 = map(source);
    x0 = v0.x;
	y0 = v0.y;

	v1 = map(destination);
    x1 = v1.x;
	y1 = v1.y;

	dx = x1-x0;
	dy = y1-y0;
	
	A[0] = 0;
	B[0] = 0;
	weight[0] = abs(dx+A[0])+abs(dy+B[0]);
	
	A[1] = a-k_inv;
	B[1] = a+k_inv;
	weight[1] = abs(dx+A[1])+abs(dy+B[1]);
	
	A[2] = -A[1];
	B[2] = -B[1];
	weight[2] = abs(dx+A[2])+abs(dy+B[2]);
	
	A[3] = 2*a-k_inv;
	B[3] = k_inv;
	weight[3] = abs(dx+A[3])+abs(dy+B[3]);
	
	A[4] = -A[3];
	B[4] = -B[3];
	weight[4] = abs(dx+A[4])+abs(dy+B[4]);
	
	A[5] = -a;
	B[5] = a;
	weight[5] = abs(dx+A[5])+abs(dy+B[5]);
	
	A[6] = -A[5];
	B[6] = -B[5];
	weight[6] = abs(dx+A[6])+abs(dy+B[6]);
	
	if(k_inv < a-k_inv) {
		A[7] = -k_inv;
		B[7] = 2*a+k_inv;
	}
	else {
		A[7] = -(3*a-k_inv);
		B[7] = a-k_inv;
	}
	weight[7] = abs(dx+A[7])+abs(dy+B[7]);
	
	A[8] = -A[7];
	B[8] = -B[7];
	weight[8] = abs(dx+A[8])+abs(dy+B[8]);
	
	//Let's decide which way is better
	minA[0]=A[0];
	minB[0]=B[0];
	minw=weight[0];
	paths=1;
	
	//printf("kk:  %d, %d (%d) ::: %d, %d, %d ::: %d, %d, %d\n",A[0],B[0],weight[0],dx, dx+A[0], abs(dx+A[0]),dy, dy+B[0], abs(dy+B[0]));
	for (i=1; i<9; i++){
	//printf("kk:  %d, %d (%d) ::: %d, %d, %d ::: %d, %d, %d\n",A[i],B[i],weight[i],dx, dx+A[i], abs(dx+A[i]),dy, dy+B[i], abs(dy+B[i]));
		if  (weight[i]==minw){
			minA[paths]=A[i];
			minB[paths]=B[i];
			paths++;
		}
		if (weight[i]<minw){
			minA[0]=A[i];
			minB[0]=B[i];
			paths=1;
			minw=weight[i];
		}
	}
	t=rand()%paths;
	
	res.rr[D_X] = dx+minA[t];
	res.rr[D_Y] = -(dy+minB[t]);
	
	res.size = minw;
	//printf("kkKKkk:  %d, %d (%d)\n",res.rr[D_X],res.rr[D_Y],res.size);
	
	return res;
}
