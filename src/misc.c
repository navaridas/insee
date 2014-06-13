/**
* @file
* @brief	Some miscellaneus tools & definitions.

FSIN Functional Simulator of Interconnection Networks
Copyright (2003-2011) J. Miguel-Alonso, A. Gonzalez

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

#include <stdio.h>

#include "misc.h"

/**
* Stops the simulation & prints an error message.
*
* When the simulation ends here the return code is -1.
* 
* @param msg A string containing the error message 
*/
void panic(char * msg) {
	fprintf(stderr, "panic: %s !!!\n", msg);
#ifdef WIN32    
	system("PAUSE");
#endif /* WIN32 */
	exit(-1);
}

/**
* Allocates memory.
* 
* @param size The size in bytes of the memory allocation.
*/
void * alloc(long size) {
	void * res;
	if((res = malloc(size)) == NULL)
		panic("alloc: Unable to allocate memory");
	return res;
}
