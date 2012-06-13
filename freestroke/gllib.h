#ifndef __GL_LIB_H__
#define __GL_LIB_H__

#include <GL/glew.h>
#include <GL/wglew.h>

#define CHECK_GL_ERRORS() CheckGLErrors(__FILE__, __FUNCTION__, __LINE__)
void CheckGLErrors(const char* filename, const char* funcname, const int line);

/*!
	AABB.
	Axis-Aligned Bounding Box.
*/
class AABB
{
public:

	void Draw();

public:

	glm::vec3 min;
	glm::vec3 max;

};

/*!
	Vertex stream.
	The class represents GL vertex stream
	which associates a vertex array object and
	several vertex buffer objects.
*/
class VertexStream
{
private:

	typedef boost::unordered_map<GLuint, std::vector<float> > IndexVertexListMap;
	typedef boost::unordered_map<GLuint, GLint> IndexVertexSizeMap;
	typedef boost::unordered_map<GLuint, GLuint> IndexVBOArrayIDMap;

public:

	enum AttributeType
	{
		POSITION,
		NORMAL,
		TEXCOORD0,
		TEXCOORD1,
		TEXCOORD2,
		TEXCOORD3,
		TANGENT
	};

public:

	VertexStream();
	virtual ~VertexStream();
	void AddAttribute(GLuint index, GLint size);
	void Begin();
	void End();
	void Draw(GLenum mode);
	void AddVertex(GLuint index, const glm::vec2& v);
	void AddVertex(GLuint index, const glm::vec3& v);
	void AddVertex(GLuint index, const glm::vec4& v);
	void AddVertex(GLuint index, const std::vector<float>& v);
	void AddVertex(GLuint index, const float* v, int n);
	void AddIndex(GLuint i);
	void AddIndex(GLuint i0, GLuint i1);
	void AddIndex(GLuint i0, GLuint i1, GLuint i2);
	void AddIndex(GLuint i0, GLuint i1, GLuint i2, GLuint i3);
	void AddIndex(const std::vector<GLuint>& v);
	void AddIndex(const GLuint* v, int n);

protected:

	IndexVertexListMap vertexListMap;
	IndexVertexSizeMap vertexSizeMap;
	IndexVBOArrayIDMap vboArrayIDMap;

	GLuint vaoID;
	GLuint indexBufferID;
	std::vector<GLuint> indexList;

};

/*!
	Triangle mesh.
	The class describes triangle mesh.
*/
class TriangleMesh : public VertexStream
{
public:

	TriangleMesh();
	virtual ~TriangleMesh();
	void Draw();

};

/*!
	Sphere mesh.
	The class describes sphere mesh.
*/
class SphereMesh : public TriangleMesh
{
public:

	SphereMesh(float radius, int slicenum, int stacknum);

};

class QuadMesh : public TriangleMesh
{
public:

	QuadMesh();
	QuadMesh(float x, float y, float w, float h);

};

/*!
	GLSL program.
	The class for handling a GLSL program with shaders.
*/
class GlslShader
{
public:

	enum ShaderType
	{
		VERTEX_SHADER,
		FRAGMENT_SHADER,
		GEOMETRY_SHADER
	};

	typedef boost::unordered_map<std::string, GLuint> UniformLocationMap;

public:

	GlslShader();
	~GlslShader();
	void AddShader(ShaderType type, const std::string& path);
	void AddShaderString(ShaderType type, const std::string& content);
	void Initialize();
	void BindAttribute(GLuint index, const std::string& name);
	void SetUniformMatrix4f(const std::string& name, const glm::mat4& mat);
	void SetUniformMatrix3f(const std::string& name, const glm::mat3& mat);
	void SetUniform1f(const std::string& name, float v);
	void SetUniform2f(const std::string& name, const glm::vec2& v);
	void SetUniform3f(const std::string& name, const glm::vec3& v);
	void SetUniform4f(const std::string& name, const glm::vec4& v);
	void SetUniformTexture(const std::string& name, int unit);
	void SetUniform1i(const std::string& name, int v);
	void Begin();
	void End();

private:

	std::string LoadShaderFile(const std::string& path);
	GLuint CreateAndCompileShader(GLuint type, const std::string& content, const std::string& path);
	GLuint GetOrCreateUniformID(const std::string& name);
	
private:

	GLuint programID;
	UniformLocationMap uniformLocationMap;

};


/*!
	Texture base.
	The texture base class which represents GL textures.
*/
class Texture
{
public:

	Texture();
	virtual ~Texture();

public:

	virtual void Bind() = 0;
	virtual void Bind(GLenum unit) = 0;
	GLuint GetID() { return textureID; }

protected:

	GLuint textureID;

};

/*!
	2D texture.
	2 dimensional texture class which represents GL_TEXTURE2D.
*/
class Texture2D : public Texture
{
public:

	Texture2D(int width, int height, GLint internalformat, GLenum format, GLint wrapmode, GLint magfilter, GLint minfilter);
	Texture2D(int width, int height, GLint internalformat, GLenum format, GLint wrapmode, GLint magfilter, GLint minfilter, const void* data);
	void Bind();
	void Bind(GLenum unit);
	void Substitute(int xoffset, int yoffset, int width, int height, GLenum format, GLenum type, const void* data);

private:

	void SetTextureParam(GLint wrapmode, GLint minfilter, GLint magfilter);

};

class ImageLoader
{
public:

	ImageLoader(const std::string& path);
	~ImageLoader();
	unsigned int GetWidth();
	unsigned int GetHeight();
	GLubyte* GetData();

private:

	class Impl;
	boost::scoped_ptr<Impl> pimpl;

};

/*!
	2D texture array.
	The class describes GL_TEXTURE_2D_ARRAY.
*/
class Texture2DArray : public Texture
{
public:

	Texture2DArray(int width, int height, int depth, GLint internalformat, GLenum format, GLint wrapmode, GLint magfilter, GLint minfilter);
	void Bind();
	void Bind(GLenum unit);
	void Substitute(int depth, GLenum format, const void* data);

private:

	void SetTextureParam(GLint wrapmode, GLint minfilter, GLint magfilter);

private:

	int width;
	int height;

};

#endif // __GL_LIB_H__