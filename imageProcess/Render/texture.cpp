#include "glad/glad.h"
#include "texture.h"
#include "stb/stb_image.h"
#include <iostream>
namespace  Engine{
Texture::Texture(unsigned char *data, int width, int height, unsigned int comp)
{
    mWidth = width;
    mHeight = height;
    mComp = comp;
    w_h_ratio = (float)mWidth/(float)mHeight;
    glGenTextures(1, &textureID);
    TextureFromData(data,width,height,comp);
}

Texture::Texture(const QString path)
{
    glGenTextures(1, &textureID);
    TextureFromFile(path);
}

Texture::~Texture()
{
    glDeleteTextures(1,&textureID);
}

void Texture::updateBuffer(unsigned char *data)
{

    glBindTexture(GL_TEXTURE_2D, textureID);
    GLenum format = GL_RGB;
    if (mComp == 1)
        format = GL_RED;
    else if (mComp == 3)
        format = GL_RGB;
    else if (mComp == 4)
        format = GL_RGBA;

    glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0,
            0,
            mWidth,
            mHeight,
            format,
            GL_UNSIGNED_BYTE,
            data
        );

    glBindTexture( GL_TEXTURE_2D, 0);
}

void Texture::TextureFromFile(const QString path)
{

    //stbi_set_flip_vertically_on_load(true);

    unsigned char *data = stbi_load(path.toStdString().c_str(), &mWidth, &mHeight, &mComp, 0);
    w_h_ratio = (float)mWidth/(float)mHeight;
    TextureFromData(data,mWidth,mHeight,mComp);

    stbi_image_free(data);
}

void Texture::TextureFromData(unsigned char *data, int width, int height, unsigned int comp)
{
    if (data)
    {
        GLenum format = GL_RGB;
        if (comp == 1)
            format = GL_RED;
        else if (comp == 3)
            format = GL_RGB;
        else if (comp == 4)
            format = GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        //glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture( GL_TEXTURE_2D, 0);
    }
    else
    {
        std::cout << "Texture failed to load " << std::endl;
    }
}
}
