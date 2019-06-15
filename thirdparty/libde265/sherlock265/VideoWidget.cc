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

#include "VideoWidget.hh"
#include <QtGui>


VideoWidget::VideoWidget(QWidget *parent)
  : QWidget(parent), mImg(NULL)
{
  setAutoFillBackground(false);
  setAttribute(Qt::WA_NoSystemBackground, true);

  QPalette palette = this->palette();
  palette.setColor(QPalette::Background, Qt::black);
  setPalette(palette);

  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  setUpdatesEnabled(true);
}

VideoWidget::~VideoWidget()
{
}


QSize VideoWidget::sizeHint() const
{
  return QSize(352,288);
}


void VideoWidget::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);

  if (mImg) {
    QRect videoRect = mImg->rect();
    videoRect.moveCenter(this->rect().center());

    QRect erect = event->rect();

    if (!videoRect.contains(event->rect())) {
      QRegion region = event->region();
      region = region.subtracted(videoRect);

      QBrush brush = palette().background();

      foreach (const QRect &rect, region.rects()) {
        painter.fillRect(rect, brush);
      }
    }

    painter.drawImage(videoRect, *mImg);
  } else {
    painter.fillRect(event->rect(), palette().background());
  }
}

void VideoWidget::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
}
