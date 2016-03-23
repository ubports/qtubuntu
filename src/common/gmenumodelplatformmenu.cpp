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
#include "gmenumodelexporter.h"
#include "registry.h"
#include "menuregistrar.h"
#include "../ubuntumirclient/logging.h"

// Qt
#include <QDebug>
#include <QWindow>
#include <QCoreApplication>

GMenuModelPlatformMenuBar::GMenuModelPlatformMenuBar()
    : m_exporter(new GMenuModelExporter(this))
    , m_registrar(new MenuRegistrar())
    , m_ready(false)
{
    connect(this, SIGNAL(menuInserted(QPlatformMenu*)), this, SIGNAL(structureChanged()));
    connect(this, SIGNAL(menuRemoved(QPlatformMenu*)), this, SIGNAL(structureChanged()));
}

GMenuModelPlatformMenuBar::~GMenuModelPlatformMenuBar()
{
    delete m_registrar;
    m_registrar = nullptr;

    delete m_exporter;
    m_exporter = nullptr;
}

void
GMenuModelPlatformMenuBar::insertMenu(QPlatformMenu *menu, QPlatformMenu *before)
{
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
    Q_UNUSED(menu)
}

void
GMenuModelPlatformMenuBar::handleReparent(QWindow *newParentWindow)
{
    setReady(true);
    m_registrar->registerMenuForWindow(newParentWindow, QDBusObjectPath(m_exporter->menuPath()));
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
    connect(this, SIGNAL(menuItemInserted(QPlatformMenuItem*)), this, SIGNAL(structureChanged()));
    connect(this, SIGNAL(menuItemRemoved(QPlatformMenuItem*)), this, SIGNAL(structureChanged()));
}

GMenuModelPlatformMenu::~GMenuModelPlatformMenu()
{
}

void GMenuModelPlatformMenu::insertMenuItem(QPlatformMenuItem *menuItem, QPlatformMenuItem *before)
{
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
    Q_UNUSED(menuItem)
}

void GMenuModelPlatformMenu::syncSeparatorsCollapsible(bool enable)
{
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

void GMenuModelPlatformMenu::showPopup(const QWindow *parentWindow, const QRect &targetRect, const QPlatformMenuItem *item)
{
    if (!m_exporter) {
        m_exporter = new GMenuModelExporter(this);
        m_exporter->exportModels();
    }

    if (parentWindow != m_parentWindow) {
        if (m_parentWindow) {
            m_registrar->unregisterMenu();
        }

        m_parentWindow = parentWindow;

        if (m_parentWindow) {
            if (!m_registrar) m_registrar = new MenuRegistrar;
            m_registrar->registerMenuForWindow(const_cast<QWindow*>(m_parentWindow),
                                               QDBusObjectPath(m_exporter->menuPath()));
        }
    }

    Q_UNUSED(targetRect);
    Q_UNUSED(item);
    setVisible(true);
    qDebug() << "SHOW!" << this << targetRect;
}

void GMenuModelPlatformMenu::dismiss()
{
    qDebug() << "DISMISS";
    if (m_registrar) { m_registrar->unregisterMenu(); }
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

QPlatformMenuItem *GMenuModelPlatformMenu::createMenuItem() const
{
    return new GMenuModelPlatformMenuItem();
}

const QList<QPlatformMenuItem *> GMenuModelPlatformMenu::menuItems() const
{
    return m_menuItems;
}

//////////////////////////////////////////////////////////////

GMenuModelPlatformMenuItem::GMenuModelPlatformMenuItem()
    : m_menu(nullptr)
{
}

GMenuModelPlatformMenuItem::~GMenuModelPlatformMenuItem()
{
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
    m_menu = menu;
    Q_EMIT propertyUpdated();
}

QPlatformMenu *GMenuModelPlatformMenuItem::menu() const
{
    return m_menu;
}
