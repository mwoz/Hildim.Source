// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__80916F7D_F768_460A_9794_47C2E73555EF__INCLUDED_)
#define AFX_STDAFX_H__80916F7D_F768_460A_9794_47C2E73555EF__INCLUDED_
#define _CONVERSION_DONT_USE_THREAD_LOCALE
#define USE_TLXLIB_VARIANT

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _WIN32_WINNT 0x0600

#define _AUTOINCREASEVERSIONFORRELEASE_		//autoincrease last number in version on successful release build
#define _AUTOINCREASEBOTHVERSIONS_			//autoincrease both File Version and Product Version

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxtempl.h>
#include <afxsock.h>		// MFC socket extensions
#include <atlconv.h>

#include <Utils\tlxlib_misc.h>
#include <Utils\sysLog.h>
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__80916F7D_F768_460A_9794_47C2E73555EF__INCLUDED_)
