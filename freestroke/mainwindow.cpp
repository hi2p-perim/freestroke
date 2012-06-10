#include "mainwindow.h"
#include "graphicsview.h"
#include "canvas.h"
#include "util.h"
#include <QGLWidget>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
	, canvas(NULL)
{

	appTitle = "Freestroke";
	appVersion = "1.0.0";
	setWindowTitle(appTitle);

	// ------------------------------------------------------------

	//
	// Graphics view
	//
	
	QGLWidget* glWidget = new QGLWidget(QGLFormat(QGL::SampleBuffers));
	graphicsView = new GraphicsView;
	graphicsView->setViewport(glWidget);
	graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

	glWidget->makeCurrent();
	glscene = new GLScene;
	graphicsView->setScene(glscene);
	
	setCentralWidget(graphicsView);
	connect(glscene, SIGNAL(UpdateFpsLabel(float)), this, SLOT(OnUpdateFpsLabel(float)));

	// ------------------------------------------------------------

	CreateAction();
	CreateMenu();
	CreateToolBar();
	CreateStatusBar();
	CreateDockWidget();
}

MainWindow::~MainWindow()
{
	SAFE_DELETE(canvas);
}

void MainWindow::NewFile()
{
	if (canvas && canvas->IsModified())
	{
		int ret = SaveOrDiscardChanges();
		if (ret == QMessageBox::Cancel)
		{
			return;
		}
		SetEnabledDockWidgets(false);
		SAFE_DELETE(canvas);
	}

	// Load proxy geometry
	QFileDialog dialog;
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setNameFilter("OBJ Models (*.obj)");
	dialog.setWindowTitle("Select a proxy model");
	if (!dialog.exec())
	{
		return;
	}

	QString file = dialog.selectedFiles()[0];

	try
	{
		canvas = new Canvas(file, glscene->width(), glscene->height());
	}
	catch (const Exception& e)
	{
		if (e.Type() == Exception::FileError)
		{
			statusBar()->showMessage("Failed to load: " + QString(e.what()));
			return;
		}
		else
		{
			throw e;
		}
	}

	InitCanvas();
	statusBar()->showMessage("Created a new canvas with a proxy object " + file);
}

void MainWindow::OpenFile()
{
	statusBar()->showMessage("OpenFile");
}

void MainWindow::SaveFile()
{
	statusBar()->showMessage("SaveFile");
}

void MainWindow::About()
{
	statusBar()->showMessage("About");
}

void MainWindow::Undo()
{
	statusBar()->showMessage("Undo");
}

void MainWindow::OnUpdateFpsLabel(float fps)
{
	QString str;
	fpsLabel->setText(str.sprintf("FPS %.1f", fps));
}

void MainWindow::CreateAction()
{
	// File
	newFileAction = new QAction(QIcon(":/MainWindow/images/new.png"), "&New", this);
	newFileAction->setShortcuts(QKeySequence::New);
	newFileAction->setStatusTip("Create new canvas");
	connect(newFileAction, SIGNAL(triggered()), this, SLOT(NewFile()));

	openFileAction = new QAction(QIcon(":/MainWindow/images/open.png"), "&Open", this);
	openFileAction->setShortcuts(QKeySequence::Open);
	openFileAction->setStatusTip(tr("Open canvas"));
	connect(openFileAction, SIGNAL(triggered()), this, SLOT(OpenFile()));

	saveFileAction = new QAction(QIcon(":/MainWindow/images/save.png"), "&Save", this);
	saveFileAction->setShortcut(QKeySequence::Save);
	saveFileAction->setStatusTip("Save canvas");
	connect(saveFileAction, SIGNAL(triggered()), this, SLOT(SaveFile()));

	exitAction = new QAction("E&xit", this);
	exitAction->setShortcuts(QKeySequence::Quit);
	exitAction->setStatusTip("Exit the application");
	connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

	// Edit
	undoAction = new QAction(QIcon(":/MainWindow/images/undo.png"), "&Undo", this);
	undoAction->setShortcut(QKeySequence::Undo);
	undoAction->setStatusTip("Undo");
	connect(undoAction, SIGNAL(triggered()), this, SLOT(Undo()));

	// Help
	aboutAction = new QAction("&About", this);
	aboutAction->setStatusTip("About the application");
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(About()));
}

void MainWindow::CreateMenu()
{
	// File
	fileMenu = menuBar()->addMenu("&File");
	fileMenu->addAction(newFileAction);
	fileMenu->addAction(openFileAction);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAction);

	// Edit
	editMenu = menuBar()->addMenu("&Edit");
	editMenu->addAction(undoAction);

	// View
	viewMenu = menuBar()->addMenu("&View");

	// Help
	helpMenu = menuBar()->addMenu("&Help");
	helpMenu->addAction(aboutAction);
}

void MainWindow::CreateToolBar()
{
	// File
	fileToolBar = addToolBar("File");
	fileToolBar->addAction(newFileAction);
	fileToolBar->addAction(openFileAction);
	fileToolBar->addAction(saveFileAction);

	// Edit
	editToolBar = addToolBar("Edit");
	editToolBar->addAction(undoAction);
}

void MainWindow::CreateStatusBar()
{
	fpsLabel = new QLabel();
	canvasStateLabel = new QLabel();
	statusBar()->addPermanentWidget(canvasStateLabel);
	statusBar()->addPermanentWidget(fpsLabel);
	statusBar()->showMessage("Ready");
	connect(Util::Get(), SIGNAL(StatusMessage(QString)), this, SLOT(OnStatusMessage(QString)));
}

void MainWindow::CreateDockWidget()
{
	QDockWidget* dock;

	// Canvas manipulator
	dock = new QDockWidget("Canvas Manipulator", this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	canvasManipWidget = new CanvasManipulatorWidget(dock);
	dock->setWidget(canvasManipWidget);
	addDockWidget(Qt::RightDockWidgetArea, dock);
	viewMenu->addAction(dock->toggleViewAction());
	connect(this, SIGNAL(ResetDockWidgets()), canvasManipWidget, SLOT(OnReset()));

	// Embedding tool
	dock = new QDockWidget("Embedding Tool", this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	embeddingToolWidget = new EmbeddingToolWidget(dock);
	dock->setWidget(embeddingToolWidget);
	addDockWidget(Qt::RightDockWidgetArea, dock);
	viewMenu->addAction(dock->toggleViewAction());
	connect(this, SIGNAL(ResetDockWidgets()), embeddingToolWidget, SLOT(OnReset()));

	SetEnabledDockWidgets(false);
}

void MainWindow::OnCanvasStateChanged( unsigned int state )
{
	QString str;
	if (state == Canvas::STATE_IDLE) str = "Idle";
	else if (state == Canvas::STATE_STROKING) str = "Stroking";
	else if (state == Canvas::STATE_ROTATING) str = "Rotating";
	else if (state == Canvas::STATE_TRANSLATING) str = "Translating";
	canvasStateLabel->setText(str);
}

void MainWindow::closeEvent( QCloseEvent* event )
{
	if (canvas && canvas->IsModified())
	{
		int ret = SaveOrDiscardChanges();
		if (ret == QMessageBox::Cancel)
		{
			event->ignore();
		}
		else
		{
			event->accept();
		}
	}
}

int MainWindow::SaveOrDiscardChanges()
{
	QMessageBox msgBox;
	msgBox.setWindowTitle(appTitle);
	msgBox.setText("The canvas has been modified.");
	msgBox.setInformativeText("Do you want to save your changes?");
	msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Save);
	msgBox.setIcon(QMessageBox::Question);
	int ret = msgBox.exec();
	if (ret == QMessageBox::Save)
	{
		SaveFile();
	}
	return ret;
}

void MainWindow::InitCanvas()
{
	// View size changed
	connect(graphicsView, SIGNAL(ResizeCanvas(QSize)), canvas, SLOT(OnResizeCanvas(QSize)));

	// Canvas state changed
	connect(canvas, SIGNAL(StateChanged(unsigned int)), this, SLOT(OnCanvasStateChanged(unsigned int)));
	
	// Draw signal and event signals
	connect(glscene, SIGNAL(DrawCanvas()), canvas, SLOT(OnDraw()));
	connect(glscene, SIGNAL(KeyPressed(QKeyEvent*)), canvas, SLOT(OnKeyPressed(QKeyEvent*)));
	connect(glscene, SIGNAL(KeyReleased(QKeyEvent*)), canvas, SLOT(OnKeyReleased(QKeyEvent*)));
	connect(glscene, SIGNAL(MousePressed(QGraphicsSceneMouseEvent*)), canvas, SLOT(OnMousePressed(QGraphicsSceneMouseEvent*)));
	connect(glscene, SIGNAL(MouseReleased(QGraphicsSceneMouseEvent*)), canvas, SLOT(OnMouseReleased(QGraphicsSceneMouseEvent*)));
	connect(glscene, SIGNAL(MouseMoved(QGraphicsSceneMouseEvent*)), canvas, SLOT(OnMouseMoved(QGraphicsSceneMouseEvent*)));
	connect(glscene, SIGNAL(MouseWheeled(QGraphicsSceneWheelEvent*)), canvas, SLOT(OnWheelEvent(QGraphicsSceneWheelEvent*)));
	
	// Canvas manipulation
	connect(canvasManipWidget, SIGNAL(ToggleWireframe(int)), canvas, SLOT(OnToggleWireframe(int)));
	connect(canvasManipWidget, SIGNAL(ToggleAABB(int)), canvas, SLOT(OnToggleAABB(int)));

	// Embedding tool
	connect(embeddingToolWidget, SIGNAL(ToolChanged(int)), canvas, SLOT(OnToolChanged(int)));
	connect(embeddingToolWidget, SIGNAL(LevelChanged(double)), canvas, SLOT(OnLevelChanged(double)));
	connect(embeddingToolWidget, SIGNAL(LevelOffsetChanged(double)), canvas, SLOT(OnLevelOffsetChanged(double)));
	connect(embeddingToolWidget, SIGNAL(StrokeStepChanged(int)), canvas, SLOT(OnStrokeStepChanged(int)));

	SetEnabledDockWidgets(true);
	emit ResetDockWidgets();
}

void MainWindow::SetEnabledDockWidgets( bool enable )
{
	canvasManipWidget->setEnabled(enable);
	embeddingToolWidget->setEnabled(enable);
}

void MainWindow::OnStatusMessage( QString mes )
{
	statusBar()->showMessage(mes);
}

// ------------------------------------------------------------

CanvasManipulatorWidget::CanvasManipulatorWidget( QWidget *parent /*= 0*/ )
	: QWidget(parent)
{
	wireframeCheckBox = new QCheckBox("Wireframe");
	connect(wireframeCheckBox, SIGNAL(stateChanged(int)), this, SIGNAL(ToggleWireframe(int)));
	aabbCheckBox = new QCheckBox("AABB");
	connect(aabbCheckBox, SIGNAL(stateChanged(int)), this, SIGNAL(ToggleAABB(int)));

	// Main layout
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(wireframeCheckBox);
	layout->addWidget(aabbCheckBox);
	layout->addStretch(0);
	setLayout(layout);
}

QSize CanvasManipulatorWidget::minimumSizeHint() const
{
	return QSize(200, 100);
}

QSize CanvasManipulatorWidget::sizeHint() const
{
	return QSize(200, 100);
}

void CanvasManipulatorWidget::OnReset()
{
	wireframeCheckBox->setCheckState(Qt::Unchecked);
	emit ToggleWireframe(Qt::Unchecked);
	aabbCheckBox->setCheckState(Qt::Unchecked);
	emit ToggleAABB(Qt::Unchecked);
}

// ------------------------------------------------------------

EmbeddingToolWidget::EmbeddingToolWidget( QWidget* parent /*= 0*/ )
{
	minLevel = -10.0;
	maxLevel = 10.0;
	sliderValueOffset = 100.0;

	// Tool selection buttons
	levelRadioButton = new QRadioButton("Level");
	hairRadioButton = new QRadioButton("Hair");
	featherRadioButton = new QRadioButton("Feather");
	toolButtonGroup = new QButtonGroup;
	QHBoxLayout* hl1 = new QHBoxLayout;
	toolButtonGroup->addButton(levelRadioButton);
	toolButtonGroup->setId(levelRadioButton, 0);
	toolButtonGroup->addButton(hairRadioButton);
	toolButtonGroup->setId(hairRadioButton, 1);
	toolButtonGroup->addButton(featherRadioButton);
	toolButtonGroup->setId(featherRadioButton, 2);
	levelRadioButton->setChecked(true);
	hl1->addWidget(levelRadioButton);
	hl1->addWidget(hairRadioButton);
	hl1->addWidget(featherRadioButton);
	connect(toolButtonGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(ToolChanged(int)));

	// Level setting
	QHBoxLayout* hl2 = new QHBoxLayout;
	levelSetSpinBox = new QDoubleSpinBox;
	levelSetSlider = new QSlider(Qt::Horizontal);
	levelSetSpinBox->setMinimumWidth(75);
	levelSetSpinBox->setSingleStep(0.01);
	levelSetSpinBox->setRange(minLevel, maxLevel);
	levelSetSpinBox->setValue(0.0);
	levelSetSlider->setRange(minLevel * sliderValueOffset, maxLevel * sliderValueOffset);
	levelSetSlider->setValue(0.0);
	hl2->addWidget(new QLabel("Level :"));
	hl2->addStretch(0);
	hl2->addWidget(levelSetSpinBox);
	connect(levelSetSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(LevelChanged(double)));
	connect(levelSetSpinBox, SIGNAL(valueChanged(double)), this, SLOT(valueChanged_LevelSetSpinBox(double)));
	connect(levelSetSlider, SIGNAL(valueChanged(int)), this, SLOT(valueChanged_LevelSetSlider(int)));

	// Level offset setting
	QHBoxLayout* hl3 = new QHBoxLayout;
	levelOffsetSpinBox = new QDoubleSpinBox;
	levelOffsetSlider = new QSlider(Qt::Horizontal);
	levelOffsetSpinBox->setMinimumWidth(75);
	levelOffsetSpinBox->setSingleStep(0.01);
	levelOffsetSpinBox->setRange(minLevel, maxLevel);
	levelOffsetSpinBox->setValue(0.0);
	levelOffsetSlider->setRange(minLevel * sliderValueOffset, maxLevel * sliderValueOffset);
	levelOffsetSlider->setValue(0.0);
	hl3->addWidget(new QLabel("Level Offset :"));
	hl3->addStretch(0);
	hl3->addWidget(levelOffsetSpinBox);
	connect(levelOffsetSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(LevelOffsetChanged(double)));
	connect(levelOffsetSpinBox, SIGNAL(valueChanged(double)), this, SLOT(valueChanged_LevelOffsetSpinBox(double)));
	connect(levelOffsetSlider, SIGNAL(valueChanged(int)), this, SLOT(valueChanged_LevelOffsetSlider(int)));

	// Stroke step
	QHBoxLayout* hl4 = new QHBoxLayout;
	strokeStepSpinBox = new QSpinBox;
	strokeStepSlider = new QSlider(Qt::Horizontal);
	strokeStepSpinBox->setMinimumWidth(75);
	strokeStepSpinBox->setRange(0, 10);
	strokeStepSpinBox->setValue(5);
	strokeStepSlider->setRange(strokeStepSpinBox->minimum(), strokeStepSpinBox->maximum());
	strokeStepSlider->setValue(strokeStepSpinBox->value());
	hl4->addWidget(new QLabel("Stroke Step :"));
	hl4->addStretch(0);
	hl4->addWidget(strokeStepSpinBox);
	connect(strokeStepSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(StrokeStepChanged(int)));
	connect(strokeStepSpinBox, SIGNAL(valueChanged(int)), strokeStepSlider, SLOT(setValue(int)));
	connect(strokeStepSlider, SIGNAL(valueChanged(int)), strokeStepSpinBox, SLOT(setValue(int)));

	// Main layout
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addLayout(hl1);
	layout->addLayout(hl2);
	layout->addWidget(levelSetSlider);
	layout->addLayout(hl3);
	layout->addWidget(levelOffsetSlider);
	layout->addLayout(hl4);
	layout->addWidget(strokeStepSlider);
	layout->addStretch(0);
	setLayout(layout);
}

QSize EmbeddingToolWidget::minimumSizeHint() const
{
	return QSize(200, 300);
}

QSize EmbeddingToolWidget::sizeHint() const
{
	return QSize(200, 300);
}

void EmbeddingToolWidget::OnReset()
{
	levelRadioButton->setChecked(true);
	emit ToolChanged(toolButtonGroup->id(levelRadioButton));
	levelSetSpinBox->setValue(0.0);
	emit LevelChanged(0.0);
	levelOffsetSpinBox->setValue(0.0);
	emit LevelOffsetChanged(0.0);
	strokeStepSpinBox->setValue(5);
	emit StrokeStepChanged(5);
}

void EmbeddingToolWidget::valueChanged_LevelSetSlider( int n )
{
	levelSetSpinBox->setValue((double)n / sliderValueOffset);
}

void EmbeddingToolWidget::valueChanged_LevelOffsetSlider( int n )
{
	levelOffsetSpinBox->setValue((double)n / sliderValueOffset);
}

void EmbeddingToolWidget::valueChanged_LevelSetSpinBox( double d )
{
	levelSetSlider->setValue((int)(d * sliderValueOffset));
}

void EmbeddingToolWidget::valueChanged_LevelOffsetSpinBox( double d )
{
	levelOffsetSlider->setValue((int)(d * sliderValueOffset));
}
