#pragma once
#include "messagebus\mbtransport.h"
#include <memory>
#include <string>
#include <stdexcept>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#define MESSAGEOBJECT "MESSAGEOBJECT*"

struct MsgWrap{
	CMessage* msg;
};
template<typename ... Args>
std::string string_format(const char* format, Args ... args)
{
	const int size_s = std::snprintf(nullptr, 0, format, args ...) + 1; // Extra space for '\0'
	if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
	auto size = (size_t)(size_s);
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format, args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}
void throw_L_error(lua_State* L, const char *str);
int wrap_cmsg(lua_State* L, CMessage* msg);
CMessage* cmessage_arg(lua_State* L,LPCSTR module , int idx = 1);
void cmesage_Destroy(lua_State* L);
void cmesage_gc(lua_State* L);
LUA_API const char* lua_pushstringW(lua_State* L, const WCHAR* s);
struct MB2W
{
	MB2W(const char* src, int cp = CP_ACP)
		: buffer(0)
	{
		int len = ::MultiByteToWideChar(cp, 0, src, -1, 0, 0);
		if (len)
		{
			buffer = new wchar_t[len];
			len = ::MultiByteToWideChar(cp, 0, src, -1, buffer, len);
		}
	}
	~MB2W()
	{
		delete[] buffer;
	}
	const wchar_t* c_str() const
	{
		return buffer;
	}
private:
	wchar_t* buffer;
};

struct W2MB
{
	W2MB(const wchar_t* src, int cp = CP_ACP)
		: buffer(0)
	{
		int len = ::WideCharToMultiByte(cp, 0, src, -1, 0, 0, 0, 0);
		if (len)
		{
			buffer = new char[len];
			len = ::WideCharToMultiByte(cp, 0, src, -1, buffer, len, 0, 0);
		}
	}
	~W2MB()
	{
		delete[] buffer;
	}
	const char* c_str() const
	{
		return buffer;
	}
private:
	char* buffer;
};
