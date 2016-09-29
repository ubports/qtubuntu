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

#include "themeplugin.h"
#include "theme.h"

#include <QDebug>

///////////////////////////////////////////////////////////

UbuntuAppMenuThemePlugin::UbuntuAppMenuThemePlugin(QObject *parent)
    : QPlatformThemePlugin(parent)
{
}

QPlatformTheme *
UbuntuAppMenuThemePlugin::create(const QString &key, const QStringList&)
{
    if (key.compare(QLatin1String(UbuntuAppMenuTheme::name), Qt::CaseInsensitive))
        return 0;

    return new UbuntuAppMenuTheme();
}
