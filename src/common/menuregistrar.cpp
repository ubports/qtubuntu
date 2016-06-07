#include "menuregistrar.h"
#include "registry.h"
#include "logging.h"

#include <QDebug>
#include <QDBusObjectPath>

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
    unregisterSurfaceMenu();
}

void MenuRegistrar::registerSurfaceMenuForWindow(QWindow* window, const QDBusObjectPath& path)
{
    qCWarning(qtubuntuMenus).nospace() << "MenuRegistrar::registerSurfaceMenuForWindow(window=" << window << ", path=" << path.path() << ")";

    if (!m_registeredSurfaceId.isEmpty()) unregisterSurfaceMenu();
    if (!m_window.isNull()) m_window->removeEventFilter(this);

    m_window = window;
    m_path = path;

    if (window) {
        if (window->property("surfaceId").isValid()) {
            registerSurfaceMenu();
        }
        window->installEventFilter(this);
    } else {
        qCWarning(qtubuntuMenus, "No window for menu registration");
    }
}

void MenuRegistrar::registerSurfaceMenu()
{
    if (!UbuntuMenuRegistry::instance()->isConnected()) return;

    const QString surfaceId = m_window->property("surfaceId").toString();
    UbuntuMenuRegistry::instance()->registerSurfaceMenu(surfaceId, m_path, m_service);
    m_registeredSurfaceId = surfaceId;
}

void MenuRegistrar::unregisterSurfaceMenu()
{
    if (m_registeredSurfaceId.isEmpty()) return;
    if (!UbuntuMenuRegistry::instance()->isConnected()) return;

    UbuntuMenuRegistry::instance()->unregisterSurfaceMenu(m_registeredSurfaceId, m_path);
    m_registeredSurfaceId.clear();
}

void MenuRegistrar::onRegistrarServiceChanged()
{
    if (!m_registeredSurfaceId.isEmpty()) {
        unregisterSurfaceMenu();
    }
    if (UbuntuMenuRegistry::instance()->isConnected()) {
        if (m_window && m_window->property("surfaceId").isValid()) {
            registerSurfaceMenu();
        }
    }
}

bool MenuRegistrar::eventFilter(QObject * watched, QEvent * event) {
    if (watched == m_window) {
        if (event->type() == QEvent::DynamicPropertyChange) {
            QDynamicPropertyChangeEvent* propertyEvent = static_cast<QDynamicPropertyChangeEvent*>(event);
            if (propertyEvent->propertyName() == "surfaceId") {
                if (m_window && m_window->property("surfaceId").isValid()) {
                    registerSurfaceMenu();
                } else {
                    unregisterSurfaceMenu();
                }
                return true;
            }
        }
    }
    return false;
}
