/* To compile: mex -O main.cpp glad.c glfw3.lib -IC:\Users\qmspc\documents\MATLAB\DMD\Externals\include -LC:\Users\qmspc\documents\MATLAB\DMD\Externals\lib
   To invoke: after compiling, run the testing script, and then call main repeatedly with apporpriate arguments (ex: main(200, 20, 20, array, 3, 50, 8.66, 5, 8.66, -5, 570, 456, 1)). Note that init
should be set to 1 for the first call to main() and to 0 for all subsequent calls. The first call will initialize the window and not dipslay any frames.
   To halt: run the "clear mex" command; this will close the window. */

#include "mex.hpp"
#include "mexAdapter.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader_s.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <math.h>
#include <cmath>
#include <ctime>
#include <chrono>

using namespace matlab::data;
using matlab::mex::ArgumentList;

void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void processInput(GLFWwindow* window);
int rowAlgorithm(int DMDRow, int DMDCol);
int columnAlgorithm(int DMDRow, int DMDCol);
int generateFrames(int numTweezers, int occupancyRows, int occupancyCols, int** tweezerPositions, int*** lTweezers, float*** dTweezers, float*** moves, int N);
void freeFrames(int** tweezerPositions, int*** lTweezers, float*** dTweezers, float*** moves);
GLFWwindow* setUpWindow();

// Configure width and height of DMD.
const unsigned int SCR_WIDTH = 1140;
const unsigned int SCR_HEIGHT = 912;

// Configuration flags:
    // DMD_MODE: Requires a secondary monitor to be connected and sends frames to this monitor.
    // WHITE_COLOR_MODE: Performs all computations as normal, but displays white when frames would normally be displayed.
const bool DMD_MODE = false;
const bool WHITE_COLOR_MODE = false;

// MAX_TIME: The maximum number of total moves between lattice sites (defines the amouunt of memory to allocate in lTweezers).
const int MAX_TIME = 40;

// Generates frames in moves and returns the total number generated. Assumes tweezerPositions is populated with occupancy matrix.
int generateFrames(int numTweezers, int occupancyRows, int occupancyCols, int** tweezerPositions, int*** lTweezers, float*** dTweezers, float*** moves, int N,
                   float vec1X, float vec1Y, float vec2X, float vec2Y, float centerX, float centerY) {
    // Initialize lTweezers
    for (int i = 0; i < numTweezers; i++) {
        lTweezers[i] = new int* [MAX_TIME];
        for (int j = 0; j < MAX_TIME; j++) {
            lTweezers[i][j] = new int[2];
        }
    }

    // Initialize dTweezers
    for (int i = 0; i < numTweezers; i++) {
        dTweezers[i] = new float* [MAX_TIME];
        for (int j = 0; j < MAX_TIME; j++) {
            dTweezers[i][j] = new float[2];
        }
    }

    // Initialize moves
    for (int i = 0; i < numTweezers; i++) {
        moves[i] = new float* [N * (MAX_TIME - 1) + 1];
        for (int j = 0; j < N * (MAX_TIME - 1) + 1; j++) {
            moves[i][j] = new float[2];
        }
    }

    // Populate lTweezers with initial positions of tweezers, based on tweezerPositions.
    int count = 0;
    for (int i = 0; i < occupancyRows; i++) {
        for (int j = 0; j < occupancyCols; j++) {
            if (tweezerPositions[i][j] == 1) {
                lTweezers[count][0][0] = i;
                lTweezers[count][0][1] = j;
                count++;
            }
        }
    }

    // Compute center-of-mass in x- and y- directions.
    int COM_x = 0;
    int COM_y = 0;
    for (int i = 0; i < occupancyRows; i++) {
        for (int j = 0; j < occupancyCols; j++) {
            if (tweezerPositions[i][j] == 1) {
                COM_x += i;
                COM_y += j;
            }
        }
    }
    COM_x /= numTweezers;
    COM_y /= numTweezers;

    int currentFrame = 0;
    while (true) {
        int numMoves = 0;
        for (int i = 0; i < numTweezers; i++) {
            int row = lTweezers[i][currentFrame][0];
            int col = lTweezers[i][currentFrame][1];
            if (row != COM_y && abs(row - COM_y) >= abs(col - COM_x)) {
                if (row > COM_y && tweezerPositions[row - 1][col] == 0) {
                    lTweezers[i][currentFrame + 1][0] = row - 1;
                    lTweezers[i][currentFrame + 1][1] = col;
                    tweezerPositions[row - 1][col] = 1;
                    tweezerPositions[row][col] = 0;
                    numMoves++;
                    continue;
                }
                else if (row < COM_y && tweezerPositions[row + 1][col] == 0) {
                    lTweezers[i][currentFrame + 1][0] = row + 1;
                    lTweezers[i][currentFrame + 1][1] = col;
                    tweezerPositions[row + 1][col] = 1;
                    tweezerPositions[row][col] = 0;
                    numMoves++;
                    continue;
                }
            }
            if (col != COM_x) {
                if (col > COM_x && tweezerPositions[row][col - 1] == 0) {
                    lTweezers[i][currentFrame + 1][0] = row;
                    lTweezers[i][currentFrame + 1][1] = col - 1;
                    tweezerPositions[row][col - 1] = 1;
                    tweezerPositions[row][col] = 0;
                    numMoves++;
                    continue;
                }
                else if (col < COM_x && tweezerPositions[row][col + 1] == 0) {
                    lTweezers[i][currentFrame + 1][0] = row;
                    lTweezers[i][currentFrame + 1][1] = col + 1;
                    tweezerPositions[row][col + 1] = 1;
                    tweezerPositions[row][col] = 0;
                    numMoves++;
                    continue;
                }
            }
            if (row != COM_y) {
                if (row > COM_y && tweezerPositions[row - 1][col] == 0) {
                    lTweezers[i][currentFrame + 1][0] = row - 1;
                    lTweezers[i][currentFrame + 1][1] = col;
                    tweezerPositions[row - 1][col] = 1;
                    tweezerPositions[row][col] = 0;
                    numMoves++;
                    continue;
                }
                else if (row < COM_y && tweezerPositions[row + 1][col] == 0) {
                    lTweezers[i][currentFrame + 1][0] = row + 1;
                    lTweezers[i][currentFrame + 1][1] = col;
                    tweezerPositions[row + 1][col] = 1;
                    tweezerPositions[row][col] = 0;
                    numMoves++;
                    continue;
                }
            }
            lTweezers[i][currentFrame + 1][0] = row;
            lTweezers[i][currentFrame + 1][1] = col;
        }
        if (numMoves == 0) break;
        currentFrame++;
    }
    int numFrames = currentFrame + 1;

    for (int i = 0; i < numTweezers; i++) {
        for (int j = 0; j < numFrames; j++) {
            lTweezers[i][j][0] -= occupancyRows / 2;
            lTweezers[i][j][1] -= occupancyCols / 2;
            dTweezers[i][j][0] = centerX + (lTweezers[i][j][0] * vec1X) + (lTweezers[i][j][1] * vec2X);
            dTweezers[i][j][1] = centerY + (lTweezers[i][j][0] * vec1Y) + (lTweezers[i][j][1] * vec2Y);
        }
    }

    for (int i = 0; i < numTweezers; i++) {
        for (int j = 0; j < numFrames - 1; j++) {
            float xDif = (dTweezers[i][j + 1][0] - dTweezers[i][j][0]) / (float)N;
            float yDif = (dTweezers[i][j + 1][1] - dTweezers[i][j][1]) / (float)N;
            for (int k = 0; k < N; k++) {
                moves[i][j * N + k][0] = dTweezers[i][j][0] + xDif * k;
                moves[i][j * N + k][1] = dTweezers[i][j][1] + yDif * k;
            }
            moves[i][(j + 1) * N][0] = dTweezers[i][numFrames - 1][0];
            moves[i][(j + 1) * N][1] = dTweezers[i][numFrames - 1][1];
        }
    }

    return numFrames;
}

void freeFrames(int** tweezerPositions, int*** lTweezers, float*** dTweezers, float*** moves) {
    // TODO: IMPLEMENT THIS
}

// Sets up a window using GLFW and returns a pointer to the window. Returns NULL on failure.
GLFWwindow* setUpWindow() {
    // GLFW Setup
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window;
    if (DMD_MODE) {
        int count;
        GLFWmonitor** monitors = glfwGetMonitors(&count);
        if (count < 2) {
            std::cout << "DMD Not Connected" << std::endl;
            return NULL;
        }
        window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "DMD Test Window", monitors[1], NULL);
    }
    else window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "DMD Test Window", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    return window;
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

int rowAlgorithm(int DMDRow, int DMDCol) {
    return -DMDCol + (int)(DMDRow / 2);
}

int columnAlgorithm(int DMDRow, int DMDCol) {
    return ((int)((DMDRow + 1) / 2)) + DMDCol;
}

class MexFunction : public matlab::mex::Function {
    //    Instance variables (mostly to do with OpenGL and GLFW functionality).
    GLFWwindow* window;
    unsigned int VBO, VAO, EBO;
    float vertices[32] = {
        1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
        1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
       -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
       -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left
    };
    unsigned int indices[6] = {
        0, 1, 3,
        1, 2, 3
    };
    unsigned int texture;
    Shader* ourShader;
    
public:
    MexFunction() {
        window = setUpWindow();
        if (window == NULL) {
            std::cout << "Failed to create window." << std::endl;
        }
        
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD." << std::endl;
        }

        //    For Windows:
        ourShader = new Shader("texture.vs", "texture.fs");

        //    For Mac:
        //    ourShader = new Shader("/Users/samir/Desktop/DMD/texture.vs", "/Users/samir/Desktop/DMD/texture.fs");
        
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

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    
    ~MexFunction() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);

        glfwTerminate();
    }
    
    /* The MEX function operator() is invoked by calling main() in MATLAB with the following parameters:
            (int) numTweezers: the total number of tweezers (i.e. the number of "1" values in the occupancy matrix
            (int) occupancyRows: the number of rows in the occupancy matrix
            (int) occupancyCols: the number of columns in the occupancy matrix
            (int array) occupancyMatrix: a one-dimensional matrix consisting of values "0" and "1"; converted to a 2D
                        matrix using the values specified by occupancyRows and occupancyCols
            (int) tweezerSize: the side length of the square defining the size of a tweezer, in pixels
            (int) N: the smoothing factor specifying how many frames should be included between consecutive lattice sites
            (float) vec1X: the x-component of the first vector specifying the lattice orientation in DMD space
            (float) vec1Y: the y-component of the first vector specifying the lattice orientation in DMD space
            (float) vec2X: the x-component of the second vector specifying the lattice orientation in DMD space
            (float) vec2Y: the y-component of the second vector specifying the lattice orientation in DMD space
            (float) centerX: the x-component of the center of the lattice in DMD space
            (float) centerY: the y-component of the center of the lattice in DMD space
            (int) init: should be set to 1 for the first call to operator and to 0 for all subsequent calls
     */

    void operator() (matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {
        int numTweezers = inputs[0][0];
        int occupancyRows = inputs[1][0];
        int occupancyCols = inputs[2][0];
        matlab::data::Array occupancyMatrix = inputs[3];
        int tweezerSize = inputs[4][0];
        int N = inputs[5][0];
        
        // Lattice configuration: vec1, vec2, and center define the lattice coordinate system in DMD space.
        float vec1X = inputs[6][0];
        float vec1Y = inputs[7][0];
        float vec2X = inputs[8][0];
        float vec2Y = inputs[9][0];
        float centerX = inputs[10][0];
        float centerY = inputs[11][0];
        int init = inputs[12][0];

        if (init == 1) return;
       
        // tweezerPositions: binary array defining starting positions of tweezers
        // lTweezers: Array defining time series of tweezer positions in lattice space.
        // dTweezers: Array defining time series of tweezer positions in DMD space, computed using lattice configuration parameters (vec1, vec2, center).
        // moves: Array defining smoothed time series of tweezer positions in DMD space, generated by smoothing dTweezers using the smoothing factor N.
        int** tweezerPositions = new int* [occupancyRows];
        int*** lTweezers = new int** [numTweezers];
        float*** dTweezers = new float** [numTweezers];
        float*** moves = new float** [numTweezers];
        
        for (int i = 0; i < occupancyRows; i++) {
            tweezerPositions[i] = new int[occupancyCols];
            for (int j = 0; j < occupancyCols; j++) {
                tweezerPositions[i][j] = occupancyMatrix[i * occupancyCols + j];
            }
        }
        
        int numFrames = generateFrames(numTweezers, occupancyRows, occupancyCols, tweezerPositions, lTweezers, dTweezers, moves, N,
                                       vec1X, vec1Y, vec2X, vec2Y, centerX, centerY);

        GLubyte* textureArray = new GLubyte[SCR_WIDTH * SCR_HEIGHT * 3];
        GLubyte* dmdTextureArray = new GLubyte[SCR_WIDTH * SCR_HEIGHT * 3];

        int iter = 0;
        
        while (!glfwWindowShouldClose(window)) {
            if (iter * 24 > (N * (numFrames - 1) + 1)) {
//              freeFrames(tweezerPositions, lTweezers, dTweezers, moves);
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                glfwSwapBuffers(window);
                return;
            }
            else {
                for (int i = 0; i < SCR_HEIGHT; i++)
                {
                    for (int j = 0; j < SCR_WIDTH; j++) {
                        textureArray[i * SCR_WIDTH * 3 + j * 3] = (GLubyte)0;
                        textureArray[i * SCR_WIDTH * 3 + j * 3 + 1] = (GLubyte)0;
                        textureArray[i * SCR_WIDTH * 3 + j * 3 + 2] = (GLubyte)0;
                        dmdTextureArray[i * SCR_WIDTH * 3 + j * 3] = (GLubyte)0;
                        dmdTextureArray[i * SCR_WIDTH * 3 + j * 3 + 1] = (GLubyte)0;
                        dmdTextureArray[i * SCR_WIDTH * 3 + j * 3 + 2] = (GLubyte)0;
                    }
                }

                // Take the next 24 binary frames and generate an RGB image.
                for (int i = 0; i < numTweezers; i++) {
                    for (int j = 0; j < 24 && (iter * 24) + j < (N * (numFrames - 1) + 1); j++) {
                        int x = (int)moves[i][(iter * 24) + j][0];
                        int y = (int)moves[i][(iter * 24) + j][1];
                        for (int dx = -tweezerSize; dx <= tweezerSize; dx++) {
                            for (int dy = -tweezerSize; dy <= tweezerSize; dy++) {
                                if (x + dx < 0 || y + dy < 0) {
                                    break;
                                }
                                if (j < 8) textureArray[(x + dx) * SCR_WIDTH * 3 + (y + dy) * 3] += (GLubyte)pow(2, 7 - (j % 8));
                                else if (j < 16) textureArray[(x + dx) * SCR_WIDTH * 3 + (y + dy) * 3 + 1] += (GLubyte)pow(2, 7 - (j % 8));
                                else textureArray[(x + dx) * SCR_WIDTH * 3 + (y + dy) * 3 + 2] += (GLubyte)pow(2, 7 - (j % 8));
                            }
                        }
                    }
                }

                // Populate dmdTextureArray with textureArray in DMD coordinate system by using rowAlgorithm() and columnAlgorithm().
                for (int i = 0; i < SCR_HEIGHT; i++) {
                    for (int j = 0; j < SCR_WIDTH; j++) {
                        int x = 607 + rowAlgorithm(i, j);
                        int y = columnAlgorithm(i, j);
                        if (x < SCR_HEIGHT && y < SCR_WIDTH) {
                            dmdTextureArray[i * SCR_WIDTH * 3 + j * 3] = textureArray[x * SCR_WIDTH * 3 + y * 3];
                            dmdTextureArray[i * SCR_WIDTH * 3 + j * 3 + 1] = textureArray[x * SCR_WIDTH * 3 + y * 3 + 1];
                            dmdTextureArray[i * SCR_WIDTH * 3 + j * 3 + 2] = textureArray[x * SCR_WIDTH * 3 + y * 3 + 2];
                        }
                    }
                }
                
                if (WHITE_COLOR_MODE) {
                    if (iter * 24 > (N * (numFrames - 1) + 1)) {
                        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                    }
                    else {
                        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                    }
                    glClear(GL_COLOR_BUFFER_BIT);
                }
                
                else {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, dmdTextureArray);
                    glGenerateMipmap(GL_TEXTURE_2D);
                    ourShader->use();
                    glBindVertexArray(VAO);
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                }
                
                glfwSwapBuffers(window);

                iter++;
            }

            glfwPollEvents();
            processInput(window);
        }
    }
};
