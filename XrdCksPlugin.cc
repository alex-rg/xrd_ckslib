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

#include "XrdCks/XrdCksXAttr.hh"
#include "XrdCks/XrdCks.hh"
#include "XrdOuc/XrdOucXAttr.hh"
#include "XrdVersion.hh"

#include "XrdOuc/XrdOucProg.hh"
#include "XrdOuc/XrdOucStream.hh"


#ifndef ENOATTR
#define ENOATTR ENODATA
#endif


class XrdCksPlugin : public XrdCks
{
public:
  XrdOucProg theProg = XrdOucProg(0);

  XrdCksPlugin(XrdSysError *erP) : XrdCks(erP) {};
    int (*ppntr)(XrdOucStream*, char**, int) =0;
    theProg.Setup("/etc/xrootd/xrd_cephsum.sh", erP, ppntr);
  };

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
    int rc;
    char* ln;
    char out_buf[9];
    //fprintf(stderr, "Going to launch program");
    rc = theProg.Run(out_buf, 9, Xfn, NULL, NULL, NULL);  
    for (int i=0; i<4; i++) {
      Cks.Value[i] = 0;
      for (int j=0; j<2; j++) {
        unsigned char c;
        Cks.Value[i] += ((c = (out_buf[2*i + j] - '0')) < 10 ? c : (out_buf[2*i + j] - 'a' + 10)) * (j == 0 ? 16 : 1);
      }
    } 
    if (rc == 0) {
      //fprintf(stderr, "Prog failed: %d\n", rc);
      Cks.Length = strnlen(Cks.Value, Cks.ValuSize);
    } 
    //fprintf(stderr, "Program finished successfully, got output %s\n", out_buf);
    return rc;
  };

  int Ver(  const char *Xfn, XrdCksData &Cks) {
    return 0;
  };

  int Config(const char *Token, char *line) {
    return 1;
  };

  int Init(const char *ConfigFN, const char *DfltCalc=0) {
    return 1;
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


extern "C" XrdCksPlugin *XrdCksInit(XrdSysError *eDest,
                                          const char  *csName,
                                          const char  *cFN,
                                          const char  *Parms) {
  return new XrdCksPlugin(eDest);
};

XrdVERSIONINFO(XrdCksInit,"MyCksums-2");
