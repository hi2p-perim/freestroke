#ifndef __MODEL_H__
#define __MODEL_H__

/*!
	Proxy object.
	The class describes the proxy model and 
	loads the Wavefront .obj file and construct related data structures.
*/
class ObjModel
{
public:

	ObjModel(const std::string& path, float size);
	~ObjModel();
	void Draw();
	void DrawAABB();
	glm::vec3 ClosestPoint(const glm::vec3& p, glm::vec3& normal);
	float Distance(const glm::vec3 p, glm::vec3& normal);

private:

	class Impl;
	boost::scoped_ptr<Impl> pimpl;

};

#endif // __MODEL_H__