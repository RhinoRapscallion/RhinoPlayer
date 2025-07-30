#ifndef MPRISDBUSINTERFACE_H
#define MPRISDBUSINTERFACE_H

#include <QObject>
#include <QDBusAbstractAdaptor>

class MprisDBusInterfaceSignaler : public QObject
{
    Q_OBJECT

public:
    explicit    MprisDBusInterfaceSignaler(QObject *parent = nullptr);

signals:
    void canRaiseChanged             ( bool        value );
    void canQuitChanged              ( bool        value );
    void fullscreenChanged           ( bool        value );
    void canSetFullscreenChanged     ( bool        value );
    void hasTracklistChanged         ( bool        value );
    void identityChanged             ( QString     value );
    void supportedURISchemesChanged  ( QStringList value );
    void supportedMimeTypesChanged   ( QStringList value );

    void raiseCalled();
    void quitCalled();
};

class MprisDBusInterface : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2");

    Q_PROPERTY(bool         CanRaise            READ getCanRaise                                    );
    Q_PROPERTY(bool         CanQuit             READ getCanQuit                                     );
    Q_PROPERTY(bool         Fullscreen          READ getFullscreen              WRITE setFullscreen );
    Q_PROPERTY(bool         CanSetFullscreen    READ getCanSetFullscreen                            );
    Q_PROPERTY(bool         HasTrackList        READ getHasTrackList                                );
    Q_PROPERTY(QString      Identity            READ getIdentity                                    );
    Q_PROPERTY(QStringList  SupportedUriSchemes READ getSupportedURISchemes                         );
    Q_PROPERTY(QStringList  SupportedMimeTypes  READ getSupportedMimeTypes                          );

public:
    MprisDBusInterfaceSignaler signaler;
    explicit    MprisDBusInterface(QObject *parent = nullptr);
    bool        getCanRaise             ();
    bool        getCanQuit              ();
    bool        getFullscreen           ();
    bool        getCanSetFullscreen     ();
    bool        getHasTrackList         ();
    QString     getIdentity             ();
    QStringList getSupportedURISchemes  ();
    QStringList getSupportedMimeTypes   ();

    void setCanRaise            ( bool        value );
    void setCanQuit             ( bool        value );
    void setFullscreen          ( bool        value );
    void setCanSetFullscreen    ( bool        value );
    void setHasTrackList        ( bool        value );
    void setIdentity            ( QString     value );
    void setSupportedURISchemes ( QStringList value );
    void setSupportedMimeTypes  ( QStringList value );

private:
    bool canRaise           = false;
    bool canQuit            = false;
    bool fullscreen         = false;
    bool canSetFullscreen   = false;
    bool hasTrackList       = false;
    QString identity        = "";
    QStringList supportedURISchemes;
    QStringList supportedMimeTypes;

public slots:
    Q_NOREPLY void Raise();
    Q_NOREPLY void Quit();
};

#endif // MPRISDBUSINTERFACE_H
