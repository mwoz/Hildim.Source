#ifndef __IUP_FONTICON_H 
#define __IUP_FONTICON_H

#ifdef __cplusplus
extern "C" {
#endif

	struct _IcontrolData
	{
		int numIcons;
	};
	typedef struct _FontIconData
	{
		wchar_t ch;
		unsigned char r;
		unsigned char g;
		unsigned char b;
	} FontIconData;

#ifdef __cplusplus
}
#endif

#endif