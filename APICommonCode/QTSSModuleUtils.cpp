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
	File:		QTSSModuleUtils.cpp

	Contains:	Implements utility routines defined in QTSSModuleUtils.h.
					
*/

#include "QTSSModuleUtils.h"
#include "QTSS_Private.h"

#include "StrPtrLen.h"
#include "OSArrayObjectDeleter.h"
#include "OSMemory.h"
#include "MyAssert.h"
#include "StringFormatter.h"
#include "ResizeableStringFormatter.h"
#include "QTAccessFile.h"
#include "StringParser.h"
#include "OSMemory.h"

#ifndef __Win32__
#include <netinet/in.h>
#endif

#ifdef __solaris__
#include <limits.h>
#endif

QTSS_TextMessagesObject 	QTSSModuleUtils::sMessages = NULL;
QTSS_ServerObject 			QTSSModuleUtils::sServer = NULL;
QTSS_StreamRef 				QTSSModuleUtils::sErrorLog = NULL;
Bool16						QTSSModuleUtils::sEnableRTSPErrorMsg = false;

void	QTSSModuleUtils::Initialize(QTSS_TextMessagesObject inMessages,
									QTSS_ServerObject inServer,
									QTSS_StreamRef inErrorLog)
{
	sMessages = inMessages;
	sServer = inServer;
	sErrorLog = inErrorLog;
}

QTSS_Error QTSSModuleUtils::ReadEntireFile(char* inPath, StrPtrLen* outData, QTSS_TimeVal inModDate, QTSS_TimeVal* outModDate)
{	
	
	QTSS_Object theFileObject = NULL;
	QTSS_Error theErr = QTSS_NoErr;
	
	outData->Ptr = NULL;
	outData->Len = 0;
	
	do { 

		// Use the QTSS file system API to read the file
		theErr = QTSS_OpenFileObject(inPath, 0, &theFileObject);
		if (theErr != QTSS_NoErr)
			break;
	
		UInt32 theParamLen = 0;
		QTSS_TimeVal* theModDate = NULL;
		theErr = QTSS_GetValuePtr(theFileObject, qtssFlObjModDate, 0, (void**)&theModDate, &theParamLen);
		Assert(theParamLen == sizeof(QTSS_TimeVal));
		if(theParamLen != sizeof(QTSS_TimeVal))
			break;
		if(outModDate != NULL)
			*outModDate = (QTSS_TimeVal)*theModDate;

		if(inModDate != -1) {	
			// If file hasn't been modified since inModDate, don't have to read the file
			if(*theModDate <= inModDate)
				break;
		}
		
		theParamLen = 0;
		UInt64* theLength = NULL;
		theErr = QTSS_GetValuePtr(theFileObject, qtssFlObjLength, 0, (void**)&theLength, &theParamLen);
		if (theParamLen != sizeof(UInt64))
			break;
		
	
		// Allocate memory for the file data
		outData->Ptr = NEW char[*theLength + 1];
		outData->Len = *theLength;
		outData->Ptr[outData->Len] = 0;
	
		// Read the data
		UInt32 recvLen = 0;
		theErr = QTSS_Read(theFileObject, outData->Ptr, outData->Len, &recvLen);
		if (theErr != QTSS_NoErr)
		{
			delete [] outData->Ptr;
			outData->Len = 0;
			break;
		}	
		Assert(outData->Len == recvLen);
	
	}while(false);
	
	// Close the file
	if(theFileObject != NULL) {
		theErr = QTSS_CloseFileObject(theFileObject);
	}
	
	return theErr;
}

void	QTSSModuleUtils::SetupSupportedMethods(QTSS_Object inServer, QTSS_RTSPMethod* inMethodArray, UInt32 inNumMethods)
{
	// Report to the server that this module handles DESCRIBE, SETUP, PLAY, PAUSE, and TEARDOWN
	UInt32 theNumMethods = 0;
	(void)QTSS_GetNumValues(inServer, qtssSvrHandledMethods, &theNumMethods);
	
	for (UInt32 x = 0; x < inNumMethods; x++)
		(void)QTSS_SetValue(inServer, qtssSvrHandledMethods, theNumMethods++, (void*)&inMethodArray[x], sizeof(inMethodArray[x]));
}

void 	QTSSModuleUtils::LogError(	QTSS_ErrorVerbosity inVerbosity,
									QTSS_AttributeID inTextMessage,
									UInt32 /*inErrNumber*/,
									char* inArgument,
									char* inArg2)
{
	static char* sEmptyArg = "";
	
	if (sMessages == NULL)
		return;
		
	// Retrieve the specified text message from the text messages dictionary.
	
	StrPtrLen theMessage;
	(void)QTSS_GetValuePtr(sMessages, inTextMessage, 0, (void**)&theMessage.Ptr, &theMessage.Len);
	if ((theMessage.Ptr == NULL) || (theMessage.Len == 0))
		(void)QTSS_GetValuePtr(sMessages, qtssMsgNoMessage, 0, (void**)&theMessage.Ptr, &theMessage.Len);

	if ((theMessage.Ptr == NULL) || (theMessage.Len == 0))
		return;
	
	// ::sprintf and ::strlen will crash if inArgument is NULL
	if (inArgument == NULL)
		inArgument = sEmptyArg;
	if (inArg2 == NULL)
		inArg2 = sEmptyArg;
	
	// Create a new string, and put the argument into the new string.
	
	UInt32 theMessageLen = theMessage.Len + ::strlen(inArgument) + ::strlen(inArg2);

	OSCharArrayDeleter theLogString(NEW char[theMessageLen + 1]);
	::sprintf(theLogString.GetObject(), theMessage.Ptr, inArgument, inArg2);
	Assert(theMessageLen >= ::strlen(theLogString.GetObject()));
	
	(void)QTSS_Write(sErrorLog, theLogString.GetObject(), ::strlen(theLogString.GetObject()),
						NULL, inVerbosity);
}


char* QTSSModuleUtils::GetFullPath(	QTSS_RTSPRequestObject inRequest,
									QTSS_AttributeID whichFileType,
									UInt32* outLen,
									StrPtrLen* suffix)
{
	Assert(outLen != NULL);
	
	// Get the proper file path attribute. This may return an error if
	// the file type is qtssFilePathTrunc attr, because there may be no path
	// once its truncated. That's ok. In that case, we just won't append a path.
	StrPtrLen theFilePath;
	(void)QTSS_GetValuePtr(inRequest, whichFileType, 0, (void**)&theFilePath.Ptr, &theFilePath.Len);

	StrPtrLen theRootDir;
	QTSS_Error theErr = QTSS_GetValuePtr(inRequest, qtssRTSPReqRootDir, 0, (void**)&theRootDir.Ptr, &theRootDir.Len);
	Assert(theErr == QTSS_NoErr);

	//construct a full path out of the root dir path for this request,
	//and the url path.
	*outLen = theFilePath.Len + theRootDir.Len + 2;
	if (suffix != NULL)
		*outLen += suffix->Len;
	
	char* theFullPath = NEW char[*outLen];
	
	//write all the pieces of the path into this new buffer.
	StringFormatter thePathFormatter(theFullPath, *outLen);
	thePathFormatter.Put(theRootDir);
	thePathFormatter.Put(theFilePath);
	if (suffix != NULL)
		thePathFormatter.Put(*suffix);
	thePathFormatter.PutTerminator();

	*outLen = *outLen - 2;
	return theFullPath;
}

QTSS_Error	QTSSModuleUtils::AppendRTPMetaInfoHeader( 	QTSS_RTSPRequestObject inRequest,
														StrPtrLen* inRTPMetaInfoHeader,
														RTPMetaInfoPacket::FieldID* inFieldIDArray)
{
	//
	// For formatting the response header
	char tempBuffer[128];
	ResizeableStringFormatter theFormatter(tempBuffer, 128);
	
	StrPtrLen theHeader(*inRTPMetaInfoHeader);
	
	//
	// For marking which fields were requested by the client
	Bool16 foundFieldArray[RTPMetaInfoPacket::kNumFields];
	::memset(foundFieldArray, 0, sizeof(Bool16) * RTPMetaInfoPacket::kNumFields);
	
	char* theEndP = theHeader.Ptr + theHeader.Len;
	UInt16 fieldNameValue = 0;
	
	while (theHeader.Ptr <= (theEndP - sizeof(RTPMetaInfoPacket::FieldName)))
	{
		RTPMetaInfoPacket::FieldName* theFieldName = (RTPMetaInfoPacket::FieldName*)theHeader.Ptr;
		::memcpy (&fieldNameValue, theFieldName, sizeof(UInt16));

		RTPMetaInfoPacket::FieldIndex theFieldIndex = RTPMetaInfoPacket::GetFieldIndexForName(ntohs(fieldNameValue));
		
		//
		// This field is not supported (not in the field ID array), so
		// don't put it in the response
		if ((theFieldIndex == RTPMetaInfoPacket::kIllegalField) ||
			(inFieldIDArray[theFieldIndex] == RTPMetaInfoPacket::kFieldNotUsed))
		{
			theHeader.Ptr += 3;
			continue;
		}
		
		//
		// Mark that this field has been requested by the client
		foundFieldArray[theFieldIndex] = true;
		
		//
		// This field is good to go... put it in the response	
		theFormatter.Put(theHeader.Ptr, sizeof(RTPMetaInfoPacket::FieldName));
		
		if (inFieldIDArray[theFieldIndex] != RTPMetaInfoPacket::kUncompressed)
		{
			//
			// If the caller wants this field to be compressed (there
			// is an ID associated with the field), put the ID in the response
			theFormatter.PutChar('=');
			theFormatter.Put(inFieldIDArray[theFieldIndex]);
		}
		
		//
		// Field separator
		theFormatter.PutChar(';');
			
		//
		// Skip onto the next field name in the header
		theHeader.Ptr += 3;
	}

	//
	// Go through the caller's FieldID array, and turn off the fields
	// that were not requested by the client.
	for (UInt32 x = 0; x < RTPMetaInfoPacket::kNumFields; x++)
	{
		if (!foundFieldArray[x])
			inFieldIDArray[x] = RTPMetaInfoPacket::kFieldNotUsed;
	}
	
	//
	// No intersection between requested headers and supported headers!
	if (theFormatter.GetCurrentOffset() == 0)
		return QTSS_ValueNotFound; // Not really the greatest error!
		
	//
	// When appending the header to the response, strip off the last ';'.
	// It's not needed.
	return QTSS_AppendRTSPHeader(inRequest, qtssXRTPMetaInfoHeader, theFormatter.GetBufPtr(), theFormatter.GetCurrentOffset() - 1);
}

QTSS_Error	QTSSModuleUtils::SendErrorResponse(	QTSS_RTSPRequestObject inRequest,
														QTSS_RTSPStatusCode inStatusCode,
														QTSS_AttributeID inTextMessage,
														StrPtrLen* inStringArg)
{
	static Bool16 sFalse = false;
	
	//set RTSP headers necessary for this error response message
	(void)QTSS_SetValue(inRequest, qtssRTSPReqStatusCode, 0, &inStatusCode, sizeof(inStatusCode));
	(void)QTSS_SetValue(inRequest, qtssRTSPReqRespKeepAlive, 0, &sFalse, sizeof(sFalse));
	StringFormatter theErrorMsgFormatter(NULL, 0);
	char *messageBuffPtr = NULL;
	
	if (sEnableRTSPErrorMsg)
	{
		// Retrieve the specified message out of the text messages dictionary.
		StrPtrLen theMessage;
		(void)QTSS_GetValuePtr(sMessages, inTextMessage, 0, (void**)&theMessage.Ptr, &theMessage.Len);

		if ((theMessage.Ptr == NULL) || (theMessage.Len == 0))
		{
			// If we couldn't find the specified message, get the default
			// "No Message" message, and return that to the client instead.
			
			(void)QTSS_GetValuePtr(sMessages, qtssMsgNoMessage, 0, (void**)&theMessage.Ptr, &theMessage.Len);
		}
		Assert(theMessage.Ptr != NULL);
		Assert(theMessage.Len > 0);
		
		// Allocate a temporary buffer for the error message, and format the error message
		// into that buffer
		UInt32 theMsgLen = 256;
		if (inStringArg != NULL)
			theMsgLen += inStringArg->Len;
		
		messageBuffPtr = NEW char[theMsgLen];
		messageBuffPtr[0] = 0;
		theErrorMsgFormatter.Set(messageBuffPtr, theMsgLen);
		//
		// Look for a %s in the string, and if one exists, replace it with the
		// argument passed into this function.
		
		//we can safely assume that message is in fact NULL terminated
		char* stringLocation = ::strstr(theMessage.Ptr, "%s");
		if (stringLocation != NULL)
		{
			//write first chunk
			theErrorMsgFormatter.Put(theMessage.Ptr, stringLocation - theMessage.Ptr);
			
			if (inStringArg != NULL && inStringArg->Len > 0)
			{
				//write string arg if it exists
				theErrorMsgFormatter.Put(inStringArg->Ptr, inStringArg->Len);
				stringLocation += 2;
			}
			//write last chunk
			theErrorMsgFormatter.Put(stringLocation, (theMessage.Ptr + theMessage.Len) - stringLocation);
		}
		else
			theErrorMsgFormatter.Put(theMessage);
		
		
		char buff[32];
		::sprintf(buff,"%lu",theErrorMsgFormatter.GetBytesWritten());
		(void)QTSS_AppendRTSPHeader(inRequest, qtssContentLengthHeader, buff, ::strlen(buff));
	}
	
	//send the response header. In all situations where errors could happen, we
	//don't really care, cause there's nothing we can do anyway!
	(void)QTSS_SendRTSPHeaders(inRequest);

	//
	// Now that we've formatted the message into the temporary buffer,
	// write it out to the request stream and the Client Session object
	(void)QTSS_Write(inRequest, theErrorMsgFormatter.GetBufPtr(), theErrorMsgFormatter.GetBytesWritten(), NULL, 0);
	(void)QTSS_SetValue(inRequest, qtssRTSPReqRespMsg, 0, theErrorMsgFormatter.GetBufPtr(), theErrorMsgFormatter.GetBytesWritten());
	
	delete [] messageBuffPtr;
	return QTSS_RequestFailed;
}

QTSS_Error	QTSSModuleUtils::SendErrorResponseWithMessage( QTSS_RTSPRequestObject inRequest,
														QTSS_RTSPStatusCode inStatusCode,
														StrPtrLen* inErrorMessagePtr)
{
    static Bool16 sFalse = false;
    
    //set RTSP headers necessary for this error response message
    (void)QTSS_SetValue(inRequest, qtssRTSPReqStatusCode, 0, &inStatusCode, sizeof(inStatusCode));
    (void)QTSS_SetValue(inRequest, qtssRTSPReqRespKeepAlive, 0, &sFalse, sizeof(sFalse));
    StrPtrLen theErrorMessage(NULL, 0);
    
    if (sEnableRTSPErrorMsg)
    {
		Assert(inErrorMessagePtr != NULL);
		//Assert(inErrorMessagePtr->Ptr != NULL);
		//Assert(inErrorMessagePtr->Len != 0);
		theErrorMessage.Set(inErrorMessagePtr->Ptr, inErrorMessagePtr->Len);
		
        char buff[32];
        sprintf(buff,"%lu",inErrorMessagePtr->Len);
        (void)QTSS_AppendRTSPHeader(inRequest, qtssContentLengthHeader, buff, ::strlen(buff));
    }
    
    //send the response header. In all situations where errors could happen, we
    //don't really care, cause there's nothing we can do anyway!
    (void)QTSS_SendRTSPHeaders(inRequest);

    //
    // Now that we've formatted the message into the temporary buffer,
    // write it out to the request stream and the Client Session object
    (void)QTSS_Write(inRequest, theErrorMessage.Ptr, theErrorMessage.Len, NULL, 0);
    (void)QTSS_SetValue(inRequest, qtssRTSPReqRespMsg, 0, theErrorMessage.Ptr, theErrorMessage.Len);
    
    return QTSS_RequestFailed;
}

void	QTSSModuleUtils::SendDescribeResponse(QTSS_RTSPRequestObject inRequest,
													QTSS_ClientSessionObject inSession,
													iovec* describeData,
													UInt32 inNumVectors,
													UInt32 inTotalLength)
{
	//write content size header
	char buf[32];
	::sprintf(buf, "%ld", inTotalLength);
	(void)QTSS_AppendRTSPHeader(inRequest, qtssContentLengthHeader, &buf[0], ::strlen(&buf[0]));

	(void)QTSS_SendStandardRTSPResponse(inRequest, inSession, 0);

        // On solaris, the maximum # of vectors is very low (= 16) so to ensure that we are still able to
        // send the SDP if we have a number greater than the maximum allowed, we coalesce the vectors into
        // a single big buffer
#ifdef __solaris__
	if (inNumVectors > IOV_MAX )
	{
        	char* describeDataBuffer = QTSSModuleUtils::CoalesceVectors(describeData, inNumVectors, inTotalLength);
        	(void)QTSS_Write(inRequest, (void *)describeDataBuffer, inTotalLength, NULL, qtssWriteFlagsNoFlags);
        	// deleting memory allocated by the CoalesceVectors call
        	delete [] describeDataBuffer;
	}
	else
		(void)QTSS_WriteV(inRequest, describeData, inNumVectors, inTotalLength, NULL);
#else
	(void)QTSS_WriteV(inRequest, describeData, inNumVectors, inTotalLength, NULL);
#endif

}

char*	QTSSModuleUtils::CoalesceVectors(iovec* inVec, UInt32 inNumVectors, UInt32 inTotalLength)
{
    if (inTotalLength == 0)
        return NULL;
    
    char* buffer = NEW char[inTotalLength];
    UInt32 bufferOffset = 0;
    
    for (UInt32 index = 0; index < inNumVectors; index++)
    {
        ::memcpy (buffer + bufferOffset, inVec[index].iov_base, inVec[index].iov_len);
        bufferOffset += inVec[index].iov_len;
    }
    
    Assert (bufferOffset == inTotalLength);
    
    return buffer;
}

QTSS_ModulePrefsObject QTSSModuleUtils::GetModulePrefsObject(QTSS_ModuleObject inModObject)
{
	QTSS_ModulePrefsObject thePrefsObject = NULL;
	UInt32 theLen = sizeof(thePrefsObject);
	QTSS_Error theErr = QTSS_GetValue(inModObject, qtssModPrefs, 0, &thePrefsObject, &theLen);
	Assert(theErr == QTSS_NoErr);
	
	return thePrefsObject;
}

QTSS_Object QTSSModuleUtils::GetModuleAttributesObject(QTSS_ModuleObject inModObject)
{
	QTSS_Object theAttributesObject = NULL;
	UInt32 theLen = sizeof(theAttributesObject);
	QTSS_Error theErr = QTSS_GetValue(inModObject, qtssModAttributes, 0, &theAttributesObject, &theLen);
	Assert(theErr == QTSS_NoErr);
	
	return theAttributesObject;
}

QTSS_ModulePrefsObject QTSSModuleUtils::GetModuleObjectByName(const StrPtrLen& inModuleName)
{
	QTSS_ModuleObject theModule = NULL;
	UInt32 theLen = sizeof(theModule);
	
	for (int x = 0; QTSS_GetValue(sServer, qtssSvrModuleObjects, x, &theModule, &theLen) == QTSS_NoErr; x++)
	{
		Assert(theModule != NULL);
		Assert(theLen == sizeof(theModule));
		
		StrPtrLen theName;
		QTSS_Error theErr = QTSS_GetValuePtr(theModule, qtssModName, 0, (void**)&theName.Ptr, &theName.Len);
		Assert(theErr == QTSS_NoErr);
		
		if (inModuleName.Equal(theName))
			return theModule;
			
#if DEBUG
		theModule = NULL;
		theLen = sizeof(theModule);
#endif
	}
	return NULL;
}

void	QTSSModuleUtils::GetAttribute(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType, 
												void* ioBuffer, void* inDefaultValue, UInt32 inBufferLen)
{
	//
	// Check to make sure this attribute is the right type. If it's not, this will coerce
	// it to be the right type. This also returns the id of the attribute
	QTSS_AttributeID theID = QTSSModuleUtils::CheckAttributeDataType(inObject, inAttributeName, inType, inDefaultValue, inBufferLen);

	//
	// Get the attribute value.
	QTSS_Error theErr = QTSS_GetValue(inObject, theID, 0, ioBuffer, &inBufferLen);
	
	//
	// Caller should KNOW how big this attribute is
	Assert(theErr != QTSS_NotEnoughSpace);
	
	if (theErr != QTSS_NoErr)
	{
		//
		// If we couldn't get the attribute value for whatever reason, just use the
		// default if it was provided.
		::memcpy(ioBuffer, inDefaultValue, inBufferLen);

		if (inBufferLen > 0)
		{
			//
			// Log an error for this pref only if there was a default value provided.
			char* theValueAsString = NULL;
			theErr = QTSS_ValueToString(inDefaultValue, inBufferLen, inType, &theValueAsString);
			Assert(theErr == QTSS_NoErr);
			OSCharArrayDeleter theValueStr(theValueAsString);
			QTSSModuleUtils::LogError( 	qtssWarningVerbosity,
										qtssServerPrefMissing,
										0,
										inAttributeName,
										theValueStr.GetObject());
		}
		
		//
		// Create an entry for this attribute							
		QTSSModuleUtils::CreateAttribute(inObject, inAttributeName, inType, inDefaultValue, inBufferLen);
	}
}

char*	QTSSModuleUtils::GetStringAttribute(QTSS_Object inObject, char* inAttributeName, char* inDefaultValue)
{
	UInt32 theDefaultValLen = 0;
	if (inDefaultValue != NULL)
		theDefaultValLen = ::strlen(inDefaultValue);
	
	//
	// Check to make sure this attribute is the right type. If it's not, this will coerce
	// it to be the right type
	QTSS_AttributeID theID = QTSSModuleUtils::CheckAttributeDataType(inObject, inAttributeName, qtssAttrDataTypeCharArray, inDefaultValue, theDefaultValLen);

	char* theString = NULL;
	(void)QTSS_GetValueAsString(inObject, theID, 0, &theString);
	if (theString != NULL)
		return theString;
	
	//
	// If we get here the attribute must be missing, so create it and log
	// an error.
	
	QTSSModuleUtils::CreateAttribute(inObject, inAttributeName, qtssAttrDataTypeCharArray, inDefaultValue, theDefaultValLen);
	
	//
	// Return the default if it was provided. Only log an error if the default value was provided
	if (theDefaultValLen > 0)
	{
		QTSSModuleUtils::LogError( 	qtssWarningVerbosity,
									qtssServerPrefMissing,
									0,
									inAttributeName,
									inDefaultValue);
	}
	
	if (inDefaultValue != NULL)
	{
		//
		// Whether to return the default value or not from this function is dependent
		// solely on whether the caller passed in a non-NULL pointer or not.
		// This ensures that if the caller wants an empty-string returned as a default
		// value, it can do that.
		theString = NEW char[theDefaultValLen + 1];
		::strcpy(theString, inDefaultValue);
		return theString;
	}
	return NULL;
}

void	QTSSModuleUtils::GetIOAttribute(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType,
							void* ioDefaultResultBuffer, UInt32 inBufferLen)
{
	char *defaultBuffPtr = NEW char[inBufferLen];
	::memcpy(defaultBuffPtr,ioDefaultResultBuffer,inBufferLen);
	QTSSModuleUtils::GetAttribute(inObject, inAttributeName, inType, ioDefaultResultBuffer, defaultBuffPtr, inBufferLen);
	delete [] defaultBuffPtr;

}
							

QTSS_AttributeID QTSSModuleUtils::GetAttrID(QTSS_Object inObject, char* inAttributeName)
{
	//
	// Get the attribute ID of this attribute.
	QTSS_Object theAttrInfo = NULL;
	QTSS_Error theErr = QTSS_GetAttrInfoByName(inObject, inAttributeName, &theAttrInfo);
	if (theErr != QTSS_NoErr)
		return qtssIllegalAttrID;

	QTSS_AttributeID theID = qtssIllegalAttrID;	
	UInt32 theLen = sizeof(theID);
	theErr = QTSS_GetValue(theAttrInfo, qtssAttrID, 0, &theID, &theLen);
	Assert(theErr == QTSS_NoErr);

	return theID;
}

QTSS_AttributeID QTSSModuleUtils::CheckAttributeDataType(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType, void* inDefaultValue, UInt32 inBufferLen)
{
	//
	// Get the attribute type of this attribute.
	QTSS_Object theAttrInfo = NULL;
	QTSS_Error theErr = QTSS_GetAttrInfoByName(inObject, inAttributeName, &theAttrInfo);
	if (theErr != QTSS_NoErr)
		return qtssIllegalAttrID;

	QTSS_AttrDataType theAttributeType = qtssAttrDataTypeUnknown;
	UInt32 theLen = sizeof(theAttributeType);
	theErr = QTSS_GetValue(theAttrInfo, qtssAttrDataType, 0, &theAttributeType, &theLen);
	Assert(theErr == QTSS_NoErr);
	
	QTSS_AttributeID theID = qtssIllegalAttrID;	
	theLen = sizeof(theID);
	theErr = QTSS_GetValue(theAttrInfo, qtssAttrID, 0, &theID, &theLen);
	Assert(theErr == QTSS_NoErr);

	if (theAttributeType != inType)
	{
		char* theValueAsString = NULL;
		theErr = QTSS_ValueToString(inDefaultValue, inBufferLen, inType, &theValueAsString);
		Assert(theErr == QTSS_NoErr);
		OSCharArrayDeleter theValueStr(theValueAsString);
		QTSSModuleUtils::LogError( 	qtssWarningVerbosity,
									qtssServerPrefWrongType,
									0,
									inAttributeName,
									theValueStr.GetObject());
									
		theErr = QTSS_RemoveInstanceAttribute( inObject, theID );
		Assert(theErr == QTSS_NoErr);
		return  QTSSModuleUtils::CreateAttribute(inObject, inAttributeName, inType, inDefaultValue, inBufferLen);
	}
	return theID;
}

QTSS_AttributeID QTSSModuleUtils::CreateAttribute(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType, void* inDefaultValue, UInt32 inBufferLen)
{
	QTSS_Error theErr = QTSS_AddInstanceAttribute(inObject, inAttributeName, NULL, inType);
	Assert((theErr == QTSS_NoErr) || (theErr == QTSS_AttrNameExists));
	
	QTSS_AttributeID theID = QTSSModuleUtils::GetAttrID(inObject, inAttributeName);
	Assert(theID != qtssIllegalAttrID);
		
	//
	// Caller can pass in NULL for inDefaultValue, in which case we don't add the default
	if (inDefaultValue != NULL)
	{
		theErr = QTSS_SetValue(inObject, theID, 0, inDefaultValue, inBufferLen);
		Assert(theErr == QTSS_NoErr);
	}
	return theID;
}

QTSS_ActionFlags QTSSModuleUtils::GetRequestActions(QTSS_RTSPRequestObject theRTSPRequest)
{
	// Don't touch write requests
	QTSS_ActionFlags action = qtssActionFlagsNoFlags;
	UInt32 len = sizeof(QTSS_ActionFlags);
	QTSS_Error theErr = QTSS_GetValue(theRTSPRequest, qtssRTSPReqAction, 0, (void*)&action, &len);
	Assert(theErr == QTSS_NoErr);
	Assert(len == sizeof(QTSS_ActionFlags));
	return action;
}

char* QTSSModuleUtils::GetLocalPath_Copy(QTSS_RTSPRequestObject theRTSPRequest)
{	char*	pathBuffStr = NULL;
	QTSS_Error theErr = QTSS_GetValueAsString(theRTSPRequest, qtssRTSPReqLocalPath, 0, &pathBuffStr);
	Assert(theErr == QTSS_NoErr);
	return pathBuffStr;
}

char* QTSSModuleUtils::GetMoviesRootDir_Copy(QTSS_RTSPRequestObject theRTSPRequest)
{	char*	movieRootDirStr = NULL;
	QTSS_Error theErr = QTSS_GetValueAsString(theRTSPRequest,qtssRTSPReqRootDir, 0, &movieRootDirStr);
	Assert(theErr == QTSS_NoErr);
	return movieRootDirStr;
}

QTSS_UserProfileObject QTSSModuleUtils::GetUserProfileObject(QTSS_RTSPRequestObject theRTSPRequest)
{	QTSS_UserProfileObject theUserProfile = NULL;
	UInt32 len = sizeof(QTSS_UserProfileObject);
	QTSS_Error theErr = QTSS_GetValue(theRTSPRequest, qtssRTSPReqUserProfile, 0, (void*)&theUserProfile, &len);
	Assert(theErr == QTSS_NoErr);
	return theUserProfile;
}

char *QTSSModuleUtils::GetUserName_Copy(QTSS_UserProfileObject inUserProfile)
{
	char*	username = NULL;	
	(void) QTSS_GetValueAsString(inUserProfile, qtssUserName, 0, &username);
	return username;
}

char**  QTSSModuleUtils::GetGroupsArray_Copy(QTSS_UserProfileObject inUserProfile, UInt32 *outNumGroupsPtr)
{
	Assert(NULL != outNumGroupsPtr)

	char** outGroupCharPtrArray = NULL;
	*outNumGroupsPtr = 0;
	
	if (NULL == inUserProfile)
		return NULL;
	
	QTSS_Error theErr = QTSS_GetNumValues (inUserProfile,qtssUserGroups, outNumGroupsPtr);
	if (theErr != QTSS_NoErr || *outNumGroupsPtr == 0)
		return NULL;
		
	outGroupCharPtrArray = NEW char*[*outNumGroupsPtr]; // array of char *
	UInt32 len = 0;
	for (UInt32 index = 0; index < *outNumGroupsPtr; index++)
	{	outGroupCharPtrArray[index] = NULL;
		QTSS_GetValuePtr(inUserProfile, qtssUserGroups, index,(void **) &outGroupCharPtrArray[index], &len);
	}	

	return outGroupCharPtrArray;
}


SInt32 SDPContainer::AddHeaderLine (StrPtrLen *theLinePtr)
{   
    Assert(theLinePtr);
    UInt32 thisLine = fNumUsedLines;
    Assert(fNumUsedLines < fNumSDPLines);
    fSDPLineArray[thisLine].Set(theLinePtr->Ptr, theLinePtr->Len);
    fNumUsedLines++;
    if (fNumUsedLines == fNumSDPLines)
    {
        SDPLine   *tempSDPLineArray = NEW SDPLine[fNumSDPLines * 2];
        for (int i = 0; i < fNumSDPLines; i++)
        {
            tempSDPLineArray[i].Set(fSDPLineArray[i].Ptr,fSDPLineArray[i].Len);
            tempSDPLineArray[i].fHeaderType = fSDPLineArray[i].fHeaderType;
        }
        delete [] fSDPLineArray;
        fSDPLineArray = tempSDPLineArray;
        fNumSDPLines = (fNumUsedLines * 2);
    }
    
    if (theLinePtr->Ptr)
        fSDPLineArray[thisLine].fHeaderType = theLinePtr->Ptr[0];
        
    return thisLine;
}

SInt32 SDPContainer::FindHeaderLineType(char id, SInt32 start)
{   
    SInt32 theIndex = -1;
    
    if (start >= fNumUsedLines || start < 0)
        return -1;
        
    for (int i = start; i < fNumUsedLines; i++)
    {   if (fSDPLineArray[i].fHeaderType == id)
        {   theIndex = i;
            fCurrentLine = theIndex;
            break;
        }
    }
    
    return theIndex;
}

SDPLine* SDPContainer::GetNextLine()
{
    if (fCurrentLine < fNumUsedLines)
    {   fCurrentLine ++;
        return &fSDPLineArray[fCurrentLine];
    }
    
    return NULL;

}

SDPLine* SDPContainer::GetLine(SInt32 lineIndex)
{
    
    if (lineIndex > -1 && lineIndex < fNumUsedLines)
    {   return &fSDPLineArray[lineIndex];
    }

    return NULL;
}

void SDPContainer::SetLine(SInt32 index)
{
    if (index > -1 && index < fNumUsedLines)
    {   fCurrentLine = index;
    }
    else
        Assert(0);
        
}

void SDPContainer::Parse()
{
	char*	    validChars = "vosiuepcbtrzkam";
	char        nameValueSeparator = '=';
	
	Bool16      valid = true;

	StringParser	sdpParser(&fSDPBuffer);
	StrPtrLen		line;
	StrPtrLen 		fieldName;
	StrPtrLen		space;
	
	while ( sdpParser.GetDataRemaining() != 0 )
	{
		sdpParser.GetThruEOL(&line);  // Read each line  
        StringParser lineParser(&line);

        lineParser.ConsumeWhitespace();//skip over leading whitespace
        if (lineParser.GetDataRemaining() == 0) // must be an empty line
            continue;

        char firstChar = lineParser.PeekFast();
        if (firstChar == '\0')
            continue; //skip over blank lines
        
		lineParser.ConsumeUntil(&fieldName, nameValueSeparator);
		if ((fieldName.Len != 1) || (::strchr(validChars, fieldName.Ptr[0]) == NULL))
		{
			valid = false; // line doesn't begin with one of the valid characters followed by an "="
			break;
		}
		
		if (!lineParser.Expect(nameValueSeparator))
		{
			valid = false; // line doesn't have the "=" after the first char
			break;
		}
		
		lineParser.ConsumeUntil(&space, StringParser::sWhitespaceMask);
		
		if (space.Len != 0)
		{
			valid = false; // line has whitespace after the "=" 
			break;
		}
		AddHeaderLine(&line);
	}
	
	if (fNumUsedLines == 0) // didn't add any lines
	{   valid = false;
	}
	fValid = valid;
	
}

Bool16 SDPContainer::SetSDPBuffer(char *sdpBuffer) 
{ 
    
    fCurrentLine = 0;
    fNumUsedLines = 0;
    fValid = false;
    if (sdpBuffer != NULL)
    {   fSDPBuffer.Set(sdpBuffer); 
        Parse(); 
    }
    
    return IsSDPBufferValid();
}

Bool16 SDPContainer::SetSDPBuffer(StrPtrLen *sdpBufferPtr)
{ 
    fCurrentLine = 0;
    fNumUsedLines = 0;
    fValid = false;

    if (sdpBufferPtr != NULL)
    {   fSDPBuffer.Set(sdpBufferPtr->Ptr, sdpBufferPtr->Len); 
        Parse(); 
    }
    
    return IsSDPBufferValid();
}


void  SDPContainer::PrintLine(SInt32 lineIndex)
{
    StrPtrLen *printLinePtr = GetLine(lineIndex);
    if (printLinePtr)
    {   printLinePtr->PrintStr();
        printf("\n");
    }

}

void  SDPContainer::PrintAllLines()
{
    if (fNumUsedLines > 0)
    {   for (int i = 0; i < fNumUsedLines; i++)
            PrintLine(i);
    }
    else
        printf("SDPContainer::PrintAllLines no lines\n"); 
}


char SDPLineSorter::sSessionOrderedLines[] = "vosiuepcbtrzka"; // chars are order dependent: declared by rfc 2327
char SDPLineSorter::sessionSingleLines[]  = "vosiuepcbzk";    // return only 1 of each of these session field types
StrPtrLen  SDPLineSorter::sEOL("\r\n");

SDPLineSorter::SDPLineSorter(SDPContainer *rawSDPContainerPtr) : fSessionLineCount(0),fSDPSessionHeaders(NULL,0)
{

	Assert(rawSDPContainerPtr != NULL);
	if (NULL == rawSDPContainerPtr) 
		return;
		
	StrPtrLen theSDPData(rawSDPContainerPtr->fSDPBuffer.Ptr,rawSDPContainerPtr->fSDPBuffer.Len);
	StrPtrLen *theMediaStart = rawSDPContainerPtr->GetLine(rawSDPContainerPtr->FindHeaderLineType('m',0));
 	if (theMediaStart && theMediaStart->Ptr && theSDPData.Ptr)
	{
		UInt32  mediaLen = theSDPData.Len - (UInt32) (theMediaStart->Ptr - theSDPData.Ptr);
		char *mediaStartPtr= theMediaStart->Ptr;
		fMediaHeaders.Set(mediaStartPtr,mediaLen);
	}       


	fSessionLineCount = rawSDPContainerPtr->FindHeaderLineType('m',0);
	if (fSessionLineCount < 0) // didn't find it use the whole buffer
	{   fSessionLineCount = rawSDPContainerPtr->GetNumLines();
	}

	for (SInt16 sessionLineIndex = 0; sessionLineIndex < fSessionLineCount; sessionLineIndex++)
		fSessionSDPContainer.AddHeaderLine( (StrPtrLen *) rawSDPContainerPtr->GetLine(sessionLineIndex));

	//printff("\nSession raw Lines:\n"); fSessionSDPContainer.PrintAllLines();

	SInt16 numHeaderTypes = sizeof(SDPLineSorter::sSessionOrderedLines) -1;

	for (SInt16 fieldTypeIndex = 0; fieldTypeIndex < numHeaderTypes; fieldTypeIndex ++)
	{
		SInt32 lineIndex = fSessionSDPContainer.FindHeaderLineType(SDPLineSorter::sSessionOrderedLines[fieldTypeIndex], 0);
		StrPtrLen *theHeaderLinePtr = fSessionSDPContainer.GetLine(lineIndex);
		while (theHeaderLinePtr != NULL)
		{
			fSDPSessionHeaders.Put(*theHeaderLinePtr);
			fSDPSessionHeaders.Put(SDPLineSorter::sEOL);

			if (NULL != ::strchr(sessionSingleLines, theHeaderLinePtr->Ptr[0] ) ) // allow 1 of this type: use first found
				break; // move on to next line type

			lineIndex = fSessionSDPContainer.FindHeaderLineType(SDPLineSorter::sSessionOrderedLines[fieldTypeIndex], lineIndex + 1);
			theHeaderLinePtr = fSessionSDPContainer.GetLine(lineIndex);
		}
	}
	fSessionHeaders.Set(fSDPSessionHeaders.GetBufPtr(),fSDPSessionHeaders.GetBytesWritten());

}

char* SDPLineSorter::GetSortedSDPCopy()
{
	char* fullbuffCopy = NEW char[fSessionHeaders.Len + fMediaHeaders.Len + 2];
	SInt32 buffPos = 0;
	memcpy(&fullbuffCopy[buffPos], fSessionHeaders.Ptr,fSessionHeaders.Len);
	buffPos += fSessionHeaders.Len;
	memcpy(&fullbuffCopy[buffPos], fMediaHeaders.Ptr,fMediaHeaders.Len);
	buffPos += fMediaHeaders.Len;
	fullbuffCopy[buffPos] = 0;	
	
	return fullbuffCopy;
}


