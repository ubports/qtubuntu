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

#ifndef UBUNTU_CLIENT_INTEGRATION_H
#define UBUNTU_CLIENT_INTEGRATION_H

#include <qpa/qplatformintegration.h>
#include <QSharedPointer>
#include <QtPlatformSupport/private/qeglconvenience_p.h>

#include "platformservices.h"
#include "screenobserver.h"

// platform-api
#include <ubuntu/application/description.h>
#include <ubuntu/application/instance.h>

class UbuntuClipboard;
class UbuntuInput;
class UbuntuNativeInterface;
class UbuntuScreen;

class UbuntuClientIntegration : public QObject, public QPlatformIntegration
{
    Q_OBJECT

public:
    UbuntuClientIntegration();
    virtual ~UbuntuClientIntegration();

    // QPlatformIntegration methods.
    bool hasCapability(QPlatformIntegration::Capability cap) const override;
    QAbstractEventDispatcher *createEventDispatcher() const override;
    QPlatformNativeInterface* nativeInterface() const override;
    QPlatformBackingStore* createPlatformBackingStore(QWindow* window) const override;
    QPlatformOpenGLContext* createPlatformOpenGLContext(QOpenGLContext* context) const override;
    QPlatformFontDatabase* fontDatabase() const override { return mFontDb; }
    QStringList themeNames() const override;
    QPlatformTheme* createPlatformTheme(const QString& name) const override;
    QVariant styleHint(StyleHint hint) const override;
    QPlatformServices *services() const override;
    QPlatformWindow* createPlatformWindow(QWindow* window) const override;
    QPlatformInputContext* inputContext() const override { return mInputContext; }
    QPlatformClipboard* clipboard() const override;

    QPlatformOpenGLContext* createPlatformOpenGLContext(QOpenGLContext* context);
    QPlatformWindow* createPlatformWindow(QWindow* window);
    UbuntuScreenObserver *screenObserver() const { return mScreenObserver.data(); }

    void initialize() override;

    QPlatformOffscreenSurface *createPlatformOffscreenSurface(QOffscreenSurface *surface) const override;

private Q_SLOTS:
    void destroyScreen(UbuntuScreen *screen);

private:
    void setupOptions();
    void setupDescription();

    UbuntuNativeInterface* mNativeInterface;
    QPlatformFontDatabase* mFontDb;

    UbuntuPlatformServices* mServices;

    UbuntuInput* mInput;
    QPlatformInputContext* mInputContext;
    QSharedPointer<UbuntuClipboard> mClipboard;
    QScopedPointer<UbuntuScreenObserver> mScreenObserver;
    qreal mScaleFactor;

    // Platform API stuff
    UApplicationOptions* mOptions;
    UApplicationDescription* mDesc;
    UApplicationInstance* mInstance;

    EGLDisplay mEglDisplay{EGL_NO_DISPLAY};
};

#endif // UBUNTU_CLIENT_INTEGRATION_H
