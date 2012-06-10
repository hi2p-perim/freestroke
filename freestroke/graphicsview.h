#ifndef __GRAPHICS_VIEW_H__
#define __GRAPHICS_VIEW_H__

#include <QGraphicsView>
#include <QGraphicsScene>

/*!
	Main graphics view.
	The graphics view which shows the canvas of the application.
*/
class GraphicsView : public QGraphicsView
{
	Q_OBJECT

public:

	GraphicsView();
	QSize minimumSizeHint() const;
	QSize sizeHint() const;

signals:

	void ResizeCanvas(QSize size);

protected:

	void resizeEvent(QResizeEvent* event);

};

class Canvas;

/*!
	GL scene.
	OpenGL scene which renders the canvas of the application.
*/
class GLScene : public QGraphicsScene
{
	Q_OBJECT

public:

	GLScene();
	void drawBackground(QPainter* painter, const QRectF& rect);

signals:

	void UpdateFpsLabel(float fps);
	void DrawCanvas();
	void KeyPressed(QKeyEvent* event);
	void KeyReleased(QKeyEvent* event);
	void MousePressed(QGraphicsSceneMouseEvent *event);
	void MouseReleased(QGraphicsSceneMouseEvent *event);
	void MouseMoved(QGraphicsSceneMouseEvent *event);
	void MouseWheeled(QGraphicsSceneWheelEvent* event);

protected:

	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void wheelEvent(QGraphicsSceneWheelEvent* event);

private:

	void UpdateFps();

private:

	// FPS calculation
	double prevTime;
	double timeSum;
	int frameCount;

};

#endif // __GRAPHICS_VIEW_H__