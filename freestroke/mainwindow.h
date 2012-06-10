#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

class GraphicsView;
class GLScene;
class Canvas;

class CanvasManipulatorWidget;
class EmbeddingToolWidget;

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

signals:

	void ResetDockWidgets();

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

	// Dock widgets
	CanvasManipulatorWidget* canvasManipWidget;
	EmbeddingToolWidget* embeddingToolWidget;

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

signals:

	void ToggleWireframe(int state);
	void ToggleAABB(int state);

private:

	QCheckBox* wireframeCheckBox;
	QCheckBox* aabbCheckBox;

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

#endif // __MAIN_WINDOW_H__
