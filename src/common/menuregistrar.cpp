#include "menuregistrar.h"
#include "registry.h"

#include <QDebug>
#include <QDBusObjectPath>

#include <gio/gio.h>

MenuRegistrar::MenuRegistrar()
{
    GError *error = NULL;
    GDBusConnection *bus;
    bus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
    if (!bus) {
        qWarning() << "Failed to retreive session bus - " << (error ? error->message : "unknown error");
        return;
    }
    m_service = g_dbus_connection_get_unique_name(bus);
    connect(UbuntuMenuRegistry::instance(), &UbuntuMenuRegistry::serviceChanged, this, &MenuRegistrar::onRegistrarServiceChanged);
}

MenuRegistrar::~MenuRegistrar()
{
    unregisterMenu();
}

void MenuRegistrar::registerMenuForWindow(QWindow* window, const QDBusObjectPath& path)
{
    if (!m_registeredSurfaceId.isEmpty()) unregisterMenu();
    if (!m_window.isNull()) m_window->removeEventFilter(this);

    m_window = window;
    m_path = path;

    if (window) {
        if (window->property("surfaceId").isValid()) {
            registerMenu();
        }
        window->installEventFilter(this);
    }
}

void MenuRegistrar::registerMenu()
{
    const QString surfaceId = m_window->property("surfaceId").toString();
    UbuntuMenuRegistry::instance()->registerMenu(surfaceId, m_path, m_service);
    m_registeredSurfaceId = surfaceId;
}

void MenuRegistrar::unregisterMenu()
{
    if (!UbuntuMenuRegistry::instance()->isConnected())
    if (m_registeredSurfaceId.isEmpty()) return;

    UbuntuMenuRegistry::instance()->unregisterMenu(m_registeredSurfaceId, m_path);
    m_registeredSurfaceId.clear();
}

void MenuRegistrar::onRegistrarServiceChanged()
{
    if (!m_registeredSurfaceId.isEmpty()) {
        unregisterMenu();
    }
    if (UbuntuMenuRegistry::instance()->isConnected()) {
        if (m_window && m_window->property("surfaceId").isValid()) {
            registerMenu();
        }
    }
}

bool MenuRegistrar::eventFilter(QObject * watched, QEvent * event) {
    if (watched == m_window) {
        if (event->type() == QEvent::DynamicPropertyChange) {
            QDynamicPropertyChangeEvent* propertyEvent = static_cast<QDynamicPropertyChangeEvent*>(event);
            if (propertyEvent->propertyName() == "surfaceId") {
                if (m_window && m_window->property("surfaceId").isValid()) {
                    registerMenu();
                } else {
                    unregisterMenu();
                }
                return true;
            }
        }
    }
    return false;
}
