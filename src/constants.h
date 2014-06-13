/**
* @file
* @brief	Some definitions for default compilation.

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

#ifndef _constants
#define _constants

#include <stdint.h>

#ifndef BIMODAL_SUPPORT
#define BIMODAL_SUPPORT 0	///< Deactivates support for bimodal traffic when zero.
#endif /* BIMODAL */

/**
 * Keep count of the number of phits within a router and if there is 0 do not enter packet moving routine. 
 * Boost traces using large cpu intervals or handling low loads. Otherwise it is not likely to help.
 */
#ifndef PCOUNT
#define PCOUNT 1	
#endif /* PCOUNT */

#ifndef TRACE_SUPPORT
#define TRACE_SUPPORT 1		///< 0: trace support is deactivated.\
								 1: occurs is implemented as a single list (less memory but slower).\
								 2+: occurs is implemented as a multilist (faster but more memory).
#endif /* TRACE */

/* Execution driven simulation */
#ifndef EXECUTION_DRIVEN
#define EXECUTION_DRIVEN 0	///< If non-zero, performs a execution driven simulation. Overrides other execution modes in #tpattern.
#endif /* EXECUTION DRIVEN */

typedef long long CLOCK_TYPE;

#ifndef PRINT_CLOCK
#define PRINT_CLOCK "lld" ///< data type to use for simulation time variables. Indent to 11 characters
#endif /* PRINT_CLOCK */

#ifndef SCAN_CLOCK
#define SCAN_CLOCK "lld" ///< data type to use for simulation time variables.
#endif /* SCAN_CLOCK */

#ifndef CLOCK_MAX
#define CLOCK_MAX LLONG_MAX ///< data type to use for simulation time variables. Indent to 11 characters
#endif /* CLOCK_MAX */

#endif /* _constants */
