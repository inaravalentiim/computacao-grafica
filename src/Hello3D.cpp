/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para as disciplinas de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 07/03/2025
 */

#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry();
GLuint loadTexture(std::string filePath, int &width, int &height);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = R"(
	#version 450
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec2 texc;
	layout(location = 2) in vec3 normal;
	uniform mat4 projection;
	uniform mat4 model;
	out vec2 texCoord;
	out vec3 FragPos;
	out vec3 Normal;
	void main()
	{
		gl_Position = projection * model * vec4(position, 1.0);
		texCoord = texc;
		FragPos  = vec3(model * vec4(position, 1.0));
		Normal   = mat3(transpose(inverse(model))) * normal;
	}
	)";

//Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = R"(
	#version 450
	in vec2 texCoord;
	in vec3 FragPos;
	in vec3 Normal;
	uniform sampler2D texBuff;
	uniform vec3 lightPos;
	uniform vec3 viewPos;
	uniform vec3 lightColor;
	out vec4 color;
	void main()
	{
		vec3 ambient  = 0.2 * lightColor;

		vec3 norm     = normalize(Normal);
		vec3 lightDir = normalize(lightPos - FragPos);
		float diff    = max(dot(norm, lightDir), 0.0);
		vec3 diffuse  = diff * lightColor;

		float shininess = 32.0;
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
		vec3 specular = spec * lightColor;

		vec3 phong = ambient + diffuse + specular;
		vec4 texColor = texture(texBuff, texCoord);
		color = vec4(phong, 1.0) * texColor;
	}
	)";

bool rotateX=false, rotateY=false, rotateZ=false;
int direction=1;
float scale = 1.0f;

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Inara!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	// Gerando um buffer simples, com a geometria de um cubo
	GLuint VAO = setupGeometry();

	int imgWidth, imgHeight;
	GLuint textID = loadTexture("../assets/tex/pixelWall.png", imgWidth, imgHeight);

	glUseProgram(shaderID);

    glUniform1i(glGetUniformLocation(shaderID, "texBuff"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textID);

    GLint projLoc = glGetUniformLocation(shaderID, "projection");
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)WIDTH / (float)HEIGHT,
        0.1f, 100.0f
    );
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	GLint modelLoc = glGetUniformLocation(shaderID, "model");

	glm::vec3 lightPos  (3.0f, 3.0f, 3.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	glm::vec3 viewPos   (0.0f, 0.0f, 0.0f);

	glUniform3fv(glGetUniformLocation(shaderID, "lightPos"),  1, glm::value_ptr(lightPos));
	glUniform3fv(glGetUniformLocation(shaderID, "lightColor"),1, glm::value_ptr(lightColor));
	glUniform3fv(glGetUniformLocation(shaderID, "viewPos"),   1, glm::value_ptr(viewPos));

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat)glfwGetTime();

		glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f));
		model = glm::scale(model, glm::vec3(scale));
		
		if (rotateX && direction == 1)
			model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
		else if (rotateX && direction == -1)
			model = glm::rotate(model, -angle, glm::vec3(1.0f, 0.0f, 0.0f));
		else if (rotateY && direction == 1)
			model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		else if (rotateY && direction == -1)
			model = glm::rotate(model, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
		else if (rotateZ && direction == 1)
			model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		else if (rotateZ && direction == -1)
			model = glm::rotate(model, -angle, glm::vec3(0.0f, 0.0f, 1.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		
		glBindVertexArray(VAO);
		glBindTexture(GL_TEXTURE_2D, textID);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDrawArrays(GL_POINTS,   0, 36);
		glBindVertexArray(0);

		glm::mat4 model2 = glm::mat4(1.0f);
		model2 = glm::translate(model2, glm::vec3(1.5f, 0.0f, -3.0f));
		model2 = glm::scale(model2, glm::vec3(scale));

		if (rotateX && direction == 1)
     		model2 = glm::rotate(model2, angle, glm::vec3(1.0f, 0.0f, 0.0f));
		else if (rotateX && direction == -1)
			model2 = glm::rotate(model2, -angle, glm::vec3(1.0f, 0.0f, 0.0f));
		else if (rotateY && direction == 1)
			model2 = glm::rotate(model2, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		else if (rotateY && direction == -1)
			model2 = glm::rotate(model2, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
		else if (rotateZ && direction == 1)
			model2 = glm::rotate(model2, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		else if (rotateZ && direction == -1)
			model2 = glm::rotate(model2, -angle, glm::vec3(0.0f, 0.0f, 1.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDrawArrays(GL_POINTS,   0, 36);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}
	glDeleteVertexArrays(1, &VAO);
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		rotateX = true;
		rotateY = false;
		rotateZ = false;
		direction = -1;
	}

	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = true;
		rotateZ = false;
		direction = -1;
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		rotateX = true;
		rotateY = false;
		rotateZ = false;
		direction = 1;
	}

	if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = true;
		rotateZ = false;
		direction = 1;
	}

	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = true;
		direction = 1;
	}

	if (key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = true;
		direction = -1;
	}	

	if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		scale += 0.1f;
	}
	if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
		scale -= 0.1f;
		if (scale < 0.1f)
			scale = 0.1f;
	}
}

int setupShader()
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

int setupGeometry()
{
	GLfloat vertices[] = {
	    // frente
	   -0.5f,-0.5f, 0.5f,  0.0f,0.0f,  0.0f,0.0f,1.0f,
	    0.5f,-0.5f, 0.5f,  1.0f,0.0f,  0.0f,0.0f,1.0f,
	    0.5f, 0.5f, 0.5f,  1.0f,1.0f,  0.0f,0.0f,1.0f,
	   -0.5f,-0.5f, 0.5f,  0.0f,0.0f,  0.0f,0.0f,1.0f,
	    0.5f, 0.5f, 0.5f,  1.0f,1.0f,  0.0f,0.0f,1.0f,
	   -0.5f, 0.5f, 0.5f,  0.0f,1.0f,  0.0f,0.0f,1.0f,

	    // trás
	   -0.5f,-0.5f,-0.5f,  1.0f,0.0f,  0.0f,0.0f,-1.0f,
	    0.5f, 0.5f,-0.5f,  0.0f,1.0f,  0.0f,0.0f,-1.0f,
	    0.5f,-0.5f,-0.5f,  0.0f,0.0f,  0.0f,0.0f,-1.0f,
	   -0.5f,-0.5f,-0.5f,  1.0f,0.0f,  0.0f,0.0f,-1.0f,
	   -0.5f, 0.5f,-0.5f,  1.0f,1.0f,  0.0f,0.0f,-1.0f,
	    0.5f, 0.5f,-0.5f,  0.0f,1.0f,  0.0f,0.0f,-1.0f,

	    // esquerda
	   -0.5f,-0.5f,-0.5f,  0.0f,0.0f, -1.0f,0.0f,0.0f,
	   -0.5f,-0.5f, 0.5f,  1.0f,0.0f, -1.0f,0.0f,0.0f,
	   -0.5f, 0.5f, 0.5f,  1.0f,1.0f, -1.0f,0.0f,0.0f,
	   -0.5f,-0.5f,-0.5f,  0.0f,0.0f, -1.0f,0.0f,0.0f,
	   -0.5f, 0.5f, 0.5f,  1.0f,1.0f, -1.0f,0.0f,0.0f,
	   -0.5f, 0.5f,-0.5f,  0.0f,1.0f, -1.0f,0.0f,0.0f,

	    // direita
	    0.5f,-0.5f,-0.5f,  1.0f,0.0f,  1.0f,0.0f,0.0f,
	    0.5f, 0.5f, 0.5f,  0.0f,1.0f,  1.0f,0.0f,0.0f,
	    0.5f,-0.5f, 0.5f,  0.0f,0.0f,  1.0f,0.0f,0.0f,
	    0.5f,-0.5f,-0.5f,  1.0f,0.0f,  1.0f,0.0f,0.0f,
	    0.5f, 0.5f,-0.5f,  1.0f,1.0f,  1.0f,0.0f,0.0f,
	    0.5f, 0.5f, 0.5f,  0.0f,1.0f,  1.0f,0.0f,0.0f,

	    // topo
	   -0.5f, 0.5f,-0.5f,  0.0f,1.0f, 0.0f,1.0f,0.0f,
	   -0.5f, 0.5f, 0.5f,  0.0f,0.0f, 0.0f,1.0f,0.0f,
	    0.5f, 0.5f, 0.5f,  1.0f,0.0f, 0.0f,1.0f,0.0f,
	   -0.5f, 0.5f,-0.5f,  0.0f,1.0f, 0.0f,1.0f,0.0f,
	    0.5f, 0.5f, 0.5f,  1.0f,0.0f, 0.0f,1.0f,0.0f,
	    0.5f, 0.5f,-0.5f,  1.0f,1.0f, 0.0f,1.0f,0.0f,

	    // fundo
	   -0.5f,-0.5f,-0.5f,  1.0f,1.0f, 0.0f,-1.0f,0.0f,
	    0.5f,-0.5f, 0.5f,  0.0f,0.0f, 0.0f,-1.0f,0.0f,
	   -0.5f,-0.5f, 0.5f,  1.0f,0.0f, 0.0f,-1.0f,0.0f,
	   -0.5f,-0.5f,-0.5f,  1.0f,1.0f, 0.0f,-1.0f,0.0f,
	    0.5f,-0.5f,-0.5f,  0.0f,1.0f, 0.0f,-1.0f,0.0f,
	    0.5f,-0.5f, 0.5f,  0.0f,0.0f, 0.0f,-1.0f,0.0f
	};

	GLuint VBO, VAO;
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return VAO;
}

GLuint loadTexture(std::string filePath, int &width, int &height)
{
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int nrChannels;
    unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture: " << filePath << std::endl;
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}
