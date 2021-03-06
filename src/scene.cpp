#include "scene.h"
#include "tracer.h"
#include "ColorTerminal.h"

using std::string;
using std::cerr;
using std::endl;

CScene::CScene(const char* path)
{
	load_model(path);
}

CScene::CScene(CModel& loaded_model)
{
	model = &loaded_model;
}

void CScene::load_model(const char* path)
{
	CModel current_model(path);
    model = &current_model;
}

void CScene::save_image(const Image &im, const char *path)
{
    BMP out;
    out.SetSize(im.n_cols, im.n_rows);

    uint r, g, b;
    RGBApixel p;
    p.Alpha = 255;
    for (uint i = 0; i < im.n_rows; ++i) {
        for (uint j = 0; j < im.n_cols; ++j) {
            std::tie(r, g, b) = im(i, j);
            p.Red = r; p.Green = g; p.Blue = b;
            out.SetPixel(j, i, p);
        }
    }

    if (!out.WriteToFile(path))
        throw string("Error writing file ") + string(path);
}

Image CScene::load_image(const char *path)
{
    BMP in;

    if (!in.ReadFromFile(path))
        throw string("Error reading file ") + string(path);

    Image res(in.TellHeight(), in.TellWidth());

    for (uint i = 0; i < res.n_rows; ++i) {
        for (uint j = 0; j < res.n_cols; ++j) {
            RGBApixel *p = in(j, i);
            res(i, j) = std::make_tuple(p->Red, p->Green, p->Blue);
        }
    }

    return res;
}

void CScene::place_router()
{
	SRouter router(vec3(0.f, 0.f, 0.f), 200.f, vec3(1.f, 0.f, 0.f));
}

CMesh::CMesh(std::vector<vec3> Vertices, std::vector<vec3> Normals)
{
    vertices = Vertices;
	normals = Normals;
}

CModel::CModel(const char* filename, const vec3& pos)
{
    position = pos;
    float max = std::numeric_limits<float>::max();
 	min_x = min_y = min_z = max;
 	max_x = max_y = max_z = -max;
    bool loaded = load(filename);
    if(!loaded){
        cerr << "Model didn't load" << endl;
        exit(0);
    }
}

bool CModel::load(const char* filename)
{
	Assimp::Importer import;
    const aiScene *scene = import.ReadFile(filename, aiProcess_Triangulate);	
	
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        cerr << rcol << "Assimp import error: " << reset << import.GetErrorString() << endl;
        return false;
    }

    calc_node(scene->mRootNode, scene);
    initialize();
    setBound();

    return true;
}

void CModel::calc_node(aiNode *node, const aiScene *scene)
{
    for(uint i = 0; i < node->mNumMeshes; ++i){
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        meshes.push_back(getMesh(mesh));			
    }
        // same for children
    for(uint i = 0; i < node->mNumChildren; ++i){
        calc_node(node->mChildren[i], scene);
    }
}  

CMesh CModel::getMesh(aiMesh *mesh)
{
    std::vector<vec3> vertices;
    std::vector<vec3> normals;

    for(uint i = 0; i < mesh->mNumVertices; ++i){
    	vec3 v;
    	v.x = mesh->mVertices[i].x;
    	v.y = mesh->mVertices[i].y;
    	v.z = mesh->mVertices[i].z;
        v += position; 
        vertices.push_back(v);
        v.x = mesh->mNormals[i].x;
        v.y = mesh->mNormals[i].y;
        v.z = mesh->mNormals[i].z;
        normals.push_back(v);
    }

    return CMesh(vertices, normals);
}  

void CModel::initialize()
{
	for(size_t i = 0; i < meshes.size(); ++i){
		for(size_t j = 0; j < meshes[i].vertices.size(); j+=3){
			vec3* vp = &meshes[i].vertices[j];
            vec3* np = &meshes[i].normals[j];
            checkBound(*vp);
			checkBound(*(vp+1));
			checkBound(*(vp+2));
			STriangle temp(*vp, *(vp+1), *(vp+2), *np);
			triangles.push_back(temp);
		}
	}
}

void CModel::checkBound(vec3 vertice)
{
    min_x = vertice.x < min_x ? vertice.x : min_x;
	min_y = vertice.y < min_y ? vertice.y : min_y;
	min_z = vertice.z < min_z ? vertice.z : min_z;
    
    max_x = vertice.x > max_x ? vertice.x : max_x;
	max_y = vertice.y > max_y ? vertice.y : max_y;
	max_z = vertice.z > max_z ? vertice.z : max_z;
}

void CModel::setBound()
{
	topleft = vec3(min_x, max_y, max_z);
	botright = vec3(max_x, min_y, min_z);
}

// void CModel::addModel(const char* path, const vec3& pos){}