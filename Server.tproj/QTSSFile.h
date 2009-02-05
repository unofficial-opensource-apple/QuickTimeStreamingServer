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
    File:       QTSSFile.h

    Contains:    
                    
    $Log: QTSSFile.h,v $
    Revision 1.6  2003/08/15 23:53:21  sbasu
    3370815 Need to update to the APSL 2
    Bug #:
    Submitted by:
    Reviewed by:

    Revision 1.5  2002/02/26 00:25:30  murata
    Convert tabs to spaces in .cpp .c and .h files.
    Bug #:
    Submitted by:
    Reviewed by:

    Revision 1.4  2001/03/13 22:24:39  murata
    Replace copyright notice for license 1.0 with license 1.2 and update the copyright year.
    Bug #:
    Submitted by:
    Reviewed by:
    
    Revision 1.3  2000/10/11 07:06:15  serenyi
    import
    
    Revision 1.1.1.1  2000/08/31 00:30:49  serenyi
    Mothra Repository
    
    Revision 1.2  2000/06/02 06:56:05  serenyi
    First checkin for module dictionaries
    
    Revision 1.1  2000/05/22 06:05:28  serenyi
    Added request body support, API additions, SETUP without DESCRIBE support, RTCP bye support
    
    
    
*/

#include "QTSSDictionary.h"
#include "QTSSModule.h"

#include "OSFileSource.h"
#include "EventContext.h"

class QTSSFile : public QTSSDictionary
{
    public:
    
        QTSSFile();
        virtual ~QTSSFile() {}
        
        static void     Initialize();
        
        //
        // Opening & Closing
        QTSS_Error          Open(char* inPath, QTSS_OpenFileFlags inFlags);
        void                Close();
        
        //
        // Implementation of stream functions.
        virtual QTSS_Error  Read(void* ioBuffer, UInt32 inLen, UInt32* outLen);
        
        virtual QTSS_Error  Seek(UInt64 inNewPosition);
        
        virtual QTSS_Error  Advise(UInt64 inPosition, UInt32 inAdviseSize);
        
        virtual QTSS_Error  RequestEvent(QTSS_EventType inEventMask);
        
    private:

        QTSSModule* fModule;
        UInt64      fPosition;
        QTSSFile*   fThisPtr;
        
        //
        // File attributes
        UInt64      fLength;
        time_t      fModDate;

        static QTSSAttrInfoDict::AttrInfo   sAttributes[];
};

