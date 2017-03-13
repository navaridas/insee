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


#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "misc.h"
#include "globals.h"

extern time_t start_time, end_time;

/**
* Aborts the simulation cleanly & prints an error message.
* @param msg A string containing the error message
*/
void abort_sim(char * msg) {
    fprintf(stderr, "******************************************************************************************************\n");
	fprintf(stderr, "* ABORTING: %-88s *\n", msg);
    fprintf(stderr, "******************************************************************************************************\n");

    aborted=B_TRUE;
}

/**
* Aborts the simulation & prints an error message.
* Should only be used when an insolvable error is encountered, otherwise should use abort (), which completes the whole execution rather than killing the program.
*
* When the simulation ends here the return code is -1.
*
* @param msg A string containing the error message
*/
void panic(char * msg) {
    fprintf(stderr, "******************************************************************************************************\n");
	fprintf(stderr, "* PANIC: %-91s *\n", msg);
	fprintf(stderr, "******************************************************************************************************\n");
#ifdef WIN32
	system("PAUSE");
#endif /* WIN32 */
    aborted=B_TRUE;
	time(&end_time);
	print_results(start_time, end_time);
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

