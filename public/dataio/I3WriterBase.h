/**
 *  $Id$
 *  
 *  Copyright (C) 2007
 *  Troy D. Straszheim  <troy@icecube.umd.edu>
 *  and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *  
 *  This file is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *  
 */
#ifndef I3_WRITERBASE_H_INCLUDED
#define I3_WRITERBASE_H_INCLUDED

#include <icetray/I3ConditionalModule.h>
#include <boost/iostreams/filtering_stream.hpp>
#include <zlib.h>

#include <vector>
#include <string>
#include <fstream>
#include <sstream>

template <class Derived>
class I3WriterBase : public I3ConditionalModule
{
  std::vector<std::string> skip_keys_;
  std::vector<I3Frame::Stream> streams_;
  bool configWritten_;
  int frameCounter_;

  bool write_geo_, write_cal_, write_status_, write_physics_, write_trayinfo_;

  // yes, static_cast is bad, but this is the Curiously Recurring Template Pattern,
  // google it for details
  inline Derived* derived() { return static_cast<Derived*>(this); }

  void WriteConfig(I3FramePtr);

protected:

  int gzip_compression_level_;
  
  boost::iostreams::filtering_ostream filterstream_;
  std::string path_;

public:

  I3WriterBase(const I3Context& ctx);

  virtual ~I3WriterBase() { }

  void Configure();
  void Finish();

  void Process();
};



#endif
