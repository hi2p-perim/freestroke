#include "canvas.h"
#include "model.h"
#include "timer.h"
#include "gllib.h"
#include "util.h"
#include <QGraphicsScene>
#include <QGLWidget>
#include <lbfgs.h>

Canvas::Canvas(QString proxyGeometryPath, int width, int height)
	: state(STATE_IDLE)
	, proxyGeometryPath(proxyGeometryPath)
	, modified(false)
	, scale(1.0f)
	, canvasWidth(width)
	, canvasHeight(height)
	, trans(0.0f)
	, backgroundTexture(NULL)
{
	// Load model
	proxyModel = new ObjModel(proxyGeometryPath.toStdString(), 100.0f);
	quad = new QuadMesh;

	// Create shaders
	renderShader = new GlslShader;
	renderShader->AddShader(GlslShader::VERTEX_SHADER, "./resources/render.vert");
	renderShader->AddShader(GlslShader::FRAGMENT_SHADER, "./resources/render.frag");
	renderShader->BindAttribute(VertexStream::POSITION, "position");
	renderShader->BindAttribute(VertexStream::NORMAL, "normal");
	renderShader->Initialize();

	flatShader = new GlslShader;
	flatShader->AddShader(GlslShader::VERTEX_SHADER, "./resources/flat.vert");
	flatShader->AddShader(GlslShader::FRAGMENT_SHADER, "./resources/flat.frag");
	flatShader->BindAttribute(VertexStream::POSITION, "position");
	flatShader->Initialize();

	flatTexShader = new GlslShader;
	flatTexShader->AddShader(GlslShader::VERTEX_SHADER, "./resources/flattex.vert");
	flatTexShader->AddShader(GlslShader::FRAGMENT_SHADER, "./resources/flattex.frag");
	flatTexShader->BindAttribute(VertexStream::POSITION, "position");
	flatTexShader->BindAttribute(VertexStream::TEXCOORD0, "texcoord");
	flatTexShader->Initialize();

	strokePointShader = new GlslShader;
	strokePointShader->AddShader(GlslShader::VERTEX_SHADER, "./resources/strokepoint.vert");
	strokePointShader->AddShader(GlslShader::GEOMETRY_SHADER, "./resources/strokepoint.geom");
	strokePointShader->AddShader(GlslShader::FRAGMENT_SHADER, "./resources/strokepoint.frag");
	strokePointShader->BindAttribute(0, "position");
	strokePointShader->BindAttribute(1, "color");
	strokePointShader->BindAttribute(2, "id");
	strokePointShader->BindAttribute(3, "size");
	strokePointShader->Initialize();

	// Load brush textures
	LoadBrushTexture();

	// Camera params
	fov = 45.0f;
	nearClip = 0.01f;
	farClip = 1000.0f;
}

Canvas::~Canvas()
{
	SAFE_DELETE(brushTextures);
	for (int i = 0; i < strokeList.size(); i++)
	{
		SAFE_DELETE(strokeList[i]);
	}
	SAFE_DELETE(renderShader);
	SAFE_DELETE(flatShader);
	SAFE_DELETE(quad);
	SAFE_DELETE(proxyModel);
}

void Canvas::LoadBrushTexture()
{
	float size = Util::Get()->GetBrushSize();
	int n = Util::Get()->GetBrushNum();
	brushTextures = new Texture2DArray(size, size, n, GL_RGBA8, GL_RGBA, GL_REPEAT, GL_LINEAR, GL_LINEAR);
	for (int i = 0; i < n; i++)
	{
		QString path = Util::Get()->GetBrushPath(i);
		QImage image;
		if (!image.load(path))
		{
			THROW_EXCEPTION(Exception::FileError,
				("Failed to load brush image: " + path).toStdString());
		}

		QImage image2(image.width(), image.height(), QImage::Format_ARGB32);
		image2.fill(Qt::transparent);
		QPainter painter(&image2);
		painter.drawImage(0, 0, image);

		QImage glimage = QGLWidget::convertToGLFormat(image2);
		brushTextures->Substitute(i, GL_RGBA, glimage.bits());
	}
}

void Canvas::OnKeyPressed( QKeyEvent* event )
{
	event->ignore();
}

void Canvas::OnKeyReleased( QKeyEvent* event )
{
	event->ignore();
}

void Canvas::OnMouseMoved( QGraphicsSceneMouseEvent* event )
{
	if (state == STATE_STROKING)
	{
		if (currentStrokeSteps == strokeSteps)
		{
			currentStrokeSteps = 0;
			currentStrokePoints.push_back(StrokePoint(
				glm::vec3((float)event->scenePos().x(), canvasHeight - (float)event->scenePos().y(), 0.0f),
				glm::vec4(brushColor, brushOpacity),
				brushID, brushSize, strokeList.size()));
		}
		else
		{
			currentStrokeSteps++;
		}
		event->accept();
	}
	else if (state == STATE_ROTATING)
	{
		int deltax = event->scenePos().x() - startx;
		int deltay = event->scenePos().y() - starty;
		glm::vec3 axis(
			360.0f * deltay / (float)canvasHeight,
			360.0f * deltax / (float)canvasWidth,
			0.0f);
		float rot = glm::length(axis);
		glm::normalize(axis);
		currentQuat = glm::cross(glm::rotate(glm::quat(), rot, axis), startQuat);
		event->accept();
	}
	else if (state == STATE_TRANSLATING)
	{
		float deltax = (float)(event->scenePos().x() - event->lastScenePos().x()) / canvasWidth * 50.0f;
		float deltay = (float)(event->scenePos().y() - event->lastScenePos().y()) / canvasHeight * 50.0f;
		trans += glm::vec3(deltax, -deltay, 0.0f);
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void Canvas::OnMousePressed( QGraphicsSceneMouseEvent* event )
{
	if (event->button() == Qt::LeftButton)
	{
		if (state == STATE_IDLE)
		{
			ChangeState(STATE_STROKING);
			currentStrokeSteps = strokeSteps; // First stroke point is the clicked point
			event->accept();
			return;
		}
	}
	else if (event->button() == Qt::RightButton)
	{
		if (state == STATE_IDLE)
		{
			ChangeState(STATE_ROTATING);
			startx = event->scenePos().x();
			starty = event->scenePos().y();
			startQuat = currentQuat;
			event->accept();
			return;
		}
	}
	else if (event->button() == Qt::MiddleButton)
	{
		if (state == STATE_IDLE)
		{
			ChangeState(STATE_TRANSLATING);
			startx = event->scenePos().x();
			starty = event->scenePos().y();
			event->accept();
			return;
		}
	}
	event->ignore();
}

void Canvas::OnMouseReleased( QGraphicsSceneMouseEvent* event )
{
	if (event->button() == Qt::LeftButton)
	{
		if (state == STATE_STROKING)
		{
			if (currentStrokePoints.size() >= 2)
			{
				Stroke* stroke = new Stroke(this, brushSpacing, camWorldPos);
				if (stroke->Embed(currentStrokePoints))
				{
					strokeList.push_back(stroke);
				}
				else
				{
					SAFE_DELETE(stroke);
				}
			}
			else
			{
				Util::Get()->ShowStatusMessage("Number of stroke points must be larger than 1");
			}
			currentStrokePoints.clear();
			ChangeState(STATE_IDLE);
			event->accept();
			return;
		}
	}
	else if (event->button() == Qt::RightButton)
	{
		if (state == STATE_ROTATING)
		{
			ChangeState(STATE_IDLE);
			event->accept();
			return;
		}
	}
	else if (event->button() == Qt::MiddleButton)
	{
		if (state == STATE_TRANSLATING)
		{
			ChangeState(STATE_IDLE);
			event->accept();
			return;
		}
	}
	event->ignore();
}

void Canvas::OnWheelEvent( QGraphicsSceneWheelEvent* event )
{
	if (event->orientation() == Qt::Vertical)
	{
		scale += event->delta() / 360.0f / 10.0f;
		if (scale < 0.001f) scale = 0.001f;
		event->accept();
	}
	event->ignore();
}

void Canvas::ChangeState( State nextState )
{
	state = nextState;
	emit StateChanged((unsigned int)nextState);
}

void Canvas::OnDraw()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (enableBackgroundTexture)
	{
		DrawBackground();
	}

	// ------------------------------------------------------------

	const float base = 200.0f;
	projectionMatrix = glm::perspective(fov, (float)canvasWidth / canvasHeight, nearClip, farClip);
	viewMatrix =
		glm::lookAt(glm::vec3(0.0f, 0.0f, base * scale), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::translate(glm::mat4(1.0f), trans) * 
		glm::mat4_cast(currentQuat);
	
	modelMatrix = glm::mat4(1.0f);
	mvMatrix = viewMatrix * modelMatrix;
	mvpMatrix = projectionMatrix * mvMatrix;

	// Camera position in the world space
	glm::mat4 mvMatrixInv = glm::inverse(mvMatrix);
	camWorldPos = glm::vec3(mvMatrixInv * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	// Camera basis in the world space
	glm::mat3 mvMatrixInv3(mvMatrixInv);
	camWorldU = glm::vec3(mvMatrixInv3 * glm::vec3(1.0f, 0.0f, 0.0f));
	camWorldV = glm::vec3(mvMatrixInv3 * glm::vec3(0.0f, 1.0f, 0.0f));
	camWorldW = glm::vec3(mvMatrixInv3 * glm::vec3(0.0f, 0.0f, 1.0f));

	// ------------------------------------------------------------

	//
	// Proxy model rendering
	//

	if (enableWireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	renderShader->Begin();
	renderShader->SetUniformMatrix4f("mvpMatrix", mvpMatrix);
	renderShader->SetUniformMatrix3f("normalMatrix", glm::mat3(glm::transpose(glm::inverse(mvMatrix))));
	renderShader->SetUniform4f("diffuseColor", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
	renderShader->SetUniform4f("emissionColor", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
	renderShader->SetUniform4f("lightDir", glm::vec4(0.2f, 0.4f, 0.6f, 0.0f));
	proxyModel->Draw();
	renderShader->End();

	if (enableWireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// ------------------------------------------------------------

	//
	// AABB rendering
	//

	if (enableAABB)
	{
		flatShader->Begin();
		flatShader->SetUniformMatrix4f("mvpMatrix", mvpMatrix);
		flatShader->SetUniform4f("color", glm::vec4(0.5f, 0.0f, 0.0f, 1.0f));
		proxyModel->DrawAABB();
		flatShader->End();
	}

	// ------------------------------------------------------------

	//
	// Grid rendering
	//

	if (enableGrid) DrawGrid(mvpMatrix);

	// ------------------------------------------------------------

	//
	// Stroke rendering
	//

	DrawStrokes();
	DrawCurrentStroke();

	// ------------------------------------------------------------

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
}

void Canvas::DrawGrid(const glm::mat4& mvpMatrix)
{
	flatShader->Begin();
	flatShader->SetUniformMatrix4f("mvpMatrix", mvpMatrix);
	flatShader->SetUniform4f("color", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));

	glBegin(GL_LINES);
	for (int i = -10; i <= 10; i++)
	{
		if (i == 0) continue;
		float v = (float)i * 10.0f;
		glVertex3f(-100.0f, 0.0f, v);
		glVertex3f(100.0f, 0.0f, v);
		glVertex3f(v, 0.0f, -100.0f);
		glVertex3f(v, 0.0f, 100.0f);
	}
	glEnd();

	flatShader->SetUniform4f("color", glm::vec4(1.0f, 0.3f, 0.3f, 1.0f));
	glBegin(GL_LINES);
	glVertex3f(-100.0f, 0.0f, 0.0f);
	glVertex3f(100.0f, 0.0f, 0.0f);
	glEnd();

	flatShader->SetUniform4f("color", glm::vec4(0.3f, 0.3f, 1.0f, 1.0f));
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, -100.0f);
	glVertex3f(0.0f, 0.0f, 100.0f);
	glEnd();

	flatShader->End();
}

void Canvas::DrawBackground()
{
	if (backgroundTexture)
	{
		glDisable(GL_DEPTH_TEST);
		flatTexShader->Begin();
		flatTexShader->SetUniformMatrix4f("mvpMatrix", glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f));
		flatTexShader->SetUniformTexture("colorMap", 0);
		backgroundTexture->Bind();
		quad->Draw();
		flatTexShader->End();
		glEnable(GL_DEPTH_TEST);
	}
}

void Canvas::DrawStrokes()
{
	if (strokeList.size() == 0)
	{
		return;
	}

	// ------------------------------------------------------------

	//
	// Create vertex array for rendering
	//
	
	std::vector<StrokePoint> vertices;
	for (int i = 0; i < strokeList.size(); i++)
	{
		Stroke* stroke = strokeList[i];
		for (int j = 0, k = 1; k < stroke->strokePoints.size(); j=k++)
		{
			StrokePoint& sp1 = stroke->strokePoints[j];
			StrokePoint& sp2 = stroke->strokePoints[k];
			float dist2 = glm::distance2(sp1.position, sp2.position);
			float sp = stroke->brushSpacing;
			if (sp * sp < dist2)
			{
				int div = (int)ceilf(glm::sqrt(dist2) / sp);
				float step = 1.0f / ((float)div + 1.0f);
				for (int l = 1; l < div; l++)
				{
					float t = step * (float)l;
					vertices.push_back(StrokePoint(
						glm::mix(sp1.position, sp2.position, t),
						glm::mix(sp1.color, sp2.color, t),
						sp1.id,
						glm::mix(sp1.size, sp1.size, t),
						sp1.guid));
				}
			}
			vertices.push_back(sp1);
			vertices.push_back(sp2);
		}
	}

	emit StrokeStateChanged(strokeList.size(), vertices.size());

	// ------------------------------------------------------------

	//
	// Sort vertices
	//

	const float C = -0.1f;
	std::vector<std::pair<float, int> > modifiedDepthList;
	for (int i = 0; i < vertices.size(); i++)
	{
		int guid = vertices[i].guid;
		glm::vec3& pi = vertices[i].position;
		glm::vec3 di = glm::normalize(pi - camWorldPos);

		// Distance from the current camera position.
		float depth = glm::distance2(pi + C * (float)guid * di, camWorldPos);
		modifiedDepthList.push_back(std::make_pair(depth, i));
	}
	std::sort(modifiedDepthList.begin(), modifiedDepthList.end());

	std::vector<unsigned int> indexList;
	for (int i = modifiedDepthList.size() - 1; i >= 0; i--)
	{
		indexList.push_back(modifiedDepthList[i].second);
	}

	// ------------------------------------------------------------

	//
	// Render
	//

	if (enableParticle) 
	{
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		strokePointShader->Begin();
		strokePointShader->SetUniformMatrix4f("mvMatrix", mvMatrix);
		strokePointShader->SetUniformMatrix4f("projectionMatrix", projectionMatrix);
		strokePointShader->SetUniformTexture("brushMap", 0);
		brushTextures->Bind();
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		GLsizei stride = sizeof(StrokePoint);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, &vertices[0].position);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, &vertices[0].color);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, &vertices[0].id);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, &vertices[0].size);
		//glDrawArrays(GL_POINTS, 0, vertices.size());
		glDrawElements(GL_POINTS, vertices.size(), GL_UNSIGNED_INT, &indexList[0]);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		strokePointShader->End();
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}

	// ------------------------------------------------------------

	//
	// Stroke line
	//
	
	if (enableStrokeLine)
	{
		for (int i = 0; i < strokeList.size(); i++)
		{
			strokeList[i]->Draw();
		}
	}
}

void Canvas::DrawCurrentStroke()
{
	//
	// Draw current stroke line
	//

	if (!enableCurrentStrokeLine || currentStrokePoints.size() < 2)
	{
		return;
	}

	glDisable(GL_DEPTH_TEST);

	flatShader->Begin();
	flatShader->SetUniformMatrix4f("mvpMatrix",
		glm::ortho(0.0f, (float)canvasWidth, 0.0f, (float)canvasHeight));
	flatShader->SetUniform4f("color", glm::vec4(0.5f, 0.0f, 0.0f, 1.0f));

	glBegin(GL_LINES);
	for (int i = 0, j = 1; j < currentStrokePoints.size(); i=j++)
	{
		glm::vec2 v1 = glm::vec2(currentStrokePoints[i].position);
		glm::vec2 v2 = glm::vec2(currentStrokePoints[j].position);
		glVertex3f(v1.x, v1.y, 0.0f);
		glVertex3f(v2.x, v2.y, 0.0f);
	}
	glEnd();

	flatShader->End();

	glEnable(GL_DEPTH_TEST);
}

void Canvas::OnResizeCanvas( QSize size )
{
	canvasWidth = size.width();
	canvasHeight = size.height();
}

void Canvas::OnToggleWireframe( int state )
{
	enableWireframe = state == Qt::Checked;
}

void Canvas::OnToggleAABB( int state )
{
	enableAABB = state == Qt::Checked;
}

void Canvas::OnToggleGrid( int state )
{
	enableGrid = state == Qt::Checked;
}

void Canvas::OnToggleParticle( int state )
{
	enableParticle = state == Qt::Checked;
}

void Canvas::OnToggleStrokeLine( int state )
{
	enableStrokeLine = state == Qt::Checked;
}

void Canvas::OnToggleCurrentStrokeLine( int state )
{
	enableCurrentStrokeLine = state == Qt::Checked;
}

void Canvas::OnToolChanged( int id )
{
	if (id < 0 || TOOL_NUM <= id)
	{
		THROW_EXCEPTION(Exception::InvalidArgument,
			(boost::format("Invalid tool ID: %d") % id).str().c_str());
	}
	currentTool = (EmbeddingTool)id;
}

void Canvas::OnLevelChanged( double level )
{
	currentLevel = (float)level;
}

void Canvas::OnLevelOffsetChanged( double level )
{
	currentLevelOffset = (float)level;
}

void Canvas::OnStrokeStepChanged( int step )
{
	strokeSteps = step;
}

void Canvas::OnResetViewButtonClicked()
{
	scale = 1.0f;
	trans = glm::vec3();
	currentQuat = glm::quat();
}

void Canvas::OnToggleBackground( int state )
{
	enableBackgroundTexture = state == Qt::Checked;
}

void Canvas::OnChangeBackgroundImage( QString path )
{
	if (!enableBackgroundTexture) return;
	SAFE_DELETE(backgroundTexture);
	QImage image;
	if (!image.load(path))
	{
		Util::Get()->ShowStatusMessage("Failed to load " + path);
		return;
	}
	QImage glimage = QGLWidget::convertToGLFormat(image);
	backgroundTexture = new Texture2D(
		glimage.width(), glimage.height(),
		GL_RGBA8, GL_RGBA, GL_CLAMP, GL_LINEAR, GL_LINEAR, (void*)glimage.bits());
	Util::Get()->ShowStatusMessage("Loaded background image " + path);
}

void Canvas::OnBrushColorChanged( QColor color )
{
	brushColor = glm::vec3(color.redF(), color.greenF(), color.blueF());
}

void Canvas::OnBrushChanged( int id )
{
	brushID = id;
}

void Canvas::OnBrushSizeChanged( int size )
{
	brushSize = (float)size;
}

void Canvas::OnBrushOpacityChanged( int opacity )
{
	brushOpacity = (float)opacity / 100.0f;
}

void Canvas::OnBrushSpacingChanged( double spacing )
{
	brushSpacing = (float)spacing;
}

// ------------------------------------------------------------

Stroke::Stroke(Canvas* canvas, float brushSpacing, const glm::vec3& camWorldPos)
	: canvas(canvas)
	, brushSpacing(brushSpacing)
	, camWorldPos(camWorldPos)
{

}

void Stroke::Draw()
{
	if (strokePoints.size() < 2)
	{
		return;
	}

	canvas->flatShader->Begin();
	canvas->flatShader->SetUniformMatrix4f("mvpMatrix", canvas->mvpMatrix);
	canvas->flatShader->SetUniform4f("color", glm::vec4(0.5f, 0.5f, 1.0f, 1.0f));
	
	glBegin(GL_LINES);
	for (int i = 0, j = 1; j < strokePoints.size(); i=j++)
	{
		glm::vec3& v1 = strokePoints[i].position;
		glm::vec3& v2 = strokePoints[j].position;
		glVertex3fv(glm::value_ptr(v1));
		glVertex3fv(glm::value_ptr(v2));
	}
	glEnd();

	glPointSize(2.0);
	glBegin(GL_POINTS);
	for (int i = 0; i < strokePoints.size(); i++)
	{
		glVertex3fv(glm::value_ptr(strokePoints[i].position));
	}
	glEnd();

	canvas->flatShader->End();
}

bool Stroke::Embed(const std::vector<StrokePoint>& points)
{
	// Copy stroke info
	strokePoints = points;

	// ------------------------------------------------------------

	double time = Timer::GetCurrentTimeMilli();

	// Initial distance of the stroke points
	std::vector<float> initialDists;
	int pointNum = points.size();

	// Calculate ray directions
	for (int i = 0; i < pointNum; i++)
	{
		// Calculate ray parameters from the raster position
		glm::vec2 rasterPos = glm::vec2(points[i].position);
		glm::vec3 cameraSample(
			-(float)canvas->canvasWidth * 0.5f + rasterPos.x,
			-(float)canvas->canvasHeight * 0.5f + rasterPos.y,
			-(float)canvas->canvasHeight / tanf(glm::radians(canvas->fov * 0.5f)) * 0.5f);
		rayDirs.push_back(glm::normalize(
			canvas->camWorldU * cameraSample.x +
			canvas->camWorldV * cameraSample.y +
			canvas->camWorldW * cameraSample.z));
	}

	// ------------------------------------------------------------

	if (canvas->currentTool == Canvas::TOOL_LEVEL)
	{
		// All stroke points are initialized with the sphere tracing.
		glm::vec3 normal;
		for (int i = 0; i < pointNum; i++)
		{
			float sumDist = SphereTrace(canvas->camWorldPos, rayDirs[i], canvas->currentLevel, normal);
			initialDists.push_back(sumDist);
		}
	}
	else
	{
		// Only first and last stroke points are initialized by the sphere tracing
		// and remaining points are initialized to linear interpolation of the two points.
		glm::vec3 normal;
		glm::vec3 normal2;

		float firstDist = SphereTrace(canvas->camWorldPos, rayDirs[0], canvas->currentLevel, normal);
		if (firstDist > canvas->farClip)
		{
			Util::Get()->ShowStatusMessage("Initial stroke must be on the proxy object");
			return false;
		}

		float lastDist = SphereTrace(canvas->camWorldPos, rayDirs[pointNum-1], canvas->currentLevelOffset, normal2);
		if (lastDist > canvas->farClip) lastDist = firstDist;
		
		// Root point
		glm::vec3 p1 = canvas->camWorldPos + firstDist * rayDirs[0];
		glm::vec3 p2 = canvas->camWorldPos + firstDist * rayDirs[pointNum-1];
		if (canvas->currentTool == Canvas::TOOL_HAIR) 
		{
			rootPoint = p1 - normal * 0.1f;
		}
		else if (canvas->currentTool == Canvas::TOOL_FEATHER)
		{
			glm::vec3 h = p2 - glm::dot(p2, normal) * normal;
			rootPoint = p1 - glm::normalize(h) * 0.1f;
		}

		// Interpolation
		for (int i = 0; i < pointNum; i++)
		{
			initialDists.push_back(glm::mix(firstDist, lastDist, (float)i / (pointNum - 1)));
		}
	}

	// ------------------------------------------------------------

	// Optimize the stroke with L-BFGS
	std::vector<float> optimizedDists = Optimize(initialDists);
	for (int i = 0; i < pointNum; i++)
	{
		strokePoints[i].position = canvas->camWorldPos + (float)optimizedDists[i] * rayDirs[i];
	}

	double elapsed = (Timer::GetCurrentTimeMilli() - time) / 1000.0f;
	Util::Get()->ShowStatusMessage(
		(boost::format("Stroke embedding is completed in %.1f seconds") % elapsed).str().c_str());

	return true;
}

float Stroke::SphereTrace( const glm::vec3& rayOrigin, const glm::vec3& rayDir, float level, glm::vec3& normal )
{
	// Sphere tracing
	float minDist = 0.0f;
	float sumDist = 0.0f;
	glm::vec3 currentPos(rayOrigin);
	int step = 1;
	do
	{
		minDist = canvas->proxyModel->Distance(currentPos, normal) - level;
		currentPos += minDist * rayDir;
		sumDist += minDist;
		Util::Get()->ShowStatusMessage((boost::format("Sphere tracing step #%d: %f") % step % sumDist).str().c_str());
		step++;
	} while (minDist > 1e-3 && minDist < canvas->farClip);
	return sumDist;
}

static lbfgsfloatval_t LBFGS_Evaluate( void *instance, const lbfgsfloatval_t *x, lbfgsfloatval_t *g, const int n, const lbfgsfloatval_t step )
{
	Stroke* stroke = (Stroke*)instance;
	Canvas* canvas = stroke->canvas;
	float E = 0.0f;

	std::vector<glm::vec3> strokePoints;
	for (int i = 0; i < n; i++)
	{
		g[i] = 0.0f;
		strokePoints.push_back(
			canvas->camWorldPos + x[i] * stroke->rayDirs[i]);
	}

	// ------------------------------------------------------------

	//
	// Level term (E_level)
	//

	float w_level = 1.0f;
	float E_level = 0.0f;
	glm::vec3 normal;
	for (int i = 0; i < n; i++)
	{
		float level = canvas->currentLevel;
		if (canvas->currentTool == Canvas::TOOL_HAIR ||
			canvas->currentTool == Canvas::TOOL_FEATHER)
		{
			if (i == 0) level = canvas->currentLevel;
			else if (i == n-1) level = canvas->currentLevelOffset;
			else continue;
		}

		float ti = x[i];
		glm::vec3& di = stroke->rayDirs[i];
		glm::vec3& p = strokePoints[i];
		glm::vec3 q = canvas->proxyModel->ClosestPoint(p, normal);

		float fpi = glm::distance(p, q);
		// If the distance is too close, use the normal as a gradient.
		glm::vec3 gradfpi;
		if (fpi < 1e-4) gradfpi = normal;
		else gradfpi = glm::normalize(p - q);

		float fpiminl = fpi - level;
		E_level += fpiminl * fpiminl;
		g[i] += w_level * 2.0f * fpiminl * glm::dot(gradfpi, di);
	}
	E += w_level * E_level;

	// ------------------------------------------------------------

	//
	// Angle term (E_angle)
	//

	float w_angle = canvas->currentTool == Canvas::TOOL_LEVEL ? 0.01f : 1.0f;
	float E_angle = 0.0f;

	// Consider the fixed root point in the hair/feather tools.
	if (canvas->currentTool == Canvas::TOOL_HAIR ||
		canvas->currentTool == Canvas::TOOL_FEATHER)
	{
		glm::vec3& pi = stroke->rootPoint;
		glm::vec3& pip1 = strokePoints[0];
		glm::vec3& pip2 = strokePoints[1];
		glm::vec3 pip1pip2 = pip2 - pip1;
		glm::vec3 pipip1 = pip1 - pi;
		float a = 1.0f / glm::distance(pip2, pip1);
		float b = 1.0f / glm::distance(pip1, pi);
		float c = glm::dot(pip1pip2, pipip1);
		float tmp = 1 - a * b * c;
		E_angle += tmp * tmp * 10000.0f; // large weight
	}

	for (int i = 0; i < n-2; i++)
	{
		float ti = x[i];
		float tip1 = x[i+1];
		float tip2 = x[i+2];
		glm::vec3& di = stroke->rayDirs[i];
		glm::vec3& dip1 = stroke->rayDirs[i];
		glm::vec3& dip2 = stroke->rayDirs[i];
		glm::vec3& pi = strokePoints[i];
		glm::vec3& pip1 = strokePoints[i+1];
		glm::vec3& pip2 = strokePoints[i+2];
		glm::vec3 pip1pip2 = pip2 - pip1;
		glm::vec3 pipip1 = pip1 - pi;
		float a = 1.0f / glm::distance(pip2, pip1);
		float b = 1.0f / glm::distance(pip1, pi);
		float c = glm::dot(pip1pip2, pipip1);
		float tmp = 1 - a * b * c;
		E_angle += tmp * tmp;

		// Gradient
		float grad_aip1 = a*a*a * glm::dot(pip1pip2, dip1);
		float grad_aip2 = -a*a*a * glm::dot(pip1pip2, dip2);
		float grad_bi = b*b*b * glm::dot(pipip1, di);
		float grad_bip1 = -b*b*b * glm::dot(pipip1, dip1);
		float d02 = glm::dot(di, dip2);
		float d01 = glm::dot(di, dip1);
		float d12 = glm::dot(dip1, dip2);
		float d11 = glm::dot(dip1, dip1);
		float grad_ci = -tip2 * d02 + tip1 * d01;
		float grad_cip1 = tip2 * d12 - 2.0f * tip1 * d11 + ti * d01;
		float grad_cip2 = tip1 * d12 - ti * d02;
		float tmp2 = w_angle * -2.0f * tmp;
		g[i] += tmp2 * (a*grad_bi*c + a*b*grad_ci);
		g[i+1] += tmp2 * (grad_aip1*b*c + a*grad_bip1*c + a*b*grad_cip1);
		g[i+2] += tmp2 * (grad_aip2*b*c + a*b*grad_cip2);
	}
	E += w_angle * E_angle;

	// ------------------------------------------------------------

	//
	// Level term (E_length)
	//

	if (canvas->currentTool != Canvas::TOOL_LEVEL)
	{
		float w_length = 0.1f;
		float E_length = 0.0f;
		for (int i = 0; i < n-1; i++)
		{
			float ti = x[i];
			float tip1 = x[i+1];
			glm::vec3& di = stroke->rayDirs[i];
			glm::vec3& dip1 = stroke->rayDirs[i];
			glm::vec3& pi = strokePoints[i];
			glm::vec3& pip1 = strokePoints[i+1];
			E_length += glm::distance2(pip1, pi);
			g[i] += -2.0f * glm::dot(pip1 - pi, di);
			g[i+1] += 2.0f * glm::dot(pip1 - pi, dip1);
		}
		E += w_length * E_length;
	}

	// ------------------------------------------------------------

	return E;
}

static int LBFGS_Progress( void *instance, const lbfgsfloatval_t *x, const lbfgsfloatval_t *g, const lbfgsfloatval_t fx, const lbfgsfloatval_t xnorm, const lbfgsfloatval_t gnorm, const lbfgsfloatval_t step, int n, int k, int ls )
{
	Stroke* stroke = (Stroke*)instance;
	Canvas* canvas = stroke->canvas;
	Util::Get()->ShowStatusMessage((boost::format("Iteration #%d : E = %f") % k % fx).str().c_str());
	return 0;
}

std::vector<float> Stroke::Optimize( const std::vector<float>& ts )
{
	int N = ts.size();
	lbfgsfloatval_t* x;
	lbfgs_parameter_t param;

	x = lbfgs_malloc(N);
	if (x == NULL)
	{
		THROW_EXCEPTION(Exception::RunTimeError,
			"Stroke optimization: failed to allocate memory");
	}

	// Initial variables
	for (int i = 0; i < N; i++) x[i] = ts[i];

	lbfgs_parameter_init(&param);
	param.epsilon = 1e-4;
	//param.max_iterations = 300;

	int ret = lbfgs(N, x, NULL, LBFGS_Evaluate, LBFGS_Progress, (void*)this, &param);
	Util::Get()->ShowStatusMessage((boost::format("L-BFGS optimization finished (%d)") % ret).str().c_str());

	std::vector<float> retts;
	for (int i = 0; i < N; i++) retts.push_back(x[i]);

	lbfgs_free(x);
	return retts;
}
