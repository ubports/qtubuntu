#ifndef MENUREGISTRAR_H
#define MENUREGISTRAR_H

#include <QObject>
#include <QWindow>
#include <QPointer>
#include <QDBusObjectPath>

class MenuRegistrar : public QObject
{
    Q_OBJECT
public:
    MenuRegistrar();
    ~MenuRegistrar();

    void registerMenuForWindow(QWindow* window, const QDBusObjectPath& path);
    void unregisterMenu();

    bool eventFilter(QObject * watched, QEvent * event);

private Q_SLOTS:
    void onRegistrarServiceChanged();

private:
    void registerMenu();

    QString m_service;
    QDBusObjectPath m_path;
    QPointer<QWindow> m_window;
    QString m_registeredSurfaceId;
};


#endif // MENUREGISTRAR_H

