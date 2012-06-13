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
	strokeStateLabel = new QLabel();
	statusBar()->addPermanentWidget(canvasStateLabel);
	statusBar()->addPermanentWidget(strokeStateLabel);
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

	// Pen tool
	dock = new QDockWidget("Pen Tool", this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	penToolWidget = new PenToolWidget(dock);
	dock->setWidget(penToolWidget);
	addDockWidget(Qt::LeftDockWidgetArea, dock);
	viewMenu->addAction(dock->toggleViewAction());
	connect(this, SIGNAL(ResetDockWidgets()), penToolWidget, SLOT(OnReset()));

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
	connect(canvas, SIGNAL(StrokeStateChanged(int, int)), this, SLOT(OnStrokeStateChanged(int, int)));

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
	connect(canvasManipWidget, SIGNAL(ToggleGrid(int)), canvas, SLOT(OnToggleGrid(int)));
	connect(canvasManipWidget, SIGNAL(ToggleParticle(int)), canvas, SLOT(OnToggleParticle(int)));
	connect(canvasManipWidget, SIGNAL(ToggleStrokeLine(int)), canvas, SLOT(OnToggleStrokeLine(int)));
	connect(canvasManipWidget, SIGNAL(ToggleCurrentStrokeLine(int)), canvas, SLOT(OnToggleCurrentStrokeLine(int)));
	connect(canvasManipWidget, SIGNAL(ResetViewButtonClicked()), canvas, SLOT(OnResetViewButtonClicked()));
	connect(canvasManipWidget, SIGNAL(ToggleBackground(int)), canvas, SLOT(OnToggleBackground(int)));
	connect(canvasManipWidget, SIGNAL(ChangeBackgroundImage(QString)), canvas, SLOT(OnChangeBackgroundImage(QString)));

	// Embedding tool
	connect(embeddingToolWidget, SIGNAL(ToolChanged(int)), canvas, SLOT(OnToolChanged(int)));
	connect(embeddingToolWidget, SIGNAL(LevelChanged(double)), canvas, SLOT(OnLevelChanged(double)));
	connect(embeddingToolWidget, SIGNAL(LevelOffsetChanged(double)), canvas, SLOT(OnLevelOffsetChanged(double)));
	connect(embeddingToolWidget, SIGNAL(StrokeStepChanged(int)), canvas, SLOT(OnStrokeStepChanged(int)));

	// Pen tool
	connect(penToolWidget, SIGNAL(BrushColorChanged(QColor)), canvas, SLOT(OnBrushColorChanged(QColor)));
	connect(penToolWidget, SIGNAL(BrushChanged(int)), canvas, SLOT(OnBrushChanged(int)));
	connect(penToolWidget, SIGNAL(BrushSizeChanged(int)), canvas, SLOT(OnBrushSizeChanged(int)));
	connect(penToolWidget, SIGNAL(BrushOpacityChanged(int)), canvas, SLOT(OnBrushOpacityChanged(int)));
	connect(penToolWidget, SIGNAL(BrushSpacingChanged(double)), canvas, SLOT(OnBrushSpacingChanged(double)));

	SetEnabledDockWidgets(true);
	emit ResetDockWidgets();
}

void MainWindow::SetEnabledDockWidgets( bool enable )
{
	canvasManipWidget->setEnabled(enable);
	embeddingToolWidget->setEnabled(enable);
	penToolWidget->setEnabled(enable);
}

void MainWindow::OnStatusMessage( QString mes )
{
	statusBar()->showMessage(mes);
}

void MainWindow::OnStrokeStateChanged( int strokeNum, int particleNum )
{
	strokeStateLabel->setText(
		(boost::format("Stroke: %d Particle: %d") % strokeNum % particleNum).str().c_str());
}

// ------------------------------------------------------------

CanvasManipulatorWidget::CanvasManipulatorWidget( QWidget *parent /*= 0*/ )
	: QWidget(parent)
{
	// Canvas setting
	QGridLayout* gl1 = new QGridLayout;
	wireframeCheckBox = new QCheckBox("Wireframe");
	aabbCheckBox = new QCheckBox("AABB");
	gridCheckBox = new QCheckBox("Grid");
	particleCheckBox = new QCheckBox("Particle");
	strokeLineCheckBox = new QCheckBox("Stroke");
	currentStrokeLineCheckBox = new QCheckBox("Current Stroke");
	gl1->addWidget(wireframeCheckBox, 0, 0);
	gl1->addWidget(aabbCheckBox, 0, 1);
	gl1->addWidget(gridCheckBox, 1, 0);
	gl1->addWidget(particleCheckBox, 1, 1);
	gl1->addWidget(strokeLineCheckBox, 2, 0);
	gl1->addWidget(currentStrokeLineCheckBox, 2, 1);
	particleCheckBox->setChecked(true);
	currentStrokeLineCheckBox->setChecked(true);
	connect(wireframeCheckBox, SIGNAL(stateChanged(int)), this, SIGNAL(ToggleWireframe(int)));
	connect(aabbCheckBox, SIGNAL(stateChanged(int)), this, SIGNAL(ToggleAABB(int)));
	connect(gridCheckBox, SIGNAL(stateChanged(int)), this, SIGNAL(ToggleGrid(int)));
	connect(particleCheckBox, SIGNAL(stateChanged(int)), this, SIGNAL(ToggleParticle(int)));
	connect(strokeLineCheckBox, SIGNAL(stateChanged(int)), this, SIGNAL(ToggleStrokeLine(int)));
	connect(currentStrokeLineCheckBox, SIGNAL(stateChanged(int)), this, SIGNAL(ToggleCurrentStrokeLine(int)));

	// Reset view
	QPushButton* resetViewButton = new QPushButton("Reset View");
	connect(resetViewButton, SIGNAL(clicked()), this, SIGNAL(ResetViewButtonClicked()));

	// Background
	QHBoxLayout* hl1 = new QHBoxLayout;
	backgroundCheckBox = new QCheckBox("Background");
	findBackgroundImageButton = new QPushButton("...");
	findBackgroundImageButton->setFixedSize(20, 20);
	findBackgroundImageButton->setDisabled(true);
	hl1->addWidget(backgroundCheckBox);
	hl1->addStretch(0);
	hl1->addWidget(findBackgroundImageButton);
	connect(backgroundCheckBox, SIGNAL(stateChanged(int)), this, SLOT(stateChanged_BackgroundCheckBox(int)));
	connect(findBackgroundImageButton, SIGNAL(clicked()), this, SLOT(clicked_FindBackgroundImageButton()));

	// Main layout
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addLayout(gl1);
	layout->addLayout(hl1);
	layout->addWidget(resetViewButton);
	layout->addStretch(0);
	setLayout(layout);
}

QSize CanvasManipulatorWidget::minimumSizeHint() const
{
	return QSize(200, 200);
}

QSize CanvasManipulatorWidget::sizeHint() const
{
	return QSize(200, 200);
}

void CanvasManipulatorWidget::OnReset()
{
	emit ToggleWireframe(wireframeCheckBox->checkState());
	emit ToggleAABB(aabbCheckBox->checkState());
	emit ToggleGrid(gridCheckBox->checkState());
	emit ToggleParticle(particleCheckBox->checkState());
	emit ToggleStrokeLine(strokeLineCheckBox->checkState());
	emit ToggleCurrentStrokeLine(currentStrokeLineCheckBox->checkState());
	emit ToggleBackground(backgroundCheckBox->checkState());
	emit ChangeBackgroundImage(backgroundImagePath);
}

void CanvasManipulatorWidget::stateChanged_BackgroundCheckBox( int state )
{
	if (state == Qt::Checked)
	{
		findBackgroundImageButton->setEnabled(true);
	}
	else
	{
		findBackgroundImageButton->setDisabled(true);
	}
	emit ToggleBackground(state);
}

void CanvasManipulatorWidget::clicked_FindBackgroundImageButton()
{
	QFileDialog dialog;
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setNameFilter("Image (*.png)");
	dialog.setWindowTitle("Select a background image");
	if (dialog.exec())
	{
		backgroundImagePath = dialog.selectedFiles()[0];
		emit ChangeBackgroundImage(backgroundImagePath);
	}
}

// ------------------------------------------------------------

EmbeddingToolWidget::EmbeddingToolWidget( QWidget* parent /*= 0*/ )
	: QWidget(parent)
{
	minLevel = -10.0;
	maxLevel = 10.0;
	sliderValueOffset = 100.0;

	// Tool selection buttons
	QHBoxLayout* hl1 = new QHBoxLayout;
	levelRadioButton = new QRadioButton("Level");
	hairRadioButton = new QRadioButton("Hair");
	featherRadioButton = new QRadioButton("Feather");
	toolButtonGroup = new QButtonGroup;
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
	strokeStepSpinBox->setValue(3);
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
	emit ToolChanged(toolButtonGroup->checkedId());
	emit LevelChanged(levelSetSpinBox->value());
	emit LevelOffsetChanged(levelOffsetSpinBox->value());
	emit StrokeStepChanged(strokeStepSpinBox->value());
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

// ------------------------------------------------------------

FlatColorWidget::FlatColorWidget( QColor color, QWidget* parent /*= 0*/ )
	: QWidget(parent)
	, color(color)
{
	
}

void FlatColorWidget::paintEvent( QPaintEvent* event )
{
	QPainter painter(this);
	painter.setBrush(QBrush(color));
	painter.drawRect(0, 0, width(), height());
}

void FlatColorWidget::SetColor( QColor color )
{
	this->color = color;
	repaint();
}

// ------------------------------------------------------------

BrushRectItem::BrushRectItem( QRect rect, int id )
	: rect(rect)
	, id(id)
{

}

QRectF BrushRectItem::boundingRect() const
{
	return rect;
}

void BrushRectItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= 0 */ )
{
	painter->setPen(pen);
	painter->setBrush(brush);
	painter->drawRect(rect);
}

// ------------------------------------------------------------

BrushScene::BrushScene(int initialID)
	: currentID(initialID)
{

}

void BrushScene::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	QList<QGraphicsItem*> itemList = items(event->scenePos());
	if (itemList.empty())
	{
		event->ignore();
	}
	else
	{	
		BrushRectItem* item = dynamic_cast<BrushRectItem*>(itemList[0]);
		currentID = item->ID();
		emit BrushSelected(currentID);
		event->accept();
		QTimer::singleShot(10, this, SLOT(update()));
	}
}

void BrushScene::drawForeground( QPainter *painter, const QRectF &rect )
{
	painter->setPen(QPen(Qt::black));
	painter->setBrush(QBrush(Qt::transparent));
	int x = currentID % 4, y = currentID / 4;
	painter->drawRect(x * 30, y * 30, 30, 30);
}

// ------------------------------------------------------------

PenToolWidget::PenToolWidget( QWidget* parent /*= 0*/ )
	: QWidget(parent)
{
	// Current color
	colorDialog = new QColorDialog(QColor(0, 0, 0), this);
	QHBoxLayout* hl1 = new QHBoxLayout;
	QPushButton* colorSelectButton = new QPushButton("...");
	colorSelectButton->setFixedSize(20, 20);
	currentColor = new FlatColorWidget(colorDialog->currentColor());
	currentColor->setFixedSize(20, 20);
	hl1->addWidget(new QLabel("Color :"));
	hl1->addStretch(0);
	hl1->addWidget(currentColor);
	hl1->addWidget(colorSelectButton);
	connect(colorSelectButton, SIGNAL(clicked()), this, SLOT(clicked_ColorSelectButton()));

	// Brush view
	Util::Get()->CreateBrushPathList();
	QHBoxLayout* hl5 = new QHBoxLayout;
	brushScene = new BrushScene(0);
	for (int i = 0; i < Util::Get()->GetBrushNum(); i++)
	{
		int x = i % 4, y = i / 4;
		QPixmap pixmap(Util::Get()->GetBrushPath(i));
		pixmap = pixmap.scaled(QSize(30, 30));
		BrushRectItem* rectItem = new BrushRectItem(QRect(0, 0, 30, 30), i);
		rectItem->setPen(QPen(Qt::lightGray));
		rectItem->setBrush(QBrush(pixmap));
		rectItem->setPos(x * 30, y * 30);
		brushScene->addItem(rectItem);
	}
	brushView = new QGraphicsView;
	brushView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	brushView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	brushView->setScene(brushScene);
	brushView->setSceneRect(0, 0, 120, 120);
	brushView->setFixedSize(123, 123);
	hl5->addWidget(new QLabel("Brush :"));
	hl5->addStretch(0);
	hl5->addWidget(brushView);
	connect(brushScene, SIGNAL(BrushSelected(int)), this, SLOT(BrushSelected_BrushScene(int)));

	// Size
	QHBoxLayout* hl2 = new QHBoxLayout;
	sizeSpinBox = new QSpinBox;
	sizeSlider = new QSlider(Qt::Horizontal);
	sizeSpinBox->setMinimumWidth(75);
	sizeSpinBox->setRange(1, 50);
	sizeSpinBox->setValue(10);
	sizeSlider->setRange(sizeSpinBox->minimum(), sizeSpinBox->maximum());
	sizeSlider->setValue(sizeSpinBox->value());
	hl2->addWidget(new QLabel("Size :"));
	hl2->addStretch(0);
	hl2->addWidget(sizeSpinBox);
	connect(sizeSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(BrushSizeChanged(int)));
	connect(sizeSpinBox, SIGNAL(valueChanged(int)), sizeSlider, SLOT(setValue(int)));
	connect(sizeSlider, SIGNAL(valueChanged(int)), sizeSpinBox, SLOT(setValue(int)));

	// Opacity
	QHBoxLayout* hl3 = new QHBoxLayout;
	opacitySpinBox = new QSpinBox;
	opacitySlider = new QSlider(Qt::Horizontal);
	opacitySpinBox->setMinimumWidth(75);
	opacitySpinBox->setRange(0, 100);
	opacitySpinBox->setValue(100);
	opacitySpinBox->setSuffix("%");
	opacitySlider->setRange(opacitySpinBox->minimum(), opacitySpinBox->maximum());
	opacitySlider->setValue(opacitySpinBox->value());
	hl3->addWidget(new QLabel("Opacity :"));
	hl3->addStretch(0);
	hl3->addWidget(opacitySpinBox);
	connect(opacitySpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(BrushOpacityChanged(int)));
	connect(opacitySpinBox, SIGNAL(valueChanged(int)), opacitySlider, SLOT(setValue(int)));
	connect(opacitySlider, SIGNAL(valueChanged(int)), opacitySpinBox, SLOT(setValue(int)));

	// Spacing
	sliderValueOffset = 100;
	QHBoxLayout* hl4 = new QHBoxLayout;
	spacingSpinBox = new QDoubleSpinBox;
	spacingSlider = new QSlider(Qt::Horizontal);
	spacingSpinBox->setMinimumWidth(75);
	spacingSpinBox->setRange(0.01, 1.0);
	spacingSpinBox->setValue(0.5);
	spacingSpinBox->setSingleStep(0.01);
	spacingSlider->setRange(spacingSpinBox->minimum() * sliderValueOffset, spacingSpinBox->maximum() * sliderValueOffset);
	spacingSlider->setValue(spacingSpinBox->value() * sliderValueOffset);
	hl4->addWidget(new QLabel("Spacing :"));
	hl4->addStretch(0);
	hl4->addWidget(spacingSpinBox);
	connect(spacingSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(BrushSpacingChanged(double)));
	connect(spacingSpinBox, SIGNAL(valueChanged(double)), this, SLOT(valueChanged_SpacingSpinBox(double)));
	connect(spacingSlider, SIGNAL(valueChanged(int)), this, SLOT(valueChanged_SpacingSlider(int)));

	// Main layout
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addLayout(hl1);
	layout->addLayout(hl5);
	layout->addLayout(hl2);
	layout->addWidget(sizeSlider);
	layout->addLayout(hl3);
	layout->addWidget(opacitySlider);
	layout->addLayout(hl4);
	layout->addWidget(spacingSlider);
	layout->addStretch(0);
	setLayout(layout);
}

QSize PenToolWidget::minimumSizeHint() const
{
	return QSize(200, 500);
}

QSize PenToolWidget::sizeHint() const
{
	return QSize(200, 500);
}

void PenToolWidget::OnReset()
{
	emit BrushColorChanged(colorDialog->currentColor());
	emit BrushChanged(brushScene->CurrentID());
	emit BrushSizeChanged(sizeSpinBox->value());
	emit BrushOpacityChanged(opacitySpinBox->value());
	emit BrushSpacingChanged(spacingSpinBox->value());
}

void PenToolWidget::clicked_ColorSelectButton()
{
	if (!colorDialog->exec())
	{
		return;
	}
	colorDialog->setCurrentColor(colorDialog->selectedColor());
	currentColor->SetColor(colorDialog->currentColor());
	emit BrushColorChanged(colorDialog->currentColor());
}

void PenToolWidget::BrushSelected_BrushScene(int id)
{
	emit BrushChanged(id);
}

void PenToolWidget::valueChanged_SpacingSlider( int n )
{
	spacingSpinBox->setValue((double)n / sliderValueOffset);
}

void PenToolWidget::valueChanged_SpacingSpinBox( double d )
{
	spacingSlider->setValue((int)(d * sliderValueOffset));
}
