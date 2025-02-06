#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_fonticon.h"
#include "iup_assert.h"
#include "iup_stdcontrols.h"
#include "iup_drvinfo.h"
#include "iup_array.h"



static int iupFontIconCreateMethod(Ihandle* ih, void** params)
{
    FontIconData* fid = NULL; 
    int count = 0;
	if (params && params[0])
	{
		//iupAttribSetStr(ih, "TITLE", (char*)(params[0]));
        const char* name = params[0];
        ih->data = iupALLOCCTRLDATA();


        int len, value_len, rez = 0;
        len = (int)strlen(name);

        char value[100];
        int maxIcon = -1;

        count = 1;
        for (char* ch = name; *ch; ch++) {
            if (*ch == '#')
                count++;
        }
        fid = (FontIconData*)malloc(count * sizeof(FontIconData));

        float fSize = 0;
        ih->data->invert = -1;
        
        do
        {   
            unsigned char q = 0;
            const char* next_name = iupStrNextValue(name, len, &value_len, '#');
            if (value_len)
            {
                  maxIcon++;
                  int i;
                  if (name[0] == ':' && maxIcon) {
                      fid[maxIcon].ch = fid[maxIcon - 1].ch + 1;
                  }
                  else if (sscanf(name, "%x", &i)) {
                      fid[maxIcon].ch = i;
                  }
                  else
                      goto err;

                  int elementsLen;

                  const char* elements = iupStrNextValue(name, value_len, &elementsLen, ':');
                  if (!elements)
                      goto err;
                  int len2 = value_len - elementsLen;
                  const char* elem = elements;
                  do {
                      
                      if (elem) {
                          switch (*elem) {
                          case 'I':
                              if (elem[1] != 'N' || elem[2] != 'V')
                                  goto err;
                              ih->data->invert = maxIcon;
                              break;
                          default:
                          {
                              fid[maxIcon].r = 0, fid[maxIcon].g = 0, fid[maxIcon].b = 0;

                              iupStrToRGB(elem, &(fid[maxIcon].r), &(fid[maxIcon].g), &(fid[maxIcon].b));
                          }
                          }
                      }
                      iupStrNextValue(elem, len2, &elementsLen, ',');
                      if (len2 > elementsLen + 1)
                          elem += (elementsLen + 1);
                      len2 -= (elementsLen + 1);

                  } while (len2 > 0);

            }

            name = next_name;
            len -= value_len + 1;
        } while (value_len);
        
	}
    iupAttribSet(ih, "WID", (char*)fid);
    ih->data->numIcons = count;
	return IUP_NOERROR;
err:
    if (fid)
        free(fid);
    return IUP_ERROR;
}

static char* iFontIconGetCountAttrib(Ihandle* ih)
{
    return iupStrReturnInt(ih->data->numIcons);
}
static char* iFontIconGetNumColorText(Ihandle* ih)
{
    return iupStrReturnInt(ih->data->invert);
}

static void iupFontIconDestroyMethod(Ihandle* ih)
{
    FontIconData* fid = (FontIconData*)iupAttribGet(ih, "WID");
    if (fid) {
        iupAttribSet(ih, "WID", NULL);
        free(fid);
    }

}

IUP_API Ihandle* IupFontIcon(const char* imgdescription)
{
	void* params[2];
	params[0] = (void*)imgdescription;
	params[1] = NULL;
	return IupCreatev("fonticon", params);
}

Iclass* iupFontIconNewClass(void)
{
	Iclass* ic = iupClassNew(NULL);

	ic->name = "fonticon";
	ic->format = "s"; /* (Ihandle**) */
	ic->nativetype = IUP_TYPEIMAGE;
	ic->childtype = IUP_CHILDNONE;  /**< can not add children using Append/Insert */
		
	ic->is_interactive = 1;

	/* Class functions */
	ic->New = iupFontIconNewClass;
	ic->Create = iupFontIconCreateMethod;
	ic->Destroy = iupFontIconDestroyMethod;

	/* Common */
    iupClassRegisterAttribute(ic, "WID", NULL, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NO_INHERIT | IUPAF_NO_STRING);
    iupClassRegisterAttribute(ic, "COUNT", iFontIconGetCountAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
    iupClassRegisterAttribute(ic, "NUMCOLORTEXT", iFontIconGetNumColorText, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);


	//iupdrvMenuInitClass(ic);

	return ic;
}

