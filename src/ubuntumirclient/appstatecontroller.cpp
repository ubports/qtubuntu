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
#include <qpa/qwindowsysteminterface_p.h>

UbuntuAppStateController::UbuntuAppStateController()
    : m_suspended(false)
    , m_lastActive(true)
{}

void UbuntuAppStateController::setSuspended()
{
    if (!m_suspended) {
        m_suspended = true;

        QWindowSystemInterface::handleApplicationStateChanged(Qt::ApplicationSuspended);
    }
}

void UbuntuAppStateController::setResumed()
{
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
        // Check if there are ApplicationInactive events in the queue, and remove them if so, to
        // avoid active-inactive-active invocations, which confuse some applications
        QWindowSystemInterfacePrivate::ApplicationStateChangedEvent *queuedApplicationStateEvent = nullptr;
        do {
            queuedApplicationStateEvent =
                static_cast<QWindowSystemInterfacePrivate::ApplicationStateChangedEvent *>
                (QWindowSystemInterfacePrivate::peekWindowSystemEvent(QWindowSystemInterfacePrivate::ApplicationStateChanged));

            if (queuedApplicationStateEvent) {
                QWindowSystemInterfacePrivate::removeWindowSystemEvent(queuedApplicationStateEvent);
            }

        } while (queuedApplicationStateEvent);

        QWindowSystemInterface::handleApplicationStateChanged(Qt::ApplicationActive);
    } else {
        QWindowSystemInterface::handleApplicationStateChanged(Qt::ApplicationInactive);
    }

    m_lastActive = focused;
}
