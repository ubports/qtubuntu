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

#include <gio/gio.h>

#include <QTimer>
#include <QSet>
#include <QMetaObject>

#include "gmenumodelplatformmenu.h"

class GMenuModelExporter : public QObject
{
    Q_OBJECT
public:
    GMenuModelExporter(GMenuModelPlatformMenuBar * bar);
    GMenuModelExporter(GMenuModelPlatformMenu * menu);
    virtual ~GMenuModelExporter();

    void exportModels();
    void unexportModels();
    void clear();

    QString menuPath() const { return m_menuPath;}

private:
    GMenuItem *createSubmenu(QPlatformMenu* platformMenu, GMenuModelPlatformMenuItem* forItem);
    GMenuItem *createMenuItem(QPlatformMenuItem* platformMenuItem);
    GMenuItem *createSection(QList<QPlatformMenuItem*>::const_iterator iter, QList<QPlatformMenuItem*>::const_iterator end);

    void addSubmenuItems(GMenuModelPlatformMenu* gplatformMenu, GMenu* menu);
    void processItemForGMenu(QPlatformMenuItem* item, GMenu* gmenu);

    void createAction(const QByteArray& name, GMenuModelPlatformMenuItem* gplatformItem);

private:
    GMenu* m_gmainMenu;
    GSimpleActionGroup* m_gactionGroup;
    QSet<QByteArray> m_actions;
    int m_exportedModel;
    int m_exportedActions;
    QTimer m_structureTimer;
    QString m_menuPath;

    QList<QMetaObject::Connection> m_propertyConnections;
};

#endif // GMENUMODELEXPORTER_H
