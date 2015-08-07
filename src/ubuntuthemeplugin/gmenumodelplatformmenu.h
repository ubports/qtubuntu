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

#ifndef EXPORTEDPLATFORMMENUBAR_H
#define EXPORTEDPLATFORMMENUBAR_H

#include <qpa/qplatformmenu.h>
#include <QTimer>

// Local
class GMenuModelParser;

class GMenuModelPlatformMenuBar : public QPlatformMenuBar
{
    Q_OBJECT
public:
    GMenuModelPlatformMenuBar();
    ~GMenuModelPlatformMenuBar();

    QString exportedPath() const;

    virtual void insertMenu(QPlatformMenu *menu, QPlatformMenu* before) override;
    virtual void removeMenu(QPlatformMenu *menu) override;
    virtual void syncMenu(QPlatformMenu *menu) override;
    virtual void handleReparent(QWindow *newParentWindow) override;
    virtual QPlatformMenu *menuForTag(quintptr tag) const override;

    QPlatformMenu *menuAt(int position) const;
    uint menuCount();

Q_SIGNALS:
    void menuInserted(QPlatformMenu *menu);
    void menuRemoved(QPlatformMenu *menu);

    void structureChanged();

private:
    QList<QPlatformMenu*> m_menus;
    QTimer m_structureTimer;
    GMenuModelParser* m_parser;

    friend class GMenuModelParser;
};

#define MENU_PROPERTY(class, name, type, defaultValue) \
    type m_##name = defaultValue; \
    static type get_##name(const class *menuItem) { return menuItem->m_##name; } \
    static void set_##name(class *menuItem, const type& value) { menuItem->m_##name = value; }


class GMenuModelPlatformMenu : public QPlatformMenu
{
    Q_OBJECT
public:
    GMenuModelPlatformMenu();
    ~GMenuModelPlatformMenu();

    virtual void insertMenuItem(QPlatformMenuItem *menuItem, QPlatformMenuItem *before) override;
    virtual void removeMenuItem(QPlatformMenuItem *menuItem) override;
    virtual void syncMenuItem(QPlatformMenuItem *menuItem) override;
    virtual void syncSeparatorsCollapsible(bool enable) override;

    virtual void setTag(quintptr tag) override;
    virtual quintptr tag() const override;

    virtual void setText(const QString &text) override;
    virtual void setIcon(const QIcon &icon) override;
    virtual void setEnabled(bool isEnabled) override;
    virtual void setVisible(bool isVisible) override;
    virtual void setMinimumWidth(int width) override;
    virtual void setFont(const QFont &font) override;

    virtual QPlatformMenuItem *menuItemAt(int position) const override;
    virtual QPlatformMenuItem *menuItemForTag(quintptr tag) const override;
    uint menuItemCount();

    virtual QPlatformMenuItem *createMenuItem() const override;

    int id() const;

Q_SIGNALS:
    void menuItemInserted(QPlatformMenuItem *menuItem);
    void menuItemRemoved(QPlatformMenuItem *menuItem);

    void propertyUpdated();

private:
    MENU_PROPERTY(GMenuModelPlatformMenu, visible, bool, true)
    MENU_PROPERTY(GMenuModelPlatformMenu, text, QString, QString())
    MENU_PROPERTY(GMenuModelPlatformMenu, enabled, bool, true)
    MENU_PROPERTY(GMenuModelPlatformMenu, icon, QIcon, QIcon())

    quintptr m_tag;
    QList<QPlatformMenuItem*> m_menuItems;
    friend class GMenuModelParser;
};


class GMenuModelPlatformMenuItem : public QPlatformMenuItem
{
    Q_OBJECT
public:
    GMenuModelPlatformMenuItem();
    ~GMenuModelPlatformMenuItem();

    virtual void setTag(quintptr tag) override;
    virtual quintptr tag() const override;

    virtual void setText(const QString &text) override;
    virtual void setIcon(const QIcon &icon) override;
    virtual void setMenu(QPlatformMenu *menu) override;
    virtual void setVisible(bool isVisible) override;
    virtual void setIsSeparator(bool isSeparator) override;
    virtual void setFont(const QFont &font) override;
    virtual void setRole(MenuRole role) override;
    virtual void setCheckable(bool checkable) override;
    virtual void setChecked(bool isChecked) override;
    virtual void setShortcut(const QKeySequence& shortcut) override;
    virtual void setEnabled(bool enabled) override;
    virtual void setIconSize(int size) override;

    QPlatformMenu* menu() const;

Q_SIGNALS:
    void propertyUpdated();

private:
    MENU_PROPERTY(GMenuModelPlatformMenuItem, separator, bool, false)
    MENU_PROPERTY(GMenuModelPlatformMenuItem, visible, bool, true)
    MENU_PROPERTY(GMenuModelPlatformMenuItem, text, QString, QString())
    MENU_PROPERTY(GMenuModelPlatformMenuItem, enabled, bool, true)
    MENU_PROPERTY(GMenuModelPlatformMenuItem, checkable, bool, false)
    MENU_PROPERTY(GMenuModelPlatformMenuItem, checked, bool, false)
    MENU_PROPERTY(GMenuModelPlatformMenuItem, shortcut, QKeySequence, QKeySequence())
    MENU_PROPERTY(GMenuModelPlatformMenuItem, icon, QIcon, QIcon())
    MENU_PROPERTY(GMenuModelPlatformMenuItem, iconSize, int, 16)

    quintptr m_tag;
    QPlatformMenu* m_menu;
    friend class GMenuModelParser;
};
#endif // EXPORTEDPLATFORMMENUBAR_H
