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

#define BAR_DEBUG_MSG qCDebug(ubuntuappmenu).nospace() << "UbuntuPlatformMenuBar[" << (void*)this <<"]::" << __func__
#define MENU_DEBUG_MSG qCDebug(ubuntuappmenu).nospace() << "UbuntuPlatformMenu[" << (void*)this <<"]::" << __func__
#define ITEM_DEBUG_MSG qCDebug(ubuntuappmenu).nospace() << "UbuntuPlatformMenuItem[" << (void*)this <<"]::" << __func__

namespace {

int logRecusion = 0;

}

QDebug operator<<(QDebug stream, UbuntuPlatformMenuBar* bar) {
    if (bar) return bar->operator<<(stream);
    return stream;
}
QDebug operator<<(QDebug stream, UbuntuPlatformMenu* menu) {
    if (menu) return menu->operator<<(stream);
    return stream;
}
QDebug operator<<(QDebug stream, UbuntuPlatformMenuItem* menuItem) {
    if (menuItem) return menuItem->operator<<(stream);
    return stream;
}

UbuntuPlatformMenuBar::UbuntuPlatformMenuBar()
    : m_exporter(new UbuntuMenuBarExporter(this))
    , m_registrar(new UbuntuMenuRegistrar())
    , m_ready(false)
{
    BAR_DEBUG_MSG << "()";

    connect(this, &UbuntuPlatformMenuBar::menuInserted, this, &UbuntuPlatformMenuBar::structureChanged);
    connect(this,&UbuntuPlatformMenuBar::menuRemoved, this, &UbuntuPlatformMenuBar::structureChanged);
}

UbuntuPlatformMenuBar::~UbuntuPlatformMenuBar()
{
    BAR_DEBUG_MSG << "()";
}

void UbuntuPlatformMenuBar::insertMenu(QPlatformMenu *menu, QPlatformMenu *before)
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
    Q_EMIT menuInserted(menu);
}

void UbuntuPlatformMenuBar::removeMenu(QPlatformMenu *menu)
{
    BAR_DEBUG_MSG << "(menu=" << menu << ")";

    QMutableListIterator<QPlatformMenu*> iterator(m_menus);
    while(iterator.hasNext()) {
        if (iterator.next() == menu) {
            iterator.remove();
            break;
        }
    }
    Q_EMIT menuRemoved(menu);
}

void UbuntuPlatformMenuBar::syncMenu(QPlatformMenu *menu)
{
    BAR_DEBUG_MSG << "(menu=" << menu << ")";

    Q_UNUSED(menu)
}

void UbuntuPlatformMenuBar::handleReparent(QWindow *parentWindow)
{
    BAR_DEBUG_MSG << "(parentWindow=" << parentWindow << ")";

    setReady(true);
    m_registrar->registerMenuForWindow(parentWindow, QDBusObjectPath(m_exporter->menuPath()));
}

QPlatformMenu *UbuntuPlatformMenuBar::menuForTag(quintptr tag) const
{
    Q_FOREACH(QPlatformMenu* menu, m_menus) {
        if (menu->tag() == tag) {
            return menu;
        }
    }
    return nullptr;
}

const QList<QPlatformMenu *> UbuntuPlatformMenuBar::menus() const
{
    return m_menus;
}

QDebug UbuntuPlatformMenuBar::operator<<(QDebug stream)
{
    stream.nospace().noquote() << QString("%1").arg("", logRecusion, QLatin1Char('\t'))
            << "UbuntuPlatformMenuBar(this=" << (void*)this << ")" << endl;
    Q_FOREACH(QPlatformMenu* menu, m_menus) {
        auto myMenu = static_cast<UbuntuPlatformMenu*>(menu);
        if (myMenu) {
            logRecusion++;
            stream << myMenu;
            logRecusion--;
        }
    }

    return stream;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
QPlatformMenu *UbuntuPlatformMenuBar::createMenu() const
{
    return new UbuntuPlatformMenu();
}
#endif

void UbuntuPlatformMenuBar::setReady(bool isReady)
{
    if (m_ready != isReady) {
        m_ready = isReady;
        Q_EMIT ready();
    }
}

//////////////////////////////////////////////////////////////

UbuntuPlatformMenu::UbuntuPlatformMenu()
    : m_tag(reinterpret_cast<quintptr>(this))
    , m_parentWindow(nullptr)
    , m_exporter(nullptr)
    , m_registrar(nullptr)
{
    MENU_DEBUG_MSG << "()";

    connect(this, &UbuntuPlatformMenu::menuItemInserted, this, &UbuntuPlatformMenu::structureChanged);
    connect(this, &UbuntuPlatformMenu::menuItemRemoved, this, &UbuntuPlatformMenu::structureChanged);
}

UbuntuPlatformMenu::~UbuntuPlatformMenu()
{
    MENU_DEBUG_MSG << "()";
}

void UbuntuPlatformMenu::insertMenuItem(QPlatformMenuItem *menuItem, QPlatformMenuItem *before)
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

void UbuntuPlatformMenu::removeMenuItem(QPlatformMenuItem *menuItem)
{
    MENU_DEBUG_MSG << "(menuItem=" << menuItem << ")";

    QMutableListIterator<QPlatformMenuItem*> iterator(m_menuItems);
    while(iterator.hasNext()) {
        if (iterator.next() == menuItem) {
            iterator.remove();
            break;
        }
    }
    Q_EMIT menuItemRemoved(menuItem);
}

void UbuntuPlatformMenu::syncMenuItem(QPlatformMenuItem *menuItem)
{
    MENU_DEBUG_MSG << "(menuItem=" << menuItem << ")";

    Q_UNUSED(menuItem)
}

void UbuntuPlatformMenu::syncSeparatorsCollapsible(bool enable)
{
    MENU_DEBUG_MSG << "(enable=" << enable << ")";
    Q_UNUSED(enable)
}

void UbuntuPlatformMenu::setTag(quintptr tag)
{
    MENU_DEBUG_MSG << "(tag=" << tag << ")";
    m_tag = tag;
}

quintptr UbuntuPlatformMenu::tag() const
{
    return m_tag;
}

void UbuntuPlatformMenu::setText(const QString &text)
{
    MENU_DEBUG_MSG << "(text=" << text << ")";
    if (m_text != text) {
        m_text = text;
        Q_EMIT propertyChanged();
    }
}

void UbuntuPlatformMenu::setIcon(const QIcon &icon)
{
    MENU_DEBUG_MSG << "(icon=" << icon.name() << ")";

    if (!icon.isNull() || (!m_icon.isNull() && icon.isNull())) {
        m_icon = icon;
        Q_EMIT propertyChanged();
    }
}

void UbuntuPlatformMenu::setEnabled(bool enabled)
{
    MENU_DEBUG_MSG << "(enabled=" << enabled << ")";

    if (m_enabled != enabled) {
        m_enabled = enabled;
        Q_EMIT enabledChanged(enabled);
        Q_EMIT propertyChanged();
    }
}

void UbuntuPlatformMenu::setVisible(bool isVisible)
{
    MENU_DEBUG_MSG << "(visible=" << isVisible << ")";

    if (m_visible != isVisible) {
        m_visible = isVisible;
        Q_EMIT propertyChanged();
    }
}

void UbuntuPlatformMenu::setMinimumWidth(int width)
{
    MENU_DEBUG_MSG << "(width=" << width << ")";

    Q_UNUSED(width)
}

void UbuntuPlatformMenu::setFont(const QFont &font)
{
    MENU_DEBUG_MSG << "(font=" << font << ")";

    Q_UNUSED(font)
}

void UbuntuPlatformMenu::showPopup(const QWindow *parentWindow, const QRect &targetRect, const QPlatformMenuItem *item)
{
    MENU_DEBUG_MSG << "(parentWindow=" << parentWindow << ", targetRect=" << targetRect << ", item=" << item << ")";

    if (!m_exporter) {
        m_exporter.reset(new UbuntuMenuExporter(this));
        m_exporter->exportModels();
    }

    if (parentWindow != m_parentWindow) {
        if (m_parentWindow) {
            m_registrar->unregisterMenu();
        }

        m_parentWindow = parentWindow;

        if (m_parentWindow) {
            if (!m_registrar) m_registrar.reset(new UbuntuMenuRegistrar);
            m_registrar->registerMenuForWindow(const_cast<QWindow*>(m_parentWindow),
                                                      QDBusObjectPath(m_exporter->menuPath()));
        }
    }

    Q_UNUSED(targetRect);
    Q_UNUSED(item);
    setVisible(true);
}

void UbuntuPlatformMenu::dismiss()
{
    MENU_DEBUG_MSG << "()";

    if (m_registrar) { m_registrar->unregisterMenu(); }
    if (m_exporter) { m_exporter->unexportModels(); }
}

QPlatformMenuItem *UbuntuPlatformMenu::menuItemAt(int position) const
{
    if (position < 0 || position >= m_menuItems.count()) return nullptr;
    return m_menuItems.at(position);
}

QPlatformMenuItem *UbuntuPlatformMenu::menuItemForTag(quintptr tag) const
{
    Q_FOREACH(QPlatformMenuItem* menuItem, m_menuItems) {
        if (menuItem->tag() == tag) {
            return menuItem;
        }
    }
    return nullptr;
}

QPlatformMenuItem *UbuntuPlatformMenu::createMenuItem() const
{
    return new UbuntuPlatformMenuItem();
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
QPlatformMenu *UbuntuPlatformMenu::createSubMenu() const
{
    return new UbuntuPlatformMenu();
}
#endif

const QList<QPlatformMenuItem *> UbuntuPlatformMenu::menuItems() const
{
    return m_menuItems;
}

QDebug UbuntuPlatformMenu::operator<<(QDebug stream)
{
    stream.nospace().noquote() << QString("%1").arg("", logRecusion, QLatin1Char('\t'))
            << "UbuntuPlatformMenu(this=" << (void*)this << ", text=\"" << m_text << "\")" << endl;
    Q_FOREACH(QPlatformMenuItem* item, m_menuItems) {
        logRecusion++;
        auto myItem = static_cast<UbuntuPlatformMenuItem*>(item);
        if (myItem) {
            stream << myItem;
        }
        logRecusion--;
    }
    return stream;
}

//////////////////////////////////////////////////////////////

UbuntuPlatformMenuItem::UbuntuPlatformMenuItem()
    : m_menu(nullptr)
    , m_tag(reinterpret_cast<quintptr>(this))
{
    ITEM_DEBUG_MSG << "()";
}

UbuntuPlatformMenuItem::~UbuntuPlatformMenuItem()
{
    ITEM_DEBUG_MSG << "()";
}

void UbuntuPlatformMenuItem::setTag(quintptr tag)
{
    ITEM_DEBUG_MSG << "(tag=" << tag << ")";
    m_tag = tag;
}

quintptr UbuntuPlatformMenuItem::tag() const
{
    return m_tag;
}

void UbuntuPlatformMenuItem::setText(const QString &text)
{
    ITEM_DEBUG_MSG << "(text=" << text << ")";
    if (m_text != text) {
        m_text = text;
        Q_EMIT propertyChanged();
    }
}

void UbuntuPlatformMenuItem::setIcon(const QIcon &icon)
{
    ITEM_DEBUG_MSG << "(icon=" << icon.name() << ")";

    if (!icon.isNull() || (!m_icon.isNull() && icon.isNull())) {
        m_icon = icon;
        Q_EMIT propertyChanged();
    }
}

void UbuntuPlatformMenuItem::setVisible(bool isVisible)
{
    ITEM_DEBUG_MSG << "(visible=" << isVisible << ")";
    if (m_visible != isVisible) {
        m_visible = isVisible;
        Q_EMIT propertyChanged();
    }
}

void UbuntuPlatformMenuItem::setIsSeparator(bool isSeparator)
{
    ITEM_DEBUG_MSG << "(separator=" << isSeparator << ")";
    if (m_separator != isSeparator) {
        m_separator = isSeparator;
        Q_EMIT propertyChanged();
    }
}

void UbuntuPlatformMenuItem::setFont(const QFont &font)
{
    ITEM_DEBUG_MSG << "(font=" << font << ")";
    Q_UNUSED(font);
}

void UbuntuPlatformMenuItem::setRole(QPlatformMenuItem::MenuRole role)
{
    ITEM_DEBUG_MSG << "(role=" << role << ")";
    Q_UNUSED(role);
}

void UbuntuPlatformMenuItem::setCheckable(bool checkable)
{
    ITEM_DEBUG_MSG << "(checkable=" << checkable << ")";
    if (m_checkable != checkable) {
        m_checkable = checkable;
        Q_EMIT propertyChanged();
    }
}

void UbuntuPlatformMenuItem::setChecked(bool isChecked)
{
    ITEM_DEBUG_MSG << "(checked=" << isChecked << ")";
    if (m_checked != isChecked) {
        m_checked = isChecked;
        Q_EMIT checkedChanged(isChecked);
        Q_EMIT propertyChanged();
    }
}

void UbuntuPlatformMenuItem::setShortcut(const QKeySequence &shortcut)
{
    ITEM_DEBUG_MSG << "(shortcut=" << shortcut << ")";
    if (m_shortcut != shortcut) {
        m_shortcut = shortcut;
        Q_EMIT propertyChanged();
    }
}

void UbuntuPlatformMenuItem::setEnabled(bool enabled)
{
    ITEM_DEBUG_MSG << "(enabled=" << enabled << ")";
    if (m_enabled != enabled) {
        m_enabled = enabled;
        Q_EMIT enabledChanged(enabled);
        Q_EMIT propertyChanged();
    }
}

void UbuntuPlatformMenuItem::setIconSize(int size)
{
    ITEM_DEBUG_MSG << "(size=" << size << ")";
    Q_UNUSED(size);
}

void UbuntuPlatformMenuItem::setMenu(QPlatformMenu *menu)
{
    ITEM_DEBUG_MSG << "(menu=" << menu << ")";
    if (m_menu != menu) {
        m_menu = menu;
        Q_EMIT propertyChanged();

        if (menu) {
            connect(menu, &QObject::destroyed,
                    this, [this] { setMenu(nullptr); });
        }
    }
}

QPlatformMenu *UbuntuPlatformMenuItem::menu() const
{
    return m_menu;
}

QDebug UbuntuPlatformMenuItem::operator<<(QDebug stream)
{
    QString properties = "text=\"" + m_text + "\"";

    stream.nospace().noquote() << QString("%1").arg("", logRecusion, QLatin1Char('\t'))
            << "UbuntuPlatformMenuItem(this=" << (void*)this << ", "
            << (m_separator ? "Separator" : properties) << ")" << endl;
    if (m_menu) {
        auto myMenu = static_cast<UbuntuPlatformMenu*>(m_menu);
        if (myMenu) {
            logRecusion++;
            stream << myMenu;
            logRecusion--;
        }
    }
    return stream;
}
