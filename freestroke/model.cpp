#include "model.h"
#include "gllib.h"
#include "util.h"

#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

class ObjModel::Impl
{
public:

	typedef CGAL::Simple_cartesian<float> K;
	typedef std::vector<K::Triangle_3> KTriList;
	typedef KTriList::iterator KTriListIter;
	typedef CGAL::AABB_triangle_primitive<K, KTriListIter> AABBTriPrimitive;
	typedef CGAL::AABB_traits<K, AABBTriPrimitive> AABBTriTrait;
	typedef CGAL::AABB_tree<AABBTriTrait> AABBTriTree;

public:

	Impl(const std::string& path, float size);
	~Impl();
	void Draw();
	void DrawAABB();
	glm::vec3 ClosestPoint(const glm::vec3& p, glm::vec3& normal);
	glm::vec3 ClosestPointAABB(const glm::vec3& p, glm::vec3& normal);
	float Distance(const glm::vec3 p, glm::vec3& normal);

private:

	glm::vec3 ClosestPointTriangle(
		const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

private:

	std::vector<glm::vec3> vertices;
	std::vector<glm::ivec3> faces;
	TriangleMesh* mesh;
	AABB* aabb;

	KTriList triangles;
	AABBTriTree aabbTree;

};

ObjModel::Impl::Impl( const std::string& path, float size )
{
	// Open file
	std::ifstream ifs(path, std::ios::in);
	if (!ifs)
	{
		THROW_EXCEPTION(Exception::FileError, "Failed to open " + path);
	}

	Util::Get()->ShowStatusMessage("Loading the proxy model");

	std::string line;
	int lineNum = 1;
	while (std::getline(ifs, line))
	{
		if (line.empty()) continue;
		std::stringstream ss(line);
		std::string type;
		ss >> type;
		if (type == "#")
		{
			continue;
		}
		else if (type == "v")
		{
			glm::vec3 v;
			ss >> v.x >> v.y >> v.z;
			vertices.push_back(v);
		}
		else if (type == "f")
		{
			int i0, i1, i2;
			ss >> i0 >> i1 >> i2;
			i0--; i1--; i2--;
			faces.push_back(glm::ivec3(i0, i1, i2));
		}
		else
		{
			THROW_EXCEPTION(Exception::FileError,
				(boost::format("Invalid token on the line %d") % lineNum).str());
		}
		lineNum++;
	}

	// ------------------------------------------------------------

	// AABB
	aabb = new AABB;
	aabb->min = aabb->max = vertices[0];
	for (int i = 1; i < vertices.size(); i++)
	{
		aabb->min = glm::min(aabb->min, vertices[i]);
		aabb->max = glm::max(aabb->max, vertices[i]);
	}

	// ------------------------------------------------------------

	// Scale proxy model
	float len = 0.0f;
	len = glm::max(len, glm::abs(aabb->min.x - aabb->max.x));
	len = glm::max(len, glm::abs(aabb->min.y - aabb->max.y));
	len = glm::max(len, glm::abs(aabb->min.z - aabb->max.z));
	glm::mat3 scale = glm::mat3(glm::scale(glm::mat4(1.0f), glm::vec3(size / len)));
	for (int i = 0; i < vertices.size(); i++)
	{
		vertices[i] = scale * vertices[i];
	}
	aabb->min = scale * aabb->min;
	aabb->max = scale * aabb->max;

	// ------------------------------------------------------------

	// Construct AABB tree
	Util::Get()->ShowStatusMessage("Constructing AABB tree");
	for (int i = 0; i < faces.size(); i++)
	{
		glm::vec3& v0 = vertices[faces[i].x];
		glm::vec3& v1 = vertices[faces[i].y];
		glm::vec3& v2 = vertices[faces[i].z];
		triangles.push_back(K::Triangle_3(
			K::Point_3(v0.x, v0.y, v0.z),
			K::Point_3(v1.x, v1.y, v1.z),
			K::Point_3(v2.x, v2.y, v2.z)));
	}
	aabbTree.rebuild(triangles.begin(), triangles.end());
	aabbTree.accelerate_distance_queries();
	Util::Get()->ShowStatusMessage("AABB tree is constructed; creating GL triangle mesh");

	// ------------------------------------------------------------

	// Create mesh for GL rendering
	mesh = new TriangleMesh;
	mesh->AddAttribute(VertexStream::POSITION, sizeof(glm::vec3));
	mesh->AddAttribute(VertexStream::NORMAL, sizeof(glm::vec3));
	mesh->Begin();
	int index = 0;
	for (int i = 0; i < faces.size(); i++)
	{
		glm::vec3& v0 = vertices[faces[i].x];
		glm::vec3& v1 = vertices[faces[i].y];
		glm::vec3& v2 = vertices[faces[i].z];
		glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
		mesh->AddVertex(VertexStream::POSITION, v0);
		mesh->AddVertex(VertexStream::POSITION, v1);
		mesh->AddVertex(VertexStream::POSITION, v2);
		mesh->AddVertex(VertexStream::NORMAL, normal);
		mesh->AddVertex(VertexStream::NORMAL, normal);
		mesh->AddVertex(VertexStream::NORMAL, normal);
		mesh->AddIndex(index, index + 1, index + 2);
		index += 3;
	}
	mesh->End();
}

ObjModel::Impl::~Impl()
{
	SAFE_DELETE(aabb);
	SAFE_DELETE(mesh);
}

glm::vec3 ObjModel::Impl::ClosestPoint( const glm::vec3& p, glm::vec3& normal )
{
	float mind2 = FLT_MAX;
	glm::vec3 minp;
	for (int i = 0; i < faces.size(); i++)
	{
		glm::vec3& v0 = vertices[faces[i].x];
		glm::vec3& v1 = vertices[faces[i].y];
		glm::vec3& v2 = vertices[faces[i].z];
		glm::vec3& pp = ClosestPointTriangle(p, v0, v1, v2);
		float d = glm::distance2(p, pp);
		if (d < mind2) {
			mind2 = d;
			minp = pp;
			normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
		}
	}
	return minp;
}

void ObjModel::Impl::Draw()
{
	mesh->Draw();
}

float ObjModel::Impl::Distance( const glm::vec3 p, glm::vec3& normal )
{
	return glm::distance(p, ClosestPoint(p, normal));
}

glm::vec3 ObjModel::Impl::ClosestPointTriangle( const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c )
{
	// Closest point is A
	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 ap = p - a;
	float d1 = glm::dot(ab, ap);
	float d2 = glm::dot(ac, ap);
	if (d1 <= 0.0f && d2 <= 0.0f)
	{
		return a;
	}

	// Closest point is B
	glm::vec3 bp = p - b;
	float d3 = glm::dot(ab, bp);
	float d4 = glm::dot(ac, bp);
	if (d3 >= 0.0f && d4 <= d3)
	{
		return b;
	}

	// Closest point is on AB
	float vc = d1*d4 - d3*d2;
	if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
	{
		float v = d1 / (d1 - d3);
		return a + v * ab;
	}

	// Closest point is C
	glm::vec3 cp = p - c;
	float d5 = glm::dot(ab, cp);
	float d6 = glm::dot(ac, cp);
	if (d6 >= 0.0f && d5 <= d6)
	{
		return c;
	}

	// Closest point is on AC
	float vb = d5*d2 - d1*d6;
	if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
	{
		float w = d2 / (d2 - d6);
		return a + w * ac;
	}

	// Closest point is on BC
	float va = d3*d6 - d5*d4;
	if (va <= 0.0f && d4 >= d3 && d5 >= d6)
	{
		float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		return b + w * (c - b);
	}

	// Closest point is in ABC
	float denom = 1.0f / (va + vb + vc);
	float v = vb * denom;
	float w = vc * denom;
	return a + ab * v + ac * w;
}

void ObjModel::Impl::DrawAABB()
{
	aabb->Draw();
}

glm::vec3 ObjModel::Impl::ClosestPointAABB( const glm::vec3& p, glm::vec3& normal )
{
	K::Point_3 point(p.x, p.y, p.z);
	AABBTriTree::Point_and_primitive_id pp = aabbTree.closest_point_and_primitive(point);
	AABBTriPrimitive::Id id = pp.second; // iterator
	glm::vec3 v0(id->vertex(0).x(), id->vertex(0).y(), id->vertex(0).z());
	glm::vec3 v1(id->vertex(1).x(), id->vertex(1).y(), id->vertex(1).z());
	glm::vec3 v2(id->vertex(2).x(), id->vertex(2).y(), id->vertex(2).z());
	normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
	return glm::vec3(pp.first.x(), pp.first.y(), pp.first.z());
}

// ------------------------------------------------------------

ObjModel::ObjModel( const std::string& path, float size )
	: pimpl(new Impl(path, size))
{

}

ObjModel::~ObjModel()
{

}

void ObjModel::Draw()
{
	pimpl->Draw();
}

void ObjModel::DrawAABB()
{
	pimpl->DrawAABB();
}

glm::vec3 ObjModel::ClosestPoint( const glm::vec3& p, glm::vec3& normal )
{
#if 0
	return pimpl->ClosestPoint(p, normal);
#else
	return pimpl->ClosestPointAABB(p, normal);
#endif
}

float ObjModel::Distance( const glm::vec3 p, glm::vec3& normal )
{
	return pimpl->Distance(p, normal);
}
