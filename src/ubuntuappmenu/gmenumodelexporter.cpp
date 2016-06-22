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

// Local
#include "gmenumodelexporter.h"
#include "registry.h"
#include "logging.h"

#include <QDebug>

#include <functional>

namespace {

inline QString getActionString(const QString& label) {
    QRegExp re("\\W");
    QStringList parts = label.split(re, QString::SkipEmptyParts);
    for(auto iter = parts.begin(); iter != parts.end(); ++iter) {
        QString& part = *iter;
        QChar* ch = part.data();
        if (!ch) continue;
        *ch = ch->toUpper();
    }
    return parts.join("").replace(QRegExp("(&|_)"), "");
}

static void activate_cb(GSimpleAction *action, GVariant *, gpointer user_data)
{
    qCDebug(ubuntuappmenu, "Activate menu action '%s'", g_action_get_name(G_ACTION(action)));
    auto item = (GMenuModelPlatformMenuItem*)user_data;
    item->activated();
}

static uint s_menuId = 0;

#define MENU_OBJECT_PATH QString("/com/ubuntu/Menu/%1")

} // namespace

GMenuModelExporter::GMenuModelExporter(GMenuModelPlatformMenuBar * bar)
    : QObject(bar)
    , m_gmainMenu(g_menu_new())
    , m_gactionGroup(g_simple_action_group_new())
    , m_exportedModel(-1)
    , m_exportedActions(-1)
    , m_menuPath(MENU_OBJECT_PATH.arg(s_menuId++))
{
    qCDebug(ubuntuappmenu, "GMenuModelExporter::GMenuModelExporter");

    connect(bar, &GMenuModelPlatformMenuBar::structureChanged, this, [this]() {
        m_structureTimer.start();
    });
    connect(&m_structureTimer, &QTimer::timeout, this, [this, bar]() {
        clear();
        auto iter = bar->menus().begin();
        for (; iter != bar->menus().end(); ++iter) {
            GMenuItem* item = createSubmenu(*iter, nullptr);
            if (item) {
                g_menu_append_item(m_gmainMenu, item);
                g_object_unref(item);
            }
        }
    });
    m_structureTimer.setSingleShot(true);
    m_structureTimer.setInterval(0);

    connect(bar, &GMenuModelPlatformMenuBar::ready, this, [this]() {
        exportModels();
    });
}

GMenuModelExporter::GMenuModelExporter(GMenuModelPlatformMenu *menu)
    : QObject(menu)
    , m_gmainMenu(g_menu_new())
    , m_gactionGroup(g_simple_action_group_new())
    , m_exportedModel(-1)
    , m_exportedActions(-1)
    , m_menuPath(MENU_OBJECT_PATH.arg(s_menuId++))
{
    qCDebug(ubuntuappmenu, "GMenuModelExporter::GMenuModelExporter");

    connect(menu, &GMenuModelPlatformMenu::structureChanged, this, [this]() {
        m_structureTimer.start();
    });
    connect(&m_structureTimer, &QTimer::timeout, this, [this, menu]() {
        clear();
        addSubmenuItems(menu, m_gmainMenu);
    });
    m_structureTimer.setSingleShot(true);
    m_structureTimer.setInterval(0);

    addSubmenuItems(menu, m_gmainMenu);
}

GMenuModelExporter::~GMenuModelExporter()
{
    qCDebug(ubuntuappmenu, "GMenuModelExporter::~GMenuModelExporter");

    unexportModels();
    clear();

    g_object_unref(m_gmainMenu);
    g_object_unref(m_gactionGroup);
}

void GMenuModelExporter::clear()
{
    Q_FOREACH(const QMetaObject::Connection& connection, m_propertyConnections) {
        QObject::disconnect(connection);
    }

    g_menu_remove_all(m_gmainMenu);

    for (auto iter = m_actions.begin(); iter != m_actions.end(); ++iter) {
        g_action_map_remove_action(G_ACTION_MAP(m_gactionGroup), (*iter).constData());
    }
    m_actions.clear();
}

void GMenuModelExporter::exportModels()
{
    GError *error = NULL;
    GDBusConnection *bus;
    bus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
    if (!bus) {
        qCWarning(ubuntuappmenu, "Failed to retreive session bus - %s", error ? error->message : "unknown error");
        return;
    }

    QByteArray menuPath(m_menuPath.toUtf8());

    if (m_exportedModel == -1) {
        m_exportedModel = g_dbus_connection_export_menu_model(bus, menuPath.constData(), G_MENU_MODEL(m_gmainMenu), &error);
        if (!m_exportedModel) {
            qCWarning(ubuntuappmenu, "Failed to export menu - %s", error ? error->message : "unknown error");
            error = NULL;
        } else {
            qCDebug(ubuntuappmenu, "Exported menu on %s", g_dbus_connection_get_unique_name(bus));
        }
    }

    if (m_exportedActions == -1) {
        m_exportedActions = g_dbus_connection_export_action_group(bus, menuPath.constData(), G_ACTION_GROUP(m_gactionGroup), &error);
        if (!m_exportedActions) {
            qCWarning(ubuntuappmenu, "Failed to export actions - %s", error ? error->message : "unknown error");
            error = NULL;
        } else {
            qCDebug(ubuntuappmenu, "Exported actions on %s", g_dbus_connection_get_unique_name(bus));
        }
    }
}

void GMenuModelExporter::unexportModels()
{
    GError *error = NULL;
    GDBusConnection *bus;
    bus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
    if (!bus) {
        qCWarning(ubuntuappmenu, "Failed to retreive session bus - %s", error ? error->message : "unknown error");
        return;
    }

    if (m_exportedModel != -1) {
        g_dbus_connection_unexport_menu_model(bus, m_exportedModel);
        m_exportedModel = -1;
    }
    if (m_exportedActions != -1) {
        g_dbus_connection_unexport_action_group(bus, m_exportedActions);
        m_exportedActions = -1;
    }
}

GMenuItem *GMenuModelExporter::createSubmenu(QPlatformMenu *platformMenu, GMenuModelPlatformMenuItem *forItem)
{
    GMenuModelPlatformMenu* gplatformMenu = qobject_cast<GMenuModelPlatformMenu*>(platformMenu);
    if (!gplatformMenu) return nullptr;
    GMenu* menu = g_menu_new();

    QByteArray label;
    if (forItem) {
        label = GMenuModelPlatformMenuItem::get_text(forItem).toUtf8();
    } else {
        label = GMenuModelPlatformMenu::get_text(gplatformMenu).toUtf8();
    }

    addSubmenuItems(gplatformMenu, menu);

    GMenuItem* gmenuItem = g_menu_item_new_submenu(label.constData(), G_MENU_MODEL(menu));
    g_object_unref(menu);
    return gmenuItem;
}

void GMenuModelExporter::addSubmenuItems(GMenuModelPlatformMenu* gplatformMenu, GMenu* menu)
{
    auto iter = gplatformMenu->menuItems().begin();
    auto lastSectionStart = iter;
    for (; iter != gplatformMenu->menuItems().end(); ++iter) {
        GMenuModelPlatformMenuItem* gplatformMenuItem = qobject_cast<GMenuModelPlatformMenuItem*>(*iter);
        if (!gplatformMenuItem) continue;

        // don't add section until we have separator
        if (GMenuModelPlatformMenuItem::get_separator(gplatformMenuItem)) {
            if (lastSectionStart != gplatformMenu->menuItems().begin()) {
                GMenuItem* section = createSection(lastSectionStart, iter);
                g_menu_append_item(menu, section);
                g_object_unref(section);
            }
            lastSectionStart = iter + 1;
        } else if (lastSectionStart == gplatformMenu->menuItems().begin()) {
            processItemForGMenu(gplatformMenuItem, menu);
        }
    }

    if (lastSectionStart != gplatformMenu->menuItems().begin() &&
            lastSectionStart != gplatformMenu->menuItems().end()) {
        GMenuItem* gsectionItem = createSection(lastSectionStart, gplatformMenu->menuItems().end());
        g_menu_append_item(menu, gsectionItem);
        g_object_unref(gsectionItem);
    }
}

GMenuItem *GMenuModelExporter::createMenuItem(QPlatformMenuItem *platformMenuItem)
{
    GMenuModelPlatformMenuItem* gplatformMenuItem = qobject_cast<GMenuModelPlatformMenuItem*>(platformMenuItem);
    if (!gplatformMenuItem) return nullptr;

    QByteArray label(GMenuModelPlatformMenuItem::get_text(gplatformMenuItem).toUtf8());
    QByteArray actionLabel(getActionString(GMenuModelPlatformMenuItem::get_text(gplatformMenuItem)).toUtf8());
    QByteArray shortcut(GMenuModelPlatformMenuItem::get_shortcut(gplatformMenuItem).toString(QKeySequence::NativeText).toUtf8());

    GMenuItem* gmenuItem = g_menu_item_new(label.constData(), NULL);
    g_menu_item_set_attribute(gmenuItem, "accel", "s", shortcut.constData());
    g_menu_item_set_detailed_action(gmenuItem, ("unity." + actionLabel).constData());

    createAction(actionLabel, gplatformMenuItem);
    return gmenuItem;
}

GMenuItem *GMenuModelExporter::createSection(QList<QPlatformMenuItem *>::const_iterator iter, QList<QPlatformMenuItem *>::const_iterator end)
{
    GMenu* gsectionMenu = g_menu_new();
    for (; iter != end; ++iter) {
        processItemForGMenu(*iter, gsectionMenu);
    }
    GMenuItem* gsectionItem = g_menu_item_new_section("", G_MENU_MODEL(gsectionMenu));
    g_object_unref(gsectionMenu);
    return gsectionItem;
}

void GMenuModelExporter::processItemForGMenu(QPlatformMenuItem *platformMenuItem, GMenu *gmenu)
{
    GMenuModelPlatformMenuItem* gplatformMenuItem = qobject_cast<GMenuModelPlatformMenuItem*>(platformMenuItem);
    if (!gplatformMenuItem) return;

    GMenuItem* gmenuItem = gplatformMenuItem->menu() ? createSubmenu(gplatformMenuItem->menu(), gplatformMenuItem) :
                                                       createMenuItem(gplatformMenuItem);
    if (gmenuItem) {
        g_menu_append_item(gmenu, gmenuItem);
        g_object_unref(gmenuItem);
    }
}

void GMenuModelExporter::createAction(const QByteArray &name, GMenuModelPlatformMenuItem *gplatformMenuItem)
{
    disconnect(gplatformMenuItem, &GMenuModelPlatformMenuItem::checkedChanged, this, 0);
    disconnect(gplatformMenuItem, &GMenuModelPlatformMenuItem::enabledChanged, this, 0);

    if (m_actions.contains(name)) {
        g_action_map_remove_action(G_ACTION_MAP(m_gactionGroup), name.constData());
        m_actions.remove(name);
    }

    bool checkable(GMenuModelPlatformMenuItem::get_checkable(gplatformMenuItem));

    GSimpleAction* action = nullptr;
    if (checkable) {
        bool checked(GMenuModelPlatformMenuItem::get_checked(gplatformMenuItem));
        action = g_simple_action_new_stateful(name.constData(), NULL, g_variant_new_boolean(checked));

        std::function<void(bool)> updateChecked = [gplatformMenuItem, action](bool checked) {
            auto type = g_action_get_state_type(G_ACTION(action));
            if (type && g_variant_type_equal(type, G_VARIANT_TYPE_BOOLEAN)) {
                g_simple_action_set_state(action, g_variant_new_boolean(checked ? TRUE : FALSE));
            }
        };
        // save the connection to disconnect in GMenuModelExporter::clear()
        m_propertyConnections << connect(gplatformMenuItem, &GMenuModelPlatformMenuItem::checkedChanged, this, updateChecked);
    } else {
        action = g_simple_action_new(name.constData(), NULL);
    }

    // Enabled update
    std::function<void(bool)> updateEnabled = [gplatformMenuItem, action](bool enabled) {
        GValue value = G_VALUE_INIT;
        g_value_init (&value, G_TYPE_BOOLEAN);
        g_value_set_boolean(&value, enabled ? TRUE : FALSE);
        g_object_set_property(G_OBJECT(action), "enabled", &value);
    };
    updateEnabled(GMenuModelPlatformMenuItem::get_enabled(gplatformMenuItem));
    // save the connection to disconnect in GMenuModelExporter::clear()
    m_propertyConnections << connect(gplatformMenuItem, &GMenuModelPlatformMenuItem::enabledChanged, this, updateEnabled);

    g_signal_connect(action, "activate", G_CALLBACK(activate_cb), gplatformMenuItem);

    m_actions.insert(name);
    g_action_map_add_action(G_ACTION_MAP(m_gactionGroup), G_ACTION(action));
    g_object_unref(action);
}
