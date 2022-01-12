#ifndef TEXTURE_H
#define TEXTURE_H
#include "QtGui/QImage"
#include "QtCore/QString"

class Texture
{
public:
    Texture(unsigned char *data,int width,int height,unsigned int comp);
    Texture(const QString path);
    ~Texture();
    inline unsigned int getTextureId(){return textureID;}
    void updateBuffer(unsigned char *data);
    inline float get_w_h_ratio(){return w_h_ratio;}
    inline int getW(){return mWidth;};
    inline int getH(){return mHeight;}
private:
    unsigned int textureID;
    void TextureFromFile(const QString path);
    void TextureFromData(unsigned char *data,int width,int height,unsigned int comp);
    int mWidth;
    int mHeight;
    int mComp;
    float w_h_ratio = 1.0f;
};


#endif // TEXTURE_H
