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
#include <vector>
#include <fstream>

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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
int setupShader();
int setupGeometry();
GLuint loadTexture(std::string filePath, int &width, int &height);
void loadTrajectoryPoints(std::vector<glm::vec3> &points, const std::string &filename);
void saveTrajectoryPoints(const std::vector<glm::vec3> &points, const std::string &filename);

const GLuint WIDTH = 1000, HEIGHT = 1000;

std::vector<glm::vec3> trajectoryPoints1 = {
    glm::vec3(0.0f, 0.0f, -3.0f),
    glm::vec3(2.0f, 0.0f, -3.0f),
    glm::vec3(2.0f, 2.0f, -3.0f),
    glm::vec3(0.0f, 2.0f, -3.0f),
    glm::vec3(0.0f, 0.0f, -3.0f)};
size_t currentTargetIndex1 = 0;
glm::vec3 cubePosition1 = trajectoryPoints1[0];

std::vector<glm::vec3> trajectoryPoints2 = {
    glm::vec3(1.5f, 0.0f, -3.0f),
    glm::vec3(1.5f, -2.0f, -3.0f),
    glm::vec3(-1.5f, -2.0f, -3.0f),
    glm::vec3(-1.5f, 0.0f, -3.0f),
    glm::vec3(1.5f, 0.0f, -3.0f)};
size_t currentTargetIndex2 = 0;
glm::vec3 cubePosition2 = trajectoryPoints2[0];

float moveSpeed = 1.0f;
int currentCube = 1;

class Camera
{
public:
    glm::vec3 position;
    glm::vec3 target;
    float radius;
    float yaw;
    float pitch;

    Camera(glm::vec3 focus = glm::vec3(0.0f, 0.0f, -3.0f), float startRadius = 6.0f, float startYaw = -90.0f, float startPitch = 0.0f)
        : target(focus), radius(startRadius), yaw(startYaw), pitch(startPitch)
    {
        updatePosition();
    }

    glm::mat4 getViewMatrix()
    {
        return glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void moveForward(float delta)
    {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        target += delta * glm::normalize(front);
        updatePosition();
    }

    void moveRight(float delta)
    {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        glm::vec3 right = glm::normalize(glm::cross(glm::normalize(front), glm::vec3(0.0f, 1.0f, 0.0f)));
        target += delta * right;
        updatePosition();
    }

    void moveUp(float delta)
    {
        target.y += delta;
        updatePosition();
    }

    void rotate(float deltaYaw, float deltaPitch)
    {
        yaw += deltaYaw;
        pitch += deltaPitch;
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
        updatePosition();
    }

private:
    void updatePosition()
    {
        position.x = target.x + radius * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
        position.y = target.y + radius * sin(glm::radians(pitch));
        position.z = target.z + radius * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    }
};

Camera camera(glm::vec3(0.0f, 0.0f, -3.0f), 6.0f, -90.0f, 0.0f);

const GLchar* vertexShaderSource = R"(
	#version 450
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec2 texc;
	layout(location = 2) in vec3 normal;
	uniform mat4 projection;
	uniform mat4 view;
	uniform mat4 model;
	out vec2 texCoord;
	out vec3 FragPos;
	out vec3 Normal;
	void main()
	{
		gl_Position = projection * view * model * vec4(position, 1.0);
		texCoord = texc;
		FragPos  = vec3(model * vec4(position, 1.0));
		Normal   = mat3(transpose(inverse(model))) * normal;
	}
	)";

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

int main()
{
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Inara!", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	GLuint shaderID = setupShader();
	GLuint VAO = setupGeometry();

	int imgWidth, imgHeight;
	GLuint textID = loadTexture("../assets/tex/pixelWall.png", imgWidth, imgHeight);

	glUseProgram(shaderID);

    glUniform1i(glGetUniformLocation(shaderID, "texBuff"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textID);

    GLint projLoc  = glGetUniformLocation(shaderID, "projection");
    GLint viewLoc  = glGetUniformLocation(shaderID, "view");
    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    GLint viewPosLoc = glGetUniformLocation(shaderID, "viewPos");

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)WIDTH / (float)HEIGHT,
        0.1f, 100.0f
    );
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glm::vec3 lightPos  (3.0f, 3.0f, 3.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

	glUniform3fv(glGetUniformLocation(shaderID, "lightPos"),  1, glm::value_ptr(lightPos));
	glUniform3fv(glGetUniformLocation(shaderID, "lightColor"),1, glm::value_ptr(lightColor));

	glEnable(GL_DEPTH_TEST);

    float lastFrameTime = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
        float currentFrameTime = glfwGetTime();
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        glm::vec3 target1 = trajectoryPoints1[currentTargetIndex1];
        glm::vec3 dir1 = glm::normalize(target1 - cubePosition1);
        float dist1 = glm::length(target1 - cubePosition1);
        if(dist1 < 0.05f)
            currentTargetIndex1 = (currentTargetIndex1 + 1) % trajectoryPoints1.size();
        else
            cubePosition1 += dir1 * moveSpeed * deltaTime;

        glm::vec3 target2 = trajectoryPoints2[currentTargetIndex2];
        glm::vec3 dir2 = glm::normalize(target2 - cubePosition2);
        float dist2 = glm::length(target2 - cubePosition2);
        if(dist2 < 0.05f)
            currentTargetIndex2 = (currentTargetIndex2 + 1) % trajectoryPoints2.size();
        else
            cubePosition2 += dir2 * moveSpeed * deltaTime;

		glfwPollEvents();
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = camera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera.position));

		float angle = (GLfloat)glfwGetTime() * direction;

		glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePosition1);
		model = glm::scale(model, glm::vec3(scale));
		if (rotateX)
			model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
		else if (rotateY)
			model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		else if (rotateZ)
			model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glm::mat4 model2 = glm::mat4(1.0f);
		model2 = glm::translate(model2, cubePosition2);
		model2 = glm::scale(model2, glm::vec3(scale));
		if (rotateX)
			model2 = glm::rotate(model2, angle, glm::vec3(1.0f, 0.0f, 0.0f));
		else if (rotateY)
			model2 = glm::rotate(model2, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		else if (rotateZ)
			model2 = glm::rotate(model2, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}
	glDeleteVertexArrays(1, &VAO);
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	const float moveSpeed = 0.1f;
	const float rotateSpeed = 2.0f;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
		camera.rotate(0.0f, rotateSpeed);
	if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
		camera.rotate(0.0f, -rotateSpeed);
	if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
		camera.rotate(-rotateSpeed, 0.0f);
	if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
		camera.rotate(rotateSpeed, 0.0f);

	if (key == GLFW_KEY_I && (action == GLFW_PRESS || action == GLFW_REPEAT))
		camera.moveForward(moveSpeed);
	if (key == GLFW_KEY_K && (action == GLFW_PRESS || action == GLFW_REPEAT))
		camera.moveForward(-moveSpeed);
	if (key == GLFW_KEY_L && (action == GLFW_PRESS || action == GLFW_REPEAT))
		camera.moveRight(moveSpeed);
	if (key == GLFW_KEY_J && (action == GLFW_PRESS || action == GLFW_REPEAT))
		camera.moveRight(-moveSpeed);

	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		rotateX = true; rotateY = false; rotateZ = false; direction = -1;
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		rotateX = false; rotateY = true; rotateZ = false; direction = -1;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		rotateX = true; rotateY = false; rotateZ = false; direction = 1;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		rotateX = false; rotateY = true; rotateZ = false; direction = 1;
	}
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		rotateX = false; rotateY = false; rotateZ = true; direction = 1;
	}
	if (key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		rotateX = false; rotateY = false; rotateZ = true; direction = -1;
	}

	if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		scale += 0.1f;
	}
	if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
		scale -= 0.1f;
		if (scale < 0.1f)
			scale = 0.1f;
	}

    if(action == GLFW_PRESS)
    {
        if(key == GLFW_KEY_1) currentCube = 1;
        else if(key == GLFW_KEY_2) currentCube = 2;

        std::string basePath = "trajectories/";

        if(key == GLFW_KEY_P)
        {
            std::string filename = basePath + "traj_cubo" + std::to_string(currentCube) + ".txt";
            if(currentCube == 1) saveTrajectoryPoints(trajectoryPoints1, filename);
            else if(currentCube == 2) saveTrajectoryPoints(trajectoryPoints2, filename);
        }
        else if(key == GLFW_KEY_O)
        {
            std::string filename = basePath + "traj_cubo" + std::to_string(currentCube) + ".txt";
            if(currentCube == 1) { loadTrajectoryPoints(trajectoryPoints1, filename); currentTargetIndex1 = 0; }
            else if(currentCube == 2) { loadTrajectoryPoints(trajectoryPoints2, filename); currentTargetIndex2 = 0; }
        }
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
	    -0.5f,-0.5f, 0.5f,  0.0f,0.0f,  0.0f,0.0f,1.0f,
	     0.5f,-0.5f, 0.5f,  1.0f,0.0f,  0.0f,0.0f,1.0f,
	     0.5f, 0.5f, 0.5f,  1.0f,1.0f,  0.0f,0.0f,1.0f,
	    -0.5f,-0.5f, 0.5f,  0.0f,0.0f,  0.0f,0.0f,1.0f,
	     0.5f, 0.5f, 0.5f,  1.0f,1.0f,  0.0f,0.0f,1.0f,
	    -0.5f, 0.5f, 0.5f,  0.0f,1.0f,  0.0f,0.0f,1.0f,
	    -0.5f,-0.5f,-0.5f,  1.0f,0.0f,  0.0f,0.0f,-1.0f,
	     0.5f, 0.5f,-0.5f,  0.0f,1.0f,  0.0f,0.0f,-1.0f,
	     0.5f,-0.5f,-0.5f,  0.0f,0.0f,  0.0f,0.0f,-1.0f,
	    -0.5f,-0.5f,-0.5f,  1.0f,0.0f,  0.0f,0.0f,-1.0f,
	    -0.5f, 0.5f,-0.5f,  1.0f,1.0f,  0.0f,0.0f,-1.0f,
	     0.5f, 0.5f,-0.5f,  0.0f,1.0f,  0.0f,0.0f,-1.0f,
	    -0.5f,-0.5f,-0.5f,  0.0f,0.0f, -1.0f,0.0f,0.0f,
	    -0.5f,-0.5f, 0.5f,  1.0f,0.0f, -1.0f,0.0f,0.0f,
	    -0.5f, 0.5f, 0.5f,  1.0f,1.0f, -1.0f,0.0f,0.0f,
	    -0.5f,-0.5f,-0.5f,  0.0f,0.0f, -1.0f,0.0f,0.0f,
	    -0.5f, 0.5f, 0.5f,  1.0f,1.0f, -1.0f,0.0f,0.0f,
	    -0.5f, 0.5f,-0.5f,  0.0f,1.0f, -1.0f,0.0f,0.0f,
	     0.5f,-0.5f,-0.5f,  1.0f,0.0f,  1.0f,0.0f,0.0f,
	     0.5f, 0.5f, 0.5f,  0.0f,1.0f,  1.0f,0.0f,0.0f,
	     0.5f,-0.5f, 0.5f,  0.0f,0.0f,  1.0f,0.0f,0.0f,
	     0.5f,-0.5f,-0.5f,  1.0f,0.0f,  1.0f,0.0f,0.0f,
	     0.5f, 0.5f,-0.5f,  1.0f,1.0f,  1.0f,0.0f,0.0f,
	     0.5f, 0.5f, 0.5f,  0.0f,1.0f,  1.0f,0.0f,0.0f,
	    -0.5f, 0.5f,-0.5f,  0.0f,1.0f, 0.0f,1.0f,0.0f,
	    -0.5f, 0.5f, 0.5f,  0.0f,0.0f, 0.0f,1.0f,0.0f,
	     0.5f, 0.5f, 0.5f,  1.0f,0.0f, 0.0f,1.0f,0.0f,
	    -0.5f, 0.5f,-0.5f,  0.0f,1.0f, 0.0f,1.0f,0.0f,
	     0.5f, 0.5f, 0.5f,  1.0f,0.0f, 0.0f,1.0f,0.0f,
	     0.5f, 0.5f,-0.5f,  1.0f,1.0f, 0.0f,1.0f,0.0f,
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

void loadTrajectoryPoints(std::vector<glm::vec3> &points, const std::string &filename)
{
    std::ifstream inFile(filename);
    if(!inFile.is_open()) return;
    points.clear();
    glm::vec3 p;
    while(inFile >> p.x >> p.y >> p.z) points.push_back(p);
    inFile.close();
}

void saveTrajectoryPoints(const std::vector<glm::vec3> &points, const std::string &filename)
{
    std::ofstream outFile(filename);
    if(!outFile.is_open()) return;
    for(const auto &p : points) outFile << p.x << ' ' << p.y << ' ' << p.z << '\n';
    outFile.close();
}