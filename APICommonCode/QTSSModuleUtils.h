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
	File:		QTSSModuleUtils.h

	Contains:	Utility routines for modules to use.
					
*/


#ifndef _QTSS_MODULE_UTILS_H_
#define _QTSS_MODULE_UTILS_H_

#include <stdlib.h>
#include "QTSS.h"
#include "StrPtrLen.h"
#include "RTPMetaInfoPacket.h"

#include "OSMemory.h"
#include "ResizeableStringFormatter.h"


class QTSSModuleUtils
{
	public:
	
		static void		Initialize(	QTSS_TextMessagesObject inMessages,
									QTSS_ServerObject inServer,
									QTSS_StreamRef inErrorLog);
	
		// Read the complete contents of the file at inPath into the StrPtrLen.
		// This function allocates memory for the file data.
		static QTSS_Error 	ReadEntireFile(char* inPath, StrPtrLen* outData, QTSS_TimeVal inModDate = -1, QTSS_TimeVal* outModDate = NULL);

		// If your module supports RTSP methods, call this function from your QTSS_Initialize
		// role to tell the server what those methods are.
		static void		SetupSupportedMethods(	QTSS_Object inServer,
												QTSS_RTSPMethod* inMethodArray,
												UInt32 inNumMethods);
												
		// Using a message out of the text messages dictionary is a common
		// way to log errors to the error log. Here is a function to
		// make that process very easy.
		
		static void 	LogError(	QTSS_ErrorVerbosity inVerbosity,
									QTSS_AttributeID inTextMessage,
									UInt32 inErrNumber,
									char* inArgument = NULL,
									char* inArg2 = NULL);
									
		// This function constructs a C-string of the full path to the file being requested.
		// You may opt to append an optional suffix, or pass in NULL. You are responsible
		// for disposing this memory

		static char* GetFullPath(	QTSS_RTSPRequestObject inRequest,
									QTSS_AttributeID whichFileType,
									UInt32* outLen,
									StrPtrLen* suffix = NULL);

		//
		// This function does 2 things:
		// 1. 	Compares the enabled fields in the field ID array with the fields in the
		//		x-RTP-Meta-Info header. Turns off the fields in the array that aren't in the request.
		//
		// 2.	Appends the x-RTP-Meta-Info header to the response, using the proper
		//		fields from the array, as well as the IDs provided in the array
		static QTSS_Error	AppendRTPMetaInfoHeader( QTSS_RTSPRequestObject inRequest,
														StrPtrLen* inRTPMetaInfoHeader,
														RTPMetaInfoPacket::FieldID* inFieldIDArray);

		// This function sends an error to the RTSP client. You must provide a
		// status code for the error, and a text message ID to describe the error.
		//
		// It always returns QTSS_RequestFailed.

		static QTSS_Error	SendErrorResponse(	QTSS_RTSPRequestObject inRequest,
														QTSS_RTSPStatusCode inStatusCode,
														QTSS_AttributeID inTextMessage,
														StrPtrLen* inStringArg = NULL);

		// This function sends an error to the RTSP client. You don't have to provide
		// a text message ID, but instead you need to provide the error message in a
		// string
		// 
		// It always returns QTSS_RequestFailed
		static QTSS_Error	SendErrorResponseWithMessage( QTSS_RTSPRequestObject inRequest,
														QTSS_RTSPStatusCode inStatusCode,
														StrPtrLen* inErrorMessageStr);

		//Modules most certainly don't NEED to use this function, but it is awfully handy
		//if they want to take advantage of it. Using the SDP data provided in the iovec,
		//this function sends a standard describe response.
		//NOTE: THE FIRST ENTRY OF THE IOVEC MUST BE EMPTY!!!!
		static void	SendDescribeResponse(QTSS_RTSPRequestObject inRequest,
													QTSS_ClientSessionObject inSession,
													iovec* describeData,
													UInt32 inNumVectors,
													UInt32 inTotalLength);

                
                // Called by SendDescribeResponse to coalesce iovec to a buffer
                // Allocates memory - remember to delete it!
                static char* CoalesceVectors(iovec* inVec, UInt32 inNumVectors, UInt32 inTotalLength);
                                                                                                        											
		//
		// SEARCH FOR A SPECIFIC MODULE OBJECT							
		static QTSS_ModulePrefsObject GetModuleObjectByName(const StrPtrLen& inModuleName);
		
		//
		// GET MODULE PREFS OBJECT
		static QTSS_ModulePrefsObject GetModulePrefsObject(QTSS_ModuleObject inModObject);
		
		// GET MODULE ATTRIBUTES OBJECT
		static QTSS_Object GetModuleAttributesObject(QTSS_ModuleObject inModObject);
		
		//
		// GET ATTRIBUTE
		//
		// This function retrieves an attribute 
		// (from any QTSS_Object, including the QTSS_ModulePrefsObject)
		// with the specified name and type
		// out of the specified object.
		//
		// Caller should pass in a buffer for ioBuffer that is large enough
		// to hold the attribute value. inBufferLen should be set to the length
		// of this buffer.
		//
		// Pass in a buffer containing a default value to use for the attribute
		// in the inDefaultValue parameter. If the attribute isn't found, or is
		// of the wrong type, the default value will be copied into ioBuffer.
		// Also, this function adds the default value to object if it is not
		// found or is of the wrong type. If no default value is provided, the
		// attribute is still added but no value is assigned to it.
		//
		// Pass in NULL for the default value or 0 for the default value length if it is not known.
		//
		// This function logs an error if there was a default value provided.
		static void	GetAttribute(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType,
							void* ioBuffer, void* inDefaultValue, UInt32 inBufferLen);
							
		static void	GetIOAttribute(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType,
							void* ioDefaultResultBuffer, UInt32 inBufferLen);
		//
		// GET STRING ATTRIBUTE
		//
		// Does the same thing as GetAttribute, but does it for string attribute. Returns a newly
		// allocated buffer with the attribute value inside it.
		//
		// Pass in NULL for the default value or an empty string if the default is not known.
		static char* GetStringAttribute(QTSS_Object inObject, char* inAttributeName, char* inDefaultValue);

		//
		// GET ATTR ID
		//
		// Given an attribute in an object, returns its attribute ID
		// or qtssIllegalAttrID if it isn't found.
		static QTSS_AttributeID GetAttrID(QTSS_Object inObject, char* inAttributeName);
		
		//
		//
		//
		/// Get the type of request. Returns qtssActionFlagsNoFlags on failure.
		//	Result is a bitmap of flags
		//
		static QTSS_ActionFlags GetRequestActions(QTSS_RTSPRequestObject theRTSPRequest);

		static char* GetLocalPath_Copy(QTSS_RTSPRequestObject theRTSPRequest);
		static char* GetMoviesRootDir_Copy(QTSS_RTSPRequestObject theRTSPRequest);
		static QTSS_UserProfileObject GetUserProfileObject(QTSS_RTSPRequestObject theRTSPRequest);
		
		static char*  GetUserName_Copy(QTSS_UserProfileObject inUserProfile);
		static char** GetGroupsArray_Copy(QTSS_UserProfileObject inUserProfile, UInt32 *outNumGroupsPtr);
		static void SetEnableRTSPErrorMsg(Bool16 enable) {QTSSModuleUtils::sEnableRTSPErrorMsg = enable; }
		
		static QTSS_AttributeID	CreateAttribute(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType, void* inDefaultValue, UInt32 inBufferLen);
	
	private:
	
		//
		// Used in the implementation of the above functions
		static QTSS_AttributeID CheckAttributeDataType(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType, void* inDefaultValue, UInt32 inBufferLen);	

		static QTSS_TextMessagesObject 	sMessages;
		static QTSS_ServerObject 		sServer;
		static QTSS_StreamRef 			sErrorLog;
		static Bool16					sEnableRTSPErrorMsg;
};

class SDPLine : public StrPtrLen
{
public:
	SDPLine() : fHeaderType('\0') {}
    virtual ~SDPLine() {}

    char    fHeaderType;
};

class SDPContainer
{
    enum { kBaseLines = 20 };

public:

    SDPContainer(UInt32 numStrPtrs = SDPContainer::kBaseLines) : fCurrentLine(0),fNumSDPLines(numStrPtrs),
	fNumUsedLines(0), fSDPLineArray(NULL), fValid(false)
    { fSDPLineArray = NEW SDPLine[fNumSDPLines]; }

    ~SDPContainer() {delete [] fSDPLineArray;}
	void		Initialize();
    SInt32      AddHeaderLine (StrPtrLen *theLinePtr);
    SInt32      FindHeaderLineType(char id, SInt32 start);
    SDPLine*    GetNextLine();
    SDPLine*    GetLine(SInt32 lineIndex);
    void        SetLine(SInt32 index);
    void        Parse();
    Bool16      SetSDPBuffer(char *sdpBuffer);
    Bool16      SetSDPBuffer(StrPtrLen *sdpBufferPtr);
    Bool16      IsSDPBufferValid() {return fValid;}
    void        PrintLine(SInt32 lineIndex);
    void        PrintAllLines();
    SInt32      GetNumLines() { return  fNumUsedLines; }
    
    SInt32      fCurrentLine;
    SInt32      fNumSDPLines;
    SInt32      fNumUsedLines;
    SDPLine*    fSDPLineArray;
    Bool16      fValid;
    StrPtrLen   fSDPBuffer;

};

class SDPLineSorter {

public:
	SDPLineSorter(): fSessionLineCount(0),fSDPSessionHeaders(NULL,0) {};
	SDPLineSorter(SDPContainer *rawSDPContainerPtr);
	
	StrPtrLen* GetSessionHeaders() { return &fSessionHeaders; }
	StrPtrLen* GetMediaHeaders() { return &fMediaHeaders; }
	char* GetSortedSDPCopy();
	
	StrPtrLen fullSDPBuffSPL;
	SInt32 fSessionLineCount;
	SDPContainer fSessionSDPContainer;
	ResizeableStringFormatter fSDPSessionHeaders;
	StrPtrLen fSessionHeaders;
	StrPtrLen fMediaHeaders;
	static char sSessionOrderedLines[];// = "vosiuepcbtrzka"; // chars are order dependent: declared by rfc 2327
	static char sessionSingleLines[];//  = "vosiuepcbzk";    // return only 1 of each of these session field types
	static StrPtrLen sEOL;
};


#endif //_QTSS_MODULE_UTILS_H_
