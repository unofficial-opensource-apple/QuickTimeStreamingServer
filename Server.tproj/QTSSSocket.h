/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2001 Apple Computer, Inc.  All Rights Reserved. The
 * contents of this file constitute Original Code as defined in and are
 * subject to the Apple Public Source License Version 1.2 (the 'License').
 * You may not use this file except in compliance with the License.  Please
 * obtain a copy of the License at http://www.apple.com/publicsource and
 * read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.  Please
 * see the License for the specific language governing rights and
 * limitations under the License.
 *
 *
 * @APPLE_LICENSE_HEADER_END@
 *
 */
/*
	File:		QTSSSocket.h

	Contains:	A QTSS Stream object for a generic socket
	
	Written By: Denis Serenyi
			
	$Log: QTSSSocket.h,v $
	Revision 1.4  2001/07/16 08:09:55  mythili
	added newline to fix warnings for solaris compile
	
	Revision 1.3  2001/03/13 22:24:39  murata
	Replace copyright notice for license 1.0 with license 1.2 and update the copyright year.
	Bug #:
	Submitted by:
	Reviewed by:
	
	Revision 1.2  2000/10/11 07:06:15  serenyi
	import
	
	Revision 1.1.1.1  2000/08/31 00:30:49  serenyi
	Mothra Repository
	
	Revision 1.1  2000/05/22 06:05:28  serenyi
	Added request body support, API additions, SETUP without DESCRIBE support, RTCP bye support
	
	
	
*/

#ifndef __QTSS_SOCKET_H__
#define __QTSS_SOCKET_H__

#include "QTSS.h"
#include "EventContext.h"
#include "QTSSStream.h"
#include "Socket.h"

class QTSSSocket : public QTSSStream
{
	public:

		QTSSSocket(int inFileDesc) : fEventContext(inFileDesc, Socket::GetEventThread()) {}
		virtual ~QTSSSocket() {}
		
		//
		// The only operation this stream supports is the requesting of events.
		virtual QTSS_Error	RequestEvent(QTSS_EventType inEventMask);
		
	private:
	
		EventContext fEventContext;
};

#endif //__QTSS_SOCKET_H__

