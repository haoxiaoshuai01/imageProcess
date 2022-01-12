#include "OpenglWindow.h"
#include <sys/timeb.h>
#include "QtWidgets/QApplication"
#include "stb/stb_image.h"
static const char *textureVS = "#version 430 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec2 aTexCoord;\n"
"out vec2 TexCoord;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0f);\n"
"   TexCoord = aTexCoord;\n"
"}\0";
static const char *textureFS = "#version 430 core\n"
"out vec4 FragColor;\n"
"in vec2 TexCoord;\n"
"uniform sampler2D texture1;\n"
"void main()\n"
"{\n"
"   FragColor = texture(texture1, TexCoord);\n"
"}\n\0";
long long systemtime()
{
	timeb t;
	ftime(&t);
	return t.time * 1000 + t.millitm;
}
OpenglWindow *OpenglWindow::ptr = nullptr;
void OpenglWindow::initializeGL()
{
	makeCurrent();
	if (!gladLoadGL())
	{
		std::cout << "init failed" << std::endl;
		return;
	}
	lastFrameTime = systemtime();

	initImage();
}
void OpenglWindow::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
}
void OpenglWindow::paintGL()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	shaderTexture->use();
	shaderTexture->setInt("texture1", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->getTextureId());
	glBindVertexArray(VAO);
	glDrawElements(drawModel, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	frameDelta = systemtime() - lastFrameTime;
	lastFrameTime = systemtime();
}
void OpenglWindow::initImage()
{
	shaderTexture = new Shader(textureVS, textureFS);
	int mWidth;
	int mHeight;
	int mComp;
	unsigned char *data = stbi_load(QString(QApplication::applicationDirPath() + "/test2.jpg").toStdString().c_str(),
		&mWidth, &mHeight, &mComp, 0);
	
	texture = new Texture(data,mWidth,mHeight,mComp);
	stbi_image_free(data);
	

	verticesTexCoords.push_back(Eigen::Vector2f(1.0f, 0.0f));
	verticesTexCoords.push_back(Eigen::Vector2f(1.0f, 1.0f));
	verticesTexCoords.push_back(Eigen::Vector2f(0.0f, 1.0f));
	verticesTexCoords.push_back(Eigen::Vector2f(0.0f, 0.0f));
	verticesPostion.push_back(Eigen::Vector3f(1,1,0));
	verticesPostion.push_back(Eigen::Vector3f(1,-1,0));
	verticesPostion.push_back(Eigen::Vector3f(-1,-1,0));
	verticesPostion.push_back(Eigen::Vector3f(-1,1,0));
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(3);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(3);
	setupMesh();
}
void OpenglWindow::setupMesh()
{
	// create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(2, VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	// A great thing about structs is that their memory layout is sequential for all its items.
	// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
	// again translates to 3/2 floats which translates to a byte array.
	glBufferData(GL_ARRAY_BUFFER, verticesPostion.size() * sizeof(Eigen::Vector3f), &verticesPostion[0], GL_DYNAMIC_DRAW);
	// set the vertex attribute pointers
	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Eigen::Vector3f), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	if (verticesTexCoords.size())
	{
		glBufferData(GL_ARRAY_BUFFER, verticesTexCoords.size() * sizeof(Eigen::Vector2f), &verticesTexCoords[0], GL_DYNAMIC_DRAW);
	}
	else
	{
		glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STATIC_DRAW);
	}

	//vertex texture coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Eigen::Vector2f), (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}
OpenglWindow::OpenglWindow()
{

}
