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
#include <dataio/I3MultiWriter.h>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/format.hpp>

#include <icetray/counter64.hpp>
#include <icetray/open.h>

using boost::algorithm::to_lower;

namespace io = boost::iostreams;
namespace dataio = I3::dataio;

I3_MODULE(I3MultiWriter);

I3MultiWriter::I3MultiWriter(const I3Context& ctx) 
  : I3WriterBase(ctx), size_limit_(0), sync_seen_(false), file_counter_(0)
{ 
  AddParameter("SizeLimit",
	       "Soft Size limit in bytes.  Files will typically exceed this limit by the size of one half of one frame.",
	       size_limit_);
  AddParameter("SyncStream",
	       "Frame type to wait for to split files. New files will always begin with a frame of this type. Useful for frames from which events need to inherit (e.g. DAQ and DetectorStatus frames).",
	       I3Frame::DAQ);
}

I3MultiWriter::~I3MultiWriter() { }

std::string 
I3MultiWriter::current_path()
{
  boost::format f(path_);
  try {
    f % file_counter_;
  } catch (const std::exception& e) {
    log_error("Exception caught: %s", e.what());
    log_error("Does your Filename contain a printf-style specifier where the number should go, e.g., '%s'?",
	      "myfile-%04u.i3.gz");
    throw;
  }
  return f.str();
}


void
I3MultiWriter::Configure_()
{
  log_trace("%s", __PRETTY_FUNCTION__);
  I3ConditionalModule::Configure_();

  GetParameter("SizeLimit", size_limit_);
  if (size_limit_ == 0)
    log_fatal("SizeLimit (%llu) must be > 0", (unsigned long long)size_limit_);
  GetParameter("SyncStream", sync_stream_);

  log_trace("path_=%s", path_.c_str());
  log_debug("Starting new file '%s'", current_path().c_str());
  dataio::open(filterstream_, current_path(), gzip_compression_level_);
}

void 
I3MultiWriter::NewFile()
{
  log_trace("%s", __PRETTY_FUNCTION__);

  file_counter_++;

  log_info("Starting new file '%s'", current_path().c_str());
  dataio::open(filterstream_, current_path(), gzip_compression_level_);
}

void
I3MultiWriter::Process()
{
  I3FramePtr frame;

  log_trace("%s", __PRETTY_FUNCTION__);

  uint64_t bytes_written;

  io::counter64* ctr = filterstream_.component<io::counter64>(filterstream_.size() - 2);
  if (!ctr) log_fatal("couldnt get counter from stream");
  bytes_written = ctr->characters();

  log_trace("%llu bytes: %s", (unsigned long long)bytes_written, current_path().c_str());

  frame = PeekFrame();
  if (frame == NULL)
    return;

  if (frame->GetStop() == sync_stream_)
    sync_seen_ = true;

  if (bytes_written > size_limit_ && (frame->GetStop() == sync_stream_ ||
    !sync_seen_))
      NewFile();

  I3WriterBase::Process();
}

void
I3MultiWriter::Finish()
{
  log_trace("%s", __PRETTY_FUNCTION__);

  uint64_t lastfile_bytes;
  io::counter64* ctr = filterstream_.component<io::counter64>(filterstream_.size() - 2);
  if (!ctr) log_fatal("couldnt get counter from stream");
  lastfile_bytes = ctr->characters();

  filterstream_.reset();

  log_trace("lastfile bytes=%llu", (unsigned long long)lastfile_bytes);

  if (lastfile_bytes == 0)
    {
      log_trace("unlinking %s", current_path().c_str());
      unlink(current_path().c_str());
    }

  I3WriterBase::Finish();
}

