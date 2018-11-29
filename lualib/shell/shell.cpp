//build@ gcc -shared -o shell.dll -I shell.cpp scite.la -lstdc++

#include <windows.h>
#include <shlwapi.h>
#include <time.h>

#include <string>
#include <iostream>
#include <filesystem>

extern "C" {
	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"
}

#define SHELLPROCOBJECT "SHELLPROCOBJECT*"
//
//#ifdef DEBUG
//BOOL APIENTRY DllMain( HANDLE hModule, 
//	DWORD  ul_reason_for_call, 
//	LPVOID lpReserved
//	)
//{
//	return TRUE;
//}
//#endif


#pragma warning(push)
#pragma warning(disable: 4710)

template < class T, int defSize >
class CMemBuffer
{
public:
	CMemBuffer()
	 : m_iSize( defSize )
	 , m_pData( NULL )
	{
		SetLength( defSize );
	}

	~CMemBuffer()
	{
		SetLength( 0 );
	}

	BOOL IsBufferEmpty()
	{
		return m_pData == NULL;
	}

	T* GetBuffer()
	{
		return m_pData;
	}

	T& operator [] ( int nItem )
	{
		return m_pData[ nItem ];
	}

	int GetBufferLength()
	{
		return m_iSize;
	}

	// установить длинну буфера точно
	// 0 - очищает буфер
	BOOL SetLength( int lenNew )
	{
		if ( lenNew > 0 )
		{
			T* sNew = (T*)malloc( lenNew * sizeof(T) );
//			T* sNew = (T*)::VirtualAlloc( NULL, lenNew * sizeof(T), MEM_COMMIT, PAGE_READWRITE );
			if ( sNew != NULL )
			{
				if ( !IsBufferEmpty() )
				{
					memcpy( sNew,
							m_pData,
							lenNew > m_iSize ? m_iSize * sizeof(T) : lenNew * sizeof(T) );
//					::VirtualFree( m_pData, 0, MEM_RELEASE );
					free( m_pData );
					m_pData = NULL;
				}
				m_pData = sNew;
				m_iSize = lenNew;
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			if ( !IsBufferEmpty() )
			{
//				::VirtualFree( m_pData, 0, MEM_RELEASE );
				free( m_pData );
				m_pData = NULL;
			}
			m_iSize = 0;
		}
		return TRUE;
	}

private:
	T* m_pData;
	int m_iSize;
};

class CSimpleString
{
public:
	CSimpleString()
	 : m_iLen( 0 )
	{
	}

	const char* GetString()
	{
		return ( m_iLen == 0 || m_sData.IsBufferEmpty() ) ? "" : m_sData.GetBuffer();
	}

	char& operator [] ( int nItem )
	{
		return m_sData[ nItem ];
	}

	int GetLenght()
	{
		return m_iLen;
	}

	void Empty()
	{
		m_sData.SetLength( 0 );
		m_iLen = 0;
	}

	void Append( const char *str, int len = -1 )
	{
		if ( str != NULL )
		{
			if ( len == -1 ) len = lstrlenA( str );
			int newLength = m_iLen + len;
			if ( m_sData.SetLength( newLength + 1 ) )
			{
				m_sData[ m_iLen ] = '\0';
				lstrcpynA( &m_sData[ m_iLen ], str, len + 1 );
				m_iLen = newLength;
			}
		}
	}

private:
	CMemBuffer< char, 128 > m_sData;
	int m_iLen;
};

class CPath
{
public:
	CPath( const char* lpszFileName )
	{
		if ( lpszFileName != NULL )
		{
			// сохраняем оригинал
			m_sPathOriginal.Append( lpszFileName );

			if ( ::PathIsURLA( lpszFileName ) == TRUE )
			{
				m_sPath.Append( lpszFileName );
			}
			else // делаем преобразования
			{
				// 1. Раскрываем переменные окружения
				CMemBuffer< char, 1024 > sExpanded;
				::ExpandEnvironmentStringsA( lpszFileName, sExpanded.GetBuffer(), 1024 );
				// 2. Убираем в пути .. и . (приводим к каноническому виду)
				CMemBuffer< char, 1024 > sCanonical;
				::PathCanonicalizeA( sCanonical.GetBuffer(), sExpanded.GetBuffer() );
				// 3. Убираем лишние пробелы
				::PathRemoveBlanksA( sCanonical.GetBuffer() );
				// 4. Проверяем существует ли преобразованный путь
				if ( ::PathFileExistsA( sCanonical.GetBuffer() ) == TRUE )
				{
					::PathMakePrettyA( sCanonical.GetBuffer() );
					::PathRemoveBackslashA( sCanonical.GetBuffer() );
					m_sPath.Append( sCanonical.GetBuffer() );
					if ( ::PathIsDirectoryA( sCanonical.GetBuffer() ) == FALSE )
					{
						m_sFileName.Append( ::PathFindFileNameA( sCanonical.GetBuffer() ) );
						::PathRemoveFileSpecA( sCanonical.GetBuffer() );
					}
					m_sPathDir.Append( sCanonical.GetBuffer() );
				}
				else
				{
					// 5. Отделяем оргументы
					char* pArg = ::PathGetArgsA( sCanonical.GetBuffer() );
					m_sFileParams.Append( pArg );
					::PathRemoveArgsA( sCanonical.GetBuffer() );
					// 6. Делаем путь по красивше
					::PathUnquoteSpacesA( sCanonical.GetBuffer() );
					::PathRemoveBackslashA( sCanonical.GetBuffer() );
					::PathMakePrettyA( sCanonical.GetBuffer() );
					// 7. Проверяем преобразованный путь это дирректория
					if ( ::PathIsDirectoryA( sCanonical.GetBuffer() ) != FALSE )
					{
						m_sPath.Append( sCanonical.GetBuffer() );
						m_sPathDir.Append( sCanonical.GetBuffer() );
					}
					else
					{
						// 8. Добавляем расширение к файлу .exe, если нету
						::PathAddExtensionA( sCanonical.GetBuffer(), NULL );
						// 9. Проверяем есть ли такой файл
						if ( ::PathFileExistsA( sCanonical.GetBuffer() ) == TRUE )
						{
							m_sPath.Append( sCanonical.GetBuffer() );
							m_sFileName.Append( ::PathFindFileNameA( sCanonical.GetBuffer() ) );
							::PathRemoveFileSpecA( sCanonical.GetBuffer() );
							m_sPathDir.Append( sCanonical.GetBuffer() );
						}
						else
						{
							// 10. Производим поиск
							::PathFindOnPathA( sCanonical.GetBuffer(), NULL );
							::PathMakePrettyA( sCanonical.GetBuffer() );
							m_sPath.Append( sCanonical.GetBuffer() );
							if ( ::PathFileExistsA( sCanonical.GetBuffer() ) == TRUE )
							{
								m_sFileName.Append( ::PathFindFileNameA( sCanonical.GetBuffer() ) );
								::PathRemoveFileSpecA( sCanonical.GetBuffer() );
								m_sPathDir.Append( sCanonical.GetBuffer() );
							}
						}
					}
				}
			}
		}
	}

	const char* GetPath()
	{
		return m_sPath.GetLenght() > 0 ? m_sPath.GetString() : NULL;
	}

	const char* GetDirectory()
	{
		return m_sPathDir.GetLenght() > 0 ? m_sPathDir.GetString() : NULL;
	}

	const char* GetFileParams()
	{
		return m_sFileParams.GetLenght() > 0 ? m_sFileParams.GetString() : NULL;
	}

private:
	CSimpleString m_sPathOriginal;
	CSimpleString m_sPath;
	CSimpleString m_sPathDir;
	CSimpleString m_sFileName;
	CSimpleString m_sFileParams;

public:
	static DWORD GetFileAttributes( const char* lpszFileName )
	{
		WIN32_FILE_ATTRIBUTE_DATA fad;
		if ( ::GetFileAttributesExA( lpszFileName, GetFileExInfoStandard, &fad ) == FALSE )
		{
			return ((DWORD)-1); //INVALID_FILE_ATTRIBUTES;
		}
		return fad.dwFileAttributes;
	}
	static BOOL SetFileAttributes( const char* lpszFileName, DWORD dwFileAttributes )
	{
		return ::SetFileAttributesA( lpszFileName, dwFileAttributes );
	}
	static BOOL IsDirectory( const char* lpszFileName )
	{
		return ::PathIsDirectoryA( lpszFileName ) != FALSE;
	}
	static BOOL IsFileExists( const char* lpszFileName )
	{
		return IsPathExist( lpszFileName ) == TRUE &&
			   IsDirectory( lpszFileName ) == FALSE;
	}
	static BOOL IsPathExist( const char* lpszFileName )
	{
		return ::PathFileExistsA( lpszFileName ) != FALSE;
	}
};
char currentDir[MAX_PATH + 1];
// получить последнее сообщение об ошибке
// для возвращаемой строки нужно вызвать LocalFree
static char* GetLastErrorString( DWORD* lastErrorCode, int* iLenMsg )
{
	char* lpMsgBuf;
	*lastErrorCode = ::GetLastError();
	::FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					  NULL,
					  *lastErrorCode,
					  MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
					  (LPSTR)&lpMsgBuf,
					  0,
					  NULL );

	*iLenMsg = lstrlenA( lpMsgBuf );

	// trim right
	while ( *iLenMsg > 0 )
	{
		(*iLenMsg)--;
		if ( lpMsgBuf[ *iLenMsg ] == '\n' ||
			 lpMsgBuf[ *iLenMsg ] == '\r' ||
			 lpMsgBuf[ *iLenMsg ] == '.' ||
			 lpMsgBuf[ *iLenMsg ] == ' ' )
		{
			lpMsgBuf[ *iLenMsg ] = 0;
		}
		else
		{
			break;
		}
	}
	(*iLenMsg)++;
	return lpMsgBuf;
}

static void lua_pushlasterr( lua_State* L, const char* lpszFunction )
{
	DWORD dw;
	int iLenMsg;
	char* lpMsgBuf = GetLastErrorString( &dw, &iLenMsg );

	if ( lpszFunction == NULL )
	{
		lua_pushstring( L, lpMsgBuf );
	}
	else
	{
		UINT uBytes = ( iLenMsg + lstrlenA( lpszFunction ) + 40 ) * sizeof(char);
		char* lpDisplayBuf = (char*)::LocalAlloc( LMEM_ZEROINIT, uBytes );
		sprintf( lpDisplayBuf, "%s failed with error %d: %s", lpszFunction, dw, lpMsgBuf );
		lua_pushstring( L, lpDisplayBuf );
		::LocalFree( lpDisplayBuf );
	}
	::LocalFree( lpMsgBuf );
}

static int getfileattr(lua_State *L) {
	std::filesystem::path f;
	try {
		f = std::filesystem::u8path(luaL_checkstring(L, 1));
	} catch (...) {
		return 0;
	}
	lua_pushinteger(L, ::GetFileAttributesW(f.c_str()));
	return 1;
}
static int getfiletime(lua_State *L) {
	std::filesystem::path f;
	try {
		f = std::filesystem::u8path(luaL_checkstring(L, 1));
	} catch (...) {
		return 0;
	}
	HANDLE hf = ::CreateFileW(f.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		FILETIME ft;
		::GetFileTime(hf, NULL, NULL, &ft);
		::CloseHandle(hf);
		SYSTEMTIME st;
		::FileTimeToSystemTime(&ft, &st);
		lua_createtable(L, 1, 0);
		lua_pushinteger(L, st.wYear);
		lua_setfield(L, -2, "Year");
		lua_pushinteger(L, st.wMonth);
		lua_setfield(L, -2, "Month");
		lua_pushinteger(L, st.wDay);
		lua_setfield(L, -2, "Day");
		lua_pushinteger(L, st.wHour);
		lua_setfield(L, -2, "Hour");
		lua_pushinteger(L, st.wMinute);
		lua_setfield(L, -2, "Minute");
		return 1;
	}
	return 0;
}
static int delete_file(lua_State *L)
{
	std::filesystem::path f;
	try {
		f = std::filesystem::u8path(luaL_checkstring(L, 1));
	} catch (...) {
		return 0;
	}

	SHFILEOPSTRUCTW ls;
	WCHAR buff[MAX_PATH + 1];
	::ZeroMemory(&ls, sizeof(SHFILEOPSTRUCTW));
	::ZeroMemory(buff, MAX_PATH+1);
	wcscpy(buff, f.c_str());
	ls.pFrom = buff;
	ls.wFunc = FO_DELETE;
	ls.fFlags = FOF_SILENT |  FOF_ALLOWUNDO | FOF_NOCONFIRMATION;  // FOF_NOERRORUI |

	lua_pushnumber(L, SHFileOperationW(&ls));
	return 1;
}
typedef struct tagENUMINFO
{
	// In Parameters
	DWORD PId;

	// Out Parameters
	HWND  hWnd;
	HWND  hEmptyWnd;
	HWND  hInvisibleWnd;
	HWND  hEmptyInvisibleWnd;
} ENUMINFO, *PENUMINFO;

typedef struct tagSHELLPROCINFO
{
	PROCESS_INFORMATION pi;
	HANDLE FReadPipe;
	HANDLE FWritePipe;

}SHELLPROCINFO, *PSHELLPROCINFO;


BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	DWORD       pid = 0;
	PENUMINFO   pInfo = (PENUMINFO)lParam;
	TCHAR       szTitle[_MAX_PATH + 1];

	// sanity checks
	if (pInfo == NULL)
		// stop the enumeration if invalid parameter is given
		return(FALSE);

	// get the processid for this window
	if (!::GetWindowThreadProcessId(hWnd, &pid))
		// this should never occur :-)
		return(TRUE);

	// compare the process ID with the one given as search parameter
	if (pInfo->PId == pid)
	{
		// look for the visibility first
		if (::IsWindowVisible(hWnd))
		{
			// look for the title next
			if (::GetWindowText(hWnd, szTitle, _MAX_PATH) != 0)
			{
				pInfo->hWnd = hWnd;

				// we have found the right window
				return(FALSE);
			}
			else
				pInfo->hEmptyWnd = hWnd;
		}
		else
		{
			// look for the title next
			if (::GetWindowText(hWnd, szTitle, _MAX_PATH) != 0)
			{
				pInfo->hInvisibleWnd = hWnd;
			}
			else
				pInfo->hEmptyInvisibleWnd = hWnd;
		}
	}

	// continue the enumeration
	return(TRUE);
}
static int set_curent_dir(lua_State *L)
{
	const char* d = luaL_checkstring(L, -1);
	strcpy(currentDir, d);
	lua_pushstring(L, currentDir);
	return 1;
}
static int load_kb_layout(lua_State *L)
{
	const char* pnL1 = luaL_checkstring(L, -1);
	HKL hkl = LoadKeyboardLayout(pnL1, KLF_SUBSTITUTE_OK | KLF_NOTELLSHELL);
	if (!hkl){
		lua_pushboolean(L, false);
		return 1;
	}
	ActivateKeyboardLayout(hkl, KLF_REORDER);
	lua_pushboolean(L, true);
	return 1;
}
static int datetime(lua_State *L){
	struct tm *newtime;
	__int64 ltime;

	_time64(&ltime);

	// Obtain coordinated universal time:
	newtime = _localtime64(&ltime); // C4996

	lua_pushinteger(L, newtime->tm_year + 1900);
	lua_pushinteger(L, newtime->tm_mon);
	lua_pushinteger(L, newtime->tm_mday);
	lua_pushinteger(L, newtime->tm_hour);
	lua_pushinteger(L, newtime->tm_min);
	lua_pushinteger(L, newtime->tm_sec);
	return 6;
}
static int async_mouse_state(lua_State *L){

	lua_pushinteger(L, ::GetAsyncKeyState(VK_LBUTTON));
	lua_pushinteger(L, ::GetAsyncKeyState(VK_MBUTTON));
	lua_pushinteger(L, ::GetAsyncKeyState(VK_RBUTTON));
	return 3;
}

static int activate_proc_wnd(lua_State *L)
{
	ENUMINFO EnumInfo;
	HWND w;
	DWORD PId;
	PId = (DWORD)luaL_checknumber(L, 1);
	// set the search parameters 
	EnumInfo.PId = PId;

	// set the return parameters to default values
	EnumInfo.hWnd = NULL;
	EnumInfo.hEmptyWnd = NULL;
	EnumInfo.hInvisibleWnd = NULL;
	EnumInfo.hEmptyInvisibleWnd = NULL;

	// do the search among the top level windows
	::EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)&EnumInfo);

	// return the one found if any
	w = NULL;
	if (EnumInfo.hWnd != NULL)
		w = EnumInfo.hWnd;
	else if (EnumInfo.hEmptyWnd != NULL)
		w = EnumInfo.hEmptyWnd;
	else if (EnumInfo.hInvisibleWnd != NULL)
		w = EnumInfo.hInvisibleWnd;
	else
		w = EnumInfo.hEmptyInvisibleWnd;
	if (w){
		int ret = ::SetForegroundWindow(w);
		lua_pushnumber(L, ret);
	} else lua_pushnumber(L, -1);
	return 1;
}
static int rename_file(lua_State *L)
{
	std::filesystem::path fo;
	std::filesystem::path fn;
	try {
		fo = std::filesystem::u8path(luaL_checkstring(L, -2));
		fn = std::filesystem::u8path(luaL_checkstring(L, -1));
	} catch (...) {
		return 0;
	}

	SHFILEOPSTRUCTW ls;
	WCHAR buff[MAX_PATH + 1];
	WCHAR buff2[MAX_PATH + 1];
	::ZeroMemory(&ls, sizeof(SHFILEOPSTRUCTW));
	::ZeroMemory(buff, MAX_PATH + 1);
	::ZeroMemory(buff2, MAX_PATH + 1);


	wcscpy(buff2, fn.c_str());
	wcscpy(buff, fo.c_str());
	ls.pFrom = buff;
	ls.pTo = buff2;
	ls.wFunc = FO_RENAME;
	ls.fFlags = FOF_SILENT |  FOF_NOCONFIRMATION; //  FOF_NOERRORUI |

	lua_pushnumber(L, SHFileOperationW(&ls));
	return 1;
}

static int setfileattr( lua_State* L )
{
	std::filesystem::path f;
	try {
		f = std::filesystem::u8path(luaL_checkstring(L, -2));
	} catch (...) {
		lua_pushboolean(L, false);
		return 1;
	}

	DWORD attr = luaL_checkint( L, -1 );
	lua_pushboolean( L, ::SetFileAttributesW(f.c_str(), attr));
	return 1;
}

static int fileexists(lua_State* L) {
	std::filesystem::path f;
	try {
		f = std::filesystem::u8path(luaL_checkstring(L, 1));
	} catch (...) {
		lua_pushboolean(L, false);
		return 1;
	}
	lua_pushboolean(L, ::PathFileExistsW(f.c_str()) != FALSE);
	return 1;
}

static int clock_start(lua_State* L) {
	lua_pushnumber(L, clock());
	return 1;
}
static int clock_diff(lua_State* L) {
	lua_Number n = luaL_checknumber(L, 1);
	lua_pushnumber(L, (clock() - n) / CLOCKS_PER_SEC * 1000);
	return 1;
}

static int greate_directory(lua_State* L) {
	std::filesystem::path f;
	try {
		f = std::filesystem::u8path(luaL_checkstring(L, 1));
	} catch (...) {
		lua_pushboolean(L, false);
		return 1;
	}
	std::error_code ec;
	std::filesystem::create_directory(f, ec);
	lua_pushboolean(L, ec.value() == 0);
	return 1;
}

static int proc_Exit(lua_State* L) {
	return 0;
}

static int proc_Continue(lua_State* L) {
	PSHELLPROCINFO sh = (PSHELLPROCINFO)lua_touserdata(L, 1);
	static const int MAX_CMD = 1024;

	if (!sh) {
		lua_pushstring(L, "E");
		lua_pushstring(L, "proc_Continue: Argument 1 isn't a process");
		return 2;
	}

	try {
		DWORD BytesToRead = 0;
		DWORD BytesRead = 0;
		DWORD TotalBytesAvail = 0;
		DWORD PipeReaded = 0;
		DWORD exit_code = 0;
		CMemBuffer< char, MAX_CMD > bufCmdLine; // строковой буфер длиной MAX_CMD
		char bufStr[MAX_CMD];
		while (::PeekNamedPipe(sh->FReadPipe, NULL, 0, &BytesRead, &TotalBytesAvail, NULL)) {
			if (TotalBytesAvail == 0) {
				if (::GetExitCodeProcess(sh->pi.hProcess, &exit_code) == FALSE ||
					exit_code != STILL_ACTIVE) {
					lua_pushstring(L, "S");
					lua_pushstring(L, "");
					break;
				} else {
					Sleep(10);
					continue;
				}
			} else {
				while (TotalBytesAvail > BytesRead) {
					if (TotalBytesAvail - BytesRead > MAX_CMD - 1) {
						BytesToRead = MAX_CMD - 1;
					} else {
						BytesToRead = TotalBytesAvail - BytesRead;
					}
					if (::ReadFile(sh->FReadPipe,
						bufCmdLine.GetBuffer(),
						BytesToRead,
						&PipeReaded,
						NULL) == FALSE) {
						lua_pushstring(L, "E");
						lua_pushstring(L, "ReadPipe Error");
						break;
					}
					if (PipeReaded <= 0) continue;
					BytesRead += PipeReaded;
					bufCmdLine[PipeReaded] = '\0';
					::OemToAnsi(bufCmdLine.GetBuffer(), bufStr);
					lua_pushstring(L, "C");
					lua_pushstring(L, bufStr);
					return 2;
				}
			}
		}
	} catch (...) {
	}

	// Код завершения процесса
	DWORD out_exitcode;
	::GetExitCodeProcess(sh->pi.hProcess,  &out_exitcode);
	lua_pushnumber(L, out_exitcode);
	::CloseHandle(sh->pi.hProcess);
	::CloseHandle(sh->FReadPipe);
	::CloseHandle(sh->FWritePipe);
	return 3;
}

static int do_startProc(lua_State* L) {
	static const int MAX_CMD = 1024;
	PSHELLPROCINFO sh = (PSHELLPROCINFO)lua_newuserdata(L, sizeof(SHELLPROCINFO));

	const char* strPath = luaL_checkstring(L, 1);

	STARTUPINFOA si = { sizeof(STARTUPINFOA) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	// устанавливаем именованные каналы на потоки ввода/вывода
	BOOL bUsePipes = FALSE;
	HANDLE FWritePipe = NULL;
	HANDLE FReadPipe = NULL;
	SECURITY_ATTRIBUTES pa = { sizeof(pa), NULL, TRUE };
	bUsePipes = ::CreatePipe(&(sh->FReadPipe), &(sh->FWritePipe), &pa, 0);
	if (bUsePipes != FALSE) {
		si.hStdOutput = sh->FWritePipe;
		si.hStdInput = sh->FReadPipe;
		si.hStdError = sh->FWritePipe;
		si.dwFlags = STARTF_USESTDHANDLES | si.dwFlags;
	}

	// запускаем процесс
	CMemBuffer< char, MAX_CMD > bufCmdLine; // строковой буфер длиной MAX_CMD
	bufCmdLine.GetBuffer()[0] = 0;

	strcat(bufCmdLine.GetBuffer(), strPath);

	const char *lp = NULL;
	if (lua_isstring(L, 2)) {
		lp = luaL_checkstring(L, 2);
	}
	sh->pi = { 0 };
	BOOL RetCode = ::CreateProcessA(NULL, // не используем имя файла, все в строке запуска
		bufCmdLine.GetBuffer(), // строка запуска
		NULL, // Process handle not inheritable
		NULL, // Thread handle not inheritable
		TRUE, // Set handle inheritance to FALSE
		0, // No creation flags
		NULL, // Use parent's environment block
		lp, //path.GetDirectory(), // устанавливаем дирректорию запуска
		&si, // STARTUPINFO
		&(sh->pi)); // PROCESS_INFORMATION

// если провалили запуск сообщаем об ошибке
	if (RetCode == FALSE) {
		::CloseHandle(sh->FReadPipe);
		::CloseHandle(sh->FWritePipe);
		lua_pushboolean(L, FALSE);
		lua_pushstring(L, "Can't run process");

		char* lpMsgBuf = NULL;
		::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),   // Default language
			lpMsgBuf,
			0,
			NULL
		);
		lua_pushstring(L, lpMsgBuf);
		delete sh;
		return 3;
	}

	// закрываем описатель потока, в нем нет необходимости 
	::CloseHandle(sh->pi.hThread);
	luaL_getmetatable(L, SHELLPROCOBJECT);
	lua_setmetatable(L, -2);
	return 1;
}

// запустить через CreateProcess в скрытом режиме
static BOOL RunProcessHide( CPath& path, DWORD* out_exitcode, CSimpleString* strOut )
{
	static const int MAX_CMD = 1024;

	STARTUPINFOA si = { sizeof(STARTUPINFOA) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	// устанавливаем именованные каналы на потоки ввода/вывода
	BOOL bUsePipes = FALSE;
	HANDLE FWritePipe = NULL;
	HANDLE FReadPipe = NULL;
	SECURITY_ATTRIBUTES pa = { sizeof(pa), NULL, TRUE };
	bUsePipes = ::CreatePipe( &FReadPipe, &FWritePipe, &pa, 0 );
	if ( bUsePipes != FALSE )
	{
		si.hStdOutput = FWritePipe;
		si.hStdInput = FReadPipe;
		si.hStdError = FWritePipe;
		si.dwFlags = STARTF_USESTDHANDLES | si.dwFlags;
	}

	// запускаем процесс
	CMemBuffer< char, MAX_CMD > bufCmdLine; // строковой буфер длиной MAX_CMD
	bufCmdLine.GetBuffer()[0] = 0;
	strcat( bufCmdLine.GetBuffer(), "\"" );
	strcat( bufCmdLine.GetBuffer(), path.GetPath() );
	strcat( bufCmdLine.GetBuffer(), "\"" );
	if ( path.GetFileParams() != NULL )
	{
		strcat( bufCmdLine.GetBuffer(), " " );
		strcat( bufCmdLine.GetBuffer(), path.GetFileParams() );
	}
	char *lp = NULL;
	if (strlen(currentDir) > 0)	lp = currentDir;
	PROCESS_INFORMATION pi = { 0 };
	BOOL RetCode = ::CreateProcessA( NULL, // не используем имя файла, все в строке запуска
									 bufCmdLine.GetBuffer(), // строка запуска
									 NULL, // Process handle not inheritable
									 NULL, // Thread handle not inheritable
									 TRUE, // Set handle inheritance to FALSE
									 0, // No creation flags
									 NULL, // Use parent's environment block
									 lp, //path.GetDirectory(), // устанавливаем дирректорию запуска
									 &si, // STARTUPINFO
									 &pi ); // PROCESS_INFORMATION

	// если провалили запуск сообщаем об ошибке
	if ( RetCode == FALSE )
	{
		::CloseHandle( FReadPipe );
		::CloseHandle( FWritePipe );
		return FALSE;
	}

	// закрываем описатель потока, в нем нет необходимости 
	::CloseHandle( pi.hThread );

	// ожидаем завершение работы процесса
	try
	{
		DWORD BytesToRead = 0;
		DWORD BytesRead = 0;
		DWORD TotalBytesAvail = 0;
		DWORD PipeReaded = 0;
		DWORD exit_code = 0;
		CMemBuffer< char, MAX_CMD > bufStr; // строковой буфер длиной MAX_CMD
		while ( ::PeekNamedPipe( FReadPipe, NULL, 0, &BytesRead, &TotalBytesAvail, NULL ) )
		{
			if ( TotalBytesAvail == 0 )
			{
				if ( ::GetExitCodeProcess( pi.hProcess, &exit_code ) == FALSE ||
					 exit_code != STILL_ACTIVE )
				{
					break;
				}
				else
				{
					Sleep(10);
					continue;
				}
			}
			else
			{
				while ( TotalBytesAvail > BytesRead )
				{
					if ( TotalBytesAvail - BytesRead > MAX_CMD - 1 )
					{
						BytesToRead = MAX_CMD - 1;
					}
					else
					{
						BytesToRead = TotalBytesAvail - BytesRead;
					}
					if ( ::ReadFile( FReadPipe,
									 bufCmdLine.GetBuffer(),
									 BytesToRead,
									 &PipeReaded,
									 NULL ) == FALSE )
					{
						break;
					}
					if ( PipeReaded <= 0 ) continue;
					BytesRead += PipeReaded;
					bufCmdLine[ PipeReaded ] = '\0';
					::OemToAnsi( bufCmdLine.GetBuffer(), bufStr.GetBuffer() );
					strOut->Append( bufStr.GetBuffer() );
				}
			}
		}
	}
	catch (...)
	{
	}

	// Код завершения процесса
	::GetExitCodeProcess( pi.hProcess, out_exitcode );
	::CloseHandle( pi.hProcess );
	::CloseHandle( FReadPipe );
	::CloseHandle( FWritePipe );
	return TRUE;
}

// запустить через ShellExecuteEx в скрытом режиме
// (см. шаманство с консолью)
static BOOL ExecuteHide( CPath& path, DWORD* out_exitcode, CSimpleString* strOut )
{
	HANDLE hSaveStdin = NULL;
	HANDLE hSaveStdout = NULL;
	HANDLE hChildStdoutRdDup = NULL;
	HANDLE hChildStdoutWr = NULL;
	try
	{
		// подключаем консоль
		STARTUPINFOA si = { sizeof(STARTUPINFOA) };
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		PROCESS_INFORMATION pi = { 0 };
		char command_line[] = "cmd";
		::CreateProcessA( NULL, // не используем имя файла, все в строке запуска
						  command_line, // Command line
						  NULL, // Process handle not inheritable
						  NULL, // Thread handle not inheritable
						  TRUE, // Set handle inheritance to FALSE
						  0, // No creation flags
						  NULL, // Use parent's environment block
						  NULL, // Use parent's starting directory
						  &si, // STARTUPINFO
						  &pi ); // PROCESS_INFORMATION
		// задержка чтобы консоль успела создаться
		::WaitForSingleObject( pi.hProcess, 100 );
		BOOL hResult = FALSE;
		HMODULE hLib = LoadLibraryA("Kernel32.dll");
		if ( hLib != NULL )
		{
			typedef BOOL (STDAPICALLTYPE *ATTACHCONSOLE)( DWORD dwProcessId );
			ATTACHCONSOLE _AttachConsole = NULL;
			_AttachConsole = (ATTACHCONSOLE)GetProcAddress( hLib, "AttachConsole" );
			if ( _AttachConsole ) hResult = _AttachConsole( pi.dwProcessId );
			FreeLibrary( hLib );
		}
		if ( hResult == FALSE ) AllocConsole();

		TerminateProcess( pi.hProcess, 0 );
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );

		HANDLE hChildStdinRd;
		HANDLE hChildStdinWr;
		HANDLE hChildStdinWrDup;
		HANDLE hChildStdoutRd;

		// Set the bInheritHandle flag so pipe handles are inherited. 
		SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
		BOOL fSuccess;

		// The steps for redirecting child process's STDOUT: 
		//     1. Save current STDOUT, to be restored later. 
		//     2. Create anonymous pipe to be STDOUT for child process. 
		//     3. Set STDOUT of the parent process to be write handle to 
		//        the pipe, so it is inherited by the child process. 
		//     4. Create a noninheritable duplicate of the read handle and
		//        close the inheritable read handle. 

		// Save the handle to the current STDOUT. 
		hSaveStdout = GetStdHandle( STD_OUTPUT_HANDLE );

		// Create a pipe for the child process's STDOUT.
		if ( !CreatePipe( &hChildStdoutRd, &hChildStdoutWr, &saAttr, 0 ) ) throw(1);

		// Set a write handle to the pipe to be STDOUT. 
		if ( !SetStdHandle( STD_OUTPUT_HANDLE, hChildStdoutWr ) ) throw(1);

		// Create noninheritable read handle and close the inheritable read 
		// handle.
		fSuccess = DuplicateHandle( GetCurrentProcess(),
									hChildStdoutRd,
									GetCurrentProcess(),
									&hChildStdoutRdDup,
									0,
									FALSE,
									DUPLICATE_SAME_ACCESS );
		if( fSuccess == FALSE ) throw(1);
		CloseHandle( hChildStdoutRd );

		// The steps for redirecting child process's STDIN: 
		//     1.  Save current STDIN, to be restored later. 
		//     2.  Create anonymous pipe to be STDIN for child process. 
		//     3.  Set STDIN of the parent to be the read handle to the 
		//         pipe, so it is inherited by the child process. 
		//     4.  Create a noninheritable duplicate of the write handle, 
		//         and close the inheritable write handle. 

		// Save the handle to the current STDIN. 
		hSaveStdin = GetStdHandle( STD_INPUT_HANDLE );

		// Create a pipe for the child process's STDIN. 
		if ( !CreatePipe( &hChildStdinRd, &hChildStdinWr, &saAttr, 0 ) ) throw(1);

		// Set a read handle to the pipe to be STDIN. 
		if ( !SetStdHandle( STD_INPUT_HANDLE, hChildStdinRd ) ) throw(1);

		// Duplicate the write handle to the pipe so it is not inherited. 
		fSuccess = DuplicateHandle( GetCurrentProcess(),
									hChildStdinWr,
									GetCurrentProcess(),
									&hChildStdinWrDup,
									0,
									FALSE,
									DUPLICATE_SAME_ACCESS );
		if ( fSuccess == FALSE ) throw(1);

		CloseHandle( hChildStdinWr );
	}
	catch (...)
	{
		return FALSE;
	}

	// Now create the child process.
	SHELLEXECUTEINFOA shinf = { sizeof(SHELLEXECUTEINFOA) };
	shinf.lpFile = path.GetPath();
	shinf.lpParameters = path.GetFileParams();
	//shinf.lpDirectory = path.GetDirectory();
	shinf.fMask = SEE_MASK_FLAG_NO_UI |
				  SEE_MASK_NO_CONSOLE |
				  SEE_MASK_FLAG_DDEWAIT |
				  SEE_MASK_NOCLOSEPROCESS;
	shinf.nShow = SW_HIDE;
	BOOL bSuccess = ::ShellExecuteExA( &shinf );
	if ( bSuccess && shinf.hInstApp <= (HINSTANCE)32 ) bSuccess = FALSE;
	HANDLE hProcess = shinf.hProcess;

	try
	{
		if ( bSuccess == FALSE || hProcess == NULL ) throw(1);

		if ( hChildStdoutWr != NULL )
		{
			CloseHandle( hChildStdoutWr );
			hChildStdoutWr = NULL;
		}

		// After process creation, restore the saved STDIN and STDOUT.
		if ( hSaveStdin != NULL )
		{
			if ( !SetStdHandle( STD_INPUT_HANDLE, hSaveStdin ) ) throw(1);
			CloseHandle( hSaveStdin );
			hSaveStdin = NULL;
		}

		if ( hSaveStdout != NULL )
		{
			if ( !SetStdHandle( STD_OUTPUT_HANDLE, hSaveStdout ) ) throw(1);
			CloseHandle( hSaveStdout );
			hSaveStdout = NULL;
		}

		if ( hChildStdoutRdDup != NULL )
		{
			// Read output from the child process, and write to parent's STDOUT.
			const int BUFSIZE = 1024;
			DWORD dwRead;
			CMemBuffer< char, BUFSIZE > bufStr; // строковой буфер
			CMemBuffer< char, BUFSIZE > bufCmdLine; // строковой буфер
			for (;;)
			{
				if( ReadFile( hChildStdoutRdDup,
							  bufCmdLine.GetBuffer(),
							  BUFSIZE,
							  &dwRead,
							  NULL ) == FALSE ||
					dwRead == 0 )
				{
					DWORD exit_code = 0;
					if ( ::GetExitCodeProcess( hProcess, &exit_code ) == FALSE ||
						 exit_code != STILL_ACTIVE )
					{
						break;
					}
					else
					{
						continue;
					}
				}
				bufCmdLine[ dwRead ] = '\0';
				::OemToAnsi( bufCmdLine.GetBuffer(), bufStr.GetBuffer() );
				strOut->Append( bufStr.GetBuffer() );
			}
			CloseHandle( hChildStdoutRdDup );
			hChildStdoutRdDup = NULL;
		}
		FreeConsole();
	}
	catch (...)
	{
		if ( hChildStdoutWr != NULL ) CloseHandle( hChildStdoutWr );
		if ( hSaveStdin != NULL ) CloseHandle( hSaveStdin );
		if ( hSaveStdout != NULL ) CloseHandle( hSaveStdout );
		if ( hChildStdoutRdDup != NULL ) CloseHandle( hChildStdoutRdDup );
		if ( bSuccess == FALSE || hProcess == NULL ) return FALSE;
	}

	::GetExitCodeProcess( hProcess, out_exitcode );
	CloseHandle( hProcess );
	return TRUE;
}

static int exec( lua_State* L )
{
	// считываем запускаемую команду
	CPath file = luaL_checkstring( L, 1 );
	const char* verb = lua_tostring( L, 2 );
	int noshow = lua_toboolean( L, 3 );
	int dowait = lua_toboolean( L, 4 );

	BOOL useConsoleOut = dowait && noshow && ( verb == NULL );

	DWORD exit_code = (DWORD)-1;
	BOOL bSuccess = FALSE;
	CSimpleString strOut;

	if ( useConsoleOut != FALSE )
	{
		bSuccess = RunProcessHide( file, &exit_code, &strOut ) ||
				   ExecuteHide( file, &exit_code, &strOut );
	}
	else
	{
		HANDLE hProcess = NULL;
		// запускаем процесс
		if ( verb != NULL && // если есть команда запуска
			 strcmp( verb, "explore" ) == 0 && // если команда запуска explore
			 CPath::IsFileExists( file.GetPath() ) ) // проверяем файл ли это
		{
			SHELLEXECUTEINFOA shinf = { sizeof(SHELLEXECUTEINFOA) };
			shinf.lpFile = "explorer.exe";
			CSimpleString sFileParams;
			sFileParams.Append( "/e, /select," );
			sFileParams.Append( file.GetPath() );
			shinf.lpParameters = sFileParams.GetString();
			shinf.fMask = SEE_MASK_FLAG_NO_UI |
						  SEE_MASK_NO_CONSOLE |
						  SEE_MASK_FLAG_DDEWAIT |
						  SEE_MASK_NOCLOSEPROCESS;
			shinf.nShow = noshow ? SW_HIDE : SW_SHOWNORMAL;
			bSuccess = ::ShellExecuteExA( &shinf );
			if ( bSuccess && shinf.hInstApp <= (HINSTANCE)32 ) bSuccess = FALSE;
			hProcess = shinf.hProcess;
		}
		else if ( verb != NULL && // если есть команда запуска
				  strcmp( verb, "select" ) == 0 && // если команда запуска select
				  CPath::IsPathExist( file.GetPath() ) ) // проверяем правильный путь
		{
			SHELLEXECUTEINFOA shinf = { sizeof(SHELLEXECUTEINFOA) };
			shinf.lpFile = "explorer.exe";
			CSimpleString sFileParams;
			sFileParams.Append( "/select," );
			sFileParams.Append( file.GetPath() );
			shinf.lpParameters = sFileParams.GetString();
			shinf.fMask = SEE_MASK_FLAG_NO_UI |
						  SEE_MASK_NO_CONSOLE |
						  SEE_MASK_FLAG_DDEWAIT |
						  SEE_MASK_NOCLOSEPROCESS;
			shinf.nShow = noshow ? SW_HIDE : SW_SHOWNORMAL;
			bSuccess = ::ShellExecuteExA( &shinf );
			if ( bSuccess && shinf.hInstApp <= (HINSTANCE)32 ) bSuccess = FALSE;
			hProcess = shinf.hProcess;
		}
		else
		{
			SHELLEXECUTEINFOA shinf = { sizeof(SHELLEXECUTEINFOA) };
			shinf.lpFile = file.GetPath();
			shinf.lpParameters = file.GetFileParams();
			shinf.lpVerb = verb;
			if (strlen(currentDir) > 3) {
				shinf.lpDirectory = currentDir;
			}
			shinf.fMask = SEE_MASK_FLAG_NO_UI |
						  SEE_MASK_NO_CONSOLE |
						  SEE_MASK_FLAG_DDEWAIT;
			if ( verb == NULL )
			{
				shinf.fMask |= SEE_MASK_NOCLOSEPROCESS;
			}
			else
			{
				shinf.fMask |= SEE_MASK_INVOKEIDLIST;
			}
			shinf.nShow = noshow ? SW_HIDE : SW_SHOWNORMAL;
			bSuccess = ::ShellExecuteExA( &shinf );
			if ( bSuccess && shinf.hInstApp <= (HINSTANCE)32 ) bSuccess = FALSE;
			hProcess = shinf.hProcess;
		}

		if ( dowait != FALSE && hProcess != NULL )
		{
			// ждем пока процесс не завершится
			::WaitForSingleObject( hProcess, INFINITE );
		}

		if ( hProcess != NULL )
		{
			if ( dowait != FALSE ) ::GetExitCodeProcess( hProcess, &exit_code );
			CloseHandle( hProcess );
		}

		if ( bSuccess != FALSE )
		{
			::SetLastError( 0 );
			DWORD dw;
			int len;
			char* lpMsgBuf = GetLastErrorString( &dw, &len );
			strOut.Append( lpMsgBuf );
			::LocalFree( lpMsgBuf );
		}
	}

	if ( bSuccess == FALSE )
	{
		lua_pushboolean( L, FALSE );
		lua_pushlasterr( L, NULL );
	}
	else
	{
		exit_code != (DWORD)-1 ? lua_pushnumber( L, exit_code ) : lua_pushboolean( L, TRUE );
		lua_pushstring( L, strOut.GetString() );
	}

	return 2;
}

static int getclipboardtext( lua_State* L )
{
	CSimpleString clipText;
	if ( ::IsClipboardFormatAvailable( CF_TEXT ) )
	{
		if ( ::OpenClipboard( NULL ) )
		{
			HANDLE hData = ::GetClipboardData( CF_TEXT );
			if ( hData != NULL )
			{
				clipText.Append( (char*)::GlobalLock( hData ) );
				::GlobalUnlock( hData );
			}
			::CloseClipboard();
		}
	}
	lua_pushstring( L, clipText.GetString() );
	return 1;
}

static int findfiles( lua_State* L )
{
	const char* filename = luaL_checkstring( L, 1 );

	WIN32_FIND_DATAA findFileData;
	HANDLE hFind = ::FindFirstFileA( filename, &findFileData );
	if ( hFind != INVALID_HANDLE_VALUE )
	{
		// create table for result
		lua_createtable( L, 1, 0 );

		lua_Integer num = 1;
		BOOL isFound = TRUE;
		while ( isFound != FALSE )
		{
			// store file info
			lua_pushinteger( L, num );
			lua_createtable( L, 0, 4 );

			lua_pushstring( L, findFileData.cFileName );
			lua_setfield( L, -2, "name" );

			lua_pushboolean( L, findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY );
			lua_setfield( L, -2, "isdirectory" );

			lua_pushinteger( L, findFileData.dwFileAttributes );
			lua_setfield( L, -2, "attributes" );

			lua_pushinteger( L, findFileData.nFileSizeHigh * ((lua_Number)MAXDWORD + 1) +
							   findFileData.nFileSizeLow );
			lua_setfield( L, -2, "size" );


			__int64 t = findFileData.ftLastWriteTime.dwLowDateTime + ((__int64)findFileData.ftLastWriteTime.dwHighDateTime << 32);
			lua_pushnumber(L, (lua_Number)t);
			lua_setfield(L, -2, "writetime");

			lua_settable(L, -3);
			num++;

			// next
			isFound = ::FindNextFileA( hFind, &findFileData );
		}

		::FindClose( hFind );

		return 1;
	}

	// files not found
	return 0;
}

struct W2MB
{
	W2MB( const wchar_t *src, int cp = CP_UTF7 )
			: buffer(0)
		{
			int len = ::WideCharToMultiByte( cp, 0, src, -1, 0, 0, 0, 0 );
			if ( len )
			{
				buffer = new char[len];
				len = ::WideCharToMultiByte( cp, 0, src, -1, buffer, len, 0, 0 );
			}
		}
	~W2MB()
		{ delete[] buffer; }
	const char *c_str() const
		{ return buffer; }
private:
	char *buffer;
};

struct MB2W
{
	MB2W( const char *src, int cp = CP_UTF7 )
			: buffer(0)
		{
			int len = ::MultiByteToWideChar( cp, 0, src, -1, 0, 0 );
			if ( len )
			{
				buffer = new wchar_t[len];
				len = ::MultiByteToWideChar( cp, 0, src, -1, buffer, len );
			}
		}
	~MB2W()
		{ delete[] buffer; }
	const wchar_t *c_str() const
		{ return buffer; }
private:
	wchar_t *buffer;
};

static int get_clipboard( lua_State* L )
{
	HGLOBAL hglb=0; 
	LPSTR lpstr;
	bool needUnic=TRUE;
	int i=0;

	OSVERSIONINFO verinf;
	verinf.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	GetVersionEx(&verinf);
	LCID lcid=0;
	int nSz = 0;
	LPSTR pText = NULL;

	HWND hWnd=NULL;

	while (!OpenClipboard(hWnd)) 
	{
		Sleep(10);
		i++;
		if (i>100)return 0;
	}
	UINT uForm=0;
	do
	{
		if (uForm==CF_TEXT)
		{
			needUnic=FALSE;
			break;
		}else if(uForm==CF_UNICODETEXT)
		{
			needUnic=TRUE;
			break;
		}
		uForm=EnumClipboardFormats(uForm);
	}while(uForm);

	if (IsClipboardFormatAvailable(CF_LOCALE))//&&needUnic)
	{
		char cls[24];
		ZeroMemory((void*)cls,24);
		GetClassName(GetForegroundWindow(),cls,24);
		if(!strcmp(cls,"FrontPageExplorerWindow"))
		{
			hglb = GetClipboardData(CF_LOCALE);
			if (!hglb) goto ERR;
			lcid=*((LCID*)GlobalLock(hglb));
			LCID lll=GetSystemDefaultLCID();
			needUnic=(lll!=lcid);
			GlobalUnlock(hglb);
		}
		if(!strcmp(cls,"wndclass_desked_gsk")||!strcmp(cls,"FNWND3100"))needUnic=TRUE;
	}
	if (IsClipboardFormatAvailable(CF_UNICODETEXT)&&needUnic)
	{
		hglb = GetClipboardData(CF_UNICODETEXT);
		//SIZE_T size=GlobalSize(hglb);||!size
		if (!hglb) goto ERR;
		lpstr = (LPSTR)GlobalLock(hglb);
		//DWORD l=lstrlen(lpstr)+1;
		//if(l<size)size=l;
		nSz=WideCharToMultiByte(CP_ACP,0,(WCHAR*)lpstr,-1,
			NULL,0,NULL,NULL);	

		pText=new char[nSz];
		WideCharToMultiByte(CP_ACP,0,(WCHAR*)lpstr,-1,
			pText,nSz,NULL,NULL);

	}else
	{
		hglb = GetClipboardData(CF_TEXT);
		if (!hglb) goto ERR;

		lpstr = (LPSTR)GlobalLock(hglb);
		if (!lpstr) goto ERR;

		nSz=lstrlen(lpstr)+1;

			pText=new char[nSz];
			lstrcpyn(pText,lpstr,nSz);
	}
	GlobalUnlock(hglb);
	hglb=NULL;

	CloseClipboard(); 

	if(pText)
	{
		lua_pushstring( L, pText );
		delete pText;
		return 1;
	}

ERR:
	return 0;
}

static int set_clipboard(lua_State* L)
{
	if ( lua_isstring(L, 1) )
	{
		size_t textLen;
		const char *textData = lua_tolstring( L, 1, &textLen );

		LPTSTR  lptstrCopy; 
		HGLOBAL hglbCopy; 
		int i=0;

		while (!OpenClipboard(NULL)) 
		{
			Sleep(10);
			i++;
			if (i>100) return 0;
		}
		EmptyClipboard(); 

		hglbCopy = GlobalAlloc(GMEM_DDESHARE, 
			(textLen + 1) * sizeof(TCHAR)); 
		if (hglbCopy == NULL) 
		{ 
			CloseClipboard(); 
			return 0; 
		} 

		// Lock the handle and copy the text to the buffer. 

		lptstrCopy = (LPSTR)GlobalLock(hglbCopy); 
		lstrcpy(lptstrCopy,textData);
		GlobalUnlock(hglbCopy);		

		// Place the handle on the clipboard. 

		SetClipboardData(CF_TEXT, hglbCopy); 

		hglbCopy = GlobalAlloc(GMEM_DDESHARE,sizeof(LCID)); 
		LCID *lcid=(LCID*)GlobalLock(hglbCopy); 
		*lcid=GetSystemDefaultLCID();
		GlobalUnlock(hglbCopy);		
		SetClipboardData(CF_LOCALE, hglbCopy); 

		CloseClipboard(); 
		return 0;
	}
	return 0;
}

static int greateCuid(lua_State* L)
{
	GUID MyGuid;
	HRESULT hr = CoCreateGuid(&MyGuid);	hr;
	lua_pushinteger(L, MyGuid.Data1);
	lua_pushinteger(L, MyGuid.Data2);
	lua_pushinteger(L, MyGuid.Data3);
	lua_pushfstring(L, (const char*)MyGuid.Data4);
	return 4;
}

extern int showinputbox( lua_State* );

#pragma warning(pop)

luaL_Reg shell[] = 
{
	{ "greateCuid", greateCuid },
	{ "exec", exec },
    { "getfiletime", getfiletime },
    { "getfileattr", getfileattr },
	{ "setfileattr", setfileattr },				   
	{ "fileexists", fileexists },
	{ "getclipboardtext", getclipboardtext },
	{ "findfiles", findfiles },
	{ "get_clipboard", get_clipboard },
	{ "set_clipboard", set_clipboard },
	{ "delete_file", delete_file },
	{ "rename_file", rename_file },
	{ "activate_proc_wnd", activate_proc_wnd },
	{ "set_curent_dir", set_curent_dir },
	{ "datetime", datetime },
	{ "async_mouse_state", async_mouse_state },
	{ "load_kb_layout", load_kb_layout },
	{ "greateDirectory", greate_directory },
	{ "clockStart", clock_start },
	{ "clockDiff", clock_diff },
	{ "startProc", do_startProc },
	{ NULL, NULL }
};

luaL_Reg proc_methods[] = {
	{"Continue",proc_Continue},
	{"Exit",proc_Exit},
	{NULL, NULL},
};

extern "C" __declspec(dllexport) int luaopen_shell( lua_State* L )
{
	::ZeroMemory(currentDir, MAX_PATH + 1);
	luaL_register( L, "shell", shell );
	luaL_newmetatable(L, SHELLPROCOBJECT);  // create metatable for window objects
	lua_pushvalue(L, -1);  // push metatable
	lua_setfield(L, -2, "__index");  // metatable.__index = metatable
	luaL_register(L, NULL, proc_methods);
	return 1;
}
