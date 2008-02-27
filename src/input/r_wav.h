/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   $Id$

   class definitions for the WAV reader module

   Written by Moritz Bunkus <moritz@bunkus.org>.
   Modified by Peter Niemayer <niemayer@isg.de>.
*/

#ifndef __R_WAV_H
#define __R_WAV_H

#include "os.h"

#include <stdio.h>

#include "common.h"
#include "dts_common.h"
#include "error.h"
#include "mm_io.h"
#include "pr_generic.h"

extern "C" {
#include "avilib.h"
}

class wav_reader_c;

class wav_demuxer_c {
public:
  wav_reader_c         *reader;
  wave_header          *wheader;
  generic_packetizer_c *ptzr;

public:
  wav_demuxer_c(wav_reader_c *n_reader, wave_header *n_wheader):
    reader(n_reader), wheader(n_wheader), ptzr(NULL) {
  };
  virtual ~wav_demuxer_c() {};

  virtual int64_t get_preferred_input_size() = 0;
  virtual unsigned char *get_buffer() = 0;
  virtual void process(int64_t len) = 0;

  virtual generic_packetizer_c *create_packetizer() = 0;

  virtual string get_codec() = 0;

  virtual bool probe(mm_io_cptr &io) = 0;
};

typedef counted_ptr<wav_demuxer_c> wav_demuxer_cptr;

struct wav_chunk_t {
  int64_t pos;
  char id[4];
  uint32_t len;
};

class wav_reader_c: public generic_reader_c {
private:
  mm_io_cptr io;
  struct wave_header wheader;
  int64_t bytes_processed, bytes_in_data_chunks, remaining_bytes_in_current_data_chunk;

  vector<wav_chunk_t> chunks;
  int cur_data_chunk_idx;

  wav_demuxer_cptr demuxer;

public:
  wav_reader_c(track_info_c &_ti) throw (error_c);
  virtual ~wav_reader_c();

  virtual file_status_e read(generic_packetizer_c *ptzr, bool force = false);
  virtual void identify();
  virtual void create_packetizer(int64_t tid);

  virtual int get_progress();

  static int probe_file(mm_io_c *io, int64_t size);

protected:
  void scan_chunks();
  int find_chunk(const char *id, int start_idx = 0, bool allow_empty = true);

  void parse_file();
  void create_demuxer();
};

#endif // __R_WAV_H
