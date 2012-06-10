#include "gllib.h"

void CheckGLErrors( const char* filename, const char* funcname, const int line )
{
	int err;
	if ((err = glGetError()) != GL_NO_ERROR)
	{
		std::string errstr;
		switch (err)
		{
		case GL_INVALID_ENUM:
			errstr = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			errstr = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			errstr = "GL_INVALID_OPERATION";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			errstr = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		case GL_OUT_OF_MEMORY:
			errstr = "GL_OUT_OF_MEMORY";
			break;
		}
		throw Exception(Exception::OpenGLError, errstr, filename, funcname, line, GetStackTrace());
	}
}

void AABB::Draw()
{
	glBegin(GL_LINES);

	glVertex3f(min.x, min.y, min.z);
	glVertex3f(max.x, min.y, min.z);
	glVertex3f(max.x, min.y, min.z);
	glVertex3f(max.x, max.y, min.z);
	glVertex3f(max.x, max.y, min.z);
	glVertex3f(min.x, max.y, min.z);
	glVertex3f(min.x, max.y, min.z);
	glVertex3f(min.x, min.y, min.z);

	glVertex3f(min.x, min.y, max.z);
	glVertex3f(max.x, min.y, max.z);
	glVertex3f(max.x, min.y, max.z);
	glVertex3f(max.x, max.y, max.z);
	glVertex3f(max.x, max.y, max.z);
	glVertex3f(min.x, max.y, max.z);
	glVertex3f(min.x, max.y, max.z);
	glVertex3f(min.x, min.y, max.z);

	glVertex3f(min.x, min.y, min.z);
	glVertex3f(min.x, min.y, max.z);
	glVertex3f(max.x, min.y, min.z);
	glVertex3f(max.x, min.y, max.z);
	glVertex3f(max.x, max.y, min.z);
	glVertex3f(max.x, max.y, max.z);
	glVertex3f(min.x, max.y, min.z);
	glVertex3f(min.x, max.y, max.z);

	glEnd();
}

VertexStream::VertexStream()
{
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &indexBufferID);
	CHECK_GL_ERRORS();
}

VertexStream::~VertexStream()
{
	BOOST_FOREACH (IndexVBOArrayIDMap::value_type& pair, vboArrayIDMap)
	{
		glDeleteBuffers(1, &pair.second);
	}
	glDeleteBuffers(1, &indexBufferID);
	glDeleteVertexArrays(1, &vaoID);
}

void VertexStream::AddAttribute( GLuint index, GLint size )
{
	if (vboArrayIDMap.find(index) != vboArrayIDMap.end())
	{
		// Given index is already registered, raise error.
		THROW_EXCEPTION(Exception::OpenGLError,
			(boost::format("Vertex attribute %d is already registered.") % index).str());
	}

	// Create VBO
	GLuint vboID;
	glGenBuffers(1, &vboID);
	vboArrayIDMap[index] = vboID;

	// Vertex attribute size
	vertexSizeMap[index] = size;
}

void VertexStream::Begin()
{
	vertexListMap.clear();
	indexList.clear();
}

void VertexStream::End()
{
	// Check validity.
	// For each attribute index, there must be same index in the vertex list map
	// and each list must be same length.
	int vertexnum = -1;
	BOOST_FOREACH (IndexVBOArrayIDMap::value_type& pair, vboArrayIDMap)
	{
		if (vertexListMap.find(pair.first) == vertexListMap.end())
		{
			THROW_EXCEPTION(Exception::OpenGLError,
				(boost::format("Invalid vertex: index attribute %d is not registerd.") % pair.first).str());
		}
		else
		{
			int length = vertexListMap[pair.first].size();
			int size = vertexSizeMap[pair.first];
			if (vertexnum < 0)
			{
				vertexnum = length / size;
			}
			else if (length / size != vertexnum)
			{
				THROW_EXCEPTION(Exception::OpenGLError,
					(boost::format("Invalid vertex: vertex number is .") % pair.first).str());				
			}
		}
	}

	glBindVertexArray(vaoID);

	// Assign index buffer data.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexList.size() * sizeof(GLuint), &indexList[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	CHECK_GL_ERRORS();

	// Assign buffer data.
	BOOST_FOREACH (IndexVertexListMap::value_type& pair, vertexListMap)
	{
		int vboID = vboArrayIDMap[pair.first];
		int size = vertexSizeMap[pair.first];
		std::vector<float>& v = vertexListMap[pair.first];
		glBindBuffer(GL_ARRAY_BUFFER, vboID);
		glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), &v[0], GL_STATIC_DRAW);
		glVertexAttribPointer(pair.first, size / sizeof(float), GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(pair.first);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS();
	}

	glBindVertexArray(0);
}

void VertexStream::AddVertex( GLuint index, const glm::vec2& v )
{
	std::vector<float>& list = vertexListMap[index];
	list.push_back(v.x);
	list.push_back(v.y);
}

void VertexStream::AddVertex( GLuint index, const glm::vec3& v )
{
	std::vector<float>& list = vertexListMap[index];
	list.push_back(v.x);
	list.push_back(v.y);
	list.push_back(v.z);
}

void VertexStream::AddVertex( GLuint index, const glm::vec4& v )
{
	std::vector<float>& list = vertexListMap[index];
	list.push_back(v.x);
	list.push_back(v.y);
	list.push_back(v.z);
	list.push_back(v.w);
}

void VertexStream::AddVertex( GLuint index, const std::vector<float>& v )
{
	std::vector<float>& list = vertexListMap[index];
	BOOST_FOREACH (float f, v)
	{
		list.push_back(f);
	}
}

void VertexStream::AddVertex( GLuint index, const float* v, int n )
{
	std::vector<float>& list = vertexListMap[index];
	for (int i = 0; i < n; i++)
	{
		list.push_back(v[i]);
	}
}

void VertexStream::Draw( GLenum mode )
{
	glBindVertexArray(vaoID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glDrawElements(mode, indexList.size(), GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void VertexStream::AddIndex( GLuint i )
{
	indexList.push_back(i);
}

void VertexStream::AddIndex( GLuint i0, GLuint i1 )
{
	indexList.push_back(i0);
	indexList.push_back(i1);
}

void VertexStream::AddIndex( GLuint i0, GLuint i1, GLuint i2 )
{
	indexList.push_back(i0);
	indexList.push_back(i1);
	indexList.push_back(i2);
}

void VertexStream::AddIndex( GLuint i0, GLuint i1, GLuint i2, GLuint i3 )
{
	indexList.push_back(i0);
	indexList.push_back(i1);
	indexList.push_back(i2);
	indexList.push_back(i3);
}

void VertexStream::AddIndex( const std::vector<GLuint>& v )
{
	BOOST_FOREACH (GLuint i, v)
	{
		indexList.push_back(i);
	}
}

void VertexStream::AddIndex( const GLuint* v, int n )
{
	for (int i = 0; i < n; i++)
	{
		indexList.push_back(v[i]);
	}
}

TriangleMesh::TriangleMesh()
{

}

TriangleMesh::~TriangleMesh()
{

}

void TriangleMesh::Draw()
{
	VertexStream::Draw(GL_TRIANGLES);
}

SphereMesh::SphereMesh( float radius, int slicenum, int stacknum )
{
	AddAttribute(POSITION, sizeof(glm::vec3));
	AddAttribute(NORMAL, sizeof(glm::vec3));
	AddAttribute(TEXCOORD0, sizeof(glm::vec2));

	const float pi = 3.141592653589f;
	float drho = pi / (float)stacknum;
	float dtheta = 2.0f * pi / (float)slicenum;
	float ds = 1.0f / (float)slicenum;
	float dt = 1.0f / (float)stacknum;
	float t = 1.0f;
	float s = 0.0f;

	unsigned int v_index = 0;
	Begin();

	for (int i = 0; i < stacknum; ++i)
	{
		float rho = (float)i * drho;
		float srho = (float)sin(rho);
		float crho = (float)cos(rho);
		float srhodrho = (float)sin(rho + drho);
		float crhodrho = (float)cos(rho + drho);

		s = 0.0f;
		glm::vec3 vertex[4];
		glm::vec3 normal[4];
		glm::vec2 texcoord[4];

		for (int j = 0; j < slicenum; ++j)
		{
			float theta = (j == slicenum) ? 0.0f : j * dtheta;
			float stheta = (float)(-sin(theta));
			float ctheta = (float)cos(theta);

			float x = stheta * srho;
			float y = ctheta * srho;
			float z = crho;

			texcoord[0][0] = s;
			texcoord[0][1] = t;
			normal[0][0] = x;
			normal[0][1] = y;
			normal[0][2] = z;
			vertex[0][0] = x * radius;
			vertex[0][1] = y * radius;
			vertex[0][2] = z * radius;

			x = stheta * srhodrho;
			y = ctheta * srhodrho;
			z = crhodrho;

			texcoord[1][0] = s;
			texcoord[1][1] = t - dt;
			normal[1][0] = x;
			normal[1][1] = y;
			normal[1][2] = z;
			vertex[1][0] = x * radius;
			vertex[1][1] = y * radius;
			vertex[1][2] = z * radius;

			theta = ((j+1) == slicenum) ? 0.0f : (j+1) * dtheta;
			stheta = (GLfloat)(-sin(theta));
			ctheta = (GLfloat)(cos(theta));

			x = stheta * srho;
			y = ctheta * srho;
			z = crho;

			s += ds;
			texcoord[2][0] = s;
			texcoord[2][1] = t;
			normal[2][0] = x;
			normal[2][1] = y;
			normal[2][2] = z;
			vertex[2][0] = x * radius;
			vertex[2][1] = y * radius;
			vertex[2][2] = z * radius;

			x = stheta * srhodrho;
			y = ctheta * srhodrho;
			z = crhodrho;

			texcoord[3][0] = s;
			texcoord[3][1] = t - dt;
			normal[3][0] = x;
			normal[3][1] = y;
			normal[3][2] = z;
			vertex[3][0] = x * radius;
			vertex[3][1] = y * radius;
			vertex[3][2] = z * radius;

			// Add triangle
			for (int k = 0; k < 3; k++)
			{
				AddVertex(POSITION, vertex[k]);
				AddVertex(NORMAL, normal[k]);
				AddVertex(TEXCOORD0, texcoord[k]);
				AddIndex(v_index++);
			}

			vertex[0] = vertex[1];
			normal[0] = normal[1];
			texcoord[0] = texcoord[1];

			vertex[1] = vertex[3];
			normal[1] = normal[3];
			texcoord[1] = texcoord[3];

			for (int k = 0; k < 3; k++)
			{
				AddVertex(POSITION, vertex[k]);
				AddVertex(NORMAL, normal[k]);
				AddVertex(TEXCOORD0, texcoord[k]);
				AddIndex(v_index++);
			}
		}
		t -= dt;
	}

	End();
}

GlslShader::GlslShader()
{
	programID = glCreateProgram();
	CHECK_GL_ERRORS();
}

GlslShader::~GlslShader()
{
	glDeleteProgram(programID);
}

// Create and compile shader
void GlslShader::AddShader( ShaderType type, const std::string& path )
{
	if (path.empty())
	{
		THROW_EXCEPTION(Exception::InvalidArgument, "Given path is empty");
	}

	// Load file
	std::string content = LoadShaderFile(path);

	// Convert shader type to GL shader type
	GLuint shaderID;
	switch (type)
	{
	case VERTEX_SHADER:
		shaderID = CreateAndCompileShader(GL_VERTEX_SHADER, content, path);
		break;

	case FRAGMENT_SHADER:
		shaderID = CreateAndCompileShader(GL_FRAGMENT_SHADER, content, path);
		break;

	case GEOMETRY_SHADER:
		shaderID = CreateAndCompileShader(GL_GEOMETRY_SHADER, content, path);
		break;

	default:
		THROW_EXCEPTION(Exception::InvalidArgument, "Invalid shader type");
	}

	// Attach to the program and delete
	glAttachShader(programID, shaderID);
	glDeleteShader(shaderID);

	CHECK_GL_ERRORS();
}

void GlslShader::AddShaderString( ShaderType type, const std::string& content )
{
	// Convert shader type to GL shader type
	GLuint shaderID;
	const std::string path = "<literal>";
	switch (type)
	{
	case VERTEX_SHADER:
		shaderID = CreateAndCompileShader(GL_VERTEX_SHADER, content, path);
		break;

	case FRAGMENT_SHADER:
		shaderID = CreateAndCompileShader(GL_FRAGMENT_SHADER, content, path);
		break;

	case GEOMETRY_SHADER:
		shaderID = CreateAndCompileShader(GL_GEOMETRY_SHADER, content, path);
		break;

	default:
		THROW_EXCEPTION(Exception::InvalidArgument, "Invalid shader type");
	}

	// Attach to the program and delete
	glAttachShader(programID, shaderID);
	glDeleteShader(shaderID);

	CHECK_GL_ERRORS();
}

// Load shader file and return its content
std::string GlslShader::LoadShaderFile(const std::string& path)
{
	std::ifstream ifs(path.c_str(), std::ios::in);
	if (!ifs.is_open())
	{
		THROW_EXCEPTION(Exception::FileError, path);
	}

	std::string line = "";
	std::string text = "";

	while (getline(ifs, line))
	{
		text += ("\n" + line);
	}

	ifs.close();

	return text;
}

GLuint GlslShader::CreateAndCompileShader( GLuint type, const std::string& content, const std::string& path )
{
	// Create and compile shader
	GLuint shaderID = glCreateShader(type);
	const char* content_ptr = content.c_str();
	glShaderSource(shaderID, 1, &content_ptr, NULL);
	glCompileShader(shaderID);

	// Check error
	GLint ret;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &ret);
	if (ret == GL_FALSE)
	{
		// Required size
		GLint length;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);

		boost::scoped_array<char> buffer(new char[length]);
		glGetShaderInfoLog(shaderID, length, NULL, buffer.get());
		glDeleteShader(shaderID);

		std::stringstream ss;
		ss << path << std::endl;
		ss << buffer.get() << std::endl;

		// Throw exception
		THROW_EXCEPTION(Exception::ShaderCompileError, ss.str());
	}

	return shaderID;
}

void GlslShader::BindAttribute( GLuint index, const std::string& name )
{
	glBindAttribLocation(programID, index, name.c_str());
	CHECK_GL_ERRORS();
}

void GlslShader::Initialize()
{
	// Link program
	glLinkProgram(programID);

	// Check error
	GLint ret;
	glGetProgramiv(programID, GL_LINK_STATUS, &ret);
	if (ret == GL_FALSE)
	{
		// Required size
		GLint length;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &length);

		boost::scoped_array<char> buffer(new char[length]);
		glGetProgramInfoLog(programID, length, NULL, buffer.get());

		// Throw exception
		THROW_EXCEPTION(Exception::ProgramLinkError, buffer.get());
	}

	CHECK_GL_ERRORS();
}

GLuint GlslShader::GetOrCreateUniformID( const std::string& name )
{
	UniformLocationMap::iterator it = uniformLocationMap.find(name);
	if (it == uniformLocationMap.end())
	{
		GLuint loc = glGetUniformLocation(programID, name.c_str());
		uniformLocationMap[name] = loc;
		return loc;
	}
	return it->second;
}

void GlslShader::SetUniformMatrix4f( const std::string& name, const glm::mat4& mat )
{
	GLuint uniformID = GetOrCreateUniformID(name);
	glUniformMatrix4fv(uniformID, 1, GL_FALSE, glm::value_ptr(mat));
	CHECK_GL_ERRORS();
}

void GlslShader::SetUniformMatrix3f( const std::string& name, const glm::mat3& mat )
{
	GLuint uniformID = GetOrCreateUniformID(name);
	glUniformMatrix3fv(uniformID, 1, GL_FALSE, glm::value_ptr(mat));
	CHECK_GL_ERRORS();
}

void GlslShader::SetUniform3f( const std::string& name, const glm::vec3& v )
{
	GLuint uniformID = GetOrCreateUniformID(name);
	glUniform3fv(uniformID, 1, glm::value_ptr(v));
	CHECK_GL_ERRORS();
}

void GlslShader::SetUniform4f( const std::string& name, const glm::vec4& v )
{
	GLuint uniformID = GetOrCreateUniformID(name);
	glUniform4fv(uniformID, 1, glm::value_ptr(v));
	CHECK_GL_ERRORS();
}

void GlslShader::Begin()
{
	glUseProgram(programID);
	CHECK_GL_ERRORS();
}

void GlslShader::End()
{
	glUseProgram(0);
	CHECK_GL_ERRORS();
}

void GlslShader::SetUniformTexture( const std::string& name, int unit )
{
	GLuint uniformID = GetOrCreateUniformID(name);
	glUniform1i(uniformID, unit);
	CHECK_GL_ERRORS();
}

void GlslShader::SetUniform1f( const std::string& name, float v )
{
	GLuint uniformID = GetOrCreateUniformID(name);
	glUniform1f(uniformID, v);
	CHECK_GL_ERRORS();
}

void GlslShader::SetUniform2f( const std::string& name, const glm::vec2& v )
{
	GLuint uniformID = GetOrCreateUniformID(name);
	glUniform2fv(uniformID, 1, glm::value_ptr(v));
	CHECK_GL_ERRORS();
}

void GlslShader::SetUniform1i( const std::string& name, int v )
{
	GLuint uniformID = GetOrCreateUniformID(name);
	glUniform1i(uniformID, v);
	CHECK_GL_ERRORS();
}
