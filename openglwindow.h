/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui/QWindow>
#include <QtGui/QOpenGLFunctions>

#include <QtNetwork\qtcpserver.h>
#include <QtNetwork\QTcpSocket>

#include <vector>

QT_BEGIN_NAMESPACE
class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;
QT_END_NAMESPACE

//! [1]
class OpenGLWindow : public QWindow, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit OpenGLWindow(QWindow *parent = 0);
    ~OpenGLWindow();

    virtual void render(QPainter *painter);
    virtual void render();

    virtual void initialize();


public slots:
    void renderNow();
	void newConnection( );
	
	void updateWeather( ) {
		// MAJ TEMPS
		temps++;
		temps %= 4;

		qDebug( ) << "--UPDATE WEATHER--";


		for( int i = 0; i != mes_clients.size( ); ++i ) {
			char *c = (char *)malloc( 20 * sizeof( char ) );
			sprintf( c, "%i", ( i + temps + 1 ) % 4 );
			mes_clients[i]->write( c );
			mes_clients[i]->flush( );
			qDebug( ) << c;

			free( c );
		}
	}

	void readyRead( ) {
		//qDebug( ) << client->readAll( );
		QByteArray c = client->readAll();
		temps = atoi( c.data() );
	}
protected:

    bool event(QEvent *event);

    void exposeEvent(QExposeEvent *event);

	QTcpServer *server;
	QTcpSocket *client;

	std::vector<QTcpSocket *> mes_clients;

	// 0: printemps
	// 1: été
	// 2: automne
	// 3: hiver
	int temps = 0;
	

	void closeConnections( ) {
		for( int i = 0; i != mes_clients.size( ); ++i ) {
			mes_clients[i]->close( );
		}

		mes_clients.clear( );
	}

	void initServer( ) {
		server = new QTcpServer( this );
		client = NULL;

		connect( server, SIGNAL( newConnection( ) ),
				 this, SLOT( newConnection( ) ) );

		server->listen( QHostAddress::Any, 4578 );
		qDebug( ) << "--SERVER INITIALIZED--";
	}

	void initClient( ) {
		server = NULL;
		client = new QTcpSocket( this );

		connect( client, SIGNAL( readyRead( ) ), this, SLOT( readyRead( ) ) );

		client->connectToHost( "127.0.0.1", 4578 );
		// we need to wait...
		if( !client->waitForConnected( 5000 ) ) {
			qDebug( ) << "--ERROR-- : " << client->errorString( );
		} else {
			qDebug( ) << "--CLIENT CONNECTED--";
		}
	}

private:


    QOpenGLContext *m_context;
    QOpenGLPaintDevice *m_device;
};
//! [1]

