// -*- C++ -*-

// PrPStreamBuf.cc
//
// Copyright (C) 2004 Christian Hudon 
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// 
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org

/* *******************************************************      
   * $Id: PrPStreamBuf.cc,v 1.7 2005/06/15 14:41:13 plearner Exp $ 
   ******************************************************* */

// Authors: Christian Hudon

/*! \file PrPStreamBuf.cc */


#include "PrPStreamBuf.h"
#include <mozilla/nspr/prio.h>
#include <stdio.h>

namespace PLearn {
using namespace std;

  PrPStreamBuf::PrPStreamBuf(PRFileDesc* in_, PRFileDesc* out_,
                             bool own_in_, bool own_out_)
    : PStreamBuf(in_ != 0, out_ != 0, 4096, 4096, default_ungetsize), 
      in(in_), out(out_), own_in(own_in_), own_out(own_out_)
  {}

  PrPStreamBuf::~PrPStreamBuf()
  {
    const bool in_and_out_equal = (in == out);
    
    try {
      flush();
    }
    catch(...)
      {
        fprintf(stderr,"Could not properly clean up (flush) in destructor of PrPStreamBuf\n");
      }
    if (in && own_in)
      {
        PR_Close(in);
        in = 0;
      }
    if (out && own_out)
      {
        // If "in" and "out" were pointing to the same PRFileDesc,
        // don't close it a second time.
        if (!in_and_out_equal)
          PR_Close(out);
        out = 0;
      }
  }

  PrPStreamBuf::streamsize PrPStreamBuf::read_(char* p, streamsize n)
  {
    return PR_Read(in, p, n);
  }

  //! writes exactly n characters from p (unbuffered, must flush)
  void PrPStreamBuf::write_(const char* p, streamsize n)
  {
    streamsize nwritten = ::PR_Write(out, p, n);
    if (nwritten != n)
      PLERROR("In PrPStreamBuf::write_ failed to write the requested number of bytes: wrote %d instead of %d",nwritten,n);
  }
  
} // end of namespace PLearn
