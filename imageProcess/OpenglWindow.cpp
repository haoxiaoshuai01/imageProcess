#include "OpenglWindow.h"
#include <sys/timeb.h>
#include "QtWidgets/QApplication"
#include "stb/stb_image.h"
#include "QtCore/qdebug.h"
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
	fboData = new unsigned char[1080 * 1920 * 4];
	initImage();
	setUPFinalFbo();
	
}
void OpenglWindow::resizeGL(int w, int h)
{
	window_w = w;
	window_h = h;
}
void OpenglWindow::paintGL()
{
	glBindFramebuffer(GL_FRAMEBUFFER, FinalFboID);
	glViewport(0, 0, 1080, 1920);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	shaderTexture->use();
	shaderTexture->setInt("texture1", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->getTextureId());
	glBindVertexArray(VAO);
	glDrawElements(drawModel, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER,this->defaultFramebufferObject());
	glViewport(0, 0, window_w, window_h);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);



	screenShader->use();
	glBindVertexArray(screenFaceVAO);
	glBindTexture(GL_TEXTURE_2D, FinalFboTexture);	// use the color attachment texture as the texture of the quad plane
	glDrawArrays(GL_TRIANGLES, 0, 6);

	


	frameDelta = systemtime() - lastFrameTime;
	lastFrameTime = systemtime();
}
void HE()
{

	int biHeight = 5;
	int biWidth = 5;
	int totalLevel = 9;
	float p[10] = { 0 };
	int a[25] = { 0,0,0,0,1,
				1,2,2,2,2,
				3,3,3,3,3,
				9,9,3,3,3,
				3,0,0,0,0
	};
	for (int i = 0; i < biHeight; i++)
	{
		for (int j = 0; j < biWidth; j++)
		{
			int gray_ = a[i*biWidth + j];
			p[gray_] = p[gray_] + 1.0f / float(biHeight* biWidth);
		}
	}

	{
		for (int i = 0; i < biHeight; i++)
		{
			for (int j = 0; j < biWidth; j++)
			{
				float dTemp = 0;

				int gray_ = a[i*biWidth + j];

				for (BYTE k = 0; k <= gray_; k++)
				{
					dTemp += *(&p[0] + k);
				};

				int target = int(totalLevel * dTemp);

				if (target < 0) target = 0;
				if (target > totalLevel) target = totalLevel;

				// Ð´ÈëÄ¿±êÍ¼Ïñ
				a[i*biWidth + j] = target;
			}
		};

	}

	float p1[10] = { 0 };
	for (int i = 0; i < biHeight; i++)
	{
		for (int j = 0; j < biWidth; j++)
		{
			int gray_ = a[i*biWidth + j];
			p1[gray_] = p1[gray_] + 1.0f / float(biHeight* biWidth);
		}
	}

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
void OpenglWindow::setUPFinalFbo()
{
	// framebuffer configuration
		// -------------------------
	int w = 1080;
	int h = 1920;

	glGenFramebuffers(1, &FinalFboID);
	glBindFramebuffer(GL_FRAMEBUFFER, FinalFboID);
	// create a color attachment texture

	glGenTextures(1, &FinalFboTexture);
	glBindTexture(GL_TEXTURE_2D, FinalFboTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FinalFboTexture, 0);
	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	int w2 = w;
	int h2 = h;
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w2, h2); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());

	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		   // positions   // texCoords
		   -1.0f,  1.0f,  0.0f, 1.0f,
		   -1.0f, -1.0f,  0.0f, 0.0f,
			1.0f, -1.0f,  1.0f, 0.0f,

		   -1.0f,  1.0f,  0.0f, 1.0f,
			1.0f, -1.0f,  1.0f, 0.0f,
			1.0f,  1.0f,  1.0f, 1.0f
	};
	// screen quad VAO

	glGenVertexArrays(1, &screenFaceVAO);
	glGenBuffers(1, &screenFaceVBO);
	glBindVertexArray(screenFaceVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenFaceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	screenShader = new Shader(
		"#version 330 core                                   \n"
		"layout (location = 0) in vec2 aPos;                 \n"
		"layout (location = 1) in vec2 aTexCoords;           \n"
		"                                                    \n"
		"out vec2 TexCoords;                                 \n"
		"                                                    \n"
		"void main()                                         \n"
		"{                                                   \n"
		"    TexCoords = aTexCoords;                         \n"
		"    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);   \n"
		"}                                                   \n"
		,
		"#version 330 core                                    \n"
		"out vec4 FragColor;                                  \n"
		"                                                     \n"
		"in vec2 TexCoords;                                   \n"
		"                                                     \n"
		"uniform sampler2D screenTexture;                     \n"
		"                                                     \n"
		"void main()                                          \n"
		"{                                                    \n"
		"    vec3 col = texture(screenTexture, TexCoords).rgb;\n"
		"    FragColor = vec4(col, 1.0);                      \n"
		"}                                                    \n"
	);
	screenShader->use();
	screenShader->setInt("screenTexture", 0);

	/*glGenBuffers(2, pboPushSteam1);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pboPushSteam1[0]);
	glBufferData(GL_PIXEL_PACK_BUFFER, 1920 * 1080 * 3, nullptr, GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pboPushSteam1[1]);
	glBufferData(GL_PIXEL_PACK_BUFFER, 1920 * 1080 * 3, nullptr, GL_STREAM_READ);*/
}
OpenglWindow::OpenglWindow()
{
	QObject::connect(this, &QOpenGLWidget::frameSwapped, this, [this]() {
		this->update();
	});
}
