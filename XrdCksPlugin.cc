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

#include <cerrno>
#include <cstdio>
#include <sys/stat.h>

#include "XrdCks/XrdCksXAttr.hh"
#include "XrdCks/XrdCks.hh"
#include "XrdOuc/XrdOucXAttr.hh"
#include "XrdVersion.hh"

#include "XrdOuc/XrdOucProg.hh"
#include "XrdOuc/XrdOucStream.hh"


#ifndef ENOATTR
#define ENOATTR ENODATA
#endif

//Maximum number of bytes checksum script may print
#define MAX_CKS_SCRIPT_OUTPUT_LENGHT 1023
#define MAX_CKS_NAME_LENGTH 7

#define SUPPORTED_CHECKSUM_LENGTH 4
const char SUPPORTED_CHECKSUM[] = "adler32";
/* Currently we support only one checksum type.
 * To add more, one should modify the following methods: 
 *
 * Get  -- to allow another type
 * Calc -- to support different length
 * Name -- to support different name
 * Size -- to support different size
 * */


class XrdCksPlugin : public XrdCks
{
  int prog_ready = 0;
public:
  XrdOucProg theProg = XrdOucProg(0);

  XrdCksPlugin(XrdSysError *erP, const char* prog) : XrdCks(erP) {
    if (prog) {
      struct stat st_buf;
      if (stat(prog, &st_buf) == 0) {
        int (*ppntr)(XrdOucStream*, char**, int) =0;
        theProg.Setup(prog, erP, ppntr);
        prog_ready = 1;
      } else {
        eDest->Emsg("CksLib: Init failed. Can not stat checksum script", prog);
      }
    } else {
      eDest->Say("CksLib: Init failed. Checksum script path not given");
    }
  };

  /******************************************************************************/
  /*                                   G e t                                    */
  /******************************************************************************/

  int Get(const char *Pfn, XrdCksData &Cks)
  {
     /**
      * Retrieve checksum value from metadata
      *
      * @parm Pfn  path of the file whose checksum is to be retrieved
      * @parm Cks  Checksum object to fill
      *
      * @return   Length of the retrieved checksum (in bytes) on success,
      *           negative error code on failure, namely:
      *           ENOTSUP (op not supported)  -- if requested checksum type is not supported
      *           ESRCH (no such process)     -- if checksum attirbute is not present
      *           ESTALE (stale file handle)  -- if checksum type written in the attributes differs from supported one
      *                                          or its length differs from the expected one.
      *
      *
      * This method is basically a copy of XrdCksManager::Get, but without stale checksum check
      */
    
     XrdOucXAttr<XrdCksXAttr> xCS;
     int rc, nFault;
  
  // Check that we support given checksum type
     if (strncmp(Cks.Name, SUPPORTED_CHECKSUM, MAX_CKS_NAME_LENGTH) ) return -ENOTSUP;
     if (!xCS.Attr.Cks.Set(Cks.Name)) return -ENOTSUP;
  
  // Retreive the attribute
  //
     if ((rc = xCS.Get(Pfn)) <= 0) return (rc && rc != -ENOATTR ? rc : -ESRCH);
  
  // Mark state of the name and copy the attribute over
  //
     nFault = strcmp(xCS.Attr.Cks.Name, Cks.Name);
     Cks = xCS.Attr.Cks;
  
     return (nFault || Cks.Length > XrdCksData::ValuSize || Cks.Length <= 0 ? -ESTALE : int(Cks.Length));
  };

  int Del(const char *Xfn, XrdCksData &Cks) {
    return -ENOTSUP;
  };

  int Calc( const char *Xfn, XrdCksData &Cks, int doSet=1) {
     /**
      * Run checksum calculation script, read its output and use it as a checksum value
      *
      * @parm Xfn    path of the file whose checksum is to be calculated
      * @parm Cks    Checksum object to fill
      * @parm doSet  Ignored
      *
      * @return  exit code from script execution
      *
      * This method is basically a copy of XrdCksManager::Get, but without stale checksum check
      */

    int rc;
    char out_buf[MAX_CKS_SCRIPT_OUTPUT_LENGHT + 1];
    uint32_t AdlerValue;
    rc = theProg.Run(out_buf, MAX_CKS_SCRIPT_OUTPUT_LENGHT, Xfn, NULL, NULL, NULL);  
    if (rc != 0) {
      eDest->Emsg("CksLib: checksum caclulation failed for ", Xfn, "Error message: ", out_buf);
    } else {
      AdlerValue = strtoull(out_buf, NULL, 16);
#ifndef Xrd_Big_Endian
      AdlerValue = htonl(AdlerValue);
#endif
      memcpy(Cks.Value, (char*)&AdlerValue, SUPPORTED_CHECKSUM_LENGTH);
      Cks.Length = SUPPORTED_CHECKSUM_LENGTH;
    }
    return rc;
  };

  int Ver(  const char *Xfn, XrdCksData &Cks) {
    return 0;
  };

  int Config(const char *Token, char *line) {
    return 1;
  };

  int Init(const char *ConfigFN, const char *DfltCalc=0) {
    return prog_ready;
  };

  char * List(const char *Xfn, char *Buff, int Blen, char Sep=' ') {
    return NULL;
  };

  const char *Name(int seqNum=0) {
    return SUPPORTED_CHECKSUM;
  };

  int Size (const char *Name=0) {
    return SUPPORTED_CHECKSUM_LENGTH;
  };

  int Set(const char *Xfn, XrdCksData &Cks, int myTIme=0) {
    return -ENOTSUP;
  };
};


extern "C" XrdCksPlugin *XrdCksInit(XrdSysError *eDest,
                                          const char  *csName,
                                          const char  *cFN,
                                          const char  *Parms) {
  return new XrdCksPlugin(eDest, cFN);
};

XrdVERSIONINFO(XrdCksInit,"MyCksums-2");
