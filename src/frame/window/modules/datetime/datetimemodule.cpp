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
#include "datetimemodule.h"
#include "datetimewidget.h"
#include "timezonelist.h"
#include "datesettings.h"
#include "clockitem.h"
#include "systemtimezone.h"

#include <types/zoneinfo.h>

#include "modules/datetime/datetimework.h"
#include "modules/datetime/datetimemodel.h"
#include "modules/datetime/timezone_dialog/timezone.h"
#include "modules/datetime/timezone_dialog/timezonechooser.h"

using namespace dcc::datetime;
using namespace DCC_NAMESPACE;
using namespace DCC_NAMESPACE::datetime;

DatetimeModule::DatetimeModule(FrameProxyInterface *frameProxy, QObject *parent)
    : QObject(parent)
    , ModuleInterface(frameProxy)
    , m_timezonelist(nullptr)
    , m_widget(nullptr)
{

}

void DatetimeModule::initialize()
{
    m_work = &DatetimeWork::getInstance();
    m_model = m_work->model();

    m_work->moveToThread(qApp->thread());
    m_model->moveToThread(qApp->thread());

#ifndef DCC_DISABLE_TIMEZONE
    connect(this, &DatetimeModule::requestSetTimeZone, m_work, &DatetimeWork::setTimezone);
    connect(this, &DatetimeModule::requestRemoveUserTimeZone, m_work, &DatetimeWork::removeUserTimeZone);
    connect(this, &DatetimeModule::requestAddUserTimeZone, m_work, &DatetimeWork::addUserTimeZone);
#endif
}

const QString DatetimeModule::name() const
{
    return QStringLiteral("datetime");
}

void DatetimeModule::active()
{
    m_widget = new DatetimeWidget;
    connect(m_widget, &DatetimeWidget::requestPushWidget, this, &DatetimeModule::onPushWidget);
    connect(m_widget, &DatetimeWidget::requestSetHourType, m_work, &DatetimeWork::set24HourType);
    connect(m_model,  &DatetimeModel::hourTypeChanged, m_widget, &DatetimeWidget::onHourTypeChanged);
    m_widget->setModel(m_model);
    m_work->activate(); //refresh data
    m_widget->setCurrentTimeZone(m_model->currentTimeZone());

    //set dbus data to 24 hour format
    m_widget->onHourTypeChanged(m_model->get24HourFormat());
    m_widget->setTimeType(QLocale::system().name());

    m_frameProxy->pushWidget(this, m_widget);
    m_widget->setDefaultWidget();
}

int DatetimeModule::load(QString path)
{
    if (!m_widget) {
        active();
    }

    QListView *list = m_widget->getListViewPointer();
    DatetimeType type = Default;

    if (!list) {
        return 0;
    }

    if (path == "Timezone List") {
        type = ETimezoneList;
    } else if (path == "Timezone List/Change System Timezone") {
        type = ESystemTimezone;
    } else if (path == "Time Settings") {
        type = TimeSetting;
    } else if (path == "Timezone List/Add Timezone") {
        type = AddTimeZone;
    }

    QModelIndex index = list->model()->index(type, 0);
    switch (type) {
    case ETimezoneList:
    case TimeSetting:
        list->setCurrentIndex(index);
        list->clicked(index);
        break;
    case ESystemTimezone:
        //First enter timezoneList
        index = list->model()->index(ETimezoneList, 0);
        list->setCurrentIndex(index);
        list->clicked(index);

        //Then enter systemTimezone
        showSystemTimezone();
        break;
    case AddTimeZone:
        //First enter timezoneList
        index = list->model()->index(ETimezoneList, 0);
        list->setCurrentIndex(index);
        list->clicked(index);

        //Then enter addTimezone
        showSystemTimezone();
        Q_EMIT m_timezonelist->requestAddTimeZone();
        break;
    default:
        break;
    }

    return 0;
}

QStringList DatetimeModule::availPage() const
{
    QStringList list;
    list << "Timezone List" << "Timezone List/Change System Timezone" << "Time Settings" << "Timezone List/Add Timezone";

    return list;
}

void DatetimeModule::updateSystemTimezone(const QString &timezone)
{
    if (timezone.isEmpty()) return;

    const QString locale = QLocale::system().name();
    const QString name = installer::GetLocalTimezoneName(timezone, locale);

    if (m_dialog) {
        m_dialog->setCurrentTimeZoneText(name);
        m_dialog->close();
    }

    if (m_timezonelist && m_model) {
        m_timezonelist->getTimezoneContentListPtr()->updateTimezones(m_model->userTimeZones());
    }

    m_widget->setCurrentTimeZone(m_model->currentTimeZone());
}

void DatetimeModule::ensureZoneChooserDialog()
{
    if (m_dialog)
        return;

    m_dialog = new TimeZoneChooser();
    m_dialog->setAttribute(Qt::WA_DeleteOnClose);

    connect(m_dialog, &TimeZoneChooser::confirmed, this, [this](const QString & timezone) {
        if (m_dialog->isAddZone()) {
            Q_EMIT requestAddUserTimeZone(timezone);
        } else {
            Q_EMIT requestSetTimeZone(timezone);
        }

        m_dialog->close();
    });

    connect(m_model, &dcc::datetime::DatetimeModel::systemTimeZoneIdChanged, this, &DatetimeModule::updateSystemTimezone);
}

void DatetimeModule::showTimezoneList()
{
    SystemTimezone *sysTimezoneWidget = new SystemTimezone;
    connect(sysTimezoneWidget, &SystemTimezone::requestSetSystemTimezone, this, &DatetimeModule::showSystemTimezone);
    connect(m_model, &DatetimeModel::currentSystemTimeZoneChanged, sysTimezoneWidget, &SystemTimezone::setSystemTimezone);
    sysTimezoneWidget->setSystemTimezone(m_model->currentSystemTimeZone());

    if (!m_timezonelist) {
        m_timezonelist = new TimezoneList;

        //first into this page, update timezonelist data
        m_timezonelist->getTimezoneContentListPtr()->addTimezones(m_model->userTimeZones());
        updateSystemTimezone(m_model->systemTimeZoneId());

        connect(m_model, &DatetimeModel::userTimeZoneAdded,
                m_timezonelist->getTimezoneContentListPtr(), &TimezoneContentList::addTimezone);
        connect(m_model, &DatetimeModel::userTimeZoneRemoved,
                m_timezonelist->getTimezoneContentListPtr(), &TimezoneContentList::removeTimezone);
        connect(m_timezonelist, &TimezoneList::requestAddTimeZone, this, [this] {
            ensureZoneChooserDialog();
            m_dialog->setIsAddZone(true);
            m_dialog->show();
        });
        connect(m_timezonelist->getTimezoneContentListPtr(), &TimezoneContentList::requestRemoveUserTimeZone,
                this, &DatetimeModule::requestRemoveUserTimeZone);
        connect(m_timezonelist, &TimezoneList::requestAddUserTimeZone,
                this, &DatetimeModule::requestAddUserTimeZone);
    }

    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->addWidget(sysTimezoneWidget, Qt::AlignTop);
    layout->addWidget(m_timezonelist);
    widget->setLayout(layout);
    m_frameProxy->pushWidget(this, widget);
}

void DatetimeModule::showSystemTimezone()
{
    if (m_dialog && m_dialog->isVisible()) return;

    ensureZoneChooserDialog();
    m_dialog->setIsAddZone(false);
    m_dialog->show();
    m_dialog->setMarkedTimeZone(installer::GetCurrentTimezone());
}

void DatetimeModule::showTimeSetting()
{
    m_setting = new DateSettings;

    connect(m_setting, &DateSettings::requestSetAutoSyncdate, m_work, &dcc::datetime::DatetimeWork::setNTP);
    connect(m_setting, &DateSettings::requestSetTime, m_work, &dcc::datetime::DatetimeWork::setDatetime);
    connect(m_setting, &DateSettings::requestBack, this, &DatetimeModule::onPopWidget);
    connect(m_setting, &DateSettings::requestNTPServer, m_work, &DatetimeWork::setNtpServer);
    connect(m_model, &DatetimeModel::systemTimeChanged, m_setting, &DateSettings::requestBack);
    connect(m_model, &dcc::datetime::DatetimeModel::NTPChanged, m_setting, &DateSettings::updateRealAutoSyncCheckState);
    connect(m_model, &dcc::datetime::DatetimeModel::NTPServerListChanged, m_setting, &DateSettings::updateNTPServerList);
    connect(m_model, &DatetimeModel::NTPServerChanged, this, [this](QString server) {
        if (m_setting) {
            m_setting->setNtpServerAddress(server);
        }
    });
    connect(m_model, &DatetimeModel::NTPServerNotChanged, this, [this](QString server) {
        if (m_setting) {
            m_setting->setLastServerAddress(server);
        }
    });


    m_setting->updateRealAutoSyncCheckState(m_model->nTP());
    m_setting->updateNTPServerList(m_model->ntpServerList());
    m_setting->setNtpServerAddress(m_model->ntpServerAddress());

    //fit the two clocks's point to same
    m_setting->setCurrentTimeZone(m_model->currentTimeZone());
    m_widget->setCurrentTimeZone(m_model->currentTimeZone());

    m_frameProxy->pushWidget(this, m_setting);
}

void DatetimeModule::onPushWidget(const int &index)
{
    switch (static_cast<DatetimeType>(index)) {
    case ETimezoneList:
        showTimezoneList();
        break;
    case TimeSetting:
        showTimeSetting();
        break;
    default:
        break;
    }
}

void DatetimeModule::onPopWidget()
{
    DListView *list = m_widget->getListViewPointer();

    if (!list) {
        qWarning() << " DListView is nullptr.";
        return;
    }

    QModelIndex index = list->model()->index(ETimezoneList, 0);
    list->setCurrentIndex(index);
    list->clicked(index);
}
