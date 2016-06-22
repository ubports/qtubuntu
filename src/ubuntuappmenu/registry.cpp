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

#include "registry.h"
#include "logging.h"
#include "menuregistrar_interface.h"

#include <QDBusObjectPath>
#include <QDBusServiceWatcher>

#define REGISTRAR_SERVICE "com.ubuntu.MenuRegistrar"
#define REGISTRY_OBJECT_PATH "/com/ubuntu/MenuRegistrar"

UbuntuMenuRegistry *UbuntuMenuRegistry::instance()
{
    static UbuntuMenuRegistry* registry(new UbuntuMenuRegistry());
    return registry;
}

UbuntuMenuRegistry::UbuntuMenuRegistry(QObject* parent)
    : QObject(parent)
    , m_serviceWatcher(new QDBusServiceWatcher(REGISTRAR_SERVICE, QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, this))
    , m_interface(new ComUbuntuMenuRegistrarInterface(REGISTRAR_SERVICE, REGISTRY_OBJECT_PATH, QDBusConnection::sessionBus(), this))
    , m_connected(m_interface->isValid())
{
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &UbuntuMenuRegistry::serviceOwnerChanged);
}

UbuntuMenuRegistry::~UbuntuMenuRegistry()
{
    delete m_interface;
}

void UbuntuMenuRegistry::registerSurfaceMenu(const QString &surfaceId, QDBusObjectPath menuObjectPath, const QString &service)
{
    qCDebug(ubuntuappmenu, "UbuntuMenuRegistry::registerMenu(surfaceId=%s, menuObjectPath=%s, service=%s)",
            qPrintable(surfaceId),
            qPrintable(menuObjectPath.path()),
            qPrintable(service));

    auto ret = m_interface->RegisterSurfaceMenu(surfaceId, menuObjectPath, menuObjectPath, service);
}

void UbuntuMenuRegistry::unregisterSurfaceMenu(const QString &surfaceId, QDBusObjectPath menuObjectPath)
{
    qCDebug(ubuntuappmenu, "UbuntuMenuRegistry::unregisterSurfaceMenu(surfaceId=%s, menuObjectPath=%s, service=%s)",
            qPrintable(surfaceId),
            qPrintable(menuObjectPath.path()));

    m_interface->UnregisterSurfaceMenu(surfaceId, menuObjectPath);
}


void UbuntuMenuRegistry::serviceOwnerChanged(const QString &serviceName, const QString& oldOwner, const QString &newOwner)
{
    qCDebug(ubuntuappmenu, "UbuntuMenuRegistry::serviceOwnerChanged(newOwner=%s)", qPrintable(newOwner));

    Q_UNUSED(oldOwner);
    if (serviceName != REGISTRAR_SERVICE) return;

    if (oldOwner != newOwner) {
        m_connected = !newOwner.isEmpty();
        Q_EMIT serviceChanged();
    }
}
