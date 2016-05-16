/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#include "theme.h"
#include "gmenumodelplatformmenu.h"

#include <QtCore/QVariant>
#include <QDebug>

const char *UbuntuTheme::name = "ubuntu";

bool useLocalMenu() {
    QByteArray menuProxy = qgetenv("UBUNTU_MENUPROXY");
    bool menuProxyIsZero = !menuProxy.isEmpty() && menuProxy.at(0) == '0';
    return menuProxyIsZero;
}

UbuntuTheme::UbuntuTheme()
{
}

UbuntuTheme::~UbuntuTheme()
{
}

QVariant UbuntuTheme::themeHint(ThemeHint hint) const
{
    if (hint == QPlatformTheme::SystemIconThemeName) {
        QByteArray iconTheme = qgetenv("QTUBUNTU_ICON_THEME");
        if (iconTheme.isEmpty()) {
            return QVariant(QStringLiteral("ubuntu-mobile"));
        } else {
            return QVariant(QString(iconTheme));
        }
    } else {
        return QGenericUnixTheme::themeHint(hint);
    }
}

QPlatformMenuItem *UbuntuTheme::createPlatformMenuItem() const
{
    if (useLocalMenu()) return QGenericUnixTheme::createPlatformMenuItem();
    return new GMenuModelPlatformMenuItem();
}

QPlatformMenu *UbuntuTheme::createPlatformMenu() const
{
    if (useLocalMenu()) return QGenericUnixTheme::createPlatformMenu();
    return new GMenuModelPlatformMenu();
}

QPlatformMenuBar *
UbuntuTheme::createPlatformMenuBar() const
{
    if (useLocalMenu()) return QGenericUnixTheme::createPlatformMenuBar();
    return new GMenuModelPlatformMenuBar();
}
