/*
  mkvmerge -- utility for splicing together matroska files
      from component media subtypes

  p_video.h

  Written by Moritz Bunkus <moritz@bunkus.org>

  Distributed under the GPL
  see the file COPYING for details
  or visit http://www.gnu.org/copyleft/gpl.html
*/

/*!
    \file
    \version \$Id: p_video.cpp,v 1.2 2003/02/19 09:31:24 mosu Exp $
    \brief video output module
    \author Moritz Bunkus         <moritz @ bunkus.org>
*/

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "queue.h"
#include "p_video.h"

#include "KaxTracks.h"
#include "KaxTrackVideo.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

video_packetizer_c::video_packetizer_c(void *pr_data, int pd_size,
                                       char *ncodec, double nfps, int nwidth,
                                       int nheight, int nbpp,
                                       int nmax_frame_size, audio_sync_t *as,
                                       range_t *nrange, int navi_compat_mode)
                                       throw (error_c) : q_c() {
  packetno = 0;
  memcpy(codec, ncodec, 4);
  codec[4] = 0;
  fps = nfps;
  width = nwidth;
  height = nheight;
  bpp = nbpp;
  max_frame_size = nmax_frame_size;
  tempbuf = (char *)malloc(max_frame_size + 1);
  memcpy(&range, nrange, sizeof(range_t));
  if (tempbuf == NULL)
    die("malloc");
  range.start *= fps;
  range.end *= fps;
  avi_compat_mode = navi_compat_mode;
  frames_output = 0;
  set_private_data(pr_data, pd_size);
  set_header();
//  add_index(serialno);
}

void video_packetizer_c::set_header() {
  using namespace LIBMATROSKA_NAMESPACE;
  int set_codec_private;
  char *codecstr;
  
  if (kax_last_entry == NULL)
    track_entry =
      &GetChild<KaxTrackEntry>(static_cast<KaxTracks &>(*kax_tracks));
  else
    track_entry =
      &GetNextChild<KaxTrackEntry>(static_cast<KaxTracks &>(*kax_tracks),
        static_cast<KaxTrackEntry &>(*kax_last_entry));
  kax_last_entry = track_entry;
  
  serialno = track_number++;
  KaxTrackNumber &tnumber =
    GetChild<KaxTrackNumber>(static_cast<KaxTrackEntry &>(*track_entry));
  *(static_cast<EbmlUInteger *>(&tnumber)) = serialno;
  
  *(static_cast<EbmlUInteger *>
    (&GetChild<KaxTrackType>(static_cast<KaxTrackEntry &>(*track_entry)))) =
      track_video;

  KaxCodecID &codec_id =
    GetChild<KaxCodecID>(static_cast<KaxTrackEntry &>(*track_entry));
  if (!avi_compat_mode) {
    codecstr = map_video_codec_fourcc(codec, &set_codec_private);
    codec_id.CopyBuffer((binary *)codecstr, countof(codecstr));
  } else
    codec_id.CopyBuffer((binary *)"V_MS/VFW/FOURCC",
                        countof("V_MS/VFW/FOURCC"));
  if (set_codec_private || avi_compat_mode) {
    KaxCodecPrivate &codec_private = 
      GetChild<KaxCodecPrivate>(static_cast<KaxTrackEntry &>(*track_entry));
    codec_private.CopyBuffer((binary *)private_data, private_data_size);
  }

  KaxTrackVideo &track_video =
    GetChild<KaxTrackVideo>(static_cast<KaxTrackEntry &>(*track_entry));

  KaxVideoPixelHeight &pheight = GetChild<KaxVideoPixelHeight>(track_video);
  *(static_cast<EbmlUInteger *>(&pheight)) = height;

  KaxVideoPixelWidth &pwidth = GetChild<KaxVideoPixelWidth>(track_video);
  *(static_cast<EbmlUInteger *>(&pwidth)) = width;

  KaxVideoFrameRate &frate = GetChild<KaxVideoFrameRate>(track_video);
  *(static_cast<EbmlFloat *>(&frate)) = fps;
}

int video_packetizer_c::process(char *buf, int size, int num_frames,
                                int key, int last_frame) {
  if (size > max_frame_size) {
    fprintf(stderr, "FATAL: p_video: size (%d) > max_frame_size (%d)\n",
            size, max_frame_size);
    exit(1);
  }

  if ((packetno >= range.start) &&
      ((range.end == 0) || (packetno < range.end))) {
    add_packet(buf, size, (u_int64_t)(1000.0 * frames_output / fps), key);
    frames_output += num_frames;
  }
  packetno++;
    
  return EMOREDATA;
}

video_packetizer_c::~video_packetizer_c() {
  if (tempbuf != NULL)
    free(tempbuf);
}
