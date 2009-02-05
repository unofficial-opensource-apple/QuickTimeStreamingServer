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
// $Id: QTAtom.h,v 1.5.16.1 2002/11/27 10:14:02 murata Exp $
//
// QTAtom:
//   The base-class for atoms in a QuickTime file.

#ifndef QTAtom_H
#define QTAtom_H


//
// Includes
#include "OSHeaders.h"

#include "QTFile.h"


//
// QTAtom class
class QTAtom {

public:
	//
	// Constructors and destructor.
						QTAtom(QTFile * File, QTFile::AtomTOCEntry * Atom,
							   Bool16 Debug = false, Bool16 DeepDebug = false);
	virtual				~QTAtom(void);


	//
	// Initialization functions.
	virtual	Bool16		Initialize(void) { return true; }

	static SInt64	QTAtom::NTOH64(SInt64 networkOrdered)
	{
	#if BIGENDIAN
		return networkOrdered;
	#else
		return (SInt64) (  (UInt64)  (networkOrdered << 56) | (UInt64)  (((UInt64) 0x00ff0000 << 32) & (networkOrdered << 40))
			| (UInt64)  ( ((UInt64)  0x0000ff00 << 32) & (networkOrdered << 24)) | (UInt64)  (((UInt64)  0x000000ff << 32) & (networkOrdered << 8))
			| (UInt64)  ( ((UInt64)  0x00ff0000 << 8) & (networkOrdered >> 8)) | (UInt64)     ((UInt64)  0x00ff0000 & (networkOrdered >> 24))
			| (UInt64)  (  (UInt64)  0x0000ff00 & (networkOrdered >> 40)) | (UInt64)  ((UInt64)  0x00ff & (networkOrdered >> 56)) );
	#endif
	}

	//
	// Read functions.
			Bool16		ReadBytes(UInt64 Offset, char * Buffer, UInt32 Length);
			Bool16		ReadInt8(UInt64 Offset, UInt8 * Datum);
			Bool16		ReadInt16(UInt64 Offset, UInt16 * Datum);
			Bool16		ReadInt32(UInt64 Offset, UInt32 * Datum);
			Bool16		ReadInt64(UInt64 Offset, UInt64 * Datum);
			Bool16		ReadInt32To64(UInt64 Offset, UInt64 * Datum);
			
			Bool16		ReadSubAtomBytes(const char * AtomPath, char * Buffer, UInt32 Length);
			Bool16		ReadSubAtomInt8(const char * AtomPath, UInt8 * Datum);
			Bool16		ReadSubAtomInt16(const char * AtomPath, UInt16 * Datum);
			Bool16		ReadSubAtomInt32(const char * AtomPath, UInt32 * Datum);
			Bool16		ReadSubAtomInt64(const char * AtomPath, UInt64 * Datum);
			
	//
	// Debugging functions.
	virtual	void		DumpAtom(void) {}


protected:
	//
	// Protected member variables.
	Bool16				fDebug, fDeepDebug;
	QTFile				*fFile;

	QTFile::AtomTOCEntry fTOCEntry;
};

#endif // QTAtom_H
