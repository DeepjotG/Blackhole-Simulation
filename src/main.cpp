#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader_s.h>
#include <vector>
#include <cmath>
#include <gl/GL.h>
#include <tuple>
#include <memory>

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
    unsigned int VAO;
    GLuint SSBO2;
    glm::vec4 color = glm::vec4(255.0f,255.0f,255.0f,1.0f);
    int amount;
    vector<glm::vec4> particles;
    vector<glm::vec4> velocities;
    int windowLength;

    Ray(int amountofRays, int GamewindowLength) {
        amount = amountofRays;
        windowLength = GamewindowLength;
    }

    void CreateParticles() {
        for(auto [i, j] = tuple{0,0}; i < amount; i++){
            particles.push_back(glm::vec4(0.0f, j, 0.0f, 0.0f));
            j = j + float(windowLength/amount);
            velocities.push_back(glm::vec4(0.1f, 0.0f, 0.0f, 0.0f));

        }
        //particles.push_back(glm::vec4(549.930371299, 325, 0.0f, 0.0f));
        //velocities.push_back(glm::vec4(0.25f, 0.0f, 0.0f, 0.0f));
    }


    void BufferPos() {



        glGenVertexArrays(1, &VAO);
        glCreateBuffers(1, &SSBO2);
        glNamedBufferStorage(SSBO2, sizeof(glm::vec4) * particles.size() , (const void*)particles.data() , GL_DYNAMIC_STORAGE_BIT);
        glBindVertexArray(VAO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, SSBO2);

    }

    void draw(Shader Obj) {
                
        Obj.vec4("ourColor", color);
        Obj.setBool("isInstanced", true);


        glEnable(GL_POINT_SIZE);
        glPointSize(6.0f);
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_POINTS, 0, 1, amount);
        glDisable(GL_POINT_SIZE);



    }
    
};


struct Blackhole {
    double mass, r_s, r_isco, r_p;
    double solarMass = 1.989e30;
    double scale = 0.00338666666;
    double c = 299792458;
    double G = 6.6743e-11;
    std::unique_ptr<glm::mat4[]> modelMatrices;
    glm::vec3 position;
    enum : uint8_t {ACTIVE = 0, CAPTURED=1, ESCAPED=2, STUCK=3};
    vector<uint8_t> state;
    vector<int> steps;
    vector<float> h_cache;

    


    Blackhole(glm::vec3 Bposition){
        position = Bposition;
        mass = solarMass * 10.0;
        r_s = ((2 * G * mass)/ (c * c)) * scale;
        r_isco = 3*r_s;
        r_p = ((1.5f) * r_s);
        circle(position.x, position.y, 100.0f);
    }

    glm::vec3 Force(glm::vec3 r_l, double h){
        float total = ((-1.5f) * float(h * h));
        glm::vec3 dis = r_l/float((std::pow(glm::length(r_l), 5.0f)));
        
        return dis * total;
        
    }

    void UpdatingLight(Ray &obj){
        float r_l;
        glm::vec3 distance;

        if (state.size() != (size_t)obj.amount){
            state.assign(obj.amount, ACTIVE);
            steps.assign(obj.amount, 0);
            h_cache.assign(obj.amount, -1.0f);
        }

        for(int i = 0; i < obj.amount; i++){
            if (state[i] == CAPTURED || state[i] == STUCK) { continue; }


            distance = glm::vec3(obj.particles[i]) - position;
            r_l = glm::length(distance);


            if(r_l > r_isco || state[i] == ESCAPED){
                if (h_cache[i] < 0.0f) {           
                    obj.particles[i] += obj.velocities[i];
                } else {                           
                    state[i] = ESCAPED;
                    obj.particles[i] += obj.velocities[i];
                }
                continue;
            }

            if (r_l <= r_isco){
                if (h_cache[i] < 0.0f){
                    h_cache[i] = glm::length(glm::cross(distance, glm::vec3(obj.velocities[i])));
                }   
                
                if(r_l < r_s){
                    state[i] = CAPTURED;
                    //obj.particles[i] = glm::vec4(-1e6f, -1e6f, 0.0f, 0.0f);
                    obj.velocities[i] = glm::vec4(0.0f,0.0f,0.0f,0.0f);
                    std::cout << "stuck because radius less than r_s" << i << endl;
                    continue;
                }

                obj.particles[i] += obj.velocities[i];
                distance = glm::vec3(obj.particles[i]) - position;
                r_l = glm::length(distance);

                obj.velocities[i] += (glm::vec4(Force(distance, h_cache[i]),0.0f) * 40.0f);
                steps[i]++;

                if (steps[i] >= 10000) { 
                    state[i] = STUCK;
                    //obj.particles[i] = glm::vec4(0.0f,0.0f,0.0f,0.0f);
                    std::cout << "stuck here" << i << endl;
                    obj.velocities[i] = glm::vec4(0.0f,0.0f,0.0f,0.0f);
                }
                
            }
            
        }

        glNamedBufferSubData(obj.SSBO2, 0, sizeof(glm::vec4) * obj.amount, obj.particles.data());

        
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
};


int main()
{
    bool firstrender = true;
    glm::vec4 BColor = glm::vec4(255.0f,0.0f,0.0f,1.0f);
    
    Window GameWindow;
    Blackhole Blackhole(glm::vec3(700.0f, 200.0f, 0.0f));

    Ray Light(6, GameWindow.length);
    Light.CreateParticles();
    Light.BufferPos();


    Shader ourShader("Shaders/Shader.vs", "Shaders/Shader.fs");

    glm::mat4 ortho = glm::ortho(0.0f, float(GameWindow.width), 0.0f, float(GameWindow.length), -1.0f, 100.0f);


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

        ourShader.setBool("isInstanced", false);
        ourShader.vec4("ourColor", BColor);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, BlackholeVertices.size()/3);
        glBindVertexArray(0);

        if (firstrender){
            Light.draw(ourShader);
            firstrender = false;
        } else {
            Blackhole.UpdatingLight(Light);
            Light.draw(ourShader);
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


