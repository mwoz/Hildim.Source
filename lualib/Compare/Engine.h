/*
 * This file is part of ComparePlus plugin for Notepad++
 * Copyright (C)2011 Jean-Sebastien Leroy (jean.sebastien.leroy@gmail.com)
 * Copyright (C)2017-2022 Pavel Nedev (pg.nedev@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <windows.h>
#include <cstdint>
#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <regex>
#include "Compare.h"
#include "NppHelpers.h"
#include "ScintillaCall.h"

#define MAIN_VIEW 0
#define SUB_VIEW 1

enum class CompareResult
{
	COMPARE_ERROR,
	COMPARE_CANCELLED,
	COMPARE_MATCH,
	COMPARE_MISMATCH
};


struct section_t
{
	section_t() : off(0), len(0), src(0) {}
	section_t(intptr_t o, intptr_t l, intptr_t s) : off(o), len(l), src(s) {}

	intptr_t off;
	intptr_t len;
	intptr_t src;
};

enum FoldType_t
{
	NOT_SET = 0,
	NO_FOLD,
	FOLD_MATCHES,
	FOLD_OUTSIDE_SELECTIONS
};

struct CompareOptions
{
	CompareOptions()
	{
		selections[0] = std::make_pair(-1, -1);
		selections[1] = std::make_pair(-1, -1);
	}

	inline void setIgnoreRegex(const std::string& regexStr, UINT codepage)
	{
		if (!regexStr.empty())
		{
			std::vector<char> line(regexStr.begin(), regexStr.end());
			line.push_back('\0');

			const int len = static_cast<int>(line.size());

			const int wLen = ::MultiByteToWideChar(codepage, 0, line.data(), len, NULL, 0);

			std::vector<wchar_t> wLine(wLen);

			::MultiByteToWideChar(codepage, 0, line.data(), len, wLine.data(), wLen);



			std::wstring wr(wLine.begin(), wLine.end());

			ignoreRegex = std::make_unique<std::wregex>(wr, std::regex::ECMAScript | std::regex::optimize);
		
		}
		else
		{
			ignoreRegex = nullptr;
		}
	}

	inline void clearIgnoreRegex()
	{
		ignoreRegex = nullptr;
	}

	int		newFileViewId;

	bool	findUniqueMode;

	bool	alignAllMatches;
	bool	neverMarkIgnored;
	bool	detectMoves;
	bool	detectCharDiffs;
	bool	ignoreEmptyLines;
	bool	ignoreFoldedLines;
	bool	ignoreChangedSpaces;
	bool    ignoreIndent;
	bool	ignoreAllSpaces;
	bool	ignoreCase;
	FoldType_t     foldType = NOT_SET;

	bool	recompareOnChange;

	std::unique_ptr<std::wregex>	ignoreRegex;
	bool						    invertRegex;

	int		changedThresholdPercent;

	bool	selectionCompare;

	std::pair<intptr_t, intptr_t>	selections[2];
};


struct AlignmentViewData
{
	intptr_t	line {0};
	int			diffMask {0};
};


struct AlignmentPair
{
	AlignmentViewData main;
	AlignmentViewData sub;
};


using AlignmentInfo_t = std::vector<AlignmentPair>;

struct CompareSummary
{
	inline void clear()
	{
		diffLines	= 0;
		added		= 0;
		removed		= 0;
		moved		= 0;
		changed		= 0;
		match		= 0;

		alignmentInfo.clear();
	}

	intptr_t	diffLines;
	intptr_t	added;
	intptr_t	removed;
	intptr_t	moved;
	intptr_t	changed;
	intptr_t	match;

	AlignmentInfo_t	alignmentInfo;
};




struct blankLineList
{
	int line;
	int length;
	struct blankLineList* next;
};

struct moveSrc
{
	intptr_t off;
	intptr_t sub_off;
	intptr_t len;
	intptr_t src;
	intptr_t block_len;
};
class MovedMapElement_t :public std::vector<moveSrc>
{
private:
	iterator cursor;
	//std::string sOut;
public:
	void close_and_sort() {
		moveSrc m;
		m.off = INTPTR_MAX;
		m.sub_off = 0;
		m.len = 0;
		m.src = 0;
		m.block_len = 0;
		push_back(m);
		std::sort(begin(), end(),
			[](moveSrc& a, moveSrc& b) {return (a.off == b.off ? a.sub_off < b.sub_off : a.off < b.off); });
		cursor = begin();
	}
	const char* text4annotation(intptr_t start, intptr_t annotLen) {
		static std::string sOut;
		sOut.clear();
		while (cursor != end() && start > cursor._Ptr->off)
			cursor++;
		moveSrc* src = nullptr;
		if (cursor._Ptr->off == start) {
			src = cursor._Ptr;
		}
		for (intptr_t i = 0; i < annotLen; i++) {
			if(i)
				sOut += '\n';
			intptr_t dLen;
			if (src && i >= (dLen = src->sub_off - (src->block_len - annotLen))) {
				sOut += " >>> ";
				sOut += std::to_string(i + - src->sub_off + src->src + 1);
				if (i == dLen + src->len - 1) {
					cursor++;
					src = (cursor._Ptr->off == start ? cursor._Ptr : nullptr);
				}
			}	
		}
		return sOut.c_str();
		//return "h";
	}
};


struct movedMap
{

	MovedMapElement_t main;
	MovedMapElement_t sub;
};

CompareResult compareViews(const CompareOptions& options, CompareSummary& summary, movedMap& moves);
