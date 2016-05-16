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

#include "ubuntuthemeplugin.h"
#include "theme.h"

#include <QDebug>

///////////////////////////////////////////////////////////
const char *UbuntuThemePlugin::name = "ubuntu";

UbuntuThemePlugin::UbuntuThemePlugin(QObject *parent)
{
    Q_UNUSED(parent);
}

QPlatformTheme *
UbuntuThemePlugin::create(const QString &key, const QStringList &paramList)
{
    Q_UNUSED(paramList);
    if (key.compare(QLatin1String(UbuntuThemePlugin::name), Qt::CaseInsensitive))
        return 0;

    return new UbuntuTheme();
}
