// Utf8_16.h
// Copyright (C) 2002 Scott Kirkwood
//
// Permission to use, copy, modify, distribute and sell this code
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all copies or
// any derived copies.  Scott Kirkwood makes no representations
// about the suitability of this software for any purpose.
// It is provided "as is" without express or implied warranty.
//
// Notes: Used the UTF information I found at:
//   http://www.cl.cam.ac.uk/~mgk25/unicode.html
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <assert.h>

#ifdef _MSC_VER
#pragma warning(disable: 4514) // nreferenced inline function has been removed
#endif

enum u78 { utf8NoBOM = 0, ascii7bits = 1, ascii8bits = 2 };

class Utf8_16 {
public:
	typedef unsigned short utf16; // 16 bits
	typedef unsigned char utf8; // 8 bits
	typedef unsigned char ubyte;
	enum encodingType {
	    eUnknown,
	    eUtf16BigEndian,
	    eUtf16LittleEndian,  // Default on Windows
	    eUtf8,
	    eLast
	};
	static const utf8 k_Boms[eLast][3];
};

// Reads UTF-16 and outputs UTF-8
class Utf16_Iter : public Utf8_16 {
public:
	Utf16_Iter();
	void reset();
	void set(const ubyte* pBuf, size_t nLen, encodingType eEncoding);
	utf8 get() const {
		return m_nCur;
	}
	void operator++();
	operator bool() { return m_pRead <= m_pEnd; }

protected:
	enum eState {
	    eStart,
	    eSecondOf4Bytes,
	    ePenultimate,
	    eFinal
	};
protected:
	encodingType m_eEncoding;
	eState m_eState;
	utf8 m_nCur;
	int m_nCur16;
	const ubyte* m_pBuf;
	const ubyte* m_pRead;
	const ubyte* m_pEnd;
};

// Reads UTF-8 and outputs UTF-16
class Utf8_Iter : public Utf8_16 {
public:
	Utf8_Iter();
	void reset();
	void set(const ubyte* pBuf, size_t nLen, encodingType eEncoding);
	int get() const {
#ifdef _DEBUG
		assert(m_eState == eStart);
#endif
		return m_nCur;
	}
	bool canGet() const { return m_eState == eStart; }
	void operator++();
	operator bool() { return m_pRead <= m_pEnd; }

protected:
	void toStart(); // Put to start state
	enum eState {
	    eStart,
	    eSecondOf4Bytes,
	    ePenultimate,
	    eFinal
	};
protected:
	encodingType m_eEncoding;
	eState m_eState;
	int m_nCur;
	const ubyte* m_pBuf;
	const ubyte* m_pRead;
	const ubyte* m_pEnd;
};

// Reads UTF16 and outputs UTF8
class Utf8_16_Read : public Utf8_16 {
public:
	Utf8_16_Read();
	~Utf8_16_Read();
	Utf8_16_Read(bool AutoCheckUtf8); //!-add-[utf8.auto.check]

	size_t convert(char* buf, size_t len, bool reset = false);
	char* getNewBuf() { return reinterpret_cast<char*>(m_pNewBuf); }

	encodingType getEncoding() const { return utf8noBoom ? eLast : m_eEncoding; }
	int _encoding = 0;
protected:
	int determineEncoding();
	u78 utf8_7bits_8bits();
	bool utf8noBoom = false;
private:
	encodingType m_eEncoding;
	ubyte* m_pBuf;
	ubyte* m_pNewBuf;
	size_t m_nBufSize;
	bool m_bFirstRead;
	size_t m_nLen;
	Utf16_Iter m_Iter16;
	int m_nAutoCheckUtf8;//!-add-[utf8.auto.check]
};

// Read in a UTF-8 buffer and write out to UTF-16 or UTF-8
class Utf8_16_Write : public Utf8_16 {
public:
	Utf8_16_Write();
	~Utf8_16_Write();

	void setEncoding(encodingType eType);

	void setfile(FILE *pFile);
	size_t fwrite(const void* p, size_t _size);
	void fclose();
	int _encoding = 0;
protected:
	encodingType m_eEncoding;
	FILE* m_pFile;
	utf16* m_pBuf;
	size_t m_nBufSize;
	bool m_bFirstWrite;
};

