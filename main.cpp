/**
* Author: Jemima Datus
* Assignment: Pong Clone
* Date due: 2024-10-12, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define STB_IMAGE_IMPLEMENTATION //need for textures

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"                // 4x4 Matrix
#include "glm/gtc/matrix_transform.hpp"  // Matrix transformation methods
#include "ShaderProgram.h"               // We'll talk about these later in the course
//include for draw text function
#include <vector>;
//include for loading textures
#include "stb_image.h"
//include for absolute value
#include "cmath"

enum AppStatus { RUNNING, TERMINATED };

// Our window dimensions
constexpr int WINDOW_WIDTH = 640 * 1.5,
WINDOW_HEIGHT = 480 * 1.5;

//make background look like sand
constexpr float BG_RED = 0.941f,
BG_BLUE = 0.804f,
BG_GREEN = 0.408f,
BG_OPACITY = 1.0f;

// Our viewport—or our "camera"'s—position and dimensions
constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// Texture Shaders
constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

//variable for draw text
constexpr int fontBankSize = 16;

//player, net, volleyball, top and bottom bar objects
constexpr char player1[] = "greyBar.png";
constexpr char player2[] = "greyBar.png";
constexpr char net[] = "net.png";
constexpr char ball1[] = "volleyball.png";
constexpr char ball2[] = "volleyball.png";
constexpr char ball3[] = "volleyball.png";
constexpr char topBar[] = "blueBar.png";
constexpr char bottomBar[] = "blueBar.png";

//font sprite sheet
constexpr char font[] = "fonts.png";

//texture ID variables for all objects
GLuint player1TextureID, player2TextureID, netTextureID, ball1TextureID; 
GLuint ball2TextureID, ball3TextureID, topBarTextureID, bottomBarTextureID, fontTextureID;

//one player bool
bool onePlayer = false;
bool moveUp = true;

//winning variables
bool gameWon = false;
int winner = 0; // 0 - no winner yet, 1- player 1 wins, 2- player 2 wins

//variables for ball bounds
float topBound = 3.1f;
float bottomBound = -3.15f;
float leftBound = -4.55f;
float rightBound = 4.95f;

//variable for number of balls
int ballNum = 1; // set to 1 by default

//scale variables
constexpr glm::vec3 playerScale = glm::vec3(2.0f, 2.0f, 0.0f);
constexpr glm::vec3 netScale = glm::vec3(3.0f, 12.0f, 0.0f);
constexpr glm::vec3 ballScale = glm::vec3(1.0f, 1.0f, 0.0f);
constexpr glm::vec3 barScale = glm::vec3(13.0f, 2.0f, 0.0f);


//inital position vectors
constexpr glm::vec3 player1InitialPos = glm::vec3(-4.55f, 0.0f, 0.0f);
constexpr glm::vec3 player2InitialPos = glm::vec3(4.95f, 0.0f, 0.0f);
constexpr glm::vec3 ballInitialPos = glm::vec3(0.0f, 0.0f, 0.0f);
constexpr glm::vec3 topBarInitialPos = glm::vec3(0.0f, 3.8f, 0.0f);
constexpr glm::vec3 bottomBarInitialPos = glm::vec3(0.0f, -3.45f, 0.0f);

//current position vectors
glm::vec3 player1Pos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 player2Pos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball1Pos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball2Pos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball3Pos = glm::vec3(0.0f, 0.0f, 0.0f);

//variable needed for delta time
float g_prevTick = 0.0f;

//vectors for keeping track of player movements and ball
glm::vec3 player1Move = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 player2Move = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball1Move = glm::vec3(-1.0f, 0.0f, 0.0f); //ball 1 starts off moving towards player 1
glm::vec3 ball2Move = glm::vec3(1.0f, 0.0f, 0.0f); //ball 2 starts off moving towards player 2
glm::vec3 ball3Move = glm::vec3(-1.0f, 0.0f, 0.0f); //ball 3 starts off moving towards player 1


AppStatus g_app_status = RUNNING;
SDL_Window* g_display_window;

ShaderProgram g_shader_program;

//create matrices
glm::mat4 g_view_matrix, player1Matrix, player2Matrix, netMatrix, ball1Matrix, ball2Matrix;
glm::mat4 ball3Matrix, topBarMatrix, bottomBarMatrix, g_projection_matrix;

//Variables for load_texture
constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL = 0;
constexpr GLint TEXTURE_BORDER = 0;

GLuint loadTexture(const char* filePath) {

    int width, height, numberOfComponents;
    unsigned char* image = stbi_load(filePath, &width, &height, &numberOfComponents, STBI_rgb_alpha);

    //show error message if the image was not loaded
    if (image == NULL) {
        std::cerr << "Unable to load image. Make sure the path is correct.";
        assert(false);
    }

    //generating a textureID and binding it to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //free up memory allocated for image file data
    stbi_image_free(image);

    return textureID;

}

//the drawObject function will make each of our textured objects appear on the screen
void drawObject(glm::mat4& modelMatrix, GLuint& textureID) {
    g_shader_program.set_model_matrix(modelMatrix);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
//draw text function
void draw_text(ShaderProgram* shader_program, GLuint font_texture_id, std::string text,
    float font_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / fontBankSize;
    float height = 1.0f / fontBankSize;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for
    // each character. Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their
        //    position relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (font_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % fontBankSize) / fontBankSize;
        float v_coordinate = (float)(spritesheet_index / fontBankSize) / fontBankSize;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    shader_program->set_model_matrix(model_matrix);
    glUseProgram(shader_program->get_program_id());

    glVertexAttribPointer(shader_program->get_position_attribute(), 2, GL_FLOAT, false, 0,
        vertices.data());
    glEnableVertexAttribArray(shader_program->get_position_attribute());

    glVertexAttribPointer(shader_program->get_tex_coordinate_attribute(), 2, GL_FLOAT,
        false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(shader_program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(shader_program->get_position_attribute());
    glDisableVertexAttribArray(shader_program->get_tex_coordinate_attribute());
}



void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Play Beach Volleyball Pong!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    if (g_display_window == nullptr)
    {
        std::cerr << "ERROR: SDL Window could not be created.\n";
        g_app_status = TERMINATED;

        SDL_Quit();
        exit(1);
    }

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // Initialise our camera
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    // Load up our shaders
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    //Load textures for all objects
    player1TextureID = loadTexture(player1);
    player2TextureID = loadTexture(player2);
    netTextureID = loadTexture(net);
    ball1TextureID = loadTexture(ball1);
    ball2TextureID = loadTexture(ball2);
    ball3TextureID = loadTexture(ball3);
    topBarTextureID = loadTexture(topBar);
    bottomBarTextureID = loadTexture(bottomBar);
    fontTextureID = loadTexture(font);

    //enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialise our view, model, and projection matrices
    g_view_matrix = glm::mat4(1.0f);  

    //Initialise all object  matrices
    player1Matrix = glm::mat4(1.0f);
    player2Matrix = glm::mat4(1.0f);
    netMatrix = glm::mat4(1.0f);
    ball1Matrix = glm::mat4(1.0f);
    ball2Matrix = glm::mat4(1.0f);
    ball3Matrix = glm::mat4(1.0f);
    topBarMatrix = glm::mat4(1.0f);
    bottomBarMatrix = glm::mat4(1.0f);

    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);


    // Each object has its own unique ID
    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
}

void process_input()
{
    //player 1 uses w/s
    //player 2 uses up/down
    //if the player presses t it switches from 2 player to 1 player mode
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {

        //set these move vectors to 0 at first so that the players will not move if no keys are touched
        player1Move = glm::vec3(0.0f, 0.0f, 0.0f);
        player2Move = glm::vec3(0.0f, 0.0f, 0.0f);


        //if the players quit or close the window the app status is set to terminated
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_app_status = TERMINATED;
        }
        //get keyboard state
        const Uint8* keyState = SDL_GetKeyboardState(NULL);
        //check for number of balls input
        if (keyState[SDL_SCANCODE_1] && !gameWon) {
            ballNum = 1;
        }
        else if (keyState[SDL_SCANCODE_2] && !gameWon) {
            ballNum = 2;
        }if (keyState[SDL_SCANCODE_3] && !gameWon) {
            ballNum = 3;
        }

        //check for player 1 move
        if (keyState[SDL_SCANCODE_W] && !gameWon) {
            player1Move.y = 1.0f;
        }
        if (keyState[SDL_SCANCODE_S] && !gameWon) {
            player1Move.y = -1.0f;
        }
        //check for player 2 move
        if (keyState[SDL_SCANCODE_UP] && !onePlayer && !gameWon) {
            player2Move.y = 1.0f;
        }
        if (keyState[SDL_SCANCODE_DOWN] && !onePlayer && !gameWon) {
            player2Move.y = -1.0f;
        }
        if (keyState[SDL_SCANCODE_T] && !onePlayer && !gameWon) {
            onePlayer = true;
        }
    }
    //normalize player 1 input
    if (glm::length(player1Move) > 1.0f) {
        player1Move = glm::normalize(player1Move);
    }
    //normalize player 2 input
    if (glm::length(player2Move) > 1.0f) {
        player2Move = glm::normalize(player2Move);
    }

}

void update() {
    //TODO: NEED TO BALL MOVEMENT, BALL COLLISION, ONE PLAYER MODE, SHOW SCORE, SHOW WINNER
    //1.translate, 2.rotate, 3.scale

    //delta time calculations
    float newTick = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = newTick - g_prevTick;
    g_prevTick = newTick;

    //reset model matrices
    player1Matrix = glm::mat4(1.0f);
    player2Matrix = glm::mat4(1.0f);
    netMatrix = glm::mat4(1.0f);
    if (!gameWon) {
        ball1Matrix = glm::mat4(1.0f);
    }
    if (ballNum >= 2 && !gameWon) {
        ball2Matrix = glm::mat4(1.0f);
    }
    if (ballNum == 3 && !gameWon) {
        ball3Matrix = glm::mat4(1.0f);
    }
    topBarMatrix = glm::mat4(1.0f);
    bottomBarMatrix = glm::mat4(1.0f);

    //put models in original positions
    player1Matrix = glm::translate(player1Matrix, player1InitialPos);
    player2Matrix = glm::translate(player2Matrix, player2InitialPos);
    if (!gameWon) {
        ball1Matrix = glm::translate(ball1Matrix, ballInitialPos);

    }
    if (ballNum >= 2 && !gameWon) {
        ball2Matrix = glm::translate(ball2Matrix, ballInitialPos);

    }
    if (ballNum == 3 && !gameWon){
        ball3Matrix = glm::translate(ball3Matrix, ballInitialPos);
    }
    topBarMatrix = glm::translate(topBarMatrix, topBarInitialPos);
    bottomBarMatrix = glm::translate(bottomBarMatrix, bottomBarInitialPos);

    //check if in one player mode
    if (onePlayer && !gameWon) {
        //if moveUp flag is set move player 2 up
        if (moveUp) {
            player2Move.y = 1.0f;
        }
        //move player 2 down
        else {
            player2Move.y = -1.0f;
        }
    }

    //adjust player position with delta time
    player1Pos += player1Move * deltaTime * 2.0f;
    player2Pos += player2Move * deltaTime * 2.0f;

    //check to make sure the players stay in the frame 
    if (player1Pos.y > 2.35f) {
        player1Pos.y = 2.35f;
    }
    else if (player1Pos.y < -2.35f) {
        player1Pos.y = -2.35f;
    }
    if (player2Pos.y > 2.35f) {
        player2Pos.y = 2.35f;
        moveUp = false;
    }
    else if (player2Pos.y < -2.35f) {
        player2Pos.y = -2.35f;
        moveUp = true;
    }

    //translate models based on user input
    player1Matrix = glm::translate(player1Matrix, player1Pos);
    player2Matrix = glm::translate(player2Matrix, player2Pos);

    //translate ball 1
    if (!gameWon) {
        ball1Pos += ball1Move * deltaTime * 2.0f;
        ball1Matrix = glm::translate(ball1Matrix, ball1Pos);
    }
    //translate ball 2
    if (ballNum >= 2 && !gameWon) {
        ball2Pos += ball2Move * deltaTime * 2.0f;
        ball2Matrix = glm::translate(ball2Matrix, ball2Pos);
    }
    //translate ball 3
    if (ballNum == 3 && !gameWon) {
        ball3Pos += ball3Move * deltaTime * 2.0f;
        ball3Matrix = glm::translate(ball3Matrix, ball3Pos);
    }

    //scale all objects 
    player1Matrix = glm::scale(player1Matrix, playerScale);
    player2Matrix = glm::scale(player2Matrix, playerScale);
    netMatrix = glm::scale(netMatrix, netScale);
    topBarMatrix = glm::scale(topBarMatrix, barScale);
    bottomBarMatrix = glm::scale(bottomBarMatrix, barScale);

    //collision detection with player 1
    float xdistance = fabs(ball1Pos.x + ballInitialPos.x - player1Pos.x - player1InitialPos.x)*4.0f - ((ballScale.x + playerScale.x)/ 2.0f);
    float ydistance = fabs(ball1Pos.y + ballInitialPos.y - player1Pos.y - player1InitialPos.y)*1.5f - ((ballScale.y + playerScale.y)/ 2.0f);
    if (xdistance < 0.0 && ydistance < 0.0) {
        //set ballMove.x to xdiff
        ball1Move.x = (ball1Pos.x + ballInitialPos.x - player1Pos.x - player1InitialPos.x)* 3.0f;
        //set ballMove.y to ydiff
        ball1Move.y = (ball1Pos.y + ballInitialPos.y - player1Pos.y - player1InitialPos.y)* 3.0f;
    }
    //collision detection with player 2
    xdistance = fabs(ball1Pos.x + ballInitialPos.x - player2Pos.x - player2InitialPos.x)*3.5f - ((ballScale.x + playerScale.x) / 2.0f);
    ydistance = fabs(ball1Pos.y + ballInitialPos.y - player2Pos.y - player2InitialPos.y)*1.25f - ((ballScale.y + playerScale.y) / 2.0f);
    if (xdistance < 0.0 && ydistance < 0.0) {
        //set ballMove.x to xdiff
        ball1Move.x = (ball1Pos.x + ballInitialPos.x - player2Pos.x - player2InitialPos.x)* 3.0f;
        //set ballMove.y to ydiff
        ball1Move.y = (ball1Pos.y + ballInitialPos.y - player2Pos.y - player2InitialPos.y)* 3.0f;
    } 
    //check that ball is within top bound
    if (ball1Pos.y >= topBound) {
        //set ballMove.x to xdiff
        ball1Move.x = (ball1Pos.x + ballInitialPos.x - topBarInitialPos.x);
        //set ballMove.y to ydiff
        ball1Move.y = (ball1Pos.y + ballInitialPos.y - topBarInitialPos.y);
    }
    //check that ball is within bottom bound
    if (ball1Pos.y <= bottomBound) {
        //set ballMove.x to xdiff
        ball1Move.x = (ball1Pos.x + ballInitialPos.x - bottomBarInitialPos.x);
        //set ballMove.y to ydiff
        ball1Move.y = (ball1Pos.y + ballInitialPos.y - bottomBarInitialPos.y);
    }
    //check if ball has gone out of bounds
    if (ball1Pos.x < leftBound) { //player 1 loses
        gameWon = true;
        winner = 2;
    }
    else if (ball1Pos.x > rightBound) { //player 2 loses
        gameWon = true;
        winner = 1;
    }
    //repeat for ball 2
    if (ballNum >= 2 && !gameWon) {
        //collision detection with player 1
        float xdistance = fabs(ball2Pos.x + ballInitialPos.x - player1Pos.x - player1InitialPos.x) * 4.0f - ((ballScale.x + playerScale.x) / 2.0f);
        float ydistance = fabs(ball2Pos.y + ballInitialPos.y - player1Pos.y - player1InitialPos.y) * 1.5f - ((ballScale.y + playerScale.y) / 2.0f);
        if (xdistance < 0.0 && ydistance < 0.0) {
            //set ballMove.x to xdiff
            ball2Move.x = (ball2Pos.x + ballInitialPos.x - player1Pos.x - player1InitialPos.x) * 3.0f;
            //set ballMove.y to ydiff
            ball2Move.y = (ball2Pos.y + ballInitialPos.y - player1Pos.y - player1InitialPos.y) * 3.0f;
        }
        //collision detection with player 2
        xdistance = fabs(ball2Pos.x + ballInitialPos.x - player2Pos.x - player2InitialPos.x) * 3.5f - ((ballScale.x + playerScale.x) / 2.0f);
        ydistance = fabs(ball2Pos.y + ballInitialPos.y - player2Pos.y - player2InitialPos.y) * 1.25f - ((ballScale.y + playerScale.y) / 2.0f);
        if (xdistance < 0.0 && ydistance < 0.0) {
            //set ballMove.x to xdiff
            ball2Move.x = (ball2Pos.x + ballInitialPos.x - player2Pos.x - player2InitialPos.x) * 3.0f;
            //set ballMove.y to ydiff
            ball2Move.y = (ball2Pos.y + ballInitialPos.y - player2Pos.y - player2InitialPos.y) * 3.0f;
        }
        //check that ball is within top bound
        if (ball2Pos.y >= topBound) {
            //set ballMove.x to xdiff
            ball2Move.x = (ball2Pos.x + ballInitialPos.x - topBarInitialPos.x);
            //set ballMove.y to ydiff
            ball2Move.y = (ball2Pos.y + ballInitialPos.y - topBarInitialPos.y);
        }
        //check that ball is within bottom bound
        if (ball2Pos.y <= bottomBound) {
            //set ballMove.x to xdiff
            ball2Move.x = (ball2Pos.x + ballInitialPos.x - bottomBarInitialPos.x);
            //set ballMove.y to ydiff
            ball2Move.y = (ball2Pos.y + ballInitialPos.y - bottomBarInitialPos.y);
        }
        //check if ball has gone out of bounds
        if (ball2Pos.x < leftBound) { //player 1 loses
            gameWon = true;
            winner = 2;
        }
        else if (ball2Pos.x > rightBound) { //player 2 loses
            gameWon = true;
            winner = 1;
        }
    }
    //repeat for ball 3
    if (ballNum == 3 && !gameWon) {
        //collision detection with player 1
        float xdistance = fabs(ball3Pos.x + ballInitialPos.x - player1Pos.x - player1InitialPos.x) * 4.0f - ((ballScale.x + playerScale.x) / 2.0f);
        float ydistance = fabs(ball3Pos.y + ballInitialPos.y - player1Pos.y - player1InitialPos.y) * 1.5f - ((ballScale.y + playerScale.y) / 2.0f);
        if (xdistance < 0.0 && ydistance < 0.0) {
            //set ballMove.x to xdiff
            ball3Move.x = (ball3Pos.x + ballInitialPos.x - player1Pos.x - player1InitialPos.x) * 3.0f;
            //set ballMove.y to ydiff
            ball3Move.y = (ball3Pos.y + ballInitialPos.y - player1Pos.y - player1InitialPos.y) * 3.0f;
        }
        //collision detection with player 2
        xdistance = fabs(ball3Pos.x + ballInitialPos.x - player2Pos.x - player2InitialPos.x) * 3.5f - ((ballScale.x + playerScale.x) / 2.0f);
        ydistance = fabs(ball3Pos.y + ballInitialPos.y - player2Pos.y - player2InitialPos.y) * 1.25f - ((ballScale.y + playerScale.y) / 2.0f);
        if (xdistance < 0.0 && ydistance < 0.0) {
            //set ballMove.x to xdiff
            ball3Move.x = (ball3Pos.x + ballInitialPos.x - player2Pos.x - player2InitialPos.x) * 3.0f;
            //set ballMove.y to ydiff
            ball3Move.y = (ball3Pos.y + ballInitialPos.y - player2Pos.y - player2InitialPos.y) * 3.0f;
        }
        //check that ball is within top bound
        if (ball3Pos.y >= topBound) {
            //set ballMove.x to xdiff
            ball3Move.x = (ball3Pos.x + ballInitialPos.x - topBarInitialPos.x);
            //set ballMove.y to ydiff
            ball3Move.y = (ball3Pos.y + ballInitialPos.y - topBarInitialPos.y);
        }
        //check that ball is within bottom bound
        if (ball3Pos.y <= bottomBound) {
            //set ballMove.x to xdiff
            ball3Move.x = (ball3Pos.x + ballInitialPos.x - bottomBarInitialPos.x);
            //set ballMove.y to ydiff
            ball3Move.y = (ball3Pos.y + ballInitialPos.y - bottomBarInitialPos.y);
        }
        //check if ball has gone out of bounds
        if (ball3Pos.x < leftBound) { //player 1 loses
            gameWon = true;
            winner = 2;
        }
        else if (ball3Pos.x > rightBound) { //player 2 loses
            gameWon = true;
            winner = 1;
        }
    }

}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    //vertices
    float vertices[] = {
    -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  
    -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   
    };

    //textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    //draw both bars, middle line, and ball
    drawObject(player1Matrix, player1TextureID);
    drawObject(player2Matrix, player2TextureID);
    drawObject(netMatrix, netTextureID);
    drawObject(topBarMatrix, topBarTextureID);
    drawObject(bottomBarMatrix, bottomBarTextureID);
    //only draw balls if the game has not been won
    if (!gameWon) {
            drawObject(ball1Matrix, ball1TextureID);
    }
    //check for number of balls
    if (ballNum >=2 && !gameWon) {
        drawObject(ball2Matrix, ball2TextureID);
    }
    if (ballNum == 3 && !gameWon) {
        drawObject(ball3Matrix, ball3TextureID);

    }

    //create winning message if someone won the game
    if (gameWon) {
        if (winner == 1) {
            draw_text(&g_shader_program,fontTextureID,"Winner", 0.5f, 0.01f,glm::vec3(-3.5f,0.0f,0.0f));
        }
        else {
            draw_text(&g_shader_program, fontTextureID, "Winner", 0.5f, 0.01f, glm::vec3(1.0f, 0.0f, 0.0f));
        }
    }


    //disable both attribute arrays 
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());



    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    // Initialise our program—whatever that means
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();  // If the player did anything—press a button, move the joystick—process it
        update();         // Using the game's previous state, and whatever new input we have, update the game's state
        render();         // Once updated, render those changes onto the screen
    }

    shutdown();  // The game is over, so let's perform any shutdown protocols
    return 0;
}