/*
   mkvextract -- extract tracks from Matroska files into other files

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   $Id$

   extracts tracks from Matroska files into other files

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __XTR_TEXTSUBS_H
#define __XTR_TEXTSUBS_H

#include "os.h"

#include <string>
#include <vector>

#include "xml_element_writer.h"

#include "xtr_base.h"

using namespace std;

class xtr_srt_c: public xtr_base_c {
public:
  int num_entries;
  string sub_charset;
  int conv;

public:
  xtr_srt_c(const string &_codec_id, int64_t _tid, track_spec_t &tspec);

  virtual void create_file(xtr_base_c *_master, KaxTrackEntry &track);
  virtual void handle_block(KaxBlock &block, KaxBlockAdditions *additions,
                            int64_t timecode, int64_t duration, int64_t bref,
                            int64_t fref);
};

class xtr_ssa_c: public xtr_base_c {
public:
  class ssa_line_c {
  public:
    string line;
    int num;

    bool operator < (const ssa_line_c &cmp) const {
      return num < cmp.num;
    }
  };

  vector<string> ssa_format;
  vector<ssa_line_c> lines;
  string sub_charset;
  int conv;
  bool warning_printed;

  static const char *kax_ssa_fields[10];

public:
  xtr_ssa_c(const string &_codec_id, int64_t _tid, track_spec_t &tspec);

  virtual void create_file(xtr_base_c *_master, KaxTrackEntry &track);
  virtual void handle_block(KaxBlock &block, KaxBlockAdditions *additions,
                            int64_t timecode, int64_t duration, int64_t bref,
                            int64_t fref);
  virtual void finish_file();
};

class xtr_usf_c: public xtr_base_c {
private:
  struct usf_entry_t {
    string m_text;
    int64_t m_start, m_end;

    usf_entry_t(const string &text, int64_t start, int64_t end):
      m_text(text), m_start(start), m_end(end) { }
  };

  string m_sub_charset, m_codec_private, m_language;
  counted_ptr<xml_formatter_c> m_formatter;
  vector<usf_entry_t> m_entries;

public:
  xtr_usf_c(const string &_codec_id, int64_t _tid, track_spec_t &tspec);

  virtual void create_file(xtr_base_c *_master, KaxTrackEntry &track);
  virtual void handle_block(KaxBlock &block, KaxBlockAdditions *additions,
                            int64_t timecode, int64_t duration, int64_t bref,
                            int64_t fref);
  virtual void finish_track();
  virtual void finish_file();
};

#endif
