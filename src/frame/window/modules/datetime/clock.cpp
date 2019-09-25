/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     wubw <wubowen_cm@deepin.com>
 *
 * Maintainer: wubw <wubowen_cm@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "clock.h"

#include <QPainter>
#include <QPainterPath>
#include <QTime>
#include <QtMath>

using namespace DCC_NAMESPACE;
using namespace DCC_NAMESPACE::datetime;

Clock::Clock(QWidget *parent)
    : QWidget(parent)
    , m_drawTicks(true)
    , m_autoNightMode(true)
    , n_bIsUseBlackPlat(true)
{

}

Clock::~Clock()
{

}

void Clock::paintEvent(QPaintEvent *)
{
    QDateTime datetime(QDateTime::currentDateTimeUtc());
    datetime = datetime.addSecs(m_timeZone.getUTCOffset());

    const QTime time(datetime.time());

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // draw plate
    painter.translate(0, 0);
    if (n_bIsUseBlackPlat) {
        painter.drawPixmap(0, 0, QPixmap(":/datetime/icons/dcc_clock_black.svg"));
    } else {
        painter.drawPixmap(0, 0, QPixmap(":/datetime/icons/dcc_clock_white.svg"));
    }

    QPixmap pix;

    int nHour = (time.hour() >= 12) ? (time.hour() - 12) : time.hour();
    int nStartAngle = 90;//The image from 0 start , but the clock need from -90 start

    // draw hour hand
    const qreal hourAngle = qreal(nHour * 30 + time.minute() * 30 / 60 + time.second() * 30 / 60 / 60 - nStartAngle);
    painter.save();
    painter.translate(width() / 2.0, height() / 2.0);
    painter.rotate(hourAngle);
    pix.load(":/datetime/icons/dcc_noun_hour.svg");
    painter.drawPixmap(-pix.width() / 2, -pix.height() / 2, pix);
    painter.restore();

    // draw minute hand
    const qreal minuteAngle = qreal(time.minute() * 6 + time.second() * 6 / 60 - nStartAngle);
    painter.save();
    painter.translate(width() / 2.0, height() / 2.0);
    painter.rotate(minuteAngle);
    pix.load(":/datetime/icons/dcc_noun_minute.svg");
    painter.drawPixmap(-pix.width() / 2, -pix.height() / 2, pix);
    painter.restore();

    // draw second hand
    const qreal secondAngle = qreal(time.second() * 6 - nStartAngle);
    painter.save();
    painter.translate(width() / 2.0, height() / 2.0);
    painter.rotate(secondAngle);
    pix.load(":/datetime/icons/dcc_noun_second.svg");
    painter.drawPixmap(-pix.width() / 2, -pix.height() / 2, pix);
    painter.restore();

    painter.end();
}

bool Clock::autoNightMode() const
{
    return m_autoNightMode;
}

void Clock::setAutoNightMode(bool autoNightMode)
{
    if (m_autoNightMode != autoNightMode) {
        m_autoNightMode = autoNightMode;
        update();
    }
}

void Clock::setPlate(bool isBlack)
{
    if (n_bIsUseBlackPlat != isBlack) {
        n_bIsUseBlackPlat = isBlack;
        update();
    }
}

void Clock::setTimeZone(const ZoneInfo &timeZone)
{
    if (m_timeZone == timeZone) return;

    m_timeZone = timeZone;
    update();
}

bool Clock::drawTicks() const
{
    return m_drawTicks;
}

void Clock::setDrawTicks(bool drawTicks)
{
    if (m_drawTicks != drawTicks) {
        m_drawTicks = drawTicks;
        update();
    }
}
