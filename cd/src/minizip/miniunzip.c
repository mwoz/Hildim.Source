/*
   miniunz.c
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

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>
#endif

#include "unzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef xxx_WIN32
#define USEWIN32IOAPI
#include "iowin32.h"
#endif
/*
  mini unzip, demo of unzip package

  usage :
  Usage : miniunz [-exvlo] file.zip [file_to_extract] [-d extractdir]

  list the file in the zipfile, and print the content of FILE_ID.ZIP or README.TXT
  if it exists
  */


/* mymkdir and change_file_date are not 100 % portable
As I don't know well Unix, I wait feedback for the unix portion */

static int mymkdir(const char* dirname)
{
  int ret = 0;
#ifdef _WIN32
  ret = _mkdir(dirname);
#elif unix
  ret = mkdir(dirname, 0775);
#elif __APPLE__
  ret = mkdir(dirname, 0775);
#endif
  return ret;
}

static int makedir(const char* newdir)
{
  char *buffer;
  char *p;
  int  len = (int)strlen(newdir);

  if (len <= 0)
    return 0;

  buffer = (char*)malloc(len + 1);
  if (buffer == NULL)
  {
    printf("Error allocating memory\n");
    return UNZ_INTERNALERROR;
  }
  strcpy(buffer, newdir);

  if (buffer[len - 1] == '/') {
    buffer[len - 1] = '\0';
  }
  if (mymkdir(buffer) == 0)
  {
    free(buffer);
    return 1;
  }

  p = buffer + 1;
  while (1)
  {
    char hold;

    while (*p && *p != '\\' && *p != '/')
      p++;
    hold = *p;
    *p = 0;
    if ((mymkdir(buffer) == -1) && (errno == ENOENT))
    {
      printf("couldn't create directory %s\n", buffer);
      free(buffer);
      return 0;
    }
    if (hold == 0)
      break;
    *p++ = hold;
  }
  free(buffer);
  return 1;
}

/* change_file_date : change the date/time of a file
    filename : the filename of the file where date/time must be modified
    dosdate : the new date at the MSDos format (4 bytes)
    tmu_date : the SAME new date at the tm_unz format */
static void change_file_date(const char* filename, uLong dosdate, tm_unz tmu_date)
{
#ifdef _WIN32
  HANDLE hFile;
  FILETIME ftm, ftLocal, ftCreate, ftLastAcc, ftLastWrite;

  hFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE,
                      0, NULL, OPEN_EXISTING, 0, NULL);
  GetFileTime(hFile, &ftCreate, &ftLastAcc, &ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate >> 16), (WORD)dosdate, &ftLocal);
  LocalFileTimeToFileTime(&ftLocal, &ftm);
  SetFileTime(hFile, &ftm, &ftLastAcc, &ftm);
  CloseHandle(hFile);
#else
#ifdef unix || __APPLE__
  struct utimbuf ut;
  struct tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  if (tmu_date.tm_year > 1900)
    newdate.tm_year=tmu_date.tm_year - 1900;
  else
    newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#endif
#endif
}

static int do_extract_currentfile(unzFile uf, const char *dirname, const int* popt_extract_without_path, int* popt_overwrite, const char* password)
{
  char inzip[256];
  char filename_inzip[1024];
  char* filename_withoutpath;
  char* p;
  int err = UNZ_OK;
  FILE *fout = NULL;
  void* buf;
  uInt size_buf;

  unz_file_info64 file_info;
  err = unzGetCurrentFileInfo64(uf, &file_info, inzip, sizeof(inzip), NULL, 0, NULL, 0);

  strcpy(filename_inzip, dirname);
  strcat(filename_inzip, "/");
  strcat(filename_inzip, inzip);

  if (err != UNZ_OK)
    return err;

  size_buf = WRITEBUFFERSIZE;
  buf = (void*)malloc(size_buf);
  if (buf == NULL)
    return UNZ_INTERNALERROR;

  p = filename_withoutpath = filename_inzip;
  while ((*p) != '\0')
  {
    if (((*p) == '/') || ((*p) == '\\'))
      filename_withoutpath = p + 1;
    p++;
  }

  if ((*filename_withoutpath) == '\0')
  {
    if ((*popt_extract_without_path) == 0)
      mymkdir(filename_inzip);
  }
  else
  {
    const char* write_filename;
    int skip = 0;

    if ((*popt_extract_without_path) == 0)
      write_filename = filename_inzip;
    else
      write_filename = filename_withoutpath;

    err = unzOpenCurrentFilePassword(uf, password);
    if (err != UNZ_OK)
    {
      free(buf);
      return err;
    }

    if ((skip == 0) && (err == UNZ_OK))
    {
      fout = fopen(write_filename, "wb");
      /* some zipfile don't contain directory alone before file */
      if ((fout == NULL) && ((*popt_extract_without_path) == 0) &&
          (filename_withoutpath != (char*)filename_inzip))
      {
        char c = *(filename_withoutpath - 1);
        *(filename_withoutpath - 1) = '\0';
        makedir(write_filename);
        *(filename_withoutpath - 1) = c;
        fout = fopen(write_filename, "wb");
      }

      if (fout == NULL)
      {
        free(buf);
        return 1;
      }
    }

    if (fout != NULL)
    {

      do
      {
        err = unzReadCurrentFile(uf, buf, size_buf);
        if (err<0)
          break;
        if (err>0)
        if (fwrite(buf, err, 1, fout) != 1)
        {
          err = UNZ_ERRNO;
          break;
        }
      } while (err > 0);
      if (fout)
        fclose(fout);

      if (err == 0)
        change_file_date(write_filename, file_info.dosDate, file_info.tmu_date);
    }

    if (err == UNZ_OK)
    {
      err = unzCloseCurrentFile(uf);
      if (err != UNZ_OK)
      {
        free(buf);
        return err;
      }
    }
    else
      unzCloseCurrentFile(uf); /* don't lose the error */
  }

  free(buf);
  return err;
}

static int do_extract(unzFile uf, const char* dirname, int opt_extract_without_path, int opt_overwrite, const char* password)
{
  uLong i;
  unz_global_info64 gi;
  int err;

  err = unzGetGlobalInfo64(uf, &gi);
  if (err != UNZ_OK)
    return err;

  for (i = 0; i < gi.number_entry; i++)
  {
    if (do_extract_currentfile(uf, dirname, &opt_extract_without_path, &opt_overwrite, password) != UNZ_OK)
      break;

    if ((i + 1) < gi.number_entry)
    {
      err = unzGoToNextFile(uf);
      if (err != UNZ_OK)
        break;
    }
  }

  return UNZ_OK;
}

int miniunzip(const char *zipfilename, const char *dirname)
{
  int ret_value;
  unzFile uf = unzOpen64(zipfilename);
  if (uf == NULL)
    return UNZ_ERRNO;

  ret_value = do_extract(uf, dirname, 0, 0, NULL);

  unzClose(uf);

  return ret_value;
}
