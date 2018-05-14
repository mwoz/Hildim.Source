#include "lua.h"
#include "lauxlib.h"
#include "iup.h"
#include "iuplua.h"
#include "iupcbs.h"
#include "iupkey.h"

#include "scite_images.h"


static Ihandle* load_image_disk_WW(void)
{
  unsigned char imgdata[] = {
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 15, 4, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 4, 15, 11,
    11, 4, 9, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 9, 4, 11,
    11, 4, 9, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 9, 4, 11,
    11, 4, 9, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 9, 4, 11,
    11, 10, 15, 11, 5, 5, 5, 5, 5, 5, 5, 5, 11, 15, 10, 11,
    11, 10, 15, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 15, 10, 11,
    11, 12, 15, 13, 5, 5, 5, 5, 5, 5, 5, 5, 13, 15, 12, 11,
    11, 12, 15, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 15, 12, 11,
    11, 12, 8, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 15, 12, 11,
    11, 12, 8, 6, 7, 7, 7, 7, 7, 7, 6, 4, 6, 15, 12, 11,
    11, 6, 8, 12, 7, 0, 0, 1, 1, 7, 12, 4, 12, 8, 6, 11,
    11, 6, 8, 12, 1, 12, 12, 1, 1, 1, 12, 4, 10, 8, 6, 11,
    11, 7, 0, 6, 1, 12, 12, 1, 1, 1, 12, 4, 4, 8, 0, 11,
    13, 3, 1, 0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 6, 13,
    11, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 11};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "115 64 139");
  IupSetAttribute(image, "1", "173 171 183");
  IupSetAttribute(image, "2", "116 118 116");
  IupSetAttribute(image, "3", "212 210 212");
  IupSetAttribute(image, "4", "167 116 193");
  IupSetAttribute(image, "5", "227 242 242");
  IupSetAttribute(image, "6", "141 89 167");
  IupSetAttribute(image, "7", "195 191 198");
  IupSetAttribute(image, "8", "189 136 213");
  IupSetAttribute(image, "9", "215 162 239");
  IupSetAttribute(image, "10", "156 110 188");
  IupSetAttribute(image, "11", "BGCOLOR");
  IupSetAttribute(image, "12", "151 100 176");
  IupSetAttribute(image, "13", "227 229 230");
  IupSetAttribute(image, "14", "189 205 204");
  IupSetAttribute(image, "15", "198 150 223");

  return image;
}

static Ihandle* load_image_bookmark__arrow_left_WW(void)
{
  unsigned char imgdata[] = {
    4, 4, 7, 3, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 7, 3, 6, 3, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    7, 3, 6, 6, 6, 3, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 6, 6, 6, 6, 6, 3, 7, 4, 4, 4, 4, 4, 4, 4, 4,
    7, 3, 6, 6, 6, 6, 6, 3, 7, 4, 4, 4, 4, 4, 4, 4,
    4, 7, 3, 6, 6, 6, 6, 6, 3, 7, 4, 4, 4, 4, 4, 4,
    4, 4, 7, 3, 6, 6, 6, 6, 6, 3, 7, 4, 4, 4, 4, 4,
    4, 4, 4, 7, 3, 6, 6, 6, 6, 6, 3, 7, 4, 4, 4, 4,
    4, 4, 4, 4, 7, 3, 6, 6, 6, 6, 6, 3, 7, 4, 4, 4,
    4, 4, 7, 1, 4, 7, 3, 6, 6, 6, 3, 6, 3, 7, 4, 4,
    4, 7, 1, 1, 4, 4, 7, 3, 6, 3, 3, 3, 6, 3, 7, 4,
    7, 1, 5, 1, 1, 1, 1, 0, 3, 6, 3, 3, 6, 6, 3, 7,
    0, 5, 5, 5, 5, 5, 5, 0, 7, 3, 6, 6, 3, 3, 3, 3,
    2, 0, 5, 0, 0, 0, 0, 1, 4, 7, 3, 6, 3, 4, 4, 4,
    4, 1, 0, 0, 4, 4, 4, 4, 4, 4, 2, 3, 3, 4, 4, 4,
    4, 4, 1, 0, 4, 4, 4, 4, 4, 7, 7, 2, 3, 7, 4, 4};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "77 141 192");
  IupSetAttribute(image, "1", "113 173 222");
  IupSetAttribute(image, "2", "191 177 180");
  IupSetAttribute(image, "3", "225 76 78");
  IupSetAttribute(image, "4", "BGCOLOR");
  IupSetAttribute(image, "5", "108 214 252");
  IupSetAttribute(image, "6", "252 112 114");
  IupSetAttribute(image, "7", "244 198 196");
  IupSetAttribute(image, "8", "0 0 0");
  IupSetAttribute(image, "9", "0 0 0");
  IupSetAttribute(image, "10", "0 0 0");
  IupSetAttribute(image, "11", "0 0 0");
  IupSetAttribute(image, "12", "0 0 0");
  IupSetAttribute(image, "13", "0 0 0");
  IupSetAttribute(image, "14", "0 0 0");
  IupSetAttribute(image, "15", "0 0 0");

  return image;
}

static Ihandle* load_image_IMAGE_Constant(void)
{
  unsigned char imgdata[] = {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 13, 10, 9, 9, 9, 9, 9, 8, 8, 8, 11, 7, 7,
    7, 7, 7, 10, 15, 15, 15, 15, 14, 14, 13, 13, 12, 8, 7, 7,
    7, 7, 7, 10, 15, 15, 15, 14, 14, 13, 13, 12, 12, 6, 7, 7,
    7, 7, 7, 9, 15, 15, 14, 5, 2, 0, 12, 12, 11, 6, 7, 7,
    7, 7, 7, 9, 15, 14, 14, 13, 13, 12, 12, 11, 11, 4, 7, 7,
    7, 7, 7, 9, 14, 14, 13, 5, 2, 0, 11, 11, 10, 3, 7, 7,
    7, 7, 7, 9, 14, 13, 13, 12, 12, 11, 11, 10, 10, 2, 7, 7,
    7, 7, 7, 9, 13, 13, 12, 12, 11, 11, 10, 10, 9, 1, 7, 7,
    7, 7, 7, 11, 6, 6, 5, 4, 3, 3, 2, 1, 1, 9, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "49 74 99");
  IupSetAttribute(image, "1", "57 78 107");
  IupSetAttribute(image, "2", "66 82 107");
  IupSetAttribute(image, "3", "68 87 115");
  IupSetAttribute(image, "4", "74 99 123");
  IupSetAttribute(image, "5", "82 99 123");
  IupSetAttribute(image, "6", "82 107 132");
  IupSetAttribute(image, "7", "BGCOLOR");
  IupSetAttribute(image, "8", "113 131 157");
  IupSetAttribute(image, "9", "154 168 189");
  IupSetAttribute(image, "10", "169 181 202");
  IupSetAttribute(image, "11", "183 192 211");
  IupSetAttribute(image, "12", "198 206 222");
  IupSetAttribute(image, "13", "214 219 228");
  IupSetAttribute(image, "14", "230 235 241");
  IupSetAttribute(image, "15", "251 251 255");

  return image;
}

static Ihandle* load_image_compile_WW(void)
{
  unsigned char imgdata[] = {
    8, 8, 6, 8, 8, 8, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 11, 6, 11, 11, 11, 8, 8, 8, 9, 0, 9, 6, 0, 6,
    8, 8, 4, 11, 4, 11, 4, 8, 8, 8, 0, 8, 0, 8, 0, 6,
    8, 8, 3, 4, 13, 13, 3, 8, 8, 8, 0, 8, 0, 8, 0, 6,
    8, 8, 12, 13, 14, 10, 12, 8, 8, 8, 9, 0, 9, 6, 0, 9,
    8, 8, 12, 10, 12, 14, 12, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 12, 10, 12, 14, 12, 8, 8, 8, 9, 0, 8, 9, 0, 9,
    8, 8, 7, 14, 1, 1, 7, 8, 8, 8, 6, 0, 8, 0, 8, 0,
    8, 8, 7, 12, 7, 1, 7, 8, 8, 8, 6, 0, 8, 0, 8, 0,
    3, 7, 7, 12, 1, 1, 7, 7, 3, 8, 9, 0, 6, 9, 0, 9,
    7, 14, 14, 1, 1, 1, 14, 14, 7, 8, 8, 8, 8, 8, 8, 8,
    15, 7, 5, 1, 1, 1, 5, 7, 15, 8, 9, 0, 9, 6, 0, 6,
    8, 15, 7, 5, 5, 5, 7, 15, 8, 8, 0, 8, 0, 8, 0, 6,
    8, 8, 15, 7, 5, 7, 15, 8, 8, 8, 0, 8, 0, 8, 0, 6,
    8, 6, 9, 2, 7, 2, 9, 6, 6, 8, 9, 0, 9, 9, 0, 9,
    8, 6, 6, 6, 6, 6, 6, 6, 8, 8, 8, 8, 8, 8, 8, 8};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "74 74 74");
  IupSetAttribute(image, "1", "31 181 47");
  IupSetAttribute(image, "2", "140 166 140");
  IupSetAttribute(image, "3", "98 187 108");
  IupSetAttribute(image, "4", "154 223 162");
  IupSetAttribute(image, "5", "33 207 51");
  IupSetAttribute(image, "6", "228 229 228");
  IupSetAttribute(image, "7", "50 139 56");
  IupSetAttribute(image, "8", "BGCOLOR");
  IupSetAttribute(image, "9", "195 194 195");
  IupSetAttribute(image, "10", "84 208 100");
  IupSetAttribute(image, "11", "186 236 194");
  IupSetAttribute(image, "12", "57 175 69");
  IupSetAttribute(image, "13", "116 218 129");
  IupSetAttribute(image, "14", "65 202 81");
  IupSetAttribute(image, "15", "164 197 164");

  return image;
}

static Ihandle* load_image_Tree_Null_WW(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    14, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 14,
    5, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 5,
    5, 15, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 15, 5,
    5, 15, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 15, 5,
    5, 15, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 15, 5,
    12, 15, 0, 3, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 15, 12,
    12, 15, 0, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 15, 12,
    4, 15, 0, 3, 0, 0, 3, 0, 3, 0, 3, 0, 3, 0, 15, 4,
    4, 15, 0, 3, 3, 0, 3, 6, 0, 0, 13, 0, 6, 0, 6, 4,
    4, 15, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 15, 4,
    4, 15, 13, 13, 13, 13, 7, 15, 15, 15, 15, 15, 15, 15, 15, 4,
    4, 13, 15, 13, 13, 15, 15, 2, 10, 4, 4, 1, 8, 8, 8, 4,
    6, 4, 2, 15, 15, 8, 12, 1, 14, 2, 0, 0, 10, 10, 4, 6,
    0, 14, 12, 4, 4, 1, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "BGCOLOR");
  IupSetAttribute(image, "1", "128 130 140");
  IupSetAttribute(image, "2", "199 198 196");
  IupSetAttribute(image, "3", "225 228 228");
  IupSetAttribute(image, "4", "87 93 111");
  IupSetAttribute(image, "5", "106 113 136");
  IupSetAttribute(image, "6", "161 160 164");
  IupSetAttribute(image, "7", "236 242 244");
  IupSetAttribute(image, "8", "206 209 212");
  IupSetAttribute(image, "9", "180 182 180");
  IupSetAttribute(image, "10", "143 147 151");
  IupSetAttribute(image, "11", "117 122 147");
  IupSetAttribute(image, "12", "97 104 123");
  IupSetAttribute(image, "13", "233 236 240");
  IupSetAttribute(image, "14", "168 168 168");
  IupSetAttribute(image, "15", "252 254 252");

  return image;
}

static Ihandle* load_image_document__plus_WW(void)
{
  unsigned char imgdata[] = {
    6, 6, 2, 8, 8, 8, 8, 8, 8, 8, 8, 4, 6, 6, 6, 6,
    6, 6, 8, 6, 6, 4, 4, 4, 4, 4, 8, 8, 4, 6, 6, 6,
    6, 6, 8, 6, 4, 4, 4, 4, 2, 2, 15, 6, 8, 4, 6, 6,
    6, 6, 8, 6, 4, 4, 4, 4, 4, 2, 1, 1, 1, 1, 6, 6,
    6, 6, 8, 6, 6, 4, 4, 4, 4, 4, 2, 2, 4, 8, 6, 6,
    6, 6, 8, 6, 6, 6, 4, 4, 4, 4, 4, 2, 4, 8, 6, 6,
    6, 6, 8, 6, 6, 6, 6, 4, 4, 4, 4, 4, 4, 8, 6, 6,
    6, 6, 8, 6, 6, 6, 6, 6, 4, 4, 4, 4, 4, 8, 6, 6,
    6, 6, 15, 6, 6, 6, 6, 6, 6, 4, 4, 4, 4, 15, 6, 6,
    6, 6, 15, 6, 6, 6, 6, 6, 6, 6, 4, 12, 3, 11, 6, 6,
    6, 6, 15, 6, 6, 6, 6, 6, 6, 6, 6, 3, 10, 14, 6, 6,
    6, 6, 15, 6, 6, 6, 6, 6, 6, 12, 14, 14, 10, 14, 3, 12,
    6, 6, 15, 6, 6, 6, 6, 6, 6, 14, 10, 10, 10, 10, 10, 14,
    6, 6, 1, 6, 6, 6, 6, 6, 6, 12, 14, 14, 9, 7, 14, 12,
    6, 4, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 9, 0, 4, 6,
    4, 2, 1, 5, 5, 5, 5, 5, 5, 5, 5, 13, 0, 13, 2, 4};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "28 124 40");
  IupSetAttribute(image, "1", "148 165 163");
  IupSetAttribute(image, "2", "211 224 224");
  IupSetAttribute(image, "3", "52 182 68");
  IupSetAttribute(image, "4", "230 242 241");
  IupSetAttribute(image, "5", "126 143 142");
  IupSetAttribute(image, "6", "BGCOLOR");
  IupSetAttribute(image, "7", "40 150 52");
  IupSetAttribute(image, "8", "181 198 198");
  IupSetAttribute(image, "9", "36 224 52");
  IupSetAttribute(image, "10", "75 232 91");
  IupSetAttribute(image, "11", "76 170 92");
  IupSetAttribute(image, "12", "98 192 110");
  IupSetAttribute(image, "13", "60 122 68");
  IupSetAttribute(image, "14", "48 167 60");
  IupSetAttribute(image, "15", "162 179 178");

  return image;
}

static Ihandle* load_image_IMAGE_PinPush(void)
{
  unsigned char imgdata[] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 12, 13, 12, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 5, 5, 5, 4, 4, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 5, 5, 5, 3, 3, 2, 4, 15, 15, 15, 15,
    15, 15, 15, 15, 8, 4, 6, 6, 5, 5, 3, 4, 11, 15, 15, 15,
    15, 15, 15, 13, 8, 4, 2, 2, 2, 0, 0, 7, 15, 15, 15, 15,
    15, 15, 15, 15, 12, 8, 6, 6, 3, 0, 4, 11, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 10, 6, 6, 3, 0, 4, 10, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 8, 5, 6, 3, 0, 1, 9, 15, 15, 15, 15,
    15, 15, 15, 15, 11, 7, 5, 6, 3, 0, 1, 7, 10, 15, 15, 15,
    15, 15, 15, 12, 8, 2, 5, 5, 3, 0, 0, 1, 8, 12, 15, 15,
    15, 15, 15, 9, 2, 3, 3, 2, 2, 1, 0, 0, 1, 10, 15, 15,
    15, 15, 10, 3, 6, 7, 6, 5, 5, 3, 3, 1, 0, 1, 12, 15,
    15, 11, 6, 6, 7, 9, 9, 7, 7, 5, 3, 2, 0, 1, 8, 15,
    15, 10, 5, 5, 6, 6, 7, 6, 5, 3, 3, 1, 0, 1, 8, 12,
    15, 13, 7, 5, 3, 4, 2, 2, 2, 1, 0, 1, 1, 7, 10, 15};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "15 92 150");
  IupSetAttribute(image, "1", "45 110 159");
  IupSetAttribute(image, "2", "50 126 183");
  IupSetAttribute(image, "3", "63 143 204");
  IupSetAttribute(image, "4", "85 140 185");
  IupSetAttribute(image, "5", "95 165 219");
  IupSetAttribute(image, "6", "115 181 231");
  IupSetAttribute(image, "7", "152 195 227");
  IupSetAttribute(image, "8", "177 202 222");
  IupSetAttribute(image, "9", "187 216 239");
  IupSetAttribute(image, "10", "216 230 239");
  IupSetAttribute(image, "11", "233 244 247");
  IupSetAttribute(image, "12", "243 247 251");
  IupSetAttribute(image, "13", "247 247 255");
  IupSetAttribute(image, "14", "247 255 255");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_Tree_Number_WW(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    7, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 7,
    6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6,
    6, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 6,
    6, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 6,
    6, 5, 3, 10, 9, 3, 9, 9, 10, 3, 9, 9, 10, 3, 5, 6,
    14, 5, 3, 9, 9, 3, 3, 10, 9, 3, 3, 10, 9, 3, 5, 14,
    14, 5, 3, 3, 9, 3, 8, 9, 10, 3, 3, 9, 10, 3, 5, 14,
    4, 5, 15, 15, 9, 15, 9, 10, 15, 15, 15, 10, 9, 15, 5, 4,
    4, 5, 15, 15, 9, 15, 9, 9, 9, 15, 9, 9, 10, 15, 5, 4,
    4, 5, 15, 15, 15, 15, 15, 5, 5, 5, 5, 5, 5, 5, 5, 4,
    4, 5, 15, 15, 15, 15, 15, 5, 5, 5, 5, 5, 5, 5, 5, 4,
    4, 15, 5, 15, 15, 5, 5, 2, 12, 4, 4, 1, 8, 8, 8, 4,
    7, 4, 2, 5, 5, 8, 14, 1, 7, 2, 0, 0, 12, 12, 4, 7,
    0, 7, 14, 4, 4, 1, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "BGCOLOR");
  IupSetAttribute(image, "1", "128 130 140");
  IupSetAttribute(image, "2", "200 200 200");
  IupSetAttribute(image, "3", "223 228 228");
  IupSetAttribute(image, "4", "87 93 111");
  IupSetAttribute(image, "5", "250 252 251");
  IupSetAttribute(image, "6", "106 113 136");
  IupSetAttribute(image, "7", "164 165 167");
  IupSetAttribute(image, "8", "208 213 212");
  IupSetAttribute(image, "9", "4 2 252");
  IupSetAttribute(image, "10", "164 166 252");
  IupSetAttribute(image, "11", "180 182 180");
  IupSetAttribute(image, "12", "143 147 151");
  IupSetAttribute(image, "13", "117 122 147");
  IupSetAttribute(image, "14", "97 104 123");
  IupSetAttribute(image, "15", "230 235 237");

  return image;
}

static Ihandle* load_image_application_sidebar_left_WW(void)
{
  unsigned char imgdata[] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    11, 1, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 1, 11,
    1, 5, 12, 12, 3, 3, 5, 5, 1, 1, 1, 1, 1, 5, 1, 1,
    6, 14, 14, 14, 6, 6, 6, 13, 13, 13, 4, 4, 4, 9, 9, 9,
    14, 2, 2, 2, 2, 2, 12, 15, 15, 15, 15, 15, 15, 15, 15, 14,
    14, 7, 7, 7, 7, 7, 3, 15, 15, 15, 15, 15, 15, 15, 15, 14,
    6, 7, 3, 3, 3, 7, 3, 15, 15, 15, 15, 15, 15, 15, 15, 6,
    6, 7, 5, 5, 5, 5, 5, 8, 8, 8, 8, 8, 8, 8, 8, 6,
    13, 3, 5, 1, 10, 1, 5, 8, 11, 11, 11, 11, 11, 11, 8, 13,
    13, 3, 1, 10, 10, 1, 1, 8, 11, 11, 11, 11, 11, 11, 8, 13,
    4, 5, 10, 10, 10, 1, 1, 11, 2, 2, 2, 2, 2, 2, 11, 4,
    4, 1, 1, 1, 1, 1, 14, 11, 2, 2, 2, 2, 2, 2, 11, 4,
    9, 1, 1, 1, 1, 1, 6, 11, 2, 2, 2, 2, 2, 2, 11, 9,
    9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9,
    8, 11, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 11, 8,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "132 130 132");
  IupSetAttribute(image, "1", "192 197 214");
  IupSetAttribute(image, "2", "227 229 230");
  IupSetAttribute(image, "3", "208 213 221");
  IupSetAttribute(image, "4", "164 164 164");
  IupSetAttribute(image, "5", "200 204 215");
  IupSetAttribute(image, "6", "180 180 180");
  IupSetAttribute(image, "7", "211 219 237");
  IupSetAttribute(image, "8", "244 245 244");
  IupSetAttribute(image, "9", "154 155 154");
  IupSetAttribute(image, "10", "180 188 220");
  IupSetAttribute(image, "11", "236 235 236");
  IupSetAttribute(image, "12", "220 219 220");
  IupSetAttribute(image, "13", "172 172 172");
  IupSetAttribute(image, "14", "188 186 188");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_IMAGE_ArrowDown(void)
{
  unsigned char imgdata[] = {
    15, 15, 15, 15, 15, 12, 0, 0, 0, 12, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 0, 7, 8, 8, 1, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 0, 6, 6, 10, 1, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 0, 6, 6, 10, 1, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 0, 6, 5, 10, 1, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 0, 6, 4, 10, 1, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 0, 6, 4, 10, 1, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 0, 5, 4, 10, 1, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 0, 4, 3, 8, 1, 15, 15, 15, 15, 15, 15,
    15, 7, 0, 0, 0, 0, 4, 3, 8, 1, 1, 1, 1, 7, 15, 15,
    15, 13, 1, 10, 6, 5, 3, 3, 5, 8, 8, 11, 1, 14, 15, 15,
    15, 15, 13, 1, 10, 5, 3, 2, 2, 2, 11, 1, 14, 15, 15, 15,
    15, 15, 15, 13, 1, 8, 2, 2, 2, 11, 1, 14, 15, 15, 15, 15,
    15, 15, 15, 15, 13, 1, 7, 2, 11, 0, 14, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 13, 1, 11, 0, 14, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 13, 1, 14, 15, 15, 15, 15, 15, 15, 15};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "30 112 175");
  IupSetAttribute(image, "1", "41 123 189");
  IupSetAttribute(image, "2", "24 153 225");
  IupSetAttribute(image, "3", "33 164 235");
  IupSetAttribute(image, "4", "45 175 241");
  IupSetAttribute(image, "5", "55 179 241");
  IupSetAttribute(image, "6", "68 186 241");
  IupSetAttribute(image, "7", "99 183 229");
  IupSetAttribute(image, "8", "115 198 239");
  IupSetAttribute(image, "9", "115 198 247");
  IupSetAttribute(image, "10", "123 206 247");
  IupSetAttribute(image, "11", "148 204 236");
  IupSetAttribute(image, "12", "165 198 231");
  IupSetAttribute(image, "13", "173 206 231");
  IupSetAttribute(image, "14", "181 214 239");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_ui_status_bar_blue_WW(void)
{
  unsigned char imgdata[] = {
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    6, 6, 6, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 6, 15, 6,
    10, 15, 10, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 10, 7, 10,
    8, 10, 8, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 8, 12, 8,
    11, 10, 14, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 14, 2, 11,
    11, 10, 14, 13, 13, 9, 13, 13, 13, 13, 13, 13, 13, 14, 2, 11,
    11, 10, 14, 6, 6, 1, 6, 6, 6, 6, 6, 6, 6, 14, 2, 11,
    11, 10, 5, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 5, 2, 11,
    0, 10, 7, 10, 8, 8, 8, 8, 4, 4, 4, 12, 12, 2, 2, 0,
    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5,
    13, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "62 124 182");
  IupSetAttribute(image, "1", "172 174 172");
  IupSetAttribute(image, "2", "15 209 252");
  IupSetAttribute(image, "3", "172 224 252");
  IupSetAttribute(image, "4", "97 187 244");
  IupSetAttribute(image, "5", "100 144 184");
  IupSetAttribute(image, "6", "225 231 233");
  IupSetAttribute(image, "7", "124 220 252");
  IupSetAttribute(image, "8", "121 191 240");
  IupSetAttribute(image, "9", "203 201 203");
  IupSetAttribute(image, "10", "151 209 248");
  IupSetAttribute(image, "11", "88 157 216");
  IupSetAttribute(image, "12", "49 203 249");
  IupSetAttribute(image, "13", "BGCOLOR");
  IupSetAttribute(image, "14", "126 158 189");
  IupSetAttribute(image, "15", "188 242 252");

  return image;
}

static Ihandle* load_image_windows_WW(void)
{
  unsigned char imgdata[] = {
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 12, 10, 4, 4, 10, 12, 14, 14, 14, 14, 14,
    14, 14, 14, 12, 4, 10, 7, 12, 12, 7, 10, 4, 12, 14, 14, 14,
    14, 14, 12, 4, 7, 15, 15, 15, 7, 12, 12, 7, 4, 12, 14, 14,
    14, 14, 4, 10, 2, 15, 13, 13, 7, 11, 7, 10, 10, 4, 14, 14,
    14, 7, 8, 10, 2, 13, 5, 5, 7, 6, 11, 10, 8, 4, 7, 14,
    14, 4, 8, 8, 2, 2, 2, 13, 7, 6, 5, 11, 8, 8, 4, 14,
    14, 0, 8, 0, 3, 11, 9, 10, 10, 12, 11, 10, 0, 8, 0, 14,
    14, 0, 8, 8, 1, 11, 9, 10, 12, 7, 7, 7, 0, 8, 0, 14,
    14, 4, 8, 3, 3, 1, 11, 7, 6, 6, 6, 7, 0, 8, 4, 14,
    14, 7, 0, 0, 0, 0, 3, 5, 6, 6, 5, 8, 8, 0, 7, 14,
    14, 14, 0, 4, 8, 8, 8, 7, 5, 5, 13, 8, 8, 0, 14, 14,
    14, 14, 12, 0, 8, 10, 10, 8, 8, 4, 8, 8, 0, 12, 14, 14,
    14, 14, 14, 12, 0, 8, 10, 10, 10, 10, 8, 0, 12, 14, 14, 14,
    14, 14, 14, 9, 12, 10, 8, 0, 0, 8, 10, 12, 9, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "52 79 122");
  IupSetAttribute(image, "1", "44 162 220");
  IupSetAttribute(image, "2", "151 100 65");
  IupSetAttribute(image, "3", "38 130 169");
  IupSetAttribute(image, "4", "82 87 113");
  IupSetAttribute(image, "5", "215 187 36");
  IupSetAttribute(image, "6", "233 219 67");
  IupSetAttribute(image, "7", "152 143 115");
  IupSetAttribute(image, "8", "81 101 136");
  IupSetAttribute(image, "9", "196 214 224");
  IupSetAttribute(image, "10", "107 130 140");
  IupSetAttribute(image, "11", "125 183 143");
  IupSetAttribute(image, "12", "177 180 175");
  IupSetAttribute(image, "13", "224 142 24");
  IupSetAttribute(image, "14", "BGCOLOR");
  IupSetAttribute(image, "15", "206 84 36");

  return image;
}

static Ihandle* load_image_cross_button_WW(void)
{
  unsigned char imgdata[] = {
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 1, 12, 4, 4, 4, 4, 4, 4, 4, 4, 12, 1, 8, 8,
    8, 1, 12, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 12, 1, 8,
    8, 12, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 12, 8,
    8, 4, 10, 9, 9, 8, 9, 9, 9, 9, 8, 9, 9, 9, 4, 8,
    8, 4, 9, 12, 8, 8, 8, 12, 12, 8, 8, 8, 12, 9, 4, 8,
    8, 11, 9, 4, 14, 8, 8, 13, 13, 5, 13, 0, 0, 15, 11, 8,
    8, 11, 12, 4, 4, 0, 5, 5, 5, 13, 0, 0, 0, 14, 11, 8,
    8, 11, 12, 15, 0, 0, 5, 5, 13, 13, 0, 0, 0, 3, 11, 8,
    8, 11, 4, 3, 3, 5, 5, 13, 13, 13, 13, 3, 3, 3, 11, 8,
    8, 11, 7, 3, 5, 5, 13, 0, 0, 13, 13, 13, 3, 3, 11, 8,
    8, 11, 7, 7, 3, 13, 3, 7, 7, 3, 13, 3, 7, 7, 11, 8,
    8, 11, 7, 7, 7, 3, 7, 7, 7, 7, 3, 7, 7, 7, 11, 8,
    8, 1, 14, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 14, 1, 8,
    13, 5, 2, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 2, 5, 13,
    8, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 8};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "167 19 21");
  IupSetAttribute(image, "1", "207 142 140");
  IupSetAttribute(image, "2", "164 126 124");
  IupSetAttribute(image, "3", "182 19 21");
  IupSetAttribute(image, "4", "187 50 52");
  IupSetAttribute(image, "5", "211 211 211");
  IupSetAttribute(image, "6", "127 53 55");
  IupSetAttribute(image, "7", "211 20 21");
  IupSetAttribute(image, "8", "BGCOLOR");
  IupSetAttribute(image, "9", "221 84 85");
  IupSetAttribute(image, "10", "240 106 104");
  IupSetAttribute(image, "11", "166 53 54");
  IupSetAttribute(image, "12", "200 69 68");
  IupSetAttribute(image, "13", "225 227 225");
  IupSetAttribute(image, "14", "167 39 41");
  IupSetAttribute(image, "15", "180 40 40");

  return image;
}

static Ihandle* load_image_building__arrow_WW(void)
{
  unsigned char imgdata[] = {
    14, 14, 14, 14, 5, 8, 8, 8, 8, 2, 12, 2, 14, 14, 14, 14,
    14, 14, 14, 14, 8, 11, 11, 11, 11, 5, 5, 12, 14, 14, 14, 14,
    14, 14, 14, 14, 8, 11, 11, 11, 11, 8, 5, 12, 14, 14, 14, 14,
    14, 14, 5, 8, 12, 8, 8, 8, 8, 12, 12, 4, 12, 2, 14, 14,
    14, 14, 8, 11, 11, 11, 11, 11, 11, 11, 8, 8, 5, 12, 14, 14,
    14, 14, 2, 5, 2, 8, 2, 8, 2, 8, 12, 12, 10, 4, 14, 14,
    14, 14, 8, 11, 3, 3, 3, 3, 3, 11, 2, 2, 8, 12, 14, 14,
    14, 14, 2, 8, 2, 8, 12, 8, 12, 8, 12, 12, 12, 4, 14, 14,
    14, 14, 2, 3, 5, 5, 5, 5, 5, 3, 2, 2, 2, 12, 14, 14,
    14, 14, 10, 8, 12, 2, 12, 2, 12, 2, 12, 4, 6, 4, 14, 14,
    14, 14, 2, 5, 5, 5, 5, 5, 5, 5, 12, 12, 7, 7, 3, 14,
    14, 14, 12, 2, 12, 2, 12, 2, 7, 7, 7, 7, 7, 15, 6, 3,
    14, 14, 2, 5, 5, 5, 5, 5, 7, 15, 9, 9, 9, 9, 9, 7,
    14, 14, 12, 5, 8, 4, 4, 4, 7, 13, 13, 13, 13, 1, 7, 5,
    14, 11, 4, 5, 5, 4, 12, 4, 5, 5, 12, 12, 13, 0, 10, 14,
    11, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 2, 11};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "44 87 137");
  IupSetAttribute(image, "1", "60 198 252");
  IupSetAttribute(image, "2", "176 175 191");
  IupSetAttribute(image, "3", "201 215 233");
  IupSetAttribute(image, "4", "126 128 146");
  IupSetAttribute(image, "5", "200 201 219");
  IupSetAttribute(image, "6", "100 154 204");
  IupSetAttribute(image, "7", "74 142 203");
  IupSetAttribute(image, "8", "187 186 203");
  IupSetAttribute(image, "9", "100 212 252");
  IupSetAttribute(image, "10", "148 170 196");
  IupSetAttribute(image, "11", "226 226 241");
  IupSetAttribute(image, "12", "156 156 174");
  IupSetAttribute(image, "13", "50 115 178");
  IupSetAttribute(image, "14", "BGCOLOR");
  IupSetAttribute(image, "15", "148 228 252");

  return image;
}

static Ihandle* load_image_document_copy_WW(void)
{
  unsigned char imgdata[] = {
    6, 5, 8, 8, 8, 8, 8, 8, 13, 6, 6, 6, 6, 6, 6, 6,
    6, 8, 6, 15, 3, 3, 3, 12, 8, 13, 6, 6, 6, 6, 6, 6,
    6, 8, 6, 3, 3, 2, 2, 11, 6, 8, 13, 6, 6, 6, 6, 6,
    6, 12, 6, 3, 3, 8, 1, 9, 9, 9, 9, 8, 13, 6, 6, 6,
    6, 12, 6, 15, 3, 1, 6, 15, 3, 3, 3, 12, 8, 13, 6, 6,
    6, 1, 6, 6, 15, 1, 6, 3, 3, 2, 2, 11, 6, 8, 13, 6,
    6, 1, 6, 6, 6, 1, 6, 3, 3, 3, 2, 4, 4, 4, 4, 6,
    6, 11, 6, 6, 6, 1, 6, 15, 3, 3, 3, 2, 2, 3, 12, 6,
    6, 11, 6, 6, 6, 1, 6, 6, 15, 3, 3, 3, 2, 3, 1, 6,
    6, 7, 6, 6, 6, 11, 6, 6, 6, 15, 3, 3, 3, 3, 1, 6,
    6, 7, 6, 6, 6, 11, 6, 6, 6, 6, 15, 3, 3, 15, 11, 6,
    6, 4, 6, 6, 6, 11, 6, 6, 6, 6, 6, 15, 3, 6, 11, 6,
    10, 4, 9, 9, 9, 0, 6, 6, 6, 6, 6, 6, 15, 6, 7, 6,
    13, 10, 10, 10, 10, 4, 6, 6, 6, 6, 6, 6, 6, 6, 7, 6,
    6, 6, 6, 6, 13, 4, 6, 6, 6, 6, 6, 6, 6, 6, 4, 13,
    6, 6, 6, 6, 10, 4, 9, 9, 9, 9, 9, 9, 9, 9, 4, 14};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "92 118 116");
  IupSetAttribute(image, "1", "171 192 191");
  IupSetAttribute(image, "2", "217 233 233");
  IupSetAttribute(image, "3", "233 245 244");
  IupSetAttribute(image, "4", "145 161 160");
  IupSetAttribute(image, "5", "204 218 220");
  IupSetAttribute(image, "6", "BGCOLOR");
  IupSetAttribute(image, "7", "160 176 176");
  IupSetAttribute(image, "8", "186 204 204");
  IupSetAttribute(image, "9", "124 147 148");
  IupSetAttribute(image, "10", "228 229 228");
  IupSetAttribute(image, "11", "167 184 184");
  IupSetAttribute(image, "12", "180 195 196");
  IupSetAttribute(image, "13", "231 237 236");
  IupSetAttribute(image, "14", "220 222 220");
  IupSetAttribute(image, "15", "236 250 252");

  return image;
}

static Ihandle* load_image_arrow_curve_270_WW(void)
{
  unsigned char imgdata[] = {
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 9, 11, 13, 9, 15, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 9, 13, 13, 11, 9, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 9, 9, 9, 13, 9, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 4, 13, 13, 13, 11, 15, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 15, 11, 13, 13, 13, 9, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 3, 13, 11, 13, 11, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 12, 13, 11, 13, 12, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 12, 13, 3, 11, 12, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 12, 3, 12, 3, 12, 8, 8, 8, 8, 8, 8,
    8, 8, 11, 0, 0, 0, 3, 7, 3, 0, 0, 0, 11, 8, 8, 8,
    8, 8, 6, 0, 5, 10, 10, 7, 10, 10, 10, 0, 6, 8, 8, 8,
    8, 8, 8, 6, 0, 14, 2, 2, 2, 14, 0, 6, 8, 8, 8, 8,
    8, 8, 8, 8, 6, 0, 14, 2, 14, 0, 6, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 15, 1, 0, 2, 0, 1, 15, 8, 8, 8, 8, 8,
    8, 8, 8, 15, 4, 6, 1, 0, 1, 6, 4, 15, 8, 8, 8, 8};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "63 127 188");
  IupSetAttribute(image, "1", "156 180 204");
  IupSetAttribute(image, "2", "18 175 247");
  IupSetAttribute(image, "3", "84 167 222");
  IupSetAttribute(image, "4", "208 222 232");
  IupSetAttribute(image, "5", "84 190 236");
  IupSetAttribute(image, "6", "194 209 223");
  IupSetAttribute(image, "7", "48 154 220");
  IupSetAttribute(image, "8", "BGCOLOR");
  IupSetAttribute(image, "9", "149 204 244");
  IupSetAttribute(image, "10", "56 169 230");
  IupSetAttribute(image, "11", "105 174 227");
  IupSetAttribute(image, "12", "77 147 206");
  IupSetAttribute(image, "13", "117 192 241");
  IupSetAttribute(image, "14", "48 188 248");
  IupSetAttribute(image, "15", "215 235 249");

  return image;
}

static Ihandle* load_image_key__plus_WW(void)
{
  unsigned char imgdata[] = {
    8, 8, 8, 3, 5, 2, 2, 5, 3, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 3, 2, 5, 5, 5, 5, 2, 3, 8, 8, 8, 8, 8, 8,
    8, 3, 2, 3, 2, 7, 7, 7, 5, 2, 3, 8, 8, 8, 8, 8,
    8, 2, 3, 2, 2, 8, 8, 2, 7, 5, 2, 8, 8, 8, 8, 8,
    8, 7, 3, 7, 7, 7, 7, 7, 7, 5, 7, 8, 8, 8, 8, 8,
    8, 13, 3, 3, 3, 5, 2, 2, 5, 5, 13, 8, 8, 8, 8, 8,
    8, 4, 2, 3, 5, 2, 7, 7, 5, 7, 4, 8, 8, 8, 8, 8,
    8, 8, 2, 7, 5, 2, 7, 2, 7, 2, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 4, 6, 5, 2, 6, 4, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 6, 5, 2, 6, 8, 8, 8, 14, 1, 14, 8, 8,
    8, 8, 8, 8, 6, 5, 6, 3, 8, 8, 8, 1, 12, 1, 8, 8,
    8, 8, 8, 8, 6, 5, 5, 6, 8, 14, 1, 1, 12, 1, 1, 14,
    8, 8, 8, 8, 6, 5, 6, 3, 8, 9, 12, 12, 12, 12, 12, 9,
    8, 8, 8, 8, 6, 5, 5, 6, 8, 14, 9, 9, 10, 9, 9, 14,
    8, 8, 8, 8, 6, 5, 6, 13, 8, 8, 8, 9, 10, 9, 8, 8,
    8, 8, 8, 3, 13, 11, 13, 4, 3, 8, 8, 15, 0, 15, 8, 8};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "52 134 52");
  IupSetAttribute(image, "1", "52 180 68");
  IupSetAttribute(image, "2", "215 186 117");
  IupSetAttribute(image, "3", "241 225 186");
  IupSetAttribute(image, "4", "215 205 188");
  IupSetAttribute(image, "5", "233 205 128");
  IupSetAttribute(image, "6", "163 135 82");
  IupSetAttribute(image, "7", "195 168 77");
  IupSetAttribute(image, "8", "BGCOLOR");
  IupSetAttribute(image, "9", "52 163 62");
  IupSetAttribute(image, "10", "36 224 52");
  IupSetAttribute(image, "11", "132 98 68");
  IupSetAttribute(image, "12", "75 232 91");
  IupSetAttribute(image, "13", "188 173 142");
  IupSetAttribute(image, "14", "100 194 113");
  IupSetAttribute(image, "15", "100 166 108");

  return image;
}

static Ihandle* load_image_IMAGE_ArrowUp(void)
{
  unsigned char imgdata[] = {
    15, 15, 15, 15, 15, 15, 13, 1, 14, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 13, 1, 11, 0, 14, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 13, 1, 7, 2, 11, 0, 14, 15, 15, 15, 15, 15,
    15, 15, 15, 13, 1, 8, 2, 2, 2, 11, 1, 14, 15, 15, 15, 15,
    15, 15, 13, 1, 10, 5, 3, 2, 2, 2, 11, 1, 14, 15, 15, 15,
    15, 13, 1, 10, 6, 5, 3, 3, 5, 8, 8, 11, 1, 14, 15, 15,
    15, 7, 0, 0, 0, 0, 4, 3, 8, 1, 1, 1, 1, 7, 15, 15,
    15, 15, 15, 15, 15, 0, 4, 3, 8, 1, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 0, 5, 4, 10, 1, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 0, 6, 4, 10, 1, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 0, 6, 4, 10, 1, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 0, 6, 5, 10, 1, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 0, 6, 6, 10, 1, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 0, 6, 6, 10, 1, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 0, 7, 8, 8, 1, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 12, 0, 0, 0, 12, 15, 15, 15, 15, 15, 15};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "30 112 175");
  IupSetAttribute(image, "1", "41 123 189");
  IupSetAttribute(image, "2", "24 153 225");
  IupSetAttribute(image, "3", "33 164 235");
  IupSetAttribute(image, "4", "45 175 241");
  IupSetAttribute(image, "5", "55 179 241");
  IupSetAttribute(image, "6", "68 186 241");
  IupSetAttribute(image, "7", "99 183 229");
  IupSetAttribute(image, "8", "115 198 239");
  IupSetAttribute(image, "9", "115 198 247");
  IupSetAttribute(image, "10", "123 206 247");
  IupSetAttribute(image, "11", "148 204 236");
  IupSetAttribute(image, "12", "165 198 231");
  IupSetAttribute(image, "13", "173 206 231");
  IupSetAttribute(image, "14", "181 214 239");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_tree_WW(void)
{
  unsigned char imgdata[] = {
    15, 15, 15, 15, 15, 15, 6, 5, 5, 6, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 6, 5, 14, 14, 5, 6, 15, 15, 15, 15, 15,
    15, 15, 6, 5, 3, 3, 14, 14, 14, 14, 3, 3, 5, 7, 15, 15,
    15, 6, 5, 14, 14, 1, 1, 1, 1, 1, 1, 1, 1, 5, 15, 15,
    15, 3, 14, 14, 14, 14, 5, 5, 5, 5, 5, 1, 1, 3, 15, 15,
    15, 3, 1, 1, 1, 1, 5, 1, 14, 14, 1, 5, 14, 5, 6, 15,
    15, 6, 3, 5, 5, 3, 1, 14, 14, 14, 14, 1, 5, 1, 3, 7,
    15, 6, 3, 5, 1, 3, 1, 1, 1, 1, 1, 1, 3, 5, 13, 6,
    15, 3, 5, 5, 5, 13, 5, 5, 5, 5, 5, 5, 13, 3, 6, 15,
    7, 0, 3, 5, 5, 13, 13, 3, 3, 3, 13, 13, 13, 6, 15, 15,
    6, 0, 0, 3, 3, 13, 13, 13, 13, 8, 10, 0, 9, 15, 15, 15,
    15, 15, 9, 0, 0, 0, 0, 8, 10, 10, 8, 9, 15, 15, 15, 15,
    15, 15, 15, 6, 7, 15, 12, 2, 10, 2, 12, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 11, 2, 4, 2, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 11, 11, 7, 10, 10, 4, 2, 12, 11, 11, 15, 15, 15,
    15, 15, 11, 12, 12, 2, 2, 2, 2, 2, 2, 12, 12, 11, 15, 15};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "53 102 52");
  IupSetAttribute(image, "1", "104 189 97");
  IupSetAttribute(image, "2", "169 117 69");
  IupSetAttribute(image, "3", "67 146 66");
  IupSetAttribute(image, "4", "240 192 140");
  IupSetAttribute(image, "5", "85 167 81");
  IupSetAttribute(image, "6", "151 190 151");
  IupSetAttribute(image, "7", "214 228 214");
  IupSetAttribute(image, "8", "86 121 66");
  IupSetAttribute(image, "9", "140 167 140");
  IupSetAttribute(image, "10", "206 159 111");
  IupSetAttribute(image, "11", "233 232 230");
  IupSetAttribute(image, "12", "213 211 205");
  IupSetAttribute(image, "13", "49 128 45");
  IupSetAttribute(image, "14", "122 208 110");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_layout_design_WW(void)
{
  unsigned char imgdata[] = {
    6, 8, 2, 8, 2, 8, 2, 8, 2, 8, 2, 8, 2, 8, 6, 14,
    8, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 8, 14,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 14,
    8, 14, 14, 14, 2, 14, 14, 14, 14, 14, 14, 14, 14, 14, 6, 14,
    15, 14, 14, 14, 2, 14, 14, 14, 14, 14, 14, 14, 14, 14, 2, 14,
    8, 14, 14, 14, 2, 14, 14, 14, 14, 14, 14, 14, 14, 14, 6, 14,
    15, 14, 14, 14, 15, 14, 11, 9, 11, 14, 14, 14, 14, 14, 2, 14,
    8, 14, 8, 14, 2, 6, 13, 3, 9, 8, 8, 8, 8, 14, 6, 14,
    12, 14, 8, 14, 6, 12, 14, 10, 11, 8, 8, 8, 8, 14, 15, 14,
    8, 8, 8, 6, 5, 4, 7, 2, 8, 8, 8, 8, 8, 8, 2, 14,
    12, 8, 6, 5, 4, 1, 2, 8, 8, 8, 8, 8, 8, 8, 15, 14,
    14, 6, 5, 4, 1, 2, 8, 8, 8, 8, 8, 8, 8, 8, 2, 14,
    8, 1, 4, 1, 15, 15, 12, 12, 12, 12, 12, 12, 12, 12, 12, 14,
    11, 14, 1, 2, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 12, 14,
    0, 1, 6, 2, 7, 7, 7, 7, 7, 7, 7, 12, 7, 12, 12, 14,
    14, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 14, 14};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "4 2 4");
  IupSetAttribute(image, "1", "167 149 60");
  IupSetAttribute(image, "2", "202 200 199");
  IupSetAttribute(image, "3", "252 130 132");
  IupSetAttribute(image, "4", "244 230 124");
  IupSetAttribute(image, "5", "196 178 68");
  IupSetAttribute(image, "6", "219 216 200");
  IupSetAttribute(image, "7", "150 150 150");
  IupSetAttribute(image, "8", "233 233 232");
  IupSetAttribute(image, "9", "212 68 68");
  IupSetAttribute(image, "10", "148 118 116");
  IupSetAttribute(image, "11", "224 182 168");
  IupSetAttribute(image, "12", "171 173 171");
  IupSetAttribute(image, "13", "188 146 148");
  IupSetAttribute(image, "14", "BGCOLOR");
  IupSetAttribute(image, "15", "187 187 181");

  return image;
}

static Ihandle* load_image_control_double_180_WW(void)
{
  unsigned char imgdata[] = {
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 12, 7, 11, 11, 11, 11, 12, 7, 11, 11,
    11, 11, 11, 11, 11, 12, 4, 4, 11, 11, 11, 12, 4, 4, 11, 11,
    11, 11, 11, 11, 12, 4, 3, 4, 11, 11, 12, 4, 3, 4, 11, 11,
    11, 11, 11, 12, 13, 3, 7, 13, 11, 12, 13, 3, 7, 13, 11, 11,
    11, 11, 12, 13, 3, 4, 7, 13, 12, 13, 7, 6, 4, 13, 11, 11,
    11, 11, 13, 3, 4, 13, 4, 13, 13, 4, 6, 6, 13, 13, 11, 11,
    11, 11, 9, 0, 7, 6, 8, 0, 9, 0, 8, 6, 8, 0, 11, 11,
    11, 11, 11, 9, 0, 14, 2, 0, 11, 9, 0, 2, 2, 0, 11, 11,
    11, 11, 11, 11, 9, 0, 2, 0, 11, 11, 9, 0, 2, 0, 11, 11,
    11, 11, 11, 11, 11, 9, 0, 0, 11, 11, 11, 9, 0, 0, 11, 11,
    11, 11, 11, 11, 5, 5, 1, 15, 10, 10, 10, 10, 1, 15, 5, 11,
    11, 11, 11, 11, 11, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "66 128 188");
  IupSetAttribute(image, "1", "144 164 188");
  IupSetAttribute(image, "2", "34 183 247");
  IupSetAttribute(image, "3", "148 225 252");
  IupSetAttribute(image, "4", "98 174 232");
  IupSetAttribute(image, "5", "229 231 229");
  IupSetAttribute(image, "6", "58 151 214");
  IupSetAttribute(image, "7", "126 203 246");
  IupSetAttribute(image, "8", "65 175 233");
  IupSetAttribute(image, "9", "164 196 225");
  IupSetAttribute(image, "10", "204 206 204");
  IupSetAttribute(image, "11", "BGCOLOR");
  IupSetAttribute(image, "12", "183 216 241");
  IupSetAttribute(image, "13", "83 155 213");
  IupSetAttribute(image, "14", "76 198 252");
  IupSetAttribute(image, "15", "92 128 164");

  return image;
}

static Ihandle* load_image_IMAGE_CheckSpelling(void)
{
  unsigned char imgdata[] = {
    15, 15, 3, 15, 10, 2, 2, 3, 15, 9, 2, 2, 2, 13, 15, 15,
    15, 10, 3, 10, 10, 2, 15, 8, 8, 2, 8, 15, 3, 8, 15, 15,
    15, 2, 15, 0, 10, 0, 1, 3, 9, 2, 10, 15, 15, 15, 12, 6,
    8, 0, 0, 0, 3, 2, 15, 3, 2, 2, 8, 15, 3, 3, 5, 6,
    1, 8, 15, 8, 0, 0, 0, 2, 10, 8, 1, 0, 0, 4, 5, 14,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 7, 5, 5, 14, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 7, 5, 5, 14, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 7, 5, 5, 12, 15, 15, 15,
    15, 15, 15, 7, 7, 15, 15, 15, 7, 6, 4, 11, 15, 15, 15, 15,
    15, 15, 7, 6, 6, 6, 15, 7, 6, 4, 11, 15, 15, 15, 15, 15,
    15, 15, 4, 4, 5, 6, 6, 6, 4, 5, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 5, 4, 5, 5, 4, 4, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 11, 4, 4, 4, 7, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 6, 4, 6, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "24 24 26");
  IupSetAttribute(image, "1", "45 62 91");
  IupSetAttribute(image, "2", "85 85 87");
  IupSetAttribute(image, "3", "126 129 133");
  IupSetAttribute(image, "4", "61 101 183");
  IupSetAttribute(image, "5", "88 126 203");
  IupSetAttribute(image, "6", "124 152 216");
  IupSetAttribute(image, "7", "145 176 238");
  IupSetAttribute(image, "8", "165 165 165");
  IupSetAttribute(image, "9", "191 191 191");
  IupSetAttribute(image, "10", "214 214 214");
  IupSetAttribute(image, "11", "193 208 234");
  IupSetAttribute(image, "12", "214 222 239");
  IupSetAttribute(image, "13", "222 222 222");
  IupSetAttribute(image, "14", "222 231 239");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_printer_WW(void)
{
  unsigned char imgdata[] = {
    6, 6, 6, 13, 5, 5, 5, 5, 5, 5, 13, 6, 6, 6, 6, 6,
    6, 6, 6, 5, 6, 6, 6, 6, 6, 8, 5, 13, 6, 6, 6, 6,
    6, 6, 6, 5, 6, 13, 13, 13, 13, 4, 6, 5, 6, 6, 6, 6,
    6, 6, 6, 8, 6, 6, 13, 13, 13, 14, 14, 14, 14, 6, 6, 6,
    6, 6, 6, 8, 6, 6, 6, 13, 13, 13, 13, 6, 8, 6, 6, 6,
    6, 5, 8, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 5, 6,
    6, 8, 13, 7, 11, 11, 11, 11, 11, 11, 11, 11, 7, 13, 8, 6,
    13, 8, 13, 12, 7, 12, 12, 15, 10, 0, 0, 10, 12, 8, 8, 13,
    5, 8, 5, 10, 12, 10, 10, 0, 0, 0, 0, 10, 10, 8, 14, 5,
    8, 5, 8, 0, 12, 10, 10, 10, 10, 10, 10, 10, 0, 2, 8, 8,
    14, 8, 14, 0, 12, 15, 15, 15, 15, 15, 15, 15, 0, 1, 14, 14,
    14, 8, 8, 7, 11, 11, 11, 11, 11, 11, 11, 11, 7, 8, 8, 14,
    1, 8, 8, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 8, 8, 1,
    1, 11, 11, 7, 13, 9, 4, 4, 3, 3, 3, 13, 7, 11, 11, 1,
    6, 13, 13, 1, 6, 6, 6, 6, 6, 6, 6, 6, 1, 13, 13, 6,
    6, 6, 6, 14, 11, 11, 11, 11, 11, 11, 11, 11, 14, 6, 6, 6};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "53 51 53");
  IupSetAttribute(image, "1", "152 155 156");
  IupSetAttribute(image, "2", "100 254 100");
  IupSetAttribute(image, "3", "55 105 121");
  IupSetAttribute(image, "4", "151 181 201");
  IupSetAttribute(image, "5", "192 208 206");
  IupSetAttribute(image, "6", "BGCOLOR");
  IupSetAttribute(image, "7", "113 114 114");
  IupSetAttribute(image, "8", "188 191 195");
  IupSetAttribute(image, "9", "68 134 228");
  IupSetAttribute(image, "10", "68 69 74");
  IupSetAttribute(image, "11", "129 134 137");
  IupSetAttribute(image, "12", "92 90 92");
  IupSetAttribute(image, "13", "220 226 226");
  IupSetAttribute(image, "14", "160 169 169");
  IupSetAttribute(image, "15", "78 80 92");

  return image;
}

static Ihandle* load_image_IMAGE_View(void)
{
  unsigned char imgdata[] = {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    12, 12, 12, 11, 11, 11, 11, 11, 10, 10, 10, 7, 7, 7, 7, 7,
    12, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    12, 7, 10, 10, 10, 10, 10, 9, 9, 9, 9, 9, 9, 7, 7, 7,
    11, 7, 10, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    11, 7, 10, 7, 9, 9, 9, 9, 8, 8, 8, 8, 8, 8, 6, 7,
    11, 7, 9, 7, 9, 14, 14, 13, 13, 13, 13, 13, 13, 13, 6, 7,
    11, 7, 9, 7, 8, 13, 15, 15, 15, 15, 15, 15, 15, 12, 4, 7,
    11, 7, 9, 7, 8, 13, 15, 15, 15, 15, 15, 14, 14, 12, 3, 7,
    11, 7, 9, 7, 8, 13, 15, 15, 15, 14, 14, 14, 14, 12, 2, 7,
    7, 7, 9, 7, 6, 12, 15, 14, 14, 14, 14, 13, 13, 11, 1, 7,
    7, 7, 9, 7, 5, 12, 14, 14, 14, 14, 13, 13, 13, 11, 1, 7,
    7, 7, 7, 7, 4, 12, 12, 11, 11, 11, 11, 11, 11, 11, 0, 7,
    7, 7, 7, 7, 3, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "49 74 99");
  IupSetAttribute(image, "1", "57 78 107");
  IupSetAttribute(image, "2", "66 82 107");
  IupSetAttribute(image, "3", "70 90 117");
  IupSetAttribute(image, "4", "82 99 123");
  IupSetAttribute(image, "5", "82 99 132");
  IupSetAttribute(image, "6", "82 107 132");
  IupSetAttribute(image, "7", "BGCOLOR");
  IupSetAttribute(image, "8", "103 122 147");
  IupSetAttribute(image, "9", "129 146 173");
  IupSetAttribute(image, "10", "158 171 193");
  IupSetAttribute(image, "11", "187 195 214");
  IupSetAttribute(image, "12", "204 210 224");
  IupSetAttribute(image, "13", "227 233 242");
  IupSetAttribute(image, "14", "243 247 252");
  IupSetAttribute(image, "15", "255 255 255");

  return image;
}

static Ihandle* load_image_clipboard_paste_WW(void)
{
  unsigned char imgdata[] = {
    7, 7, 7, 7, 7, 7, 14, 15, 15, 14, 7, 7, 7, 7, 7, 7,
    7, 7, 4, 2, 2, 2, 4, 3, 3, 4, 2, 2, 2, 4, 7, 7,
    7, 7, 2, 10, 6, 14, 14, 3, 3, 14, 14, 6, 10, 2, 7, 7,
    7, 7, 2, 10, 2, 2, 2, 2, 2, 2, 2, 2, 10, 2, 7, 7,
    7, 7, 2, 10, 4, 6, 6, 5, 5, 5, 5, 5, 4, 2, 7, 7,
    7, 7, 2, 10, 4, 4, 4, 5, 7, 7, 7, 7, 8, 5, 3, 7,
    7, 7, 2, 10, 6, 6, 6, 5, 7, 7, 7, 3, 1, 7, 8, 3,
    7, 7, 2, 4, 6, 6, 6, 5, 7, 7, 7, 7, 1, 1, 1, 1,
    7, 7, 2, 4, 6, 2, 2, 13, 7, 7, 7, 7, 7, 3, 7, 8,
    7, 7, 13, 6, 2, 2, 2, 13, 7, 7, 7, 7, 7, 7, 7, 15,
    7, 7, 13, 6, 2, 2, 2, 13, 7, 7, 7, 7, 7, 7, 7, 1,
    7, 7, 13, 6, 2, 2, 2, 12, 7, 7, 7, 7, 7, 7, 7, 1,
    7, 7, 9, 6, 2, 2, 2, 12, 7, 7, 7, 7, 7, 7, 7, 1,
    7, 7, 9, 6, 2, 2, 2, 12, 0, 0, 0, 0, 0, 0, 11, 1,
    7, 3, 9, 6, 6, 6, 2, 2, 2, 2, 2, 2, 2, 9, 3, 7,
    3, 14, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 14, 3};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "101 63 52");
  IupSetAttribute(image, "1", "155 172 171");
  IupSetAttribute(image, "2", "165 97 48");
  IupSetAttribute(image, "3", "226 227 230");
  IupSetAttribute(image, "4", "181 130 95");
  IupSetAttribute(image, "5", "140 117 90");
  IupSetAttribute(image, "6", "177 111 61");
  IupSetAttribute(image, "7", "BGCOLOR");
  IupSetAttribute(image, "8", "177 194 193");
  IupSetAttribute(image, "9", "126 71 52");
  IupSetAttribute(image, "10", "208 139 88");
  IupSetAttribute(image, "11", "132 146 148");
  IupSetAttribute(image, "12", "108 86 60");
  IupSetAttribute(image, "13", "129 91 68");
  IupSetAttribute(image, "14", "204 204 212");
  IupSetAttribute(image, "15", "183 186 188");

  return image;
}

static Ihandle* load_image_IMAGE_Replace(void)
{
  unsigned char imgdata[] = {
    15, 15, 15, 15, 15, 10, 6, 11, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 12, 6, 10, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 10, 9, 12, 12, 6, 10, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 6, 2, 2, 3, 12, 6, 6, 10, 9, 12, 15, 15, 15, 15, 15,
    12, 6, 11, 4, 0, 12, 6, 10, 12, 6, 6, 15, 15, 15, 15, 15,
    15, 10, 4, 3, 0, 12, 6, 10, 15, 10, 6, 15, 15, 15, 15, 15,
    9, 2, 6, 6, 0, 12, 6, 10, 15, 9, 6, 15, 15, 15, 15, 15,
    6, 0, 3, 2, 0, 10, 6, 6, 10, 9, 12, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 6, 15, 15, 15, 15, 15, 15, 10, 6, 12, 15, 15, 13, 13, 15,
    15, 2, 12, 15, 15, 15, 15, 4, 2, 2, 2, 12, 8, 1, 1, 7,
    15, 2, 4, 11, 3, 12, 12, 6, 11, 4, 0, 8, 1, 5, 5, 1,
    15, 11, 3, 3, 3, 3, 15, 10, 4, 3, 0, 7, 1, 5, 15, 13,
    15, 15, 14, 14, 3, 11, 9, 2, 9, 9, 0, 7, 1, 3, 5, 1,
    15, 15, 15, 15, 15, 15, 6, 0, 2, 2, 0, 4, 8, 1, 1, 7,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "82 123 206");
  IupSetAttribute(image, "1", "82 222 231");
  IupSetAttribute(image, "2", "90 189 82");
  IupSetAttribute(image, "3", "123 123 123");
  IupSetAttribute(image, "4", "123 198 255");
  IupSetAttribute(image, "5", "148 255 148");
  IupSetAttribute(image, "6", "173 214 255");
  IupSetAttribute(image, "7", "189 255 181");
  IupSetAttribute(image, "8", "206 255 198");
  IupSetAttribute(image, "9", "222 222 222");
  IupSetAttribute(image, "10", "222 255 247");
  IupSetAttribute(image, "11", "231 231 231");
  IupSetAttribute(image, "12", "247 247 247");
  IupSetAttribute(image, "13", "247 255 239");
  IupSetAttribute(image, "14", "255 255 255");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_IMAGE_WithLineNumber(void)
{
  unsigned char imgdata[] = {
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 15, 6, 8, 8,
    8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 9, 6, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 15, 4, 14, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 5, 3, 1, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 7, 11, 8,
    8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 13, 4, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 14, 5, 14, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 3, 1, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 4, 8,
    8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 7, 12, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 10, 3, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 3, 14, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "0 0 0");
  IupSetAttribute(image, "1", "49 74 132");
  IupSetAttribute(image, "2", "49 74 140");
  IupSetAttribute(image, "3", "53 82 148");
  IupSetAttribute(image, "4", "60 93 164");
  IupSetAttribute(image, "5", "66 107 185");
  IupSetAttribute(image, "6", "74 111 198");
  IupSetAttribute(image, "7", "77 121 212");
  IupSetAttribute(image, "8", "BGCOLOR");
  IupSetAttribute(image, "9", "82 132 222");
  IupSetAttribute(image, "10", "148 173 222");
  IupSetAttribute(image, "11", "156 173 222");
  IupSetAttribute(image, "12", "181 198 231");
  IupSetAttribute(image, "13", "189 206 239");
  IupSetAttribute(image, "14", "198 206 231");
  IupSetAttribute(image, "15", "218 230 251");

  return image;
}

static Ihandle* load_image_IMAGE_FormatBasic(void)
{
  unsigned char imgdata[] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 10, 3, 2, 9, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 9, 2, 8, 2, 1, 15,
    15, 15, 15, 15, 15, 10, 3, 3, 3, 9, 2, 8, 2, 0, 0, 15,
    13, 13, 13, 13, 10, 3, 10, 9, 10, 3, 3, 2, 0, 0, 6, 13,
    13, 13, 12, 5, 4, 6, 3, 11, 9, 10, 1, 0, 0, 6, 13, 13,
    6, 5, 4, 7, 7, 7, 6, 3, 10, 2, 3, 0, 3, 13, 13, 13,
    5, 7, 7, 7, 7, 7, 7, 5, 1, 3, 1, 1, 0, 13, 13, 13,
    5, 7, 7, 7, 7, 7, 5, 5, 4, 0, 1, 0, 0, 11, 13, 13,
    10, 4, 7, 7, 5, 5, 5, 4, 4, 1, 0, 0, 6, 13, 13, 13,
    13, 6, 4, 5, 5, 5, 4, 4, 4, 1, 0, 9, 13, 13, 13, 13,
    12, 12, 3, 1, 4, 4, 4, 4, 1, 0, 3, 12, 12, 12, 12, 12,
    12, 12, 12, 3, 1, 4, 4, 1, 0, 3, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 3, 1, 1, 0, 3, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 6, 0, 1, 12, 12, 12, 12, 12, 12, 12, 12};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "77 78 79");
  IupSetAttribute(image, "1", "139 126 96");
  IupSetAttribute(image, "2", "125 123 170");
  IupSetAttribute(image, "3", "172 166 166");
  IupSetAttribute(image, "4", "218 201 119");
  IupSetAttribute(image, "5", "227 213 150");
  IupSetAttribute(image, "6", "215 211 188");
  IupSetAttribute(image, "7", "253 251 185");
  IupSetAttribute(image, "8", "173 177 222");
  IupSetAttribute(image, "9", "212 210 222");
  IupSetAttribute(image, "10", "228 226 226");
  IupSetAttribute(image, "11", "235 235 239");
  IupSetAttribute(image, "12", "247 243 247");
  IupSetAttribute(image, "13", "247 247 255");
  IupSetAttribute(image, "14", "247 255 255");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_color_WW(void)
{
  unsigned char imgdata[] = {
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 8, 15, 15, 11, 15, 8, 6, 6, 6, 6, 6,
    6, 6, 6, 4, 8, 8, 8, 8, 8, 8, 15, 11, 8, 6, 6, 6,
    6, 6, 4, 8, 4, 4, 8, 15, 15, 15, 8, 8, 11, 8, 6, 6,
    6, 6, 4, 4, 4, 8, 8, 8, 15, 15, 15, 15, 15, 11, 6, 6,
    6, 4, 4, 4, 4, 4, 4, 8, 8, 8, 15, 15, 15, 11, 15, 6,
    6, 3, 4, 4, 4, 4, 4, 6, 8, 8, 8, 15, 15, 11, 5, 6,
    6, 7, 4, 3, 4, 4, 6, 6, 6, 8, 8, 15, 5, 5, 14, 6,
    6, 1, 3, 3, 3, 3, 6, 6, 6, 2, 2, 5, 5, 5, 14, 6,
    6, 7, 7, 7, 3, 3, 3, 6, 6, 2, 2, 13, 13, 14, 14, 6,
    6, 7, 7, 7, 7, 3, 3, 3, 3, 2, 10, 13, 13, 0, 13, 6,
    6, 6, 1, 7, 7, 12, 12, 12, 12, 10, 10, 13, 0, 0, 6, 6,
    6, 6, 3, 1, 7, 12, 12, 12, 12, 10, 10, 9, 0, 2, 6, 6,
    6, 6, 6, 2, 1, 1, 12, 12, 12, 10, 9, 0, 2, 6, 6, 6,
    6, 6, 6, 4, 2, 10, 1, 9, 9, 9, 13, 2, 2, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "49 63 204");
  IupSetAttribute(image, "1", "59 180 92");
  IupSetAttribute(image, "2", "180 177 220");
  IupSetAttribute(image, "3", "175 241 174");
  IupSetAttribute(image, "4", "237 236 137");
  IupSetAttribute(image, "5", "173 112 247");
  IupSetAttribute(image, "6", "BGCOLOR");
  IupSetAttribute(image, "7", "107 235 100");
  IupSetAttribute(image, "8", "245 175 183");
  IupSetAttribute(image, "9", "50 131 188");
  IupSetAttribute(image, "10", "102 179 248");
  IupSetAttribute(image, "11", "223 95 192");
  IupSetAttribute(image, "12", "109 243 219");
  IupSetAttribute(image, "13", "116 124 233");
  IupSetAttribute(image, "14", "108 52 212");
  IupSetAttribute(image, "15", "244 128 194");

  return image;
}

static Ihandle* load_image_yin_yang_WW(void)
{
  unsigned char imgdata[] = {
    13, 13, 13, 13, 4, 8, 2, 2, 2, 8, 4, 13, 13, 13, 13, 13,
    13, 13, 13, 2, 12, 8, 4, 13, 4, 8, 12, 2, 13, 13, 13, 13,
    13, 4, 12, 8, 13, 13, 13, 13, 13, 13, 13, 8, 12, 4, 13, 13,
    13, 12, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 12, 13, 13,
    8, 6, 6, 5, 9, 1, 8, 4, 4, 4, 4, 4, 4, 6, 8, 13,
    12, 6, 9, 9, 9, 9, 5, 8, 8, 2, 2, 2, 2, 12, 12, 13,
    15, 14, 9, 9, 1, 9, 11, 15, 2, 12, 14, 12, 2, 2, 15, 13,
    14, 9, 3, 1, 13, 5, 7, 14, 2, 14, 0, 14, 2, 2, 14, 13,
    14, 9, 3, 3, 5, 11, 7, 5, 8, 2, 14, 2, 8, 12, 14, 13,
    1, 9, 3, 7, 7, 11, 11, 3, 12, 8, 8, 8, 8, 14, 1, 13,
    12, 3, 11, 11, 11, 11, 11, 11, 9, 6, 8, 2, 1, 3, 12, 13,
    13, 9, 11, 11, 11, 11, 11, 11, 3, 3, 3, 3, 11, 9, 13, 13,
    13, 8, 11, 11, 11, 3, 3, 3, 3, 3, 3, 11, 11, 8, 13, 13,
    13, 13, 8, 3, 10, 11, 3, 3, 3, 11, 10, 3, 8, 13, 13, 13,
    13, 4, 8, 12, 1, 3, 10, 0, 10, 3, 1, 2, 8, 4, 13, 13,
    13, 13, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 13, 13, 13};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "24 24 24");
  IupSetAttribute(image, "1", "142 142 144");
  IupSetAttribute(image, "2", "204 204 207");
  IupSetAttribute(image, "3", "81 81 89");
  IupSetAttribute(image, "4", "237 237 237");
  IupSetAttribute(image, "5", "114 116 118");
  IupSetAttribute(image, "6", "177 176 178");
  IupSetAttribute(image, "7", "58 59 63");
  IupSetAttribute(image, "8", "219 219 222");
  IupSetAttribute(image, "9", "99 100 100");
  IupSetAttribute(image, "10", "44 46 47");
  IupSetAttribute(image, "11", "71 70 78");
  IupSetAttribute(image, "12", "192 191 194");
  IupSetAttribute(image, "13", "BGCOLOR");
  IupSetAttribute(image, "14", "132 133 136");
  IupSetAttribute(image, "15", "156 158 159");

  return image;
}

static Ihandle* load_image_folder_search_result_WW(void)
{
  unsigned char imgdata[] = {
    10, 10, 10, 10, 10, 10, 10, 10, 10, 13, 2, 2, 2, 2, 7, 10,
    10, 10, 10, 5, 2, 2, 2, 2, 2, 2, 10, 7, 15, 0, 2, 5,
    10, 10, 10, 8, 3, 3, 3, 3, 3, 8, 3, 3, 3, 3, 3, 8,
    10, 10, 10, 8, 3, 3, 3, 3, 3, 3, 8, 8, 8, 8, 8, 8,
    10, 10, 10, 8, 5, 13, 13, 5, 5, 3, 3, 3, 3, 3, 3, 8,
    10, 10, 10, 8, 8, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 8,
    10, 10, 7, 8, 5, 6, 1, 1, 6, 5, 5, 5, 5, 5, 5, 8,
    10, 10, 2, 13, 8, 14, 7, 7, 14, 6, 5, 13, 5, 5, 5, 8,
    10, 10, 8, 5, 6, 12, 12, 12, 12, 9, 5, 13, 13, 13, 5, 6,
    10, 10, 6, 5, 6, 12, 7, 7, 12, 6, 5, 8, 13, 13, 5, 6,
    10, 10, 8, 13, 2, 4, 7, 7, 4, 8, 2, 8, 13, 13, 13, 6,
    10, 10, 4, 8, 5, 13, 8, 8, 2, 13, 6, 8, 13, 13, 13, 9,
    10, 11, 6, 5, 6, 8, 5, 5, 8, 9, 1, 1, 1, 1, 1, 6,
    4, 6, 5, 6, 4, 9, 1, 1, 9, 11, 10, 10, 10, 10, 10, 10,
    1, 5, 9, 4, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    14, 1, 14, 12, 11, 7, 7, 7, 7, 10, 10, 10, 10, 10, 10, 10};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "44 182 244");
  IupSetAttribute(image, "1", "169 134 67");
  IupSetAttribute(image, "2", "225 196 135");
  IupSetAttribute(image, "3", "250 235 166");
  IupSetAttribute(image, "4", "212 202 180");
  IupSetAttribute(image, "5", "245 217 147");
  IupSetAttribute(image, "6", "201 168 98");
  IupSetAttribute(image, "7", "231 233 233");
  IupSetAttribute(image, "8", "220 186 117");
  IupSetAttribute(image, "9", "189 155 92");
  IupSetAttribute(image, "10", "BGCOLOR");
  IupSetAttribute(image, "11", "225 217 201");
  IupSetAttribute(image, "12", "203 216 228");
  IupSetAttribute(image, "13", "237 204 133");
  IupSetAttribute(image, "14", "184 171 142");
  IupSetAttribute(image, "15", "68 202 252");

  return image;
}

static Ihandle* load_image_IMAGE_FormRun(void)
{
  unsigned char imgdata[] = {
    15, 15, 15, 15, 0, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 0, 0, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 0, 14, 0, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 0, 14, 14, 0, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 0, 13, 13, 12, 0, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 0, 12, 11, 11, 9, 0, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 0, 11, 9, 9, 7, 6, 0, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 0, 9, 7, 6, 6, 5, 5, 0, 15, 15, 15, 15,
    15, 15, 15, 15, 0, 6, 6, 5, 5, 4, 0, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 0, 6, 5, 4, 4, 0, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 0, 4, 4, 3, 0, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 0, 3, 3, 0, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 0, 2, 0, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 0, 0, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 0, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "16 115 8");
  IupSetAttribute(image, "1", "16 140 8");
  IupSetAttribute(image, "2", "24 140 8");
  IupSetAttribute(image, "3", "24 148 16");
  IupSetAttribute(image, "4", "37 154 18");
  IupSetAttribute(image, "5", "45 165 24");
  IupSetAttribute(image, "6", "63 175 33");
  IupSetAttribute(image, "7", "74 181 41");
  IupSetAttribute(image, "8", "74 189 41");
  IupSetAttribute(image, "9", "82 189 41");
  IupSetAttribute(image, "10", "82 198 41");
  IupSetAttribute(image, "11", "94 202 49");
  IupSetAttribute(image, "12", "99 206 57");
  IupSetAttribute(image, "13", "107 214 57");
  IupSetAttribute(image, "14", "115 214 57");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_disk__plus_WW(void)
{
  unsigned char imgdata[] = {
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 13, 1, 13, 8, 8,
    8, 14, 5, 12, 12, 12, 12, 12, 12, 12, 12, 1, 11, 9, 14, 8,
    8, 5, 15, 8, 8, 8, 8, 8, 8, 13, 1, 1, 11, 1, 9, 13,
    8, 5, 15, 8, 8, 8, 8, 8, 8, 1, 11, 11, 11, 11, 11, 1,
    8, 5, 15, 8, 8, 8, 8, 8, 8, 13, 1, 1, 6, 9, 9, 13,
    8, 7, 14, 8, 8, 8, 8, 8, 8, 8, 8, 9, 6, 9, 7, 8,
    8, 7, 14, 8, 4, 4, 4, 4, 4, 4, 4, 13, 9, 10, 7, 8,
    8, 7, 14, 4, 8, 8, 8, 8, 8, 8, 8, 8, 4, 14, 7, 8,
    8, 7, 14, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 14, 7, 8,
    8, 7, 14, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 14, 7, 8,
    8, 7, 14, 3, 12, 12, 12, 12, 12, 12, 3, 5, 7, 14, 7, 8,
    8, 3, 5, 7, 12, 3, 3, 2, 2, 12, 7, 5, 7, 14, 3, 8,
    8, 3, 14, 7, 2, 7, 7, 2, 2, 2, 7, 5, 7, 14, 3, 8,
    8, 12, 0, 3, 2, 7, 7, 2, 2, 2, 7, 5, 5, 14, 0, 8,
    4, 12, 2, 0, 10, 10, 10, 10, 10, 10, 0, 0, 0, 0, 3, 4,
    8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "108 58 132");
  IupSetAttribute(image, "1", "49 171 64");
  IupSetAttribute(image, "2", "173 171 183");
  IupSetAttribute(image, "3", "137 84 162");
  IupSetAttribute(image, "4", "226 227 229");
  IupSetAttribute(image, "5", "179 126 204");
  IupSetAttribute(image, "6", "36 224 52");
  IupSetAttribute(image, "7", "152 100 177");
  IupSetAttribute(image, "8", "BGCOLOR");
  IupSetAttribute(image, "9", "41 144 55");
  IupSetAttribute(image, "10", "111 119 114");
  IupSetAttribute(image, "11", "80 225 96");
  IupSetAttribute(image, "12", "192 199 202");
  IupSetAttribute(image, "13", "98 184 110");
  IupSetAttribute(image, "14", "195 146 220");
  IupSetAttribute(image, "15", "215 162 239");

  return image;
}

static Ihandle* load_image_arrow_return_270_WW(void)
{
  unsigned char imgdata[] = {
    11, 11, 11, 7, 15, 13, 6, 6, 13, 15, 7, 11, 11, 11, 11, 11,
    11, 11, 7, 6, 13, 4, 4, 4, 4, 13, 6, 7, 11, 11, 11, 11,
    11, 7, 6, 10, 10, 10, 4, 4, 10, 10, 10, 6, 7, 11, 11, 11,
    11, 15, 13, 13, 10, 6, 3, 3, 6, 10, 13, 13, 15, 11, 11, 11,
    11, 6, 10, 13, 6, 13, 11, 11, 13, 6, 13, 13, 6, 11, 11, 11,
    11, 8, 13, 13, 8, 11, 11, 11, 11, 8, 13, 13, 8, 11, 11, 11,
    11, 8, 13, 13, 8, 11, 11, 11, 11, 8, 3, 3, 8, 11, 11, 11,
    11, 8, 13, 13, 8, 11, 11, 11, 11, 8, 3, 3, 8, 11, 11, 11,
    11, 8, 13, 6, 8, 11, 11, 11, 11, 8, 3, 3, 8, 11, 11, 11,
    11, 0, 3, 3, 0, 11, 11, 11, 11, 0, 3, 3, 0, 11, 11, 11,
    11, 0, 12, 12, 0, 11, 14, 0, 0, 0, 5, 12, 0, 0, 0, 14,
    11, 0, 12, 12, 0, 11, 15, 0, 10, 5, 12, 2, 2, 12, 0, 15,
    11, 0, 12, 12, 0, 11, 11, 15, 0, 5, 2, 2, 2, 0, 15, 11,
    11, 14, 0, 0, 14, 11, 11, 11, 15, 0, 12, 2, 0, 15, 11, 11,
    11, 11, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 1, 7, 11, 11,
    11, 7, 7, 7, 9, 9, 9, 9, 9, 9, 1, 1, 9, 9, 7, 11};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "63 126 186");
  IupSetAttribute(image, "1", "152 168 184");
  IupSetAttribute(image, "2", "17 180 249");
  IupSetAttribute(image, "3", "86 175 229");
  IupSetAttribute(image, "4", "140 217 252");
  IupSetAttribute(image, "5", "76 200 252");
  IupSetAttribute(image, "6", "110 182 233");
  IupSetAttribute(image, "7", "223 227 230");
  IupSetAttribute(image, "8", "82 151 210");
  IupSetAttribute(image, "9", "207 207 207");
  IupSetAttribute(image, "10", "133 206 248");
  IupSetAttribute(image, "11", "BGCOLOR");
  IupSetAttribute(image, "12", "45 184 246");
  IupSetAttribute(image, "13", "123 199 245");
  IupSetAttribute(image, "14", "112 158 200");
  IupSetAttribute(image, "15", "164 199 230");

  return image;
}

static Ihandle* load_image_disk__pencil_WW(void)
{
  unsigned char imgdata[] = {
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 2, 6, 12,
    14, 3, 13, 2, 2, 2, 2, 2, 2, 2, 2, 2, 7, 4, 11, 6,
    14, 13, 12, 14, 14, 14, 14, 14, 14, 14, 14, 5, 7, 14, 9, 12,
    14, 13, 3, 14, 14, 14, 14, 14, 14, 14, 5, 1, 5, 4, 4, 14,
    14, 13, 3, 14, 14, 14, 14, 14, 14, 5, 1, 5, 1, 3, 13, 14,
    14, 13, 3, 14, 14, 14, 14, 14, 5, 1, 5, 1, 2, 3, 13, 14,
    14, 13, 3, 14, 14, 14, 14, 2, 1, 5, 1, 2, 14, 3, 13, 14,
    14, 13, 3, 2, 14, 14, 14, 1, 14, 1, 2, 14, 2, 3, 13, 14,
    14, 13, 3, 3, 3, 3, 3, 0, 8, 13, 3, 3, 3, 3, 13, 14,
    14, 13, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 13, 14,
    14, 4, 3, 4, 2, 2, 2, 2, 2, 2, 4, 13, 4, 3, 4, 14,
    14, 4, 3, 4, 7, 4, 4, 7, 7, 7, 4, 13, 13, 3, 4, 14,
    14, 4, 3, 13, 7, 13, 13, 7, 7, 7, 13, 13, 13, 3, 4, 14,
    14, 2, 10, 4, 7, 13, 13, 7, 7, 7, 13, 13, 13, 3, 10, 14,
    14, 2, 7, 10, 15, 15, 15, 15, 15, 15, 10, 10, 10, 10, 4, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "4 2 4");
  IupSetAttribute(image, "1", "184 169 73");
  IupSetAttribute(image, "2", "200 207 202");
  IupSetAttribute(image, "3", "194 144 217");
  IupSetAttribute(image, "4", "139 86 162");
  IupSetAttribute(image, "5", "240 229 145");
  IupSetAttribute(image, "6", "216 72 72");
  IupSetAttribute(image, "7", "178 177 187");
  IupSetAttribute(image, "8", "132 98 68");
  IupSetAttribute(image, "9", "116 66 100");
  IupSetAttribute(image, "10", "108 58 132");
  IupSetAttribute(image, "11", "252 130 132");
  IupSetAttribute(image, "12", "231 181 207");
  IupSetAttribute(image, "13", "157 107 181");
  IupSetAttribute(image, "14", "BGCOLOR");
  IupSetAttribute(image, "15", "116 118 116");

  return image;
}

static Ihandle* load_image_arrow_return_270_left_WW(void)
{
  unsigned char imgdata[] = {
    10, 10, 10, 10, 10, 15, 11, 7, 7, 7, 7, 11, 15, 10, 10, 10,
    10, 10, 10, 10, 8, 7, 14, 4, 4, 4, 4, 14, 7, 8, 10, 10,
    10, 10, 10, 15, 7, 4, 14, 4, 4, 4, 4, 14, 14, 7, 15, 10,
    10, 10, 10, 11, 7, 14, 14, 7, 3, 3, 7, 14, 14, 7, 11, 10,
    10, 10, 10, 3, 14, 7, 7, 11, 10, 10, 11, 7, 7, 14, 3, 10,
    10, 10, 10, 12, 14, 14, 12, 10, 10, 10, 10, 12, 7, 7, 12, 10,
    10, 10, 10, 6, 14, 14, 6, 10, 10, 10, 10, 6, 3, 3, 6, 10,
    10, 10, 10, 6, 7, 7, 6, 10, 10, 10, 10, 6, 12, 6, 6, 10,
    10, 10, 10, 6, 3, 3, 6, 10, 10, 10, 10, 6, 12, 6, 6, 10,
    10, 10, 10, 0, 3, 3, 0, 10, 10, 10, 10, 0, 6, 6, 0, 10,
    12, 0, 0, 0, 5, 13, 0, 0, 0, 12, 10, 0, 13, 13, 0, 10,
    11, 0, 4, 5, 13, 2, 13, 13, 0, 11, 10, 0, 2, 2, 0, 10,
    10, 11, 0, 5, 2, 2, 13, 0, 11, 10, 10, 0, 2, 2, 0, 10,
    10, 10, 11, 0, 13, 13, 0, 11, 10, 10, 10, 12, 0, 0, 12, 10,
    10, 15, 15, 1, 0, 0, 1, 15, 15, 15, 15, 15, 15, 10, 10, 10,
    15, 8, 8, 8, 1, 1, 9, 9, 9, 9, 9, 9, 8, 15, 10, 10};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "63 126 186");
  IupSetAttribute(image, "1", "152 170 188");
  IupSetAttribute(image, "2", "19 179 249");
  IupSetAttribute(image, "3", "92 178 228");
  IupSetAttribute(image, "4", "140 215 252");
  IupSetAttribute(image, "5", "80 202 252");
  IupSetAttribute(image, "6", "77 152 211");
  IupSetAttribute(image, "7", "114 187 237");
  IupSetAttribute(image, "8", "212 221 228");
  IupSetAttribute(image, "9", "196 204 210");
  IupSetAttribute(image, "10", "BGCOLOR");
  IupSetAttribute(image, "11", "147 202 240");
  IupSetAttribute(image, "12", "101 160 211");
  IupSetAttribute(image, "13", "45 184 246");
  IupSetAttribute(image, "14", "124 201 245");
  IupSetAttribute(image, "15", "230 231 230");

  return image;
}

static Ihandle* load_image_IMAGE_String(void)
{
  unsigned char imgdata[] = {
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 7, 4, 1, 8, 8, 8, 1, 4, 7, 8, 8, 8,
    8, 8, 8, 8, 4, 1, 6, 8, 8, 8, 6, 1, 4, 8, 8, 8,
    8, 8, 8, 8, 2, 1, 8, 8, 8, 8, 8, 1, 2, 8, 8, 8,
    8, 8, 8, 8, 2, 1, 8, 8, 8, 8, 8, 1, 2, 8, 8, 8,
    8, 8, 8, 8, 1, 5, 8, 8, 8, 8, 8, 5, 1, 8, 8, 8,
    8, 8, 8, 0, 3, 8, 8, 8, 8, 8, 8, 8, 3, 0, 8, 8,
    8, 8, 8, 8, 0, 5, 8, 8, 8, 8, 8, 5, 0, 8, 8, 8,
    8, 8, 8, 8, 2, 0, 8, 8, 8, 8, 8, 0, 2, 8, 8, 8,
    8, 8, 8, 8, 2, 0, 8, 8, 8, 8, 8, 0, 2, 8, 8, 8,
    8, 8, 8, 8, 4, 0, 7, 8, 8, 8, 7, 0, 4, 8, 8, 8,
    8, 8, 8, 8, 7, 4, 1, 8, 8, 8, 1, 4, 7, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "8 8 8");
  IupSetAttribute(image, "1", "49 49 49");
  IupSetAttribute(image, "2", "74 74 74");
  IupSetAttribute(image, "3", "99 99 99");
  IupSetAttribute(image, "4", "140 140 140");
  IupSetAttribute(image, "5", "189 189 189");
  IupSetAttribute(image, "6", "222 222 222");
  IupSetAttribute(image, "7", "239 239 239");
  IupSetAttribute(image, "8", "BGCOLOR");

  return image;
}

static Ihandle* load_image_ui_menu_blue_WW(void)
{
  unsigned char imgdata[] = {
    3, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 3,
    7, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 7,
    7, 14, 5, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 7,
    7, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 7,
    9, 1, 1, 1, 1, 8, 8, 8, 8, 4, 4, 4, 4, 4, 4, 10,
    9, 1, 14, 1, 1, 8, 8, 8, 8, 4, 4, 4, 4, 4, 4, 10,
    9, 1, 1, 1, 1, 8, 8, 8, 8, 4, 4, 4, 4, 4, 4, 10,
    2, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 2,
    2, 6, 11, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 2,
    2, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    15, 6, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 6, 15,
    12, 13, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 13, 12,
    5, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 5,
    12, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 12,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "92 90 92");
  IupSetAttribute(image, "1", "100 176 236");
  IupSetAttribute(image, "2", "181 180 181");
  IupSetAttribute(image, "3", "227 225 227");
  IupSetAttribute(image, "4", "77 153 213");
  IupSetAttribute(image, "5", "143 144 143");
  IupSetAttribute(image, "6", "239 240 239");
  IupSetAttribute(image, "7", "203 204 203");
  IupSetAttribute(image, "8", "90 166 226");
  IupSetAttribute(image, "9", "148 177 196");
  IupSetAttribute(image, "10", "137 165 185");
  IupSetAttribute(image, "11", "116 114 116");
  IupSetAttribute(image, "12", "168 168 168");
  IupSetAttribute(image, "13", "BGCOLOR");
  IupSetAttribute(image, "14", "252 254 252");
  IupSetAttribute(image, "15", "172 174 172");

  return image;
}

static Ihandle* load_image_settings_WW(void)
{
  unsigned char imgdata[] = {
    11, 11, 11, 10, 11, 10, 10, 10, 10, 10, 10, 11, 10, 11, 11, 11,
    11, 11, 10, 10, 10, 10, 8, 0, 0, 8, 10, 10, 10, 10, 11, 11,
    11, 10, 10, 6, 7, 10, 6, 0, 0, 6, 10, 7, 6, 10, 10, 11,
    10, 10, 6, 0, 0, 4, 2, 0, 0, 1, 3, 0, 0, 6, 10, 10,
    11, 10, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 10, 11,
    10, 10, 10, 3, 0, 0, 0, 1, 1, 0, 0, 0, 4, 10, 10, 10,
    10, 8, 6, 1, 0, 0, 5, 10, 10, 5, 0, 0, 2, 6, 8, 10,
    10, 0, 0, 0, 0, 1, 10, 10, 10, 10, 1, 0, 0, 0, 0, 9,
    10, 0, 0, 0, 0, 1, 10, 10, 10, 10, 1, 0, 0, 0, 0, 9,
    10, 8, 6, 2, 0, 0, 5, 10, 10, 5, 0, 0, 1, 6, 8, 10,
    10, 10, 10, 4, 0, 0, 0, 1, 1, 0, 0, 0, 3, 10, 10, 10,
    11, 10, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 10, 11,
    10, 10, 6, 0, 0, 3, 1, 0, 0, 2, 4, 0, 0, 6, 10, 10,
    11, 10, 10, 6, 7, 10, 6, 0, 0, 6, 10, 7, 6, 10, 10, 11,
    11, 11, 10, 10, 10, 10, 8, 0, 0, 8, 10, 10, 10, 10, 11, 11,
    11, 11, 11, 10, 11, 10, 10, 10, 10, 10, 10, 11, 10, 11, 11, 11};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "66 66 66");
  IupSetAttribute(image, "1", "74 74 74");
  IupSetAttribute(image, "2", "82 82 82");
  IupSetAttribute(image, "3", "107 107 107");
  IupSetAttribute(image, "4", "115 115 115");
  IupSetAttribute(image, "5", "156 156 156");
  IupSetAttribute(image, "6", "165 165 165");
  IupSetAttribute(image, "7", "181 181 181");
  IupSetAttribute(image, "8", "198 198 198");
  IupSetAttribute(image, "9", "239 239 239");
  IupSetAttribute(image, "10", "247 247 247");
  IupSetAttribute(image, "11", "BGCOLOR");
  IupSetAttribute(image, "12", "255 255 255");
  IupSetAttribute(image, "13", "255 255 255");
  IupSetAttribute(image, "14", "255 255 255");
  IupSetAttribute(image, "15", "255 255 255");

  return image;
}

static Ihandle* load_image_edit_lowercase_WW(void)
{
  unsigned char imgdata[] = {
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 7, 5, 6, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 1, 15, 6, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 2, 7, 8, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 7, 2, 3, 1, 14, 7, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 1, 14, 14, 14, 14, 1, 4, 3, 6, 6, 6, 15, 4, 4,
    4, 4, 6, 3, 4, 4, 5, 6, 4, 5, 5, 4, 8, 6, 4, 4,
    4, 6, 0, 6, 4, 9, 0, 0, 8, 4, 4, 9, 5, 6, 4, 4,
    4, 4, 4, 3, 4, 4, 4, 4, 4, 7, 2, 2, 5, 6, 4, 4,
    4, 4, 4, 13, 7, 4, 11, 3, 4, 6, 8, 4, 1, 14, 4, 4,
    4, 4, 4, 7, 12, 11, 10, 10, 4, 14, 8, 4, 5, 14, 7, 4,
    4, 4, 4, 4, 4, 4, 11, 9, 4, 8, 0, 0, 1, 6, 6, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 7, 7, 7, 7, 7, 7, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "31 32 31");
  IupSetAttribute(image, "1", "154 155 154");
  IupSetAttribute(image, "2", "94 94 94");
  IupSetAttribute(image, "3", "210 213 226");
  IupSetAttribute(image, "4", "BGCOLOR");
  IupSetAttribute(image, "5", "114 114 114");
  IupSetAttribute(image, "6", "78 79 78");
  IupSetAttribute(image, "7", "224 226 227");
  IupSetAttribute(image, "8", "132 133 132");
  IupSetAttribute(image, "9", "191 190 193");
  IupSetAttribute(image, "10", "64 92 208");
  IupSetAttribute(image, "11", "39 74 183");
  IupSetAttribute(image, "12", "100 130 212");
  IupSetAttribute(image, "13", "148 170 236");
  IupSetAttribute(image, "14", "57 57 57");
  IupSetAttribute(image, "15", "168 168 168");

  return image;
}

static Ihandle* load_image_IMAGE_AlignToGridHS(void)
{
  unsigned char imgdata[] = {
    8, 8, 8, 7, 8, 2, 2, 2, 8, 8, 8, 6, 8, 8, 8, 8,
    8, 8, 8, 7, 8, 8, 2, 8, 8, 8, 8, 6, 8, 8, 8, 8,
    8, 8, 8, 7, 8, 8, 8, 8, 8, 8, 8, 6, 8, 8, 8, 8,
    7, 7, 7, 3, 3, 3, 3, 3, 3, 1, 6, 6, 6, 6, 6, 8,
    8, 8, 8, 3, 15, 15, 15, 14, 11, 1, 8, 6, 8, 8, 8, 8,
    2, 8, 8, 3, 15, 15, 13, 13, 9, 1, 8, 6, 8, 8, 8, 8,
    2, 2, 8, 3, 15, 13, 13, 12, 9, 0, 8, 5, 8, 8, 8, 8,
    2, 8, 8, 3, 13, 13, 12, 12, 9, 0, 8, 5, 8, 8, 8, 8,
    8, 8, 8, 3, 10, 9, 9, 9, 7, 0, 8, 5, 8, 8, 8, 8,
    8, 8, 8, 1, 1, 1, 1, 0, 0, 0, 8, 4, 8, 8, 8, 8,
    8, 8, 8, 6, 8, 8, 8, 8, 8, 8, 8, 4, 8, 8, 8, 8,
    6, 6, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 8,
    8, 8, 8, 6, 8, 8, 8, 8, 8, 8, 8, 4, 8, 8, 8, 8,
    8, 8, 8, 5, 8, 8, 8, 8, 8, 8, 8, 4, 8, 8, 8, 8,
    8, 8, 8, 5, 8, 8, 8, 8, 8, 8, 8, 4, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "13 13 19");
  IupSetAttribute(image, "1", "42 49 54");
  IupSetAttribute(image, "2", "41 74 144");
  IupSetAttribute(image, "3", "117 123 140");
  IupSetAttribute(image, "4", "45 90 165");
  IupSetAttribute(image, "5", "62 108 182");
  IupSetAttribute(image, "6", "83 130 202");
  IupSetAttribute(image, "7", "128 159 207");
  IupSetAttribute(image, "8", "BGCOLOR");
  IupSetAttribute(image, "9", "156 173 206");
  IupSetAttribute(image, "10", "173 189 214");
  IupSetAttribute(image, "11", "181 189 214");
  IupSetAttribute(image, "12", "206 222 247");
  IupSetAttribute(image, "13", "218 226 247");
  IupSetAttribute(image, "14", "222 239 255");
  IupSetAttribute(image, "15", "236 241 255");

  return image;
}

static Ihandle* load_image_navigation_WW(void)
{
  unsigned char imgdata[] = {
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 15, 1, 5, 13, 13, 5, 1, 15, 9, 9, 9, 9,
    9, 9, 9, 11, 7, 7, 10, 10, 10, 10, 7, 7, 11, 9, 9, 9,
    9, 9, 11, 13, 10, 5, 5, 7, 7, 5, 5, 10, 13, 11, 9, 9,
    9, 15, 7, 10, 7, 7, 2, 9, 9, 7, 7, 7, 5, 7, 15, 9,
    9, 1, 7, 7, 2, 2, 13, 4, 9, 9, 2, 2, 7, 2, 1, 9,
    9, 5, 5, 13, 13, 13, 13, 0, 11, 15, 15, 0, 0, 13, 5, 9,
    9, 6, 7, 13, 9, 15, 4, 4, 15, 15, 15, 15, 0, 0, 6, 9,
    9, 6, 7, 3, 4, 4, 4, 15, 15, 15, 15, 15, 0, 0, 6, 9,
    9, 5, 2, 3, 0, 0, 0, 0, 11, 15, 15, 0, 3, 3, 5, 9,
    9, 1, 6, 3, 3, 3, 3, 11, 15, 15, 0, 3, 3, 14, 1, 9,
    9, 15, 6, 13, 2, 3, 3, 15, 15, 0, 3, 3, 3, 6, 15, 9,
    9, 9, 11, 14, 2, 2, 2, 3, 3, 2, 2, 3, 14, 11, 9, 9,
    9, 9, 9, 11, 14, 14, 2, 2, 2, 2, 14, 14, 11, 9, 9, 9,
    9, 9, 15, 4, 11, 12, 6, 8, 8, 6, 12, 11, 4, 15, 9, 9,
    9, 9, 9, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 9, 9, 9};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "43 79 188");
  IupSetAttribute(image, "1", "156 169 220");
  IupSetAttribute(image, "2", "70 112 225");
  IupSetAttribute(image, "3", "56 98 217");
  IupSetAttribute(image, "4", "211 213 215");
  IupSetAttribute(image, "5", "100 127 214");
  IupSetAttribute(image, "6", "76 94 178");
  IupSetAttribute(image, "7", "88 121 219");
  IupSetAttribute(image, "8", "60 66 148");
  IupSetAttribute(image, "9", "BGCOLOR");
  IupSetAttribute(image, "10", "110 145 238");
  IupSetAttribute(image, "11", "185 192 221");
  IupSetAttribute(image, "12", "132 130 172");
  IupSetAttribute(image, "13", "71 104 205");
  IupSetAttribute(image, "14", "67 84 185");
  IupSetAttribute(image, "15", "227 228 227");

  return image;
}

static Ihandle* load_image_key_WW(void)
{
  unsigned char imgdata[] = {
    7, 7, 7, 7, 7, 10, 4, 13, 13, 4, 10, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 2, 13, 15, 15, 4, 4, 1, 2, 7, 7, 7, 7,
    7, 7, 7, 10, 1, 2, 1, 8, 8, 8, 12, 1, 10, 7, 7, 7,
    7, 7, 7, 13, 2, 1, 13, 10, 10, 13, 8, 12, 13, 7, 7, 7,
    7, 7, 7, 14, 2, 8, 8, 8, 8, 8, 8, 12, 14, 7, 7, 7,
    7, 7, 7, 6, 2, 2, 2, 4, 1, 4, 12, 12, 6, 7, 7, 7,
    7, 7, 7, 11, 1, 2, 4, 1, 8, 14, 12, 14, 11, 7, 7, 7,
    7, 7, 7, 7, 13, 14, 4, 13, 14, 1, 14, 13, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 11, 5, 4, 1, 5, 11, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 5, 4, 1, 5, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 9, 4, 9, 10, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 9, 4, 4, 9, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 5, 4, 5, 10, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 9, 4, 4, 9, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 9, 4, 9, 3, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 10, 6, 0, 6, 11, 10, 10, 7, 7, 7, 7};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "132 98 68");
  IupSetAttribute(image, "1", "215 184 107");
  IupSetAttribute(image, "2", "245 222 160");
  IupSetAttribute(image, "3", "196 186 180");
  IupSetAttribute(image, "4", "229 204 132");
  IupSetAttribute(image, "5", "167 139 81");
  IupSetAttribute(image, "6", "188 173 142");
  IupSetAttribute(image, "7", "BGCOLOR");
  IupSetAttribute(image, "8", "194 167 64");
  IupSetAttribute(image, "9", "160 131 82");
  IupSetAttribute(image, "10", "237 229 216");
  IupSetAttribute(image, "11", "218 209 190");
  IupSetAttribute(image, "12", "246 207 111");
  IupSetAttribute(image, "13", "215 189 133");
  IupSetAttribute(image, "14", "196 168 99");
  IupSetAttribute(image, "15", "236 214 140");

  return image;
}

static Ihandle* load_image_Tree_String_WW(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    5, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 5,
    8, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8,
    8, 7, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 7, 8,
    8, 7, 3, 3, 3, 3, 3, 3, 9, 3, 3, 3, 3, 3, 7, 8,
    8, 7, 3, 15, 9, 9, 10, 3, 9, 3, 3, 3, 3, 3, 7, 8,
    8, 7, 3, 3, 3, 13, 9, 3, 9, 9, 9, 11, 3, 3, 7, 8,
    4, 7, 3, 6, 9, 9, 9, 3, 9, 3, 10, 9, 3, 3, 7, 4,
    4, 7, 3, 9, 11, 3, 9, 3, 9, 3, 10, 9, 3, 3, 7, 4,
    4, 7, 3, 10, 9, 9, 9, 3, 9, 9, 9, 15, 3, 3, 7, 4,
    4, 7, 3, 3, 3, 3, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4,
    4, 7, 3, 3, 3, 3, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4,
    4, 3, 7, 3, 3, 7, 7, 2, 12, 4, 4, 1, 13, 13, 13, 4,
    12, 4, 2, 7, 7, 13, 8, 1, 5, 2, 0, 0, 12, 12, 4, 12,
    0, 5, 4, 4, 4, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "BGCOLOR");
  IupSetAttribute(image, "1", "128 130 140");
  IupSetAttribute(image, "2", "200 200 200");
  IupSetAttribute(image, "3", "226 230 231");
  IupSetAttribute(image, "4", "89 96 114");
  IupSetAttribute(image, "5", "170 171 170");
  IupSetAttribute(image, "6", "188 126 124");
  IupSetAttribute(image, "7", "250 252 251");
  IupSetAttribute(image, "8", "106 113 136");
  IupSetAttribute(image, "9", "132 2 4");
  IupSetAttribute(image, "10", "196 135 136");
  IupSetAttribute(image, "11", "212 184 184");
  IupSetAttribute(image, "12", "148 152 156");
  IupSetAttribute(image, "13", "210 212 214");
  IupSetAttribute(image, "14", "117 122 147");
  IupSetAttribute(image, "15", "220 198 200");

  return image;
}

static Ihandle* load_image_IMAGE_Function(void)
{
  unsigned char imgdata[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "0 0 0");
  IupSetAttribute(image, "1", "BGCOLOR");

  return image;
}

static Ihandle* load_image_IMAGE_Trigger(void)
{
  unsigned char imgdata[] = {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 15, 12, 12, 12, 12, 8, 8, 13, 7, 7,
    7, 7, 7, 7, 7, 7, 9, 13, 11, 12, 10, 8, 1, 15, 7, 7,
    7, 7, 7, 7, 7, 14, 9, 14, 11, 10, 8, 4, 14, 7, 7, 7,
    7, 7, 7, 7, 7, 9, 13, 11, 10, 8, 5, 14, 7, 7, 7, 7,
    7, 7, 7, 7, 14, 9, 14, 10, 8, 6, 14, 15, 7, 7, 7, 7,
    7, 7, 7, 7, 9, 11, 11, 12, 12, 10, 8, 1, 14, 7, 7, 7,
    7, 7, 7, 7, 8, 6, 4, 6, 11, 8, 0, 14, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 15, 12, 11, 8, 2, 14, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 13, 11, 8, 3, 14, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 14, 12, 8, 3, 14, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 13, 12, 3, 14, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 14, 8, 14, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "107 74 41");
  IupSetAttribute(image, "1", "90 74 61");
  IupSetAttribute(image, "2", "131 107 49");
  IupSetAttribute(image, "3", "159 120 27");
  IupSetAttribute(image, "4", "107 99 74");
  IupSetAttribute(image, "5", "140 123 74");
  IupSetAttribute(image, "6", "148 132 82");
  IupSetAttribute(image, "7", "BGCOLOR");
  IupSetAttribute(image, "8", "184 153 106");
  IupSetAttribute(image, "9", "231 206 90");
  IupSetAttribute(image, "10", "243 186 118");
  IupSetAttribute(image, "11", "255 231 123");
  IupSetAttribute(image, "12", "228 195 141");
  IupSetAttribute(image, "13", "225 206 172");
  IupSetAttribute(image, "14", "239 233 217");
  IupSetAttribute(image, "15", "245 242 239");

  return image;
}

static Ihandle* load_image_IMAGE_Pin(void)
{
  unsigned char imgdata[] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 10, 5, 4, 3, 11, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 7, 1, 7, 4, 3, 13, 14,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 9, 0, 6, 7, 3, 1, 10,
    15, 15, 15, 15, 15, 15, 15, 15, 11, 3, 7, 2, 4, 7, 3, 1,
    15, 15, 15, 15, 13, 10, 9, 9, 3, 7, 6, 4, 0, 3, 6, 0,
    14, 7, 6, 4, 2, 2, 2, 3, 7, 6, 4, 2, 0, 0, 0, 8,
    13, 3, 6, 7, 7, 6, 4, 7, 6, 4, 2, 0, 3, 10, 15, 15,
    15, 5, 4, 6, 12, 7, 4, 2, 4, 2, 1, 1, 9, 15, 15, 15,
    15, 9, 3, 6, 7, 7, 6, 4, 1, 1, 1, 5, 15, 15, 15, 15,
    15, 14, 5, 2, 6, 6, 7, 4, 3, 1, 1, 5, 15, 15, 15, 15,
    15, 15, 13, 3, 2, 4, 4, 6, 4, 2, 0, 8, 15, 15, 15, 15,
    15, 15, 11, 5, 1, 0, 3, 4, 3, 2, 0, 10, 15, 15, 15, 15,
    15, 11, 8, 1, 5, 5, 0, 2, 2, 0, 0, 14, 15, 15, 15, 15,
    11, 5, 1, 8, 15, 15, 5, 0, 0, 0, 5, 15, 15, 15, 15, 15,
    8, 1, 8, 15, 15, 15, 15, 11, 8, 5, 10, 15, 15, 15, 15, 15};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "10 82 141");
  IupSetAttribute(image, "1", "34 96 146");
  IupSetAttribute(image, "2", "29 113 177");
  IupSetAttribute(image, "3", "58 132 191");
  IupSetAttribute(image, "4", "76 149 204");
  IupSetAttribute(image, "5", "144 171 189");
  IupSetAttribute(image, "6", "108 180 236");
  IupSetAttribute(image, "7", "165 209 243");
  IupSetAttribute(image, "8", "197 203 207");
  IupSetAttribute(image, "9", "208 222 235");
  IupSetAttribute(image, "10", "226 239 243");
  IupSetAttribute(image, "11", "239 243 247");
  IupSetAttribute(image, "12", "189 222 255");
  IupSetAttribute(image, "13", "243 247 255");
  IupSetAttribute(image, "14", "247 255 255");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_edit_uppercase_WW(void)
{
  unsigned char imgdata[] = {
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 3, 2, 2, 2, 12, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 5, 5, 4, 5, 15, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 12, 5, 15, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 8, 2, 2, 14, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 2, 5, 4, 1, 7, 4, 4, 4, 14, 14, 4, 4, 4, 4,
    4, 4, 13, 5, 4, 14, 13, 8, 4, 8, 14, 15, 8, 4, 4, 4,
    4, 4, 1, 0, 0, 12, 15, 2, 4, 1, 1, 15, 1, 4, 4, 4,
    4, 4, 3, 4, 4, 4, 4, 4, 4, 2, 8, 5, 2, 4, 4, 4,
    4, 4, 10, 8, 4, 6, 3, 4, 8, 14, 3, 1, 7, 8, 4, 4,
    4, 4, 8, 11, 6, 9, 9, 4, 1, 7, 7, 7, 7, 1, 4, 4,
    4, 4, 4, 4, 4, 6, 3, 4, 15, 8, 4, 4, 5, 15, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 2, 0, 2, 8, 12, 0, 0, 2, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 8, 8, 8, 8, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "31 32 31");
  IupSetAttribute(image, "1", "154 155 154");
  IupSetAttribute(image, "2", "88 89 88");
  IupSetAttribute(image, "3", "209 213 227");
  IupSetAttribute(image, "4", "BGCOLOR");
  IupSetAttribute(image, "5", "124 124 124");
  IupSetAttribute(image, "6", "39 74 183");
  IupSetAttribute(image, "7", "62 61 62");
  IupSetAttribute(image, "8", "224 226 227");
  IupSetAttribute(image, "9", "64 92 208");
  IupSetAttribute(image, "10", "148 170 236");
  IupSetAttribute(image, "11", "100 130 212");
  IupSetAttribute(image, "12", "182 180 182");
  IupSetAttribute(image, "13", "52 54 52");
  IupSetAttribute(image, "14", "108 107 108");
  IupSetAttribute(image, "15", "78 79 78");

  return image;
}

static Ihandle* load_image_IMAGE_RunPart(void)
{
  unsigned char imgdata[] = {
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    12, 13, 3, 12, 5, 12, 5, 12, 5, 12, 5, 12, 5, 12, 1, 12,
    13, 3, 13, 15, 15, 14, 14, 14, 14, 14, 14, 14, 14, 14, 11, 1,
    12, 3, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 11, 11, 11, 7,
    13, 3, 7, 14, 3, 13, 6, 6, 5, 5, 5, 5, 11, 11, 11, 1,
    12, 3, 3, 12, 2, 3, 13, 15, 15, 15, 15, 5, 11, 11, 11, 6,
    12, 7, 7, 2, 7, 3, 2, 12, 7, 2, 15, 1, 11, 9, 9, 1,
    7, 10, 2, 7, 3, 2, 2, 0, 13, 15, 15, 1, 9, 9, 9, 6,
    12, 11, 10, 2, 2, 0, 0, 12, 7, 2, 15, 1, 9, 9, 9, 1,
    1, 11, 11, 10, 0, 0, 12, 15, 15, 15, 15, 1, 9, 9, 9, 6,
    7, 11, 11, 11, 0, 13, 7, 2, 2, 2, 13, 1, 9, 8, 8, 1,
    1, 11, 9, 9, 12, 15, 13, 13, 13, 13, 13, 1, 8, 8, 8, 6,
    7, 9, 9, 9, 1, 1, 1, 1, 1, 1, 0, 0, 8, 8, 8, 0,
    1, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6,
    7, 1, 6, 1, 6, 1, 6, 1, 6, 1, 6, 0, 6, 0, 6, 0,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "75 98 140");
  IupSetAttribute(image, "1", "109 130 150");
  IupSetAttribute(image, "2", "63 111 207");
  IupSetAttribute(image, "3", "115 148 210");
  IupSetAttribute(image, "4", "BGCOLOR");
  IupSetAttribute(image, "5", "144 156 181");
  IupSetAttribute(image, "6", "153 167 185");
  IupSetAttribute(image, "7", "160 179 212");
  IupSetAttribute(image, "8", "255 255 115");
  IupSetAttribute(image, "9", "255 255 144");
  IupSetAttribute(image, "10", "223 229 174");
  IupSetAttribute(image, "11", "255 255 177");
  IupSetAttribute(image, "12", "190 203 221");
  IupSetAttribute(image, "13", "230 233 235");
  IupSetAttribute(image, "14", "252 255 197");
  IupSetAttribute(image, "15", "253 255 230");

  return image;
}

static Ihandle* load_image_binocular__pencil_WW(void)
{
  unsigned char imgdata[] = {
    14, 14, 14, 7, 4, 7, 14, 14, 14, 14, 7, 4, 7, 14, 14, 14,
    14, 14, 14, 4, 7, 3, 14, 14, 14, 14, 3, 7, 4, 14, 14, 14,
    14, 14, 7, 3, 3, 3, 7, 14, 14, 7, 3, 3, 3, 7, 14, 14,
    14, 14, 3, 13, 7, 7, 3, 3, 3, 3, 13, 7, 7, 3, 14, 14,
    14, 14, 3, 7, 4, 4, 12, 7, 7, 12, 7, 4, 4, 3, 14, 14,
    14, 14, 3, 7, 4, 3, 12, 12, 12, 12, 3, 4, 3, 3, 14, 14,
    14, 7, 12, 3, 12, 12, 3, 7, 7, 3, 12, 3, 12, 12, 7, 14,
    14, 3, 7, 7, 4, 3, 12, 12, 12, 12, 4, 7, 4, 6, 2, 15,
    14, 3, 7, 4, 3, 3, 12, 5, 5, 12, 7, 4, 3, 8, 9, 2,
    14, 3, 7, 4, 3, 3, 12, 14, 14, 12, 7, 7, 3, 14, 12, 15,
    14, 3, 7, 4, 3, 3, 12, 14, 14, 12, 7, 1, 11, 12, 12, 14,
    14, 12, 7, 4, 4, 4, 0, 14, 14, 3, 1, 11, 1, 4, 12, 14,
    14, 12, 3, 3, 12, 12, 0, 14, 14, 1, 11, 6, 3, 12, 12, 14,
    14, 12, 4, 4, 3, 4, 0, 14, 10, 11, 1, 6, 4, 4, 12, 14,
    5, 3, 0, 0, 0, 0, 3, 13, 14, 6, 12, 0, 0, 0, 3, 5,
    14, 14, 14, 5, 5, 5, 5, 0, 1, 5, 5, 5, 5, 14, 14, 14};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "19 20 19");
  IupSetAttribute(image, "1", "151 134 47");
  IupSetAttribute(image, "2", "184 42 40");
  IupSetAttribute(image, "3", "69 69 68");
  IupSetAttribute(image, "4", "90 90 90");
  IupSetAttribute(image, "5", "214 211 209");
  IupSetAttribute(image, "6", "120 92 76");
  IupSetAttribute(image, "7", "119 117 103");
  IupSetAttribute(image, "8", "100 62 68");
  IupSetAttribute(image, "9", "252 130 132");
  IupSetAttribute(image, "10", "196 182 76");
  IupSetAttribute(image, "11", "244 230 124");
  IupSetAttribute(image, "12", "48 46 47");
  IupSetAttribute(image, "13", "145 142 121");
  IupSetAttribute(image, "14", "BGCOLOR");
  IupSetAttribute(image, "15", "236 188 188");

  return image;
}

static Ihandle* load_image_folder_open_document_WW(void)
{
  unsigned char imgdata[] = {
    13, 13, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, 13, 13,
    13, 13, 4, 8, 4, 4, 8, 2, 2, 2, 2, 2, 2, 4, 13, 13,
    13, 13, 4, 2, 2, 2, 4, 6, 1, 1, 1, 1, 1, 14, 13, 13,
    13, 13, 4, 2, 2, 2, 2, 14, 5, 13, 13, 5, 1, 15, 5, 13,
    13, 13, 4, 2, 2, 2, 2, 14, 7, 7, 5, 5, 1, 13, 10, 5,
    13, 13, 4, 2, 2, 2, 2, 6, 7, 7, 5, 5, 12, 12, 12, 12,
    13, 13, 4, 2, 2, 2, 2, 6, 5, 7, 13, 5, 5, 5, 5, 1,
    13, 13, 14, 2, 8, 8, 2, 6, 5, 5, 13, 13, 5, 5, 5, 1,
    13, 13, 14, 2, 8, 8, 2, 6, 5, 5, 13, 13, 13, 5, 13, 1,
    13, 13, 14, 8, 8, 8, 8, 9, 5, 5, 13, 13, 13, 13, 13, 1,
    13, 13, 14, 8, 8, 8, 8, 9, 5, 5, 13, 13, 13, 13, 13, 1,
    13, 13, 6, 8, 8, 8, 8, 9, 5, 5, 13, 13, 13, 13, 13, 12,
    13, 5, 6, 8, 8, 8, 8, 9, 5, 5, 13, 13, 13, 13, 13, 12,
    5, 7, 9, 6, 4, 8, 8, 9, 5, 5, 13, 13, 13, 13, 13, 3,
    13, 13, 13, 7, 6, 6, 14, 0, 11, 3, 3, 3, 3, 3, 3, 12,
    13, 13, 13, 13, 13, 5, 14, 6, 5, 5, 5, 5, 5, 5, 5, 5};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "132 106 36");
  IupSetAttribute(image, "1", "179 186 169");
  IupSetAttribute(image, "2", "250 230 161");
  IupSetAttribute(image, "3", "125 147 148");
  IupSetAttribute(image, "4", "226 191 119");
  IupSetAttribute(image, "5", "225 229 228");
  IupSetAttribute(image, "6", "189 154 84");
  IupSetAttribute(image, "7", "216 215 208");
  IupSetAttribute(image, "8", "240 211 138");
  IupSetAttribute(image, "9", "174 138 70");
  IupSetAttribute(image, "10", "192 210 212");
  IupSetAttribute(image, "11", "120 136 136");
  IupSetAttribute(image, "12", "151 169 168");
  IupSetAttribute(image, "13", "BGCOLOR");
  IupSetAttribute(image, "14", "206 175 107");
  IupSetAttribute(image, "15", "161 157 129");

  return image;
}

static Ihandle* load_image_bookmark_WW(void)
{
  unsigned char imgdata[] = {
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 9, 8, 9, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 9, 8, 14, 8, 9, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 9, 4, 14, 6, 14, 4, 9,
    12, 12, 12, 12, 12, 12, 12, 12, 9, 4, 14, 13, 13, 6, 14, 4,
    12, 12, 12, 12, 12, 12, 12, 9, 4, 6, 13, 13, 13, 14, 4, 9,
    12, 12, 12, 12, 12, 12, 9, 4, 6, 13, 13, 13, 14, 4, 9, 12,
    12, 12, 12, 12, 12, 9, 4, 6, 2, 2, 13, 6, 4, 9, 12, 12,
    12, 12, 12, 12, 9, 4, 6, 2, 2, 2, 6, 4, 9, 12, 12, 12,
    12, 12, 12, 9, 4, 13, 2, 2, 2, 6, 4, 9, 12, 12, 12, 12,
    12, 12, 9, 4, 13, 8, 2, 2, 6, 4, 9, 12, 12, 12, 12, 12,
    12, 9, 10, 13, 8, 8, 8, 13, 10, 9, 12, 12, 12, 12, 12, 12,
    9, 10, 2, 2, 8, 8, 13, 10, 9, 12, 12, 12, 12, 12, 12, 12,
    10, 10, 10, 10, 2, 13, 10, 9, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 10, 2, 10, 9, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 5, 11, 0, 0, 15, 5, 5, 5, 5, 5, 5, 12, 12, 12, 12,
    12, 11, 7, 0, 1, 3, 3, 3, 11, 11, 11, 5, 5, 5, 12, 12};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "185 57 57");
  IupSetAttribute(image, "1", "188 158 156");
  IupSetAttribute(image, "2", "248 102 103");
  IupSetAttribute(image, "3", "215 214 215");
  IupSetAttribute(image, "4", "227 79 79");
  IupSetAttribute(image, "5", "240 240 240");
  IupSetAttribute(image, "6", "252 121 121");
  IupSetAttribute(image, "7", "204 206 204");
  IupSetAttribute(image, "8", "241 91 89");
  IupSetAttribute(image, "9", "244 193 193");
  IupSetAttribute(image, "10", "211 70 69");
  IupSetAttribute(image, "11", "226 227 226");
  IupSetAttribute(image, "12", "BGCOLOR");
  IupSetAttribute(image, "13", "252 112 111");
  IupSetAttribute(image, "14", "252 133 132");
  IupSetAttribute(image, "15", "204 166 164");

  return image;
}

static Ihandle* load_image_IMAGE_AddDocument(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 6, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 8, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 9, 8, 5, 4, 4, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 5, 4, 2, 2, 0,
    0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 4, 2, 0, 0, 0,
    1, 12, 12, 12, 12, 12, 12, 1, 0, 0, 0, 4, 2, 0, 0, 0,
    1, 12, 12, 12, 12, 12, 12, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 12, 12, 12, 12, 12, 12, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 11, 11, 11, 11, 11, 11, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    1, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    1, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    1, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "BGCOLOR");
  IupSetAttribute(image, "1", "41 82 181");
  IupSetAttribute(image, "2", "66 123 8");
  IupSetAttribute(image, "3", "74 132 8");
  IupSetAttribute(image, "4", "74 132 16");
  IupSetAttribute(image, "5", "82 140 16");
  IupSetAttribute(image, "6", "82 148 24");
  IupSetAttribute(image, "7", "90 148 24");
  IupSetAttribute(image, "8", "90 148 33");
  IupSetAttribute(image, "9", "99 156 41");
  IupSetAttribute(image, "10", "165 181 222");
  IupSetAttribute(image, "11", "231 239 247");
  IupSetAttribute(image, "12", "239 247 255");
  IupSetAttribute(image, "13", "247 247 255");
  IupSetAttribute(image, "14", "255 255 255");

  return image;
}

static Ihandle* load_image_IMAGE_Sub(void)
{
  unsigned char imgdata[] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 13, 3, 14, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 13, 3, 6, 3, 14, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 11, 3, 7, 7, 6, 3, 13, 15,
    15, 15, 15, 10, 8, 4, 4, 15, 3, 6, 7, 6, 6, 6, 3, 14,
    15, 15, 15, 15, 15, 15, 15, 15, 1, 2, 6, 7, 6, 6, 6, 1,
    15, 10, 9, 8, 8, 4, 4, 15, 1, 3, 2, 6, 7, 6, 1, 0,
    15, 15, 15, 15, 15, 15, 15, 15, 1, 3, 3, 2, 6, 1, 1, 0,
    15, 15, 10, 9, 8, 4, 4, 15, 11, 1, 3, 3, 0, 1, 0, 0,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 11, 1, 2, 0, 1, 0, 11,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 11, 1, 0, 0, 11, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 11, 0, 11, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "124 44 126");
  IupSetAttribute(image, "1", "155 71 156");
  IupSetAttribute(image, "2", "197 103 197");
  IupSetAttribute(image, "3", "203 120 203");
  IupSetAttribute(image, "4", "156 167 173");
  IupSetAttribute(image, "5", "211 179 200");
  IupSetAttribute(image, "6", "239 152 239");
  IupSetAttribute(image, "7", "247 170 247");
  IupSetAttribute(image, "8", "214 214 173");
  IupSetAttribute(image, "9", "235 231 173");
  IupSetAttribute(image, "10", "247 247 173");
  IupSetAttribute(image, "11", "236 214 236");
  IupSetAttribute(image, "12", "247 214 247");
  IupSetAttribute(image, "13", "247 222 247");
  IupSetAttribute(image, "14", "247 231 247");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_edit_symbol_WW(void)
{
  unsigned char imgdata[] = {
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 14, 10, 10, 14, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 4, 12, 3, 11, 11, 3, 12, 4, 8, 8, 8, 8,
    8, 8, 8, 8, 12, 13, 4, 8, 8, 4, 13, 12, 8, 8, 8, 8,
    8, 8, 8, 8, 11, 1, 8, 8, 8, 8, 1, 11, 8, 8, 8, 8,
    8, 8, 8, 8, 3, 6, 8, 8, 8, 8, 6, 3, 8, 8, 8, 8,
    8, 8, 8, 8, 7, 5, 7, 8, 8, 7, 5, 7, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 15, 9, 8, 8, 9, 15, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 1, 2, 9, 8, 8, 9, 2, 1, 8, 8, 8, 8,
    8, 8, 8, 14, 9, 0, 0, 14, 14, 0, 0, 9, 4, 8, 8, 8,
    8, 8, 8, 14, 4, 10, 10, 10, 10, 10, 10, 4, 4, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "28 30 28");
  IupSetAttribute(image, "1", "148 148 148");
  IupSetAttribute(image, "2", "212 210 212");
  IupSetAttribute(image, "3", "96 96 96");
  IupSetAttribute(image, "4", "236 236 236");
  IupSetAttribute(image, "5", "76 74 76");
  IupSetAttribute(image, "6", "132 130 132");
  IupSetAttribute(image, "7", "184 184 184");
  IupSetAttribute(image, "8", "BGCOLOR");
  IupSetAttribute(image, "9", "57 55 57");
  IupSetAttribute(image, "10", "228 229 228");
  IupSetAttribute(image, "11", "84 84 84");
  IupSetAttribute(image, "12", "140 140 140");
  IupSetAttribute(image, "13", "100 102 100");
  IupSetAttribute(image, "14", "244 242 244");
  IupSetAttribute(image, "15", "188 190 188");

  return image;
}

static Ihandle* load_image_cross_script_WW(void)
{
  unsigned char imgdata[] = {
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 9, 7, 7, 11, 6, 6, 6, 6, 6, 12, 10, 2, 13, 6,
    6, 6, 15, 10, 10, 5, 15, 6, 6, 6, 11, 7, 7, 9, 6, 6,
    6, 6, 13, 15, 2, 5, 5, 7, 4, 11, 7, 2, 12, 6, 6, 6,
    6, 6, 6, 4, 11, 15, 5, 5, 7, 7, 2, 12, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 4, 15, 7, 7, 15, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 12, 2, 15, 3, 15, 15, 15, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 11, 15, 3, 15, 13, 2, 15, 8, 6, 6, 6, 6,
    6, 6, 6, 1, 15, 14, 15, 1, 6, 6, 11, 15, 11, 6, 6, 6,
    6, 6, 12, 0, 14, 3, 8, 6, 6, 6, 6, 11, 8, 9, 6, 6,
    6, 6, 11, 3, 14, 0, 4, 6, 6, 6, 6, 6, 11, 11, 6, 6,
    6, 6, 0, 14, 3, 11, 6, 6, 6, 6, 6, 6, 6, 1, 4, 6,
    6, 12, 1, 0, 8, 9, 12, 12, 6, 6, 12, 12, 12, 9, 9, 6,
    6, 6, 12, 12, 12, 12, 12, 6, 6, 6, 6, 6, 6, 12, 12, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "132 49 50");
  IupSetAttribute(image, "1", "188 149 150");
  IupSetAttribute(image, "2", "183 79 78");
  IupSetAttribute(image, "3", "186 33 32");
  IupSetAttribute(image, "4", "227 205 203");
  IupSetAttribute(image, "5", "224 87 88");
  IupSetAttribute(image, "6", "BGCOLOR");
  IupSetAttribute(image, "7", "189 59 60");
  IupSetAttribute(image, "8", "150 71 72");
  IupSetAttribute(image, "9", "214 184 183");
  IupSetAttribute(image, "10", "236 119 119");
  IupSetAttribute(image, "11", "185 116 115");
  IupSetAttribute(image, "12", "234 223 223");
  IupSetAttribute(image, "13", "215 155 156");
  IupSetAttribute(image, "14", "214 21 22");
  IupSetAttribute(image, "15", "166 47 48");

  return image;
}

static Ihandle* load_image_Tree_Binary_WW(void)
{
  unsigned char imgdata[] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 4, 4,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 5, 2, 4,
    2, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 2, 3, 3, 3,
    2, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 5, 3,
    2, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 5, 3,
    2, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 5, 3,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 3,
    2, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 5, 3,
    2, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 5, 3,
    2, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 5, 3,
    2, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 5, 3,
    2, 5, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 5, 3,
    2, 3, 3, 5, 5, 5, 5, 2, 3, 3, 3, 3, 2, 0, 5, 3,
    4, 4, 4, 2, 3, 3, 3, 3, 4, 4, 4, 4, 3, 2, 5, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "255 255 255");
  IupSetAttribute(image, "1", "0 0 255");
  IupSetAttribute(image, "2", "128 128 128");
  IupSetAttribute(image, "3", "0 0 0");
  IupSetAttribute(image, "4", "BGCOLOR");
  IupSetAttribute(image, "5", "192 192 192");
  IupSetAttribute(image, "6", "0 0 0");
  IupSetAttribute(image, "7", "0 0 0");
  IupSetAttribute(image, "8", "0 0 0");
  IupSetAttribute(image, "9", "0 0 0");
  IupSetAttribute(image, "10", "0 0 0");
  IupSetAttribute(image, "11", "0 0 0");
  IupSetAttribute(image, "12", "0 0 0");
  IupSetAttribute(image, "13", "0 0 0");
  IupSetAttribute(image, "14", "0 0 0");
  IupSetAttribute(image, "15", "0 0 0");

  return image;
}

static Ihandle* load_image_clipboard_list_WW(void)
{
  unsigned char imgdata[] = {
    7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7,
    7, 7, 11, 13, 13, 6, 11, 3, 3, 11, 6, 13, 13, 11, 7, 7,
    7, 7, 13, 14, 13, 8, 8, 3, 3, 8, 8, 13, 14, 13, 7, 7,
    7, 7, 13, 7, 2, 12, 12, 2, 2, 2, 12, 2, 7, 13, 7, 7,
    7, 7, 13, 7, 7, 3, 3, 3, 3, 3, 3, 7, 7, 13, 7, 7,
    7, 7, 13, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 13, 7, 7,
    7, 7, 13, 7, 1, 7, 8, 3, 3, 3, 7, 7, 7, 13, 7, 7,
    7, 7, 13, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 13, 7, 7,
    7, 7, 13, 7, 9, 4, 8, 3, 8, 3, 3, 3, 7, 13, 7, 7,
    7, 7, 6, 7, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 7, 7,
    7, 7, 6, 4, 10, 4, 8, 8, 8, 3, 4, 4, 4, 6, 7, 7,
    7, 7, 15, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 15, 7, 7,
    7, 7, 15, 4, 5, 4, 8, 8, 8, 8, 8, 4, 4, 15, 7, 7,
    7, 7, 15, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 15, 7, 7,
    7, 4, 0, 7, 4, 4, 4, 4, 4, 4, 4, 4, 7, 0, 4, 7,
    4, 3, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 3, 4};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "109 59 52");
  IupSetAttribute(image, "1", "52 170 36");
  IupSetAttribute(image, "2", "164 164 166");
  IupSetAttribute(image, "3", "210 219 220");
  IupSetAttribute(image, "4", "229 240 240");
  IupSetAttribute(image, "5", "116 118 116");
  IupSetAttribute(image, "6", "140 89 65");
  IupSetAttribute(image, "7", "BGCOLOR");
  IupSetAttribute(image, "8", "191 199 203");
  IupSetAttribute(image, "9", "196 114 108");
  IupSetAttribute(image, "10", "100 154 252");
  IupSetAttribute(image, "11", "180 138 112");
  IupSetAttribute(image, "12", "148 146 148");
  IupSetAttribute(image, "13", "164 98 51");
  IupSetAttribute(image, "14", "220 150 100");
  IupSetAttribute(image, "15", "137 78 52");

  return image;
}

static Ihandle* load_image_bookmark__arrow_WW(void)
{
  unsigned char imgdata[] = {
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 9, 7, 9, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 9, 7, 15, 7, 9, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 9, 7, 15, 15, 15, 7, 9,
    14, 14, 14, 14, 14, 14, 14, 14, 9, 7, 15, 5, 5, 15, 15, 7,
    14, 14, 14, 14, 14, 14, 14, 9, 7, 15, 5, 5, 5, 15, 7, 9,
    14, 14, 14, 14, 14, 14, 9, 7, 15, 5, 5, 5, 15, 7, 9, 14,
    14, 14, 14, 14, 14, 9, 7, 15, 5, 5, 5, 15, 7, 9, 14, 14,
    14, 14, 14, 14, 9, 7, 15, 5, 5, 5, 15, 7, 9, 14, 14, 14,
    14, 14, 14, 9, 7, 5, 5, 5, 5, 15, 7, 9, 14, 14, 14, 14,
    14, 14, 9, 7, 5, 7, 5, 5, 15, 7, 9, 14, 12, 4, 14, 14,
    14, 9, 3, 5, 7, 7, 7, 5, 3, 9, 14, 14, 1, 1, 4, 14,
    9, 3, 5, 5, 7, 7, 5, 3, 13, 1, 1, 1, 1, 10, 1, 4,
    3, 3, 3, 3, 5, 5, 3, 9, 0, 10, 10, 8, 8, 8, 10, 0,
    14, 14, 14, 3, 5, 3, 9, 14, 1, 0, 0, 0, 0, 8, 0, 11,
    14, 14, 6, 3, 3, 2, 6, 6, 6, 14, 14, 14, 0, 0, 12, 14,
    14, 6, 4, 3, 2, 4, 4, 6, 6, 6, 6, 6, 13, 12, 14, 14};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "68 137 194");
  IupSetAttribute(image, "1", "97 162 221");
  IupSetAttribute(image, "2", "196 162 160");
  IupSetAttribute(image, "3", "205 67 67");
  IupSetAttribute(image, "4", "194 214 230");
  IupSetAttribute(image, "5", "250 107 107");
  IupSetAttribute(image, "6", "227 227 227");
  IupSetAttribute(image, "7", "232 83 83");
  IupSetAttribute(image, "8", "84 207 252");
  IupSetAttribute(image, "9", "244 193 194");
  IupSetAttribute(image, "10", "130 222 252");
  IupSetAttribute(image, "11", "180 198 212");
  IupSetAttribute(image, "12", "148 186 224");
  IupSetAttribute(image, "13", "112 132 172");
  IupSetAttribute(image, "14", "BGCOLOR");
  IupSetAttribute(image, "15", "252 126 125");

  return image;
}

static Ihandle* load_image_ui_tab__pencil_WW(void)
{
  unsigned char imgdata[] = {
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 7, 14, 11,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 6, 15, 4, 9,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 6, 8, 13, 10, 11,
    13, 13, 6, 2, 2, 2, 2, 2, 2, 2, 8, 5, 3, 1, 7, 13,
    13, 13, 2, 6, 13, 13, 13, 13, 13, 6, 5, 3, 12, 8, 13, 13,
    13, 13, 2, 13, 13, 13, 13, 13, 6, 5, 3, 12, 7, 2, 13, 13,
    13, 13, 2, 13, 13, 13, 13, 6, 5, 3, 12, 7, 13, 2, 13, 13,
    13, 13, 8, 13, 6, 6, 6, 5, 13, 12, 2, 6, 13, 8, 13, 13,
    13, 13, 8, 6, 6, 6, 6, 0, 10, 2, 6, 6, 6, 8, 13, 13,
    2, 8, 8, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 8, 8, 2,
    8, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 8,
    8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8,
    13, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "4 2 4");
  IupSetAttribute(image, "1", "148 148 143");
  IupSetAttribute(image, "2", "203 203 200");
  IupSetAttribute(image, "3", "242 230 137");
  IupSetAttribute(image, "4", "252 130 132");
  IupSetAttribute(image, "5", "194 176 82");
  IupSetAttribute(image, "6", "234 234 232");
  IupSetAttribute(image, "7", "225 213 191");
  IupSetAttribute(image, "8", "181 179 177");
  IupSetAttribute(image, "9", "196 62 60");
  IupSetAttribute(image, "10", "156 126 124");
  IupSetAttribute(image, "11", "236 188 188");
  IupSetAttribute(image, "12", "172 156 62");
  IupSetAttribute(image, "13", "BGCOLOR");
  IupSetAttribute(image, "14", "236 82 84");
  IupSetAttribute(image, "15", "188 158 156");

  return image;
}

static Ihandle* load_image_IMAGE_Library(void)
{
  unsigned char imgdata[] = {
    5, 5, 5, 15, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 6, 2, 15, 6, 10, 2, 15, 15, 3, 5, 2, 15, 15, 3,
    5, 6, 2, 15, 6, 12, 2, 15, 15, 3, 12, 2, 15, 15, 3, 1,
    14, 14, 14, 4, 13, 10, 3, 3, 3, 12, 12, 9, 3, 3, 8, 1,
    14, 14, 14, 4, 13, 13, 13, 13, 11, 12, 13, 13, 13, 9, 8, 1,
    14, 6, 4, 2, 12, 10, 3, 3, 1, 12, 9, 9, 3, 1, 8, 0,
    14, 14, 14, 4, 12, 13, 13, 13, 11, 12, 13, 13, 13, 9, 8, 0,
    7, 14, 14, 4, 10, 13, 13, 13, 11, 12, 13, 12, 12, 9, 8, 0,
    7, 14, 14, 4, 10, 13, 13, 11, 11, 9, 12, 12, 12, 9, 8, 0,
    7, 14, 14, 4, 10, 11, 11, 11, 11, 9, 12, 12, 12, 9, 8, 0,
    7, 6, 4, 2, 3, 3, 3, 1, 1, 9, 9, 8, 8, 1, 8, 0,
    4, 14, 14, 4, 3, 11, 11, 11, 11, 9, 10, 10, 10, 9, 8, 0,
    4, 2, 2, 2, 3, 3, 1, 1, 1, 9, 9, 8, 8, 1, 8, 0,
    4, 7, 7, 4, 3, 11, 11, 11, 11, 3, 9, 8, 8, 9, 0, 5,
    4, 4, 2, 2, 3, 3, 1, 0, 0, 3, 3, 0, 0, 0, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "100 111 68");
  IupSetAttribute(image, "1", "155 145 87");
  IupSetAttribute(image, "2", "98 100 151");
  IupSetAttribute(image, "3", "160 154 109");
  IupSetAttribute(image, "4", "109 109 203");
  IupSetAttribute(image, "5", "BGCOLOR");
  IupSetAttribute(image, "6", "137 138 203");
  IupSetAttribute(image, "7", "138 138 227");
  IupSetAttribute(image, "8", "139 184 92");
  IupSetAttribute(image, "9", "151 183 119");
  IupSetAttribute(image, "10", "177 197 122");
  IupSetAttribute(image, "11", "239 209 99");
  IupSetAttribute(image, "12", "180 207 146");
  IupSetAttribute(image, "13", "219 222 154");
  IupSetAttribute(image, "14", "174 173 242");
  IupSetAttribute(image, "15", "226 235 221");

  return image;
}

static Ihandle* load_image_disks_WW(void)
{
  unsigned char imgdata[] = {
    7, 7, 7, 7, 7, 10, 4, 3, 3, 3, 3, 3, 3, 3, 4, 10,
    7, 7, 7, 7, 7, 4, 9, 7, 7, 7, 7, 7, 7, 7, 9, 4,
    7, 7, 7, 7, 7, 4, 9, 7, 7, 7, 7, 7, 7, 7, 9, 4,
    7, 7, 7, 7, 7, 4, 9, 7, 7, 7, 7, 7, 7, 7, 9, 4,
    7, 7, 7, 7, 7, 11, 9, 13, 7, 7, 7, 7, 7, 13, 9, 11,
    10, 4, 3, 3, 3, 15, 8, 8, 8, 12, 14, 10, 10, 10, 10, 11,
    4, 9, 7, 7, 7, 7, 7, 7, 7, 9, 0, 14, 14, 14, 9, 11,
    4, 9, 7, 7, 7, 7, 7, 7, 7, 9, 14, 6, 14, 14, 10, 14,
    4, 9, 7, 7, 7, 7, 7, 7, 7, 9, 5, 6, 14, 11, 10, 14,
    11, 9, 13, 7, 7, 7, 7, 7, 13, 9, 5, 1, 11, 4, 10, 5,
    11, 10, 10, 10, 10, 10, 10, 10, 10, 10, 0, 15, 0, 0, 0, 2,
    11, 10, 14, 14, 14, 14, 14, 14, 14, 9, 14, 13, 13, 13, 13, 13,
    14, 10, 14, 6, 6, 6, 6, 14, 14, 10, 14, 7, 7, 7, 7, 7,
    14, 10, 14, 6, 5, 1, 6, 14, 11, 10, 14, 7, 7, 7, 7, 7,
    1, 5, 14, 1, 14, 1, 1, 11, 4, 10, 5, 13, 7, 7, 7, 7,
    3, 1, 0, 15, 15, 15, 15, 0, 0, 0, 2, 13, 7, 7, 7, 7};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "114 58 139");
  IupSetAttribute(image, "1", "178 166 194");
  IupSetAttribute(image, "2", "132 100 148");
  IupSetAttribute(image, "3", "189 206 204");
  IupSetAttribute(image, "4", "166 115 191");
  IupSetAttribute(image, "5", "127 75 155");
  IupSetAttribute(image, "6", "190 191 193");
  IupSetAttribute(image, "7", "BGCOLOR");
  IupSetAttribute(image, "8", "151 135 183");
  IupSetAttribute(image, "9", "209 156 233");
  IupSetAttribute(image, "10", "191 140 215");
  IupSetAttribute(image, "11", "153 103 182");
  IupSetAttribute(image, "12", "140 66 172");
  IupSetAttribute(image, "13", "227 226 234");
  IupSetAttribute(image, "14", "143 88 167");
  IupSetAttribute(image, "15", "117 117 123");

  return image;
}

static Ihandle* load_image_IMAGE_LocaleFun(void)
{
  unsigned char imgdata[] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 13, 4, 12, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 13, 4, 7, 4, 13, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 4, 7, 7, 5, 4, 13,
    15, 15, 15, 15, 15, 8, 8, 3, 2, 15, 4, 5, 5, 5, 5, 1,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 4, 4, 5, 5, 1, 1,
    15, 15, 15, 8, 8, 8, 3, 3, 3, 15, 4, 4, 4, 1, 1, 1,
    15, 15, 15, 15, 15, 15, 15, 15, 12, 4, 7, 4, 4, 1, 1, 1,
    15, 14, 9, 3, 3, 3, 8, 6, 4, 7, 4, 7, 4, 1, 1, 11,
    15, 9, 10, 2, 2, 9, 2, 4, 7, 7, 5, 4, 7, 1, 11, 15,
    14, 9, 2, 8, 3, 9, 2, 4, 5, 5, 5, 5, 1, 15, 15, 15,
    10, 9, 2, 3, 3, 2, 2, 4, 4, 5, 5, 1, 1, 15, 15, 15,
    9, 10, 10, 10, 10, 9, 9, 2, 4, 4, 1, 1, 1, 15, 15, 15,
    3, 10, 0, 0, 0, 9, 3, 0, 4, 4, 1, 1, 1, 15, 15, 15,
    3, 10, 9, 0, 9, 9, 3, 0, 11, 4, 1, 1, 11, 15, 15, 15,
    3, 9, 9, 9, 3, 3, 3, 0, 15, 11, 1, 11, 15, 15, 15, 15,
    10, 2, 2, 2, 0, 0, 0, 10, 15, 15, 15, 15, 15, 15, 15, 15};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "101 79 128");
  IupSetAttribute(image, "1", "164 60 164");
  IupSetAttribute(image, "2", "121 137 160");
  IupSetAttribute(image, "3", "181 171 199");
  IupSetAttribute(image, "4", "215 107 215");
  IupSetAttribute(image, "5", "239 140 239");
  IupSetAttribute(image, "6", "222 187 219");
  IupSetAttribute(image, "7", "255 173 255");
  IupSetAttribute(image, "8", "232 230 177");
  IupSetAttribute(image, "9", "210 214 226");
  IupSetAttribute(image, "10", "222 231 239");
  IupSetAttribute(image, "11", "237 222 239");
  IupSetAttribute(image, "12", "255 214 255");
  IupSetAttribute(image, "13", "255 235 255");
  IupSetAttribute(image, "14", "247 247 247");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_control_double_WW(void)
{
  unsigned char imgdata[] = {
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 6, 12, 9, 9, 9, 9, 6, 12, 9, 9, 9, 9, 9, 9,
    9, 9, 3, 3, 12, 9, 9, 9, 3, 3, 12, 9, 9, 9, 9, 9,
    9, 9, 3, 8, 3, 12, 9, 9, 3, 8, 3, 12, 9, 9, 9, 9,
    9, 9, 11, 6, 8, 11, 12, 9, 11, 6, 8, 11, 12, 9, 9, 9,
    9, 9, 11, 6, 3, 8, 11, 12, 11, 3, 7, 6, 11, 12, 9, 9,
    9, 9, 11, 6, 3, 11, 6, 11, 11, 3, 7, 7, 3, 11, 9, 9,
    9, 9, 0, 3, 7, 6, 0, 1, 0, 10, 7, 10, 0, 1, 9, 9,
    9, 9, 0, 2, 13, 0, 1, 9, 0, 2, 2, 0, 1, 9, 9, 9,
    9, 9, 0, 13, 0, 1, 9, 9, 0, 2, 0, 1, 9, 9, 9, 9,
    9, 9, 0, 0, 1, 9, 9, 9, 0, 0, 1, 9, 9, 9, 9, 9,
    9, 15, 14, 5, 4, 4, 4, 4, 14, 5, 4, 15, 15, 9, 9, 9,
    9, 9, 15, 15, 15, 15, 15, 15, 15, 15, 15, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "66 128 188");
  IupSetAttribute(image, "1", "170 196 220");
  IupSetAttribute(image, "2", "40 184 248");
  IupSetAttribute(image, "3", "98 175 233");
  IupSetAttribute(image, "4", "206 208 206");
  IupSetAttribute(image, "5", "140 166 188");
  IupSetAttribute(image, "6", "128 202 246");
  IupSetAttribute(image, "7", "58 152 214");
  IupSetAttribute(image, "8", "153 227 252");
  IupSetAttribute(image, "9", "BGCOLOR");
  IupSetAttribute(image, "10", "68 176 232");
  IupSetAttribute(image, "11", "83 154 213");
  IupSetAttribute(image, "12", "183 216 241");
  IupSetAttribute(image, "13", "72 200 252");
  IupSetAttribute(image, "14", "92 128 164");
  IupSetAttribute(image, "15", "228 229 228");

  return image;
}

static Ihandle* load_image_broom_code_WW(void)
{
  unsigned char imgdata[] = {
    11, 5, 0, 3, 3, 0, 5, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    3, 0, 5, 11, 11, 5, 0, 3, 11, 11, 11, 11, 11, 11, 11, 11,
    1, 15, 11, 11, 11, 11, 15, 1, 11, 11, 11, 11, 11, 11, 11, 11,
    3, 0, 1, 11, 11, 1, 0, 3, 11, 11, 11, 11, 11, 8, 12, 8,
    11, 5, 0, 3, 3, 0, 5, 11, 8, 6, 6, 3, 8, 7, 4, 7,
    11, 11, 11, 11, 11, 11, 11, 3, 15, 8, 8, 4, 7, 2, 7, 8,
    11, 11, 11, 11, 11, 11, 8, 15, 3, 15, 13, 8, 4, 14, 8, 11,
    11, 11, 3, 8, 13, 6, 13, 13, 15, 3, 15, 13, 13, 4, 8, 11,
    13, 6, 6, 6, 13, 13, 13, 6, 8, 15, 5, 10, 13, 13, 4, 11,
    3, 4, 13, 13, 6, 4, 6, 13, 6, 13, 10, 5, 10, 4, 4, 11,
    11, 5, 14, 12, 4, 13, 13, 4, 13, 13, 6, 9, 15, 0, 8, 11,
    11, 11, 3, 4, 6, 2, 2, 13, 6, 6, 13, 6, 0, 5, 11, 11,
    11, 11, 11, 5, 14, 4, 13, 4, 4, 13, 4, 12, 8, 11, 11, 11,
    11, 11, 11, 11, 8, 2, 12, 12, 6, 2, 12, 8, 11, 11, 11, 11,
    11, 11, 11, 3, 3, 1, 14, 6, 12, 14, 1, 11, 11, 11, 11, 11,
    11, 11, 3, 3, 5, 5, 1, 14, 14, 1, 3, 3, 11, 11, 11, 11};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "77 78 77");
  IupSetAttribute(image, "1", "178 175 170");
  IupSetAttribute(image, "2", "161 122 65");
  IupSetAttribute(image, "3", "229 226 221");
  IupSetAttribute(image, "4", "179 136 79");
  IupSetAttribute(image, "5", "200 198 197");
  IupSetAttribute(image, "6", "191 155 93");
  IupSetAttribute(image, "7", "151 86 52");
  IupSetAttribute(image, "8", "217 202 174");
  IupSetAttribute(image, "9", "108 110 108");
  IupSetAttribute(image, "10", "130 129 130");
  IupSetAttribute(image, "11", "BGCOLOR");
  IupSetAttribute(image, "12", "156 107 57");
  IupSetAttribute(image, "13", "204 175 111");
  IupSetAttribute(image, "14", "120 87 52");
  IupSetAttribute(image, "15", "147 148 149");

  return image;
}

static Ihandle* load_image_edit_column_WW(void)
{
  unsigned char imgdata[] = {
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 9, 6, 6, 13, 9, 8, 8, 9, 2, 2, 2, 9, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 13, 13, 13, 2, 2, 8, 8, 2, 14, 14, 5, 5, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 2, 2, 2, 2, 2, 8, 8, 5, 5, 5, 5, 11, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 2, 2, 14, 14, 5, 8, 8, 5, 11, 11, 11, 11, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 14, 5, 5, 5, 5, 8, 8, 11, 11, 3, 7, 7, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 10, 4, 5, 11, 11, 15, 10, 10, 13, 7, 12, 0, 5, 10, 8,
    8, 10, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 10, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "20 18 20");
  IupSetAttribute(image, "1", "228 230 228");
  IupSetAttribute(image, "2", "76 76 76");
  IupSetAttribute(image, "3", "44 46 44");
  IupSetAttribute(image, "4", "100 102 100");
  IupSetAttribute(image, "5", "65 64 65");
  IupSetAttribute(image, "6", "84 86 84");
  IupSetAttribute(image, "7", "31 33 31");
  IupSetAttribute(image, "8", "BGCOLOR");
  IupSetAttribute(image, "9", "122 123 122");
  IupSetAttribute(image, "10", "241 241 241");
  IupSetAttribute(image, "11", "56 56 56");
  IupSetAttribute(image, "12", "28 26 28");
  IupSetAttribute(image, "13", "84 82 84");
  IupSetAttribute(image, "14", "68 70 68");
  IupSetAttribute(image, "15", "92 94 92");

  return image;
}

static Ihandle* load_image_application_sidebar_right_WW(void)
{
  unsigned char imgdata[] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    13, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 13,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 9, 9, 11, 11, 7, 7,
    1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 6, 6, 6, 4,
    6, 15, 15, 15, 15, 15, 15, 15, 15, 11, 12, 12, 12, 12, 12, 6,
    6, 15, 15, 15, 15, 15, 15, 15, 15, 9, 10, 10, 10, 10, 10, 6,
    4, 15, 15, 15, 15, 15, 15, 15, 15, 9, 10, 9, 9, 9, 10, 4,
    4, 14, 14, 14, 14, 14, 14, 14, 14, 7, 7, 7, 7, 7, 10, 4,
    3, 14, 13, 13, 13, 13, 13, 13, 14, 7, 7, 5, 7, 7, 9, 3,
    3, 14, 13, 13, 13, 13, 13, 13, 14, 7, 7, 5, 5, 7, 9, 3,
    2, 13, 12, 12, 12, 12, 12, 12, 13, 7, 7, 5, 5, 5, 7, 2,
    2, 13, 12, 12, 12, 12, 12, 12, 13, 6, 7, 7, 7, 7, 7, 2,
    1, 13, 12, 12, 12, 12, 12, 12, 13, 4, 7, 7, 7, 7, 7, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    14, 13, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 14,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "132 132 132");
  IupSetAttribute(image, "1", "156 156 156");
  IupSetAttribute(image, "2", "165 165 165");
  IupSetAttribute(image, "3", "173 173 173");
  IupSetAttribute(image, "4", "181 181 181");
  IupSetAttribute(image, "5", "181 189 222");
  IupSetAttribute(image, "6", "189 189 189");
  IupSetAttribute(image, "7", "198 198 214");
  IupSetAttribute(image, "8", "206 206 214");
  IupSetAttribute(image, "9", "214 214 222");
  IupSetAttribute(image, "10", "214 222 239");
  IupSetAttribute(image, "11", "222 222 222");
  IupSetAttribute(image, "12", "231 231 231");
  IupSetAttribute(image, "13", "239 239 239");
  IupSetAttribute(image, "14", "247 247 247");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_IMAGE_Folder(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 5, 14, 14, 14, 13, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    5, 14, 14, 14, 14, 14, 14, 4, 4, 4, 4, 4, 3, 3, 0, 0,
    5, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 10, 2, 0,
    5, 15, 15, 15, 15, 14, 14, 14, 14, 13, 13, 11, 10, 9, 1, 0,
    5, 15, 14, 14, 14, 14, 14, 13, 12, 12, 10, 10, 10, 8, 1, 0,
    5, 15, 14, 14, 14, 14, 14, 13, 12, 11, 10, 10, 10, 8, 1, 0,
    5, 15, 14, 13, 13, 13, 13, 12, 12, 10, 10, 10, 9, 8, 1, 0,
    4, 15, 14, 12, 12, 12, 12, 12, 12, 10, 10, 10, 9, 8, 1, 0,
    4, 15, 13, 12, 12, 12, 12, 12, 10, 10, 10, 9, 9, 8, 1, 0,
    4, 15, 11, 10, 10, 10, 10, 10, 10, 10, 9, 9, 9, 7, 1, 0,
    4, 12, 9, 9, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 1, 0,
    0, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "BGCOLOR");
  IupSetAttribute(image, "1", "132 90 8");
  IupSetAttribute(image, "2", "140 90 8");
  IupSetAttribute(image, "3", "148 99 8");
  IupSetAttribute(image, "4", "173 112 0");
  IupSetAttribute(image, "5", "185 123 0");
  IupSetAttribute(image, "6", "189 148 82");
  IupSetAttribute(image, "7", "198 156 82");
  IupSetAttribute(image, "8", "215 177 98");
  IupSetAttribute(image, "9", "249 206 116");
  IupSetAttribute(image, "10", "255 219 129");
  IupSetAttribute(image, "11", "255 226 136");
  IupSetAttribute(image, "12", "255 231 140");
  IupSetAttribute(image, "13", "255 235 148");
  IupSetAttribute(image, "14", "255 243 163");
  IupSetAttribute(image, "15", "255 248 189");

  return image;
}

static Ihandle* load_image_edit_indent_WW(void)
{
  unsigned char imgdata[] = {
    15, 15, 10, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 10, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 1, 11, 11, 11, 11, 11, 11, 11, 1, 15, 15,
    15, 15, 15, 15, 15, 7, 9, 9, 9, 9, 9, 9, 9, 7, 15, 15,
    15, 15, 4, 6, 15, 11, 4, 4, 4, 4, 4, 4, 4, 11, 15, 15,
    15, 15, 4, 3, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 3, 6, 15, 7, 3, 3, 3, 3, 3, 7, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 3, 9, 9, 9, 9, 9, 3, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 11, 4, 4, 4, 4, 4, 11, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 5, 5, 5, 5, 12, 12, 12, 12, 12, 0, 0, 0, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 8, 2, 12, 12, 12, 12, 12, 0, 14, 13, 13, 13, 8, 15, 15,
    15, 8, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 8, 15, 15};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "42 42 42");
  IupSetAttribute(image, "1", "140 162 236");
  IupSetAttribute(image, "2", "92 92 92");
  IupSetAttribute(image, "3", "38 73 184");
  IupSetAttribute(image, "4", "56 92 202");
  IupSetAttribute(image, "5", "62 63 62");
  IupSetAttribute(image, "6", "200 210 236");
  IupSetAttribute(image, "7", "84 116 204");
  IupSetAttribute(image, "8", "240 240 240");
  IupSetAttribute(image, "9", "196 206 252");
  IupSetAttribute(image, "10", "120 122 120");
  IupSetAttribute(image, "11", "103 134 229");
  IupSetAttribute(image, "12", "55 56 55");
  IupSetAttribute(image, "13", "228 230 228");
  IupSetAttribute(image, "14", "80 80 80");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_arrow_curve_090_WW(void)
{
  unsigned char imgdata[] = {
    11, 11, 11, 11, 11, 11, 14, 3, 14, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 14, 3, 4, 3, 14, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 14, 3, 4, 5, 4, 3, 14, 11, 11, 11, 11, 11,
    11, 11, 11, 14, 3, 4, 5, 5, 5, 4, 3, 14, 11, 11, 11, 11,
    11, 11, 14, 3, 4, 5, 5, 3, 5, 5, 4, 3, 14, 11, 11, 11,
    11, 11, 8, 12, 12, 12, 5, 3, 5, 12, 12, 12, 8, 11, 11, 11,
    11, 11, 11, 11, 11, 12, 5, 12, 3, 12, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 12, 3, 10, 3, 12, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 6, 3, 10, 3, 6, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 12, 13, 10, 13, 6, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 8, 13, 2, 2, 10, 7, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 14, 0, 2, 2, 10, 9, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 1, 10, 2, 2, 8, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 1, 0, 10, 0, 8, 11, 11, 11, 11,
    11, 11, 11, 11, 7, 7, 9, 9, 1, 15, 6, 0, 6, 11, 11, 11,
    11, 11, 11, 11, 11, 7, 7, 7, 7, 7, 7, 7, 7, 11, 11, 11};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "56 126 186");
  IupSetAttribute(image, "1", "151 177 204");
  IupSetAttribute(image, "2", "28 171 241");
  IupSetAttribute(image, "3", "95 170 227");
  IupSetAttribute(image, "4", "149 225 252");
  IupSetAttribute(image, "5", "120 195 245");
  IupSetAttribute(image, "6", "80 135 188");
  IupSetAttribute(image, "7", "222 230 237");
  IupSetAttribute(image, "8", "127 173 215");
  IupSetAttribute(image, "9", "201 210 215");
  IupSetAttribute(image, "10", "57 146 207");
  IupSetAttribute(image, "11", "BGCOLOR");
  IupSetAttribute(image, "12", "83 154 214");
  IupSetAttribute(image, "13", "71 166 225");
  IupSetAttribute(image, "14", "196 223 244");
  IupSetAttribute(image, "15", "108 142 180");

  return image;
}

static Ihandle* load_image_property2_WW(void)
{
  unsigned char imgdata[] = {
    12, 12, 12, 12, 6, 13, 13, 13, 13, 13, 13, 13, 13, 13, 6, 12,
    12, 12, 12, 12, 13, 6, 13, 13, 6, 6, 3, 3, 6, 6, 13, 6,
    12, 12, 12, 12, 8, 13, 13, 6, 7, 6, 3, 13, 13, 3, 3, 13,
    2, 15, 1, 1, 5, 7, 6, 7, 6, 7, 6, 3, 13, 3, 13, 7,
    15, 8, 2, 1, 7, 6, 14, 3, 11, 3, 7, 3, 13, 7, 11, 15,
    1, 1, 1, 4, 3, 14, 14, 4, 3, 11, 3, 11, 4, 14, 2, 12,
    1, 12, 12, 15, 14, 15, 12, 15, 4, 3, 11, 5, 8, 1, 12, 12,
    1, 12, 12, 12, 12, 12, 12, 12, 15, 4, 5, 8, 8, 1, 12, 12,
    1, 12, 10, 12, 1, 8, 8, 2, 12, 12, 12, 12, 12, 1, 12, 12,
    1, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 1, 12, 12,
    1, 12, 11, 2, 1, 8, 8, 8, 1, 8, 1, 8, 12, 1, 12, 12,
    5, 12, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 12, 5, 12, 12,
    5, 2, 9, 2, 1, 1, 1, 1, 1, 1, 8, 2, 2, 5, 12, 12,
    5, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 5, 12, 12,
    5, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 5, 2, 12,
    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 8, 2};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "132 128 128");
  IupSetAttribute(image, "1", "184 184 184");
  IupSetAttribute(image, "2", "232 229 226");
  IupSetAttribute(image, "3", "232 186 135");
  IupSetAttribute(image, "4", "151 98 49");
  IupSetAttribute(image, "5", "162 160 158");
  IupSetAttribute(image, "6", "239 206 167");
  IupSetAttribute(image, "7", "189 138 85");
  IupSetAttribute(image, "8", "202 203 202");
  IupSetAttribute(image, "9", "100 154 252");
  IupSetAttribute(image, "10", "52 170 36");
  IupSetAttribute(image, "11", "176 118 76");
  IupSetAttribute(image, "12", "BGCOLOR");
  IupSetAttribute(image, "13", "213 166 116");
  IupSetAttribute(image, "14", "158 113 66");
  IupSetAttribute(image, "15", "204 191 176");

  return image;
}

static Ihandle* load_image_terminal_WW(void)
{
  unsigned char imgdata[] = {
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    13, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 13,
    6, 9, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 9, 6,
    10, 12, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 7, 12, 10,
    10, 12, 7, 13, 8, 2, 2, 5, 5, 3, 7, 7, 7, 0, 12, 10,
    10, 12, 7, 3, 13, 8, 5, 5, 3, 7, 7, 7, 3, 7, 12, 10,
    10, 4, 7, 6, 2, 5, 6, 8, 7, 7, 3, 3, 3, 7, 4, 10,
    1, 4, 7, 3, 5, 3, 7, 7, 3, 3, 3, 5, 5, 3, 4, 1,
    1, 4, 7, 3, 3, 3, 7, 3, 3, 3, 5, 5, 5, 3, 4, 1,
    1, 13, 0, 3, 7, 7, 3, 3, 5, 5, 5, 5, 5, 3, 13, 1,
    1, 13, 0, 7, 7, 7, 5, 5, 5, 5, 5, 5, 5, 3, 13, 1,
    11, 13, 7, 0, 0, 7, 7, 3, 3, 3, 3, 3, 3, 5, 13, 11,
    8, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 8,
    11, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 11,
    14, 9, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 9, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "63 63 63");
  IupSetAttribute(image, "1", "166 165 166");
  IupSetAttribute(image, "2", "113 114 113");
  IupSetAttribute(image, "3", "86 85 93");
  IupSetAttribute(image, "4", "215 214 215");
  IupSetAttribute(image, "5", "97 98 108");
  IupSetAttribute(image, "6", "188 186 188");
  IupSetAttribute(image, "7", "74 75 78");
  IupSetAttribute(image, "8", "137 138 138");
  IupSetAttribute(image, "9", "236 234 236");
  IupSetAttribute(image, "10", "178 179 178");
  IupSetAttribute(image, "11", "156 154 156");
  IupSetAttribute(image, "12", "227 229 227");
  IupSetAttribute(image, "13", "200 200 200");
  IupSetAttribute(image, "14", "BGCOLOR");
  IupSetAttribute(image, "15", "124 126 124");

  return image;
}

static Ihandle* load_image_edit_diff_WW(void)
{
  unsigned char imgdata[] = {
    1, 4, 4, 4, 4, 4, 4, 1, 10, 10, 12, 12, 12, 12, 10, 10,
    4, 9, 9, 9, 9, 9, 9, 15, 1, 10, 10, 10, 10, 10, 10, 10,
    7, 9, 2, 2, 2, 8, 9, 9, 15, 7, 7, 7, 7, 7, 7, 15,
    7, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 7,
    13, 9, 8, 8, 8, 8, 9, 9, 9, 9, 5, 5, 5, 5, 9, 13,
    13, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 13,
    3, 9, 8, 8, 5, 5, 9, 9, 7, 3, 3, 3, 3, 3, 3, 7,
    3, 9, 9, 9, 9, 9, 9, 7, 7, 10, 10, 10, 10, 10, 10, 10,
    7, 3, 3, 3, 3, 3, 3, 7, 10, 10, 0, 0, 0, 0, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 12, 0, 0, 0, 10, 10, 9, 3, 3, 3, 3, 3, 3, 7,
    10, 10, 10, 10, 10, 10, 10, 9, 13, 11, 9, 9, 9, 9, 9, 13,
    4, 13, 13, 13, 13, 13, 13, 13, 15, 9, 5, 12, 12, 12, 9, 13,
    10, 10, 10, 10, 10, 10, 10, 9, 14, 11, 9, 9, 9, 9, 9, 14,
    10, 6, 0, 0, 0, 0, 6, 6, 11, 14, 14, 14, 14, 14, 14, 7,
    10, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 10, 10};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "61 61 61");
  IupSetAttribute(image, "1", "137 163 241");
  IupSetAttribute(image, "2", "92 106 148");
  IupSetAttribute(image, "3", "41 73 179");
  IupSetAttribute(image, "4", "99 133 233");
  IupSetAttribute(image, "5", "83 96 136");
  IupSetAttribute(image, "6", "224 227 230");
  IupSetAttribute(image, "7", "85 115 207");
  IupSetAttribute(image, "8", "92 102 148");
  IupSetAttribute(image, "9", "196 206 252");
  IupSetAttribute(image, "10", "BGCOLOR");
  IupSetAttribute(image, "11", "164 177 223");
  IupSetAttribute(image, "12", "72 75 81");
  IupSetAttribute(image, "13", "54 91 207");
  IupSetAttribute(image, "14", "68 98 236");
  IupSetAttribute(image, "15", "122 148 230");

  return image;
}

static Ihandle* load_image_IMAGE_Dim(void)
{
  unsigned char imgdata[] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 10, 5, 14, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 10, 5, 6, 5, 13, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 11, 5, 6, 6, 6, 5, 10, 15, 15, 15,
    15, 15, 15, 15, 15, 11, 5, 6, 6, 6, 7, 7, 2, 15, 15, 15,
    15, 15, 15, 15, 15, 5, 6, 7, 6, 7, 9, 1, 0, 15, 15, 15,
    15, 15, 15, 15, 15, 4, 5, 9, 7, 9, 2, 0, 0, 15, 15, 15,
    15, 15, 15, 15, 15, 4, 4, 5, 7, 2, 0, 0, 0, 15, 15, 15,
    15, 15, 15, 15, 15, 4, 4, 4, 0, 0, 0, 0, 8, 15, 15, 15,
    15, 15, 15, 15, 15, 8, 4, 4, 0, 0, 0, 8, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 8, 4, 0, 0, 8, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 8, 0, 7, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "0 49 156");
  IupSetAttribute(image, "1", "24 74 173");
  IupSetAttribute(image, "2", "41 86 177");
  IupSetAttribute(image, "3", "69 127 197");
  IupSetAttribute(image, "4", "90 156 222");
  IupSetAttribute(image, "5", "125 179 235");
  IupSetAttribute(image, "6", "146 197 245");
  IupSetAttribute(image, "7", "167 205 245");
  IupSetAttribute(image, "8", "189 214 235");
  IupSetAttribute(image, "9", "189 222 251");
  IupSetAttribute(image, "10", "206 231 255");
  IupSetAttribute(image, "11", "214 231 255");
  IupSetAttribute(image, "12", "214 239 255");
  IupSetAttribute(image, "13", "222 239 255");
  IupSetAttribute(image, "14", "231 247 255");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_tag__arrow_WW(void)
{
  unsigned char imgdata[] = {
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 8, 2, 2, 2, 2, 15,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 8, 2, 5, 5, 5, 5, 2,
    5, 5, 5, 5, 5, 5, 5, 5, 8, 2, 5, 2, 4, 2, 5, 2,
    5, 5, 5, 5, 5, 5, 5, 8, 2, 5, 5, 14, 5, 14, 5, 2,
    5, 5, 5, 5, 5, 5, 8, 2, 5, 5, 5, 15, 2, 15, 5, 2,
    5, 5, 5, 5, 5, 8, 2, 5, 5, 5, 5, 5, 5, 5, 2, 8,
    5, 5, 5, 5, 8, 2, 5, 5, 5, 5, 5, 5, 5, 2, 8, 5,
    5, 5, 5, 8, 14, 5, 8, 8, 5, 5, 5, 5, 14, 8, 5, 5,
    5, 5, 8, 14, 5, 8, 8, 8, 8, 5, 5, 14, 8, 5, 5, 5,
    5, 3, 14, 5, 8, 8, 8, 8, 8, 5, 14, 8, 9, 6, 5, 5,
    3, 7, 5, 3, 3, 8, 8, 8, 5, 7, 3, 5, 12, 12, 6, 5,
    7, 5, 3, 3, 3, 3, 8, 5, 10, 10, 10, 10, 10, 11, 10, 6,
    3, 7, 5, 3, 3, 3, 5, 7, 0, 11, 11, 1, 1, 1, 11, 10,
    5, 3, 4, 5, 3, 5, 4, 3, 12, 0, 0, 0, 0, 1, 0, 2,
    5, 5, 2, 4, 5, 4, 2, 8, 8, 5, 5, 5, 0, 0, 14, 5,
    5, 3, 2, 14, 4, 14, 15, 15, 15, 3, 8, 8, 13, 7, 5, 5};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "67 135 193");
  IupSetAttribute(image, "1", "84 207 252");
  IupSetAttribute(image, "2", "184 199 200");
  IupSetAttribute(image, "3", "220 232 233");
  IupSetAttribute(image, "4", "138 153 154");
  IupSetAttribute(image, "5", "BGCOLOR");
  IupSetAttribute(image, "6", "183 215 241");
  IupSetAttribute(image, "7", "159 178 181");
  IupSetAttribute(image, "8", "229 235 235");
  IupSetAttribute(image, "9", "140 194 244");
  IupSetAttribute(image, "10", "89 155 212");
  IupSetAttribute(image, "11", "130 222 252");
  IupSetAttribute(image, "12", "105 169 223");
  IupSetAttribute(image, "13", "100 134 172");
  IupSetAttribute(image, "14", "176 185 186");
  IupSetAttribute(image, "15", "214 219 218");

  return image;
}

static Ihandle* load_image_IMAGE_Frame(void)
{
  unsigned char imgdata[] = {
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 4, 4, 4, 4, 8,
    8, 7, 7, 7, 7, 7, 7, 7, 7, 7, 5, 5, 5, 5, 4, 8,
    8, 7, 7, 7, 7, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 8,
    8, 11, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 13, 13, 1, 8,
    8, 11, 15, 15, 15, 15, 3, 3, 3, 3, 3, 13, 13, 11, 1, 8,
    8, 6, 15, 6, 6, 6, 3, 0, 0, 0, 3, 1, 1, 11, 1, 8,
    8, 6, 3, 3, 3, 3, 3, 3, 3, 3, 3, 13, 13, 11, 1, 8,
    8, 6, 3, 0, 0, 3, 2, 2, 2, 2, 2, 1, 1, 11, 1, 8,
    8, 6, 3, 3, 3, 3, 14, 14, 13, 13, 13, 13, 13, 10, 1, 9,
    8, 6, 14, 6, 6, 6, 2, 2, 2, 13, 2, 1, 1, 10, 1, 9,
    8, 6, 14, 14, 14, 14, 13, 13, 13, 13, 13, 13, 12, 10, 1, 9,
    8, 6, 14, 12, 12, 12, 12, 11, 11, 11, 10, 10, 10, 10, 1, 9,
    8, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "24 37 49");
  IupSetAttribute(image, "1", "53 74 103");
  IupSetAttribute(image, "2", "80 102 127");
  IupSetAttribute(image, "3", "90 239 16");
  IupSetAttribute(image, "4", "73 118 207");
  IupSetAttribute(image, "5", "105 150 239");
  IupSetAttribute(image, "6", "148 164 186");
  IupSetAttribute(image, "7", "146 181 246");
  IupSetAttribute(image, "8", "BGCOLOR");
  IupSetAttribute(image, "9", "198 181 123");
  IupSetAttribute(image, "10", "165 189 222");
  IupSetAttribute(image, "11", "175 192 216");
  IupSetAttribute(image, "12", "196 210 233");
  IupSetAttribute(image, "13", "220 230 247");
  IupSetAttribute(image, "14", "239 243 255");
  IupSetAttribute(image, "15", "251 255 255");

  return image;
}

static Ihandle* load_image_Tree_DateTime_WW(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    14, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 14,
    4, 10, 10, 10, 10, 10, 10, 1, 1, 1, 1, 1, 1, 1, 1, 4,
    4, 10, 10, 10, 10, 10, 10, 1, 1, 1, 1, 1, 1, 1, 1, 4,
    4, 15, 15, 2, 15, 15, 2, 15, 15, 2, 15, 15, 2, 15, 15, 4,
    4, 15, 15, 2, 15, 15, 2, 15, 15, 2, 15, 15, 2, 15, 15, 4,
    13, 2, 2, 2, 2, 2, 2, 2, 2, 9, 9, 9, 9, 2, 2, 13,
    13, 15, 15, 2, 15, 15, 2, 15, 15, 9, 15, 15, 9, 15, 15, 13,
    3, 15, 15, 2, 15, 15, 2, 15, 15, 9, 15, 15, 9, 15, 15, 3,
    3, 2, 2, 2, 2, 2, 2, 2, 2, 9, 9, 9, 9, 2, 2, 3,
    3, 15, 15, 2, 15, 15, 2, 15, 15, 2, 15, 15, 2, 15, 15, 3,
    3, 6, 6, 2, 6, 6, 2, 6, 6, 2, 6, 6, 2, 6, 15, 3,
    3, 2, 2, 2, 2, 2, 2, 12, 8, 3, 3, 1, 2, 12, 12, 3,
    5, 3, 6, 2, 6, 6, 13, 1, 14, 2, 0, 0, 8, 8, 3, 5,
    0, 14, 13, 3, 3, 1, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "BGCOLOR");
  IupSetAttribute(image, "1", "132 130 133");
  IupSetAttribute(image, "2", "196 194 196");
  IupSetAttribute(image, "3", "87 93 111");
  IupSetAttribute(image, "4", "106 113 136");
  IupSetAttribute(image, "5", "156 158 164");
  IupSetAttribute(image, "6", "236 242 244");
  IupSetAttribute(image, "7", "117 122 147");
  IupSetAttribute(image, "8", "143 147 151");
  IupSetAttribute(image, "9", "132 2 4");
  IupSetAttribute(image, "10", "4 2 132");
  IupSetAttribute(image, "11", "180 182 180");
  IupSetAttribute(image, "12", "208 210 212");
  IupSetAttribute(image, "13", "97 104 123");
  IupSetAttribute(image, "14", "168 168 168");
  IupSetAttribute(image, "15", "252 254 252");

  return image;
}

static Ihandle* load_image_marker_WW(void)
{
  unsigned char imgdata[] = {
    8, 8, 8, 8, 13, 11, 15, 15, 15, 11, 13, 8, 8, 8, 8, 8,
    8, 8, 8, 13, 15, 3, 11, 11, 11, 3, 15, 13, 8, 8, 8, 8,
    8, 8, 8, 11, 3, 11, 3, 3, 3, 11, 3, 11, 8, 8, 8, 8,
    8, 8, 8, 15, 11, 3, 4, 14, 4, 3, 11, 15, 8, 8, 8, 8,
    8, 8, 8, 7, 11, 3, 0, 0, 0, 3, 11, 7, 8, 8, 8, 8,
    8, 8, 8, 7, 3, 3, 9, 0, 9, 3, 3, 7, 8, 8, 8, 8,
    8, 8, 8, 1, 7, 3, 3, 3, 3, 3, 7, 1, 8, 8, 8, 8,
    8, 8, 8, 13, 12, 3, 3, 3, 3, 3, 12, 13, 8, 8, 8, 8,
    8, 8, 8, 8, 1, 12, 3, 15, 3, 12, 1, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 12, 7, 15, 7, 12, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 1, 12, 15, 12, 1, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 2, 15, 2, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 2, 15, 2, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 2, 15, 2, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 13, 2, 7, 2, 6, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 13, 10, 5, 2, 5, 10, 6, 8, 8, 8, 8, 8};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "80 71 72");
  IupSetAttribute(image, "1", "228 162 160");
  IupSetAttribute(image, "2", "176 55 54");
  IupSetAttribute(image, "3", "250 115 114");
  IupSetAttribute(image, "4", "188 90 92");
  IupSetAttribute(image, "5", "180 134 132");
  IupSetAttribute(image, "6", "223 222 223");
  IupSetAttribute(image, "7", "225 91 90");
  IupSetAttribute(image, "8", "BGCOLOR");
  IupSetAttribute(image, "9", "156 86 84");
  IupSetAttribute(image, "10", "204 206 204");
  IupSetAttribute(image, "11", "247 142 142");
  IupSetAttribute(image, "12", "208 74 72");
  IupSetAttribute(image, "13", "252 230 228");
  IupSetAttribute(image, "14", "100 86 84");
  IupSetAttribute(image, "15", "239 95 95");

  return image;
}

static Ihandle* load_image_control_WW(void)
{
  unsigned char imgdata[] = {
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 8, 14, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 4, 4, 14, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 4, 6, 4, 14, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 10, 8, 6, 10, 14, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 10, 8, 4, 6, 10, 14, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 11, 8, 4, 10, 8, 11, 14, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 11, 4, 11, 11, 11, 4, 11, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 0, 12, 7, 7, 4, 0, 1, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 0, 2, 7, 12, 0, 1, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 0, 2, 2, 0, 1, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 0, 2, 0, 1, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 0, 0, 1, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 5, 13, 3, 3, 15, 15, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 15, 15, 15, 15, 15, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "66 128 188");
  IupSetAttribute(image, "1", "169 196 222");
  IupSetAttribute(image, "2", "46 186 248");
  IupSetAttribute(image, "3", "208 210 208");
  IupSetAttribute(image, "4", "102 178 233");
  IupSetAttribute(image, "5", "92 130 164");
  IupSetAttribute(image, "6", "151 226 252");
  IupSetAttribute(image, "7", "44 158 225");
  IupSetAttribute(image, "8", "132 204 246");
  IupSetAttribute(image, "9", "BGCOLOR");
  IupSetAttribute(image, "10", "92 162 224");
  IupSetAttribute(image, "11", "76 150 208");
  IupSetAttribute(image, "12", "80 182 236");
  IupSetAttribute(image, "13", "140 162 188");
  IupSetAttribute(image, "14", "183 215 241");
  IupSetAttribute(image, "15", "226 228 226");

  return image;
}

static Ihandle* load_image_edit_WW(void)
{
  unsigned char imgdata[] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 12, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 12, 15,
    15, 2, 12, 12, 12, 12, 12, 12, 3, 3, 3, 3, 3, 3, 2, 15,
    15, 6, 12, 5, 6, 6, 6, 3, 3, 6, 6, 6, 6, 5, 6, 15,
    15, 6, 12, 6, 15, 15, 6, 5, 5, 6, 15, 15, 6, 5, 6, 15,
    15, 3, 13, 1, 15, 15, 13, 8, 8, 13, 15, 15, 1, 13, 3, 15,
    15, 15, 15, 15, 15, 15, 13, 2, 2, 13, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 13, 6, 6, 13, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 4, 6, 6, 4, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 4, 2, 2, 4, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 11, 2, 2, 11, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 9, 11, 2, 2, 11, 9, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 2, 0, 4, 2, 2, 4, 0, 2, 15, 15, 15, 15,
    15, 15, 15, 15, 0, 8, 8, 2, 2, 2, 2, 0, 15, 15, 15, 15,
    15, 15, 15, 7, 13, 0, 0, 0, 0, 0, 0, 13, 7, 15, 15, 15,
    15, 15, 15, 10, 7, 7, 7, 7, 7, 7, 7, 7, 14, 15, 15, 15};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "23 24 23");
  IupSetAttribute(image, "1", "172 174 172");
  IupSetAttribute(image, "2", "84 85 89");
  IupSetAttribute(image, "3", "118 118 118");
  IupSetAttribute(image, "4", "57 57 60");
  IupSetAttribute(image, "5", "98 99 98");
  IupSetAttribute(image, "6", "74 75 75");
  IupSetAttribute(image, "7", "228 230 228");
  IupSetAttribute(image, "8", "92 90 108");
  IupSetAttribute(image, "9", "204 202 204");
  IupSetAttribute(image, "10", "244 242 244");
  IupSetAttribute(image, "11", "40 42 40");
  IupSetAttribute(image, "12", "127 128 127");
  IupSetAttribute(image, "13", "64 64 64");
  IupSetAttribute(image, "14", "236 238 236");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_navigation_180_WW(void)
{
  unsigned char imgdata[] = {
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 15, 1, 4, 13, 13, 4, 1, 15, 9, 9, 9, 9,
    9, 9, 9, 11, 7, 7, 10, 10, 10, 10, 7, 7, 11, 9, 9, 9,
    9, 9, 11, 13, 10, 4, 4, 7, 7, 4, 4, 10, 13, 11, 9, 9,
    9, 15, 7, 10, 7, 7, 7, 9, 9, 2, 7, 7, 4, 7, 15, 9,
    9, 1, 7, 7, 2, 2, 9, 9, 5, 13, 2, 2, 7, 2, 1, 9,
    9, 4, 4, 13, 13, 9, 9, 11, 0, 0, 0, 0, 0, 13, 4, 9,
    9, 6, 7, 13, 9, 15, 5, 5, 15, 15, 15, 15, 0, 14, 6, 9,
    9, 6, 7, 3, 5, 5, 5, 15, 15, 15, 15, 15, 0, 14, 6, 9,
    9, 4, 2, 3, 0, 5, 15, 11, 0, 0, 0, 0, 3, 3, 4, 9,
    9, 1, 6, 3, 3, 0, 15, 15, 11, 3, 3, 3, 3, 14, 1, 9,
    9, 15, 6, 13, 2, 3, 0, 15, 15, 3, 3, 3, 3, 6, 15, 9,
    9, 9, 11, 14, 2, 2, 2, 3, 3, 2, 2, 3, 14, 11, 9, 9,
    9, 9, 9, 11, 14, 14, 2, 2, 2, 2, 14, 14, 11, 9, 9, 9,
    9, 9, 15, 5, 11, 12, 6, 8, 8, 6, 12, 11, 5, 15, 9, 9,
    9, 9, 9, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 9, 9, 9};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "42 78 186");
  IupSetAttribute(image, "1", "156 169 220");
  IupSetAttribute(image, "2", "70 112 225");
  IupSetAttribute(image, "3", "56 98 217");
  IupSetAttribute(image, "4", "100 127 214");
  IupSetAttribute(image, "5", "211 213 215");
  IupSetAttribute(image, "6", "76 94 178");
  IupSetAttribute(image, "7", "88 121 219");
  IupSetAttribute(image, "8", "60 66 148");
  IupSetAttribute(image, "9", "BGCOLOR");
  IupSetAttribute(image, "10", "110 145 238");
  IupSetAttribute(image, "11", "185 192 221");
  IupSetAttribute(image, "12", "132 130 172");
  IupSetAttribute(image, "13", "71 104 205");
  IupSetAttribute(image, "14", "67 84 185");
  IupSetAttribute(image, "15", "227 228 227");

  return image;
}

static Ihandle* load_image_edit_signiture_WW(void)
{
  unsigned char imgdata[] = {
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 15, 4, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 15, 15, 11, 15, 14, 15,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 15, 14, 3, 0,
    2, 4, 4, 4, 4, 4, 4, 15, 15, 15, 11, 15, 14, 3, 0, 15,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 15, 14, 3, 0, 15, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 7, 3, 0, 15, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 15, 7, 8, 14, 15, 5, 5, 5,
    7, 1, 5, 5, 5, 5, 5, 2, 14, 3, 14, 12, 5, 5, 5, 5,
    11, 3, 11, 7, 11, 7, 5, 13, 8, 0, 15, 5, 5, 5, 5, 5,
    9, 15, 9, 15, 10, 3, 15, 6, 6, 1, 2, 11, 11, 5, 5, 5,
    10, 5, 5, 5, 5, 5, 5, 11, 11, 11, 11, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "52 50 52");
  IupSetAttribute(image, "1", "177 173 169");
  IupSetAttribute(image, "2", "222 217 209");
  IupSetAttribute(image, "3", "106 107 106");
  IupSetAttribute(image, "4", "197 198 197");
  IupSetAttribute(image, "5", "BGCOLOR");
  IupSetAttribute(image, "6", "156 120 52");
  IupSetAttribute(image, "7", "89 88 79");
  IupSetAttribute(image, "8", "220 186 116");
  IupSetAttribute(image, "9", "140 140 140");
  IupSetAttribute(image, "10", "124 124 124");
  IupSetAttribute(image, "11", "233 232 233");
  IupSetAttribute(image, "12", "220 206 180");
  IupSetAttribute(image, "13", "172 142 76");
  IupSetAttribute(image, "14", "65 63 58");
  IupSetAttribute(image, "15", "183 183 183");

  return image;
}

static Ihandle* load_image_ui_tooltip_balloon_bottom_WW(void)
{
  unsigned char imgdata[] = {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    1, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 1,
    10, 14, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 14, 10,
    10, 5, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 5, 10,
    4, 15, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 15, 4,
    4, 15, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 15, 4,
    13, 12, 12, 12, 12, 15, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13,
    7, 7, 7, 7, 0, 0, 8, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 2, 0, 9, 2, 2, 2, 11, 11, 3, 3, 7, 7, 7,
    7, 7, 7, 3, 11, 11, 3, 3, 3, 3, 3, 3, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "172 154 52");
  IupSetAttribute(image, "1", "228 218 124");
  IupSetAttribute(image, "2", "216 215 216");
  IupSetAttribute(image, "3", "239 239 239");
  IupSetAttribute(image, "4", "204 188 72");
  IupSetAttribute(image, "5", "252 241 162");
  IupSetAttribute(image, "6", "244 224 116");
  IupSetAttribute(image, "7", "BGCOLOR");
  IupSetAttribute(image, "8", "196 190 156");
  IupSetAttribute(image, "9", "180 170 132");
  IupSetAttribute(image, "10", "219 202 83");
  IupSetAttribute(image, "11", "228 229 228");
  IupSetAttribute(image, "12", "188 170 60");
  IupSetAttribute(image, "13", "204 194 108");
  IupSetAttribute(image, "14", "252 246 172");
  IupSetAttribute(image, "15", "250 230 125");

  return image;
}

static Ihandle* load_image_node_insert_next_WW(void)
{
  unsigned char imgdata[] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 2, 5, 2, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 5, 2, 15, 2, 5, 15, 15, 15, 15,
    15, 15, 5, 5, 5, 5, 5, 14, 4, 4, 4, 14, 15, 15, 15, 15,
    15, 15, 15, 15, 14, 15, 15, 14, 5, 13, 5, 14, 15, 15, 15, 15,
    15, 15, 15, 15, 14, 15, 15, 4, 1, 6, 1, 4, 15, 15, 15, 15,
    15, 15, 15, 15, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 1, 15, 15, 4, 10, 3, 10, 4, 15, 15, 15, 15,
    15, 15, 15, 15, 1, 15, 15, 10, 10, 9, 10, 10, 15, 2, 7, 15,
    15, 15, 15, 15, 14, 1, 1, 7, 8, 8, 8, 7, 15, 7, 12, 15,
    15, 15, 15, 15, 15, 15, 15, 12, 3, 8, 3, 12, 15, 2, 11, 15,
    15, 15, 15, 15, 4, 4, 13, 5, 11, 0, 11, 5, 13, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 4, 4, 13, 13, 13, 13, 4, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "52 134 52");
  IupSetAttribute(image, "1", "160 159 160");
  IupSetAttribute(image, "2", "212 221 212");
  IupSetAttribute(image, "3", "44 190 60");
  IupSetAttribute(image, "4", "231 239 233");
  IupSetAttribute(image, "5", "191 191 191");
  IupSetAttribute(image, "6", "140 142 140");
  IupSetAttribute(image, "7", "52 175 63");
  IupSetAttribute(image, "8", "60 229 76");
  IupSetAttribute(image, "9", "100 238 116");
  IupSetAttribute(image, "10", "87 201 97");
  IupSetAttribute(image, "11", "73 153 81");
  IupSetAttribute(image, "12", "87 175 95");
  IupSetAttribute(image, "13", "231 232 231");
  IupSetAttribute(image, "14", "173 174 173");
  IupSetAttribute(image, "15", "BGCOLOR");

  return image;
}

static Ihandle* load_image_node_insert_WW(void)
{
  unsigned char imgdata[] = {
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 4, 11, 3, 11, 4, 10, 10, 10, 10, 2, 7, 2, 10,
    6, 4, 10, 11, 11, 9, 11, 11, 10, 10, 10, 7, 2, 10, 2, 7,
    13, 6, 10, 6, 8, 8, 8, 6, 7, 7, 7, 1, 10, 10, 10, 1,
    12, 2, 10, 13, 3, 8, 3, 13, 10, 10, 10, 1, 7, 14, 7, 1,
    10, 10, 14, 7, 15, 0, 15, 7, 14, 14, 14, 2, 1, 5, 1, 2,
    10, 10, 10, 14, 14, 14, 14, 14, 10, 10, 14, 14, 14, 14, 14, 14,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "52 134 52");
  IupSetAttribute(image, "1", "167 169 167");
  IupSetAttribute(image, "2", "215 215 215");
  IupSetAttribute(image, "3", "44 190 60");
  IupSetAttribute(image, "4", "215 239 220");
  IupSetAttribute(image, "5", "140 142 140");
  IupSetAttribute(image, "6", "52 175 63");
  IupSetAttribute(image, "7", "195 195 195");
  IupSetAttribute(image, "8", "60 229 76");
  IupSetAttribute(image, "9", "100 238 116");
  IupSetAttribute(image, "10", "BGCOLOR");
  IupSetAttribute(image, "11", "87 201 97");
  IupSetAttribute(image, "12", "52 158 60");
  IupSetAttribute(image, "13", "87 175 95");
  IupSetAttribute(image, "14", "224 229 224");
  IupSetAttribute(image, "15", "84 152 92");

  return image;
}

static Ihandle* load_image_IMAGE_UpFolder(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 7, 15, 15, 6, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 7, 15, 15, 15, 15, 6, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 7, 14, 14, 14, 14, 14, 14, 5, 0, 0, 0, 0, 0,
    0, 0, 7, 14, 14, 14, 14, 13, 13, 13, 13, 5, 0, 0, 0, 0,
    0, 7, 13, 13, 13, 13, 13, 13, 12, 12, 12, 13, 4, 0, 0, 0,
    0, 6, 6, 6, 6, 13, 13, 12, 12, 4, 4, 4, 4, 0, 0, 0,
    0, 0, 0, 0, 5, 13, 13, 12, 11, 4, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 5, 12, 12, 10, 10, 4, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 5, 12, 11, 10, 10, 3, 3, 3, 3, 2, 0, 0,
    0, 0, 0, 0, 4, 11, 10, 10, 10, 10, 10, 11, 11, 2, 0, 0,
    0, 0, 0, 0, 4, 10, 10, 9, 9, 10, 10, 10, 10, 2, 0, 0,
    0, 0, 0, 0, 3, 9, 9, 8, 8, 8, 8, 8, 8, 1, 0, 0,
    0, 0, 0, 0, 0, 3, 3, 2, 2, 2, 2, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "BGCOLOR");
  IupSetAttribute(image, "1", "49 49 49");
  IupSetAttribute(image, "2", "57 57 57");
  IupSetAttribute(image, "3", "66 66 66");
  IupSetAttribute(image, "4", "74 74 74");
  IupSetAttribute(image, "5", "82 82 82");
  IupSetAttribute(image, "6", "90 90 90");
  IupSetAttribute(image, "7", "99 99 99");
  IupSetAttribute(image, "8", "107 107 107");
  IupSetAttribute(image, "9", "115 115 115");
  IupSetAttribute(image, "10", "127 127 127");
  IupSetAttribute(image, "11", "140 140 140");
  IupSetAttribute(image, "12", "152 152 152");
  IupSetAttribute(image, "13", "173 173 173");
  IupSetAttribute(image, "14", "189 189 189");
  IupSetAttribute(image, "15", "206 206 206");

  return image;
}

static Ihandle* load_image_ui_toolbar__arrow_WW(void)
{
  unsigned char imgdata[] = {
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    13, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 13,
    6, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6,
    6, 8, 6, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6,
    6, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6,
    11, 8, 11, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, 11,
    11, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 11,
    11, 4, 11, 11, 4, 4, 4, 4, 4, 4, 4, 4, 7, 2, 4, 11,
    15, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 10, 10, 2, 15,
    15, 3, 3, 3, 3, 3, 3, 3, 0, 14, 14, 14, 14, 12, 14, 5,
    8, 4, 4, 4, 4, 4, 4, 4, 14, 12, 9, 1, 1, 1, 9, 14,
    8, 8, 8, 8, 8, 8, 8, 8, 5, 14, 14, 14, 14, 1, 14, 2,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0, 0, 2, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 5, 2, 8, 8};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "68 129 183");
  IupSetAttribute(image, "1", "84 207 252");
  IupSetAttribute(image, "2", "170 200 226");
  IupSetAttribute(image, "3", "148 150 148");
  IupSetAttribute(image, "4", "236 235 236");
  IupSetAttribute(image, "5", "116 158 196");
  IupSetAttribute(image, "6", "202 203 202");
  IupSetAttribute(image, "7", "132 186 228");
  IupSetAttribute(image, "8", "BGCOLOR");
  IupSetAttribute(image, "9", "112 216 252");
  IupSetAttribute(image, "10", "92 166 228");
  IupSetAttribute(image, "11", "184 185 184");
  IupSetAttribute(image, "12", "148 228 252");
  IupSetAttribute(image, "13", "220 218 220");
  IupSetAttribute(image, "14", "69 139 197");
  IupSetAttribute(image, "15", "172 170 172");

  return image;
}

static Ihandle* load_image_keyboards_WW(void)
{
  unsigned char imgdata[] = {
    14, 14, 14, 14, 14, 14, 8, 15, 1, 1, 1, 1, 1, 15, 8, 14,
    14, 14, 14, 14, 14, 14, 15, 4, 2, 2, 2, 2, 2, 4, 15, 14,
    14, 14, 14, 14, 14, 14, 6, 4, 2, 5, 14, 5, 2, 4, 6, 14,
    14, 14, 14, 14, 14, 14, 6, 4, 7, 5, 14, 5, 7, 4, 6, 14,
    14, 14, 14, 14, 14, 14, 6, 4, 7, 5, 14, 5, 7, 4, 6, 14,
    14, 14, 14, 14, 14, 14, 11, 12, 4, 2, 14, 8, 4, 12, 11, 14,
    14, 14, 14, 14, 14, 14, 11, 12, 15, 1, 1, 1, 15, 12, 11, 14,
    14, 8, 15, 1, 1, 1, 9, 13, 3, 6, 1, 1, 1, 1, 11, 14,
    14, 15, 4, 2, 2, 2, 2, 2, 1, 0, 10, 10, 10, 3, 12, 14,
    14, 6, 4, 2, 5, 14, 5, 2, 4, 6, 14, 14, 14, 14, 14, 14,
    14, 6, 4, 7, 5, 14, 5, 7, 4, 6, 14, 14, 14, 14, 14, 14,
    14, 6, 4, 7, 5, 14, 5, 7, 4, 6, 14, 14, 14, 14, 14, 14,
    14, 11, 12, 4, 2, 14, 8, 4, 12, 11, 14, 14, 14, 14, 14, 14,
    14, 11, 12, 15, 1, 1, 1, 15, 12, 11, 14, 14, 14, 14, 14, 14,
    5, 11, 1, 1, 1, 1, 1, 1, 1, 11, 8, 5, 5, 14, 14, 14,
    8, 7, 13, 10, 10, 10, 10, 10, 13, 1, 12, 2, 8, 5, 14, 14};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "116 114 116");
  IupSetAttribute(image, "1", "188 187 188");
  IupSetAttribute(image, "2", "226 227 226");
  IupSetAttribute(image, "3", "152 152 152");
  IupSetAttribute(image, "4", "209 208 209");
  IupSetAttribute(image, "5", "244 244 244");
  IupSetAttribute(image, "6", "177 178 177");
  IupSetAttribute(image, "7", "200 200 200");
  IupSetAttribute(image, "8", "236 236 236");
  IupSetAttribute(image, "9", "132 134 132");
  IupSetAttribute(image, "10", "132 130 132");
  IupSetAttribute(image, "11", "164 164 164");
  IupSetAttribute(image, "12", "218 217 218");
  IupSetAttribute(image, "13", "148 146 148");
  IupSetAttribute(image, "14", "BGCOLOR");
  IupSetAttribute(image, "15", "196 194 196");

  return image;
}

static Ihandle* load_image_IMAGE_AlignObjectsLeft(void)
{
  unsigned char imgdata[] = {
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 0, 0, 8, 7, 7, 6, 6, 6, 5, 8, 8, 8, 8, 8,
    8, 8, 0, 0, 8, 7, 13, 11, 11, 9, 1, 8, 8, 8, 8, 8,
    8, 8, 0, 0, 8, 6, 4, 4, 3, 1, 1, 8, 8, 8, 8, 8,
    8, 8, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 0, 0, 8, 6, 6, 6, 6, 5, 5, 5, 5, 5, 8, 8,
    8, 8, 0, 0, 8, 6, 15, 15, 14, 14, 14, 13, 12, 4, 8, 8,
    8, 8, 0, 0, 8, 6, 15, 12, 10, 10, 7, 7, 7, 2, 8, 8,
    8, 8, 0, 0, 8, 5, 4, 2, 2, 2, 2, 2, 2, 2, 8, 8,
    8, 8, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 0, 0, 8, 8, 0, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8,
    8, 8, 0, 0, 8, 8, 0, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "0 0 0");
  IupSetAttribute(image, "1", "74 74 74");
  IupSetAttribute(image, "2", "57 78 105");
  IupSetAttribute(image, "3", "90 90 90");
  IupSetAttribute(image, "4", "97 108 123");
  IupSetAttribute(image, "5", "143 155 171");
  IupSetAttribute(image, "6", "173 177 184");
  IupSetAttribute(image, "7", "193 203 218");
  IupSetAttribute(image, "8", "BGCOLOR");
  IupSetAttribute(image, "9", "214 214 214");
  IupSetAttribute(image, "10", "214 222 231");
  IupSetAttribute(image, "11", "231 231 231");
  IupSetAttribute(image, "12", "222 231 243");
  IupSetAttribute(image, "13", "235 243 247");
  IupSetAttribute(image, "14", "243 247 255");
  IupSetAttribute(image, "15", "255 255 255");

  return image;
}

static Ihandle* load_image_IMAGE_CheckSpelling2(void)
{
  unsigned char imgdata[] = {
    9, 9, 4, 9, 7, 2, 2, 4, 9, 6, 2, 2, 2, 8, 9, 9,
    9, 7, 4, 7, 7, 2, 9, 5, 5, 2, 5, 9, 4, 5, 9, 9,
    9, 2, 9, 0, 7, 0, 1, 4, 6, 2, 7, 9, 9, 9, 9, 9,
    5, 0, 0, 0, 4, 2, 9, 4, 2, 2, 5, 9, 4, 4, 9, 9,
    1, 5, 9, 5, 0, 0, 0, 2, 7, 5, 1, 0, 0, 3, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 4, 9, 7, 2, 2, 4, 9, 6, 2, 2, 2, 8, 9,
    9, 9, 7, 4, 7, 7, 2, 9, 5, 5, 2, 5, 9, 4, 5, 9,
    9, 9, 2, 9, 0, 7, 0, 1, 4, 6, 2, 7, 9, 9, 9, 9,
    9, 5, 0, 0, 0, 4, 2, 9, 4, 2, 2, 5, 9, 4, 4, 9,
    9, 1, 5, 9, 5, 0, 0, 0, 2, 7, 5, 1, 0, 0, 3, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 4, 9, 7, 2, 2, 4, 9, 6, 2, 2, 2, 8,
    9, 9, 9, 7, 4, 7, 7, 2, 9, 5, 5, 2, 5, 9, 4, 5,
    9, 9, 9, 2, 9, 0, 7, 0, 1, 4, 6, 2, 7, 9, 9, 9,
    9, 9, 5, 0, 0, 0, 4, 2, 9, 4, 2, 2, 5, 9, 4, 4};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "24 24 24");
  IupSetAttribute(image, "1", "57 57 57");
  IupSetAttribute(image, "2", "82 82 82");
  IupSetAttribute(image, "3", "99 99 99");
  IupSetAttribute(image, "4", "123 123 123");
  IupSetAttribute(image, "5", "132 132 132");
  IupSetAttribute(image, "6", "156 156 156");
  IupSetAttribute(image, "7", "165 165 165");
  IupSetAttribute(image, "8", "173 173 173");
  IupSetAttribute(image, "9", "BGCOLOR");

  return image;
}

static Ihandle* load_image_report__exclamation_WW(void)
{
  unsigned char imgdata[] = {
    6, 6, 10, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 10, 6, 6,
    6, 6, 3, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 1, 6, 6,
    6, 8, 13, 14, 8, 6, 13, 13, 13, 13, 13, 13, 6, 1, 6, 6,
    6, 6, 3, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 1, 6, 6,
    6, 8, 13, 14, 8, 6, 13, 13, 13, 13, 13, 13, 6, 1, 6, 6,
    6, 6, 14, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 1, 6, 6,
    6, 8, 13, 14, 8, 6, 13, 13, 13, 13, 13, 13, 6, 1, 6, 6,
    6, 6, 14, 6, 6, 6, 6, 6, 6, 6, 4, 2, 4, 1, 6, 6,
    6, 8, 13, 14, 8, 6, 13, 13, 13, 13, 2, 4, 2, 1, 6, 6,
    6, 6, 14, 6, 6, 6, 6, 6, 6, 4, 7, 4, 7, 15, 6, 6,
    6, 8, 13, 14, 8, 6, 13, 13, 13, 12, 4, 9, 4, 5, 6, 6,
    6, 6, 14, 6, 6, 6, 6, 6, 13, 2, 7, 0, 7, 5, 4, 6,
    6, 8, 13, 14, 8, 6, 13, 13, 12, 7, 2, 12, 2, 2, 2, 6,
    6, 6, 11, 6, 6, 6, 6, 4, 12, 7, 2, 0, 2, 2, 12, 4,
    6, 13, 9, 9, 9, 9, 9, 15, 2, 7, 7, 4, 7, 7, 2, 12,
    6, 6, 13, 13, 13, 13, 13, 12, 5, 5, 5, 5, 5, 5, 5, 12};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "4 2 4");
  IupSetAttribute(image, "1", "68 149 68");
  IupSetAttribute(image, "2", "215 205 73");
  IupSetAttribute(image, "3", "191 206 215");
  IupSetAttribute(image, "4", "238 233 174");
  IupSetAttribute(image, "5", "169 159 48");
  IupSetAttribute(image, "6", "BGCOLOR");
  IupSetAttribute(image, "7", "235 222 88");
  IupSetAttribute(image, "8", "156 154 156");
  IupSetAttribute(image, "9", "64 96 64");
  IupSetAttribute(image, "10", "116 178 116");
  IupSetAttribute(image, "11", "124 146 148");
  IupSetAttribute(image, "12", "196 183 87");
  IupSetAttribute(image, "13", "215 221 228");
  IupSetAttribute(image, "14", "180 184 178");
  IupSetAttribute(image, "15", "112 136 32");

  return image;
}

static Ihandle* load_image_Tree_Bool_WW(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    7, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 7,
    5, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 5,
    5, 15, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 15, 5,
    5, 15, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 15, 5,
    5, 15, 3, 3, 3, 9, 9, 3, 3, 3, 3, 9, 3, 3, 15, 5,
    13, 15, 3, 3, 9, 3, 3, 9, 3, 3, 9, 9, 3, 3, 15, 13,
    13, 15, 3, 3, 9, 3, 3, 9, 3, 3, 3, 9, 3, 3, 15, 13,
    4, 15, 3, 3, 9, 3, 3, 9, 3, 3, 3, 9, 3, 3, 15, 4,
    4, 15, 3, 3, 3, 9, 9, 3, 3, 3, 3, 9, 3, 3, 15, 4,
    4, 15, 14, 3, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 4,
    4, 15, 14, 14, 14, 14, 6, 15, 15, 15, 15, 15, 15, 15, 15, 4,
    4, 14, 15, 14, 14, 15, 15, 2, 11, 4, 4, 1, 8, 8, 8, 4,
    7, 4, 2, 15, 15, 8, 13, 1, 7, 2, 0, 0, 11, 11, 4, 7,
    0, 7, 13, 4, 4, 1, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "BGCOLOR");
  IupSetAttribute(image, "1", "128 130 140");
  IupSetAttribute(image, "2", "199 198 196");
  IupSetAttribute(image, "3", "226 229 228");
  IupSetAttribute(image, "4", "87 93 111");
  IupSetAttribute(image, "5", "106 113 136");
  IupSetAttribute(image, "6", "236 242 244");
  IupSetAttribute(image, "7", "164 165 167");
  IupSetAttribute(image, "8", "206 209 212");
  IupSetAttribute(image, "9", "4 2 252");
  IupSetAttribute(image, "10", "180 182 180");
  IupSetAttribute(image, "11", "143 147 151");
  IupSetAttribute(image, "12", "117 122 147");
  IupSetAttribute(image, "13", "97 104 123");
  IupSetAttribute(image, "14", "233 236 240");
  IupSetAttribute(image, "15", "252 254 252");

  return image;
}

static Ihandle* load_image_IMAGE_search(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 12, 11, 8, 0, 0, 0, 12, 11, 8, 0, 0, 0, 0,
    0, 0, 0, 8, 15, 2, 4, 0, 4, 8, 15, 2, 0, 0, 0, 0,
    0, 0, 12, 11, 9, 7, 7, 0, 12, 11, 9, 7, 7, 0, 0, 0,
    0, 0, 12, 15, 13, 11, 5, 2, 12, 15, 13, 12, 5, 0, 0, 0,
    0, 11, 11, 10, 9, 7, 8, 8, 10, 11, 9, 7, 5, 10, 0, 0,
    12, 11, 15, 15, 14, 10, 7, 4, 11, 15, 15, 14, 9, 6, 0, 0,
    12, 11, 15, 14, 12, 9, 4, 11, 11, 14, 14, 12, 9, 4, 11, 0,
    12, 10, 9, 8, 7, 5, 4, 4, 11, 9, 8, 7, 4, 4, 8, 0,
    12, 15, 14, 13, 12, 5, 0, 0, 0, 10, 15, 14, 13, 11, 7, 0,
    11, 15, 13, 12, 10, 5, 0, 0, 0, 10, 15, 14, 13, 11, 7, 0,
    11, 15, 13, 12, 10, 5, 0, 0, 0, 10, 15, 14, 13, 11, 7, 0,
    11, 15, 13, 12, 10, 5, 0, 0, 10, 10, 14, 13, 13, 9, 7, 0,
    10, 9, 8, 8, 8, 5, 5, 0, 10, 10, 9, 9, 8, 8, 3, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 12, 9, 9, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "BGCOLOR");
  IupSetAttribute(image, "1", "8 41 115");
  IupSetAttribute(image, "2", "8 49 115");
  IupSetAttribute(image, "3", "16 49 115");
  IupSetAttribute(image, "4", "22 57 123");
  IupSetAttribute(image, "5", "33 66 132");
  IupSetAttribute(image, "6", "33 74 132");
  IupSetAttribute(image, "7", "43 74 137");
  IupSetAttribute(image, "8", "53 86 144");
  IupSetAttribute(image, "9", "80 110 163");
  IupSetAttribute(image, "10", "98 127 177");
  IupSetAttribute(image, "11", "116 141 183");
  IupSetAttribute(image, "12", "133 156 197");
  IupSetAttribute(image, "13", "164 182 213");
  IupSetAttribute(image, "14", "209 221 237");
  IupSetAttribute(image, "15", "243 247 251");

  return image;
}

static Ihandle* load_image_scissors_WW(void)
{
  unsigned char imgdata[] = {
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 9, 2, 6, 6, 6, 6, 6, 6, 6, 6, 6, 2, 9, 6, 6,
    6, 10, 10, 2, 6, 6, 6, 6, 6, 6, 6, 2, 10, 10, 6, 6,
    6, 1, 2, 10, 2, 6, 6, 6, 6, 6, 2, 10, 2, 1, 6, 6,
    6, 2, 1, 2, 10, 2, 6, 6, 6, 2, 10, 2, 1, 2, 6, 6,
    6, 6, 2, 1, 2, 10, 2, 6, 2, 10, 2, 1, 2, 6, 6, 6,
    6, 6, 6, 2, 1, 2, 1, 10, 1, 2, 1, 2, 6, 6, 6, 6,
    6, 6, 6, 6, 2, 1, 2, 3, 9, 1, 2, 6, 6, 6, 6, 6,
    6, 6, 15, 14, 5, 11, 8, 6, 11, 5, 5, 14, 15, 6, 6, 6,
    6, 14, 13, 14, 12, 12, 7, 0, 12, 12, 12, 14, 13, 14, 6, 6,
    15, 13, 12, 11, 8, 14, 11, 7, 11, 14, 8, 11, 12, 13, 15, 6,
    13, 12, 11, 8, 14, 11, 4, 6, 4, 11, 14, 8, 11, 12, 13, 6,
    11, 14, 8, 14, 8, 4, 6, 6, 6, 4, 8, 14, 8, 14, 11, 6,
    7, 5, 12, 8, 4, 6, 6, 6, 6, 6, 4, 8, 12, 5, 7, 6,
    9, 7, 8, 1, 2, 2, 2, 2, 2, 2, 2, 1, 8, 7, 9, 6,
    2, 2, 2, 2, 6, 6, 6, 6, 6, 6, 6, 2, 2, 2, 2, 6};

  Ihandle* image = IupImage(16, 16, imgdata);

  IupSetAttribute(image, "0", "196 10 12");
  IupSetAttribute(image, "1", "159 152 153");
  IupSetAttribute(image, "2", "216 215 216");
  IupSetAttribute(image, "3", "116 118 116");
  IupSetAttribute(image, "4", "220 162 164");
  IupSetAttribute(image, "5", "220 80 80");
  IupSetAttribute(image, "6", "BGCOLOR");
  IupSetAttribute(image, "7", "212 110 112");
  IupSetAttribute(image, "8", "178 56 58");
  IupSetAttribute(image, "9", "204 190 188");
  IupSetAttribute(image, "10", "179 179 179");
  IupSetAttribute(image, "11", "200 59 59");
  IupSetAttribute(image, "12", "250 141 142");
  IupSetAttribute(image, "13", "217 99 98");
  IupSetAttribute(image, "14", "247 121 121");
  IupSetAttribute(image, "15", "244 166 164");

  return image;
}

static Ihandle * load_image_control_record_WW(void) {
	unsigned char imgdata[] = {
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 7, 10, 12, 4, 12, 10, 7, 13, 13, 13, 13, 13,
		13, 13, 13, 1, 15, 6, 6, 6, 6, 6, 15, 1, 13, 13, 13, 13,
		13, 13, 7, 12, 6, 6, 6, 6, 6, 6, 6, 12, 7, 13, 13, 13,
		13, 13, 9, 15, 15, 15, 15, 15, 15, 15, 15, 15, 9, 13, 13, 13,
		13, 13, 12, 15, 4, 4, 4, 5, 0, 0, 0, 5, 12, 13, 13, 13,
		13, 13, 4, 15, 5, 3, 0, 0, 0, 0, 0, 3, 4, 13, 13, 13,
		13, 13, 12, 4, 3, 3, 3, 3, 3, 3, 3, 3, 12, 13, 13, 13,
		13, 13, 9, 5, 8, 3, 3, 3, 3, 3, 3, 5, 9, 13, 13, 13,
		13, 13, 7, 14, 5, 8, 8, 8, 8, 8, 3, 14, 7, 13, 13, 13,
		13, 13, 13, 1, 14, 5, 8, 8, 8, 5, 14, 1, 13, 13, 13, 13,
		13, 13, 13, 7, 1, 2, 11, 11, 11, 2, 1, 7, 13, 13, 13, 13,
		13, 13, 13, 13, 7, 7, 7, 7, 7, 7, 7, 7, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13 };

	Ihandle* image = IupImage(16, 16, imgdata);

	IupSetAttribute(image, "0", "163 20 21");
	IupSetAttribute(image, "1", "212 186 185");
	IupSetAttribute(image, "2", "148 102 100");
	IupSetAttribute(image, "3", "182 21 22");
	IupSetAttribute(image, "4", "184 58 56");
	IupSetAttribute(image, "5", "182 39 39");
	IupSetAttribute(image, "6", "223 88 88");
	IupSetAttribute(image, "7", "233 227 225");
	IupSetAttribute(image, "8", "204 19 20");
	IupSetAttribute(image, "9", "196 130 132");
	IupSetAttribute(image, "10", "215 125 127");
	IupSetAttribute(image, "11", "127 57 55");
	IupSetAttribute(image, "12", "188 78 76");
	IupSetAttribute(image, "13", "BGCOLOR");
	IupSetAttribute(image, "14", "160 66 68");
	IupSetAttribute(image, "15", "204 70 69");

	return image;
}

static Ihandle * load_image_control_stop_square_WW(void) {
	unsigned char imgdata[] = {
		12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
		12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
		12, 12, 1, 8, 8, 8, 8, 8, 8, 8, 8, 8, 1, 12, 12, 12,
		12, 12, 8, 11, 11, 11, 11, 11, 11, 11, 11, 11, 8, 12, 12, 12,
		12, 12, 3, 11, 5, 5, 5, 5, 5, 5, 5, 1, 3, 12, 12, 12,
		12, 12, 3, 1, 8, 8, 8, 8, 8, 3, 3, 5, 3, 12, 12, 12,
		12, 12, 10, 1, 8, 3, 10, 10, 10, 10, 10, 3, 10, 12, 12, 12,
		12, 12, 10, 5, 10, 4, 4, 4, 4, 4, 4, 3, 10, 12, 12, 12,
		12, 12, 10, 8, 4, 4, 4, 4, 4, 4, 4, 7, 10, 12, 12, 12,
		12, 12, 15, 13, 2, 2, 2, 2, 2, 2, 2, 7, 15, 12, 12, 12,
		12, 12, 15, 13, 2, 2, 2, 2, 2, 2, 2, 14, 15, 12, 12, 12,
		12, 12, 0, 13, 14, 14, 14, 14, 14, 14, 14, 14, 0, 12, 12, 12,
		12, 12, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 12, 12, 12,
		12, 12, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 12, 12, 12,
		12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
		12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12 };

	Ihandle* image = IupImage(16, 16, imgdata);

	IupSetAttribute(image, "0", "53 115 173");
	IupSetAttribute(image, "1", "136 203 248");
	IupSetAttribute(image, "2", "16 170 244");
	IupSetAttribute(image, "3", "91 166 224");
	IupSetAttribute(image, "4", "51 152 216");
	IupSetAttribute(image, "5", "119 194 247");
	IupSetAttribute(image, "6", "229 231 229");
	IupSetAttribute(image, "7", "48 172 236");
	IupSetAttribute(image, "8", "99 178 235");
	IupSetAttribute(image, "9", "100 132 164");
	IupSetAttribute(image, "10", "74 147 205");
	IupSetAttribute(image, "11", "142 219 252");
	IupSetAttribute(image, "12", "BGCOLOR");
	IupSetAttribute(image, "13", "44 186 249");
	IupSetAttribute(image, "14", "15 183 252");
	IupSetAttribute(image, "15", "64 128 188");

	return image;
}

void load_all_images_Images(void)
{
  IupSetHandle("disk_µ", load_image_disk_WW());
  IupSetHandle("bookmark__arrow_left_µ", load_image_bookmark__arrow_left_WW());
  IupSetHandle("IMAGE_Constant", load_image_IMAGE_Constant());
  IupSetHandle("compile_µ", load_image_compile_WW());
  IupSetHandle("Tree_Null_µ", load_image_Tree_Null_WW());
  IupSetHandle("document__plus_µ", load_image_document__plus_WW());
  IupSetHandle("IMAGE_PinPush", load_image_IMAGE_PinPush());
  IupSetHandle("Tree_Number_µ", load_image_Tree_Number_WW());
  IupSetHandle("application_sidebar_left_µ", load_image_application_sidebar_left_WW());
  IupSetHandle("IMAGE_ArrowDown", load_image_IMAGE_ArrowDown());
  IupSetHandle("ui_status_bar_blue_µ", load_image_ui_status_bar_blue_WW());
  IupSetHandle("windows_µ", load_image_windows_WW());
  IupSetHandle("cross_button_µ", load_image_cross_button_WW());
  IupSetHandle("building__arrow_µ", load_image_building__arrow_WW());
  IupSetHandle("document_copy_µ", load_image_document_copy_WW());
  IupSetHandle("arrow_curve_270_µ", load_image_arrow_curve_270_WW());
  IupSetHandle("key__plus_µ", load_image_key__plus_WW());
  IupSetHandle("IMAGE_ArrowUp", load_image_IMAGE_ArrowUp());
  IupSetHandle("tree_µ", load_image_tree_WW());
  IupSetHandle("layout_design_µ", load_image_layout_design_WW());
  IupSetHandle("control_double_180_µ", load_image_control_double_180_WW());
  IupSetHandle("IMAGE_CheckSpelling", load_image_IMAGE_CheckSpelling());
  IupSetHandle("printer_µ", load_image_printer_WW());
  IupSetHandle("IMAGE_View", load_image_IMAGE_View());
  IupSetHandle("clipboard_paste_µ", load_image_clipboard_paste_WW());
  IupSetHandle("IMAGE_Replace", load_image_IMAGE_Replace());
  IupSetHandle("IMAGE_WithLineNumber", load_image_IMAGE_WithLineNumber());
  IupSetHandle("IMAGE_FormatBasic", load_image_IMAGE_FormatBasic());
  IupSetHandle("color_µ", load_image_color_WW());
  IupSetHandle("yin_yang_µ", load_image_yin_yang_WW());
  IupSetHandle("folder_search_result_µ", load_image_folder_search_result_WW());
  IupSetHandle("IMAGE_FormRun", load_image_IMAGE_FormRun());
  IupSetHandle("disk__plus_µ", load_image_disk__plus_WW());
  IupSetHandle("arrow_return_270_µ", load_image_arrow_return_270_WW());
  IupSetHandle("disk__pencil_µ", load_image_disk__pencil_WW());
  IupSetHandle("arrow_return_270_left_µ", load_image_arrow_return_270_left_WW());
  IupSetHandle("IMAGE_String", load_image_IMAGE_String());
  IupSetHandle("ui_menu_blue_µ", load_image_ui_menu_blue_WW());
  IupSetHandle("settings_µ", load_image_settings_WW());
  IupSetHandle("edit_lowercase_µ", load_image_edit_lowercase_WW());
  IupSetHandle("IMAGE_AlignToGridHS", load_image_IMAGE_AlignToGridHS());
  IupSetHandle("navigation_µ", load_image_navigation_WW());
  IupSetHandle("key_µ", load_image_key_WW());
  IupSetHandle("Tree_String_µ", load_image_Tree_String_WW());
  IupSetHandle("IMAGE_Function", load_image_IMAGE_Function());
  IupSetHandle("IMAGE_Trigger", load_image_IMAGE_Trigger());
  IupSetHandle("IMAGE_Pin", load_image_IMAGE_Pin());
  IupSetHandle("edit_uppercase_µ", load_image_edit_uppercase_WW());
  IupSetHandle("IMAGE_RunPart", load_image_IMAGE_RunPart());
  IupSetHandle("binocular__pencil_µ", load_image_binocular__pencil_WW());
  IupSetHandle("folder_open_document_µ", load_image_folder_open_document_WW());
  IupSetHandle("bookmark_µ", load_image_bookmark_WW());
  IupSetHandle("IMAGE_AddDocument", load_image_IMAGE_AddDocument());
  IupSetHandle("IMAGE_Sub", load_image_IMAGE_Sub());
  IupSetHandle("edit_symbol_µ", load_image_edit_symbol_WW());
  IupSetHandle("cross_script_µ", load_image_cross_script_WW());
  IupSetHandle("Tree_Binary_µ", load_image_Tree_Binary_WW());
  IupSetHandle("clipboard_list_µ", load_image_clipboard_list_WW());
  IupSetHandle("bookmark__arrow_µ", load_image_bookmark__arrow_WW());
  IupSetHandle("ui_tab__pencil_µ", load_image_ui_tab__pencil_WW());
  IupSetHandle("IMAGE_Library", load_image_IMAGE_Library());
  IupSetHandle("disks_µ", load_image_disks_WW());
  IupSetHandle("IMAGE_LocaleFun", load_image_IMAGE_LocaleFun());
  IupSetHandle("control_double_µ", load_image_control_double_WW());
  IupSetHandle("broom_code_µ", load_image_broom_code_WW());
  IupSetHandle("edit_column_µ", load_image_edit_column_WW());
  IupSetHandle("application_sidebar_right_µ", load_image_application_sidebar_right_WW());
  IupSetHandle("IMAGE_Folder", load_image_IMAGE_Folder());
  IupSetHandle("edit_indent_µ", load_image_edit_indent_WW());
  IupSetHandle("arrow_curve_090_µ", load_image_arrow_curve_090_WW());
  IupSetHandle("property2_µ", load_image_property2_WW());
  IupSetHandle("terminal_µ", load_image_terminal_WW());
  IupSetHandle("edit_diff_µ", load_image_edit_diff_WW());
  IupSetHandle("IMAGE_Dim", load_image_IMAGE_Dim());
  IupSetHandle("tag__arrow_µ", load_image_tag__arrow_WW());
  IupSetHandle("IMAGE_Frame", load_image_IMAGE_Frame());
  IupSetHandle("Tree_DateTime_µ", load_image_Tree_DateTime_WW());
  IupSetHandle("marker_µ", load_image_marker_WW());
  IupSetHandle("control_µ", load_image_control_WW());
  IupSetHandle("edit_µ", load_image_edit_WW());
  IupSetHandle("navigation_180_µ", load_image_navigation_180_WW());
  IupSetHandle("edit_signiture_µ", load_image_edit_signiture_WW());
  IupSetHandle("ui_tooltip_balloon_bottom_µ", load_image_ui_tooltip_balloon_bottom_WW());
  IupSetHandle("node_insert_next_µ", load_image_node_insert_next_WW());
  IupSetHandle("node_insert_µ", load_image_node_insert_WW());
  IupSetHandle("IMAGE_UpFolder", load_image_IMAGE_UpFolder());
  IupSetHandle("ui_toolbar__arrow_µ", load_image_ui_toolbar__arrow_WW());
  IupSetHandle("keyboards_µ", load_image_keyboards_WW());
  IupSetHandle("IMAGE_AlignObjectsLeft", load_image_IMAGE_AlignObjectsLeft());
  IupSetHandle("IMAGE_CheckSpelling2", load_image_IMAGE_CheckSpelling2());
  IupSetHandle("report__exclamation_µ", load_image_report__exclamation_WW());
  IupSetHandle("Tree_Bool_µ", load_image_Tree_Bool_WW());
  IupSetHandle("IMAGE_search", load_image_IMAGE_search());
  IupSetHandle("scissors_µ", load_image_scissors_WW());
  IupSetHandle("control_record_µ", load_image_control_record_WW());
  IupSetHandle("control_stop_square_µ", load_image_control_stop_square_WW());
}

