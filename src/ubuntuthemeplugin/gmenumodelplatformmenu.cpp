/*
 * Copyright (C) 2015 Canonical, Ltd.
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
#include "gmenumodelplatformmenu.h"

// Qt
#include <QDebug>

#include <gio/gio.h>

#define ENABLE_DEBUG_LOGGING

#ifdef ENABLE_DEBUG_LOGGING
#define LOG qDebug() << __PRETTY_FUNCTION__ << " -> " << this
#define LOG_VAR(x) qDebug() << __PRETTY_FUNCTION__ << " -> " << this << " : " << #x ":" << x
#else
#define LOG QNoDebug()
#define LOG_VAR(x) QNoDebug() << __PRETTY_FUNCTION__<< #x ":" << x
#endif

#define OBJ_PATH "/org/gtk/TestMenus"
#define BUS_NAME "org.gtk.TestMenus"

inline QString actionString(const QString& label) {
    QRegExp re("\\W");
    QStringList parts = label.split(re, QString::SkipEmptyParts);
    for(auto iter = parts.begin(); iter != parts.end(); ++iter) {
        QString& part = *iter;
        QChar* ch = part.data();
        if (!ch) continue;
        *ch = ch->toUpper();
    }
    return parts.join("");
}

class GMenuModelParser
{
public:
    GMenuModelParser()
        : m_gmainMenu(g_menu_new())
        , m_gactionGroup(g_simple_action_group_new())
        , m_exportedModel(-1)
        , m_exportedActions(-1)
    {
        exportModels();
    }

    ~GMenuModelParser()
    {
        exportModels();
        clear();
    }

    int debugMenuLevel = 0;
    inline QDebug debugLog() const { return qDebug() << QString(debugMenuLevel*4, ' ').toUtf8().constData(); }

    void clear()
    {
        g_menu_remove_all(m_gmainMenu);

        for (auto iter = m_actions.begin(); iter != m_actions.end(); ++iter) {
            g_action_map_remove_action(G_ACTION_MAP(m_gactionGroup), g_action_get_name(G_ACTION(iter.value())));
        }
        m_actions.clear();
    }

    void exportModels()
    {
        GError *error = NULL;
        GDBusConnection *bus;
        bus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);

        if (m_exportedModel == -1) {
            m_exportedModel = g_dbus_connection_export_menu_model(bus, OBJ_PATH, G_MENU_MODEL(m_gmainMenu), &error);
            if (!m_exportedModel) {
                qWarning() << "Failed to export menu " << error->message;
            } else {
                qDebug() << "Exported menu on" << g_dbus_connection_get_unique_name(bus);
            }
        }

        if (m_exportedActions == -1) {
            m_exportedActions = g_dbus_connection_export_action_group(bus, OBJ_PATH, G_ACTION_GROUP(m_gactionGroup), &error);
            if (!m_exportedActions) {
                qWarning() << "Failed to export actions " << error->message;
            } else {
                qDebug() << "Exported actions on" << g_dbus_connection_get_unique_name(bus);
            }
        }
        g_bus_own_name_on_connection (bus, BUS_NAME, G_BUS_NAME_OWNER_FLAGS_NONE, NULL, NULL, NULL, NULL);
    }

    void unexportModels()
    {
        GDBusConnection *bus;
        bus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);

        if (m_exportedModel != -1) {
            g_dbus_connection_unexport_menu_model(bus, m_exportedModel);
            m_exportedModel = -1;
        }
        if (m_exportedActions != -1) {
            g_dbus_connection_unexport_action_group(bus, m_exportedActions);
            m_exportedActions = -1;
        }
    }

    GMenuItem* createSubmenu(QPlatformMenu* platformMenu)
    {
        debugMenuLevel++;
        GMenu* menu = g_menu_new();
        GMenuModelPlatformMenu* gplatformMenu = qobject_cast<GMenuModelPlatformMenu*>(platformMenu);
        if (!gplatformMenu) return nullptr;

        debugLog() << "start submenu" << GMenuModelPlatformMenu::get_text(gplatformMenu);

        auto iter = gplatformMenu->m_menuItems.begin();
        auto lastSectionStart = iter;
        for (; iter != gplatformMenu->m_menuItems.end(); ++iter) {
            GMenuModelPlatformMenuItem* gplatformMenuItem = qobject_cast<GMenuModelPlatformMenuItem*>(*iter);
            if (!gplatformMenuItem) continue;

            // don't add section until we have separator
            if (GMenuModelPlatformMenuItem::get_separator(gplatformMenuItem)) {
                if (lastSectionStart != gplatformMenu->m_menuItems.begin()) {
                    qDebug() << "APPEND SECTION";
                    GMenuItem* section = createSection(lastSectionStart, iter);
                    g_menu_append_item(menu, section);
                }
                lastSectionStart = iter + 1;
            } else if (lastSectionStart == gplatformMenu->m_menuItems.begin()) {
                processItemForGMenu(gplatformMenuItem, menu);
            }
        }

        if (lastSectionStart != gplatformMenu->m_menuItems.begin() &&
            lastSectionStart != gplatformMenu->m_menuItems.end()) {
            GMenuItem* section = createSection(lastSectionStart, gplatformMenu->m_menuItems.end());
            g_menu_append_item(menu, section);
        }

        QByteArray label(GMenuModelPlatformMenu::get_text(gplatformMenu).toUtf8());
        GMenuItem* gmenuItem = g_menu_item_new_submenu(label.constData(), G_MENU_MODEL(menu));
        debugLog() << "done submenu";

        debugMenuLevel--;
        return gmenuItem;
    }

    GMenuItem* createItem(QPlatformMenuItem* platformMenuItem)
    {
        GMenuModelPlatformMenuItem* gplatformItem = qobject_cast<GMenuModelPlatformMenuItem*>(platformMenuItem);
        if (!gplatformItem) return nullptr;

        QByteArray label(GMenuModelPlatformMenuItem::get_text(gplatformItem).toUtf8());
        GMenuItem* gmenuItem = g_menu_item_new(label.constData(), NULL);
        g_menu_item_set_detailed_action(gmenuItem, (QByteArray("unity.") + label).constData());

        createAction(gplatformItem);
        return gmenuItem;
    }

    GMenuItem* createSection(QList<QPlatformMenuItem*>::iterator iter, QList<QPlatformMenuItem*>::iterator end)
    {
        debugLog() << "start section";

        GMenu* sectionMenu = g_menu_new();
        for (; iter != end; ++iter) {
            processItemForGMenu(*iter, sectionMenu);
        }
        debugLog() << "done section";
        return g_menu_item_new_section("", G_MENU_MODEL(sectionMenu));
    }

    void processItemForGMenu(QPlatformMenuItem* item, GMenu* gmenu)
    {
        GMenuModelPlatformMenuItem* gplatformMenuItem = qobject_cast<GMenuModelPlatformMenuItem*>(item);
        if (!gplatformMenuItem) return;

        if (gplatformMenuItem->menu()) {
            debugLog() << "APPEND SUBMENU" << GMenuModelPlatformMenuItem::get_text(gplatformMenuItem);
            GMenuItem* subMenu = createSubmenu(gplatformMenuItem->menu());
            g_menu_append_item(gmenu, subMenu);
        } else {
            debugLog() << "APPEND ITEM" << GMenuModelPlatformMenuItem::get_text(gplatformMenuItem);
            GMenuItem* item = createItem(gplatformMenuItem);
            g_menu_append_item(gmenu, item);
        }
    }

    void restructuregmenumodel(GMenuModelPlatformMenuBar* bar)
    {
        clear();

        auto iter = bar->m_menus.begin();
        for (; iter != bar->m_menus.end(); ++iter) {
            debugLog() << "APPEND MENU";
            GMenuItem* item = createSubmenu(*iter);
            g_menu_append_item(m_gmainMenu, item);
        }
    }

    static void activate_cb(GSimpleAction *action,
                            GVariant      *,
                            gpointer       user_data)
    {
        auto item = (GMenuModelPlatformMenuItem*)user_data;
        item->activated();
        qDebug() << "ACTIVATE" << g_action_get_name(G_ACTION(action));
        GVariant* v = g_action_get_state(G_ACTION(action));
        gboolean state = !g_variant_get_boolean(v);
        g_simple_action_set_state(action, g_variant_new_boolean(state == TRUE ? FALSE : TRUE));

        g_variant_unref(v);
    }

    GSimpleAction* createAction(GMenuModelPlatformMenuItem* gplatformItem)
    {
        if (m_actions.contains(gplatformItem)) {
            g_action_map_remove_action(G_ACTION_MAP(m_gactionGroup), g_action_get_name(G_ACTION(m_actions[gplatformItem])));
            g_object_unref(m_actions.take(gplatformItem));
        }

        QByteArray actionName(actionString(GMenuModelPlatformMenuItem::get_text(gplatformItem)).toUtf8());
        bool checkable(GMenuModelPlatformMenuItem::get_checkable(gplatformItem));

        GSimpleAction* action = nullptr;
        if (checkable) {
            action = g_simple_action_new_stateful(actionName.constData(), NULL, g_variant_new_boolean(FALSE));
        } else {
            action = g_simple_action_new(actionName.constData(), NULL);
        }

        g_signal_connect(action,
                         "activate",
                         G_CALLBACK(activate_cb),
                         gplatformItem);

        m_actions[gplatformItem] = action;
        g_action_map_add_action(G_ACTION_MAP(m_gactionGroup), G_ACTION(action));
        debugLog() << "Added action" << action;
        return action;
    }

    GMenu* m_gmainMenu;
    GSimpleActionGroup* m_gactionGroup;
    QHash<GMenuModelPlatformMenuItem*, GSimpleAction*> m_actions;
    int m_exportedModel;
    int m_exportedActions;
};

GMenuModelPlatformMenuBar::GMenuModelPlatformMenuBar()
    : m_parser(new GMenuModelParser)
{
    LOG;
    connect(this, SIGNAL(menuInserted(QPlatformMenu*)), this, SIGNAL(structureChanged()));
    connect(this, SIGNAL(menuRemoved(QPlatformMenu*)), this, SIGNAL(structureChanged()));

    connect(this, &GMenuModelPlatformMenuBar::structureChanged, this, [this]() {
        m_structureTimer.start();
    });
    connect(&m_structureTimer, &QTimer::timeout, this, [this]() {
        m_parser->restructuregmenumodel(this);
    });
    m_structureTimer.setSingleShot(true);
    m_structureTimer.setInterval(0);
}

GMenuModelPlatformMenuBar::~GMenuModelPlatformMenuBar()
{
    delete m_parser;
    m_parser = nullptr;
    LOG;
}

void
GMenuModelPlatformMenuBar::insertMenu(QPlatformMenu *menu, QPlatformMenu *before)
{
    LOG_VAR(menu);

    if (!before) {
        m_menus.push_back(menu);
    } else {
        for (auto iter = m_menus.begin(); iter != m_menus.end(); ++iter) {
            if (*iter == before) {
                m_menus.insert(iter, menu);
                break;
            }
        }
    }
    connect(menu, SIGNAL(menuItemInserted(QPlatformMenuItem*)), this, SIGNAL(structureChanged()));

    Q_EMIT menuInserted(menu);
}

void
GMenuModelPlatformMenuBar::removeMenu(QPlatformMenu *menu)
{
    LOG_VAR(menu);

    for (auto iter = m_menus.begin(); iter != m_menus.end(); ++iter) {
        if (*iter == menu) {
            iter = m_menus.erase(iter);
        }
    }
    connect(menu, SIGNAL(removeMenuItem(QPlatformMenuItem*)), this, SIGNAL(structureChanged()));

    Q_EMIT menuRemoved(menu);
}

void
GMenuModelPlatformMenuBar::syncMenu(QPlatformMenu *menu)
{
    LOG_VAR(menu);
    Q_UNUSED(menu)
}

void
GMenuModelPlatformMenuBar::handleReparent(QWindow *newParentWindow)
{
    LOG_VAR(newParentWindow);
    Q_UNUSED(newParentWindow)
}

QPlatformMenu *
GMenuModelPlatformMenuBar::menuForTag(quintptr tag) const
{
    LOG_VAR(tag);

    for (auto iter = m_menus.begin(); iter != m_menus.end(); ++iter) {
        if ((*iter)->tag() == tag) {
            return *iter;
        }
    }
    return nullptr;
}

QPlatformMenu *
GMenuModelPlatformMenuBar::menuAt(int position) const
{
    if (position < 0 || position >= m_menus.count()) return nullptr;
    return m_menus.at(position);
}

uint GMenuModelPlatformMenuBar::menuCount()
{
    return m_menus.count();
}

//////////////////////////////////////////////////////////////

GMenuModelPlatformMenu::GMenuModelPlatformMenu()
{
    LOG;
}

GMenuModelPlatformMenu::~GMenuModelPlatformMenu()
{
    LOG;
}

void GMenuModelPlatformMenu::insertMenuItem(QPlatformMenuItem *menuItem, QPlatformMenuItem *before)
{
    LOG_VAR(menuItem);

    if (!before) {
        m_menuItems.push_back(menuItem);
    } else {
        for (auto iter = m_menuItems.begin(); iter != m_menuItems.end(); ++iter) {
            if (*iter == before) {
                m_menuItems.insert(iter, menuItem);
                break;
            }
        }
    }

    Q_EMIT menuItemInserted(menuItem);
}

void GMenuModelPlatformMenu::removeMenuItem(QPlatformMenuItem *menuItem)
{
    LOG_VAR(menuItem);

    QMutableListIterator<QPlatformMenuItem*> iter(m_menuItems);
    while (iter.hasNext()) {
        if (iter.next() == menuItem) {
            iter.remove();
        }
    }

    Q_EMIT menuItemRemoved(menuItem);
}

void GMenuModelPlatformMenu::syncMenuItem(QPlatformMenuItem *menuItem)
{
    LOG_VAR(menuItem);
    Q_UNUSED(menuItem)
}

void GMenuModelPlatformMenu::syncSeparatorsCollapsible(bool enable)
{
    LOG_VAR(enable);
    Q_UNUSED(enable)
}

void GMenuModelPlatformMenu::setTag(quintptr tag)
{
    m_tag = tag;
}

quintptr GMenuModelPlatformMenu::tag() const
{
    return m_tag;
}

void GMenuModelPlatformMenu::setText(const QString &text)
{
    set_text(this, text);
    Q_EMIT propertyUpdated();
}

void GMenuModelPlatformMenu::setIcon(const QIcon &icon)
{
    set_icon(this, icon);
    Q_EMIT propertyUpdated();
}

void GMenuModelPlatformMenu::setEnabled(bool isEnabled)
{
    set_enabled(this, isEnabled);
    Q_EMIT propertyUpdated();
}

void GMenuModelPlatformMenu::setVisible(bool isVisible)
{
    set_visible(this, isVisible);
    Q_EMIT propertyUpdated();
}

void GMenuModelPlatformMenu::setMinimumWidth(int width)
{
    Q_UNUSED(width)
}

void GMenuModelPlatformMenu::setFont(const QFont &font)
{
    Q_UNUSED(font)
}

QPlatformMenuItem *GMenuModelPlatformMenu::menuItemAt(int position) const
{
    if (position < 0 || position >= m_menuItems.count()) return nullptr;
    return m_menuItems.at(position);
}

QPlatformMenuItem *GMenuModelPlatformMenu::menuItemForTag(quintptr tag) const
{
    for (auto iter = m_menuItems.begin(); iter != m_menuItems.end(); ++iter) {
        if ((*iter)->tag() == tag) {
            return *iter;
        }
    }
    return nullptr;
}

uint GMenuModelPlatformMenu::menuItemCount()
{
    return 0;
}

QPlatformMenuItem *GMenuModelPlatformMenu::createMenuItem() const
{
    return new GMenuModelPlatformMenuItem();
}

//////////////////////////////////////////////////////////////

GMenuModelPlatformMenuItem::GMenuModelPlatformMenuItem()
    : m_menu(nullptr)
{
    LOG;
}

GMenuModelPlatformMenuItem::~GMenuModelPlatformMenuItem()
{
    LOG;
}

void GMenuModelPlatformMenuItem::setTag(quintptr tag)
{
    m_tag = tag;
}

quintptr GMenuModelPlatformMenuItem::tag() const
{
    return m_tag;
}

void GMenuModelPlatformMenuItem::setText(const QString &text)
{
    set_text(this, text);
    Q_EMIT propertyUpdated();
}

void GMenuModelPlatformMenuItem::setIcon(const QIcon &icon)
{
    set_icon(this, icon);
    Q_EMIT propertyUpdated();
}

void GMenuModelPlatformMenuItem::setVisible(bool isVisible)
{
    set_visible(this, isVisible);
    Q_EMIT propertyUpdated();
}

void GMenuModelPlatformMenuItem::setIsSeparator(bool isSeparator)
{
    set_separator(this, isSeparator);
    Q_EMIT propertyUpdated();
}

void GMenuModelPlatformMenuItem::setFont(const QFont &font)
{
    Q_UNUSED(font);
}

void GMenuModelPlatformMenuItem::setRole(QPlatformMenuItem::MenuRole role)
{
    Q_UNUSED(role);
}

void GMenuModelPlatformMenuItem::setCheckable(bool checkable)
{
    set_checkable(this, checkable);
    Q_EMIT propertyUpdated();
}

void GMenuModelPlatformMenuItem::setChecked(bool isChecked)
{
    set_checked(this, isChecked);
    Q_EMIT propertyUpdated();
}

void GMenuModelPlatformMenuItem::setShortcut(const QKeySequence &shortcut)
{
    set_shortcut(this, shortcut);
    Q_EMIT propertyUpdated();
}

void GMenuModelPlatformMenuItem::setEnabled(bool enabled)
{
    set_enabled(this, enabled);
    Q_EMIT propertyUpdated();
}

void GMenuModelPlatformMenuItem::setIconSize(int size)
{
    set_iconSize(this, size);
    Q_EMIT propertyUpdated();
}

void GMenuModelPlatformMenuItem::setMenu(QPlatformMenu *menu)
{
    LOG_VAR(menu);
    m_menu = menu;
    Q_EMIT propertyUpdated();
}

QPlatformMenu *GMenuModelPlatformMenuItem::menu() const
{
    return m_menu;
}
