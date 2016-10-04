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

#ifndef UBUNTU_DESKTOP_WINDOW_H
#define UBUNTU_DESKTOP_WINDOW_H

#include <qpa/qplatformwindow.h>

// TODO Implement it. For now it's just an empty, dummy class.
class UbuntuDesktopWindow : public QPlatformWindow
{
public:
    UbuntuDesktopWindow(QWindow*);
};

#endif // UBUNTU_DESKTOP_WINDOW_H
