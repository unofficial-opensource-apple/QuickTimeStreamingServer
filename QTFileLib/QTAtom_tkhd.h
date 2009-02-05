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
// $Id: QTAtom_tkhd.h,v 1.4.18.1 2002/11/27 10:14:03 murata Exp $
//
// QTAtom_tkhd:
//   The 'tkhd' QTAtom class.

#ifndef QTAtom_tkhd_H
#define QTAtom_tkhd_H


//
// Includes
#include "OSHeaders.h"

#include "QTFile.h"
#include "QTAtom.h"


//
// QTAtom class
class QTAtom_tkhd : public QTAtom {
	//
	// Class constants
	enum {
		flagEnabled		= 0x00000001,
		flagInMovie		= 0x00000002,
		flagInPreview	= 0x00000004,
		flagInPoster	= 0x00000008
	};


public:
	//
	// Constructors and destructor.
						QTAtom_tkhd(QTFile * File, QTFile::AtomTOCEntry * Atom,
							   Bool16 Debug = false, Bool16 DeepDebug = false);
	virtual				~QTAtom_tkhd(void);


	//
	// Initialization functions.
	virtual	Bool16		Initialize(void);

	//
	// Accessors.
	inline	UInt32		GetTrackID(void) { return fTrackID; }
	inline	UInt32		GetFlags(void) { return fFlags; }
	inline	UInt64		GetCreationTime(void) { return fCreationTime; }
	inline	UInt64		GetModificationTime(void) { return fModificationTime; }
	inline	UInt64		GetDuration(void) { return fDuration; }


	//
	// Debugging functions.
	virtual	void		DumpAtom(void);


protected:
	//
	// Protected member variables.
	UInt8		fVersion;
	UInt32		fFlags; // 24 bits in the low 3 bytes
	UInt64		fCreationTime, fModificationTime;
	UInt32		fTrackID;
	UInt32		freserved1;
	UInt64		fDuration;
	UInt32		freserved2, freserved3;
	UInt16		fLayer, fAlternateGroup;
	UInt16		fVolume;
	UInt16		freserved4;
	UInt32		fa, fb, fu, fc, fd, fv, fx, fy, fw;
	UInt32		fTrackWidth, fTrackHeight;
};

#endif // QTAtom_tkhd_H
