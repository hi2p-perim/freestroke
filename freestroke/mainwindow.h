#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

class GraphicsView;
class GLScene;
class Canvas;

class CanvasManipulatorWidget;
class EmbeddingToolWidget;
class PenToolWidget;

/*!
	Main window.
	The main window of the application.
*/
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:

	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MainWindow();

private slots:

	void NewFile();
	void OpenFile();
	void SaveFile();
	void About();
	void Undo();
	void OnUpdateFpsLabel(float fps);
	void OnCanvasStateChanged(unsigned int state);
	void OnStatusMessage(QString mes);
	void OnStrokeStateChanged(int strokeNum, int particleNum);

signals:

	void ResetDockWidgets();
	void UndoStroke();

protected:

	void closeEvent(QCloseEvent* event);

private:

	void InitCanvas();
	int SaveOrDiscardChanges();
	void CreateAction();
	void CreateMenu();
	void CreateToolBar();
	void CreateStatusBar();
	void CreateDockWidget();
	void SetEnabledDockWidgets(bool enable);

private:

	QAction* newFileAction;
	QAction* openFileAction;
	QAction* saveFileAction;
	QAction* exitAction;
	QAction* undoAction;
	QAction* aboutAction;
	QMenu* fileMenu;
	QMenu* editMenu;
	QMenu* viewMenu;
	QMenu* helpMenu;
	QToolBar* fileToolBar;
	QToolBar* editToolBar;

	// Status bar info
	QLabel* fpsLabel;
	QLabel* canvasStateLabel;
	QLabel* strokeStateLabel;

	// Dock widgets
	CanvasManipulatorWidget* canvasManipWidget;
	EmbeddingToolWidget* embeddingToolWidget;
	PenToolWidget* penToolWidget;

private:

	QString appTitle;
	QString appVersion;

	GraphicsView* graphicsView;
	GLScene* glscene;
	Canvas* canvas;

};

/*!
	Canvas manipulation tool widget.
	The widget shows the manipulation of the canvas.
*/
class CanvasManipulatorWidget : public QWidget
{
	Q_OBJECT

public:

	CanvasManipulatorWidget(QWidget *parent = 0);
	QSize minimumSizeHint() const;
	QSize sizeHint() const;

public slots:

	void OnReset();
	void stateChanged_BackgroundCheckBox(int state);
	void clicked_FindBackgroundImageButton();

signals:

	void ToggleWireframe(int state);
	void ToggleAABB(int state);
	void ToggleGrid(int state);
	void ToggleParticle(int state);
	void ToggleStrokeLine(int state);
	void ToggleCurrentStrokeLine(int state);
	void ToggleProxyObjectCheckBox(int state);
	void ResetViewButtonClicked();
	void ToggleBackground(int state);
	void ChangeBackgroundImage(QString path);

private:

	QCheckBox* wireframeCheckBox;
	QCheckBox* aabbCheckBox;
	QCheckBox* gridCheckBox;
	QCheckBox* particleCheckBox;
	QCheckBox* strokeLineCheckBox;
	QCheckBox* currentStrokeLineCheckBox;
	QCheckBox* proxyObjectCheckBox;

	QCheckBox* backgroundCheckBox;
	QPushButton* findBackgroundImageButton;
	QString backgroundImagePath;

};

/*!
	Embedding tool widget.
	Configuration of the stroke embedding tools.
*/
class EmbeddingToolWidget : public QWidget
{
	Q_OBJECT

public:

	EmbeddingToolWidget(QWidget* parent = 0);
	QSize minimumSizeHint() const;
	QSize sizeHint() const;

public slots:

	void OnReset();
	void valueChanged_LevelSetSlider(int n);
	void valueChanged_LevelOffsetSlider(int n);
	void valueChanged_LevelSetSpinBox(double d);
	void valueChanged_LevelOffsetSpinBox(double d);

signals:

	void ToolChanged(int id);
	void LevelChanged(double level);
	void LevelOffsetChanged(double level);
	void StrokeStepChanged(int step);

private:

	double minLevel, maxLevel;
	double sliderValueOffset;

	QButtonGroup* toolButtonGroup;
	QRadioButton* levelRadioButton;
	QRadioButton* hairRadioButton;
	QRadioButton* featherRadioButton;
	
	QDoubleSpinBox* levelSetSpinBox;
	QDoubleSpinBox* levelOffsetSpinBox;
	QSpinBox* strokeStepSpinBox;
	QSlider* levelSetSlider;
	QSlider* levelOffsetSlider;
	QSlider* strokeStepSlider;

};

/*!
	Flat color widget.
	The widget is used for displaying current selected color.
*/
class FlatColorWidget : public QWidget
{
	Q_OBJECT

public:

	FlatColorWidget(QColor color, QWidget* parent = 0);

public slots:

	void SetColor(QColor color);

protected:

	void paintEvent(QPaintEvent* event);

private:

	QColor color;

};

/*!
	Brush rect item.
	The class is the brush item for the graphics view
	in the pen tool widget.
*/
class BrushRectItem : public QGraphicsItem
{
public:

	BrushRectItem(QRect rect, int id);
	QRectF boundingRect() const;
	void setPen(QPen pen) { this->pen = pen; }
	void setBrush(QBrush brush) { this->brush = brush; }
	int ID() { return id; }

protected:

	void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0 );

private:

	QPen pen;
	QBrush brush;
	QRect rect;
	int id;

};

/*!
	Brush scene.
	The class is the brush selection scene for the graphics view
	in the pen tool widget.
*/
class BrushScene : public QGraphicsScene
{
	Q_OBJECT

public:

	BrushScene(int initialID);
	int CurrentID() { return currentID; }

signals:

	void BrushSelected(int id);

protected:

	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void drawForeground( QPainter *painter, const QRectF &rect );

private:

	int currentID;

};

/*!
	Pen tool widget.
	Configuration of the stroke rendering.
*/
class PenToolWidget : public QWidget
{
	Q_OBJECT

public:

	PenToolWidget(QWidget* parent = 0);
	QSize minimumSizeHint() const;
	QSize sizeHint() const;

public slots:

	void OnReset();
	void clicked_ColorSelectButton();
	void BrushSelected_BrushScene(int id);
	void valueChanged_SpacingSlider(int n);
	void valueChanged_SpacingSpinBox(double d);

signals:

	void BrushColorChanged(QColor color);
	void BrushChanged(int id);
	void BrushSizeChanged(int size);
	void BrushOpacityChanged(int opacity);
	void BrushSpacingChanged(double spacing);

private:

	int sliderValueOffset;

	QColorDialog* colorDialog;
	FlatColorWidget* currentColor;

	QGraphicsView* brushView;
	BrushScene* brushScene;

	QSpinBox* sizeSpinBox;
	QSpinBox* opacitySpinBox;
	QDoubleSpinBox* spacingSpinBox;
	QSlider* sizeSlider;
	QSlider* opacitySlider;
	QSlider* spacingSlider;

};

#endif // __MAIN_WINDOW_H__
