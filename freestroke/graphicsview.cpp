#include "graphicsview.h"
#include "gllib.h"
#include "timer.h"
#include "canvas.h"

GraphicsView::GraphicsView()
{

}

GLScene::GLScene()
	: timeSum(0.0)
	, frameCount(0)
	, prevTime(Timer::GetCurrentTimeMilli())
{
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		THROW_EXCEPTION(Exception::OpenGLError, (char*)glewGetErrorString(err));
	}
}

void GraphicsView::resizeEvent( QResizeEvent* event )
{
	if (scene())
	{
		// Set same scene rect as the graphics view.
		scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
		emit ResizeCanvas(event->size());
	}
}

QSize GraphicsView::minimumSizeHint() const
{
	return QSize(100, 100);
}

QSize GraphicsView::sizeHint() const
{
	return QSize(500, 500);
}

void GLScene::drawBackground( QPainter* painter, const QRectF& rect )
{
	emit DrawCanvas();
	UpdateFps();
	QTimer::singleShot(10, this, SLOT(update()));
}

void GLScene::keyPressEvent( QKeyEvent* event )
{
	emit KeyPressed(event);
}

void GLScene::keyReleaseEvent( QKeyEvent* event )
{
	emit KeyReleased(event);
}

void GLScene::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	emit MousePressed(event);
}

void GLScene::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
	emit MouseReleased(event);
}

void GLScene::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	emit MouseMoved(event);
}

void GLScene::wheelEvent( QGraphicsSceneWheelEvent* event )
{
	emit MouseWheeled(event);
}

void GLScene::UpdateFps()
{
	double currentTime = Timer::GetCurrentTimeMilli();
	double elapsedTime = currentTime - prevTime;
	prevTime = currentTime;

	// Calculate FPS
	timeSum += elapsedTime;
	frameCount++;
	if (frameCount >= 13)
	{
		emit UpdateFpsLabel(1000.0 * 13.0 / timeSum);
		timeSum = 0.0;
		frameCount = 0;
	}
}
