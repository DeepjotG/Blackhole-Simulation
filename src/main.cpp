#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader_s.h>
#include <vector>
#include <cmath>
#include <gl/GL.h>
#include <tuple>

using namespace std; 

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void circle(float x, float y, float radius);

vector<float> BlackholeVertices;

struct Window {
    float width = 1000;
    float length = 400;
    GLFWwindow* window;

    Window() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, length, "Black-Hole Simulation", NULL, NULL);

        if (window == NULL)
        {
            cout << "Failed to create GLFW window" << endl;
            glfwTerminate();
        }

        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            cout << "Failed to initialize GLAD" << endl;
        }

    }    
    
};

struct Ray{
    glm::vec3 pos;
    unsigned int VAO, VBO;
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec4 color = glm::vec4(255.0f,255.0f,255.0f,1.0f);

    Ray(glm::vec3 position) {
        pos = position;
    }

    void BufferPos() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(pos), &pos[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);
    }

    void draw(glm::vec3 translation, Shader Obj) {
        
        
        model = glm::translate(model, translation);
        Obj.mat4("model", model);
        Obj.vec4("ourColor", color);


        glEnable(GL_POINT_SIZE);
        glPointSize(6.0f);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, 1);
        glDisable(GL_POINT_SIZE);
    }
    
};


int main()
{
    glm::vec4 BColor = glm::vec4(255.0f,0.0f,0.0f,1.0f);
    
    Window GameWindow;
    vector<Ray> LightRays;

    for (auto [i, j] = tuple{0,0}; i < 6; i++) {
        Ray Light(glm::vec3(0.0f, j ,0.0f));
        Light.BufferPos();
        LightRays.push_back(Light);
        j = j + 80.0f;
    }

    Shader ourShader("Shaders/Shader.vs", "Shaders/Shader.fs");

    glm::mat4 ortho = glm::ortho(0.0f, float(GameWindow.width), 0.0f, float(GameWindow.length), -1.0f, 100.0f);



    circle(700.0f, 200.0f, 100.0f);

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * BlackholeVertices.size(), &BlackholeVertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    while(!glfwWindowShouldClose(GameWindow.window))
    {
        processInput(GameWindow.window);
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        
        ourShader.use();
        ourShader.mat4("ortho", ortho);

        ourShader.mat4("model", glm::mat4(1.0f));
        ourShader.vec4("ourColor", BColor);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, BlackholeVertices.size()/3);
        glBindVertexArray(0);

        for (int i = 0; i < LightRays.size(); i++){
            LightRays[i].draw(glm::vec3(0.25f, 0.0f, 0.0f), ourShader);
        }

        glfwSwapBuffers(GameWindow.window);
        glfwPollEvents();
    
    }


    glfwTerminate();
    return 0;

}


void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
} 


void circle(float x, float y, float radius) {
    float centerx = x;
    float centery = y;
    BlackholeVertices.push_back(x);
    BlackholeVertices.push_back(y);
    BlackholeVertices.push_back(0.0f);
    for (int i = 0; i <= 360; i += 5) {
        float angle = i * 3.14159f / 180.0f;
        x = radius * cos(angle) + centerx;
        y = radius * sin(angle) + centery;
        BlackholeVertices.push_back(x);
        BlackholeVertices.push_back(y);
        BlackholeVertices.push_back(0.0f);

    }  
}