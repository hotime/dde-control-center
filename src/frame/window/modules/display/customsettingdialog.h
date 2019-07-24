/*
 * Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     lq <longqi_cm@deepin.com>
 *
 * Maintainer: lq <longqi_cm@deepin.com>
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

#ifndef CUSTOMSETTINGDIALOG_H
#define CUSTOMSETTINGDIALOG_H

#include "../../namespace.h"

#include <QDialog>
#include <memory>

class QListView;
class QVBoxLayout;

namespace dcc {

namespace display {
class DisplayModel;
class Monitor;
class MonitorControlWidget;
class MonitorIndicator;
}

namespace widgets{
class BasicListView;
}
}

namespace DCC_NAMESPACE {

namespace display {

class CustomSettingDialog : public QDialog
{
    Q_OBJECT
public:
    CustomSettingDialog(QWidget *parent = nullptr);
    CustomSettingDialog(dcc::display::Monitor *mon, dcc::display::DisplayModel *model, QWidget *parent = nullptr);
    ~CustomSettingDialog();
public:
    void setModel(dcc::display::DisplayModel *model);
Q_SIGNALS:
    void requestShowRotateDialog(dcc::display::Monitor *mon);
    void requestRecognize();
    void requestMerge();
    void requestSplit();
    void requestSetMonitorPosition(dcc::display::Monitor *mon, const int x, const int y);
    void requestSetResolution(dcc::display::Monitor *mon, int resolution);
private Q_SLOTS:
    void onMonitorPress(dcc::display::Monitor *mon);
    void onMonitorRelease(dcc::display::Monitor *mon);
private:
    void init();
    void initWithModel();
    void initOtherDialog();
    void initResolutionList();
private:
    bool m_isPrimary{false};
    dcc::display::Monitor *m_monitor{nullptr};
    dcc::display::DisplayModel *m_model{nullptr};
    QListView *m_resolutionList{nullptr};
    QList<CustomSettingDialog *> m_otherDialog;
    QVBoxLayout *m_layout{nullptr};
    std::unique_ptr<dcc::display::MonitorIndicator> m_fullIndication;
    dcc::display::MonitorControlWidget *m_monitroControlWidget{nullptr};
    dcc::widgets::BasicListView *m_baseListView{nullptr};
};
}

}

#endif // CUSTOMSETTINGDIALOG_H
