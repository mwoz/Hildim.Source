/** \file
 * \brief PPTX library
 *
 * See Copyright Notice in cd.h
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

#include "cd.h"
#include "cd_private.h"

#include "lodepng.h"
#include "pptx.h"

#define to_angle(_)   (int)((_)*60000)

struct _pptxPresentation
{
  char baseDir[10240];

  FILE* slideFile;
  FILE* slideRelsFile;
  FILE* masterSlideFile;
  FILE* masterSlideRelsFile;
  FILE* presentationFile;

  FILE *tmpFile;
  FILE *tmpRelsFile;

  int slideHeight;
  int slideWidth;
  int slide_xfactor;
  int slide_yfactor;

  int slideNum;
  int objectNum;
  int mediaNum;
  int imageId;

  char* importMasterSlideFile;
};

#define PPTX_PPT_DIR          "ppt"
#define PPTX_PPT_RELS_DIR     "ppt/_rels"
#define PPTX_LAYOUTS_DIR      "ppt/slideLayouts"
#define PPTX_LAYOUTS_RELS_DIR "ppt/slideLayouts/_rels"
#define PPTX_MASTERS_DIR      "ppt/slideMasters"
#define PPTX_MASTERS_RELS_DIR "ppt/slideMasters/_rels"
#define PPTX_SLIDES_DIR       "ppt/slides"
#define PPTX_SLIDES_RELS_DIR  "ppt/slides/_rels"
#define PPTX_THEME_DIR        "ppt/theme"
#define PPTX_MEDIA_DIR        "ppt/media"
#define PPTX_MEDIA_MEDIA_DIR  "ppt/media/media"
#define PPTX_RELS_DIR         "_rels"

#define PPTX_CONTENT_TYPES_FILE     "[Content_Types].xml"
#define PPTX_PRESENTATION_FILE      "ppt/presentation.xml"
#define PPTX_PRES_PROPS_FILE        "ppt/presProps.xml"
#define PPTX_IMAGE_FILE             "ppt/media/media/image%d.png"
#define PPTX_SLIDE_LAYOUT_FILE      "ppt/slideLayouts/slideLayout1.xml"
#define PPTX_SLIDE_LAYOUT_RELS_FILE "ppt/slideLayouts/_rels/slideLayout1.xml.rels"
#define PPTX_SLIDE_MASTER_FILE      "ppt/slideMasters/slideMaster1.xml"
#define PPTX_SLIDE_MASTER_RELS      "ppt/slideMasters/_rels/slideMaster1.xml.rels"
#define PPTX_SLIDE_FILE             "ppt/slides/slide%d.xml"
#define PPTX_SLIDE_RELS_FILE        "ppt/slides/_rels/slide%d.xml.rels"
#define PPTX_THEME_FILE             "ppt/theme/theme1.xml"
#define PPTX_PRESENTATION_RELS      "ppt/_rels/presentation.xml.rels"
#define PPTX_RELS_FILE              "_rels/.rels"


int minizip(const char *filename, const char *dirname, const char **files, const int nFiles);
int miniunzip(const char *filename, const char *dirname);


/************************************  PRINT  ******************************************************/


static void printOpenSlide(FILE* slideFile, int objectNum)
{
  const char *prefix =
  {
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
    "<p:sld xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:mv=\"urn:schemas-microsoft-com:mac:vml\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" xmlns:dgm=\"http://schemas.openxmlformats.org/drawingml/2006/diagram\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:pvml=\"urn:schemas-microsoft-com:office:powerpoint\" xmlns:com=\"http://schemas.openxmlformats.org/drawingml/2006/compatibility\" xmlns:p14=\"http://schemas.microsoft.com/office/powerpoint/2010/main\">\n"
    "   <p:cSld>\n"
    "      <p:spTree>\n"
    "         <p:nvGrpSpPr>\n"
    "            <p:cNvPr id=\"%d\" name=\"Shape %d\"/>\n"
    "            <p:cNvGrpSpPr/>\n"
    "            <p:nvPr/>\n"
    "         </p:nvGrpSpPr>\n"
    "         <p:grpSpPr>\n"
    "            <a:xfrm>\n"
    "               <a:off x=\"0\" y=\"0\"/>\n"
    "               <a:ext cx=\"0\" cy=\"0\"/>\n"
    "               <a:chOff x=\"0\" y=\"0\"/>\n"
    "               <a:chExt cx=\"0\" cy=\"0\"/>\n"
    "            </a:xfrm>\n"
    "         </p:grpSpPr>\n"
  };

  fprintf(slideFile, prefix, objectNum, objectNum);
}

static void printOpenSlideRels(FILE* slideRelsFile)
{
  const char *rels =
  {
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
    "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
    "   <Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout\" Target=\"../slideLayouts/slideLayout1.xml\"/>\n"
  };

  fprintf(slideRelsFile, "%s", rels);
}

static void printPresProps(FILE* presPropsFile)
{
  const char *presProps =
  {
    "<?xml version = \"1.0\" encoding = \"UTF-8\" standalone = \"yes\"?>\n"
    "<p:presentationPr xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
    "xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:mv=\"urn:schemas-microsoft-com:mac:vml\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" "
    "xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" xmlns:dgm=\"http://schemas.openxmlformats.org/drawingml/2006/diagram\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
    "xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:pvml=\"urn:schemas-microsoft-com:office:powerpoint\" xmlns:com=\"http://schemas.openxmlformats.org/drawingml/2006/compatibility\" "
    "xmlns:p14=\"http://schemas.microsoft.com/office/powerpoint/2010/main\"/>\n"
  };

  fprintf(presPropsFile, "%s", presProps);
}

static void printRels(FILE* relsFile)
{
  const char *rels =
  {
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
    "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
    "   <Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"ppt/presentation.xml\"/>\n"
    "</Relationships>\n"
  };

  fprintf(relsFile, "%s", rels);
}

static void printLayoutRelsFile(FILE* layoutRelsFile)
{
  const char *rels =
  {
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
    "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
    "   <Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster\" Target=\"../slideMasters/slideMaster1.xml\"/>\n"
    "</Relationships>\n"
  };

  fprintf(layoutRelsFile, "%s", rels);
}

static void printLayoutFile(FILE* layoutFile)
{
  const char *rels =
  {
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
    "   <p:sldLayout xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
    "xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:mv=\"urn:schemas-microsoft-com:mac:vml\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" "
    "xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" xmlns:dgm=\"http://schemas.openxmlformats.org/drawingml/2006/diagram\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
    "xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:pvml=\"urn:schemas-microsoft-com:office:powerpoint\" xmlns:com=\"http://schemas.openxmlformats.org/drawingml/2006/compatibility\" "
    "xmlns:p14=\"http://schemas.microsoft.com/office/powerpoint/2010/main\" type=\"blank\">\n"
    "   <p:cSld name=\"Blank\">\n"
    "      <p:spTree>\n"
    "         <p:nvGrpSpPr>\n"
    "            <p:cNvPr id=\"48\" name=\"Shape 48\"/>\n"
    "            <p:cNvGrpSpPr/>\n"
    "            <p:nvPr/>\n"
    "         </p:nvGrpSpPr>\n"
    "         <p:grpSpPr>\n"
    "            <a:xfrm>\n"
    "               <a:off x=\"0\" y=\"0\"/>\n"
    "               <a:ext cx=\"0\" cy=\"0\"/>\n"
    "               <a:chOff x=\"0\" y=\"0\"/>\n"
    "               <a:chExt cx=\"0\" cy=\"0\"/>\n"
    "            </a:xfrm>\n"
    "         </p:grpSpPr>\n"
    "         </p:spTree>\n"
    "   </p:cSld>\n"
    "   <p:clrMapOvr>\n"
    "      <a:masterClrMapping/>\n"
    "   </p:clrMapOvr>\n"
    "</p:sldLayout>\n"
  };

  fprintf(layoutFile, "%s", rels);
}

static void printOpenMasterRelsFile(FILE* masterRelsFile)
{
  const char *rels =
  {
    "<?xml version = \"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
    "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
    "   <Relationship Id=\"rId11\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout\" Target=\"../slideLayouts/slideLayout1.xml\"/>\n"
    "   <Relationship Id=\"rId12\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme\" Target=\"../theme/theme1.xml\"/>\n"
  };

  fprintf(masterRelsFile, "%s", rels);
}

static void printCloseMasterRelsFile(FILE* masterRelsFile)
{
  const char *rels =
  {
    "</Relationships>\n"
  };

  fprintf(masterRelsFile, "%s", rels);
}

static void printOpenMasterFile(FILE* masterFile)
{
  const char *rels =
  {
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
    "<p:sldMaster xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
    "xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:mv=\"urn:schemas-microsoft-com:mac:vml\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" "
    "xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" xmlns:dgm=\"http://schemas.openxmlformats.org/drawingml/2006/diagram\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
    "xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:pvml=\"urn:schemas-microsoft-com:office:powerpoint\" xmlns:com=\"http://schemas.openxmlformats.org/drawingml/2006/compatibility\" "
    "xmlns:p14=\"http://schemas.microsoft.com/office/powerpoint/2010/main\">\n"
    "   <p:cSld>\n"
    "      <p:bg>\n"
    "         <p:bgPr>\n"
    "            <a:solidFill>\n"
    "               <a:schemeClr val=\"lt1\"/>\n"
    "            </a:solidFill>\n"
    "         </p:bgPr>\n"
    "      </p:bg>\n"
    "      <p:spTree>\n"
    "         <p:nvGrpSpPr>\n"
    "            <p:cNvPr id=\"5\" name=\"Shape 5\"/>\n"
    "            <p:cNvGrpSpPr/>\n"
    "            <p:nvPr/>\n"
    "         </p:nvGrpSpPr>\n"
    "         <p:grpSpPr>\n"
    "            <a:xfrm>\n"
    "               <a:off x=\"0\" y=\"0\"/>\n"
    "               <a:ext cx=\"0\" cy=\"0\"/>\n"
    "               <a:chOff x=\"0\" y=\"0\"/>\n"
    "               <a:chExt cx=\"0\" cy=\"0\"/>\n"
    "            </a:xfrm>\n"
    "         </p:grpSpPr>\n"
  };

  fprintf(masterFile, "%s", rels);
}

static void printCloseMasterFile(FILE* masterFile)
{
  const char *rels =
  {
    "      </p:spTree>\n"
    "   </p:cSld>\n"
    "   <p:clrMap accent1=\"accent4\" accent2=\"accent2\" accent3=\"accent3\" accent4=\"accent4\" accent5=\"accent5\" accent6=\"accent6\" bg1=\"lt1\" bg2=\"dk2\" tx1=\"dk1\" tx2=\"lt2\" folHlink=\"folHlink\" "
    "hlink=\"hlink\"/>\n"
    "   <p:sldLayoutIdLst>\n"
    "      <p:sldLayoutId id=\"2147483658\" r:id=\"rId11\"/>\n"
    "   </p:sldLayoutIdLst>\n"
    "   <p:hf dt=\"0\" ftr=\"0\" hdr=\"0\" sldNum=\"0\"/>\n"
    "</p:sldMaster>\n"
  };

  fprintf(masterFile, "%s", rels);
}

static void printThemeFile(FILE* themeFile)
{
  const char *rels =
  {
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
    "<a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" name=\"simple-light-2\">\n"
    "   <a:themeElements>\n"
    "      <a:clrScheme name=\"Simple Light\">\n"
    "         <a:dk1>\n"
    "            <a:srgbClr val=\"000000\"/>\n"
    "         </a:dk1>\n"
    "         <a:lt1>\n"
    "            <a:srgbClr val=\"FFFFFF\"/>\n"
    "         </a:lt1>\n"
    "         <a:dk2>\n"
    "            <a:srgbClr val=\"595959\"/>\n"
    "         </a:dk2>\n"
    "         <a:lt2>\n"
    "            <a:srgbClr val=\"EEEEEE\"/>\n"
    "         </a:lt2>\n"
    "         <a:accent1>\n"
    "            <a:srgbClr val=\"FFAB40\"/>\n"
    "         </a:accent1>\n"
    "         <a:accent2>\n"
    "            <a:srgbClr val=\"212121\"/>\n"
    "         </a:accent2>\n"
    "         <a:accent3>\n"
    "            <a:srgbClr val=\"78909C\"/>\n"
    "         </a:accent3>\n"
    "         <a:accent4>\n"
    "            <a:srgbClr val=\"FFAB40\"/>\n"
    "         </a:accent4>\n"
    "         <a:accent5>\n"
    "            <a:srgbClr val=\"0097A7\"/>\n"
    "         </a:accent5>\n"
    "         <a:accent6>\n"
    "            <a:srgbClr val=\"EEFF41\"/>\n"
    "         </a:accent6>\n"
    "         <a:hlink>\n"
    "            <a:srgbClr val=\"0097A7\"/>\n"
    "         </a:hlink>\n"
    "         <a:folHlink>\n"
    "            <a:srgbClr val=\"0097A7\"/>\n"
    "         </a:folHlink>\n"
    "      </a:clrScheme>\n"
    "      <a:fontScheme name=\"Office\">\n"
    "         <a:majorFont>\n"
    "            <a:latin typeface=\"Arial\"/>\n"
    "               <a:ea typeface=\"\"/>\n"
    "               <a:cs typeface=\"\"/>\n"
    "               <a:font script=\"Arab\" typeface=\"Times New Roman\"/>\n"
    "               <a:font script=\"Hebr\" typeface=\"Times New Roman\"/>\n"
    "               <a:font script=\"Thai\" typeface=\"Angsana New\"/>\n"
    "               <a:font script=\"Ethi\" typeface=\"Nyala\"/>\n"
    "               <a:font script=\"Beng\" typeface=\"Vrinda\"/>\n"
    "               <a:font script=\"Gujr\" typeface=\"Shruti\"/>\n"
    "               <a:font script=\"Khmr\" typeface=\"MoolBoran\"/>\n"
    "               <a:font script=\"Knda\" typeface=\"Tunga\"/>\n"
    "               <a:font script=\"Guru\" typeface=\"Raavi\"/>\n"
    "               <a:font script=\"Cans\" typeface=\"Euphemia\"/>\n"
    "               <a:font script=\"Cher\" typeface=\"Plantagenet Cherokee\"/>\n"
    "               <a:font script=\"Yiii\" typeface=\"Microsoft Yi Baiti\"/>\n"
    "               <a:font script=\"Tibt\" typeface=\"Microsoft Himalaya\"/>\n"
    "               <a:font script=\"Thaa\" typeface=\"MV Boli\"/>\n"
    "               <a:font script=\"Deva\" typeface=\"Mangal\"/>\n"
    "               <a:font script=\"Telu\" typeface=\"Gautami\"/>\n"
    "               <a:font script=\"Taml\" typeface=\"Latha\"/>\n"
    "               <a:font script=\"Syrc\" typeface=\"Estrangelo Edessa\"/>\n"
    "               <a:font script=\"Orya\" typeface=\"Kalinga\"/>\n"
    "               <a:font script=\"Mlym\" typeface=\"Kartika\"/>\n"
    "               <a:font script=\"Laoo\" typeface=\"DokChampa\"/>\n"
    "               <a:font script=\"Sinh\" typeface=\"Iskoola Pota\"/>\n"
    "               <a:font script=\"Mong\" typeface=\"Mongolian Baiti\"/>\n"
    "               <a:font script=\"Viet\" typeface=\"Times New Roman\"/>\n"
    "               <a:font script=\"Uigh\" typeface=\"Microsoft Uighur\"/>\n"
    "               <a:font script=\"Geor\" typeface=\"Sylfaen\"/>\n"
    "         </a:majorFont>\n"
    "         <a:minorFont>\n"
    "            <a:latin typeface=\"Arial\"/>\n"
    "            <a:ea typeface=\"\"/>\n"
    "            <a:cs typeface=\"\"/>\n"
    "            <a:font script=\"Arab\" typeface=\"Arial\"/>\n"
    "            <a:font script=\"Hebr\" typeface=\"Arial\"/>\n"
    "            <a:font script=\"Thai\" typeface=\"Cordia New\"/>\n"
    "            <a:font script=\"Ethi\" typeface=\"Nyala\"/>\n"
    "            <a:font script=\"Beng\" typeface=\"Vrinda\"/>\n"
    "            <a:font script=\"Gujr\" typeface=\"Shruti\"/>\n"
    "            <a:font script=\"Khmr\" typeface=\"DaunPenh\"/>\n"
    "            <a:font script=\"Knda\" typeface=\"Tunga\"/>\n"
    "            <a:font script=\"Guru\" typeface=\"Raavi\"/>\n"
    "            <a:font script=\"Cans\" typeface=\"Euphemia\"/>\n"
    "            <a:font script=\"Cher\" typeface=\"Plantagenet Cherokee\"/>\n"
    "            <a:font script=\"Yiii\" typeface=\"Microsoft Yi Baiti\"/>\n"
    "            <a:font script=\"Tibt\" typeface=\"Microsoft Himalaya\"/>\n"
    "            <a:font script=\"Thaa\" typeface=\"MV Boli\"/>\n"
    "            <a:font script=\"Deva\" typeface=\"Mangal\"/>\n"
    "            <a:font script=\"Telu\" typeface=\"Gautami\"/>\n"
    "            <a:font script=\"Taml\" typeface=\"Latha\"/>\n"
    "            <a:font script=\"Syrc\" typeface=\"Estrangelo Edessa\"/>\n"
    "            <a:font script=\"Orya\" typeface=\"Kalinga\"/>\n"
    "            <a:font script=\"Mlym\" typeface=\"Kartika\"/>\n"
    "            <a:font script=\"Laoo\" typeface=\"DokChampa\"/>\n"
    "            <a:font script=\"Sinh\" typeface=\"Iskoola Pota\"/>\n"
    "            <a:font script=\"Mong\" typeface=\"Mongolian Baiti\"/>\n"
    "            <a:font script=\"Viet\" typeface=\"Arial\"/>\n"
    "            <a:font script=\"Uigh\" typeface=\"Microsoft Uighur\"/>\n"
    "            <a:font script=\"Geor\" typeface=\"Sylfaen\"/>\n"
    "         </a:minorFont>\n"
    "      </a:fontScheme>\n"
    "      <a:fmtScheme name=\"Offce\">\n"
    "         <a:fillStyleLst>\n"
    "            <a:solidFill>\n"
    "               <a:schemeClr val=\"phClr\"/>\n"
    "            </a:solidFill>\n"
    "            <a:gradFill rotWithShape=\"1\">\n"
    "               <a:gsLst>\n"
    "                  <a:gs pos=\"0\">\n"
    "                     <a:schemeClr val=\"phClr\">\n"
    "                        <a:tint val=\"50000\"/>\n"
    "                        <a:satMod val=\"300000\"/>\n"
    "                     </a:schemeClr>\n"
    "                  </a:gs>\n"
    "                  <a:gs pos=\"35000\">\n"
    "                     <a:schemeClr val=\"phClr\">\n"
    "                        <a:tint val=\"37000\"/>\n"
    "                        <a:satMod val=\"300000\"/>\n"
    "                     </a:schemeClr>\n"
    "                  </a:gs>\n"
    "                  <a:gs pos=\"100000\">\n"
    "                     <a:schemeClr val=\"phClr\">\n"
    "                        <a:tint val=\"15000\"/>\n"
    "                        <a:satMod val=\"350000\"/>\n"
    "                     </a:schemeClr>\n"
    "                  </a:gs>\n"
    "               </a:gsLst>\n"
    "               <a:lin ang=\"16200000\" scaled=\"1\"/>\n"
    "            </a:gradFill>\n"
    "            <a:gradFill rotWithShape=\"1\">\n"
    "               <a:gsLst>\n"
    "                  <a:gs pos=\"0\">\n"
    "                     <a:schemeClr val=\"phClr\">\n"
    "                        <a:tint val=\"100000\"/>\n"
    "                        <a:shade val=\"100000\"/>\n"
    "                        <a:satMod val=\"130000\"/>\n"
    "                     </a:schemeClr>\n"
    "                  </a:gs>\n"
    "                  <a:gs pos=\"100000\">\n"
    "                     <a:schemeClr val=\"phClr\">\n"
    "                        <a:tint val=\"50000\"/>\n"
    "                        <a:shade val=\"100000\"/>\n"
    "                        <a:satMod val=\"350000\"/>\n"
    "                     </a:schemeClr>\n"
    "                  </a:gs>\n"
    "               </a:gsLst>\n"
    "               <a:lin ang=\"16200000\" scaled=\"0\"/>\n"
    "            </a:gradFill>\n"
    "         </a:fillStyleLst>\n"
    "         <a:lnStyleLst>\n"
    "            <a:ln w=\"9525\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\">\n"
    "               <a:solidFill>\n"
    "                  <a:schemeClr val=\"phClr\">\n"
    "                     <a:shade val=\"95000\"/>\n"
    "                     <a:satMod val=\"105000\"/>\n"
    "                  </a:schemeClr>\n"
    "               </a:solidFill>\n"
    "               <a:prstDash val=\"solid\"/>\n"
    "            </a:ln>\n"
    "            <a:ln w=\"25400\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\">\n"
    "               <a:solidFill>\n"
    "                  <a:schemeClr val=\"phClr\"/>\n"
    "               </a:solidFill>\n"
    "               <a:prstDash val=\"solid\"/>\n"
    "            </a:ln>\n"
    "            <a:ln w=\"38100\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\">\n"
    "               <a:solidFill>\n"
    "                  <a:schemeClr val=\"phClr\"/>\n"
    "               </a:solidFill>\n"
    "               <a:prstDash val=\"solid\"/>\n"
    "            </a:ln>\n"
    "         </a:lnStyleLst>\n"
    "         <a:effectStyleLst>\n"
    "            <a:effectStyle>\n"
    "               <a:effectLst>\n"
    "                  <a:outerShdw blurRad=\"40000\" dist=\"20000\" dir=\"5400000\" rotWithShape=\"0\">\n"
    "                     <a:srgbClr val=\"000000\">\n"
    "                        <a:alpha val=\"38000\"/>\n"
    "                     </a:srgbClr>\n"
    "                  </a:outerShdw>\n"
    "               </a:effectLst>\n"
    "            </a:effectStyle>\n"
    "            <a:effectStyle>\n"
    "               <a:effectLst>\n"
    "                  <a:outerShdw blurRad=\"40000\" dist=\"23000\" dir=\"5400000\" rotWithShape=\"0\">\n"
    "                     <a:srgbClr val=\"000000\">\n"
    "                        <a:alpha val=\"35000\"/>\n"
    "                     </a:srgbClr>\n"
    "                  </a:outerShdw>\n"
    "               </a:effectLst>\n"
    "            </a:effectStyle>\n"
    "            <a:effectStyle>\n"
    "               <a:effectLst>\n"
    "                  <a:outerShdw blurRad=\"40000\" dist=\"23000\" dir=\"5400000\" rotWithShape=\"0\">\n"
    "                     <a:srgbClr val=\"000000\">\n"
    "                        <a:alpha val=\"35000\"/>\n"
    "                     </a:srgbClr>\n"
    "                  </a:outerShdw>\n"
    "               </a:effectLst>\n"
    "               <a:scene3d>\n"
    "                  <a:camera prst=\"orthographicFront\">\n"
    "                     <a:rot lat=\"0\" lon=\"0\" rev=\"0\"/>\n"
    "                  </a:camera>\n"
    "                  <a:lightRig rig=\"threePt\" dir=\"t\">\n"
    "                     <a:rot lat=\"0\" lon=\"0\" rev=\"1200000\"/>\n"
    "                  </a:lightRig>\n"
    "               </a:scene3d>\n"
    "               <a:sp3d>\n"
    "               <a:bevelT w=\"63500\" h=\"25400\"/>\n"
    "               </a:sp3d>\n"
    "            </a:effectStyle>\n"
    "         </a:effectStyleLst>\n"
    "         <a:bgFillStyleLst>\n"
    "            <a:solidFill>\n"
    "               <a:schemeClr val=\"phClr\"/>\n"
    "            </a:solidFill>\n"
    "            <a:gradFill rotWithShape=\"1\">\n"
    "               <a:gsLst>\n"
    "                  <a:gs pos=\"0\">\n"
    "                     <a:schemeClr val=\"phClr\">\n"
    "                        <a:tint val=\"40000\"/>\n"
    "                        <a:satMod val=\"350000\"/>\n"
    "                     </a:schemeClr>\n"
    "                  </a:gs>\n"
    "                  <a:gs pos=\"40000\">\n"
    "                     <a:schemeClr val=\"phClr\">\n"
    "                        <a:tint val=\"45000\"/>\n"
    "                        <a:shade val=\"99000\"/>\n"
    "                        <a:satMod val=\"350000\"/>\n"
    "                     </a:schemeClr>\n"
    "                  </a:gs>\n"
    "                  <a:gs pos=\"100000\">\n"
    "                     <a:schemeClr val=\"phClr\">\n"
    "                        <a:shade val=\"20000\"/>\n"
    "                        <a:satMod val=\"255000\"/>\n"
    "                     </a:schemeClr>\n"
    "                  </a:gs>\n"
    "               </a:gsLst>\n"
    "               <a:path path=\"circle\">\n"
    "                  <a:fillToRect l=\"50000\" t=\"-80000\" r=\"50000\" b=\"180000\"/>\n"
    "               </a:path>\n"
    "            </a:gradFill>\n"
    "            <a:gradFill rotWithShape=\"1\">\n"
    "               <a:gsLst>\n"
    "                  <a:gs pos=\"0\">\n"
    "                     <a:schemeClr val=\"phClr\">\n"
    "                        <a:tint val=\"80000\"/>\n"
    "                        <a:satMod val=\"300000\"/>\n"
    "                     </a:schemeClr>\n"
    "                  </a:gs>\n"
    "                  <a:gs pos=\"100000\">\n"
    "                     <a:schemeClr val=\"phClr\">\n"
    "                        <a:shade val=\"30000\"/>\n"
    "                        <a:satMod val=\"200000\"/>\n"
    "                     </a:schemeClr>\n"
    "                  </a:gs>\n"
    "               </a:gsLst>\n"
    "               <a:path path=\"circle\">\n"
    "                  <a:fillToRect l=\"50000\" t=\"50000\" r=\"50000\" b=\"50000\"/>\n"
    "               </a:path>\n"
    "            </a:gradFill>\n"
    "         </a:bgFillStyleLst>\n"
    "      </a:fmtScheme>\n"
    "   </a:themeElements>\n"
    "</a:theme>\n"
  };

  fprintf(themeFile, "%s", rels);
}

static void printPresentation(FILE* presentationFile, int nSlides, int height, int width)
{
  int i;

  const char *presentationPrefix =
  {
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
    "<p:presentation xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:mv=\"urn:schemas-microsoft-com:mac:vml\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" xmlns:dgm=\"http://schemas.openxmlformats.org/drawingml/2006/diagram\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:pvml=\"urn:schemas-microsoft-com:office:powerpoint\" xmlns:com=\"http://schemas.openxmlformats.org/drawingml/2006/compatibility\" xmlns:p14=\"http://schemas.microsoft.com/office/powerpoint/2010/main\" autoCompressPictures=\"0\" strictFirstAndLastChars=\"0\" saveSubsetFonts=\"1\">\n"
    "   <p:sldMasterIdLst>\n"
    "      <p:sldMasterId id=\"2147483659\" r:id=\"rId3\"/>\n"
    "   </p:sldMasterIdLst>\n"
    "   <p:sldIdLst>\n"
  };

  const char *slides =
  {
    "      <p:sldId id=\"%d\" r:id=\"rId%d\"/>\n"
  };

  const char *presentationSuffix =
  {
    "   </p:sldIdLst>\n"
    "   <p:sldSz cx=\"%d\" cy=\"%d\"/>\n"
    "   <p:notesSz cx=\"%d\" cy=\"%d\"/>\n"
    "</p:presentation>\n"
  };

  fprintf(presentationFile, "%s", presentationPrefix);
  for (i = 0; i < nSlides; i++)
    fprintf(presentationFile, slides, i + 256, i + 4);
  fprintf(presentationFile, presentationSuffix, width, height, height, width);  /* Notes size is swapped */
}

static void printContentTypes(FILE* ctFile, int nSlides)
{
  int i;

  const char *contentTypesPrefix =
  {
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
    "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">\n"
    "   <Default Extension=\"png\" ContentType=\"image/png\"/>\n"
    "   <Default Extension=\"jpeg\" ContentType=\"image/jpeg\"/>\n"
    "   <Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>\n"
    "   <Default Extension=\"xml\" ContentType=\"application/xml\"/>\n"
    "   <Default Extension=\"jpg\" ContentType=\"application/octet-stream\"/>\n"
    "   <Override PartName=\"/ppt/slideMasters/slideMaster1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml\"/>\n"
    "   <Override PartName=\"/ppt/slideLayouts/slideLayout1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml\"/>\n"
  };

  const char *slide =
  {
    "   <Override PartName=\"/ppt/slides/slide%d.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slide+xml\"/>\n"
  };

  const char *contentTypesSuffix =
  {
    "   <Override PartName=\"/ppt/presentation.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml\"/>\n"
    "   <Override PartName=\"/ppt/presProps.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.presProps+xml\"/>\n"
    "   <Override PartName=\"/ppt/theme/theme1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.theme+xml\"/>\n"
    "</Types>\n"
  };

  fprintf(ctFile, "%s", contentTypesPrefix);
  for (i = 0; i < nSlides; i++)
    fprintf(ctFile, slide, i + 1);
  fprintf(ctFile, "%s", contentTypesSuffix);
}

static void printPptRelsFile(FILE* pptRelsFile, int nSlides)
{
  int i;

  const char *relsPrefix =
  {
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
    "<Relationships xmlns = \"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
    "   <Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme\" Target=\"theme/theme1.xml\"/>\n"
    "   <Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/presProps\" Target=\"presProps.xml\"/>\n"
    "   <Relationship Id=\"rId3\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster\" Target=\"slideMasters/slideMaster1.xml\"/>\n"
  };

  const char *slides =
  {
    "   <Relationship Id=\"rId%d\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide\" Target=\"slides/slide%d.xml\"/>\n"
  };

  const char *relsSuffix =
  {
    "</Relationships>\n"
  };

  fprintf(pptRelsFile, "%s", relsPrefix);
  for (i = 0; i < nSlides; i++)
    fprintf(pptRelsFile, slides, i + 4, i + 1);
  fprintf(pptRelsFile, "%s", relsSuffix);
}

static void printCloseSlide(FILE* slideFile)
{
  const char *suffix =
  {
    "      </p:spTree>\n"
    "   </p:cSld>\n"
    "</p:sld>\n"
  };

  fprintf(slideFile, "%s", suffix);
}

static void printCloseSlideRels(FILE* slideRelsFile)
{
  const char *rels =
  {
    "</Relationships>\n"
  };

  fprintf(slideRelsFile, "%s", rels);
}

static void printSlideRels(pptxPresentation *presentation)
{
  const char *rels =
  {
    "   <Relationship Id=\"rId%d\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" Target=\"../media/media/image%d.png\"/>\n"
  };

  fprintf(presentation->slideRelsFile, rels, presentation->imageId, presentation->mediaNum);
}

/**************************************  FILE OPEN/CLOSE   ******************************************************/


int pptxOpenSlide(pptxPresentation *presentation)
{
  char filename[10240];

  presentation->slideNum++;

  sprintf(filename, "%s/"PPTX_SLIDE_FILE, presentation->baseDir, presentation->slideNum);

  presentation->slideFile = fopen(filename, "w");
  if (!presentation->slideFile)
    return 0;

  sprintf(filename, "%s/"PPTX_SLIDE_RELS_FILE, presentation->baseDir, presentation->slideNum);

  presentation->slideRelsFile = fopen(filename, "w");
  if (!presentation->slideRelsFile)
    return 0;

  printOpenSlide(presentation->slideFile, presentation->objectNum);

  printOpenSlideRels(presentation->slideRelsFile);

  presentation->objectNum++;
  return 1;
}

void pptxCloseSlide(pptxPresentation *presentation)
{
  fflush(presentation->slideFile);

  printCloseSlide(presentation->slideFile);

  printCloseSlideRels(presentation->slideRelsFile);

  fclose(presentation->slideFile);

  fclose(presentation->slideRelsFile);
}

static FILE* openFile(pptxPresentation *presentation, const char* subfile)
{
  char filename[10240];

  sprintf(filename, "%s/%s", presentation->baseDir, subfile);

  return fopen(filename, "w");
}

void pptxWritePresProps(pptxPresentation *presentation)
{
  FILE* presPropsFile = openFile(presentation, PPTX_PRES_PROPS_FILE);
  if (!presPropsFile)
    return;

  printPresProps(presPropsFile);

  fclose(presPropsFile);
}

void pptxWriteRels(pptxPresentation *presentation)
{
  FILE *relsFile = openFile(presentation, PPTX_RELS_FILE);
  if (!relsFile)
    return;

  printRels(relsFile);

  fclose(relsFile);
}

void pptxWriteLayoutRels(pptxPresentation *presentation)
{
  FILE *pptLayoutRels = openFile(presentation, PPTX_SLIDE_LAYOUT_RELS_FILE);
  if (!pptLayoutRels)
    return;

  printLayoutRelsFile(pptLayoutRels);

  fclose(pptLayoutRels);
}

void pptxWriteLayout(pptxPresentation *presentation)
{
  FILE *layoutRels = openFile(presentation, PPTX_SLIDE_LAYOUT_FILE);
  if (!layoutRels)
    return;

  printLayoutFile(layoutRels);

  fclose(layoutRels);
}

void pptxOpenWriteMasterRels(pptxPresentation *presentation)
{
  presentation->masterSlideRelsFile = openFile(presentation, PPTX_SLIDE_MASTER_RELS);
  if (!presentation->masterSlideRelsFile)
    return;

  printOpenMasterRelsFile(presentation->masterSlideRelsFile);
}

void pptxCloseWriteMasterRels(pptxPresentation *presentation)
{
  if (!presentation->masterSlideRelsFile)
    return;

  printCloseMasterRelsFile(presentation->masterSlideRelsFile);

  fclose(presentation->masterSlideRelsFile);
}

void pptxOpenWriteMaster(pptxPresentation *presentation)
{
  presentation->masterSlideFile = openFile(presentation, PPTX_SLIDE_MASTER_FILE);
  if (!presentation->masterSlideFile)
    return;

  printOpenMasterFile(presentation->masterSlideFile);
}

void pptxCloseWriteMaster(pptxPresentation *presentation)
{
  if (!presentation->masterSlideFile)
    return;

  printCloseMasterFile(presentation->masterSlideFile);

  fclose(presentation->masterSlideFile);
}

void pptxWriteTheme(pptxPresentation *presentation)
{
  FILE *theme = openFile(presentation, PPTX_THEME_FILE);
  if (!theme)
    return;

  printThemeFile(theme);

  fclose(theme);
}

void pptxWritePresentation(pptxPresentation *presentation)
{
  presentation->presentationFile = openFile(presentation, PPTX_PRESENTATION_FILE);
  if (!presentation->presentationFile)
    return;

  printPresentation(presentation->presentationFile, presentation->slideNum, presentation->slideHeight, presentation->slideWidth);

  fclose(presentation->presentationFile);
}

void pptxWriteContentTypes(pptxPresentation *presentation)
{
  FILE *ctFile = openFile(presentation, PPTX_CONTENT_TYPES_FILE);
  if (!ctFile)
    return;

  printContentTypes(ctFile, presentation->slideNum);

  fclose(ctFile);
}

void pptxWritePptRels(pptxPresentation *presentation)
{
  FILE *pptRels = openFile(presentation, PPTX_PRESENTATION_RELS);
  if (!pptRels)
    return;

  printPptRelsFile(pptRels, presentation->slideNum);

  fclose(pptRels);
}


/**************************************  PRIMITIVES   ******************************************************/


void pptxBeginPath(pptxPresentation *presentation, int xmin, int ymin, int w, int h)
{
  const char *linePrefix =
  {/*012345678| - primitives ident (9) */
    "         <p:sp>\n"                /* to be closed (pptxEndLine or pptxEndFill) */
    "            <p:nvSpPr>\n"
    "               <p:cNvPr id=\"%d\" name=\"Shape %d\"/>\n"
    "               <p:cNvSpPr/>\n"
    "               <p:nvPr/>\n"
    "            </p:nvSpPr>\n"
    "            <p:spPr>\n"           /* to be closed (pptxEndLine or pptxEndFill) */
    "               <a:xfrm>\n"
    "                  <a:off x=\"%d\" y=\"%d\"/>\n"
    "                  <a:ext cx=\"%d\" cy=\"%d\"/>\n"
    "               </a:xfrm>\n"
    "               <a:custGeom>\n"    /* to be closed (pptxClosePath) */
    "                  <a:pathLst>\n"  /* to be closed (pptxClosePath) */
    "                     <a:path extrusionOk=\"0\" w=\"%d\" h=\"%d\">\n"  /* to be closed (pptxClosePath) */
  };

  fprintf(presentation->slideFile, linePrefix, presentation->objectNum, presentation->objectNum, xmin*presentation->slide_xfactor, ymin*presentation->slide_yfactor,
          w*presentation->slide_xfactor, h*presentation->slide_yfactor, w*presentation->slide_xfactor, h*presentation->slide_yfactor);
}

void pptxMoveTo(pptxPresentation *presentation, int x, int y)
{
  const char *line =
  {/*012345678901234567890123| - path primitives ident (24) */
    "                        <a:moveTo>\n"
    "                           <a:pt x=\"%d\" y=\"%d\"/>\n"
    "                        </a:moveTo>\n"
  };

  fprintf(presentation->slideFile, line, x*presentation->slide_xfactor, y*presentation->slide_yfactor);
}

void pptxLineTo(pptxPresentation *presentation, int x, int y)
{
  const char *line =
  {/*012345678901234567890123| - path primitives ident (24) */
    "                        <a:lnTo>\n"
    "                           <a:pt x=\"%d\" y=\"%d\"/>\n"
    "                        </a:lnTo>\n"
  };

  fprintf(presentation->slideFile, line, x*presentation->slide_xfactor, y*presentation->slide_yfactor);
}

void pptxArcTo(pptxPresentation *presentation, int h, int w, double stAng, double swAng)
{
  const char *arc =
  {/*012345678901234567890123| - path primitives ident (24) */
    "                        <a:arcTo hR=\"%d\" wR=\"%d\" stAng=\"%d\" swAng=\"%d\"/>\n"
  };

  fprintf(presentation->slideFile, arc, h*presentation->slide_yfactor, w*presentation->slide_xfactor, to_angle(stAng), to_angle(swAng));
}

void pptxBezierLineTo(pptxPresentation *presentation, int c1x, int c1y, int c2x, int c2y, int c3x, int c3y)
{
  const char *line =
  {/*012345678901234567890123| - path primitives ident (24) */
    "                        <a:cubicBezTo>\n"
    "                           <a:pt x=\"%d\" y=\"%d\"/>\n"
    "                           <a:pt x=\"%d\" y=\"%d\"/>\n"
    "                           <a:pt x=\"%d\" y=\"%d\"/>\n"
    "                        </a:cubicBezTo>\n"
  };

  fprintf(presentation->slideFile, line, c1x*presentation->slide_xfactor, c1y*presentation->slide_yfactor, c2x*presentation->slide_xfactor, c2y*presentation->slide_yfactor,
          c3x*presentation->slide_xfactor, c3y*presentation->slide_yfactor);
}

void pptxClosePath(pptxPresentation *presentation)
{
  const char *lineSuffix =
  {/*012345678901234567890| - closing path ident (21) */
    "                     </a:path>\n"
    "                  </a:pathLst>\n"
    "               </a:custGeom>\n"
  };

  fprintf(presentation->slideFile, "%s", lineSuffix);
}

void pptxNoFill(pptxPresentation *presentation)
{
  const char *noFill =
  {/*012345678901234| - path style ident (15) */
    "               <a:noFill/>\n"
  };

  fprintf(presentation->slideFile, "%s", noFill);
}

void pptxSolidFill(pptxPresentation *presentation, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
  int alphaPct = (int)(((double)alpha / 255.) * 100 * 1000);

  const char *fill =
  {/*012345678901234| - path style ident (15) */
    "               <a:solidFill>\n"
    "                  <a:srgbClr val=\"%02X%02X%02X\">\n"
    "                     <a:alpha val=\"%d\"/>\n"
    "                  </a:srgbClr>\n"
    "               </a:solidFill>\n"
  };

  fprintf(presentation->slideFile, fill, red, green, blue, alphaPct);
}

void pptxHatchLine(pptxPresentation *presentation, const char* style, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha,
                   unsigned char bRed, unsigned char bGreen, unsigned char bBlue, unsigned char bAlpha)
{
  int alphaPct = (int)(((double)alpha / 255.) * 100 * 1000);

  const char *patt =
  {/*012345678901234| - path style ident (15) */
    "               <a:pattFill prst=\"%s\">\n"
    "                  <a:fgClr>\n"
    "                     <a:srgbClr val=\"%02X%02X%02X\">\n"
    "                        <a:alpha val=\"%d\"/>\n"
    "                     </a:srgbClr>\n"
    "                  </a:fgClr>\n"
    "                  <a:bgClr>\n"
    "                     <a:srgbClr val=\"%02X%02X%02X\">\n"
    "                        <a:alpha val=\"%d\"/>\n"
    "                     </a:srgbClr>\n"
    "                  </a:bgClr>\n"
    "               </a:pattFill>\n"
  };

  fprintf(presentation->slideFile, patt, style, red, green, blue, alphaPct, bRed, bGreen, bBlue, bAlpha);
}

void pptxPattern(pptxPresentation *presentation, const unsigned char* rgba_data, int width, int height)
{
  const char *img =
  {/*012345678901234| - path style ident (15) */
    "               <a:blipFill dpi=\"0\" rotWithShape=\"1\">\n"
    "                  <a:blip r:embed=\"rId%d\"/>\n"
    "                  <a:srcRect/>\n"
    "                  <a:tile tx=\"0\" ty=\"0\" sx=\"100000\" sy=\"100000\" flip=\"none\" algn=\"tl\"/>\n"
    "               </a:blipFill>\n"
  };
  char filename[10240];

  sprintf(filename, "%s/"PPTX_IMAGE_FILE, presentation->baseDir, presentation->mediaNum);

  lodepng_encode_file(filename, rgba_data, width, height, LCT_RGBA, 8);

  fprintf(presentation->slideFile, img, presentation->imageId);

  printSlideRels(presentation);

  presentation->mediaNum++;
  presentation->imageId++;
}

void pptxStipple(pptxPresentation *presentation, const unsigned char *rgba_data, int width, int height)
{
  const char *img =
  {/*012345678901234| - path style ident (15) */
    "               <a:blipFill dpi=\"0\" rotWithShape=\"1\">\n"
    "                  <a:blip r:embed=\"rId%d\"/>\n"
    "                  <a:srcRect/>\n"
    "                  <a:tile tx=\"0\" ty=\"0\" sx=\"100000\" sy=\"100000\" flip=\"none\" algn=\"tl\"/>\n"
    "               </a:blipFill>\n"
  };
  char filename[10240];

  sprintf(filename, "%s/"PPTX_IMAGE_FILE, presentation->baseDir, presentation->mediaNum);

  lodepng_encode_file(filename, rgba_data, width, height, LCT_RGBA, 8);

  fprintf(presentation->slideFile, img, presentation->imageId);

  printSlideRels(presentation);

  presentation->mediaNum++;
  presentation->imageId++;
}

void pptxEndLine(pptxPresentation *presentation, int line_width, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha, const char* lineStyle, int nDashes, int *dashes)
{
  int i;
  int alphaPct = (int)(((double)alpha / 255.) * 100 * 1000);

  const char *lineEndPrefix =
  {/*012345678901234| - path style ident (15) */
    "               <a:ln cap=\"flat\" cmpd=\"sng\" w=\"%d\">\n"  /* to be closed (lineEndSuffix) */
    "                  <a:solidFill>\n"
    "                     <a:srgbClr val=\"%02X%02X%02X\">\n"
    "                        <a:alpha val=\"%d\"/>\n"
    "                     </a:srgbClr>\n"
    "                  </a:solidFill>\n"
  };

  const char *style =
  {/*012345678901234567| - line style ident (18) */
    "                  <a:prstDash val = \"%s\"/>\n"
  };

  const char *lineEndSuffix =
  {/*012345678| - primitives ident (9) */
    "                  <a:round/>\n"
    "                  <a:headEnd len=\"lg\" w=\"lg\" type=\"none\"/>\n"
    "                  <a:tailEnd len=\"lg\" w=\"lg\" type=\"none\"/>\n"
    "               </a:ln>\n"
    "            </p:spPr>\n"
    "         </p:sp>\n"
  };

  fprintf(presentation->slideFile, lineEndPrefix, line_width*presentation->slide_xfactor, red, green, blue, alphaPct);

  if (strcmp(lineStyle, "custom") != 0)
    fprintf(presentation->slideFile, style, lineStyle);
  else
  {                                   /*012345678901234567| - line style ident (18) */
    fprintf(presentation->slideFile, "%s", "                  <a:custDash>\n");
    for (i = 0; i < nDashes; i += 2)
      fprintf(presentation->slideFile, "                     <a:ds d=\"%d%%\" sp=\"%d%%\"/>\n", dashes[i], dashes[i + 1]);
    fprintf(presentation->slideFile, "%s", "                  </a:custDash>\n");
  }

  fprintf(presentation->slideFile, "%s", lineEndSuffix);

  presentation->objectNum++;
}

void pptxEndFill(pptxPresentation *presentation)
{
  const char *fillEnd =
  {/*012345678| - primitives ident (9) */
    "            </p:spPr>\n"
    "         </p:sp>\n"
  };

  fprintf(presentation->slideFile, "%s", fillEnd);

  presentation->objectNum++;
}

void pptxBeginSector(pptxPresentation *presentation, const char *geomType, int xmin, int ymin, int w, int h, double angle1, double angle2)
{
  const char *arc =
  {/*012345678| - primitives ident (9) */
    "         <p:sp>\n"                /* to be closed */
    "            <p:nvSpPr>\n"
    "               <p:cNvPr id=\"%d\" name=\"Shape %d\"/>\n"
    "               <p:cNvSpPr/>\n"
    "               <p:nvPr/>\n"
    "            </p:nvSpPr>\n"
    "            <p:spPr>\n"           /* to be closed */
    "               <a:xfrm>\n"
    "                  <a:off x=\"%d\" y=\"%d\"/>\n"
    "                  <a:ext cx=\"%d\" cy=\"%d\"/>\n"
    "               </a:xfrm>\n"
    "               <a:prstGeom prst=\"%s\">\n"
    "                  <a:avLst>\n"
    "                     <a:gd fmla=\"val %d\" name=\"adj1\"/>\n"
    "                     <a:gd fmla=\"val %d\" name=\"adj2\"/>\n"
    "                  </a:avLst>\n"
    "               </a:prstGeom>\n"
  };

  fprintf(presentation->slideFile, arc, presentation->objectNum, presentation->objectNum, xmin*presentation->slide_xfactor, ymin*presentation->slide_yfactor, w*presentation->slide_xfactor, h*presentation->slide_yfactor,
          geomType, to_angle(angle1), to_angle(angle2));
}

void pptxText(pptxPresentation *presentation, int xmin, int ymin, int w, int h, double rotAngle, int bold, int italic, int underline, int strikeout, int size, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha,
              const char* typeface, const char* text)
{
  int alphaPct = (int)(((double)alpha / 255.) * 100 * 1000);

  const char *textInit =
  {/*012345678| - primitives ident (9) */
    "         <p:sp>\n"
    "            <p:nvSpPr>\n"
    "               <p:cNvPr id=\"%d\" name=\"Shape %d\"/>\n"
    "               <p:cNvSpPr txBox=\"0\"/>\n"
    "               <p:nvPr/>\n"
    "            </p:nvSpPr>\n"
    "            <p:spPr>\n"
    "               <a:xfrm rot=\"%d\">\n"
    "                  <a:off x=\"%d\" y=\"%d\"/>\n"
    "                  <a:ext cx=\"%d\" cy=\"%d\"/>\n"
    "               </a:xfrm>\n"
    "               <a:prstGeom prst=\"rect\">\n"
    "                  <a:avLst/>\n"
    "               </a:prstGeom>\n"
    "               <a:noFill/>\n"
    "               <a:ln>\n"
    "                  <a:noFill/>\n"
    "               </a:ln>\n"
    "            </p:spPr>\n"
    "            <p:txBody>\n"
    "               <a:bodyPr wrap=\"none\" anchorCtr=\"1\" anchor=\"ctr\" bIns=\"0\" lIns=\"0\" rIns=\"0\" tIns=\"0\">\n"
    "                  <a:noAutofit/>\n"
    "               </a:bodyPr>\n"
    "               <a:lstStyle/>\n"
    "               <a:p>\n"
    "                  <a:pPr lvl=\"0\">\n"
    "                     <a:spcBef>\n"
    "                        <a:spcPts val=\"0\"/>\n"
    "                     </a:spcBef>\n"
    "                     <a:buNone/>\n"
    "                  </a:pPr>\n"
    "                  <a:r>\n"
    "                     <a:rPr b=\"%d\" i=\"%d\" strike=\"%s\" lang=\"en\" u=\"%s\" sz=\"%d\">\n"
    "                        <a:solidFill>\n"
    "                           <a:srgbClr val=\"%02X%02X%02X\">\n"
    "                              <a:alpha val=\"%d\"/>\n"
    "                           </a:srgbClr>\n"
    "                        </a:solidFill>\n"
    "                        <a:latin typeface=\"%s\"/>\n"
    "                        <a:ea typeface=\"%s\"/>\n"
    "                        <a:cs typeface=\"%s\"/>\n"
    "                        <a:sym typeface=\"%s\"/>\n"
    "                     </a:rPr>\n"
    "                     <a:t>%s</a:t>\n"
    "                  </a:r>\n"
    "               </a:p>\n"
    "            </p:txBody>\n"
    "         </p:sp>\n"
  };

  fprintf(presentation->slideFile, textInit, presentation->objectNum, presentation->objectNum, to_angle(rotAngle), xmin*presentation->slide_xfactor, ymin*presentation->slide_yfactor, w*presentation->slide_xfactor,
          h*presentation->slide_yfactor, bold, italic, strikeout ? "sngStrike" : "noStrike", underline ? "sng" : "none", (size * 2 / 3) * 100,
          red, green, blue, alphaPct, typeface, typeface, typeface, typeface, text);

  presentation->objectNum++;
}

void pptxPixel(pptxPresentation *presentation, int x, int y, int width, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
  int alphaPct = (int)(((double)alpha / 255.) * 100 * 1000);
  int w = width*presentation->slide_xfactor;

  const char *pixel =
  {/*012345678| - primitives ident (9) */
    "         <p:sp>\n"
    "            <p:nvSpPr>\n"
    "               <p:cNvPr id=\"%d\" name=\"Shape %d\"/>\n"
    "               <p:cNvSpPr/>\n"
    "               <p:nvPr/>\n"
    "            </p:nvSpPr>\n"
    "         <p:spPr>\n"
    "            <a:xfrm>\n"
    "               <a:off x=\"%d\" y=\"%d\"/>\n"
    "               <a:ext cx=\"%d\" cy=\"%d\"/>\n"
    "            </a:xfrm>\n"
    "            <a:custGeom>\n"
    "               <a:pathLst>\n"
    "                  <a:path extrusionOk=\"0\" w=\"%d\" h=\"%d\">\n"
    "                     <a:moveTo>\n"
    "                        <a:pt x=\"0\" y=\"0\"/>\n"
    "                     </a:moveTo>\n"
    "                     <a:lnTo>\n"
    "                        <a:pt x=\"%d\" y=\"0\"/>\n"
    "                     </a:lnTo>\n"
    "                     <a:lnTo>\n"
    "                        <a:pt x=\"%d\" y=\"%d\"/>\n"
    "                     </a:lnTo>\n"
    "                     <a:lnTo>\n"
    "                        <a:pt x=\"0\" y=\"%d\"/>\n"
    "                     </a:lnTo>\n"
    "                     <a:lnTo>\n"
    "                        <a:pt x=\"0\" y=\"0\"/>\n"
    "                     </a:lnTo>\n"
    "                  </a:path>\n"
    "               </a:pathLst>\n"
    "            </a:custGeom>\n"
    "            <a:noFill/>\n"
    "               <a:ln cap=\"flat\" cmpd=\"sng\" w=\"%d\">\n"
    "                  <a:solidFill>\n"
    "                     <a:srgbClr val=\"%02X%02X%02X\">\n"
    "                        <a:alpha val=\"%d\"/>\n"
    "                     </a:srgbClr>\n"
    "                  </a:solidFill>\n"
    "                  <a:prstDash val=\"solid\"/>\n"
    "                  <a:round/>\n"
    "                  <a:headEnd len=\"lg\" w=\"lg\" type=\"none\"/>\n"
    "                  <a:tailEnd len=\"lg\" w=\"lg\" type=\"none\"/>\n"
    "               </a:ln>\n"
    "            </p:spPr>\n"
    "         </p:sp>\n"
  };

  fprintf(presentation->slideFile, pixel, presentation->objectNum, presentation->objectNum, x*presentation->slide_xfactor, y*presentation->slide_yfactor,
          w, w, w, w, w, w, w, w, w, red, green, blue, alphaPct);

  presentation->objectNum++;
}

void pptxImageRGB(pptxPresentation *presentation, int iw, int ih, const unsigned char *rgb_data, int x, int y, int w, int h)
{
  const char *image =
  {/*012345678| - primitives ident (9) */
    "         <p:pic>\n"
    "            <p:nvPicPr>\n"
    "               <p:cNvPr id=\"%d\" name=\"Shape %d\"/>\n"
    "               <p:cNvPicPr preferRelativeResize=\"0\"/>\n"
    "               <p:nvPr/>\n"
    "            </p:nvPicPr>\n"
    "            <p:blipFill>\n"
    "               <a:blip r:embed=\"rId%d\">\n"
    "                  <a:alphaModFix/>\n"
    "               </a:blip>\n"
    "               <a:stretch>\n"
    "                  <a:fillRect/>\n"
    "               </a:stretch>\n"
    "            </p:blipFill>\n"
    "            <p:spPr>\n"
    "               <a:xfrm>\n"
    "                  <a:off x=\"%d\" y=\"%d\"/>\n"
    "                  <a:ext cx=\"%d\" cy=\"%d\"/>\n"
    "               </a:xfrm>\n"
    "               <a:prstGeom prst=\"rect\">\n"
    "                  <a:avLst/>\n"
    "               </a:prstGeom>\n"
    "               <a:noFill/>\n"
    "               <a:ln>\n"
    "                  <a:noFill/>\n"
    "               </a:ln>\n"
    "            </p:spPr>\n"
    "         </p:pic>\n"
  };
  char filename[10240];

  sprintf(filename, "%s/"PPTX_IMAGE_FILE, presentation->baseDir, presentation->mediaNum);

  lodepng_encode_file(filename, rgb_data, iw, ih, LCT_RGB, 8);

  fprintf(presentation->slideFile, image, presentation->objectNum, presentation->objectNum, presentation->imageId, x*presentation->slide_xfactor, y*presentation->slide_yfactor,
          w*presentation->slide_xfactor, h*presentation->slide_yfactor);

  printSlideRels(presentation);

  presentation->objectNum++;
  presentation->mediaNum++;
  presentation->imageId++;
}

void pptxImageRGBA(pptxPresentation *presentation, int iw, int ih, const unsigned char *rgba_data, int x, int y, int w, int h)
{
  const char *image =
  {/*012345678| - primitives ident (9) */
    "         <p:pic>\n"
    "            <p:nvPicPr>\n"
    "               <p:cNvPr id=\"%d\" name=\"Shape %d\"/>\n"
    "               <p:cNvPicPr preferRelativeResize=\"0\"/>\n"
    "               <p:nvPr/>\n"
    "            </p:nvPicPr>\n"
    "            <p:blipFill>\n"
    "               <a:blip r:embed=\"rId%d\">\n"
    "                  <a:alphaModFix/>\n"
    "               </a:blip>\n"
    "               <a:stretch>\n"
    "                  <a:fillRect/>\n"
    "               </a:stretch>\n"
    "            </p:blipFill>\n"
    "            <p:spPr>\n"
    "               <a:xfrm>\n"
    "                  <a:off x=\"%d\" y=\"%d\"/>\n"
    "                  <a:ext cx=\"%d\" cy=\"%d\"/>\n"
    "               </a:xfrm>\n"
    "               <a:prstGeom prst=\"rect\">\n"
    "                  <a:avLst/>\n"
    "               </a:prstGeom>\n"
    "               <a:noFill/>\n"
    "               <a:ln>\n"
    "                  <a:noFill/>\n"
    "               </a:ln>\n"
    "            </p:spPr>\n"
    "         </p:pic>\n"
  };
  char filename[10240];

  sprintf(filename, "%s/"PPTX_IMAGE_FILE, presentation->baseDir, presentation->mediaNum);

  lodepng_encode_file(filename, rgba_data, iw, ih, LCT_RGBA, 8);

  fprintf(presentation->slideFile, image, presentation->objectNum, presentation->objectNum, presentation->imageId, x*presentation->slide_xfactor, y*presentation->slide_yfactor,
          w*presentation->slide_xfactor, h*presentation->slide_yfactor);

  printSlideRels(presentation);

  presentation->objectNum++;
  presentation->mediaNum++;
  presentation->imageId++;
}


/*************************************  CREATE/KILL  *********************************************************/


static void createSubDirectory(const char* dirname, const char* subdir)
{
  char dir[10240];
  sprintf(dir, "%s/%s", dirname, subdir);
  cdMakeDirectory(dir);
}

static void removeSubDirectory(const char* dirname, const char* subdir)
{
  char dir[10240];
  sprintf(dir, "%s/%s", dirname, subdir);
  cdRemoveDirectory(dir);
}

static void removeFile(const char* dirname, const char* file)
{
  char filename[10240];
  sprintf(filename, "%s/%s", dirname, file);
  remove(filename);
}

pptxPresentation* pptxCreatePresentation(double width_mm, double height_mm, int width, int height)
{
  pptxPresentation *presentation = (pptxPresentation*)malloc(sizeof(pptxPresentation));
  memset(presentation, 0, sizeof(pptxPresentation));

  /* Units are in EMU (English Metric Units) min=914400 max=51206400.
     1 emu = 1 / 914400 in = 1 / 360000 cm */
  presentation->slideHeight = (int)(height_mm * 36000);  /* min=25mm max=1422mm */
  presentation->slideWidth = (int)(width_mm * 36000);

  presentation->slide_xfactor = presentation->slideWidth / width;
  presentation->slide_yfactor = presentation->slideHeight / height;

  cdStrTmpFileName(presentation->baseDir);
  remove(presentation->baseDir);
  if (!cdMakeDirectory(presentation->baseDir))
  {
    free(presentation);
    return NULL;
  }

  /* must be created in this order */
  createSubDirectory(presentation->baseDir, PPTX_PPT_DIR);
  createSubDirectory(presentation->baseDir, PPTX_PPT_RELS_DIR);
  createSubDirectory(presentation->baseDir, PPTX_LAYOUTS_DIR);
  createSubDirectory(presentation->baseDir, PPTX_LAYOUTS_RELS_DIR);
  createSubDirectory(presentation->baseDir, PPTX_MASTERS_DIR);
  createSubDirectory(presentation->baseDir, PPTX_MASTERS_RELS_DIR);
  createSubDirectory(presentation->baseDir, PPTX_SLIDES_DIR);
  createSubDirectory(presentation->baseDir, PPTX_SLIDES_RELS_DIR);
  createSubDirectory(presentation->baseDir, PPTX_THEME_DIR);
  createSubDirectory(presentation->baseDir, PPTX_MEDIA_DIR);
  createSubDirectory(presentation->baseDir, PPTX_MEDIA_MEDIA_DIR);
  createSubDirectory(presentation->baseDir, PPTX_RELS_DIR);

  presentation->slideNum = 0;
  presentation->objectNum = 51;
  presentation->mediaNum = 0;
  presentation->imageId = 4;

  if (!pptxOpenSlide(presentation))
  {
    free(presentation);
    return NULL;
  }

  pptxWritePresProps(presentation);

  pptxWriteRels(presentation);

  pptxWriteLayoutRels(presentation);

  pptxWriteLayout(presentation);

  pptxOpenWriteMasterRels(presentation);

  pptxOpenWriteMaster(presentation);

  pptxWriteTheme(presentation);

  return presentation;
}

static void closePresentation(pptxPresentation *presentation)
{
  pptxWritePresentation(presentation);

  pptxCloseWriteMasterRels(presentation);

  pptxCloseWriteMaster(presentation);

  pptxWriteContentTypes(presentation);

  pptxWritePptRels(presentation);

  pptxCloseSlide(presentation);
}

static void removeTempFiles(const char* dirname, const char **files, int nFiles)
{
  int i;

  for (i = 0; i < nFiles; i++)
    removeFile(dirname, files[i]);

  /* must be removed in this order (reverse from creation) */
  removeSubDirectory(dirname, PPTX_RELS_DIR);
  removeSubDirectory(dirname, PPTX_MEDIA_MEDIA_DIR);
  removeSubDirectory(dirname, PPTX_MEDIA_DIR);
  removeSubDirectory(dirname, PPTX_THEME_DIR);
  removeSubDirectory(dirname, PPTX_SLIDES_RELS_DIR);
  removeSubDirectory(dirname, PPTX_SLIDES_DIR);
  removeSubDirectory(dirname, PPTX_MASTERS_RELS_DIR);
  removeSubDirectory(dirname, PPTX_MASTERS_DIR);
  removeSubDirectory(dirname, PPTX_LAYOUTS_RELS_DIR);
  removeSubDirectory(dirname, PPTX_LAYOUTS_DIR);
  removeSubDirectory(dirname, PPTX_PPT_RELS_DIR);
  removeSubDirectory(dirname, PPTX_PPT_DIR);

  cdRemoveDirectory(dirname);
}

static int writeZipFile(pptxPresentation *presentation, const char* dirname, const char* filename)
{
  char **files;
  char mediadir[10240];
  cdDirData* dirData;
  int i, j, k, ret,
    count = presentation->mediaNum + 2 * presentation->slideNum + 10 + 10;

  strcpy(mediadir, dirname);
  strcat(mediadir, "/");
  strcat(mediadir, PPTX_MEDIA_DIR);

  dirData = cdDirIterOpen(mediadir);
  if (!dirData)
    return 0;

  if (cdDirIter(dirData) == 2)
  {
    cdDirClose(dirData);
    return 0;
  }

  files = malloc(count * sizeof(char*));
  for (i = 0; i < count; ++i)
    files[i] = malloc(80);

  i = 0;
  strcpy(files[i], PPTX_CONTENT_TYPES_FILE); i++;
  strcpy(files[i], PPTX_PRESENTATION_FILE); i++;
  strcpy(files[i], PPTX_PRES_PROPS_FILE); i++;
  for (j = 0; j < presentation->mediaNum; j++)
  {
    sprintf(files[i], PPTX_IMAGE_FILE, j); i++;
  }
  strcpy(files[i], PPTX_SLIDE_LAYOUT_FILE); i++;
  strcpy(files[i], PPTX_SLIDE_LAYOUT_RELS_FILE); i++;
  strcpy(files[i], PPTX_SLIDE_MASTER_FILE); i++;
  strcpy(files[i], PPTX_SLIDE_MASTER_RELS); i++;
  for (k = 0; k < presentation->slideNum; k++)
  {
    sprintf(files[i], PPTX_SLIDE_FILE, k + 1); i++;
    sprintf(files[i], PPTX_SLIDE_RELS_FILE, k + 1); i++;
  }
  strcpy(files[i], PPTX_THEME_FILE); i++;
  strcpy(files[i], PPTX_PRESENTATION_RELS); i++;
  strcpy(files[i], PPTX_RELS_FILE); i++;

  while (cdDirIter(dirData) == 1)
  {
    if (dirData->isDir == 0)
    {
      strcpy(files[i], PPTX_MEDIA_DIR);
      strcat(files[i], "/");
      strcat(files[i], dirData->filename);
      i++;
    }

    if (i == count)
    {
      int buff = 10;
      files = realloc(files, (count + buff) * sizeof(char*));
      for (i = count; i < count + buff; ++i)
        files[i] = malloc(80);
      count =+ buff;
    }
  }

  cdDirClose(dirData);

  ret = minizip(filename, dirname, files, i);

  removeTempFiles(dirname, files, i);

  for (i = 0; i < count; ++i)
    free(files[i]);
  free(files);

  return ret == 0;
}

void pptsBeginMasterFile(pptxPresentation *presentation)
{
  presentation->tmpFile = presentation->slideFile;
  presentation->tmpRelsFile = presentation->slideRelsFile;

  presentation->slideFile = presentation->masterSlideFile;
  presentation->slideRelsFile = presentation->masterSlideRelsFile;

}

void pptsEndMasterFile(pptxPresentation *presentation)
{
  presentation->slideFile = presentation->tmpFile;
  presentation->slideRelsFile = presentation->tmpRelsFile;

}

void pptxSetImportedMasterSlideFile(pptxPresentation* presentation, char* importedMasterSlideFile)
{
  presentation->importMasterSlideFile = cdStrDup(importedMasterSlideFile);
}

static void pptxCopyFile(char *unzippedDir, char *baseDir, char *fileName)
{
  char srcFile[1024];
  char destFile[1024];

  strcpy(srcFile, unzippedDir);
  strcat(srcFile, "//");
  strcat(srcFile, fileName);

  strcpy(destFile, baseDir);
  strcat(destFile, "//");
  strcat(destFile, fileName);

  cdCopyFile(srcFile, destFile);
}

static void pptxCopyMediaFiles(pptxPresentation* presentation, char* unzippedDir)
{
  char path[1024];
  char destDir[1024];
  cdDirData* dirData;

  strcpy(path, unzippedDir);
  strcat(path, "//");
  strcat(path, PPTX_MEDIA_DIR);

  strcpy(destDir, presentation->baseDir);
  strcat(destDir, "//");
  strcat(destDir, PPTX_MEDIA_DIR);

  dirData = cdDirIterOpen(path);
  if (!dirData)
    return;

  if (cdDirIter(dirData) == 2)
    return;

  if (dirData->isDir == 0)
    pptxCopyFile(path, destDir, dirData->filename);

  while (cdDirIter(dirData) == 1)
  {
    if (dirData->isDir == 0)
      pptxCopyFile(path, destDir, dirData->filename);
  }

  cdDirClose(dirData);
}

static void removeFilesFromDir(const char *dir)
{
  cdDirData* dirData = cdDirIterOpen(dir);
  if (!dirData)
    return;

  while (cdDirIter(dirData) == 1)
  {
    if (strcmp(dirData->filename, ".") == 0 || strcmp(dirData->filename, "..") == 0)
      continue;

    if (dirData->isDir == 1)
      removeFilesFromDir(dirData->filename);
    else
      removeFile(dir, dirData->filename);
  }

  cdDirClose(dirData);
}

static void pptxImportMasterSlide(pptxPresentation* presentation)
{
  char unzippedDir[10240];

  cdStrTmpFileName(unzippedDir);
  remove(unzippedDir);
  if (!cdMakeDirectory(unzippedDir))
    return;

  if (miniunzip(presentation->importMasterSlideFile, unzippedDir) == 0)
  {
    removeFile(presentation->baseDir, PPTX_THEME_FILE);
    pptxCopyFile(unzippedDir, presentation->baseDir, PPTX_THEME_FILE);
    removeFile(presentation->baseDir, PPTX_SLIDE_LAYOUT_RELS_FILE);
    pptxCopyFile(unzippedDir, presentation->baseDir, PPTX_SLIDE_LAYOUT_RELS_FILE);
    removeFile(presentation->baseDir, PPTX_SLIDE_LAYOUT_FILE);
    pptxCopyFile(unzippedDir, presentation->baseDir, PPTX_SLIDE_LAYOUT_FILE);
    removeFile(presentation->baseDir, PPTX_SLIDE_MASTER_RELS);
    pptxCopyFile(unzippedDir, presentation->baseDir, PPTX_SLIDE_MASTER_RELS);
    removeFile(presentation->baseDir, PPTX_SLIDE_MASTER_FILE);
    pptxCopyFile(unzippedDir, presentation->baseDir, PPTX_SLIDE_MASTER_FILE);

    pptxCopyMediaFiles(presentation, unzippedDir);
  }

  removeFilesFromDir(unzippedDir);

  cdRemoveDirectory(unzippedDir);
}

int pptxKillPresentation(pptxPresentation *presentation, const char* filename)
{
  int ret;

  closePresentation(presentation);

  if (presentation->importMasterSlideFile)
    pptxImportMasterSlide(presentation);

  ret = writeZipFile(presentation, presentation->baseDir, filename);

  if (presentation->importMasterSlideFile)
    free(presentation->importMasterSlideFile);

  free(presentation);

  return ret == 0;
}

