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

#ifndef EXPORTEDPLATFORMMENUBAR_H
#define EXPORTEDPLATFORMMENUBAR_H

#include <qpa/qplatformmenu.h>

// Local
class UbuntuGMenuModelExporter;
class UbuntuMenuRegistrar;
class QWindow;

class UbuntuPlatformMenuBar : public QPlatformMenuBar
{
    Q_OBJECT
public:
    UbuntuPlatformMenuBar();
    ~UbuntuPlatformMenuBar();

    QString exportedPath() const;

    virtual void insertMenu(QPlatformMenu *menu, QPlatformMenu* before) override;
    virtual void removeMenu(QPlatformMenu *menu) override;
    virtual void syncMenu(QPlatformMenu *menu) override;
    virtual void handleReparent(QWindow *newParentWindow) override;
    virtual QPlatformMenu *menuForTag(quintptr tag) const override;

    const QList<QPlatformMenu*> menus() const;

    QDebug operator<<(QDebug stream);

Q_SIGNALS:
    void menuInserted(QPlatformMenu *menu);
    void menuRemoved(QPlatformMenu *menu);

    void structureChanged();
    void ready();

private:
    void setReady(bool);

    QList<QPlatformMenu*> m_menus;
    QScopedPointer<UbuntuGMenuModelExporter> m_exporter;
    QScopedPointer<UbuntuMenuRegistrar> m_registrar;
    bool m_ready;
};

#define MENU_PROPERTY(class, name, type, defaultValue) \
    static type get_##name(const class *menuItem) { return menuItem->m_##name; } \
    type m_##name = defaultValue;

class Q_DECL_EXPORT UbuntuPlatformMenu : public QPlatformMenu
{
    Q_OBJECT
public:
    UbuntuPlatformMenu();
    ~UbuntuPlatformMenu();

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

    virtual void showPopup(const QWindow *parentWindow, const QRect &targetRect, const QPlatformMenuItem *item);

    virtual void dismiss(); // Closes this and all its related menu popups

    virtual QPlatformMenuItem *menuItemAt(int position) const override;
    virtual QPlatformMenuItem *menuItemForTag(quintptr tag) const override;

    int id() const;

    const QList<QPlatformMenuItem*> menuItems() const;

    QDebug operator<<(QDebug stream);

Q_SIGNALS:
    void menuItemInserted(QPlatformMenuItem *menuItem);
    void menuItemRemoved(QPlatformMenuItem *menuItem);
    void structureChanged();
    void propertyChanged();

private:
    MENU_PROPERTY(UbuntuPlatformMenu, visible, bool, true)
    MENU_PROPERTY(UbuntuPlatformMenu, text, QString, QString())
    MENU_PROPERTY(UbuntuPlatformMenu, enabled, bool, true)
    MENU_PROPERTY(UbuntuPlatformMenu, icon, QIcon, QIcon())

    quintptr m_tag;
    QList<QPlatformMenuItem*> m_menuItems;
    const QWindow* m_parentWindow;
    QScopedPointer<UbuntuGMenuModelExporter> m_exporter;
    QScopedPointer<UbuntuMenuRegistrar> m_registrar;

    friend class UbuntuGMenuModelExporter;
};


class Q_DECL_EXPORT UbuntuPlatformMenuItem : public QPlatformMenuItem
{
    Q_OBJECT
public:
    UbuntuPlatformMenuItem();
    ~UbuntuPlatformMenuItem();

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

    QDebug operator<<(QDebug stream);

Q_SIGNALS:
    void checkedChanged(bool);
    void enabledChanged(bool);
    void propertyChanged();

private:
    MENU_PROPERTY(UbuntuPlatformMenuItem, separator, bool, false)
    MENU_PROPERTY(UbuntuPlatformMenuItem, visible, bool, true)
    MENU_PROPERTY(UbuntuPlatformMenuItem, text, QString, QString())
    MENU_PROPERTY(UbuntuPlatformMenuItem, enabled, bool, true)
    MENU_PROPERTY(UbuntuPlatformMenuItem, checkable, bool, false)
    MENU_PROPERTY(UbuntuPlatformMenuItem, checked, bool, false)
    MENU_PROPERTY(UbuntuPlatformMenuItem, shortcut, QKeySequence, QKeySequence())
    MENU_PROPERTY(UbuntuPlatformMenuItem, icon, QIcon, QIcon())
    MENU_PROPERTY(UbuntuPlatformMenuItem, iconSize, int, 16)
    MENU_PROPERTY(UbuntuPlatformMenuItem, menu, QPlatformMenu*, nullptr)


    quintptr m_tag;
    friend class UbuntuGMenuModelExporter;
};
#endif // EXPORTEDPLATFORMMENUBAR_H
