/****************************************************************************
**
** Copyright (C) 2015-2017 Canonical, Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "qmirclientcursor.h"

#include "qmirclientlogging.h"
#include "qmirclientwindow.h"

#include <mir_toolkit/mir_client_library.h>

Q_LOGGING_CATEGORY(mirclientCursor, "qt.qpa.mirclient.cursor", QtWarningMsg)

QMirClientCursor::QMirClientCursor(MirConnection *connection)
    : mConnection(connection)
{
    /*
     * TODO: Add the missing cursors to Mir (LP: #1388987)
     *       Those are the ones without a mir_ prefix, which are X11 cursors
     *       and won't be understood by any shell other than Unity8.
     */
    mShapeToCursorName[Qt::ArrowCursor] = mir_arrow_cursor_name;
    mShapeToCursorName[Qt::UpArrowCursor] = "up_arrow";
    mShapeToCursorName[Qt::CrossCursor] = mir_crosshair_cursor_name;
    mShapeToCursorName[Qt::WaitCursor] = mir_busy_cursor_name;
    mShapeToCursorName[Qt::IBeamCursor] = mir_caret_cursor_name;
    mShapeToCursorName[Qt::SizeVerCursor] = mir_vertical_resize_cursor_name;
    mShapeToCursorName[Qt::SizeHorCursor] = mir_horizontal_resize_cursor_name;
    mShapeToCursorName[Qt::SizeBDiagCursor] = mir_diagonal_resize_bottom_to_top_cursor_name;
    mShapeToCursorName[Qt::SizeFDiagCursor] = mir_diagonal_resize_top_to_bottom_cursor_name;
    mShapeToCursorName[Qt::SizeAllCursor] = mir_omnidirectional_resize_cursor_name;
    mShapeToCursorName[Qt::BlankCursor] = mir_disabled_cursor_name;
    mShapeToCursorName[Qt::SplitVCursor] = mir_vsplit_resize_cursor_name;
    mShapeToCursorName[Qt::SplitHCursor] = mir_hsplit_resize_cursor_name;
    mShapeToCursorName[Qt::PointingHandCursor] = mir_pointing_hand_cursor_name;
    mShapeToCursorName[Qt::ForbiddenCursor] = "forbidden";
    mShapeToCursorName[Qt::WhatsThisCursor] = "whats_this";
    mShapeToCursorName[Qt::BusyCursor] = "left_ptr_watch";
    mShapeToCursorName[Qt::OpenHandCursor] = mir_open_hand_cursor_name;
    mShapeToCursorName[Qt::ClosedHandCursor] = mir_closed_hand_cursor_name;
    mShapeToCursorName[Qt::DragCopyCursor] = "dnd-copy";
    mShapeToCursorName[Qt::DragMoveCursor] = "dnd-move";
    mShapeToCursorName[Qt::DragLinkCursor] = "dnd-link";
}

namespace {
const char *qtCursorShapeToStr(Qt::CursorShape shape)
{
    switch (shape) {
    case Qt::ArrowCursor:
        return "Arrow";
    case Qt::UpArrowCursor:
        return "UpArrow";
    case Qt::CrossCursor:
        return "Cross";
    case Qt::WaitCursor:
        return "Wait";
    case Qt::IBeamCursor:
        return "IBeam";
    case Qt::SizeVerCursor:
        return "SizeVer";
    case Qt::SizeHorCursor:
        return "SizeHor";
    case Qt::SizeBDiagCursor:
        return "SizeBDiag";
    case Qt::SizeFDiagCursor:
        return "SizeFDiag";
    case Qt::SizeAllCursor:
        return "SizeAll";
    case Qt::BlankCursor:
        return "Blank";
    case Qt::SplitVCursor:
        return "SplitV";
    case Qt::SplitHCursor:
        return "SplitH";
    case Qt::PointingHandCursor:
        return "PointingHand";
    case Qt::ForbiddenCursor:
        return "Forbidden";
    case Qt::WhatsThisCursor:
        return "WhatsThis";
    case Qt::BusyCursor:
        return "Busy";
    case Qt::OpenHandCursor:
        return "OpenHand";
    case Qt::ClosedHandCursor:
        return "ClosedHand";
    case Qt::DragCopyCursor:
        return "DragCopy";
    case Qt::DragMoveCursor:
        return "DragMove";
    case Qt::DragLinkCursor:
        return "DragLink";
    case Qt::BitmapCursor:
        return "Bitmap";
    default:
        return "???";
    }
}

class CursorWindowSpec
{
public:
    CursorWindowSpec(MirConnection *connection, const char *name)
    :  CursorWindowSpec(connection)
    {
        mir_window_spec_set_cursor_name(spec, name);
    }

    CursorWindowSpec(MirConnection *connection, MirRenderSurface *surface, int hotspotX, int hotspotY)
    :  CursorWindowSpec(connection)
    {
        mir_window_spec_set_cursor_render_surface(spec, surface, hotspotX, hotspotY);
    }

    ~CursorWindowSpec()
    {
        mir_window_spec_release(spec);
    }

    void apply(MirWindow *window)
    {
        mir_window_apply_spec(window, spec);
    }
private:
    CursorWindowSpec(MirConnection *connection) : spec(mir_create_window_spec(connection)) {}
    MirWindowSpec * const spec;
};

class BufferStream
{
public:
    BufferStream(MirRenderSurface *surface, int width, int height)
    : stream(mir_render_surface_get_buffer_stream(surface, width, height, mir_pixel_format_argb_8888))
    {
    }

    void draw(QImage const& image)
    {
        MirGraphicsRegion region;
        const bool validRegion = mir_buffer_stream_get_graphics_region(stream, &region);
        if (!validRegion)
            throw std::runtime_error("Could not get graphics region to draw into");

        auto regionLine = region.vaddr;
        Q_ASSERT(image.bytesPerLine() <= region.stride);

        for (int i = 0; i < image.height(); ++i) {
           memcpy(regionLine, image.scanLine(i), image.bytesPerLine());
           regionLine += region.stride;
        }
        mir_buffer_stream_swap_buffers_sync(stream);
    }

private:
    MirBufferStream * const stream;
};

class RenderSurface
{
public:
    RenderSurface(MirConnection *connection, int width, int height)
    : surface(mir_connection_create_render_surface_sync(connection, width, height)),
      stream(surface, width, height)
    {
        if (!mir_render_surface_is_valid(surface)) {
            throw std::runtime_error(mir_render_surface_get_error_message(surface));
        }
    }

    ~RenderSurface() { mir_render_surface_release(surface); }
    operator MirRenderSurface *() const { return surface; }
    void draw(QImage const& image) { stream.draw(image); }

private:
    MirRenderSurface * const surface;
    BufferStream stream;
};

} // anonymous namespace

void QMirClientCursor::changeCursor(QCursor *windowCursor, QWindow *window)
{
    if (!window) {
        return;
    }

    MirWindow *mirWindow = static_cast<QMirClientWindow*>(window->handle())->mirWindow();

    if (!mirWindow) {
        return;
    }


    if (windowCursor) {
        qCDebug(mirclientCursor, "changeCursor shape=%s, window=%p", qtCursorShapeToStr(windowCursor->shape()), window);
        if (!windowCursor->pixmap().isNull()) {
            configureMirCursorWithPixmapQCursor(mirWindow, *windowCursor);
        } else if (windowCursor->shape() == Qt::BitmapCursor) {
            // TODO: Implement bitmap cursor support
            applyDefaultCursorConfiguration(mirWindow);
        } else {
            const auto &cursorName = mShapeToCursorName.value(windowCursor->shape(), QByteArray("left_ptr"));
            CursorWindowSpec spec(mConnection, cursorName.data());
            spec.apply(mirWindow);
        }
    } else {
        applyDefaultCursorConfiguration(mirWindow);
    }

}

void QMirClientCursor::configureMirCursorWithPixmapQCursor(MirWindow *window, QCursor &cursor)
{
    QImage image = cursor.pixmap().toImage();

    if (image.format() != QImage::Format_ARGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }

    try {
        RenderSurface surface(mConnection, image.width(), image.height());
        surface.draw(image);

        CursorWindowSpec spec(mConnection, surface, cursor.hotSpot().x(), cursor.hotSpot().y());
        spec.apply(window);
    } catch(std::exception const& e) {
        qWarning("Error applying pixmap cursor: %s", e.what());
    }
}

void QMirClientCursor::applyDefaultCursorConfiguration(MirWindow *window)
{

    CursorWindowSpec spec(mConnection, "left_ptr");
    spec.apply(window);
}
