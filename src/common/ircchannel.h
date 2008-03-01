/***************************************************************************
 *   Copyright (C) 2005/06 by the Quassel Project                          *
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

#ifndef _IRCCHANNEL_H_
#define _IRCCHANNEL_H_

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "syncableobject.h"

class IrcUser;
class Network;
class SignalProxy;

class IrcChannel : public SyncableObject {
  Q_OBJECT

  Q_PROPERTY(QString name READ name STORED false)
  Q_PROPERTY(QString topic READ topic WRITE setTopic STORED false)
  Q_PROPERTY(QString password READ password WRITE setPassword STORED false)

public:
  IrcChannel(const QString &channelname, Network *network);
  ~IrcChannel();

  bool isKnownUser(IrcUser *ircuser) const;
  bool isValidChannelUserMode(const QString &mode) const;

  inline QString name() const { return _name; }
  inline QString topic() const { return _topic; }
  inline QString password() const { return _password; }

  inline QList<IrcUser *> ircUsers() const { return _userModes.keys(); }

  QString userModes(IrcUser *ircuser) const;
  QString userModes(const QString &nick) const;

  inline QTextCodec *codecForEncoding() const { return _codecForEncoding; }
  inline QTextCodec *codecForDecoding() const { return _codecForDecoding; }
  void setCodecForEncoding(const QString &codecName);
  void setCodecForEncoding(QTextCodec *codec);
  void setCodecForDecoding(const QString &codecName);
  void setCodecForDecoding(QTextCodec *codec);

  QString decodeString(const QByteArray &text) const;
  QByteArray encodeString(const QString &string) const;

public slots:
  void setTopic(const QString &topic);
  void setPassword(const QString &password);

  void joinIrcUsers(const QList<IrcUser *> &users, const QStringList &modes);
  void joinIrcUsers(const QStringList &nicks, const QStringList &modes);
  void joinIrcUsers(IrcUser *ircuser);
  void joinIrcUsers(const QString &nick);

  void part(IrcUser *ircuser);
  void part(const QString &nick);

  void setUserModes(IrcUser *ircuser, const QString &modes);
  void setUserModes(const QString &nick, const QString &modes);

  void addUserMode(IrcUser *ircuser, const QString &mode);
  void addUserMode(const QString &nick, const QString &mode);

  void removeUserMode(IrcUser *ircuser, const QString &mode);
  void removeUserMode(const QString &nick, const QString &mode);

  // init geters
  QVariantMap initUserModes() const;

  // init seters
  void initSetUserModes(const QVariantMap &usermodes);

signals:
  void topicSet(const QString &topic);
  void passwordSet(const QString &password);
  void userModesSet(QString nick, QString modes);
  //void userModesSet(IrcUser *ircuser, QString modes);
  void userModeAdded(QString nick, QString mode);
  //void userModeAdded(IrcUser *ircuser, QString mode);
  void userModeRemoved(QString nick, QString mode);
  //void userModeRemoved(IrcUser *ircuser, QString mode);

  void ircUsersJoined(QList<IrcUser *> ircusers);
  void ircUsersJoined(QStringList nicks, QStringList modes);
  void ircUserParted(IrcUser *ircuser);
  void ircUserNickSet(IrcUser *ircuser, QString nick);
  void ircUserModeAdded(IrcUser *ircuser, QString mode);
  void ircUserModeRemoved(IrcUser *ircuser, QString mode);
  void ircUserModesSet(IrcUser *ircuser, QString modes);

private slots:
   void ircUserDestroyed();
   void ircUserNickSet(QString nick);

private:
  bool _initialized;
  QString _name;
  QString _topic;
  QString _password;

  QHash<IrcUser *, QString> _userModes;

  Network *network;

  QTextCodec *_codecForEncoding;
  QTextCodec *_codecForDecoding;
};

#endif
