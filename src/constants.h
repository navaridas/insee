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

#include <limits.h>
#include <stdint.h>

#ifndef BIMODAL_SUPPORT
#define BIMODAL_SUPPORT 0	///< Deactivates support for bimodal traffic when zero.
#endif /* BIMODAL */

/**
 * Keep count of the number of phits within a router and if there is 0 do not enter packet moving routine.
 * Boost traces using large cpu intervals or handling low loads. Otherwise it is not likely to help, but barely harms performance, so it's the default mode.
 */
#ifndef PCOUNT
#define PCOUNT 0
#endif /* PCOUNT */

#ifndef TRACE_SUPPORT
#define TRACE_SUPPORT 2		///< 0: trace support is deactivated.
                            ///< 1: occurs is implemented as a single list (less memory but slower).
							///< 2+: occurs is implemented as a multilist (much faster but more memory). This is default
#endif /* TRACE */

#ifndef CHECK_TRC_DEADLOCK
#define CHECK_TRC_DEADLOCK 10000 ///< A debugging mode that checks in run-time whether there is a trace-level deadlock (DEFAULT: 10k cycles with all nodes receiving) false positives with long communications.
#endif /* CHECK_TRC_DEADLOCK */

#ifndef SKIP_CPU_BURSTS
#define SKIP_CPU_BURSTS 1 ///< If all the nodes are executing CPU events skip enough cycles until there is any communication event. Checking is somewhat compute intensive (although there is some space for improvement), so only should be used with cpu-intensive traces
#endif /* CHECK_TRC_DEADLOCK */


/* Execution driven simulation */
#ifndef EXECUTION_DRIVEN
#define EXECUTION_DRIVEN 0	///< If non-zero, performs a execution driven simulation. Overrides other execution modes in #tpattern.
#endif /* EXECUTION DRIVEN */

#ifndef CLOCK_TYPE
#define CLOCK_TYPE long long  ///< data type to use for simulation time variables.
#endif /* CLOCK_TYPE */

#ifndef PRINT_CLOCK
#define PRINT_CLOCK "lld" ///< data type to use for printing simulation time variables. the % sign is left outside so that different formatting can be applied in other places. (e.g. %8lld
#endif /* PRINT_CLOCK */

#ifndef SCAN_CLOCK
#define SCAN_CLOCK "lld" ///< data type to use for scanning simulation time variables.
#endif /* SCAN_CLOCK */

#ifndef CLOCK_MAX
#define CLOCK_MAX LONG_LONG_MAX ///< data type to use for simulation time variables. Indent to 11 characters
#endif /* CLOCK_MAX */

#ifndef LONG_LONG_MAX
#define LONG_LONG_MAX LLONG_MAX
#endif /* LONG_LONG_MAX */

#endif /* _constants */

