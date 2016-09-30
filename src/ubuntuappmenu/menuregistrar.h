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

#ifndef MENUREGISTRAR_H
#define MENUREGISTRAR_H

#include <QObject>
#include <QWindow>
#include <QPointer>
#include <QDBusObjectPath>

#include <gio/gio.h>

class UbuntuMenuRegistrar : public QObject
{
    Q_OBJECT
public:
    UbuntuMenuRegistrar();
    ~UbuntuMenuRegistrar();

    void registerMenuForWindow(QWindow* window, const QDBusObjectPath& path);
    void unregisterMenu();

private Q_SLOTS:
    void registerSurfaceMenu();
    void onRegistrarServiceChanged();

private:
    void registerMenu();

    void registerApplicationMenu();
    void unregisterApplicationMenu();

    void unregisterSurfaceMenu();

    GDBusConnection *m_connection;
    QString m_service;
    QDBusObjectPath m_path;
    QPointer<QWindow> m_window;
    QString m_registeredSurfaceId;
    pid_t m_registeredProcessId;
};


#endif // MENUREGISTRAR_H

