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

#include "appstatecontroller.h"

#include <qpa/qwindowsysteminterface.h>

/*
 * UbuntuAppStateController - updates Qt's QApplication::applicationState property.
 *
 * Tries to avoid active-inactive-active invocations using a timer. The rapid state
 * change can confuse some applications.
 */

UbuntuAppStateController::UbuntuAppStateController()
    : m_suspended(false)
    , m_lastActive(true)
{
    m_inactiveTimer.setSingleShot(true);
    m_inactiveTimer.setInterval(10);
    QObject::connect(&m_inactiveTimer, &QTimer::timeout, []()
    {
        QWindowSystemInterface::handleApplicationStateChanged(Qt::ApplicationInactive);
    });
}

void UbuntuAppStateController::setSuspended()
{
    m_inactiveTimer.stop();
    if (!m_suspended) {
        m_suspended = true;

        QWindowSystemInterface::handleApplicationStateChanged(Qt::ApplicationSuspended);
    }
}

void UbuntuAppStateController::setResumed()
{
    m_inactiveTimer.stop();
    if (m_suspended) {
        m_suspended = false;

        if (m_lastActive) {
            QWindowSystemInterface::handleApplicationStateChanged(Qt::ApplicationActive);
        } else {
            QWindowSystemInterface::handleApplicationStateChanged(Qt::ApplicationInactive);
        }
    }
}

void UbuntuAppStateController::setWindowFocused(bool focused)
{
    if (m_suspended) {
        return;
    }

    if (focused) {
        m_inactiveTimer.stop();
        QWindowSystemInterface::handleApplicationStateChanged(Qt::ApplicationActive);
    } else {
        m_inactiveTimer.start();
    }

    m_lastActive = focused;
}
