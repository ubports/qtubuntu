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

#ifndef UBUNTU_THEME_H
#define UBUNTU_THEME_H

#include <QtPlatformSupport/private/qgenericunixthemes_p.h>

class UbuntuAppMenuTheme : public QGenericUnixTheme
{
public:
    static const char* name;
    UbuntuAppMenuTheme();
    ~UbuntuAppMenuTheme() = default;

    // From QPlatformTheme
    QVariant themeHint(ThemeHint hint) const override;
    const QFont *font(Font type) const override;

    // For the menus
    QPlatformMenuItem* createPlatformMenuItem() const override;
    QPlatformMenu* createPlatformMenu() const override;
    QPlatformMenuBar* createPlatformMenuBar() const override;
    QPlatformSystemTrayIcon *createPlatformSystemTrayIcon() const override;

private:
    QFont systemFont, fixedFont;
};

#endif // UBUNTU_THEME_H
