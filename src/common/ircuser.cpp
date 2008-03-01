/***************************************************************************
 *   Copyright (C) 2005-08 by the Quassel Project                          *
 *   devel@quassel-irc.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) version 3.                                           *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "ircuser.h"
#include "util.h"

#include "network.h"
#include "signalproxy.h"
#include "ircchannel.h"

#include <QTextCodec>
#include <QDebug>

IrcUser::IrcUser(const QString &hostmask, Network *network) : SyncableObject(network),
    _initialized(false),
    _nick(nickFromMask(hostmask)),
    _user(userFromMask(hostmask)),
    _host(hostFromMask(hostmask)),
    _realName(),
    _awayMessage(),
    _away(false),
    _server(),
    _idleTime(QDateTime::currentDateTime()),
    _ircOperator(),
    _lastAwayMessage(0),
    _network(network),
    _codecForEncoding(0),
    _codecForDecoding(0)
{
  updateObjectName();
}

IrcUser::~IrcUser() {
}

// ====================
//  PUBLIC:
// ====================

QString IrcUser::user() const {
  return _user;
}

QString IrcUser::host() const {
  return _host;
}

QString IrcUser::nick() const {
  return _nick;
}

QString IrcUser::realName() const {
  return _realName;
}

QString IrcUser::hostmask() const {
  return QString("%1!%2@%3").arg(nick()).arg(user()).arg(host());
}

bool IrcUser::isAway() const {
  return _away;
}

QString IrcUser::awayMessage() const {
  return _awayMessage;
}

QString IrcUser::server() const {
  return _server;
}

QDateTime IrcUser::idleTime() const {
  return _idleTime;
}

QString IrcUser::ircOperator() const {
  return _ircOperator;
}

int IrcUser::lastAwayMessage() const {
  return _lastAwayMessage;
}

QString IrcUser::userModes() const {
  return _userModes;
}

QStringList IrcUser::channels() const {
  QStringList chanList;
  IrcChannel *channel;
  foreach(channel, _channels) {
    chanList << channel->name();
  }
  return chanList;
}

Network* IrcUser::network() const {
  return _network;
}

QTextCodec *IrcUser::codecForEncoding() const {
  return _codecForEncoding;
}

void IrcUser::setCodecForEncoding(const QString &name) {
  setCodecForEncoding(QTextCodec::codecForName(name.toAscii()));
}

void IrcUser::setCodecForEncoding(QTextCodec *codec) {
  _codecForEncoding = codec;
}

QTextCodec *IrcUser::codecForDecoding() const {
  return _codecForDecoding;
}

void IrcUser::setCodecForDecoding(const QString &name) {
  setCodecForDecoding(QTextCodec::codecForName(name.toAscii()));
}

void IrcUser::setCodecForDecoding(QTextCodec *codec) {
  _codecForDecoding = codec;
}

QString IrcUser::decodeString(const QByteArray &text) const {
  if(!codecForDecoding()) return network()->decodeString(text);
  return ::decodeString(text, codecForDecoding());
}

QByteArray IrcUser::encodeString(const QString &string) const {
  if(codecForEncoding()) {
    return codecForEncoding()->fromUnicode(string);
  }
  return network()->encodeString(string);
}

// ====================
//  PUBLIC SLOTS:
// ====================
void IrcUser::setUser(const QString &user) {
  if(!user.isEmpty() && _user != user) {
    _user = user;
    emit userSet(user);
  }
}

void IrcUser::setRealName(const QString &realName) {
  if (!realName.isEmpty() && _realName != realName) {
    _realName = realName;
    emit realNameSet(realName);
  }
}

void IrcUser::setAway(const bool &away) {
  if(away != _away) {
    _away = away;
    emit awaySet(away);
  }
}

void IrcUser::setAwayMessage(const QString &awayMessage) {
  if(!awayMessage.isEmpty() && _awayMessage != awayMessage) {
    _awayMessage = awayMessage;
    emit awayMessageSet(awayMessage);
  }
}

void IrcUser::setIdleTime(const QDateTime &idleTime) {
  if(idleTime.isValid() && _idleTime != idleTime) {
    _idleTime = idleTime;
    emit idleTimeSet(idleTime);
  }
}

void IrcUser::setServer(const QString &server) {
  if(!server.isEmpty() && _server != server) {
    _server = server;
    emit serverSet(server);
  }
}

void IrcUser::setIrcOperator(const QString &ircOperator) {
  if(!ircOperator.isEmpty() && _ircOperator != ircOperator) {
    _ircOperator = ircOperator;
    emit ircOperatorSet(ircOperator);
  }
}

void IrcUser::setLastAwayMessage(const int &lastAwayMessage) {
  if(lastAwayMessage > _lastAwayMessage) {
    _lastAwayMessage = lastAwayMessage;
    emit lastAwayMessageSet(lastAwayMessage);
  }
}

void IrcUser::setHost(const QString &host) {
  if(!host.isEmpty() && _host != host) {
    _host = host;
    emit hostSet(host);
  }
}

void IrcUser::setNick(const QString &nick) {
  if(!nick.isEmpty() && nick != _nick) {
    _nick = nick;
    updateObjectName();
    emit nickSet(nick);
  }
}

void IrcUser::updateObjectName() {
  renameObject(QString::number(network()->networkId().toInt()) + "/" + _nick);
}

void IrcUser::updateHostmask(const QString &mask) {
  if(mask == hostmask())
    return;

  QString user = userFromMask(mask);
  QString host = hostFromMask(mask);
  setUser(user);
  setHost(host);
}

void IrcUser::joinChannel(IrcChannel *channel) {
  Q_ASSERT(channel);
  if(!_channels.contains(channel)) {
    _channels.insert(channel);
    channel->joinIrcUsers(this);
    connect(channel, SIGNAL(destroyed()), this, SLOT(channelDestroyed()));
  }
}

void IrcUser::joinChannel(const QString &channelname) {
  joinChannel(network()->newIrcChannel(channelname));
}

void IrcUser::partChannel(IrcChannel *channel) {
  if(_channels.contains(channel)) {
    _channels.remove(channel);
    disconnect(channel, 0, this, 0);
    channel->part(this);
    emit channelParted(channel->name());
    if(_channels.isEmpty() && network()->isMe(this))
      deleteLater();
  }
}

void IrcUser::partChannel(const QString &channelname) {
  IrcChannel *channel = network()->ircChannel(channelname);
  if(channel == 0) {
    qWarning() << "IrcUser::partChannel(): received part for unknown Channel" << channelname;
  } else {
    partChannel(channel);
  }
}

void IrcUser::channelDestroyed() {
  // private slot!
  IrcChannel *channel = static_cast<IrcChannel*>(sender());
  if(_channels.contains(channel)) {
    _channels.remove(channel);
    if(_channels.isEmpty())
      deleteLater();
  }
}

void IrcUser::setUserModes(const QString &modes) {
  _userModes = modes;
  emit userModesSet(modes);
}

void IrcUser::addUserMode(const QString &mode) {
  if(!_userModes.contains(mode)) {
    _userModes += mode;
    emit userModeAdded(mode);
  }
}

void IrcUser::removeUserMode(const QString &mode) {
  if(_userModes.contains(mode)) {
    _userModes.remove(mode);
    emit userModeRemoved(mode);
  }
}

void IrcUser::initSetChannels(const QStringList channels) {
  foreach(QString channel, channels) {
    joinChannel(channel);
  }
}

