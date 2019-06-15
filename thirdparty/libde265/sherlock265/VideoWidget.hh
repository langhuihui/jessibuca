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

#ifndef VIDEOWIDGET_HH
#define VIDEOWIDGET_HH

#include <qglobal.h>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets/QtWidgets>
#else
#include <QtGui/QWidget>
#endif


class VideoWidget : public QWidget
{
     Q_OBJECT
public:
  VideoWidget(QWidget *parent = 0);
  ~VideoWidget();

  QSize sizeHint() const;

public slots:
  void setImage(QImage* img)
  {
    mImg=img; repaint();
  }

 protected:
  void paintEvent(QPaintEvent *event);
  void resizeEvent(QResizeEvent *event);

 private:

  QImage* mImg;
};

#endif
