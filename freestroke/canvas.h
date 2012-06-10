#ifndef __CANVAS_H__
#define __CANVAS_H__

class GlslShader;
class ObjModel;
class Stroke2D;
class Stroke;
class Camera;

/*!
	Canvas.
	The class describes the canvas of the application and manages
	related operations to the stroke manipulations.
*/
class Canvas : public QObject
{
	Q_OBJECT

public:

	enum State
	{
		STATE_IDLE,
		STATE_STROKING,
		STATE_ROTATING,
		STATE_TRANSLATING
	};

	enum EmbeddingTool
	{
		TOOL_LEVEL,
		TOOL_HAIR,
		TOOL_FEATHER,
		TOOL_NUM
	};

public:

	Canvas(QString proxyGeometryPath, int width, int height);
	~Canvas();
	State GetState() { return state; }
	bool IsModified() { return modified; }
	void ShowStatusMessage(QString mes);

public slots:

	void OnResizeCanvas(QSize size);
	void OnDraw();
	void OnKeyPressed(QKeyEvent* event);
	void OnKeyReleased(QKeyEvent* event);
	void OnMousePressed(QGraphicsSceneMouseEvent* event);
	void OnMouseReleased(QGraphicsSceneMouseEvent* event);
	void OnMouseMoved(QGraphicsSceneMouseEvent* event);
	void OnWheelEvent(QGraphicsSceneWheelEvent* event);
	void OnToggleWireframe(int state);
	void OnToggleAABB(int state);
	void OnToolChanged(int id);
	void OnLevelChanged(double level);
	void OnLevelOffsetChanged(double level);
	void OnStrokeStepChanged(int step);

signals:

	void StateChanged(unsigned int state);
	void StatusMessage(QString mes);

private:

	void ChangeState(State nextState);

public:

	// Canvas info
	QString proxyGeometryPath;
	bool modified;	
	int canvasWidth;
	int canvasHeight;
	State state;

	// ------------------------------------------------------------

	// Strokes
	Stroke2D* currentStroke;
	std::vector<Stroke*> strokeList;

	// ------------------------------------------------------------

	// Proxy model rendering
	ObjModel* proxyModel;
	GlslShader* renderShader;
	GlslShader* flatShader;

	// ------------------------------------------------------------

	// Rotation
	float scale;
	int startx, starty;
	glm::quat startQuat;
	glm::quat currentQuat;

	// Translation
	glm::vec3 trans;

	// View state
	bool enableWireframe;
	bool enableAABB;

	// Stroke embedding
	EmbeddingTool currentTool;
	float currentLevel;
	float currentLevelOffset;
	int strokeSteps;
	int currentStrokeSteps;

	// ------------------------------------------------------------

	// Matrix
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 modelMatrix;
	glm::mat4 mvMatrix;
	glm::mat4 mvpMatrix;

	// ------------------------------------------------------------

	// Camera params
	float fov;
	float nearClip, farClip;
	glm::vec3 camWorldU, camWorldV, camWorldW;
	glm::vec3 camWorldPos;

};

/*!
	2D Stroke.
	The class holds the 2D points of the stroke.
*/
class Stroke2D
{
public:

	Stroke2D(Canvas* canvas);
	void Draw();
	void AddStroke(const glm::vec2& point);

public:

	Canvas* canvas;
	std::vector<glm::vec2> strokePoints;

};

/*!
	Stroke.
	The class describes single stroke.
*/
class Stroke
{
public:
	
	Stroke(Canvas* canvas);
	void Draw();
	bool Embed(Stroke2D* stroke);

protected:

	float SphereTrace(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float level, glm::vec3& normal);
	std::vector<float> Optimize(const std::vector<float>& ts);

public:

	Canvas* canvas;
	std::vector<glm::vec3> strokePoints;
	// The fixed point used for the hair and feather tools
	glm::vec3 rootPoint;

	// Ray directions
	std::vector<glm::vec3> rayDirs;

};

#endif // __CANVAS_H__