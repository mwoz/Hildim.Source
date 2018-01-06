// This file is part of Notepad++ project
// Copyright (C)2003 Don HO <don.h@free.fr>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Note that the GPL places important restrictions on "derived works", yet
// it does not provide a detailed definition of that term.  To avoid
// misunderstandings, we consider an application to constitute a
// "derivative work" for the purpose of this license if it does any of the
// following:
// 1. Integrates source code from Notepad++.
// 2. Integrates/includes/aggregates Notepad++ into a proprietary executable
//    installer, such as those produced by InstallShield.
// 3. Links to a library or executes a program that does any of the above.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include <algorithm>
#include <stdexcept>
#include <shlwapi.h>
#include <shlobj.h>
#include <uxtheme.h>

#include "Encoding.h"

WcharMbcsConvertor* WcharMbcsConvertor::_pSelf = new WcharMbcsConvertor;

inline static bool isContinuation(UCHAR c) { return (c & 0xC0) == 0x80; }
inline static bool isSingleByte(UCHAR c) { return c < 0x80; }
inline static bool isFirstOfMultibyte(UCHAR c) { return c >= 0xC2 && c < 0xF5; } // 0xF5 to 0xFD are defined by UTF-8, but are not currently valid Unicode
																				 // number of continuation bytes for a given valid first character (0 for single byte characters)
inline static int  continuationBytes(UCHAR c) {
	static const char _len[] = { 1,1,2,3 };
	return (c < 0xC0) ? 0 : _len[(c & 0x30) >> 4];
}

inline static bool isValid(const char* buf, int buflen) {
	if (isSingleByte(buf[0])) return true; // single byte is valid
	if (!isFirstOfMultibyte(buf[0])) return false; // not single byte, nor valid multi-byte first byte
	int charContinuationBytes = continuationBytes(buf[0]);
	if (buflen < charContinuationBytes + 1) return false; // character does not fit in buffer
	for (int i = charContinuationBytes; i>0; --i)
		if (!isContinuation(*(++buf))) return false; // not enough continuation bytes
	return true;  // the character is valid (if there are too many continuation bytes, it is the next character that will be invalid)
}

inline static int characterStart(const char* buf, int startingIndex) {
	int charContinuationBytes = 0;
	while (charContinuationBytes < startingIndex	// rewind past start of buffer?
		&& charContinuationBytes < 5	// UTF-8 support up to 5 continuation bytes (but valid sequences currently do not have more than 3)
		&& isContinuation(buf[startingIndex - charContinuationBytes])
		)
		++charContinuationBytes;
	return startingIndex - charContinuationBytes;
}

wchar_t * WcharMbcsConvertor::char2wchar(const char * mbcs2Convert, UINT codepage, int lenMbcs, int *pLenWc, int *pBytesNotProcessed)
{
	// Do not process NULL pointer
	if (!mbcs2Convert)
		return nullptr;

	// Do not process empty strings
	if (lenMbcs == 0 || lenMbcs == -1 && mbcs2Convert[0] == 0)
	{
		_wideCharStr.empty();
		return _wideCharStr;
	}

	int bytesNotProcessed = 0;
	int lenWc = 0;

	// If length not specified, simply convert without checking
	if (lenMbcs == -1)
	{
		lenWc = MultiByteToWideChar(codepage, 0, mbcs2Convert, lenMbcs, NULL, 0);
	}
	// Otherwise, test if we are cutting a multi-byte character at end of buffer
	else if (lenMbcs != -1 && codepage == CP_UTF8) // For UTF-8, we know how to test it
	{
		int indexOfLastChar = characterStart(mbcs2Convert, lenMbcs-1); // get index of last character
		if (indexOfLastChar != 0 && !isValid(mbcs2Convert+indexOfLastChar, lenMbcs-indexOfLastChar)) // if it is not valid we do not process it right now (unless its the only character in string, to ensure that we always progress, e.g. that bytesNotProcessed < lenMbcs)
		{
			bytesNotProcessed = lenMbcs-indexOfLastChar;
		}
		lenWc = MultiByteToWideChar(codepage, 0, mbcs2Convert, lenMbcs-bytesNotProcessed, NULL, 0);
	}
	else // For other encodings, ask system if there are any invalid characters; note that it will not correctly know if last character is cut when there are invalid characters inside the text
	{
		lenWc = MultiByteToWideChar(codepage, (lenMbcs == -1) ? 0 : MB_ERR_INVALID_CHARS, mbcs2Convert, lenMbcs, NULL, 0);
		if (lenWc == 0 && GetLastError() == ERROR_NO_UNICODE_TRANSLATION)
		{
			// Test without last byte
			if (lenMbcs > 1) lenWc = MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS, mbcs2Convert, lenMbcs-1, NULL, 0);
			if (lenWc == 0) // don't have to check that the error is still ERROR_NO_UNICODE_TRANSLATION, since only the length parameter changed
			{
				// TODO: should warn user about incorrect loading due to invalid characters
				// We still load the file, but the system will either strip or replace invalid characters (including the last character, if cut in half)
				lenWc = MultiByteToWideChar(codepage, 0, mbcs2Convert, lenMbcs, NULL, 0);
			}
			else
			{
				// We found a valid text by removing one byte.
				bytesNotProcessed = 1;
			}
		}
	}

	if (lenWc > 0)
	{
		_wideCharStr.sizeTo(lenWc);
		MultiByteToWideChar(codepage, 0, mbcs2Convert, lenMbcs-bytesNotProcessed, _wideCharStr, lenWc);
	}
	else
		_wideCharStr.empty();

	if (pLenWc)
		*pLenWc = lenWc;
	if (pBytesNotProcessed)
		*pBytesNotProcessed = bytesNotProcessed;

	return _wideCharStr;
}


// "mstart" and "mend" are pointers to indexes in mbcs2Convert,
// which are converted to the corresponding indexes in the returned wchar_t string.
wchar_t * WcharMbcsConvertor::char2wchar(const char * mbcs2Convert, UINT codepage, int *mstart, int *mend)
{
	// Do not process NULL pointer
	if (!mbcs2Convert) return NULL;

	int len = MultiByteToWideChar(codepage, 0, mbcs2Convert, -1, NULL, 0);
	if (len > 0)
	{
		_wideCharStr.sizeTo(len);
		len = MultiByteToWideChar(codepage, 0, mbcs2Convert, -1, _wideCharStr, len);

		if ((size_t)*mstart < strlen(mbcs2Convert) && (size_t)*mend <= strlen(mbcs2Convert))
		{
			*mstart = MultiByteToWideChar(codepage, 0, mbcs2Convert, *mstart, _wideCharStr, 0);
			*mend   = MultiByteToWideChar(codepage, 0, mbcs2Convert, *mend, _wideCharStr, 0);
			if (*mstart >= len || *mend >= len)
			{
				*mstart = 0;
				*mend = 0;
			}
		}
	}
	else
	{
		_wideCharStr.empty();
		*mstart = 0;
		*mend = 0;
	}
	return _wideCharStr;
}


char* WcharMbcsConvertor::wchar2char(const wchar_t * wcharStr2Convert, UINT codepage, int lenWc, int *pLenMbcs)
{
	if (nullptr == wcharStr2Convert)
		return nullptr;

	int lenMbcs = WideCharToMultiByte(codepage, 0, wcharStr2Convert, lenWc, NULL, 0, NULL, NULL);
	if (lenMbcs > 0)
	{
		_multiByteStr.sizeTo(lenMbcs);
		WideCharToMultiByte(codepage, 0, wcharStr2Convert, lenWc, _multiByteStr, lenMbcs, NULL, NULL);
	}
	else
		_multiByteStr.empty();

	if (pLenMbcs)
		*pLenMbcs = lenMbcs;
	return _multiByteStr;
}


char * WcharMbcsConvertor::wchar2char(const wchar_t * wcharStr2Convert, UINT codepage, long *mstart, long *mend)
{
	if (nullptr == wcharStr2Convert)
		return nullptr;

	int len = WideCharToMultiByte(codepage, 0, wcharStr2Convert, -1, NULL, 0, NULL, NULL);
	if (len > 0)
	{
		_multiByteStr.sizeTo(len);
		len = WideCharToMultiByte(codepage, 0, wcharStr2Convert, -1, _multiByteStr, len, NULL, NULL); // not needed?

        if (*mstart < lstrlenW(wcharStr2Convert) && *mend < lstrlenW(wcharStr2Convert))
        {
			*mstart = WideCharToMultiByte(codepage, 0, wcharStr2Convert, *mstart, NULL, 0, NULL, NULL);
			*mend = WideCharToMultiByte(codepage, 0, wcharStr2Convert, *mend, NULL, 0, NULL, NULL);
			if (*mstart >= len || *mend >= len)
			{
				*mstart = 0;
				*mend = 0;
			}
		}
	}
	else
		_multiByteStr.empty();

	return _multiByteStr;
}

