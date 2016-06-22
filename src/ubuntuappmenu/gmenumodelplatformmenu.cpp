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
#include "gmenumodelplatformmenu.h"
#include "gmenumodelexporter.h"
#include "registry.h"
#include "menuregistrar.h"
#include "logging.h"

// Qt
#include <QDebug>
#include <QWindow>
#include <QCoreApplication>

#define BAR_DEBUG_MSG qCDebug(ubuntuappmenu).nospace() << "GMenuModelPlatformMenuBar[" << (void*)this <<"]::" << __func__
#define MENU_DEBUG_MSG qCDebug(ubuntuappmenu).nospace() << "GMenuModelPlatformMenu[" << (void*)this <<"]::" << __func__
#define ITEM_DEBUG_MSG qCDebug(ubuntuappmenu).nospace() << "GMenuModelPlatformMenuItem[" << (void*)this <<"]::" << __func__

int logRecusion = 0;

QDebug operator<<(QDebug stream, GMenuModelPlatformMenuBar* bar) {
    if (bar) return bar->operator<<(stream);
    return stream;
}
QDebug operator<<(QDebug stream, GMenuModelPlatformMenu* menu) {
    if (menu) return menu->operator<<(stream);
    return stream;
}
QDebug operator<<(QDebug stream, GMenuModelPlatformMenuItem* menuItem) {
    if (menuItem) return menuItem->operator<<(stream);
    return stream;
}

GMenuModelPlatformMenuBar::GMenuModelPlatformMenuBar()
    : m_exporter(new GMenuModelExporter(this))
    , m_registrar(new MenuRegistrar())
    , m_ready(false)
{
    BAR_DEBUG_MSG << "()";

    connect(this, SIGNAL(menuInserted(QPlatformMenu*)), this, SIGNAL(structureChanged()));
    connect(this, SIGNAL(menuRemoved(QPlatformMenu*)), this, SIGNAL(structureChanged()));
}

GMenuModelPlatformMenuBar::~GMenuModelPlatformMenuBar()
{
    BAR_DEBUG_MSG << "()";

    delete m_registrar;
    m_registrar = nullptr;

    delete m_exporter;
    m_exporter = nullptr;
}

void
GMenuModelPlatformMenuBar::insertMenu(QPlatformMenu *menu, QPlatformMenu *before)
{
    BAR_DEBUG_MSG << "(menu=" << menu << ", before=" <<  before << ")";

    if (m_menus.contains(menu)) return;

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
    connect(menu, SIGNAL(structureChanged()), this, SIGNAL(structureChanged()));
    Q_EMIT menuInserted(menu);
}

void
GMenuModelPlatformMenuBar::removeMenu(QPlatformMenu *menu)
{
    BAR_DEBUG_MSG << "(menu=" << menu << ")";

    for (auto iter = m_menus.begin(); iter != m_menus.end(); ++iter) {
        if (*iter == menu) {
            m_menus.erase(iter);
            break;
        }
    }
    connect(menu, SIGNAL(structureChanged()), this, SIGNAL(structureChanged()));
    Q_EMIT menuRemoved(menu);
}

void
GMenuModelPlatformMenuBar::syncMenu(QPlatformMenu *menu)
{
    BAR_DEBUG_MSG << "(menu=" << menu << ")";

    Q_UNUSED(menu)
}

void
GMenuModelPlatformMenuBar::handleReparent(QWindow *parentWindow)
{
    BAR_DEBUG_MSG << "(parentWindow=" << parentWindow << ")";

    setReady(true);
    m_registrar->registerSurfaceMenuForWindow(parentWindow, QDBusObjectPath(m_exporter->menuPath()));
}

QPlatformMenu *
GMenuModelPlatformMenuBar::menuForTag(quintptr tag) const
{
    for (auto iter = m_menus.begin(); iter != m_menus.end(); ++iter) {
        if ((*iter)->tag() == tag) {
            return *iter;
        }
    }
    return nullptr;
}

const QList<QPlatformMenu *> GMenuModelPlatformMenuBar::menus() const
{
    return m_menus;
}

QDebug GMenuModelPlatformMenuBar::operator<<(QDebug stream)
{
    stream.nospace().noquote() << QString("%1").arg("", logRecusion, QLatin1Char('\t'))
            << "GMenuModelPlatformMenuBar(this=" << (void*)this << ")" << endl;
    Q_FOREACH(QPlatformMenu* menu, m_menus) {
        auto myMenu = qobject_cast<GMenuModelPlatformMenu*>(menu);
        if (myMenu) {
            logRecusion++;
            stream << myMenu;
            logRecusion--;
        }
    }

    return stream;
}

void GMenuModelPlatformMenuBar::setReady(bool _ready)
{
    if (m_ready != _ready) {
        m_ready = _ready;
        Q_EMIT ready();
    }
}

//////////////////////////////////////////////////////////////

GMenuModelPlatformMenu::GMenuModelPlatformMenu()
    : m_parentWindow(nullptr)
    , m_exporter(nullptr)
    , m_registrar(nullptr)
{
    MENU_DEBUG_MSG << "()";

    connect(this, SIGNAL(menuItemInserted(QPlatformMenuItem*)), this, SIGNAL(structureChanged()));
    connect(this, SIGNAL(menuItemRemoved(QPlatformMenuItem*)), this, SIGNAL(structureChanged()));
}

GMenuModelPlatformMenu::~GMenuModelPlatformMenu()
{
    MENU_DEBUG_MSG << "()";
}

void GMenuModelPlatformMenu::insertMenuItem(QPlatformMenuItem *menuItem, QPlatformMenuItem *before)
{
    MENU_DEBUG_MSG << "(menuItem=" << menuItem << ", before=" << before << ")";

    if (m_menuItems.contains(menuItem)) return;

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
    MENU_DEBUG_MSG << "(menuItem=" << menuItem << ")";

    for (auto iter = m_menuItems.begin(); iter != m_menuItems.end(); ++iter) {
        if (*iter == menuItem) {
            m_menuItems.erase(iter);
            break;
        }
    }
    Q_EMIT menuItemRemoved(menuItem);
}

void GMenuModelPlatformMenu::syncMenuItem(QPlatformMenuItem *menuItem)
{
    MENU_DEBUG_MSG << "(menuItem=" << menuItem << ")";

    Q_UNUSED(menuItem)
}

void GMenuModelPlatformMenu::syncSeparatorsCollapsible(bool enable)
{
    MENU_DEBUG_MSG << "(enable=" << enable << ")";
    Q_UNUSED(enable)
}

void GMenuModelPlatformMenu::setTag(quintptr tag)
{
    MENU_DEBUG_MSG << "(tag=" << tag << ")";
    m_tag = tag;
}

quintptr GMenuModelPlatformMenu::tag() const
{
    return m_tag;
}

void GMenuModelPlatformMenu::setText(const QString &text)
{
    MENU_DEBUG_MSG << "(text=" << text << ")";
    if (m_text != text) {
        m_text = text;
        Q_EMIT propertyChanged();
    }
}

void GMenuModelPlatformMenu::setIcon(const QIcon &icon)
{
    MENU_DEBUG_MSG << "(icon=" << icon.name() << ")";

    if (!icon.isNull() || (!m_icon.isNull() && icon.isNull())) {
        m_icon = icon;
        Q_EMIT propertyChanged();
    }
}

void GMenuModelPlatformMenu::setEnabled(bool enabled)
{
    MENU_DEBUG_MSG << "(enabled=" << enabled << ")";

    if (m_enabled != enabled) {
        m_enabled = enabled;
        Q_EMIT propertyChanged();
    }
}

void GMenuModelPlatformMenu::setVisible(bool isVisible)
{
    MENU_DEBUG_MSG << "(visible=" << isVisible << ")";

    if (m_visible != isVisible) {
        m_visible = isVisible;
        Q_EMIT propertyChanged();
    }
}

void GMenuModelPlatformMenu::setMinimumWidth(int width)
{
    MENU_DEBUG_MSG << "(width=" << width << ")";

    Q_UNUSED(width)
}

void GMenuModelPlatformMenu::setFont(const QFont &font)
{
    MENU_DEBUG_MSG << "(font=" << font << ")";

    Q_UNUSED(font)
}

void GMenuModelPlatformMenu::showPopup(const QWindow *parentWindow, const QRect &targetRect, const QPlatformMenuItem *item)
{
    MENU_DEBUG_MSG << "(parentWindow=" << parentWindow << ", targetRect=" << targetRect << ", item=" << item << ")";

    if (!m_exporter) {
        m_exporter = new GMenuModelExporter(this);
        m_exporter->exportModels();
    }

    if (parentWindow != m_parentWindow) {
        if (m_parentWindow) {
            m_registrar->unregisterSurfaceMenu();
        }

        m_parentWindow = parentWindow;

        if (m_parentWindow) {
            if (!m_registrar) m_registrar = new MenuRegistrar;
            m_registrar->registerSurfaceMenuForWindow(const_cast<QWindow*>(m_parentWindow),
                                                      QDBusObjectPath(m_exporter->menuPath()));
        }
    }

    Q_UNUSED(targetRect);
    Q_UNUSED(item);
    setVisible(true);
}

void GMenuModelPlatformMenu::dismiss()
{
    MENU_DEBUG_MSG << "()";

    if (m_registrar) { m_registrar->unregisterSurfaceMenu(); }
    if (m_exporter) { m_exporter->unexportModels(); }
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

const QList<QPlatformMenuItem *> GMenuModelPlatformMenu::menuItems() const
{
    return m_menuItems;
}

QDebug GMenuModelPlatformMenu::operator<<(QDebug stream)
{
    stream.nospace().noquote() << QString("%1").arg("", logRecusion, QLatin1Char('\t'))
            << "GMenuModelPlatformMenu(this=" << (void*)this << ", text=\"" << m_text << "\")" << endl;
    Q_FOREACH(QPlatformMenuItem* item, m_menuItems) {
        logRecusion++;
        auto myItem = qobject_cast<GMenuModelPlatformMenuItem*>(item);
        if (myItem) {
            stream << myItem;
        }
        logRecusion--;
    }
    return stream;
}

//////////////////////////////////////////////////////////////

GMenuModelPlatformMenuItem::GMenuModelPlatformMenuItem()
    : m_menu(nullptr)
{
    ITEM_DEBUG_MSG << "()";
}

GMenuModelPlatformMenuItem::~GMenuModelPlatformMenuItem()
{
    ITEM_DEBUG_MSG << "()";
}

void GMenuModelPlatformMenuItem::setTag(quintptr tag)
{
    ITEM_DEBUG_MSG << "(tag=" << tag << ")";
    m_tag = tag;
}

quintptr GMenuModelPlatformMenuItem::tag() const
{
    return m_tag;
}

void GMenuModelPlatformMenuItem::setText(const QString &text)
{
    ITEM_DEBUG_MSG << "(text=" << text << ")";
    if (m_text != text) {
        m_text = text;
        Q_EMIT propertyChanged();
    }
}

void GMenuModelPlatformMenuItem::setIcon(const QIcon &icon)
{
    ITEM_DEBUG_MSG << "(icon=" << icon.name() << ")";

    if (!icon.isNull() || (!m_icon.isNull() && icon.isNull())) {
        m_icon = icon;
        Q_EMIT propertyChanged();
    }
}

void GMenuModelPlatformMenuItem::setVisible(bool isVisible)
{
    ITEM_DEBUG_MSG << "(visible=" << isVisible << ")";
    if (m_visible != isVisible) {
        m_visible = isVisible;
        Q_EMIT propertyChanged();
    }
}

void GMenuModelPlatformMenuItem::setIsSeparator(bool isSeparator)
{
    ITEM_DEBUG_MSG << "(separator=" << isSeparator << ")";
    if (m_separator != isSeparator) {
        m_separator = isSeparator;
        Q_EMIT propertyChanged();
    }
}

void GMenuModelPlatformMenuItem::setFont(const QFont &font)
{
    ITEM_DEBUG_MSG << "(font=" << font << ")";
    Q_UNUSED(font);
}

void GMenuModelPlatformMenuItem::setRole(QPlatformMenuItem::MenuRole role)
{
    ITEM_DEBUG_MSG << "(role=" << role << ")";
    Q_UNUSED(role);
}

void GMenuModelPlatformMenuItem::setCheckable(bool checkable)
{
    ITEM_DEBUG_MSG << "(checkable=" << checkable << ")";
    if (m_checkable != checkable) {
        m_checkable = checkable;
        Q_EMIT propertyChanged();
    }
}

void GMenuModelPlatformMenuItem::setChecked(bool isChecked)
{
    ITEM_DEBUG_MSG << "(checked=" << isChecked << ")";
    if (m_checked != isChecked) {
        m_checked = isChecked;
        Q_EMIT checkedChanged(isChecked);
        Q_EMIT propertyChanged();
    }
}

void GMenuModelPlatformMenuItem::setShortcut(const QKeySequence &shortcut)
{
    ITEM_DEBUG_MSG << "(shortcut=" << shortcut << ")";
    if (m_shortcut != shortcut) {
        m_shortcut = shortcut;
        Q_EMIT propertyChanged();
    }
}

void GMenuModelPlatformMenuItem::setEnabled(bool enabled)
{
    ITEM_DEBUG_MSG << "(enabled=" << enabled << ")";
    if (m_enabled != enabled) {
        m_enabled = enabled;
        Q_EMIT enabledChanged(enabled);
        Q_EMIT propertyChanged();
    }
}

void GMenuModelPlatformMenuItem::setIconSize(int size)
{
    ITEM_DEBUG_MSG << "(size=" << size << ")";
    Q_UNUSED(size);
}

void GMenuModelPlatformMenuItem::setMenu(QPlatformMenu *menu)
{
    ITEM_DEBUG_MSG << "(menu=" << menu << ")";
    if (m_menu != menu) {
        m_menu = menu;
        Q_EMIT propertyChanged();
    }
}

QPlatformMenu *GMenuModelPlatformMenuItem::menu() const
{
    return m_menu;
}

QDebug GMenuModelPlatformMenuItem::operator<<(QDebug stream)
{
    QString properties = "text=\"" + m_text + "\"";

    stream.nospace().noquote() << QString("%1").arg("", logRecusion, QLatin1Char('\t'))
            << "GMenuModelPlatformMenuItem(this=" << (void*)this << ", "
            << (m_separator ? "Separator" : properties) << ")" << endl;
    if (m_menu) {
        auto myMenu = qobject_cast<GMenuModelPlatformMenu*>(m_menu);
        if (myMenu) {
            logRecusion++;
            stream << myMenu;
            logRecusion--;
        }
    }
    return stream;
}
