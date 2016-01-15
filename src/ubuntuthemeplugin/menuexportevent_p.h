#ifndef MENUEXPORTEVENT_P_H
#define MENUEXPORTEVENT_P_H

#include <QEvent>

class MenuExportEvent : public QEvent
{
public:
    MenuExportEvent(const QString& objectName, const QString& objectPath)
        : QEvent(mType)
        , mObjectName(objectName)
        , mObjectPath(objectPath)
    {
    }

    static const QEvent::Type mType;
    QString mObjectName;
    QString mObjectPath;
};

#endif // MENUEXPORTEVENT_P_H
