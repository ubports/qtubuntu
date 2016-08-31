/*
 * Copyright (C) 2014,2016 Canonical, Ltd.
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

#include "clipboard.h"
#include "logging.h"
#include "window.h"

#include <QDBusPendingCallWatcher>
#include <QGuiApplication>
#include <QSignalBlocker>
#include <QtCore/QMimeData>
#include <QtCore/QStringList>

// content-hub
#include <com/ubuntu/content/hub.h>

// get this cumbersome nested namespace out of the way
using namespace com::ubuntu::content;

UbuntuClipboard::UbuntuClipboard()
    : mMimeData(new QMimeData)
    , mContentHub(Hub::Client::instance())
{
    connect(mContentHub, &Hub::pasteboardChanged, this, [this]() {
        if (mClipboardState == UbuntuClipboard::SyncedClipboard) {
            mClipboardState = UbuntuClipboard::OutdatedClipboard;
            emitChanged(QClipboard::Clipboard);
        }
    });

    connect(qGuiApp, &QGuiApplication::applicationStateChanged,
        this, &UbuntuClipboard::onApplicationStateChanged);

    requestMimeData();
}

UbuntuClipboard::~UbuntuClipboard()
{
    delete mMimeData;
}

QMimeData* UbuntuClipboard::mimeData(QClipboard::Mode mode)
{
    if (mode != QClipboard::Clipboard)
        return nullptr;

    // Blocks dataChanged() signal from being emitted. Makes no sense to emit it from
    // inside the data getter.
    const QSignalBlocker blocker(this);

    if (mClipboardState == OutdatedClipboard) {
        updateMimeData();
    } else if (mClipboardState == SyncingClipboard) {
        mPasteReply->waitForFinished();
    }

    return mMimeData;
}

void UbuntuClipboard::setMimeData(QMimeData* mimeData, QClipboard::Mode mode)
{
    QWindow *focusWindow = QGuiApplication::focusWindow();
    if (focusWindow && mode == QClipboard::Clipboard && mimeData != nullptr) {
        QString surfaceId = static_cast<UbuntuWindow*>(focusWindow->handle())->persistentSurfaceId();

        QDBusPendingCall reply = mContentHub->createPaste(surfaceId, *mimeData);

        // Don't care whether it succeeded
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
        connect(watcher, &QDBusPendingCallWatcher::finished,
                watcher, &QObject::deleteLater);

        mMimeData = mimeData;
        mClipboardState = SyncedClipboard;
        emitChanged(QClipboard::Clipboard);
    }
}

bool UbuntuClipboard::supportsMode(QClipboard::Mode mode) const
{
    return mode == QClipboard::Clipboard;
}

bool UbuntuClipboard::ownsMode(QClipboard::Mode mode) const
{
    Q_UNUSED(mode);
    return false;
}

void UbuntuClipboard::onApplicationStateChanged(Qt::ApplicationState state)
{
    if (state == Qt::ApplicationActive) {
        // Only focused or active applications might be allowed to paste, so we probably
        // missed changes in the clipboard while we were hidden, inactive or, more importantly,
        // suspended.
        requestMimeData();
    }
}

void UbuntuClipboard::updateMimeData()
{
    if (qGuiApp->applicationState() != Qt::ApplicationActive) {
        // Don't even bother asking as content-hub would probably ignore our request (and should).
        return;
    }

    delete mMimeData;

    QWindow *focusWindow = QGuiApplication::focusWindow();
    if (focusWindow) {
        QString surfaceId = static_cast<UbuntuWindow*>(focusWindow->handle())->persistentSurfaceId();
        mMimeData = mContentHub->latestPaste(surfaceId);
        mClipboardState = SyncedClipboard;
        emitChanged(QClipboard::Clipboard);
    }
}

void UbuntuClipboard::requestMimeData()
{
    if (qGuiApp->applicationState() != Qt::ApplicationActive) {
        // Don't even bother asking as content-hub would probably ignore our request (and should).
        return;
    }

    QWindow *focusWindow = QGuiApplication::focusWindow();
    if (!focusWindow) {
        return;
    }

    QString surfaceId = static_cast<UbuntuWindow*>(focusWindow->handle())->persistentSurfaceId();
    QDBusPendingCall reply = mContentHub->requestLatestPaste(surfaceId);
    mClipboardState = SyncingClipboard;

    mPasteReply = new QDBusPendingCallWatcher(reply, this);
    connect(mPasteReply, &QDBusPendingCallWatcher::finished,
            this, [this]() {
        delete mMimeData;
        mMimeData = mContentHub->paste(*mPasteReply);
        mClipboardState = SyncedClipboard;
        mPasteReply->deleteLater();
        mPasteReply = nullptr;
        emitChanged(QClipboard::Clipboard);
    });
}
