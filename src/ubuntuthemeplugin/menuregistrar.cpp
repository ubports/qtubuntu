#include "menuregistrar.h"
#include "registry.h"
#include "logging.h"

#include <QDebug>
#include <QDBusObjectPath>
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>

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
}

MenuRegistrar::~MenuRegistrar()
{
    if (!m_registeredSurfaceId.isEmpty()) {
        unregisterSurfaceMenu();
    }
}

void MenuRegistrar::registerSurfaceMenuForWindow(QWindow* window, const QDBusObjectPath& path)
{
    qCWarning(qtubuntuMenus).nospace() << "MenuRegistrar::registerSurfaceMenuForWindow(window=" << window << ", path=" << path.path() << ")";

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

    UbuntuMenuRegistry::instance()->registerSurfaceMenu(persistentSurfaceId, m_path, m_service);
    m_registeredSurfaceId = persistentSurfaceId;
}

void MenuRegistrar::unregisterSurfaceMenu()
{
    if (!UbuntuMenuRegistry::instance()->isConnected()) return;

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
