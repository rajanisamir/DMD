#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <shader_s.h>
#include <iostream>
#include <cstdlib>
#include <Windows.h>
#include <math.h>
#include <cmath>
#include <ctime>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// Configure width and height of DMD.
const unsigned int SCR_WIDTH = 1140;
const unsigned int SCR_HEIGHT = 912;

// Configuration flags:
    // DMD_MODE: Requires a secondary monitor to be connected and sends frames to this monitor.
    // DMD_COORD_MODE: Uses rowAlgorithm and colAlgorithm to switch to DMD coordinates.
    // LOOP_MODE: Loop the sequence of RGB images; useful for debugging.
    // SLOW_MODE: Wait between displaying each subsequent frame; useful for debugging.
const bool DMD_MODE = false;
const bool DMD_COORD_MODE = false;
const bool LOOP_MODE = true;
const bool SLOW_MODE = true;

int rowAlgorithm(int DMDRow, int DMDCol) {
    return -DMDCol + (int) (DMDRow / 2);
}

int columnAlgorithm(int DMDRow, int DMDCol) {
    return ((int)((DMDRow + 1) / 2)) + DMDCol;
}

int main()
{
    // Tweezer configuration:
        // NUM_TWEEZERS: The number of tweezers to be moved by generated frames.
        // MAX_TIME: The maximum number of total moves between lattice sites (defines the amouunt of memory to allocate in lTweezers).
        // N: Smoothing factor defining the number of binary frames that should be generated for a move between two consecutive lattice sites.
    const int NUM_TWEEZERS = 10;
    const int MAX_TIME = 15;
    const int N = 3;

    // tweezerPositions: binary array defining starting positions of tweezers
    bool tweezerPositions[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            tweezerPositions[i][j] = 0;
        }
    }

    // lTweezers: Array defining time series of tweezer positions in lattice space.
    int lTweezers[NUM_TWEEZERS][MAX_TIME][2];

    // Populate tweezerPositions and lTweezers with initial positions of tweezers.
    srand(time(NULL));
    int count = 0;
    while (count < NUM_TWEEZERS) {
        int i = rand() % 5;
        int j = rand() % 5;
        if (tweezerPositions[i][j] == 0) {
            tweezerPositions[i][j] = 1;
            lTweezers[count][0][0] = i;
            lTweezers[count][0][1] = j;
            count++;
        }
    }

    // Compute center-of-mass in x- and y- directions.
    int COM_x = 0;
    int COM_y = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (tweezerPositions[i][j] == 1) {
                COM_x += i;
                COM_y += j;
            }
        }
    }
    COM_x /= NUM_TWEEZERS;
    COM_y /= NUM_TWEEZERS;

    int currentFrame = 0;
    while (true) {
        /* Print tweezerPositions */
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                std::cout << tweezerPositions[i][j] << ", ";
            }
            std::cout << '\n';
        }

        int numMoves = 0;
        for (int i = 0; i < NUM_TWEEZERS; i++) {
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
        std::cout << "number of moves: " << numMoves << '\n';
        if (numMoves == 0) break;
        currentFrame++;
    }
    int numFrames = currentFrame + 1;

    // Lattice configuration: vec1, vec2, and center define the lattice coordinate system in DMD space.
    float vec1[] = { 17.3, 10 };
    float vec2[] = { 17.3, -10 };
    float center[] = { SCR_WIDTH / 2, SCR_HEIGHT / 2 };

    /*
    // lTweezers: Array defining time series of tweezer positions in lattice space.
    int lTweezers[NUM_TWEEZERS][numFrames][2] = {
        {{1, 2}, {2, 2}, {2, 3}, {3, 3}, {3, 4}, {3, 5}, {4, 5}, {4, 6}, {4, 7}, {5, 7}, {6, 7}, {7, 7}, {6, 7}, {6, 6}},
        {{-2, -11}, {-3, -11}, {-3, -12}, {-2, -12}, {-1, -12}, {0, -12}, {0, -11}, {0, -10}, {1, -10}, {0, -10}, {-1, -10}, {-2, -10}, {-2, -11}, {-2, -12}},
        {{9, 9}, {9, 9}, {9, 9}, {9, 9}, {9, 9}, {9, 9}, {9, 9}, {9, 9}, {9, 9}, {9, 9}, {9, 9}, {9, 9}, {9, 9}, {9, 9}}
    };
    */

    // dTweezers: Array defining time series of tweezer positions in DMD space, computed using lattice configuration parameters (vec1, vec2, center).
    float dTweezers[NUM_TWEEZERS][MAX_TIME][2];
    for (int i = 0; i < NUM_TWEEZERS; i++) {
        for (int j = 0; j < numFrames; j++) {
            dTweezers[i][j][0] = center[0] + (lTweezers[i][j][0] * vec1[0]) + (lTweezers[i][j][1] * vec2[0]);
            dTweezers[i][j][1] = center[1] + (lTweezers[i][j][0] * vec1[1]) + (lTweezers[i][j][1] * vec2[1]);
        }
    }

    // moves: Array defining smoothed time series of tweezer positions in DMD space, generated by smoothing dTweezers using the smoothing factor N.
    float moves[NUM_TWEEZERS][N * (MAX_TIME - 1) + 1][2];
    for (int i = 0; i < NUM_TWEEZERS; i++) {
        for (int j = 0; j < numFrames - 1; j++) {
            float xDif = (dTweezers[i][j + 1][0] - dTweezers[i][j][0]) / (float) N;
            float yDif = (dTweezers[i][j + 1][1] - dTweezers[i][j][1]) / (float) N;
            for (int k = 0; k < N; k++) {
                moves[i][j * N + k][0] = dTweezers[i][j][0] + xDif * k;
                moves[i][j * N + k][1] = dTweezers[i][j][1] + yDif * k;
            }
            moves[i][(j + 1) * N][0] = dTweezers[i][numFrames - 1][0];
            moves[i][(j + 1) * N][1] = dTweezers[i][numFrames - 1][1];
        }
    }

    // Log the sequence of moves to the console.
    for (int i = 0; i < NUM_TWEEZERS; i++) {
        std::cout << "Tweezer " << std::to_string(i + 1) << ": ";
        for (int j = 0; j <  N * (numFrames - 1) + 1; j++) {
            std::cout << "(" << std::to_string(moves[i][j][0]) << "," << std::to_string(moves[i][j][1]) << "); ";
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }

    // GLFW Setup
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    if (DMD_MODE) {
        int count;
        GLFWmonitor** monitors = glfwGetMonitors(&count);
        if (count < 2) {
            std::cout << "DMD Not Connected" << std::endl;
            return -1;
        }
        window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "DMD Test Window", monitors[1], NULL);
    }
    else window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "DMD Test Window", NULL, NULL);


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

    GLubyte* textureArray = new GLubyte[SCR_WIDTH * SCR_HEIGHT * 3];
    GLubyte* dmdTextureArray = new GLubyte[SCR_WIDTH * SCR_HEIGHT * 3];

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
        if (SLOW_MODE) Sleep(1000);
        if (LOOP_MODE && iter * 24 > (N * (numFrames - 1) + 1)) iter = 0;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (int i = 0; i < SCR_HEIGHT; i++)
        {
            for (int j = 0; j < SCR_WIDTH; j++) {
                textureArray[i * SCR_WIDTH * 3 + j * 3] = (GLubyte)0;
                textureArray[i * SCR_WIDTH * 3 + j * 3 + 1] = (GLubyte)0;
                textureArray[i * SCR_WIDTH * 3 + j * 3 + 2] = (GLubyte)0;
                /*
                // Draw Square
                if (i >= 3 * SCR_HEIGHT / 8 && i <= 5 * SCR_HEIGHT / 8 && j >= 3 * SCR_WIDTH / 8 && j <= 5 * SCR_WIDTH / 8) {
                    textureArray[i * SCR_WIDTH * 3 + j * 3] = (GLubyte)255;
                    textureArray[i * SCR_WIDTH * 3 + j * 3 + 1] = (GLubyte)255;
                    textureArray[i * SCR_WIDTH * 3 + j * 3 + 2] = (GLubyte)255;
                }
                // Draw Circle
                if (pow(i - (int) (SCR_HEIGHT / 2), 2) + pow(j - (int) (SCR_WIDTH / 2), 2) <= 10000) {
                    textureArray[i * SCR_WIDTH * 3 + j * 3] = (GLubyte)255;
                    textureArray[i * SCR_WIDTH * 3 + j * 3 + 1] = (GLubyte)255;
                    textureArray[i * SCR_WIDTH * 3 + j * 3 + 2] = (GLubyte)255;
                }
                */
            }
        }

        // Take the next 24 binary frames and generate an RGB image.
        for (int i = 0; i < NUM_TWEEZERS; i++) {
            for (int j = 0; j < 24 && (iter * 24) + j < (N * (numFrames - 1) + 1); j++) {
                int x = (int)moves[i][(iter * 24) + j][0];
                int y = (int)moves[i][(iter * 24) + j][1];
                for (int dx = -5; dx <= 5; dx++) {
                    for (int dy = -5; dy <= 5; dy++) {
                        if (j < 8) textureArray[(x + dx) * SCR_WIDTH * 3 + (y + dy) * 3] += (GLubyte) pow(2, 7 - (j % 8));
                        else if (j < 16) textureArray[(x + dx) * SCR_WIDTH * 3 + (y + dy) * 3 + 1] += (GLubyte) pow(2, 7 - (j % 8));
                        else textureArray[(x + dx) * SCR_WIDTH * 3 + (y + dy) * 3 + 2] += (GLubyte) pow(2, 7 - (j % 8));
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

        if (DMD_COORD_MODE) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, dmdTextureArray);
        else glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, textureArray);

        glGenerateMipmap(GL_TEXTURE_2D);

        ourShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
        iter++;
    }

    delete[] textureArray;

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