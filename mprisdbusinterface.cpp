#include "mprisdbusinterface.h"
#include <QDBusConnection>

MprisDBusInterfaceSignaler::MprisDBusInterfaceSignaler(QObject *parent) : QObject(parent) {}

MprisDBusInterface::MprisDBusInterface(QObject *parent) : QDBusAbstractAdaptor(parent) {}

bool        MprisDBusInterface::getCanRaise             () { return canRaise;            }
bool        MprisDBusInterface::getCanQuit              () { return canQuit;             }
bool        MprisDBusInterface::getFullscreen           () { return fullscreen;          }
bool        MprisDBusInterface::getCanSetFullscreen     () { return canSetFullscreen;    }
bool        MprisDBusInterface::getHasTrackList         () { return hasTrackList;        }
QString     MprisDBusInterface::getIdentity             () { return identity;            }
QStringList MprisDBusInterface::getSupportedURISchemes  () { return supportedURISchemes; }
QStringList MprisDBusInterface::getSupportedMimeTypes   () { return supportedMimeTypes;  }

void MprisDBusInterface::setCanRaise            ( bool        value ) { canRaise            = value; emit signaler.canRaiseChanged(value);            }
void MprisDBusInterface::setCanQuit             ( bool        value ) { canQuit             = value; emit signaler.canQuitChanged(value);             }
void MprisDBusInterface::setFullscreen          ( bool        value ) { fullscreen          = value; emit signaler.fullscreenChanged(value);          }
void MprisDBusInterface::setCanSetFullscreen    ( bool        value ) { canSetFullscreen    = value; emit signaler.canSetFullscreenChanged(value);    }
void MprisDBusInterface::setHasTrackList        ( bool        value ) { hasTrackList        = value; emit signaler.hasTracklistChanged(value);        }
void MprisDBusInterface::setIdentity            ( QString     value ) { identity            = value; emit signaler.identityChanged(value);            }
void MprisDBusInterface::setSupportedURISchemes ( QStringList value ) { supportedURISchemes = value; emit signaler.supportedURISchemesChanged(value); }
void MprisDBusInterface::setSupportedMimeTypes  ( QStringList value ) { supportedMimeTypes  = value; emit signaler.supportedMimeTypesChanged(value);  }

void MprisDBusInterface::Raise() { emit signaler.raiseCalled(); }
void MprisDBusInterface::Quit()  { emit signaler.quitCalled();  }
