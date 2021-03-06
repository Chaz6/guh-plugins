/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef TVEVENTHANDLER_H
#define TVEVENTHANDLER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QDateTime>
#include <QTextStream>
#include <QRegExp>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

class TvEventHandler : public QTcpServer
{
    Q_OBJECT
public:
    explicit TvEventHandler(const QHostAddress &host, const int &port = 8080, QObject *parent = 0);
    void incomingConnection(qintptr socket) override;

private:
    QHostAddress m_host;
    int m_port;
    bool m_expectingData;

signals:
    void eventOccured(const QByteArray &path);

private slots:
    void readClient();
    void onDisconnected();

};

#endif // TVEVENTHANDLER_H
