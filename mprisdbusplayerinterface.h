#ifndef MPRISDBUSPLAYERINTERFACE_H
#define MPRISDBUSPLAYERINTERFACE_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QObject>

class MprisDBusPlayerSignaler : public QObject
{
    Q_OBJECT

public:
    MprisDBusPlayerSignaler(QObject *parent = nullptr);

signals:
    void Next();
    void Previous();
    void Pause();
    void PlayPause();
    void Stop();
    void Play();
    void Seek(qlonglong Offset);
    void SetPosition(QDBusObjectPath TrackId, qlonglong Offset);
    void OpenURI(QString Uri);

    void playbackStatusChanged (QString                 value );
    void loopStatusChanged     (QString                 value );
    void playbackRateChanged   (double                  value );
    void shuffleChanged        (bool                    value );
    void metadataChanged       (QMap<QString, QVariant> value );
    void volumeChanged         (double                  value );
    void positionChanged       (qlonglong               value );
    void minimumRateChanged    (double                  value );
    void maximumRateChanged    (double                  value );

    void canGoNextChanged     (bool value);
    void canGoPreviousChanged (bool value);
    void canPlayChanged       (bool value);
    void canPauseChanged      (bool value);
    void canSeekChanged       (bool value);
    void canControlChanged    (bool value);
};

class MprisDBusPlayerInterface : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player");

    Q_PROPERTY(QString                 PlaybackStatus READ getPlaybackStatus                       );
    Q_PROPERTY(QString                 LoopStatus     READ getLoopStatus     WRITE setLoopStatus   );
    Q_PROPERTY(double                  Rate           READ getPlaybackRate   WRITE setPlaybackRate );
    Q_PROPERTY(bool                    Shuffle        READ getShuffle        WRITE setShuffle      );
    Q_PROPERTY(QMap<QString, QVariant> Metadata       READ getMetadata                             );
    Q_PROPERTY(double                  Volume         READ getVolume         WRITE setVolume       );
    Q_PROPERTY(qlonglong               Position       READ getPosition                             );
    Q_PROPERTY(double                  MinimumRate    READ getMinimumRate                          );
    Q_PROPERTY(double                  MaximumRate    READ getMaximumRate                          );
    Q_PROPERTY(bool                    CanGoNext      READ getCanGoNext                            );
    Q_PROPERTY(bool                    CanGoPrevious  READ getCanGoPrevious                        );
    Q_PROPERTY(bool                    CanPlay        READ getCanPlay                              );
    Q_PROPERTY(bool                    CanPause       READ getCanPause                             );
    Q_PROPERTY(bool                    CanSeek        READ getCanSeek                              );
    Q_PROPERTY(bool                    CanControl     READ getCanControl                           );

public:
    explicit MprisDBusPlayerInterface(QObject *parent);
    MprisDBusPlayerSignaler signaler;

    // D-Bus Property Variables

    QString                 getPlaybackStatus ();
    QString                 getLoopStatus     ();
    double                  getPlaybackRate   ();
    bool                    getShuffle        ();
    QMap<QString, QVariant> getMetadata       ();
    double                  getVolume         ();
    qlonglong               getPosition       ();
    double                  getMinimumRate    ();
    double                  getMaximumRate    ();

    bool getCanGoNext     ();
    bool getCanGoPrevious ();
    bool getCanPlay       ();
    bool getCanPause      ();
    bool getCanSeek       ();
    bool getCanControl    ();

    void setLoopStatus     (QString                 value, bool emitChange = true);
    void setPlaybackRate   (double                  value, bool emitChange = true);
    void setShuffle        (bool                    value, bool emitChange = true);
    void setVolume         (double                  value, bool emitChange = true);

    void setPlaybackStatus (QString                 value );
    void setMetadata       (QMap<QString, QVariant> value );
    void setPosition       (qlonglong               value );
    void setMinimumRate    (double                  value );
    void setMaximumRate    (double                  value );

    void setCanGoNext     (bool value);
    void setCanGoPrevious (bool value);
    void setCanPlay       (bool value);
    void setCanPause      (bool value);
    void setCanSeek       (bool value);
    void setCanControl    (bool value);


private:
    QString     playbackStatus       = "Stopped" ;
    QString     loopStatus           = "None"    ;
    double      playbackRate         = 1.0       ;
    bool        shuffle              = false     ;
    QMap<QString, QVariant> metadata             ;
    double      volume               = 1         ;
    qlonglong   position             = 0         ;
    double      minimumRate          = 1         ;
    double      maximumRate          = 1         ;

    bool canGoNext      = false;
    bool canGoPrevious  = false;
    bool canPlay        = false;
    bool canPause       = false;
    bool canSeek        = false;
    bool canControl     = false;

public slots:
    Q_NOREPLY void Next();
    Q_NOREPLY void Previous();
    Q_NOREPLY void Pause();
    Q_NOREPLY void PlayPause();
    Q_NOREPLY void Stop();
    Q_NOREPLY void Play();
    Q_NOREPLY void Seek(qlonglong Offset);
    Q_NOREPLY void SetPosition(QDBusObjectPath TrackId, qlonglong Offset);
    Q_NOREPLY void OpenURI(QString Uri);

signals:
    void Seeked(qlonglong Position);
};

#endif // MPRISDBUSPLAYERINTERFACE_H
