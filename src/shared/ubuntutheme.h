/*
 * Copyright (C) 2016-2017 Canonical, Ltd.
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

#include <QVariant>
#include <QtThemeSupport/private/qgenericunixthemes_p.h>
#include <QGuiApplication>
#include <QScreen>

class UbuntuTheme : public QGenericUnixTheme
{
public:
    UbuntuTheme()
      : mSystemFont(QStringLiteral("Ubuntu Regular"), 10),
        mFixedFont(QStringLiteral("Ubuntu Mono Regular"), 13)
    {
        mSystemFont.setStyleHint(QFont::System);
        mFixedFont.setStyleHint(QFont::TypeWriter);
    }
    ~UbuntuTheme() = default;

    QVariant themeHint(ThemeHint hint) const override
    {
        switch (hint) {
        case QPlatformTheme::SystemIconThemeName: {
            QByteArray iconTheme = qgetenv("QTUBUNTU_ICON_THEME");
            if (iconTheme.isEmpty()) {
                return QStringLiteral("suru");
            } else {
                return iconTheme;
            }
        }
        case QPlatformTheme::MouseDoubleClickDistance: {
            // Impl mostly yoinked from the QAndroidPlatformTheme
            QScreen *screen = qGuiApp->primaryScreen();
            if (screen) {
                qreal dotsPerInch = screen->physicalDotsPerInch();
                // Allow 15% of an inch between taps when double clicking
                return qRound(dotsPerInch * 0.15);
            } else {
                return 5;
            }
        }
        case QPlatformTheme::TouchDoubleTapDistance: {
            bool ok = false;
            int dist = themeHint(QPlatformTheme::MouseDoubleClickDistance).toInt(&ok) * 2;
            return QVariant(ok ? dist : 10);
        }
        default:
            break;
        }
        return QGenericUnixTheme::themeHint(hint);
    }

    const QFont *font(Font type) const override
    {
        switch (type) {
        case QPlatformTheme::SystemFont:
            return &mSystemFont;
        case QPlatformTheme::FixedFont:
            return &mFixedFont;
        default:
            return nullptr;
        }
    }

private:
    QFont mSystemFont, mFixedFont;
};
