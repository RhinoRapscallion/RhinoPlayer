#include "musicdatabase.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QMediaMetaData>
#include <QAudioOutput>
#include <QImage>
#include <QCryptographicHash>
#include <qregularexpression.h>


// Default Constructor, database always stars as invalid.
// Use connectToDatabase or CreateDatabase to connect and validate
// a music database
MusicDatabase::MusicDatabase(QObject *parent)
    : QObject{parent}
{
    valid = false;

    QObject::connect(this, &MusicDatabase::scanComplete, this, [=](){
        if (mp != nullptr)
        {
            delete mp;
            mp = nullptr;
        }
    });
}

// Destructor
// As the scanner uses a temperary QMusicPlayer Object
// the scanner must be deleted upon deletion of this object
// The Scanner normally takes care of the deletion when scanning is complete
// this destructor ensures its deletion provided it hasnt already been deleted.
MusicDatabase::~MusicDatabase()
{
    if (mp != nullptr)
    {
        delete mp;
        mp = nullptr;
    }
}

// Connects to and validates an existing database
// The database must contain a tabel named 'Songs'
// The 'Songs' table must contain columns:
// 'Image', 'Artist', 'Album', 'Track', and 'File'
bool MusicDatabase::connectToDatabase(QString databaseFilePath)
{
    // Open Database connection on default connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(databaseFilePath);

    // Begin Database Verification
    bool ok = db.open();
    qDebug() << "Database Is Open: " << ok;

    if (!ok)
    {
        valid = false;
        qDebug() << "Database Error: " << db.lastError();
        return false;
    }

    // Verify songs table
    QStringList tables = db.tables();

    if (!tables.contains("Songs"))   { valid = false; db.close(); return false; };
    QSqlRecord verifyRecord = db.record("Songs");

    // Magic Value Issue, To Be Removed
    //if (verifyRecord.count() != 7) { valid = false; db.close(); return false; };
    QStringList verifyColumns;

    for (int i = verifyRecord.count(); i > 0; i--) { verifyColumns << verifyRecord.fieldName(i); }

    if (!(verifyColumns.contains("Image" )  &&
          verifyColumns.contains("Artist")  &&
          verifyColumns.contains("ContributingArtist") &&
          verifyColumns.contains("Album" )  &&
          verifyColumns.contains("Track" )  &&
          verifyColumns.contains("Title" )  &&
          verifyColumns.contains("File"  )))

    // Return True if DB validated
    valid = true;
    return true;
}

// Creates a database
// WARNING: this fuction will delete the database if it already exists
// used to fix corrupted databases or make a non-existant one
bool MusicDatabase::createDatabase(QString databaseFilePath)
{
    // Remove Old Database
    if (QFile::exists(databaseFilePath)) { QFile::remove(databaseFilePath); }

    //Open Database
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(databaseFilePath);

    bool ok = db.open();
    qDebug() << "Database Is Open: " << ok;

    if (!ok)
    {
        valid = false;
        qDebug() << "Database Error: " << db.lastError();
        return false;
    }

    // Create Tables
    QSqlQuery("CREATE TABLE Songs (Image TEXT COLLATE NOCASE, Artist TEXT COLLATE NOCASE, ContributingArtist TEXT COLLATE NOCASE, Album TEXT COLLATE NOCASE, Track int, Title TEXT, File TEXT PRIMARY KEY)");

    qDebug() << (db.tables());

    valid = true;
    return true;
}

// Scans a folder into the database
// Uses a QMediaPlayer for metadata
// If the database is not valid, returns without doing anything
// Will Scan MP3, Flac, and M4A files
void MusicDatabase::scanFolder(QString directory)
{
    if (!valid) { return; }
    QDirIterator ittr(directory, {"*mp3", "*.flac", "*.m4a"}, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);


    while (ittr.hasNext()) {
        scanList << (ittr.next());
    }

    if (mp == nullptr)
    {
        mp = new QMediaPlayer();
        QObject::connect(mp, &QMediaPlayer::mediaStatusChanged, this, &MusicDatabase::scanMedia);
    }



    if (scanList.isEmpty()) { return; }
    emit scanStatus(scanList.first());
    mp->setSource(QUrl::fromLocalFile(scanList.first()));
}

// Gets the media Metadata from loaded file and inserts it into the database
// will continue scanning until "scanList" QStack is empty.
// File scans are initiated by setting the source for the QMediaPlayer pointed to by mp
void MusicDatabase::scanMedia(QMediaPlayer::MediaStatus status)
{
    if (status != QMediaPlayer::LoadedMedia) {
        return;
    }
    qDebug() << mp->source();

    QMediaMetaData metaData = mp->metaData();
    qDebug() << metaData.stringValue(metaData.Title) << metaData.stringValue(metaData.AlbumTitle) << metaData.stringValue(metaData.AlbumArtist);

    // use of Regex to get titles and track numbers from files if they exist
    static QRegularExpression titleFromFile("(?<title>.*(?=\\..*$))");
    static QRegularExpression trackFromFile("(?<track>^[0-9]*)");
    QRegularExpressionMatch match;

    // Prepare INSERT into database
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO "
                  "Songs  ( Image,  Artist,  ContributingArtist,  Album,  Track,  Title,  File) "
                  "VALUES (:image, :artist, :contributingArtist, :album, :track, :title, :file)");

    // Album and artist cannot be determined from filename at the current moment in time
    // Will likely support artist/album/## song.ext folder structure
    // Uses album artist tag to prevent large lists of extranious artists from being displayed
    // To Do: User Choice to use album artist tag or artist tag
    QString artist = metaData.stringValue(metaData.AlbumArtist);
    if (artist.isEmpty()) artist = "Unknown Artist";
    query.bindValue(":artist", artist);

    QString contribArtist = metaData.value(metaData.ContributingArtist).toStringList().join(",");
    if (artist.isEmpty()) artist = "Unknown Artist";
    query.bindValue(":contributingArtist", contribArtist);

    QString album = metaData.stringValue(metaData.AlbumTitle);
    if (album.isEmpty()) album = "Unknown Album";
    query.bindValue(":album", album);

    // Limitations of the FFMpeg backend prevent mp3s from reporting their tracknumber in a way that QMediaMetadata can easily read
    // in the case where track == 0, indicating error
    // the track number is read from the file name usig whatever numbers are out front of the track title.
    // as a result, if the track number is not read, and the file name cannot be read correctly, the track is set to zero.
    int track = metaData.value(metaData.TrackNumber).toInt();
    if (track == 0) {
        match = trackFromFile.match(mp->source().fileName());
        if ( match.hasCaptured("track")) {track = match.captured("track").toInt(); }
    }

    query.bindValue(":track", track);

    // Ensures that the title of the track is always read
    // If the title is reported by QMediaMetadata, it is used
    // otherwise the filename without extention is used
    // if the REGEX match fails to report a filename, the filename with extention is used
    QString title = metaData.stringValue(metaData.Title);
    if(title.isEmpty()) {
        match = titleFromFile.match(mp->source().fileName());
        if (match.hasCaptured("title")) { title = match.captured("title"); }
        else {title = mp->source().fileName(); }
    }

    query.bindValue(":title", title);

    query.bindValue(":file", mp->source().toString());

    // Saves the album art into a hidden folder as a hash of its data, artist, and album.
    // the image path is then stored in the image column. this is to reduce storage cost of these files
    QString hash = "";
    QString filename = "";

    if (metaData.value(metaData.ThumbnailImage).isValid())
    {
        QImage art = metaData.value(metaData.ThumbnailImage).value<QImage>();
        QByteArray arr(art.bits());
        arr.append(QString("%1%2").arg(artist, album).toUtf8());
        hash = QCryptographicHash::hash(arr, QCryptographicHash::Sha256).toHex();

        QDir().mkpath(QDir::currentPath() + "/.images");
        filename = QDir::currentPath() + QString("/.images/%1.png").arg(hash);;
        if(!QFile::exists(filename))
        { art.save(filename); }
    }

    query.bindValue(":image", filename);

    qDebug() << query.boundValues();
    qDebug() << query.boundValueNames();

    if (!query.exec())
    {
        qDebug() << query.lastError();
        qDebug () << query.lastQuery();
        return;
    }

    scanList.removeFirst();
    //scanList.clear();

    //QList<QMediaMetaData::Key> keys = metaData.keys();
    //foreach (QMediaMetaData::Key key, keys)
    //{
    //    qDebug() << key;
    //}

    if (scanList.isEmpty()) { emit scanComplete(); return; }
    else { emit scanStatus(scanList.first()); };
    mp->setSource(QUrl::fromLocalFile(scanList.first()));
}

// Returns a list of the artists in the database
// as filtering is top down Artist->album->song no filtering takes place
QStringList MusicDatabase::getArtists() {

    if (!valid) { return QStringList(); }

    QSqlQuery query;
    query.prepare("SELECT DISTINCT Artist FROM 'Songs' "
                  "ORDER BY Artist;");

    if (!query.exec())
    {
        qDebug() << query.lastError();
        qDebug () << query.lastQuery();
        return QStringList();
    }

    QStringList ret;

    int artistIndex = query.record().indexOf("Artist");

    while (query.next())
    {
        ret << query.value(artistIndex).toString();
    }

    return ret;
}

// Returns a list of albums
// will filter by "filterArtist" QString if it is not empty
// If not filtered, the string will report artist information
QStringList MusicDatabase::getAlbums() {
    if (!valid) { return QStringList(); }

    QSqlQuery query;
    query.prepare("SELECT DISTINCT Artist, Album FROM 'Songs' "
                  "WHERE Artist LIKE :artist "
                  "ORDER BY Artist, Album;");
    query.bindValue(":artist", filterArtist.isEmpty() ? "%" : filterArtist);

    if (!query.exec())
    {
        qDebug() << query.lastError();
        qDebug () << query.lastQuery();
        return QStringList();
    }
    QStringList ret;

    int artistIndex = query.record().indexOf("Artist");
    int albumIndex = query.record().indexOf("Album");

    while (query.next())
    {
        ret << (filterArtist.isEmpty() ? QString("%1 - %2")
                                             .arg(query.value(artistIndex).toString(),
                                                  query.value(albumIndex).toString())
                                       : query.value(albumIndex).toString());
    }

    return ret;
}

// Gets songs filtered via the filterArtist and filterAlbum strings
// if not filtered, will report artist and album information along side the title
QStringList MusicDatabase::getSongNames() {
    if (!valid) { return QStringList(); }

    QSqlQuery query;
    query.prepare("SELECT * FROM 'Songs' "
                  "WHERE Album LIKE :album AND Artist LIKE :artist "
                  "ORDER BY Artist, Album, Track");
    query.bindValue(":artist", filterArtist.isEmpty() ? "%" : filterArtist);
    query.bindValue(":album" , filterAlbum.isEmpty() ? "%" : filterAlbum );

    if (!query.exec())
    {
        qDebug() << query.lastError();
        qDebug () << query.lastQuery();
        return QStringList();
    }

    QStringList ret;

    int artistIndex = query.record().indexOf("Artist");
    int albumIndex = query.record().indexOf("Album");
    int songIndex = query.record().indexOf("Title");

    QString song;

    while (query.next())
    {
        song = "";
        if (filterArtist.isEmpty()) song.append(QString("%1 - ").arg(query.value(artistIndex).toString()));
        if (filterAlbum.isEmpty()) song.append(QString("%1 - ").arg(query.value(albumIndex).toString()));
        song.append(query.value(songIndex).toString());
        ret << song;
    }

    return ret;
}

// Sets artist and album filters based its id an album list
//
// The album list will be constructed in alphebetical order
//
// If filterByArtist is set, the list will be filtered by the artist
// set by setFilterArtist(). If false, the list will be unfiltered
bool MusicDatabase::setFiltersByAlbumID(int idx, bool filterByArtist)
{
    if (!valid) { return false; }

    QSqlQuery query;
    query.prepare("SELECT DISTINCT Artist, Album FROM 'Songs' "
                  "WHERE Artist LIKE :artist "
                  "ORDER BY Artist, Album;");
    query.bindValue(":artist", filterArtist.isEmpty() || !filterByArtist ? "%" : filterArtist);

    if (!query.exec())
    {
        qDebug() << query.lastError();
        qDebug () << query.lastQuery();
        return false;
    }

    if (!query.seek(idx))
    {
        qDebug() << "Album Index out of Range";
        return false;
    }

    int artistIndex = query.record().indexOf("Artist");
    int albumIndex = query.record().indexOf("Album");

    filterAlbum = query.value(albumIndex).toString();
    filterArtist = query.value(artistIndex).toString();

    return true;
}

// Sets the filters to match the selected album
// Used when there is no artist filter for albums to use
// calls getSongs when filters are set
//
// Internally uses setFiltersByAlbumID, see comments for details on filterByArtist feild
QStringList MusicDatabase::getSongNamesByAlbumID(int idx, bool filterByArtist) {

    if (setFiltersByAlbumID(idx, filterByArtist)) return getSongNames();
    else return QStringList();
}

// Returns information about a song at index Idx with current filters
// this is used to pass information to the MusicPlayer class
Song MusicDatabase::getSong(int idx)
{
    if (!valid) { return Song {"", "", "", "", "", "", -1}; }

    QSqlQuery query;
    query.prepare("SELECT * FROM 'Songs'"
                  "WHERE Album LIKE :album AND Artist LIKE :artist "
                  "ORDER BY Artist, Album, Track");
    query.bindValue(":artist", filterArtist.isEmpty() ? "%" : filterArtist);
    query.bindValue(":album" , filterAlbum.isEmpty() ? "%" : filterAlbum );

    if (!query.exec())
    {
        qDebug() << query.lastError();
        qDebug () << query.lastQuery();
        return Song {"", "", "", "", "", "", -1};
    }

    int artistIndex = query.record().indexOf("Artist");
    int albumArtistIndex = query.record().indexOf("AlbumArtist");
    int albumIndex = query.record().indexOf("Album");
    int titleIndex = query.record().indexOf("Title");
    int fileIndex = query.record().indexOf("File");
    int trackIndex = query.record().indexOf("Track");
    int imageIndex = query.record().indexOf("Image");

    if (query.seek(idx))
    {
        int track = query.value(trackIndex).toInt();
        return Song {
            query.value(artistIndex).toString(),
            query.value(albumArtistIndex).toString(),
            query.value(albumIndex).toString(),
            query.value(titleIndex).toString(),
            query.value(fileIndex).toString(),
            query.value(imageIndex).toString(),
            track < 0 ? 0 : track
        };
    }
    else
    {
        return Song {"", "", "", "", "", "", -1};
    }
}

QList<Song> MusicDatabase::getSongs()
{
    QList<Song> ret;

    if (!valid) { return ret; }

    QSqlQuery query;
    query.prepare("SELECT * FROM 'Songs' "
                  "WHERE Album LIKE :album AND Artist LIKE :artist "
                  "ORDER BY Artist, Album, Track");
    query.bindValue(":artist", filterArtist.isEmpty() ? "%" : filterArtist);
    query.bindValue(":album" , filterAlbum.isEmpty() ? "%" : filterAlbum );

    if (!query.exec())
    {
        qDebug() << query.lastError();
        qDebug () << query.lastQuery();
        return ret;
    }

    int artistIndex = query.record().indexOf("Artist");
    int albumArtistIndex = query.record().indexOf("AlbumArtist");
    int albumIndex = query.record().indexOf("Album");
    int titleIndex = query.record().indexOf("Title");
    int fileIndex = query.record().indexOf("File");
    int trackIndex = query.record().indexOf("Track");
    int imageIndex = query.record().indexOf("Image");

    while (query.next())
    {
        int track = query.value(trackIndex).toInt();
        ret.append(
            Song {
                query.value(artistIndex).toString(),
                query.value(albumArtistIndex).toString(),
                query.value(albumIndex).toString(),
                query.value(titleIndex).toString(),
                query.value(fileIndex).toString(),
                query.value(imageIndex).toString(),
                track < 0 ? 0 : track
        });
    }

    return ret;
}

void MusicDatabase::setArtist(QString artist){
    filterArtist = artist;
    emit songsFiltered();
}

void MusicDatabase::setAlbum(QString album){
    filterAlbum = album;
    emit songsFiltered();
}

bool MusicDatabase::filteredByArtist()
{
    return !filterArtist.isEmpty();
}

bool MusicDatabase::filteredByAlbum()
{
    return !filterAlbum.isEmpty();
}

