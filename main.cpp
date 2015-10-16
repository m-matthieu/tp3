#include "openglwindow.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>

#include <QtCore/qmath.h>
#include <QMouseEvent>
#include <QKeyEvent>
//#include <sys/time.h>
#include <time.h>
#include <iostream>

#include <QtCore>
#include <QtGui>
#include <QThread>
#include <QTimer>

using namespace std;


struct point
{
    float x, y ,z; 

};


class paramCamera
{
public:
    float rotX = -45.0;
    float rotY = -45.0;
    float ss = 1.0f;
    float anim = 0.0f;

    int etat = 0;
};

class monThread : public QThread {
private:
	void run() {
		QThread::run();
		qDebug( ) << "From worker thread: " << currentThreadId( );
	}
};

class TriangleWindow : public OpenGLWindow
{
	
public:
    TriangleWindow();
	explicit TriangleWindow( int maj );
    void initialize();
    void render();
    bool event(QEvent *event);

    void keyPressEvent(QKeyEvent *event);

    void displayTriangles();
    void displayLines();
    void displayTrianglesC();
    void displayPoints();
    void displayTrianglesTexture();

    void displayColor(float);

    void loadMap(QString localPath);
    paramCamera* c;

	struct partic {
		partic() {
			x = 0; y = 0; z = 0;
		}
		partic( float a, float b, float c ) {
			x = a; y = b; z = c;
		}
		float x, y, z;
	};

	struct goutte_eau {
		partic pos;
		float speed;
	};

private:
    bool master = false;
    int m_frame;
    QImage m_image;
    point *p;


    int carte=1;


    int maj = 20;

    QTimer *timer;

	monThread thr;

	QTimer *server_timer;

	

	partic gravity; // ( 0, -1, 0 );
	partic wind; // ( 0.5f, 0, 0.5f );
	float windForce = .2f;
	float speedRand = 0.05f;
	float speedBase = .1f;
	float skyPos = 1.2f;
	float skyBoundX = 2, skyBoundZ = 2;

	partic color;

	goutte_eau eau[50];

	void initEau() {
		gravity = partic( 0, 0, -1 );
		wind = partic( 0.5f, 0.5f, 0 );

		color = partic( 0, 0, 1 );

		for( int i = 0; i != 50; ++i ) {
			eau[i].speed = speedBase + ( (float)rand() / (float)RAND_MAX * speedRand );
			eau[i].pos.y = skyPos;
			eau[i].pos.x = -( (float)rand() / (float)RAND_MAX * ( skyBoundX ) );
			eau[i].pos.z = -( (float)rand() / (float)RAND_MAX * ( skyBoundZ ) );
		}
	}

	void udateEau() {
		if( OpenGLWindow::temps == 2 ) { // automne
			color = partic( 0, 0, 1 );
			speedBase = .1f;
			speedRand = .05f;
			windForce = .2f;
		} else if( OpenGLWindow::temps == 3 ) { // hiver
			color = partic( 1, 1, 1 );
			speedBase = .01f;
			speedRand = .014f;
			windForce = .01f;
		} else {
			return;
		}

		for( int i = 0; i != 50; ++i ) {
			// gravity
			eau[i].pos.y += gravity.y * eau[i].speed;
			eau[i].pos.x += gravity.x * eau[i].speed;
			eau[i].pos.z += gravity.z * eau[i].speed;

			// wind
			eau[i].pos.y += wind.y * windForce;
			eau[i].pos.x += wind.x * windForce;
			eau[i].pos.z += wind.z * windForce;
			float groundHeight;

			if( eau[i].pos.x < 0 || eau[i].pos.y < 0 || eau[i].pos.x > 1.0f || eau[i].pos.y > 1.0f ) { // out of bounds
				groundHeight = 0.0f;
			} else {
				QRgb px = m_image.pixel( (int)( eau[i].pos.x * (float)m_image.width() ), (int)( eau[i].pos.y * (float)m_image.height() ) );

				groundHeight = 0.001f * (float)( qRed( px ) );
			}

			if( eau[i].pos.z <= groundHeight ) {
				eau[i].pos.z = skyPos;
				eau[i].pos.x = -( (float)rand() / (float)RAND_MAX * ( skyBoundX ) );
				eau[i].pos.y = -( (float)rand() / (float)RAND_MAX * ( skyBoundZ ) );
				eau[i].speed = speedBase + ( (float)rand( ) / (float)RAND_MAX * speedRand );
			}
		}
	}

};

TriangleWindow::TriangleWindow()
{
    QString s ("FPS : ");
    s += QString::number(1000/maj);
    s += "(";
    s += QString::number(maj);
    s += ")";
    setTitle(s);
    timer = new QTimer();
    timer->connect(timer, SIGNAL(timeout()),this, SLOT(renderNow()));
    timer->start(maj);
    master = true;

	OpenGLWindow::initServer();

	server_timer = new QTimer();
	server_timer->connect( server_timer, SIGNAL( timeout( ) ), this, SLOT( updateWeather( ) ) );
	server_timer->setInterval( 1000 /* 60 */ * 5 );
	server_timer->start( );

	initEau();
	thr.start();

}
TriangleWindow::TriangleWindow( int _maj )
{

    maj = _maj;
    QString s ("FPS : ");
    s += QString::number(1000/maj);
    s += "(";
    s += QString::number(maj);
    s += ")";
    setTitle(s);
    timer = new QTimer();
    timer->connect(timer, SIGNAL(timeout()),this, SLOT(renderNow()));
    timer->start(maj);

	OpenGLWindow::initClient();

	initEau();
	thr.start();
}
int main(int argc, char **argv)
{
    srand(time(NULL));
    QGuiApplication app(argc, argv);

    QSurfaceFormat format;
    format.setSamples(16);

    paramCamera* c=new paramCamera();

    TriangleWindow window;
    window.c = c;
    window.setFormat(format);
    window.resize(500,375);
    window.setPosition(0,0);
    window.show();

    TriangleWindow window2(100);
    window2.c = c;
    window2.setFormat(format);
    window2.resize(500,375);
    window2.setPosition(500, 0);
    window2.show();

    TriangleWindow window3(100);
    window3.c = c;
    window3.setFormat(format);
    window3.resize(500,375);
    window3.setPosition(0, 450);
    window3.show();

    TriangleWindow window4(100);
    window4.c = c;
    window4.setFormat(format);
    window4.resize(500,375);
    window4.setPosition(500,450);
    window4.show();

    return app.exec();
}


void TriangleWindow::initialize()
{
    const qreal retinaScale = devicePixelRatio();


    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -100.0, 100.0);


    loadMap(":/bureau256.png");

}

void TriangleWindow::loadMap(QString localPath)
{

    if (QFile::exists(localPath)) {
        m_image = QImage(localPath);
    }


    uint id = 0;
    p = new point[m_image.width() * m_image.height()];
    QRgb pixel;
    for(int i = 0; i < m_image.width(); i++)
    {
        for(int j = 0; j < m_image.height(); j++)
        {

            pixel = m_image.pixel(i,j);

            id = i*m_image.width() +j;

            p[id].x = (float)i/(m_image.width()) - ((float)m_image.width()/2.0)/m_image.width();
            p[id].y = (float)j/(m_image.height()) - ((float)m_image.height()/2.0)/m_image.height();
            p[id].z = 0.001f * (float)(qRed(pixel));
        }
    }
}

void TriangleWindow::render()
{

	if( OpenGLWindow::temps == 1 ) {
		glClearColor( 0.2f, 0.65f, 0.86f, 1.0f );
	} else {
		glClearColor( 0,0,0, 1 );
	}
    glClear(GL_COLOR_BUFFER_BIT);


    glLoadIdentity();
    glScalef(c->ss,c->ss,c->ss);

    glRotatef(c->rotX,1.0f,0.0f,0.0f);
    if(c->anim == 0.0f)
    {
        glRotatef(c->rotY,0.0f,0.0f,1.0f);
    }
    else
    {
        glRotatef(c->anim,0.0f,0.0f,1.0f);
        if(master)
            c->anim +=0.05f;
    }
    switch(c->etat)
    {
    case 0:
        displayPoints();
        break;
    case 1:
        displayLines();
        break;
    case 2:
        displayTriangles();
        break;
    case 3:
        displayTrianglesC();
        break;
    case 4:
        displayTrianglesTexture();
        break;
    case 5:

        displayTrianglesTexture();
        displayLines();
        break;
    default:
        displayPoints();
        break;
    }

	// particles
	if( OpenGLWindow::temps > 1 ) {
		glPointSize( 3 );
		udateEau();
		glColor3f( color.x, color.y, color.z );
		glBegin( GL_POINTS );
		for( int i = 0; i != 50; ++i ) {
			glVertex3f( eau[i].pos.x, eau[i].pos.y, eau[i].pos.z );
		}
		glEnd();
		glPointSize( 1 );
	}
    ++m_frame;
}

bool TriangleWindow::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::UpdateRequest:
        renderNow();
        return true;
    default:
        return QWindow::event(event);
    }
}

void TriangleWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case 'C':
        if(c->anim == 0.0f)
            c->anim = c->rotY;
        else
            c->anim = 0.0f;
        break;
    case 'Z':
        c->ss += 0.10f;
        break;
    case 'S':
        c->ss -= 0.10f;
        break;
    case 'A':
        c->rotX += 1.0f;
        break;
    case 'E':
        c->rotX -= 1.0f;
        break;
    case 'Q':
        c->rotY += 1.0f;
        break;
    case 'D':
        c->rotY -= 1.0f;
        break;
    case 'W':
        c->etat ++;
        if(c->etat > 5)
            c->etat = 0;
        break;
    case 'P':
        maj++;
        timer->stop();
        timer->start(maj);
        break;
    case 'O':
        maj--;
        if(maj < 1)
            maj = 1;
        timer->stop();
        timer->start(maj);
        break;
    case 'L':
        maj = maj - 20;
        if(maj < 1)
            maj = 1;
        timer->stop();
        timer->start(maj);
        break;
    case 'M':
        maj = maj + 20;

        timer->stop();
        timer->start(maj);
        break;
    case 'X':
        carte ++;
        if(carte > 3)
            carte = 1;
        QString depth (":/heightmap-");
        depth += QString::number(carte) ;
        depth += ".png" ;

        loadMap(depth);
        break;
    }
    QString s ("FPS : ");
    s += QString::number(1000/maj);
    s += "(";
    s += QString::number(maj);
    s += ")";
    setTitle(s);
}


void TriangleWindow::displayPoints()
{
    //glColor3f(1.0f, 1.0f, 1.0f);
	if( OpenGLWindow::temps == 0 ) {
		glColor3f( 1, 0.79f, 0.055f );
	} else if( OpenGLWindow::temps == 1 ) {
		glColor3f( 1, 0.79f, 0.055f );
	} else if( OpenGLWindow::temps == 2 ) {
		glColor3f( 0.93f, 0.37f, 0.125f );
	} else {
		glColor3f( 1.0f, 1.0f, 1.0f );
	}
    glBegin(GL_POINTS);
    uint id = 0;
    for(int i = 0; i < m_image.width(); i++)
    {
        for(int j = 0; j < m_image.height(); j++)
        {
            id = i*m_image.width() +j;
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);

        }
    }
    glEnd();
}


void TriangleWindow::displayTriangles()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    uint id = 0;

    for(int i = 0; i < m_image.width()-1; i++)
    {
        for(int j = 0; j < m_image.height()-1; j++)
        {

            id = i*m_image.width() +j;
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = i*m_image.width() +(j+1);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = (i+1)*m_image.width() +j;
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);



            id = i*m_image.width() +(j+1);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = (i+1)*m_image.width() +j+1;
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = (i+1)*m_image.width() +j;
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
        }
    }

    glEnd();
}

void TriangleWindow::displayTrianglesC()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    uint id = 0;

    for(int i = 0; i < m_image.width()-1; i++)
    {
        for(int j = 0; j < m_image.height()-1; j++)
        {
            glColor3f(0.0f, 1.0f, 0.0f);
            id = i*m_image.width() +j;
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = i*m_image.width() +(j+1);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = (i+1)*m_image.width() +j;
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);


            glColor3f(1.0f, 1.0f, 1.0f);
            id = i*m_image.width() +(j+1);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = (i+1)*m_image.width() +j+1;
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = (i+1)*m_image.width() +j;
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
        }
    }
    glEnd();
}


void TriangleWindow::displayLines()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    uint id = 0;

    for(int i = 0; i < m_image.width()-1; i++)
    {
        for(int j = 0; j < m_image.height()-1; j++)
        {

            id = i*m_image.width() +j;
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = i*m_image.width() +(j+1);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);

            id = (i+1)*m_image.width() +j;
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = i*m_image.width() +j;
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);

            id = (i+1)*m_image.width() +j;
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = i*m_image.width() +(j+1);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);

            id = i*m_image.width() +(j+1);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = (i+1)*m_image.width() +j+1;
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);

            id = (i+1)*m_image.width() +j+1;
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);

            id = (i+1)*m_image.width() +(j);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
        }
    }

    glEnd();
}

void TriangleWindow::displayTrianglesTexture()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    uint id = 0;

    for(int i = 0; i < m_image.width()-1; i++)
    {
        for(int j = 0; j < m_image.height()-1; j++)
        {

            id = i*m_image.width() +j;
            displayColor(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = i*m_image.width() +(j+1);
            displayColor(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = (i+1)*m_image.width() +j;
            displayColor(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);



            id = i*m_image.width() +(j+1);
            displayColor(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = (i+1)*m_image.width() +j+1;
            displayColor(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = (i+1)*m_image.width() +j;
            displayColor(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
        }
    }
    glEnd();
}


void TriangleWindow::displayColor(float alt)
{
    if (alt > 0.2)
    {
        glColor3f(01.0f, 1.0f, 1.0f);
    }
    else     if (alt > 0.1)
    {
        glColor3f(alt, 1.0f, 1.0f);
    }
    else     if (alt > 0.05f)
    {
        glColor3f(01.0f, alt, alt);
    }
    else
    {
        glColor3f(0.0f, 0.0f, 1.0f);
    }

}
