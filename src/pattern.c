/**
* @file
* @brief	Definition of all synthetic traffic patterns in use by FSIN.

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

#include <math.h>

#include "pattern.h"

#define i_esimo(b,i) (((b) & (1 << (i))) ? 1:0) ///< Gets the bit in position i of a given number.

/**
* Calculates the destination using transpose permutation.
* 
* @param node The source node.
* @param nnodes Total number of Nodes.
* @return The destination node.
*/
long transpose(long node, long nnodes) {
	long i;
	long to = 0;
	long nbits;

	nbits = (long)ceil(log(nnodes)/log(2.0));
	if (pow(2, nbits-1) >= nnodes) nbits--;

	for(i = 0; i < nbits; ++i)
		to |= (i_esimo(node, i) == 1) ? (1 << ((nbits / 2 + i) % nbits)):0;
	return to;
}

/**
* Calculates the destination complementing the source.
* 
* @param node The source node.
* @param nnodes Total number of Nodes.
* @return The destination node.
*/
long complement(long node, long nnodes) {
	long i;
	long to = 0;
	long nbits;

	nbits = (long)ceil(log(nnodes)/log(2.0));
	if (pow(2, nbits-1) >= nnodes) nbits--;

	for(i = 0; i < nbits; ++i)
		to |= (i_esimo(node, i) == 0) ? (1 << i):0;
	return to;
}

/**
* Calculates the destination using butterfly permutation.
* 
* @param node The source node.
* @param nnodes Total number of Nodes.
* @return The destination node.
*/
long butterfly(long node, long nnodes) {
	long to = node;
	long nbits;

	nbits = (long)ceil(log(nnodes)/log(2.0));
	if (pow(2, nbits-1) >= nnodes) nbits--;

	if(i_esimo(node, 0) == 1)
		to |= (1 << (nbits - 1));
	else
		to &= ~(1 << (nbits - 1));

	if(i_esimo(node, nbits -1) == 1)
		to |= 1;
	else
		to &= ~1;
	return to;
}

/**
* Calculates the destination using the perfect shuffle permutation.
* 
* @param node The source node.
* @param nnodes Total number of Nodes.
* @return The destination node.
*/
long shuffle(long node, long nnodes) {
	long i;
	long to = 0;
	long nbits;

	nbits = (long)ceil(log(nnodes)/log(2.0));
	if (pow(2, nbits-1) >= nnodes) nbits--;

	for(i = 0; i < nbits; ++i)
		to |= (i_esimo(node, i) == 1) ? (1 << ((i + 1) % nbits)):0;
	return to;
}

/**
* Calculates the destination reversing the source node.
* 
* @param node The source node.
* @param nnodes Total number of Nodes.
* @return The destination node.
*/
long reversal(long node, long nnodes) {
	long i;
	long to = 0;
	long nbits;

	nbits = (long)ceil(log(nnodes)/log(2.0));
	if (pow(2, nbits-1) >= nnodes) nbits--;

	for(i = 0; i < nbits; ++i)
		to |= (i_esimo(node, i) == 1) ? (1 << (nbits - i - 1)):0;

	return to;
}

