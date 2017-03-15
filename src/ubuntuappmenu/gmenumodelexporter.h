/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GMENUMODELEXPORTER_H
#define GMENUMODELEXPORTER_H

#include "gmenumodelplatformmenu.h"

#include <gio/gio.h>

#include <QTimer>
#include <QMap>
#include <QSet>
#include <QMetaObject>

class QtUbuntuExtraActionHandler;

// Base class for a gmenumodel exporter
class UbuntuGMenuModelExporter : public QObject
{
    Q_OBJECT
public:
    virtual ~UbuntuGMenuModelExporter();

    void exportModels();
    void unexportModels();

    QString menuPath() const { return m_menuPath;}

    void aboutToShow(quint64 tag);

protected:
    UbuntuGMenuModelExporter(QObject *parent);

    GMenuItem *createSubmenu(QPlatformMenu* platformMenu, UbuntuPlatformMenuItem* forItem);
    GMenuItem *createMenuItem(QPlatformMenuItem* platformMenuItem, GMenu *parentMenu);
    GMenuItem *createSection(QList<QPlatformMenuItem*>::const_iterator iter, QList<QPlatformMenuItem*>::const_iterator end);
    void addAction(const QByteArray& name, UbuntuPlatformMenuItem* gplatformItem, GMenu *parentMenu);

    void addSubmenuItems(UbuntuPlatformMenu* gplatformMenu, GMenu* menu);
    void processItemForGMenu(QPlatformMenuItem* item, GMenu* gmenu);

    void clear();

    void timerEvent(QTimerEvent *e) override;

protected:
    GDBusConnection *m_connection;
    GMenu *m_gmainMenu;
    GSimpleActionGroup *m_gactionGroup;
    guint m_exportedModel;
    guint m_exportedActions;
    QtUbuntuExtraActionHandler *m_qtubuntuExtraHandler;
    QTimer m_structureTimer;
    QString m_menuPath;

    // UbuntuPlatformMenu::tag -> UbuntuPlatformMenu
    QMap<quint64, UbuntuPlatformMenu*> m_submenusWithTag;

    // UbuntuPlatformMenu -> reload TimerId (startTimer)
    QHash<UbuntuPlatformMenu*, int> m_reloadMenuTimers;

    QHash<UbuntuPlatformMenu*, GMenu*> m_gmenusForMenus;

    QHash<GMenu*, QSet<QByteArray>> m_actions;
    QHash<GMenu*, QVector<QMetaObject::Connection>> m_propertyConnections;

};

// Class which exports a qt platform menu bar.
class UbuntuMenuBarExporter : public UbuntuGMenuModelExporter
{
public:
    UbuntuMenuBarExporter(UbuntuPlatformMenuBar *parent);
    ~UbuntuMenuBarExporter();
};

// Class which exports a qt platform menu.
// This will allow exporting of context menus.
class UbuntuMenuExporter : public UbuntuGMenuModelExporter
{
public:
    UbuntuMenuExporter(UbuntuPlatformMenu *parent);
    ~UbuntuMenuExporter();
};

#endif // GMENUMODELEXPORTER_H
