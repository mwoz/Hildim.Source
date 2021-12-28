/*
minizip.c
Version 1.1, February 14h, 2010
sample part of the MiniZip project - ( http://www.winimage.com/zLibDll/minizip.html )

Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

Modifications of Unzip for Zip64
Copyright (C) 2007-2008 Even Rouault

Modifications for Zip64 support on both zip and unzip
Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#include <utime.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "zip.h"

#define WRITEBUFFERSIZE (16384)

static uLong filetime(const char *filename, tm_zip *tmzip, uLong *dt)
               /* f - name of file to get info on */
               /* tmzip - return value: access, modific. and creation times */
               /* dt - dostime */
{
  int ret = 0;
#ifdef WIN32
  FILETIME ftLocal;
  HANDLE hFind;
  WIN32_FIND_DATAA ff32;

  hFind = FindFirstFileA(filename, &ff32);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    FileTimeToLocalFileTime(&(ff32.ftLastWriteTime), &ftLocal);
    FileTimeToDosDateTime(&ftLocal, ((LPWORD)dt) + 1, ((LPWORD)dt) + 0);
    FindClose(hFind);
    ret = 1;
  }
  (void)tmzip;
#else
  struct stat s;
  if (stat(filename, &s) == 0)
  {
    time_t tm_t = s.st_mtime;
    struct tm* filedate = localtime(&tm_t);

    tmzip->tm_sec = filedate->tm_sec;
    tmzip->tm_min = filedate->tm_min;
    tmzip->tm_hour = filedate->tm_hour;
    tmzip->tm_mday = filedate->tm_mday;
    tmzip->tm_mon = filedate->tm_mon;
    tmzip->tm_year = filedate->tm_year;
    ret = 1;
  }
  (void)dt;
#endif
  return ret;
}

int minizip(const char *filename, const char *dirname, const char **files, int nFiles)
{
  int i;
  int err = ZIP_OK;
  int size_buf = WRITEBUFFERSIZE;
  void* buf = NULL;
  zipFile zf;
  char filenameinzip[10240];

  buf = (void*)malloc(size_buf);
  if (buf == NULL)
    return ZIP_INTERNALERROR;

  zf = zipOpen(filename, 0);
  if (zf == NULL)
  {
    free(buf);
    return ZIP_ERRNO;
  }

  for (i = 0; (i < nFiles) && (err == ZIP_OK); i++)
  {
    FILE * fin = NULL;
    int size_read;
    zip_fileinfo zi;

    sprintf(filenameinzip, "%s/%s", dirname, files[i]);

    zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
    zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
    zi.dosDate = 0;
    zi.internal_fa = 0;
    zi.external_fa = 0;
    filetime(filenameinzip, &zi.tmz_date, &zi.dosDate);

    err = zipOpenNewFileInZip(zf, files[i], &zi,
                              NULL, 0, NULL, 0, NULL /* comment*/,
                              Z_DEFLATED, Z_DEFAULT_COMPRESSION);

    if (err == ZIP_OK)
    {
      fin = fopen(filenameinzip, "rb");
      if (fin == NULL)
        err = ZIP_ERRNO;
    }

    if (err == ZIP_OK)
    do
    {
      err = ZIP_OK;
      size_read = (int)fread(buf, 1, size_buf, fin);
      if (size_read < size_buf)
      if (feof(fin) == 0)
        err = ZIP_ERRNO;

      if (size_read>0)
        err = zipWriteInFileInZip(zf, buf, size_read);
    } while ((err == ZIP_OK) && (size_read>0));

    if (fin)
      fclose(fin);

    if (err < 0)
      err = ZIP_ERRNO;
    else
      err = zipCloseFileInZip(zf);
  }
  zipClose(zf, NULL);

  free(buf);
  return err;
}
