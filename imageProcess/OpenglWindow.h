#pragma once
#include "glad/glad.h"
#include "QtGui/qopenglwindow.h"
#include <iostream>
#include "Eigen/Dense"
#include <vector>
#include "Eigen/Core"
#include "shader.h"
#include  "texture.h"
#include "QtWidgets/QOpenGLWidget"
class OpenglWindow :public QOpenGLWidget
{
public:
	static OpenglWindow *Instance() {
		if (ptr == nullptr)
			return ptr = new OpenglWindow();
		else
			return ptr;
	};
protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;
private:
	void initImage();
	void setupMesh();
	void setUPFinalFbo();
private:
	std::vector<Eigen::Vector3f> verticesPostion;
	std::vector<Eigen::Vector2f> verticesTexCoords;
	std::vector<unsigned int> indices;
	unsigned int drawModel = GL_TRIANGLES;;
	unsigned int VAO;
	unsigned int VBO[2];
	unsigned int EBO;

	Shader *shaderTexture;
	Texture *texture;

	static OpenglWindow *ptr;
	explicit OpenglWindow();
	long long lastFrameTime = 0;
	long long frameDelta = 0;

	unsigned char * fboData;


	unsigned int FinalFboID;
	unsigned int FinalFboTexture;
	unsigned int screenFaceVAO;
	unsigned int screenFaceVBO;
	Shader*screenShader;

	int window_w = 0;
	int window_h = 0;
};

