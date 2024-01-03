/* The file is based on XrootD's implementation of Checksum manager, so all copyrights are included below */

/******************************************************************************/
/*                                                                            */
/*                   X r d O s s C k s M a n a g e r . c c                    */
/*                                                                            */
/* (c) 2011 by the Board of Trustees of the Leland Stanford, Jr., University  */
/*                            All Rights Reserved                             */
/*   Produced by Andrew Hanushevsky for Stanford University under contract    */
/*              DE-AC02-76-SFO0515 with the Department of Energy              */
/*                                                                            */
/* This file is part of the XRootD software suite.                            */
/*                                                                            */
/* XRootD is free software: you can redistribute it and/or modify it under    */
/* the terms of the GNU Lesser General Public License as published by the     */
/* Free Software Foundation, either version 3 of the License, or (at your     */
/* option) any later version.                                                 */
/*                                                                            */
/* XRootD is distributed in the hope that it will be useful, but WITHOUT      */
/* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or      */
/* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public       */
/* License for more details.                                                  */
/*                                                                            */
/* You should have received a copy of the GNU Lesser General Public License   */
/* along with XRootD in a file called COPYING.LESSER (LGPL license) and file  */
/* COPYING (GPL license).  If not, see <http://www.gnu.org/licenses/>.        */
/*                                                                            */
/* The copyright holder's institutional names and contributor's names may not */
/* be used to endorse or promote products derived from this software without  */
/* specific prior written permission of the institution or contributor.       */
/******************************************************************************/


/* The following implementation of adler32 was derived from zlib and is
                   * Copyright (C) 1995-1998 Mark Adler
   Below are the zlib license terms for this implementation.
*/

/* zlib.h -- interface of the 'zlib' general purpose compression library
  version 1.1.4, March 11th, 2002

  Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Jean-loup Gailly        Mark Adler
  jloup@gzip.org          madler@alumni.caltech.edu


  The data format used by the zlib library is described by RFCs (Request for
  Comments) 1950 to 1952 in the files ftp://ds.internic.net/rfc/rfc1950.txt
  (zlib format), rfc1951.txt (deflate format) and rfc1952.txt (gzip format).
*/


#include <cerrno>
#include <cstdio>

#include <sys/stat.h>
#include <fcntl.h>

#include "XrdCks/XrdCksXAttr.hh"
#include "XrdCks/XrdCks.hh"
#include "XrdOuc/XrdOucXAttr.hh"
#include "XrdOss/XrdOss.hh"
#include "XrdOss/XrdOssApi.hh"
#include "XrdVersion.hh"

#include "XrdOuc/XrdOucProg.hh"
#include "XrdOuc/XrdOucStream.hh"
#include "XrdOuc/XrdOucPinLoader.hh"
#include "XrdOuc/XrdOucEnv.hh"


#ifndef ENOATTR
#define ENOATTR ENODATA
#endif


<<<<<<< HEAD:XrdCksPlugin.cc
class XrdCksPlugin : public XrdCks
=======
#define DO1(buf)  {unSum1 += *buf++; unSum2 += unSum1;}
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);
#define DO16(buf) DO8(buf); DO8(buf);




class MyXrdCksManager : public XrdCks
{
  static const unsigned int AdlerBase  = 0xFFF1;
  static const unsigned int AdlerStart = 0x0001;
  static const          int AdlerNMax  = 5552;
  size_t max_parms_length = 4096;
  size_t buf_size = 1024;
  unsigned char* buf = NULL;
  char* oss_parms = NULL;
  XrdOss* ossp = NULL;
public:
  XrdOucProg theProg = XrdOucProg(0);

  XrdCksPlugin(XrdSysError *erP, const char* parms=NULL) : XrdCks(erP) {
    if (NULL != parms) {
      /* First parameter is buffer size */
      char* str_end;
      size_t tval = strtoul(parms, &str_end, 10);
      if (tval > 0 && errno != ERANGE) {
        buf_size = tval;
      }
      /* The rest is osslib params */
      if ('\0' != *str_end) {
        if (' ' == *str_end) {
          str_end++;
        }
        size_t parm_len = strnlen(str_end, max_parms_length);
        oss_parms = new char[parm_len + 1]; 
        strncpy(oss_parms, str_end, parm_len + 1);
      }
    }
    buf = new unsigned char[buf_size];
  };

  ~XrdCksPlugin() {
    delete [] buf;
    if (NULL != oss_parms) {
      delete [] oss_parms;
    }
  }

  /******************************************************************************/
  /*                                   G e t                                    */
  /******************************************************************************/

  int Get(const char *Pfn, XrdCksData &Cks)
  {
     /* This method is basically a copy of XrdCksManager::Get, but without stale
      * checksum check
      */
    
     XrdOucXAttr<XrdCksXAttr> xCS;
     int rc, nFault;
  
  // Determine which checksum to get (we will accept unsupported ones as well)
  //
     if (strncmp(Cks.Name, "adler32", 7) ) return -ENOTSUP;
     if (!xCS.Attr.Cks.Set(Cks.Name)) return -ENOTSUP;
  
  // Retreive the attribute
  //
     if ((rc = xCS.Get(Pfn)) <= 0) return (rc && rc != -ENOATTR ? rc : -ESRCH);
  
  // Mark state of the name and copy the attribute over
  //
     nFault = strcmp(xCS.Attr.Cks.Name, Cks.Name);
     Cks = xCS.Attr.Cks;
  
     return (nFault  ||  Cks.Length > XrdCksData::ValuSize || Cks.Length <= 0 ? -ESTALE : int(Cks.Length));
  };

  int Del(const char *Xfn, XrdCksData &Cks) {
    return -ENOTSUP;
  };

  int Calc( const char *Xfn, XrdCksData &Cks, int doSet=1) {
    unsigned int unSum1=1;
    unsigned int unSum2=0;
    unsigned int AdlerValue=0;
    unsigned char* local_buf;
    int rc=0, k=0, BLen=0;
    off_t file_size=0, offset=0;
    size_t chunk_size=0;
    XrdOssDF* oss_file=NULL;
    XrdOucEnv env;
    struct stat stat_data;

    if (!ossp) {
      return -ELIBBAD;
    }
    oss_file = ossp->newFile(Xfn);
    if (!oss_file) {
      return -ELIBACC;
    }

    /*Open file*/
    rc = oss_file->Open(Xfn, O_RDONLY, 0, env);
    if (rc != XrdOssOK) {
      return rc;
    }

    /*Stat file, to get its size*/
    rc = oss_file->Fstat(&stat_data);
    if (rc != 0) {
      return rc;
    }
    file_size = stat_data.st_size;

    /* Copied from XrdCks/XrdCksCalcadler32.hh, which in turn based on zlib.
 *   zlib copyright is applicable, see above.
 *  */
    while (offset < file_size) {
      chunk_size = buf_size <= file_size - offset ? buf_size : file_size - offset;
      BLen = oss_file->Read(buf, offset, chunk_size);
      local_buf = buf;
      printf("offset=%u, blen=%u\n", offset, BLen);
      offset += BLen;
      while(BLen > 0) {
        k = (BLen < AdlerNMax ? BLen : AdlerNMax);
        BLen -= k;
        while(k >= 16) {
          DO16(local_buf);
          k -= 16;
        }
        if (k != 0) do {DO1(local_buf);} while (--k);
        unSum1 %= AdlerBase; unSum2 %= AdlerBase;
      }
    }
    AdlerValue = (unSum2 << 16) | unSum1;
#ifndef Xrd_Big_Endian
    AdlerValue = htonl(AdlerValue);
#endif
  
    //Insert timestamp addition here
    memcpy(Cks.Value, (char*)&AdlerValue, 4);
    Cks.Length = 4;

    if (doSet) {
      XrdOucXAttr<XrdCksXAttr> xCS;
      memcpy(&xCS.Attr.Cks, &Cks, sizeof(xCS.Attr.Cks));
      if ((rc = xCS.Set(Xfn))) return -rc;
    }
    oss_file->Close();
    return 0;
  };

  int Ver(  const char *Xfn, XrdCksData &Cks) {
    return 0;
  };

  int Config(const char *Token, char *line) {
    return 1;
  };

  int Init(const char *ConfigFN, const char *DfltCalc=0) {
    XrdOucPinLoader *myLib;
    XrdOssSys myOssSys;
    XrdOssGetStorageSystem_t getOSS;

    int res = 1;
    
    if (NULL == oss_parms) {
      res = 0;
      eDest->Say("ChkLib: Failed to load osslib for checksums: probably it is not specified.");
    } else {
      //Let's separate lib path from its params. Pretty ugly way to do it.
      char lib_path[1024];
      char* lib_parms = oss_parms, *tptr=lib_path;
      while (' ' != *lib_parms && '\0' != *lib_parms) {
        *tptr = *lib_parms; 
        lib_parms++;
        tptr++;
      }
      *tptr = '\0';
      if (' ' == *lib_parms) {
        lib_parms++;
      } else {
        lib_parms = NULL;
      }

      try {
        myLib = new XrdOucPinLoader(eDest, myOssSys.myVersion, "osslib", lib_path);
      } catch(std::bad_alloc) {
        eDest->Say("ChkLib: Failed to load osslib for checksums: can not allocate memory for plugin.");
        res = 0;
      }
     
      if (1 == res) {
        getOSS = (XrdOssGetStorageSystem_t) myLib->Resolve("XrdOssGetStorageSystem");
        ossp = getOSS((XrdOss*) &myOssSys, eDest->logger(), lib_path, lib_parms);
        delete myLib;
      }
    }

    return res;
  };

  char * List(const char *Xfn, char *Buff, int Blen, char Sep=' ') {
    return NULL;
  };

  const char *Name(int seqNum=0) {
    return "adler32";
  };

  int Size (const char *Name=0) {
    return 4;
  };

  int Set(const char *Xfn, XrdCksData &Cks, int myTIme=0) {
    return -ENOTSUP;
  };
};


<<<<<<< HEAD:XrdCksPlugin.cc
extern "C" XrdCksPlugin *XrdCksInit(XrdSysError *eDest,
                                          const char  *csName,
                                          const char  *cFN,
                                          const char  *Parms) {
  return new XrdCksPlugin(eDest, cFN);
};

XrdVERSIONINFO(XrdCksInit,"MyCksums-2");
