/*
Copyright (C) 2008 StrmnNrmn

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef CONFIG_H_
#define CONFIG_H_

///////////////////////////////////////////////////////////////////////////////
//
//	Config options for the public release build
//
///////////////////////////////////////////////////////////////////////////////
#undef	DAEDALUS_DEBUG_PIF					// Enable to enable various debugging options for PIF (Peripheral interface)
#define DAEDALUS_ALIGN_REGISTERS
#undef  DAEDALUS_DEBUG_DYNAREC				// Enable to enable various debugging options for the dynarec
#undef  DAEDALUS_ENABLE_SYNCHRONISATION		// Enable for sync testing
#undef  DAEDALUS_LOG						// Enable various logging
#undef  DAEDALUS_ENABLE_ASSERTS				// Enable asserts
#undef  DAEDALUS_DEBUG_DISPLAYLIST			// Enable the display list debugger
#undef  DAEDALUS_ENABLE_PROFILING			// Enable the built-in profiler
#undef  DAEDALUS_PROFILE_EXECUTION			// Enable to keep track of various execution stats
#undef  DAEDALUS_BATCH_TEST_ENABLED			// Enable the batch test
#undef	DAEDALUS_DEBUG_MEMORY
#undef	ALLOW_TRACES_WHICH_EXCEPT
#define DAEDALUS_SILENT						// Undef to enable debug messages 
// Define this to turn off various debugging features for public release.
#define DAEDALUS_PUBLIC_RELEASE


#endif // CONFIG_H_
