/*
 * H.265 video codec.
 * Copyright (c) 2013-2014 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * This file is part of libde265.
 *
 * libde265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libde265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libde265.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>

#include <sys/time.h>

#ifndef WIN32
#include <sys/times.h>
#endif

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>



static struct {
  const char* name;
  const char* value;
} variables[] = {
  { "$HOME"    , "/home/domain/farindk" },
  { "$ROOT"    , "/home/domain/farindk/prog/h265" },
  { "$ENC265"  , "$ROOT/libde265/enc265/enc265" },
  { "$DEC265"  , "$ROOT/libde265/dec265/dec265" },
  { "$YUVDIST" , "$ROOT/libde265/tools/yuv-distortion" },
  { "$YUVTMP"  , "/mnt/temp/dirk/yuv/ftp.tnt.uni-hannover.de/testsequences" },
  { "$YUV"     , "/storage/users/farindk/yuv" },
  { "$HMENC"   , "HM13enc" },
  { "$HM13CFG" , "$ROOT/HM/HM-13.0-dev/cfg" },
  { "$HMSCCENC", "HM-SCC-enc" },
  { "$HMSCCCFG", "$ROOT/HM/HM-SCC-extensions/cfg" },
  { "$X265ENC" , "$ROOT/x265/build/linux/x265" },
  { "$X264"    , "x264" },
  { "$FFMPEG"  , "ffmpeg" },
  { "$F265"    , "$ROOT/f265/build/f265cli" },
  { 0,0 }
};


bool keepStreams = false;
int  maxFrames = 0;
std::string encoderParameters;


std::string replace_variables(std::string str)
{
  bool replaced = false;
  for (int i=0;variables[i].name;i++) {
    size_t pos = str.find(variables[i].name);
    if (pos != std::string::npos) {
      replaced = true;
      str = str.replace(pos, strlen(variables[i].name), variables[i].value);
      break;
    }
  }

  if (!replaced) return str;
  else return replace_variables(str);
}


// ---------------------------------------------------------------------------

struct Preset
{
  const int ID;
  const char* name;
  const char* descr;

  const char* options_de265;
  const char* options_hm;
  const char* options_hm_scc;
  const char* options_x265;
  const char* options_f265;
  const char* options_x264;
  const char* options_x264_ffmpeg;
  const char* options_ffmpeg_mpeg2;

  //int nFrames;
};


Preset preset[] = {
  { 1, "pre01-intra-noLF", "intra, no LF, no SBH, CTB-size 32, min CB=8",
    /* de265  */ "--sop-structure intra",
    /* HM     */ "-c $HM13CFG/encoder_intra_main.cfg -SBH 0 --SAO=0 --LoopFilterDisable --DeblockingFilterControlPresent --MaxCUSize=32 --MaxPartitionDepth=2",
    /* HM SCC */ "-c $HMSCCCFG/encoder_intra_main_scc.cfg -SBH 0 --SAO=0 --LoopFilterDisable --DeblockingFilterControlPresent --MaxCUSize=32 --MaxPartitionDepth=2",
    /* x265   */ "--no-lft -I 1 --no-signhide",
    /* f265   */ "key-frame-spacing=1",
    /* x264   */ "-I 1",
    /* ffmpeg */ "-g 1",
    /* mpeg-2 */ "-g 1"
    // 0 // all frames
  },

  { 2, "pre02-fastIntra", "intra, no LF, no SBH, CTB-size 32, min CB=8",
    /* de265  */ "--sop-structure intra --TB-IntraPredMode minSSD",
    /* HM     */ "-c $HM13CFG/encoder_intra_main.cfg -SBH 0 --SAO=0 --LoopFilterDisable --DeblockingFilterControlPresent --MaxCUSize=32 --MaxPartitionDepth=2",
    /* HM SCC */ "-c $HMSCCCFG/encoder_intra_main_scc.cfg -SBH 0 --SAO=0 --LoopFilterDisable --DeblockingFilterControlPresent --MaxCUSize=32 --MaxPartitionDepth=2",
    /* x265   */ "--no-lft -I 1 --no-signhide",
    /* f265   */ "key-frame-spacing=1",
    /* x264   */ "-I 1",
    /* ffmpeg */ "-g 1",
    /* mpeg-2 */ "-g 1"
    // 0 // all frames
  },

  { 3, "pre03-fastIntra", "pre02, but fast-brute",
    /* de265  */ "--sop-structure intra --TB-IntraPredMode fast-brute",
    /* HM     */ "-c $HM13CFG/encoder_intra_main.cfg -SBH 0 --SAO=0 --LoopFilterDisable --DeblockingFilterControlPresent --MaxCUSize=32 --MaxPartitionDepth=2",
    /* HM SCC */ "-c $HMSCCCFG/encoder_intra_main_scc.cfg -SBH 0 --SAO=0 --LoopFilterDisable --DeblockingFilterControlPresent --MaxCUSize=32 --MaxPartitionDepth=2",
    /* x265   */ "--no-lft -I 1 --no-signhide",
    /* f265   */ "key-frame-spacing=1",
    /* x264   */ "-I 1",
    /* ffmpeg */ "-g 1",
    /* mpeg-2 */ "-g 1"
    // 0 // all frames
  },

  { 50, "cb-auto16", "(development test)",
    /* de265  */ "--max-cb-size 16 --min-cb-size 8",
    /* HM     */ "-c $HM13CFG/encoder_intra_main.cfg -SBH 0 --SAO=0 --LoopFilterDisable --DeblockingFilterControlPresent --MaxCUSize=32 --MaxPartitionDepth=2",
    /* HM SCC */ "-c $HMSCCCFG/encoder_intra_main_scc.cfg -SBH 0 --SAO=0 --LoopFilterDisable --DeblockingFilterControlPresent --MaxCUSize=32 --MaxPartitionDepth=2",
    /* x265   */ "--no-lft -I 1 --no-signhide",
    /* f265   */ "key-frame-spacing=1",
    /* x264   */ "-I 1",
    /* ffmpeg */ "-g 1",
    /* mpeg-2 */ "-g 1"
    // 0 // all frames
  },

  { 80, "lowdelay", "default (low-default) encoder parameters",
    "--MEMode search --max-cb-size 32 --min-cb-size 8 --min-tb-size 4 --CB-IntraPartMode-Fixed-partMode 2Nx2N --CB-IntraPartMode fixed --TB-IntraPredMode min-residual --PB-MV-TestMode zero",
    /* de265  */ //"--sop-structure low-delay --MEMode search --max-cb-size 32 --min-cb-size 8 --min-tb-size 4 --CB-IntraPartMode fixed --TB-IntraPredMode min-residual",
    /* HM     */ "-c $HM13CFG/encoder_lowdelay_main.cfg -ip 248",
    /* HM SCC */ "-c $HMSCCCFG/encoder_lowdelay_main_scc.cfg -ip 248",
    /* x265   */ "-I 248 --no-wpp --bframes 0", // GOP size: 248
    /* f265   */ 0, //"key-frame-spacing=248",
    /* x264   */ "",
    /* ffmpeg */ "-g 248 -bf 0",
    /* mpeg-2 */ "" // GOP size 248 does not make sense here
    // 0 // all frames
  },

  { 98, "best", "default (random-access) encoder parameters",
    /* de265  */ "--max-cb-size 16 --min-cb-size 8",
    /* HM     */ "-c $HM13CFG/encoder_randomaccess_main.cfg",
    /* HM SCC */ "-c $HMSCCCFG/encoder_randomaccess_main_scc.cfg",
    /* x265   */ "",
    /* f265   */ "",
    /* x264   */ "",
    /* ffmpeg */ "",
    /* mpeg-2 */ ""
    // 0 // all frames
  },

  { 99, "besteq", "default (random-access) encoder parameters, I-frame distance = 248",
    /* de265  */ "",
    /* HM     */ "-c $HM13CFG/encoder_randomaccess_main.cfg -ip 248",
    /* HM SCC */ "-c $HMSCCCFG/encoder_randomaccess_main_scc.cfg -ip 248",
    /* x265   */ "-I 248 --no-wpp", // GOP size: 248
    /* f265   */ "key-frame-spacing=248",
    /* x264   */ "",
    /* ffmpeg */ "-g 248",
    /* mpeg-2 */ "" // GOP size 248 does not make sense here
    // 0 // all frames
  },

  { 0, NULL }
};

// ---------------------------------------------------------------------------

class Input
{
public:
  Input() {
    width=height=0;
    maxFrames=0;
  }

  void setInput(const char* yuvfilename,int w,int h, float fps) {
    mInputFilename = yuvfilename;
    width = w;
    height = h;
    mFPS = fps;
  }

  void setMaxFrames(int n) { maxFrames=n; }

  std::string options_de265() const {
    std::stringstream sstr;
    sstr << " -i " << mInputFilename << " --width " << width << " --height " << height;
    if (maxFrames) sstr << " --frames " << maxFrames;

    return sstr.str();
  }

  std::string options_HM() const {
    std::stringstream sstr;
    sstr << "-i " << mInputFilename << " -wdt " << width << " -hgt " << height
         << " -fr " << mFPS;
    if (maxFrames) sstr << " -f " << maxFrames;

    return sstr.str();
  }

  std::string options_x265() const {
    std::stringstream sstr;
    sstr << mInputFilename << " --input-res " << width << "x" << height
         << " --fps " << mFPS;
    if (maxFrames) sstr << " -f " << maxFrames;

    return sstr.str();
  }

  std::string options_x264() const {
    std::stringstream sstr;
    sstr << mInputFilename << " --input-res " << width << "x" << height;
    sstr << " --fps 25"; // TODO: check why crf/qp rate-control freaks out when fps is != 25
    if (maxFrames) sstr << " --frames " << maxFrames;

    return sstr.str();
  }

  std::string options_ffmpeg() const {
    std::stringstream sstr;
    sstr << "-f rawvideo -vcodec rawvideo -s " << width << "x" << height; // << " -r " << mFPS
    sstr << " -pix_fmt yuv420p -i " << mInputFilename;
    if (maxFrames) sstr << " -vframes " << maxFrames;

    return sstr.str();
  }

  std::string options_f265() const {
    std::stringstream sstr;
    sstr << mInputFilename << " -w " << width << ":" << height;
    if (maxFrames) sstr << " -c " << maxFrames;

    return sstr.str();
  }

  std::string getFilename() const { return mInputFilename; }
  float getFPS() const { return mFPS; }
  int   getNFrames() const { return maxFrames; }
  int   getWidth() const { return width; }
  int   getHeight() const { return height; }

private:
  std::string mInputFilename;
  int width, height;
  int maxFrames;
  float mFPS;
};

Input input;

struct InputSpec
{
  const char* name;
  const char* filename;
  int width,height, nFrames;
  float fps;
} inputSpec[] = {
  { "paris",       "$YUV/paris_cif.yuv",352,288,1065, 30.0 },
  { "paris10",     "$YUV/paris_cif.yuv",352,288,  10, 30.0 },
  { "paris100",    "$YUV/paris_cif.yuv",352,288, 100, 30.0 },
  { "johnny",      "$YUV/Johnny_1280x720_60.yuv",1280,720,600,60.0 },
  { "johnny10",    "$YUV/Johnny_1280x720_60.yuv",1280,720, 10,60.0 },
  { "johnny100",   "$YUV/Johnny_1280x720_60.yuv",1280,720,100,60.0 },
  { "cactus",      "$YUV/Cactus_1920x1080_50.yuv",1920,1080,500,50.0 },
  { "cactus10",    "$YUV/Cactus_1920x1080_50.yuv",1920,1080, 10,50.0 },
  { "4people",     "$YUVTMP/FourPeople_1280x720_60.yuv",1280,720,600,60.0 },
  { "4people100",  "$YUVTMP/FourPeople_1280x720_60.yuv",1280,720,100,60.0 },
  { "slideedit",   "$YUVTMP/SlideEditing_1280x720_30.yuv",1280,720,300,30.0 },
  { "slideedit100","$YUVTMP/SlideEditing_1280x720_30.yuv",1280,720,100,30.0 },
  { "slideshow",   "$YUVTMP/SlideShow_1280x720_20.yuv",1280,720,500,20.0 },
  { "slideshow100","$YUVTMP/SlideShow_1280x720_20.yuv",1280,720,100,20.0 },
  { "screensharing","$HOME/test-screensharing-encoding/Screensharing.yuv",1360,768,4715,60.0 },
  { NULL }
};


void setInput(const char* input_preset)
{
  bool presetFound=false;

  for (int i=0;inputSpec[i].name;i++) {
    if (strcmp(input_preset, inputSpec[i].name)==0) {
      input.setInput(inputSpec[i].filename,
                     inputSpec[i].width,
                     inputSpec[i].height,
                     inputSpec[i].fps);
      input.setMaxFrames(inputSpec[i].nFrames);
      presetFound=true;
      break;
    }
  }

  if (!presetFound) {
    fprintf(stderr,"no input preset '%s'\n",input_preset);
    exit(5);
  }
}


float bitrate(const char* filename)
{
  struct stat s;
  stat(filename,&s);

  long size = s.st_size;

  int frames = input.getNFrames();
  assert(frames!=0);

  float bitrate = size*8/(frames/input.getFPS());
  return bitrate;
}


// ---------------------------------------------------------------------------

class Quality
{
public:
  virtual ~Quality() { }

  virtual void measure(const char* h265filename);
  virtual void measure_yuv(const char* yuvfilename);

  float psnr, ssim;
};


void Quality::measure(const char* h265filename)
{
  std::stringstream sstr;
  sstr << "$DEC265 " << h265filename << " -q -t6 -m " << input.getFilename() << " | grep total "
    //"| awk '{print $2}' "
    ">/tmp/xtmp";

  //std::cout << sstr.str() << "\n";
  int retval = system(replace_variables(sstr.str()).c_str());

  std::ifstream istr;
  istr.open("/tmp/xtmp");
  std::string dummy;
  istr >> dummy >> psnr >> dummy >> dummy >> ssim;

  unlink("/tmp/xtmp");
}


void Quality::measure_yuv(const char* yuvfilename)
{
  std::stringstream sstr;

  sstr << "$YUVDIST " << input.getFilename() << " " << yuvfilename
       << " " << input.getWidth() << " " << input.getHeight()
       << "|grep total "
    //"|awk '{print $2}' "
    ">/tmp/ytmp";

  //std::cout << sstr.str() << "\n";
  int retval = system(replace_variables(sstr.str()).c_str());

  std::ifstream istr;
  istr.open("/tmp/ytmp");
  std::string dummy;
  istr >> dummy >> psnr >> ssim;

  unlink("/tmp/ytmp");
}

Quality quality;

// ---------------------------------------------------------------------------

long ticks_per_second;

void init_clock()
{
#ifndef WIN32
  ticks_per_second = sysconf(_SC_CLK_TCK);
#endif
}

double get_cpu_time()
{
#ifndef WIN32
  struct tms t;
  times(&t);
  return double(t.tms_cutime)/ticks_per_second;
#else
  return 0; // not supported on windows (TODO)
#endif
}

double get_wall_time()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double t  = tv.tv_sec;
  double ut = tv.tv_usec/1000000.0f;
  t += ut;
  return t;
}


struct RDPoint
{
  float rate;
  float psnr;
  float ssim;
  double cpu_time; // computation time in seconds
  double wall_time;


  RDPoint() { }

  void compute_from_h265(std::string stream_name) {
    rate = bitrate(stream_name.c_str());
    quality.measure(stream_name.c_str());
    psnr = quality.psnr;
    ssim = quality.ssim;
  }

  void compute_from_yuv(std::string stream_name, std::string yuv_name) {
    rate = bitrate(stream_name.c_str());
    quality.measure_yuv(yuv_name.c_str());
    psnr = quality.psnr;
    ssim = quality.ssim;
  }

  void start_timer() {
    cpu_time = get_cpu_time();
    wall_time= get_wall_time();
  }

  void end_timer() {
    cpu_time = get_cpu_time() - cpu_time;
    wall_time= get_wall_time()- wall_time;
  }
};


FILE* output_fh;

void write_rd_line(RDPoint p)
{
  fprintf(output_fh,"%9.2f %6.4f %5.3f %5.4f %5.4f\n",
          p.rate/1024, p.psnr, p.ssim,
          p.cpu_time/60, p.wall_time/60);
  fflush(output_fh);
}




class Encoder
{
public:
  virtual ~Encoder() { }

  virtual std::vector<RDPoint> encode_curve(const Preset& preset) const = 0;

private:
};


class Encoder_de265 : public Encoder
{
public:
  Encoder_de265();
  void setQPRange(int low,int high,int step) { mQPLow=low; mQPHigh=high; mQPStep=step; }

  virtual std::vector<RDPoint> encode_curve(const Preset& preset) const;

private:
  RDPoint encode(const Preset& preset,int qp) const;

  int mQPLow,mQPHigh,mQPStep;
};


Encoder_de265::Encoder_de265()
{
  mQPLow = 14;
  mQPHigh= 40;
  mQPStep=  2;
}


std::vector<RDPoint> Encoder_de265::encode_curve(const Preset& preset) const
{
  std::vector<RDPoint> curve;

  for (int qp=mQPHigh ; qp>=mQPLow ; qp-=mQPStep) {
    curve.push_back(encode(preset, qp));
  }

  return curve;
}


RDPoint Encoder_de265::encode(const Preset& preset,int qp) const
{
  std::stringstream streamname;
  streamname << "de265-" << preset.name << "-" << qp << ".265";

  std::stringstream cmd1;
  cmd1 << "$ENC265 " << input.options_de265()
       << " " << preset.options_de265
       << " -q " << qp << " -o " << streamname.str()
       << " " << encoderParameters;

  std::string cmd2 = replace_variables(cmd1.str());

  printf("cmdline: %s\n",cmd2.c_str());

  RDPoint rd;
  rd.start_timer();
  int retval = system(cmd2.c_str());
  rd.end_timer();

  rd.compute_from_h265(streamname.str());

  if (!keepStreams) { unlink(streamname.str().c_str()); }

  write_rd_line(rd);

  return rd;
}




class Encoder_HM : public Encoder
{
public:
  Encoder_HM();

  void enableSCC(bool flag=true) { useSCC = flag; }
  void setQPRange(int low,int high,int step) { mQPLow=low; mQPHigh=high; mQPStep=step; }

  virtual std::vector<RDPoint> encode_curve(const Preset& preset) const;

private:
  RDPoint encode(const Preset& preset,int qp) const;

  bool useSCC;
  int mQPLow,mQPHigh,mQPStep;
};


Encoder_HM::Encoder_HM()
{
  mQPLow = 14;
  mQPHigh= 40;
  mQPStep=  2;

  useSCC = false;
}


std::vector<RDPoint> Encoder_HM::encode_curve(const Preset& preset) const
{
  std::vector<RDPoint> curve;

  for (int qp=mQPHigh ; qp>=mQPLow ; qp-=mQPStep) {
    curve.push_back(encode(preset, qp));
  }

  return curve;
}


RDPoint Encoder_HM::encode(const Preset& preset,int qp) const
{
  std::stringstream streamname;
  streamname << (useSCC ? "hmscc-" : "hm-") << preset.name << "-" << qp << ".265";

  char recoyuv_prefix[] = "/tmp/reco-XXXXXX";
  char *tempfile = mktemp(recoyuv_prefix);
  assert(tempfile != NULL && tempfile[0] != 0);
  std::string recoyuv = std::string(recoyuv_prefix) + ".yuv";

  std::stringstream cmd1;
  cmd1 << (useSCC ? "$HMSCCENC " : "$HMENC ")
       << input.options_HM()
       << " " << (useSCC ? preset.options_hm_scc : preset.options_hm)
       << " -q " << qp << " -o " << recoyuv << " -b " << streamname.str()
       << " " << encoderParameters << " >&2";

  std::string cmd2 = replace_variables(cmd1.str());

  std::cout << "CMD: '" << cmd2 << "'\n";
  RDPoint rd;
  rd.start_timer();
  int retval = system(cmd2.c_str());
  rd.end_timer();

  rd.compute_from_yuv(streamname.str(), recoyuv);
  if (!keepStreams) { unlink(streamname.str().c_str()); }
  unlink(recoyuv.c_str());

  write_rd_line(rd);

  return rd;
}



class Encoder_x265 : public Encoder
{
public:
  Encoder_x265();
  void setQPRange(int low,int high,int step) { mQPLow=low; mQPHigh=high; mQPStep=step; }

  virtual std::vector<RDPoint> encode_curve(const Preset& preset) const;

private:
  RDPoint encode(const Preset& preset,int qp) const;

  int mQPLow,mQPHigh,mQPStep;
};


Encoder_x265::Encoder_x265()
{
  /* CRF
  mQPLow =  4;
  mQPHigh= 34;
  mQPStep=  2;
  */

  mQPLow = 14;
  mQPHigh= 40;
  mQPStep=  2;
}


std::vector<RDPoint> Encoder_x265::encode_curve(const Preset& preset) const
{
  std::vector<RDPoint> curve;

  for (int qp=mQPHigh ; qp>=mQPLow ; qp-=mQPStep) {
    curve.push_back(encode(preset, qp));
  }

  return curve;
}


RDPoint Encoder_x265::encode(const Preset& preset,int qp) const
{
  std::stringstream streamname;
  streamname << "x265-" << preset.name << "-" << qp << ".265";

  std::stringstream cmd1;
  cmd1 << "$X265ENC " << input.options_x265()
       << " " << preset.options_x265
       << " --qp " << qp << " " << streamname.str()
       << " " << encoderParameters
       << " >&2";

  std::string cmd2 = replace_variables(cmd1.str());

  //std::cout << "CMD: '" << cmd2 << "'\n";
  RDPoint rd;
  rd.start_timer();
  int retval = system(cmd2.c_str());
  rd.end_timer();

  rd.compute_from_h265(streamname.str());
  if (!keepStreams) { unlink(streamname.str().c_str()); }

  write_rd_line(rd);

  return rd;
}




class Encoder_f265 : public Encoder
{
public:
  Encoder_f265();
  void setQPRange(int low,int high,int step) { mQPLow=low; mQPHigh=high; mQPStep=step; }

  virtual std::vector<RDPoint> encode_curve(const Preset& preset) const;

private:
  RDPoint encode(const Preset& preset,int qp) const;

  int mQPLow,mQPHigh,mQPStep;
};


Encoder_f265::Encoder_f265()
{
  mQPLow = 14;
  mQPHigh= 40;
  mQPStep=  2;
}


std::vector<RDPoint> Encoder_f265::encode_curve(const Preset& preset) const
{
  std::vector<RDPoint> curve;

  for (int qp=mQPHigh ; qp>=mQPLow ; qp-=mQPStep) {
    curve.push_back(encode(preset, qp));
  }

  return curve;
}


RDPoint Encoder_f265::encode(const Preset& preset,int qp) const
{
  std::stringstream cmd1;
  cmd1 << "$F265 " << input.options_f265()
       << " f265.out -v -p\"" << preset.options_f265 << " qp=" << qp
       << " " << encoderParameters
       << "\" >&2";

  std::string cmd2 = replace_variables(cmd1.str());

  std::cout << "CMD: '" << cmd2 << "'\n";
  RDPoint rd;
  rd.start_timer();
  int retval = system(cmd2.c_str());
  rd.end_timer();

  rd.compute_from_h265("f265.out");
  if (!keepStreams) { unlink("f265.out"); }

  write_rd_line(rd);

  return rd;
}



class Encoder_x264 : public Encoder
{
public:
  Encoder_x264();
  //void setCRFRange(int low,int high,int step) { mCRFLow=low; mCRFHigh=high; mCRFStep=step; }

  virtual std::vector<RDPoint> encode_curve(const Preset& preset) const;

private:
  RDPoint encode(const Preset& preset,int crf) const;

  int mCRFLow,mCRFMid,mCRFHigh;
  int mCRFStepHigh, mCRFStepLow;
};


Encoder_x264::Encoder_x264()
{
  // in the upper bit-rate range [mid;high], use larger CRF step-size 'StepHigh'
  // in the lower bit-rate range [low;mid], use smaller CRF step-size 'StepLow'

  mCRFLow = 10;
  mCRFMid = 20;
  mCRFHigh= 36;
  mCRFStepHigh= 2;
  mCRFStepLow = 1;
}


std::vector<RDPoint> Encoder_x264::encode_curve(const Preset& preset) const
{
  std::vector<RDPoint> curve;

  for (int crf=mCRFLow ; crf<mCRFMid ; crf+=mCRFStepHigh) {
    curve.push_back(encode(preset, crf));
  }

  for (int crf=mCRFMid ; crf<=mCRFHigh ; crf+=mCRFStepLow) {
    curve.push_back(encode(preset, crf));
  }

  return curve;
}


RDPoint Encoder_x264::encode(const Preset& preset,int qp_crf) const
{
  std::stringstream streamname;
  streamname << "x264-" << preset.name << "-" << qp_crf << ".264";

  std::stringstream cmd1;
#if 0
  cmd1 << "$X264 " << input.options_x264()
       << " " << preset.options_x264
       << " --crf " << qp_crf
       << " -o " << streamname.str();
#else
  cmd1 << "$FFMPEG " << input.options_ffmpeg()
       << " " << preset.options_x264_ffmpeg
       << " -crf " << qp_crf
       << " -threads 6"
       << " -f h264 " << streamname.str()
       << " " << encoderParameters;
#endif

  std::string cmd2 = replace_variables(cmd1.str());

  std::cerr << "-----------------------------\n";

  std::cerr << "CMD: '" << cmd2 << "'\n";

  RDPoint rd;
  rd.start_timer();
  int retval = system(cmd2.c_str());
  rd.end_timer();

  char tmpyuv_prefix[] = "/tmp/rdout-XXXXXX";
  char *tempfile = mktemp(tmpyuv_prefix);
  assert(tempfile != NULL && tempfile[0] != 0);
  std::string tmpyuv = std::string(tmpyuv_prefix) + ".yuv";

  std::string cmd3 = "ffmpeg -i " + streamname.str() + " -threads 6 " + tmpyuv;

  retval = system(cmd3.c_str());

  rd.compute_from_yuv(streamname.str(), tmpyuv);

  unlink(tmpyuv.c_str());
  if (!keepStreams) { unlink(streamname.str().c_str()); }

  write_rd_line(rd);

  return rd;
}


class Encoder_mpeg2 : public Encoder
{
public:
  Encoder_mpeg2();

  virtual std::vector<RDPoint> encode_curve(const Preset& preset) const;

private:
  RDPoint encode(const Preset& preset,int bitrate) const;
};


Encoder_mpeg2::Encoder_mpeg2()
{
}


std::vector<RDPoint> Encoder_mpeg2::encode_curve(const Preset& preset) const
{
  std::vector<RDPoint> curve;

  int bitrates[] = { 250,500,750,1000,1250,1500,1750,2000,2500,3000,3500,4000,4500,5000,
                     6000,7000,8000,9000,10000,12000,14000,16000,18000,20000,25000,30000,
                     -1 };

  for (int i=0; bitrates[i]>0; i++) {
    curve.push_back(encode(preset, bitrates[i]));
  }

  return curve;
}


RDPoint Encoder_mpeg2::encode(const Preset& preset,int br) const
{
  std::stringstream streamname;
  streamname << "mpeg2-" << preset.name << "-"
             << std::setfill('0') << std::setw(5) << br << ".mp2";

  std::stringstream cmd1;
  cmd1 << "$FFMPEG " << input.options_ffmpeg()
       << " " << preset.options_x264_ffmpeg
       << " -b " << br << "k "
       << " -threads 6"
       << " -f mpeg2video " << streamname.str()
       << " " << encoderParameters;

  std::string cmd2 = replace_variables(cmd1.str());

  std::cerr << "-----------------------------\n";

  std::cerr << "CMD: '" << cmd2 << "'\n";

  RDPoint rd;
  rd.start_timer();
  int retval = system(cmd2.c_str());
  rd.end_timer();

  char tmpyuv_prefix[] = "/tmp/rdout-XXXXXX";
  char *tempfile = mktemp(tmpyuv_prefix);
  assert(tempfile != NULL && tempfile[0] != 0);
  std::string tmpyuv = std::string(tmpyuv_prefix) + ".yuv";

  std::string cmd3 = "ffmpeg -i " + streamname.str() + " -threads 6 " + tmpyuv;

  retval = system(cmd3.c_str());

  rd.compute_from_yuv(streamname.str(), tmpyuv);

  unlink(tmpyuv.c_str());
  if (!keepStreams) { unlink(streamname.str().c_str()); }

  write_rd_line(rd);

  return rd;
}


Encoder_de265 enc_de265;
Encoder_HM enc_hm;
Encoder_x265 enc_x265;
Encoder_f265 enc_f265;
Encoder_x264 enc_x264;
Encoder_mpeg2 enc_mpeg2;

// ---------------------------------------------------------------------------

static struct option long_options[] = {
  {"keep-streams",      no_argument,       0, 'k' },
  //{"write-bytestream", required_argument,0, 'B' },
  {0,         0,                 0,  0 }
};


void show_usage()
{
  fprintf(stderr,
          "usage: rd-curves 'preset_id' 'input_preset' 'encoder'\n"
          "supported encoders: de265 / hm / hmscc / x265 / f265 / x264 / mpeg2\n");
  fprintf(stderr,
          "presets:\n");

  for (int i=0;preset[i].name!=NULL;i++) {
    fprintf(stderr,
            " %2d %-20s %s\n",preset[i].ID,preset[i].name,preset[i].descr);
  }

  fprintf(stderr,
          "\ninput presets:\n");
  for (int i=0;inputSpec[i].name;i++) {
    fprintf(stderr,
            " %-12s %-30s %4dx%4d, %4d frames, %5.2f fps\n",
            inputSpec[i].name,
            inputSpec[i].filename,
            inputSpec[i].width,
            inputSpec[i].height,
            inputSpec[i].nFrames,
            inputSpec[i].fps);
  }
}

int main(int argc, char** argv)
{
  init_clock();

  while (1) {
    int option_index = 0;

    int c = getopt_long(argc, argv, "kf:p:",
                        long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 'k': keepStreams=true; break;
    case 'f': maxFrames=atoi(optarg); break;
    case 'p': encoderParameters=optarg; break;
    }
  }

  if (optind != argc-3) {
    show_usage();
    exit(5);
  }

  int presetID = atoi( argv[optind] );
  const char* inputName = argv[optind+1];
  const char* encoderName = argv[optind+2];

  int presetIdx = -1;

  for (int i=0;preset[i].name != NULL;i++) {
    if (preset[i].ID == presetID) {
      presetIdx = i;
      break;
    }
  }

  if (presetIdx == -1) {
    fprintf(stderr,"preset ID %d does not exist\n",presetID);
    exit(5);
  }

  setInput(inputName);
  if (maxFrames) input.setMaxFrames(maxFrames);


  Encoder* enc = NULL;
  /**/ if (strcmp(encoderName,"de265")==0) { enc = &enc_de265; }
  else if (strcmp(encoderName,"hm"   )==0) { enc = &enc_hm;   }
  else if (strcmp(encoderName,"hmscc")==0) { enc = &enc_hm;   enc_hm.enableSCC(); }
  else if (strcmp(encoderName,"x265" )==0) { enc = &enc_x265; }
  else if (strcmp(encoderName,"f265" )==0) { enc = &enc_f265; }
  else if (strcmp(encoderName,"x264" )==0) { enc = &enc_x264; }
  else if (strcmp(encoderName,"mpeg2")==0) { enc = &enc_mpeg2; }

  if (enc==NULL) {
    fprintf(stderr, "unknown encoder");
    exit(5);
  }


  std::stringstream data_filename;
  data_filename << encoderName << "-" << inputName << "-" << preset[presetIdx].name << ".rd";
  output_fh = fopen(data_filename.str().c_str(), "wb");

  fprintf(output_fh,"# %s\n", preset[presetIdx].descr);
  fprintf(output_fh,"# 1:rate 2:psnr 3:ssim 4:cputime(min) 5:walltime(min)\n");

  std::vector<RDPoint> curve = enc->encode_curve(preset[presetIdx]);

  for (int i=0;i<curve.size();i++) {
    //fprintf(out_fh,"%7.2f %6.4f\n", curve[i].rate/1024, curve[i].psnr);
  }

  fclose(output_fh);

  return 0;
}
