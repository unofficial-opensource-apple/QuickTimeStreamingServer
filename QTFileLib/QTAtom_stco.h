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
// $Id: QTAtom_stco.h,v 1.6 2001/11/20 01:08:47 murata Exp $
//
// QTAtom_stco:
//   The 'stco' QTAtom class.

#ifndef QTAtom_stco_H
#define QTAtom_stco_H


//
// Includes
#ifndef __Win32__
#include <netinet/in.h>
#endif
#include "QTFile.h"
#include "QTAtom.h"


//
// QTAtom class
class QTAtom_stco : public QTAtom {

public:
	//
	// Constructors and destructor.
						QTAtom_stco(QTFile * File, QTFile::AtomTOCEntry * Atom,
						UInt16 offSetSize = 4, Bool16 Debug = false, Bool16 DeepDebug = false);
	virtual				~QTAtom_stco(void);


	//
	// Initialization functions.
	virtual	Bool16		Initialize(void);
	
	//
	// Accessors.
							
	inline	Bool16		ChunkOffset(UInt32 ChunkNumber, UInt64 *Offset = NULL)
						{
							if (Offset && ChunkNumber && (ChunkNumber<=fNumEntries)) 
							{
								if (4 == fOffSetSize)
									*Offset = (UInt64) ntohl( ( (UInt32 *) fTable)[ChunkNumber-1]);
								else
									*Offset = (UInt64) QTAtom::NTOH64( ( (UInt64 *) fTable)[ChunkNumber-1]);
										
								return true;
							} 
							
							return false; 
						}


	//
	// Debugging functions.
	virtual	void		DumpAtom(void);
	virtual	void		DumpTable(void);


protected:
	//
	// Protected member variables.
	UInt8		fVersion;
	UInt32		fFlags; // 24 bits in the low 3 bytes

	UInt32		fNumEntries;
	UInt16		fOffSetSize;
	char		*fChunkOffsetTable;
	void		*fTable; // longword-aligned version of the above
};

#endif // QTAtom_stco_H
