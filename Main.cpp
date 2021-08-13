#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <shader_s.h>
#include <iostream>
#include <cstdlib>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 912;
const unsigned int SCR_HEIGHT = 1140;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    if (count < 2) {
        std::cout << "DMD Not Connected" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "DMD Test Window", monitors[1], NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader ourShader("texture.vs", "texture.fs");

    float vertices[] = {
         1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);



    /*
    int width, height, nrChannels;

    unsigned char* data = stbi_load("170.bmp", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        // Don't forget to deallocate texture array!
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    */


    GLubyte** textureArrays = new GLubyte * [3];
    textureArrays[0] = new GLubyte[SCR_WIDTH * SCR_HEIGHT * 3];
    textureArrays[1] = new GLubyte[SCR_WIDTH * SCR_HEIGHT * 3];
    textureArrays[2] = new GLubyte[SCR_WIDTH * SCR_HEIGHT * 3];
    for (int i = 0; i < SCR_HEIGHT; i++)
    {
        for (int j = 0; j < SCR_WIDTH; j++) {
            textureArrays[0][i * SCR_WIDTH * 3 + j * 3] = (GLubyte)0;
            textureArrays[0][i * SCR_WIDTH * 3 + j * 3 + 1] = (GLubyte)0;
            textureArrays[0][i * SCR_WIDTH * 3 + j * 3 + 2] = (GLubyte)0;
            textureArrays[1][i * SCR_WIDTH * 3 + j * 3] = (GLubyte)170;
            textureArrays[1][i * SCR_WIDTH * 3 + j * 3 + 1] = (GLubyte)170;
            textureArrays[1][i * SCR_WIDTH * 3 + j * 3 + 2] = (GLubyte)170;
            textureArrays[2][i * SCR_WIDTH * 3 + j * 3] = (GLubyte)255;
            textureArrays[2][i * SCR_WIDTH * 3 + j * 3 + 1] = (GLubyte)255;
            textureArrays[2][i * SCR_WIDTH * 3 + j * 3 + 2] = (GLubyte)255;
        }
    }

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int iter = 0;
    int frames[] = {0, 0, 0, 0, 0, 0};
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        /*
        int color;
        if (iter % 3 == 0) color = 0;
        if (iter % 3 == 1) color = 170;
        if (iter % 3 == 2) color = 255;
        

        int i = iter / SCR_WIDTH;
        int j = iter % SCR_WIDTH;

        for (int i = 0; i < SCR_HEIGHT; i++) {
            for (int j = 0; j < SCR_WIDTH; j++) {
                textureArrays[0][i * SCR_WIDTH * 3 + j * 3] = (GLubyte)color;
                textureArrays[0][i * SCR_WIDTH * 3 + j * 3 + 1] = (GLubyte)color;
                textureArrays[0][i * SCR_WIDTH * 3 + j * 3 + 2] = (GLubyte)color;
            }
        }
        */

        // 6 frames; white, grey, black; choose randomly; wait 5-10 seconds
        if (iter % 300 == 0) {
            std::cout << "Displaying sequence: ";
            for (int i = 0; i < 6; i++) {
                frames[i] = rand() % 3;
                std::cout << frames[i];
            }
            std::cout << std::endl;
        }

        if (iter % 300 < 6) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, textureArrays[frames[iter % 6]]);
        }
        else glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, textureArrays[0]);

        glGenerateMipmap(GL_TEXTURE_2D);

        ourShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
        iter++;
    }

    delete[] textureArrays[0];
    delete[] textureArrays[1];
    delete[] textureArrays[2];
    delete[] textureArrays;

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}