
#include "../model.h"
#include <string.h>  // memcpy
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <drx3D/Common/b3Logging.h>

namespace TinyRender
{
Model::Model(tukk filename) : verts_(), faces_(), norms_(), uv_(), diffusemap_(), normalmap_(), specularmap_()
{
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;
    STxt line;
    while (!in.eof())
    {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v "))
        {
            iss >> trash;
            Vec3f v;
            for (i32 i = 0; i < 3; i++) iss >> v[i];
            verts_.push_back(v);
        }
        else if (!line.compare(0, 3, "vn "))
        {
            iss >> trash >> trash;
            Vec3f n;
            for (i32 i = 0; i < 3; i++) iss >> n[i];
            norms_.push_back(n);
        }
        else if (!line.compare(0, 3, "vt "))
        {
            iss >> trash >> trash;
            Vec2f uv;
            for (i32 i = 0; i < 2; i++) iss >> uv[i];
            uv_.push_back(uv);
        }
        else if (!line.compare(0, 2, "f "))
        {
            std::vector<Vec3i> f;
            Vec3i tmp;
            iss >> trash;
            while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2])
            {
                for (i32 i = 0; i < 3; i++) tmp[i]--;  // in wavefront obj all indices start at 1, not zero
                f.push_back(tmp);
            }
            faces_.push_back(f);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << " vt# " << uv_.size() << " vn# " << norms_.size() << std::endl;
    load_texture(filename, "_diffuse.tga", diffusemap_);
    load_texture(filename, "_nm_tangent.tga", normalmap_);
    load_texture(filename, "_spec.tga", specularmap_);
}

Model::Model() : verts_(), faces_(), norms_(), uv_(), diffusemap_(), normalmap_(), specularmap_()
{
}

void Model::setDiffuseTextureFromData(u8 *textureImage, i32 textureWidth, i32 textureHeight)
{
    {
        D3_PROFILE("new TGAImage");
        diffusemap_ = TGAImage(textureWidth, textureHeight, TGAImage::RGB);
    }
    TGAColor color;
    color.bgra[3] = 255;

    color.bytespp = 3;
    {
        D3_PROFILE("copy texels");
        memcpy(diffusemap_.buffer(), textureImage, textureHeight * textureWidth * 3);
    }
    {
        D3_PROFILE("flip_vertically");
        diffusemap_.flip_vertically();
    }
}

void Model::loadDiffuseTexture(tukk relativeFileName)
{
    diffusemap_.read_tga_file(relativeFileName);
}

void Model::reserveMemory(i32 numVertices, i32 numIndices)
{
    verts_.reserve(numVertices);
    norms_.reserve(numVertices);
    uv_.reserve(numVertices);
    faces_.reserve(numIndices);
}

void Model::addVertex(float x, float y, float z, float normalX, float normalY, float normalZ, float u, float v)
{
    verts_.push_back(Vec3f(x, y, z));
    norms_.push_back(Vec3f(normalX, normalY, normalZ));
    uv_.push_back(Vec2f(u, v));
}
void Model::addTriangle(i32 vertexposIndex0, i32 normalIndex0, i32 uvIndex0,
                        i32 vertexposIndex1, i32 normalIndex1, i32 uvIndex1,
                        i32 vertexposIndex2, i32 normalIndex2, i32 uvIndex2)
{
    std::vector<Vec3i> f;
    f.push_back(Vec3i(vertexposIndex0, normalIndex0, uvIndex0));
    f.push_back(Vec3i(vertexposIndex1, normalIndex1, uvIndex1));
    f.push_back(Vec3i(vertexposIndex2, normalIndex2, uvIndex2));
    faces_.push_back(f);
}

Model::~Model() {}

i32 Model::nverts()
{
    return (i32)verts_.size();
}

i32 Model::nfaces()
{
    return (i32)faces_.size();
}

std::vector<i32> Model::face(i32 idx)
{
    std::vector<i32> face;
        face.reserve((i32)faces_[idx].size());
        for (i32 i = 0; i < (i32)faces_[idx].size(); i++)
          face.push_back(faces_[idx][i][0]);
        return face;
}


Vec3f Model::vert(i32 i)
{
    return verts_[i];
}

Vec3f Model::vert(i32 iface, i32 nthvert)
{
    return verts_[faces_[iface][nthvert][0]];
}

void Model::load_texture(STxt filename, tukk suffix, TGAImage &img)
{
    STxt texfile(filename);
    size_t dot = texfile.find_last_of('.');
    if (dot != STxt::npos)
    {
        texfile = texfile.substr(0, dot) + STxt(suffix);
        std::cerr << "загрузка файла текстуры " << texfile << " из " << (img.read_tga_file(texfile.c_str()) ? "прошла успешно" : "не удалась") << std::endl;
        img.flip_vertically();
    }
}

TGAColor Model::diffuse(Vec2f uvf)
{
    if (diffusemap_.get_width() && diffusemap_.get_height())
    {
        double val;
        //      bool repeat = true;
        //      if (repeat)
        {
            uvf[0] = std::modf(uvf[0], &val);
            if (uvf[0] < 0)
            {
                uvf[0] = uvf[0] + 1;
            }
            uvf[1] = std::modf(uvf[1], &val);
            if (uvf[1] < 0)
            {
                uvf[1] = uvf[1] + 1;
            }
        }
            Vec2i uv(uvf[0] * diffusemap_.get_width(), uvf[1] * diffusemap_.get_height());
        return diffusemap_.get(uv[0], uv[1]);
    }
    return TGAColor(255, 255, 255, 255);
}


Vec3f Model::normal(Vec2f uvf)
{
    Vec2i uv(uvf[0] * normalmap_.get_width(), uvf[1] * normalmap_.get_height());
    TGAColor c = normalmap_.get(uv[0], uv[1]);
    Vec3f res;
    for (i32 i = 0; i < 3; i++)
        res[2 - i] = (float)c[i] / 255.f * 2.f - 1.f;
    return res;
}

Vec2f Model::uv(i32 iface, i32 nthvert)
{
    return uv_[faces_[iface][nthvert][1]];
}

float Model::specular(Vec2f uvf)
{
    if (specularmap_.get_width() && specularmap_.get_height())
    {
        Vec2i uv(uvf[0] * specularmap_.get_width(), uvf[1] * specularmap_.get_height());
        return specularmap_.get(uv[0], uv[1])[0] / 1.f;
    }
    return 2.0;
}

Vec3f Model::normal(i32 iface, i32 nthvert)
{
    i32 idx = faces_[iface][nthvert][2];
    return norms_[idx].normalize();
}
}
