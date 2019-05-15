/*
 * libde265 example application "sherlock265".
 * Copyright (c) 2013-2014 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * This file is part of sherlock265, an example application using libde265.
 *
 * sherlock265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * sherlock265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sherlock265.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "VideoPlayer.hh"


int main(int argc, char **argv)
{
  if (argc != 2) {
    fprintf(stderr,"usage: sherlock265 videofile.bin\n");
    fprintf(stderr,"The video file must be a raw h.265 bitstream (e.g. HM-10.0 output)\n");
    exit(5);
  }


  QApplication app(argc, argv);

  VideoPlayer videoPlayer(argv[1]);
  videoPlayer.show();

  return app.exec();
}

