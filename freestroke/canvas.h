#ifndef __CANVAS_H__
#define __CANVAS_H__

class GlslShader;
class ObjModel;
class Stroke2D;
class Stroke;
class Camera;
class Texture2D;
class Texture2DArray;
class QuadMesh;

namespace boost
{
	namespace serialization 
	{

		template <class Archive>
		void serialize(Archive& ar, glm::vec3& v, const unsigned int version)
		{
			ar & v.x & v.y & v.z;
		}

		template <class Archive>
		void serialize(Archive& ar, glm::vec4& v, const unsigned int version)
		{
			ar & v.x & v.y & v.z & v.w;
		}

	}
}

/*!
	Stroke point.
	The structure describes the stroke point of the embedded stroke.
*/
struct StrokePoint
{

	StrokePoint()
	{

	}

	StrokePoint(const glm::vec3& position, const glm::vec4& color, int id, float size, int guid)
		: position(position)
		, color(color)
		, id(id)
		, size(size)
		, guid(guid)
	{

	}

	glm::vec3 position;	//!< Stroke point.
	glm::vec4 color;	//!< Brush color. Opacity term included in the color.
	int id;				//!< Brush ID.
	float size;			//!< Brush size in the world space.
	int guid;			//!< GUID assigned for each stroke.	

private:

	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & position & color & id & size & guid;
	}

};

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

	Canvas();
	Canvas(std::string proxyGeometryPath, int width, int height);
	~Canvas();
	State GetState() { return state; }
	bool IsModified() { return modified; }
	void SetModified(bool enable) { modified = enable; }
	void Initialize();

public slots:

	void OnUndoStroke();

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
	void OnToggleGrid(int state);
	void OnToggleParticle(int state);
	void OnToggleStrokeLine(int state);
	void OnToggleCurrentStrokeLine(int state);
	void OnToggleProxyObjectCheckBox(int state);
	void OnResetViewButtonClicked();
	void OnToggleBackground(int state);
	void OnChangeBackgroundImage(QString path);

	void OnToolChanged(int id);
	void OnLevelChanged(double level);
	void OnLevelOffsetChanged(double level);
	void OnStrokeStepChanged(int step);
	void OnStrokeOrderOffsetChanged(double offset);

	void OnBrushColorChanged(QColor color);
	void OnBrushChanged(int id);
	void OnBrushSizeChanged(int size);
	void OnBrushOpacityChanged(int opacity);
	void OnBrushSpacingChanged(double spacing);

signals:

	void StateChanged(unsigned int state);
	void StrokeStateChanged(int strokeNum, int particleNum);

private:

	void ChangeState(State nextState);
	void DrawGrid(const glm::mat4& mvpMatrix);
	void LoadBrushTexture();
	void DrawBackground();
	void DrawStrokes();
	void DrawCurrentStroke();
	void DrawProxyObject();

private:

	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & proxyGeometryPath & canvasWidth & canvasHeight & strokeList;
	}

public:

	// Canvas info
	std::string proxyGeometryPath;
	bool modified;	
	int canvasWidth;
	int canvasHeight;
	State state;

	// ------------------------------------------------------------

	// Strokes
	std::vector<StrokePoint> currentStrokePoints;
	std::vector<Stroke*> strokeList;

	// ------------------------------------------------------------

	// Proxy model rendering
	ObjModel* proxyModel;
	GlslShader* renderShader;
	GlslShader* flatShader;
	GlslShader* flatTexShader;
	GlslShader* strokePointShader;

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
	bool enableGrid;
	bool enableParticle;
	bool enableStrokeLine;
	bool enableCurrentStrokeLine;
	bool enableProxyObject;

	// Background
	QuadMesh* quad;
	bool enableBackgroundTexture;
	Texture2D* backgroundTexture;

	// Brush state
	Texture2DArray* brushTextures;
	glm::vec3 brushColor;
	int brushID;
	float brushSize;
	float brushOpacity;
	float brushSpacing;

	// Stroke embedding
	EmbeddingTool currentTool;
	float currentLevel;
	float currentLevelOffset;
	int strokeSteps;
	int currentStrokeSteps;
	float strokeOrderOffset;

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
	Stroke.
	The class describes single stroke.
*/
class Stroke
{
public:
	
	Stroke();
	Stroke(Canvas* canvas, float brushSpacing);
	void Draw();
	bool Embed(const std::vector<StrokePoint>& points);

protected:

	float SphereTrace(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float level, glm::vec3& normal);
	std::vector<float> Optimize(const std::vector<float>& ts);

private:

	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & strokePoints & brushSpacing;
	}

public:

	Canvas* canvas;
	std::vector<StrokePoint> strokePoints;
	float brushSpacing;

	// The fixed point used for the hair and feather tools
	glm::vec3 rootPoint;

	// Ray directions
	std::vector<glm::vec3> rayDirs;

};

#endif // __CANVAS_H__