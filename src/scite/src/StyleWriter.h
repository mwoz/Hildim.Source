// SciTE - Scintilla based Text Editor
/** @file StyleWriter.h
 ** Simple buffered interface to the text and styles of a document held by Scintilla.
 **/
// Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef STYLEWRITER_H
#define STYLEWRITER_H

// Read only access to a document, its styles and other data
class TextReader {
	// Private so TextReader objects can not be copied
	TextReader(const TextReader &source);
	TextReader &operator=(const TextReader &);
protected:
	enum {extremePosition=0x7FFFFFFF};
	/** @a bufferSize is a trade off between time taken to copy the characters
	 * and retrieval overhead.
	 * @a slopSize positions the buffer before the desired position
	 * in case there is some backtracking. */
	enum {bufferSize=4000, slopSize=bufferSize/8};
	char buf[bufferSize+1];
	Sci_Position startPos;
	Sci_Position endPos;
	int codePage;

	GUI::ScintillaWindow &sw;
	Sci_Position lenDoc;

	bool InternalIsLeadByte(char ch) const;
	void Fill(Sci_Position position);
public:
	TextReader(GUI::ScintillaWindow &sw_) :
		startPos(extremePosition),
		endPos(0),
		codePage(0),
		sw(sw_),
		lenDoc(-1) {
	}
	char operator[](Sci_Position position) {
		if (position < startPos || position >= endPos) {
			Fill(position);
		}
		return buf[position - startPos];
	}
	/** Safe version of operator[], returning a defined value for invalid position. */
	char SafeGetCharAt(Sci_Position position, char chDefault=' ') {
		if (position < startPos || position >= endPos) {
			Fill(position);
			if (position < startPos || position >= endPos) {
				// Position is outside range of document
				return chDefault;
			}
		}
		return buf[position - startPos];
	}
	bool IsLeadByte(char ch) const {
		return codePage && InternalIsLeadByte(ch);
	}
	void SetCodePage(int codePage_) {
		codePage = codePage_;
	}
	bool Match(int pos, const char *s);
	char StyleAt(Sci_Position position);
	Sci_Position GetLine(Sci_Position position);
	Sci_Position LineStart(Sci_Position line);
	int LevelAt(Sci_Position line);
	Sci_Position Length();
	int GetLineState(Sci_Position line);
};

// Adds methods needed to write styles and folding
class StyleWriter : public TextReader {
	// Private so StyleWriter objects can not be copied
	StyleWriter(const StyleWriter &source);
	StyleWriter &operator=(const StyleWriter &);
protected:
	char styleBuf[bufferSize];
	int validLen;
	Sci_Position startSeg;
public:
	StyleWriter(GUI::ScintillaWindow &sw_) :
		TextReader(sw_),
		validLen(0),
		startSeg(0) {
	}
	void Flush();
	int SetLineState(Sci_Position line, int state);

	void StartAt(Sci_Position start, char chMask=31);
	Sci_Position GetStartSegment() { return startSeg; }
	void StartSegment(Sci_Position pos);
	void ColourTo(Sci_Position pos, int chAttr);
	void SetLevel(Sci_Position line, int level);
};

#endif
