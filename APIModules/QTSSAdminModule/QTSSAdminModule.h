/*
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 *
 */
/*
    File:       QTSSWebStatsModule.h

    Contains:   A module that uses the information available in the server
                to present a web page containing that information. Uses the Filter
                module feature of QTSS API.

    $Log: QTSSAdminModule.h,v $
    Revision 1.5  2003/08/15 23:52:42  sbasu
    3370815 Need to update to the APSL 2
    Bug #:
    Submitted by:
    Reviewed by:

    Revision 1.4  2002/02/26 00:17:42  murata
    Convert tabs to spaces in .cpp .c and .h files.
    Bug #:
    Submitted by:
    Reviewed by:

    Revision 1.3  2001/03/13 22:24:03  murata
    Replace copyright notice for license 1.0 with license 1.2 and update the copyright year.
    Bug #:
    Submitted by:
    Reviewed by:
    
    Revision 1.2  2000/10/13 08:20:27  serenyi
    import
    
    Revision 1.1.1.1  2000/08/31 00:30:22  serenyi
    Mothra Repository
    
    Revision 1.1  2000/07/24 23:50:25  murata
    tuff for Ghidra/Rodan
    
    Revision 1.1  2000/06/17 04:20:11  murata
    Add QTSSAdminModule and fix Project Builder,Jam and Posix Make builds.
    Bug #:
    Submitted by:
    Reviewed by:
        
    
*/

#ifndef __QTSSADMINMODULE_H__
#define __QTSSADMINMODULE_H__

#include "QTSS.h"

extern "C"
{
    EXPORT QTSS_Error QTSSAdminModule_Main(void* inPrivateArgs);
}

#endif // __QTSSADMINMODULE_H__

