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
	uniform mat4 projection;
	uniform mat4 model;
	out vec2 texCoord;
	void main()
	{
		gl_Position = projection * model * vec4(position, 1.0);
		texCoord = texc;
	}
	)";

//Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = R"(
	#version 450
	in vec2 texCoord;
	uniform sampler2D texBuff;
	out vec4 color;
	void main()
	{
		color = texture(texBuff, texCoord);
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

	//Muita atenção aqui: alguns ambientes não aceitam essas configurações
	//Você deve adaptar para a versão do OpenGL suportada por sua placa
	//Sugestão: comente essas linhas de código para desobrir a versão e
	//depois atualize (por exemplo: 4.5 com 4 e 5)
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Essencial para computadores da Apple
//#ifdef __APPLE__
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif

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

    // binding da textura
    glUniform1i(glGetUniformLocation(shaderID, "texBuff"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture  (GL_TEXTURE_2D, textID);

    // envia projection
    GLint projLoc = glGetUniformLocation(shaderID, "projection");
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)WIDTH / (float)HEIGHT,
        0.1f, 100.0f
    );
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // prepara modelLoc
	glm::mat4 model = glm::mat4(1.0f); // matriz identidade
    // afasta o cubo da câmera
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f));
	GLint modelLoc = glGetUniformLocation(shaderID, "model");
	//
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glEnable(GL_DEPTH_TEST);

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat)glfwGetTime();

		model = glm::mat4(1.0f);
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
		
		// Chamada de desenho - drawcall
		// Poligono Preenchido - GL_TRIANGLES
		glBindVertexArray(VAO);
		glBindTexture(GL_TEXTURE_2D, textID);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Chamada de desenho - drawcall
		// CONTORNO - GL_POINTS
		glDrawArrays(GL_POINTS, 0, 36);
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
		glDrawArrays(GL_POINTS, 0, 36);
		glBindVertexArray(0);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
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

//Esta função está basntante hardcoded - objetivo é compilar e "buildar" um programa de
// shader simples e único neste exemplo de código
// O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
// fragmentShader source no iniçio deste arquivo
// A função retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

// Esta função está bastante hardcoded - objetivo é criar os buffers que armazenam a 
// geometria de um cubo com textura
// Apenas atributo coordenada nos vértices (x,y,z) + coord de textura (s,t)
// A função retorna o identificador do VAO
int setupGeometry()
{
	// Aqui montamos as 6 faces do cubo (36 vértices)
	GLfloat vertices[] = {
	    // face frontal
	   -0.5f,-0.5f, 0.5f,  0.0f,0.0f,
	    0.5f,-0.5f, 0.5f,  1.0f,0.0f,
	    0.5f, 0.5f, 0.5f,  1.0f,1.0f,
	   -0.5f,-0.5f, 0.5f,  0.0f,0.0f,
	    0.5f, 0.5f, 0.5f,  1.0f,1.0f,
	   -0.5f, 0.5f, 0.5f,  0.0f,1.0f,

	   // face traseira
	   -0.5f,-0.5f,-0.5f,  1.0f,0.0f,
	    0.5f, 0.5f,-0.5f,  0.0f,1.0f,
	    0.5f,-0.5f,-0.5f,  0.0f,0.0f,
	   -0.5f,-0.5f,-0.5f,  1.0f,0.0f,
	   -0.5f, 0.5f,-0.5f,  1.0f,1.0f,
	    0.5f, 0.5f,-0.5f,  0.0f,1.0f,

	   // esquerda
	   -0.5f,-0.5f,-0.5f,  0.0f,0.0f,
	   -0.5f,-0.5f, 0.5f,  1.0f,0.0f,
	   -0.5f, 0.5f, 0.5f,  1.0f,1.0f,
	   -0.5f,-0.5f,-0.5f,  0.0f,0.0f,
	   -0.5f, 0.5f, 0.5f,  1.0f,1.0f,
	   -0.5f, 0.5f,-0.5f,  0.0f,1.0f,

	   // direita
	    0.5f,-0.5f,-0.5f,  1.0f,0.0f,
	    0.5f, 0.5f, 0.5f,  0.0f,1.0f,
	    0.5f,-0.5f, 0.5f,  0.0f,0.0f,
	    0.5f,-0.5f,-0.5f,  1.0f,0.0f,
	    0.5f, 0.5f,-0.5f,  1.0f,1.0f,
	    0.5f, 0.5f, 0.5f,  0.0f,1.0f,

	   // topo
	   -0.5f, 0.5f,-0.5f,  0.0f,1.0f,
	   -0.5f, 0.5f, 0.5f,  0.0f,0.0f,
	    0.5f, 0.5f, 0.5f,  1.0f,0.0f,
	   -0.5f, 0.5f,-0.5f,  0.0f,1.0f,
	    0.5f, 0.5f, 0.5f,  1.0f,0.0f,
	    0.5f, 0.5f,-0.5f,  1.0f,1.0f,

	   // fundo (embaixo)
	   -0.5f,-0.5f,-0.5f,  1.0f,1.0f,
	    0.5f,-0.5f, 0.5f,  0.0f,0.0f,
	   -0.5f,-0.5f, 0.5f,  1.0f,0.0f,
	   -0.5f,-0.5f,-0.5f,  1.0f,1.0f,
	    0.5f,-0.5f,-0.5f,  0.0f,1.0f,
	    0.5f,-0.5f, 0.5f,  0.0f,0.0f
	};

	GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula e preenche os buffers
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// posição (layout = 0)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// texcoord (layout = 1)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return VAO;
}

GLuint loadTexture(std::string filePath, int &width, int &height)
{
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // wrapping/filtering
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
