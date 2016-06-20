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

#include "menuregistrar.h"
#include "registry.h"
#include "logging.h"

#include <QDebug>
#include <QDBusObjectPath>
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformwindow.h>

#include <gio/gio.h>

MenuRegistrar::MenuRegistrar()
{
    GError *error = NULL;
    GDBusConnection *bus;
    bus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
    if (!bus) {
        qCWarning(qtubuntuMenus, "Failed to retreive session bus - %s", error ? error->message : "unknown error");
        return;
    }
    m_service = g_dbus_connection_get_unique_name(bus);
    connect(UbuntuMenuRegistry::instance(), &UbuntuMenuRegistry::serviceChanged, this, &MenuRegistrar::onRegistrarServiceChanged);

    auto nativeInterface = qGuiApp->platformNativeInterface();
    connect(nativeInterface, &QPlatformNativeInterface::windowPropertyChanged, this, [this](QPlatformWindow* window, const QString &property) {
        if (property != QStringLiteral("persistentSurfaceId")) {
            return;
        }
        if (window->window() == m_window) {
            qDebug() << "EMIT " << property << m_window->handle();
            registerSurfaceMenuForWindow(m_window, m_path);
        }
    });
}

MenuRegistrar::~MenuRegistrar()
{
    if (!m_registeredSurfaceId.isEmpty()) {
        unregisterSurfaceMenu();
    }
}

void MenuRegistrar::registerSurfaceMenuForWindow(QWindow* window, const QDBusObjectPath& path)
{
    if (!m_registeredSurfaceId.isEmpty()) {
        unregisterSurfaceMenu();
    }

    m_window = window;
    m_path = path;

    if (UbuntuMenuRegistry::instance()->isConnected() && m_window) {
        registerSurfaceMenu();
    }
}

void MenuRegistrar::registerSurfaceMenu()
{
    auto nativeInterface = qGuiApp->platformNativeInterface();
    QString persistentSurfaceId = nativeInterface->windowProperty(m_window->handle(), "persistentSurfaceId", QString()).toString();
    if (persistentSurfaceId.isEmpty()) return;

    qCDebug(qtubuntuMenus).nospace() << "MenuRegistrar::registerSurfaceMenuForWindow(window=" << m_window << ", path=" << m_path.path() << ")";

    UbuntuMenuRegistry::instance()->registerSurfaceMenu(persistentSurfaceId, m_path, m_service);
    m_registeredSurfaceId = persistentSurfaceId;
}

void MenuRegistrar::unregisterSurfaceMenu()
{
    if (!UbuntuMenuRegistry::instance()->isConnected()) return;

    qCDebug(qtubuntuMenus).nospace() << "MenuRegistrar::unregisterSurfaceMenu(window=" << m_window << ", path=" << m_path.path() << ")";

    UbuntuMenuRegistry::instance()->unregisterSurfaceMenu(m_registeredSurfaceId, m_path);
    m_registeredSurfaceId.clear();
}

void MenuRegistrar::onRegistrarServiceChanged()
{
    if (!m_registeredSurfaceId.isEmpty()) {
        unregisterSurfaceMenu();
    }
    if (UbuntuMenuRegistry::instance()->isConnected() && m_window) {
        registerSurfaceMenu();
    }
}
