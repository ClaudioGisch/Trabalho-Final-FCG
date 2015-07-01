
#define GLEW_STATIC

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
//  To use functions with variables arguments
#include <stdarg.h>

//  for malloc
#include <stdlib.h>

#include "GL/glew.h"
#include "GL/freeglut.h"

// Include GLM
#include "glm/glm.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "audiere-1.9.4-win32/include/audiere.h"
#include "glm/gtc/type_ptr.hpp"

using namespace glm;
using namespace audiere;

#include "common/shader.cpp"
#include "common/texture.cpp"
#include "common/objloader.cpp"
#include "common/text2D.cpp"

#define pi 3.14159265
#define MAXSPEED 0.7
#define TURNINGSPEED 1.3f

bool keystates[256];

GLuint programID;
GLuint matrixID;        // Get a handle for our "MVP" uniform
GLuint modelID;

//road
GLuint vertexID;
GLuint vertexBuffer;
GLuint uvBuffer;
GLuint textureID1;                  // Get a handle for our "myTextureSampler" uniform
std::vector<glm::vec3> roadVertices;
std::vector<glm::vec2> roadUV;
std::vector<glm::vec3> roadNormals;

//car
GLuint vertexID2;
GLuint vertexBuffer2;
GLuint uvBuffer2;
GLuint textureID2;                  // Get a handle for our "myTextureSampler" uniform
std::vector<glm::vec3> carVertices;
std::vector<glm::vec2> carUV;
std::vector<glm::vec3> carNormals;

//sand
GLuint vertexID3;
GLuint vertexBuffer3;
GLuint uvBuffer3;
GLuint textureID3;
std::vector<glm::vec3> areiaVertices;
std::vector<glm::vec2> areiaUV;
std::vector<glm::vec3> areiaNormals;

//checkpoint
GLuint vertexID4;
GLuint vertexBuffer4;
GLuint uvBuffer4;
GLuint textureID4;
std::vector<glm::vec3> checkVertices;
std::vector<glm::vec2> checkUV;
std::vector<glm::vec3> checkNormals;

//finish line
GLuint vertexID5;
GLuint vertexBuffer5;
GLuint uvBuffer5;
GLuint textureID5;
std::vector<glm::vec3> finishVertices;
std::vector<glm::vec2> finishUV;
std::vector<glm::vec3> finishNormals;

// bot textures
GLuint textureID6;
GLuint textureID7;
GLuint textureID8;

//powerup
GLuint vertexID6;
GLuint vertexBuffer6;
GLuint uvBuffer6;
std::vector<glm::vec3> powerVertices;
std::vector<glm::vec2> powerUV;
std::vector<glm::vec3> powerNormals;
GLuint textureID9;
GLuint textureID10;
GLuint textureID11;

/** Globals */

int number_of_laps = 5;
int number_of_laps_to_win = 4;
int place = 1;
float count_down = 4;
bool race_over = false;
bool game_pause = false;
bool begining_game = true;

/** Sound */

// background music
string lightbringer = "playlist/lightbringer.mp3";
string deathfire_grasp = "playlist/deathfire_grasp.mp3";
string last_whisper = "playlist/last_whisper.mp3";
string megitsune = "playlist/megitsune.mp3";
string angus_mcfife = "playlist/angus_mcfife.mp3";
string topgear1 = "playlist/topgear1.mp3";
string background = "sounds/happyrock.mp3";
string cardrive = "sounds/carrun.mp3";
string gravel = "sounds/gravel.mp3";
string success = "sounds/success.mp3";
string colision = "sounds/colision.mp3";
int currentMusic = 0;
bool changingMusic = false;
int playListSize = 1;
float volume = 0.1f;
bool changingVolume = false;
AudioDevicePtr device(OpenDevice());

OutputStreamPtr s_success(OpenSound(device, success.c_str(), false));
OutputStreamPtr sound(OpenSound(device, background.c_str(), false));
//OutputStreamPtr track1(OpenSound(device, megitsune.c_str(), false));
//OutputStreamPtr track2(OpenSound(device, angus_mcfife.c_str(), false));
//OutputStreamPtr track3(OpenSound(device, lightbringer.c_str(), false));
//OutputStreamPtr track4(OpenSound(device, deathfire_grasp.c_str(), false));
//OutputStreamPtr track5(OpenSound(device, last_whisper.c_str(), false));
//OutputStreamPtr track6(OpenSound(device, topgear1.c_str(), false));
OutputStreamPtr playList[] = {sound/*, track6, track1, track2, track3, track4, track5*/};
OutputStreamPtr sound2(OpenSound(device, cardrive.c_str(), false));
OutputStreamPtr sound3(OpenSound(device, gravel.c_str(), false));
OutputStreamPtr sound4(OpenSound(device, colision.c_str(), false));


/** Light */

struct Light {
    glm::vec3 position;
    glm::vec3 intensities; //a.k.a. the color of the light
};

Light gLight;
GLfloat ambientShininess = 0.30;

/** ----------- */

//fps
float game_fps = 60;

/** player position */
double posx = 3;
double posz = 0;
double posy = 1.5f;

/* Some globals */
double car_radius = 0.8;
float car_angle = 270;
float camAngle = 0;
float pauseCamAngle = 0;
int upArrow = 0;
int downArrow = 1;
int rightArrow = 2;
int leftArrow = 3;
float turning_speed = TURNINGSPEED;
double velocity = 0;
int slowFactorAreia = 5;
double acceleration = 0.005;
double maxSpeed = MAXSPEED;
bool drift = false;
int bot_number = 3;
int bot_last_pinpoint[3] = {0,0,0};
vec2 bot_position[3] = {vec2(3.0, 3.0), vec2(6.0, 3.0), vec2(6.0, 0.0)};
float bot_angle[3] = {0.0f, 0.0f, 0.0f};
float bot_velocity[3] = {0,0,0};
float bot_maxSpeed[3] = {MAXSPEED, MAXSPEED*1.05, MAXSPEED*1.1};
int bot_laps[3] = {0,0,0};

/** camera */
glm::vec3 current_camera_pos;
glm::vec3 current_camera_look;

float change_camera_delay = 0.5;
float current_camera_delay = 0.5;

// normal camera +
std::vector<glm::vec3> camera_pos = {vec3(3, 4, 3), vec3(0.1, 1.6, 0.1), vec3(3, 2, 3)};
std::vector<glm::vec3> camera_look = {vec3(2, 1, 2), vec3(2, 1, 2),  vec3(2, 1, 2)};

int camera_index=0;

/** Powerups */
typedef struct pu {
    int type; // 0: nitro, 1: freeze, 2: handle
    float duration;
} powerup;

int pu_index = 0;
std::vector<glm::vec2> pu_nitro_pos = {vec2(88.1, 0.4), vec2(31.1, 53.7), vec2(85.7, 167.6)};
std::vector<glm::vec2> pu_freeze_pos = {vec2(32.4, 103.0), vec2(10.4, 242.1), vec2(-93.4, 7.7)};
std::vector<glm::vec2> pu_handle_pos = {vec2(32.2, 50.7), vec2(-4.4, 98.9), vec2(-37.1, 0.0)};

powerup nitro;
powerup freeze;
powerup handle;

std::vector<powerup> powerup_types = {nitro, freeze, handle};

bool car_occupied_slot = false;
powerup car_pu_slot;
float car_powerup_delay_upd = 0; //atualiza

void initPowerups(){
    nitro.type = 0;
    nitro.duration = 6;

    freeze.type = 1;
    freeze.duration = 4;

    handle.type = 2;
    handle.duration = 6;
}

/** Checkpoints */

glm::vec2 current_checkpoint_pos;
double checkpoint_radius = 12.0;
int checkpoint_index = 0;
float checkpoint_angle = 0;

/** terrain info */
double road_length = 8.0;

std::vector<glm::vec2> pinpoints = {vec2(88.0, 0.0), vec2(90.0, 75.0), vec2(120.0, 75.0), vec2(120.0, 31.0),
                                    vec2(196.0, 31.0), vec2(196.0, -45.0), vec2(151.0, -45.0), vec2(151.0, -95.0),
                                    vec2(132.0, -95.0), vec2(117.0, -79.0), vec2(43.0, -79.0), vec2(43.0, -45.0),
                                    vec2(-23.0, -45.0), vec2(-23.0, 0.0)
                                   };

std::vector<glm::vec2> checkpoints = {vec2(88.1, 0.4), vec2(92.1, 1.2), vec2(98.4, 3.5), vec2(101.2, 5.8), vec2(102.9, 8.2),
                                       vec2(105.3, 12.4), vec2(106.6, 17.3), vec2(106.2, 24.7), vec2(104.2, 30.1), vec2(101.8, 33.8),
                                       vec2(98.3, 36.3), vec2(94.2, 38.5), vec2(90.6, 39.2), vec2(84.6, 39.9), vec2(48.5, 40.1),
                                       vec2(45.1, 40.8), vec2(41.6, 42.0), vec2(37.8, 43.9), vec2(34.5, 47.5), vec2(32.2, 50.7),
                                       vec2(31.1, 53.7), vec2(30.4, 56.7), vec2(29.9, 60.9), vec2(29.9, 89.2), vec2(30.3, 93.7),
                                       vec2(31.2, 98.0), vec2(32.4, 103.0), vec2(34.3, 107.0), vec2(37.2, 111.7), vec2(40.7, 115.0),
                                       vec2(43.5, 118.4), vec2(47.3, 121.7), vec2(51.3, 125.1), vec2(72.0, 140.5), vec2(77.3, 145.7),
                                       vec2(80.1, 149.6), vec2(83.2, 154.6), vec2(83.9, 157.3), vec2(84.8, 160.4), vec2(85.5, 164.0),
                                       vec2(85.7, 167.6), vec2(85.9, 172.3), vec2(85.8, 208.3), vec2(85.3, 214.4), vec2(84.4, 219.6),
                                       vec2(83.1, 223.9), vec2(81.0, 228.8), vec2(79.0, 233.0), vec2(76.6, 236.3), vec2(72.4, 241.3),
                                       vec2(68.9, 244.1), vec2(64.7, 246.9), vec2(61.1, 249.1), vec2(56.9, 250.5), vec2(51.6, 252.1),
                                       vec2(45.4, 253.1), vec2(41.5, 253.4), vec2(36.7, 253.4), vec2(32.6, 252.8), vec2(27.3, 251.5),
                                       vec2(22.4, 249.6), vec2(18.2, 247.6), vec2(13.9, 245.2), vec2(10.4, 242.1), vec2(6.5, 237.8),
                                       vec2(3.6, 233.2), vec2(1.1, 228.4), vec2(-0.6, 223.7), vec2(-1.6, 219.5), vec2(-2.3, 213.7),
                                       vec2(-2.8, 207.9), vec2(-3.0, 202.7), vec2(-3.0, 161.8), vec2(-3.0, 106.0), vec2(-3.3, 102.3),
                                       vec2(-4.4, 98.9), vec2(-5.5, 96.8), vec2(-7.7, 93.6), vec2(-10.4, 90.8), vec2(-14.1, 88.3),
                                       vec2(-17.9, 86.7), vec2(-21.0, 86.0), vec2(-24.8, 85.9), vec2(-29.5, 86.0), vec2(-59.0, 85.9),
                                       vec2(-80.3, 85.7), vec2(-85.1, 84.8), vec2(-88.1, 83.0), vec2(-91.9, 80.3), vec2(-94.7, 76.8),
                                       vec2(-96.3, 73.3), vec2(-97.1, 69.8), vec2(-97.5, 65.5), vec2(-97.5, 39.8), vec2(-97.6, 19.4),
                                       vec2(-96.9, 14.9), vec2(-95.7, 11.9), vec2(-93.4, 7.7), vec2(-91.2, 5.2), vec2(-89.5, 3.8),
                                       vec2(-85.5, 1.6),  vec2(-81.7, 0.3), vec2(-79.4, -0.1), vec2(-76.9, 0.0), vec2(-28.1, 0.0)
                                       };

//  The number of frames
int frameCount = 0;

//  Number of frames per second
float fps = 0;

//  currentTime - previousTime is the time elapsed
//  between every call of the Idle function
int currentTime = 0, previousTime = 0;

/** Funções auxiliares */

int pseudo = -2;

int pseudoRandom2(){
    pseudo += 2;
    if(pseudo > 2){
        pseudo = -2;
    }

    return pseudo;
}

void restart_game(){
    race_over = false;
    number_of_laps = 0;
    place = 1;
    count_down = 4;
    game_pause = false;
    begining_game = true;
    keystates['p'] = true;

    currentTime = 0;
    previousTime = 0;
    frameCount = 0;
    /** player position */
    posx = 3;
    posz = 0;
    posy = 1.5f;

    /* Some globals */
    car_angle = 270;
    camAngle = 0;
    pauseCamAngle = 0;
    velocity = 0;
    maxSpeed = 1.0;
    drift = false;
    bot_last_pinpoint[0] = 0;
    bot_last_pinpoint[1] = 0;
    bot_last_pinpoint[2] = 0;
    bot_position[0] = vec2(3.0, 3.0);
    bot_position[1] = vec2(6.0, 3.0);
    bot_position[2] = vec2(6.0, 0.0);
    bot_angle[0] = 0.0f;
    bot_angle[1] = 0.0f;
    bot_angle[2] = 0.0f;
    bot_velocity[0] = 0.0f;
    bot_velocity[1] = 0.0f;
    bot_velocity[2] = 0.0f;
    bot_maxSpeed[0] = MAXSPEED;
    bot_maxSpeed[1] = MAXSPEED;
    bot_maxSpeed[2] = MAXSPEED;
    bot_laps[0] = 0;
    bot_laps[1] = 0;
    bot_laps[2] = 0;

    change_camera_delay = 0.5;
    current_camera_delay = 0.5;

    pseudo = -2;

    //Power-ups
    car_occupied_slot = false;
    car_powerup_delay_upd = 0;
    pu_index = 0;
    pu_nitro_pos = {vec2(88.1, 0.4), vec2(31.1, 53.7), vec2(85.7, 167.6)};
    pu_freeze_pos = {vec2(32.4, 103.0), vec2(10.4, 242.1), vec2(-93.4, 7.7)};
    pu_handle_pos = {vec2(32.2, 50.7), vec2(-4.4, 98.9), vec2(-37.1, 0.0)};
}

/** Sphere Collision */

// colliders
typedef struct sphere_col
{
    double radius;
    double x;
    double z;
} sphereCollider;

// uma espécie de construtor
void getObjectSphereCollider(sphereCollider *collider, int x, int z, int radius)
{
    collider->x = x;
    collider->z = z;
    collider->radius = radius;
}

bool SphereColliderCmp(sphereCollider sphere1, sphereCollider sphere2)
{
    double distance_x = sphere2.x - sphere1.x;
    double distance_z = sphere2.z - sphere1.z;
    double distance = sqrt(distance_x*distance_x + distance_z*distance_z);
    double sum_radius = sphere1.radius + sphere2.radius;

    return sum_radius > distance;
}

/** Retactangular Collision */
typedef struct ret_col
{
    vec2 up;
    vec2 down;
    vec2 right;
    vec2 left;
} rectangularCollider;

void getObjectRectangularCollider(rectangularCollider *collider, int x, int z, vec2 desloc)
{
    collider->up = vec2(x+desloc.x, z);
    collider->down = vec2(x-desloc.x, z);
    collider->right = vec2(x, z+desloc.y);
    collider->left = vec2(x, z-desloc.y);
}

/** Vector logic */

float distance(vec2 p, vec2 v)
{
    return sqrt(pow(v.y - p.y, 2) + pow(v.x - p.x, 2) );
}

float length_squared(vec2 p, vec2 v)
{
    return (pow(v.y - p.y, 2) + pow(v.x - p.x, 2));
}

float dot(vec2 p, vec2 v)
{
    return p.x * v.x + p.y * v.y;
}

vec2 vetor_unitario_ponto(vec2 p1, vec2 p2){
    vec2 new_point;
    float p_distance = distance(p1, p2);
    new_point.y = (p2.y - p1.y) / p_distance;
    new_point.x = (p2.x - p1.x) / p_distance;
    return new_point;
}

vec2 move_car(vec2 p1, vec2 p2, int bot){
    int deslocation = 0;
    if(bot == 1){
        deslocation = 2;
    }
    if(bot == 2){
        deslocation = -2;
    }
    p2.y += deslocation;
    p2.x += deslocation;
    float distance_before = distance(p1, p2);
    vec2 bot_direction = vetor_unitario_ponto(p1, p2);
    p1.y += bot_direction.y * bot_velocity[bot];
    p1.x += bot_direction.x * bot_velocity[bot];
    float distance_after = distance(p1, p2);
    if(distance_before <= distance_after){
        bot_last_pinpoint[bot]++;
        if(bot_last_pinpoint[bot] == checkpoints.size()){
            bot_last_pinpoint[bot] = 0;
            bot_laps[bot]++;
        }
    }

    return p1;
}


vec2 test_pinpoint(vec2 p1, vec2 p2, int last_pinpoint){
}

float vectorSize(vec2 a){
    return sqrt(pow(a.x, 2.0) + pow(a.y, 2.0));
}

float crossAngle(vec2 vector1, vec2 vector2){

    vector1 = vector1 / vectorSize(vector1);
    vector2 = vector2 / vectorSize(vector2);

    float dotcalc = dot(vector1, vector2);

    float cos0 = dotcalc;

    //getchar();

    return acos(cos0) * 180.0 / pi;
}

float minimum_distance(vec2 v, vec2 w, vec2 p)
{
    // Return minimum distance between line segment vw and point p
    const float l2 = length_squared(v, w);  // i.e. |w-v|^2 -  avoid a sqrt
    if (l2 == 0.0) return distance(p, v);   // v == w case
    // Consider the line extending the segment, parameterized as v + t (w - v).
    // We find projection of point p onto the line.
    // It falls where t = [(p-v) . (w-v)] / |w-v|^2
    const float t = dot(p - v, w - v) / l2;
    if (t < 0.0) return distance(p, v);       // Beyond the 'v' end of the segment
    else if (t > 1.0) return distance(p, w);  // Beyond the 'w' end of the segment
    const vec2 projection = v + t * (w - v);  // Projection falls on the segment
    return distance(p, projection);
}

void loadTexture(unsigned int width, unsigned int height, const unsigned char * data)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D, // target
        0, // level, 0 = base, no minimap,
        GL_RGB, // internalformat
        width, // width
        height, // height
        0, // border, always 0 in OpenGL ES
        GL_RGB, // format
        GL_UNSIGNED_BYTE, // type
        data // data
    );
}

int tamanho;

int init_resources()
{
    int i;
    for (i = 0; i < 256; i++)
    {
        keystates[i] = false;
    }

    glClearColor(0.4f, 0.698f, 1.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);

    //load objects
    bool res = loadOBJ("objects/bezcurve2.obj", roadVertices, roadUV, roadNormals);
    bool res2 = loadOBJ("objects/car4.obj", carVertices, carUV, carNormals);
    bool res3 = loadOBJ("objects/sand2.obj", areiaVertices, areiaUV, areiaNormals);
    bool res4 = loadOBJ("objects/arrow.obj", checkVertices, checkUV, checkNormals);
    bool res5 = loadOBJ("objects/finish.obj", finishVertices, finishUV, finishNormals);
    bool res6 = loadOBJ("objects/powerup.obj", powerVertices, powerUV, powerNormals);

    initText2D("textures/Arial.dds");

    //setup vertexID
    glGenVertexArrays(1, &vertexID);
    glBindVertexArray(vertexID);

    glGenVertexArrays(1, &vertexID2);
    glBindVertexArray(vertexID2);

    glGenVertexArrays(1, &vertexID3);
    glBindVertexArray(vertexID3);

    glGenVertexArrays(1, &vertexID4);
    glBindVertexArray(vertexID4);

    glGenVertexArrays(1, &vertexID5);
    glBindVertexArray(vertexID5);

    glGenVertexArrays(1, &vertexID6);
    glBindVertexArray(vertexID6);

    gLight.position = vec3(105.0f, 5.0f, 125.0f);
    gLight.intensities = vec3(1.0f, 1.0f, 1.0f);

    programID = LoadShaders( "TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader" );
    matrixID = glGetUniformLocation(programID, "MVP");
    modelID = glGetUniformLocation(programID, "model");

    /**loading textures */

    //road texture
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &textureID1);
    glBindTexture(GL_TEXTURE_2D, textureID1);

    textureID1  = loadBMP_custom("textures/road.bmp");

    //car texture
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &textureID2);
    glBindTexture(GL_TEXTURE_2D, textureID2);

    textureID2  = loadBMP_custom("textures/texture.bmp");

    //sand texture
    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &textureID3);
    glBindTexture(GL_TEXTURE_2D, textureID3);

    textureID3  = loadBMP_custom("textures/sand.bmp");

    //checkpoint texture
    glActiveTexture(GL_TEXTURE3);
    glGenTextures(1, &textureID4);
    glBindTexture(GL_TEXTURE_2D, textureID4);

    textureID4  = loadBMP_custom("textures/green.bmp");

    //checkpoint texture
    glActiveTexture(GL_TEXTURE4);
    glGenTextures(1, &textureID5);
    glBindTexture(GL_TEXTURE_2D, textureID5);

    textureID5  = loadBMP_custom("textures/finish.bmp");

    // bot1 texture
    glActiveTexture(GL_TEXTURE5);
    glGenTextures(1, &textureID6);
    glBindTexture(GL_TEXTURE_2D, textureID6);

    textureID6  = loadBMP_custom("textures/bot1.bmp");

    // bot2 texture
    glActiveTexture(GL_TEXTURE6);
    glGenTextures(1, &textureID7);
    glBindTexture(GL_TEXTURE_2D, textureID7);

    textureID7  = loadBMP_custom("textures/bot2.bmp");

    // bot3 texture
    glActiveTexture(GL_TEXTURE7);
    glGenTextures(1, &textureID8);
    glBindTexture(GL_TEXTURE_2D, textureID8);

    textureID8  = loadBMP_custom("textures/bot3.bmp");

    // powerup nitro texture
    glActiveTexture(GL_TEXTURE8);
    glGenTextures(1, &textureID9);
    glBindTexture(GL_TEXTURE_2D, textureID9);

    textureID9  = loadBMP_custom("textures/pu_nitro.bmp");

    // powerup freeze texture
    glActiveTexture(GL_TEXTURE9);
    glGenTextures(1, &textureID10);
    glBindTexture(GL_TEXTURE_2D, textureID10);

    textureID10  = loadBMP_custom("textures/pu_freeze.bmp");

    // powerup handle texture
    glActiveTexture(GL_TEXTURE10);
    glGenTextures(1, &textureID11);
    glBindTexture(GL_TEXTURE_2D, textureID11);

    textureID11  = loadBMP_custom("textures/pu_handle.bmp");


    /** generate and bind vertices and uvs */
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, roadVertices.size() * sizeof(glm::vec3), &roadVertices[0], GL_STATIC_DRAW);
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, roadUV.size() * sizeof(glm::vec2), &roadUV[0], GL_STATIC_DRAW);

    glGenBuffers(1, &vertexBuffer2);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer2);
    glBufferData(GL_ARRAY_BUFFER, carVertices.size() * sizeof(glm::vec3), &carVertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvBuffer2);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer2);
    glBufferData(GL_ARRAY_BUFFER, carUV.size() * sizeof(glm::vec2), &carUV[0], GL_STATIC_DRAW);

    glGenBuffers(1, &vertexBuffer3);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer3);
    glBufferData(GL_ARRAY_BUFFER, areiaVertices.size() * sizeof(glm::vec3), &areiaVertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvBuffer3);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer3);
    glBufferData(GL_ARRAY_BUFFER, areiaUV.size() * sizeof(glm::vec2), &areiaUV[0], GL_STATIC_DRAW);

    glGenBuffers(1, &vertexBuffer4);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer4);
    glBufferData(GL_ARRAY_BUFFER, checkVertices.size() * sizeof(glm::vec3), &checkVertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvBuffer4);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer4);
    glBufferData(GL_ARRAY_BUFFER, checkUV.size() * sizeof(glm::vec2), &checkUV[0], GL_STATIC_DRAW);

    glGenBuffers(1, &vertexBuffer5);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer5);
    glBufferData(GL_ARRAY_BUFFER, finishVertices.size() * sizeof(glm::vec3), &finishVertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvBuffer5);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer5);
    glBufferData(GL_ARRAY_BUFFER, finishUV.size() * sizeof(glm::vec2), &finishUV[0], GL_STATIC_DRAW);

    glGenBuffers(1, &vertexBuffer6);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer6);
    glBufferData(GL_ARRAY_BUFFER, powerVertices.size() * sizeof(glm::vec3), &powerVertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvBuffer6);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer6);
    glBufferData(GL_ARRAY_BUFFER, powerUV.size() * sizeof(glm::vec2), &powerUV[0], GL_STATIC_DRAW);

    return programID;
}

void keyboardDown(unsigned char key, int x, int y)
{
    if(key != 'p')
    {
        keystates[key] = true;
    }
    else
    {
        if(keystates[key])
        {
            if(!race_over){
                keystates[key] = false;
                camAngle = pauseCamAngle;
            }
        }
        else
        {
            keystates[key] = true;
            pauseCamAngle = camAngle;
        }
    }
}

void keyboardUp(unsigned char key, int x, int y)
{
    if(key != 'p')
    {
        keystates[key] = false;
        if(key == 's')
        {
            camAngle = 0;
        }
    }
    if(key == 'u' || key == 'i')
    {
        changingMusic = false;
    }
    if(key == 'j' || key == 'k')
    {
        changingVolume = false;
    }
}

void specialKeyboardDown(int key, int x, int y)
{
    if(key == GLUT_KEY_UP)
    {
        keystates[upArrow] = true;
    }
    if(key == GLUT_KEY_DOWN)
    {
        keystates[downArrow] = true;
    }
    if(key == GLUT_KEY_RIGHT)
    {
        keystates[rightArrow] = true;
    }
    if(key == GLUT_KEY_LEFT)
    {
        keystates[leftArrow] = true;
    }
}

void specialKeyboardUp(int key, int x, int y)
{
    if(key == GLUT_KEY_UP)
    {
        keystates[upArrow] = false;
    }
    if(key == GLUT_KEY_DOWN)
    {
        keystates[downArrow] = false;
    }
    if(key == GLUT_KEY_RIGHT)
    {
        keystates[rightArrow] = false;
    }
    if(key == GLUT_KEY_LEFT)
    {
        keystates[leftArrow] = false;
    }
}

void print(int x, int y, char *message)
{
    //set the position of the text in the window using the x and y coordinates
    glRasterPos2f(x,y);
    //get the length of the string to display
    int len = (int) strlen(message);

    //loop to display character by character
    for (int i = 0; i < len; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,message[i]);
    }
}

void calculateFPS()
{
    //  Increase frame count
    frameCount++;

    //  Get the number of milliseconds since glutInit called
    //  (or first call to glutGet(GLUT ELAPSED TIME)).
    currentTime = glutGet(GLUT_ELAPSED_TIME);

    //  Calculate time passed
    int timeInterval = currentTime - previousTime;

    if(timeInterval > 1000)
    {
        //  calculate the number of frames per second
        fps = frameCount / (timeInterval / 1000.0f);

        //  Set time
        previousTime = currentTime;

        //  Reset frame count
        frameCount = 0;
    }
}

void idle()
{
    struct timeval start_timer, end_timer;

    long mtime, seconds, useconds;

    gettimeofday(&start_timer, NULL);

    if(count_down >= 1){
        count_down -= 0.02;
        keystates['p'] = true;
    }
    else{
        if(count_down >= 0){
            count_down -= 0.02;
            keystates['p'] = false;
        }
        else{
            begining_game = false;
        }
    }


    if(!(playList[currentMusic]->isPlaying()))
    {
        playList[currentMusic]->play();
        playList[currentMusic]->setVolume(volume);
    }
    if(keystates['j'] && !changingVolume)
    {
        if(volume > 0.02f)
        {
            changingVolume = true;
            volume -= 0.02;
            playList[currentMusic]->setVolume(volume);
        }
    }
    if(keystates['k'] && !changingVolume)
    {
        if(volume < 1.0f)
        {
            changingVolume = true;
            volume += 0.02;
            playList[currentMusic]->setVolume(volume);
        }
    }
    if(keystates['u'] && !changingMusic)
    {
        changingMusic = true;
        playList[currentMusic]->stop();
        playList[currentMusic]->reset();
        currentMusic -= 1;
        if(currentMusic <0)
        {
            currentMusic = playListSize-1;
        }
    }
    if(keystates['i'] && !changingMusic)
    {
        changingMusic = true;
        playList[currentMusic]->stop();
        playList[currentMusic]->reset();
        currentMusic += 1;
        if(currentMusic >= playListSize)
        {
            currentMusic = 0;
        }
    }
    // camera
    if(current_camera_delay > 0){
        current_camera_delay -= 0.05;
    }
    else{
        current_camera_delay = 0;
    }

    if(keystates['v'])
    {
        if(current_camera_delay <= 0)
        {
            camera_index++;

            if(camera_index == camera_pos.size())
            {
                camera_index = 0;
            }

            current_camera_pos = camera_pos[camera_index];
            current_camera_look = camera_look[camera_index];

            current_camera_delay = change_camera_delay;
        }

    }

    if(!keystates['p'])
    {
        game_pause = false;
        float mindis = 10000;
        glm::vec2 car_pos;
        vec2 bot_pos;
        car_pos.x = posx;
        car_pos.y = posz;
        float dis = 10000;
        int j = 0;
        int i = 0;

        int bot;
        int deslocation;
        for(bot = 0; bot < bot_number; bot++){
            if(bot_velocity[bot] < bot_maxSpeed[bot]){
                bot_velocity[bot] += acceleration;
            }
            if(bot_velocity[bot] > bot_maxSpeed[bot]){
                bot_velocity[bot] -= acceleration*3;
            }
            bot_position[bot] = move_car(bot_position[bot], checkpoints[bot_last_pinpoint[bot]], bot);
        }

        for(i = 0; i < checkpoints.size()-1; i++)
        {
            dis = minimum_distance(checkpoints[i], checkpoints[i+1], car_pos);
            //printf("%.2f ", dis);
            if ( dis < mindis)
            {
                //printf("- %.2f to %.2f - ", mindis, dis);
                mindis = dis;
            }
        }

        dis = minimum_distance(checkpoints[i], checkpoints[0], car_pos);
        if ( dis < mindis)
        {
            mindis = dis;
        }

        printf("\n Distance to middle: %.2f\n",mindis);
        if(mindis > road_length)
        {
            printf(" Voce esta na areia!\n");
            if (abs(velocity) > maxSpeed/slowFactorAreia)
            {
                if (velocity > 0)
                {
                    velocity -= acceleration * 10;
                }
                else
                {
                    velocity += acceleration * 10;
                }
            }
            if(velocity != 0)
            {
                if(posy > 1.5)
                {
                    posy -= 0.03;
                }
                else
                {
                    posy += 0.015;
                }
            }
            // gravel sound
            sound3->play();
            sound3->setVolume(abs(velocity*0.5));
        }
        else
        {
            sound3->stop();
            printf(" Voce esta na pista!\n");
            posy = 1.5f;
        }

        // engine
        sound2->play();
        sound2->setPitchShift(abs(velocity)*7);
        sound2->setVolume(abs(velocity)*1.3);

        sphereCollider car_sphere;
        sphereCollider pu_sphere;
        sphereCollider bot_sphere;

        int offset;
        int pos = checkpoints.size();

        /** Power ups */
        getObjectSphereCollider(&car_sphere, posx, posz, car_radius);
        srand (time(NULL));
        offset = rand() % pos;

        for(int i = 0; i < pu_nitro_pos.size(); i++){
            getObjectSphereCollider(&pu_sphere, pu_nitro_pos[i].x, pu_nitro_pos[i].y, 2.0);
            if(SphereColliderCmp(car_sphere, pu_sphere)){
                printf("Colidindo com a esfera %d de Nitro \n", i);
                if(!car_occupied_slot){
                    // troca posição da esfera pra uma aleatória (função)
                    pu_nitro_pos[i].x = checkpoints[offset].x + pseudoRandom2();
                    pu_nitro_pos[i].y = checkpoints[offset].y + pseudoRandom2();
                    // adiciona o power up
                    car_occupied_slot = true;
                    car_pu_slot = nitro;
                }

            }
        }

        for(int i = 0; i < pu_freeze_pos.size(); i++){
            getObjectSphereCollider(&pu_sphere, pu_freeze_pos[i].x, pu_freeze_pos[i].y, 2.0);
            if(SphereColliderCmp(car_sphere, pu_sphere)){
                printf("Colidindo com a esfera %d de Freeze \n", i);
                if(!car_occupied_slot){
                    // troca posição da esfera pra uma aleatória (função)
                    pu_freeze_pos[i].x = checkpoints[offset].x + pseudoRandom2();
                    pu_freeze_pos[i].y = checkpoints[offset].y + pseudoRandom2();
                    // adiciona o power up
                    car_occupied_slot = true;
                    car_pu_slot = freeze;
                }
            }
        }

        for(int i = 0; i < pu_handle_pos.size(); i++){
            getObjectSphereCollider(&pu_sphere, pu_handle_pos[i].x, pu_handle_pos[i].y, 2.0);
            if(SphereColliderCmp(car_sphere, pu_sphere)){
                printf("Colidindo com a esfera %d de Handle \n", i);
                if(!car_occupied_slot){
                    // troca posição da esfera pra uma aleatória (função)
                    pu_handle_pos[i].x = checkpoints[offset].x + pseudoRandom2();
                    pu_handle_pos[i].y = checkpoints[offset].y + pseudoRandom2();
                    // adiciona o power up
                    car_occupied_slot = true;
                    car_pu_slot = handle;
                }
            }
        }

        if(car_powerup_delay_upd > 0){
            car_powerup_delay_upd -= 0.05;
        }
        else{
            car_powerup_delay_upd = 0;
            maxSpeed = MAXSPEED;
            turning_speed = TURNINGSPEED;
        }

        for(int bot = 0; bot < bot_number; bot++){
            getObjectSphereCollider(&bot_sphere, bot_position[bot].x, bot_position[bot].y, 1.6);
            if(SphereColliderCmp(car_sphere, bot_sphere)){
                printf("Colidindo com o bot %d! \n", bot);
                sound4->play();
                sound4->setPitchShift(1.6);
                sound4->setVolume(2);
                float aux = bot_velocity[bot];
                if(velocity > 0){
                    bot_velocity[bot] = 0;
                    velocity = aux/2;
                }
                if(velocity <= 0){
                    bot_velocity[bot] += abs(velocity)/3;
                    velocity = velocity/3;
                }
            }
        }

        if (keystates[' ']){
            if(car_occupied_slot){
                car_occupied_slot = false;
                float min_distance = 10000;
                int nearst_car = -1;
                switch(car_pu_slot.type){
                    case 0: //nitro
                        printf("Usou nitro\n");
                        velocity = velocity * 1.5;
                        maxSpeed = MAXSPEED * 1.2;
                        car_powerup_delay_upd = nitro.duration;
                        break;

                    case 1: //freeze
                        printf("Usou freeze\n");
                        for(bot = 0; bot < bot_number; bot++){
                            float cars_distance = distance(car_pos, bot_position[bot]);
                            if(cars_distance < min_distance){
                                min_distance = cars_distance;
                                nearst_car = bot;
                            }
                        }
                        bot_velocity[nearst_car] = 0;
                        break;

                    case 2: //handle
                        printf("Usou handle\n");
                        turning_speed = TURNINGSPEED * 2.0f;
                        car_powerup_delay_upd = handle.duration;
                        break;
                }
            }
        }


        //andar para frente ou para tras
        if (keystates[upArrow])     //-9 < z|x < 9
        {
            posz += velocity * cos(pi*car_angle/180);   //cos() e sin() usam radianos, então deve-se multiplicar o
            posx += velocity * sin(pi*car_angle/180);   //angulo por pi e dividir por 180 para ter o valor certo
            if(abs(velocity) < maxSpeed)
            {
                if(velocity >= 0)
                {
                    velocity -= acceleration*6;
                }
                else
                {
                    velocity -= acceleration;
                }
            }
        }
        if (keystates[downArrow])
        {
            posz += velocity * cos(pi*car_angle/180);
            posx += velocity * sin(pi*car_angle/180);
            if(abs(velocity) < maxSpeed)
            {
                if(velocity <= 0)
                {
                    velocity += acceleration*6;
                }
                else
                {
                    if(velocity < maxSpeed/4){
                        velocity += acceleration;
                    }
                }
            }
        }

        if(!keystates[upArrow] && !keystates[downArrow])
        {
            if(velocity > 0)
            {
                velocity -= acceleration * 2;
                if(velocity < 0)
                {
                    velocity = 0;
                }
                posz += velocity * cos(pi*car_angle/180);   //cos() e sin() usam radianos, então deve-se multiplicar o
                posx += velocity * sin(pi*car_angle/180);   //angulo por pi e dividir por 180 para ter o valor certo
            }
            if(velocity < 0)
            {
                velocity += acceleration * 2;
                if(velocity > 0)
                {
                    velocity = 0;
                }
                posz += velocity * cos(pi*car_angle/180);   //cos() e sin() usam radianos, então deve-se multiplicar o
                posx += velocity * sin(pi*car_angle/180);   //angulo por pi e dividir por 180 para ter o valor certo
            }
        }

        //angulo para rotacionar
        if (keystates[leftArrow])
        {
            drift = true;
            if(velocity < 0)
            {
                car_angle += turning_speed;
                if(camAngle <= 20 || camAngle > 350)
                {
                    camAngle -= turning_speed;
                }
            }
            if(velocity > 0)
            {
                car_angle -= turning_speed;
                if(camAngle < 10 || camAngle > 340)
                {
                    camAngle += turning_speed;
                }
            }
        }
        if (keystates[rightArrow])
        {
            drift = true;
            if(velocity < 0)
            {
                car_angle -= turning_speed;
                if(camAngle < 10 || camAngle > 340)
                {
                    camAngle += turning_speed;
                }
            }
            if(velocity > 0)
            {
                car_angle += turning_speed;
                if(camAngle <= 20 || camAngle > 350)
                {
                    camAngle -= turning_speed;
                }
            }
        }
        if(keystates['w'])
        {
            camAngle = 0;
        }
        if(keystates['s'])
        {
            camAngle = 180;
        }
        if(drift && !keystates[rightArrow] && !keystates[leftArrow])
        {
            if(camAngle > 340 && camAngle < 360)
            {
                camAngle += turning_speed;
                if(camAngle >= 360)
                {
                    drift = false;
                    camAngle = 0;
                }
            }
            if(camAngle > 0 && camAngle < 20)
            {
                camAngle -= turning_speed;
                if(camAngle <= 0)
                {
                    drift = false;
                    camAngle = 0;
                }
            }
        }
        if(abs(velocity) > maxSpeed + acceleration){
            if(velocity < 0){
                velocity += acceleration;
            }
            else{
                velocity -= acceleration;
            }
        }

        if (car_angle >= 360)
            car_angle = car_angle - 360;
        if (car_angle < 0)
            car_angle = 360 + car_angle;
        if (camAngle >= 360)
            camAngle = camAngle - 360;
        if (camAngle < 0)
            camAngle = 360 + camAngle;
    }
    else
    {
        sound2->stop();
        sound3->stop();
        if(!race_over){
            game_pause = true;
        }
        if(keystates['q'])
        {
            camAngle += 0.5f;
        }
        if(keystates['e'])
        {
            camAngle -= 0.5f;
        }
        if(keystates['w'])
        {
            camAngle = 0;
        }
        if(keystates['s'])
        {
            camAngle = 180;
        }
        if(keystates['r']){
            restart_game();
        }
    }

    sphereCollider car_collider;
    sphereCollider checkpoint_collider;

    car_collider.radius = car_radius;
    car_collider.x = posx;
    car_collider.z = posz;

    checkpoint_collider.radius = checkpoint_radius;
    checkpoint_collider.x = current_checkpoint_pos.x;
    checkpoint_collider.z = current_checkpoint_pos.y;

    bool col = SphereColliderCmp(car_collider, checkpoint_collider);

    if(col){
        checkpoint_index++;

        s_success->play();
        s_success->setVolume(0.2);

        if(checkpoint_index == checkpoints.size()){
            checkpoint_index = 0;
            number_of_laps++;
        }
        current_checkpoint_pos = checkpoints[checkpoint_index];
        if(number_of_laps > number_of_laps_to_win){
            keystates['p'] = true;
            race_over = true;
        }
    }

    for(int bot=0; bot < bot_number; bot++){
        if(number_of_laps < bot_laps[bot]){
            place++;
            if(place > 4){
                place = 4;
            }
        }
        else{
            if(number_of_laps == bot_laps[bot]){
                if(checkpoint_index <= bot_last_pinpoint[bot]){
                    place++;
                    if(place > 4){
                        place = 4;
                    }
                }
                else{
                    place --;
                    if(place < 1){
                        place = 1;
                    }
                }
            }
            else{
                place--;
                if(place < 1){
                    place = 1;
                }
            }
        }
    }

    glutPostRedisplay();

    /** **/

    calculateFPS();

    gettimeofday(&end_timer, NULL);

    seconds  = end_timer.tv_sec  - start_timer.tv_sec;
    useconds = end_timer.tv_usec - start_timer.tv_usec;

    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

    while(mtime < 1000.0f/game_fps)
    {
        gettimeofday(&end_timer, NULL);

        seconds  = end_timer.tv_sec  - start_timer.tv_sec;
        useconds = end_timer.tv_usec - start_timer.tv_usec;

        mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
    }

    char gameName[] = "Super Speed Runners";

    print(0.1f, 0.1f, gameName);

    printf("Elapsed time: %ld milliseconds\n", mtime);

    printf("\n FPS: %.2f\n Angle: %.0f\n X,Y,Z = (%.1f, %.1f, %.1f)\n Velocity = %.2f\n", fps, car_angle, posx, 0.0, posz, velocity);
}

//draw method
void drawMesh(int vAttri, GLuint vBuffer,
              int tAttri, GLuint tBuffer,
              GLuint texture, GLfloat uniform, int vSize)
{

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(vAttri);
    glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
    glVertexAttribPointer(
        0,                  // attribute
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    //binding texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(texture, uniform);


    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(tAttri);
    glBindBuffer(GL_ARRAY_BUFFER, tBuffer);
    glVertexAttribPointer(
        1,                                // attribute
        2,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

    glDrawArrays(GL_TRIANGLES, 0, vSize );
}

void printToScreen(){
    printText2D("Speed:", 10, 20, 30);

    char speed[5];
 	snprintf(speed, 6, "%f", abs(velocity*100.0));

    printText2D(speed, 200, 20, 30);

    printText2D("Lap:", 10, 60, 30);

    char lap[5];
 	snprintf(lap, 6, "%d", number_of_laps);

 	printText2D(lap, 150, 60, 30);

 	printText2D("/", 180, 60, 30);

 	char lap2[5];
 	snprintf(lap2, 6, "%d", number_of_laps_to_win);

 	printText2D(lap2, 210, 60, 30);

 	printText2D("Place:", 10, 100, 30);

 	char position[5];
    snprintf(position, 6, "%d", place);

    printText2D(position, 190, 100, 30);

 	printText2D("/", 220, 100, 30);

 	printText2D(lap2, 250, 100, 30);

 	printText2D("Power:", 10, 140, 30);

 	if(car_occupied_slot){
        switch(car_pu_slot.type){
            case 0: printText2D("Nitro", 190, 140, 30);
                    break;
            case 1: printText2D("Freeze", 190, 140, 30);
                    break;
            case 2: printText2D("Handle", 190, 140, 30);
                    break;
        }
 	}
 	else{
        printText2D("None", 190, 140, 30);
 	}


 	if(race_over){
        printText2D("RACE OVER", 180, 500, 50);
        char position[40];
        snprintf(position, 9, "%d Place", place);
        printText2D(position, 220, 440, 50);
 	}
 	if(game_pause && !begining_game){
        printText2D("RACE PAUSED", 160, 500, 50);
 	}
 	if(begining_game){
        if(count_down>=1){
            char cont[5];
            snprintf(cont, 6, "%d", int(count_down));
            printText2D(cont, 390, 400, 50);
        }
        else{
            printText2D("GO", 380, 400, 50);
        }
 	}

}

void onDisplay()
{

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram(programID);

    glm::mat4 Projection = glm::perspective(60.0f, 16.0f / 9.0f, 0.1f, 100.0f);

    glm::mat4 View       = glm::lookAt(

                               glm::vec3(posx-(current_camera_pos.x)*cos(pi*(-90-car_angle-camAngle)/180), (current_camera_pos.y), posz-(current_camera_pos.z)*sin(pi*(-90-car_angle-camAngle)/180)),
                               glm::vec3(posx+(current_camera_look.x)*cos(pi*(-90-car_angle-camAngle)/180), (current_camera_look.y), posz+(current_camera_look.z)*sin(pi*(-90-car_angle-camAngle)/180)),

                               glm::vec3(0,1,0)
                           );

    glm::mat4 Model      = glm::mat4(1.0f);

    glm::mat4 MVP;

    // road

    glm::mat4 transRoad = glm::translate(mat4(1.0f), vec3(0.0, 0.75f, 0.0));

    MVP        = Projection * View * Model * transRoad;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer, 1, uvBuffer, textureID1, 0, roadVertices.size());

    // car
    glm::mat4 escCar = glm::scale(mat4(1.0f), vec3(0.3f, 0.3f, 0.3f));
    glm::mat4 transCar = glm::translate(mat4(1.0f), vec3(posx, posy, posz));
    glm::mat4 fixCar = glm::rotate(mat4(1.0f), 90.0f, vec3(0, 1.0f, 0));
    glm::mat4 rotCar = glm::rotate(mat4(1.0f), car_angle, vec3(0, 1.0f, 0));
    MVP        = Projection * View * Model * transCar * escCar * rotCar * fixCar;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer2, 1, uvBuffer2, textureID2, 1, carVertices.size());

    // bots
    glm::mat4 escBot1 = glm::scale(mat4(1.0f), vec3(0.3f, 0.3f, 0.3f));
    glm::mat4 transBot1 = glm::translate(mat4(1.0f), vec3(bot_position[0].x, 1.5f, bot_position[0].y));
    glm::mat4 fixBot1 = glm::rotate(mat4(1.0f), 0.0f, vec3(0, 1.0f, 0));
    glm::mat4 rotBot1 = glm::rotate(mat4(1.0f), bot_angle[0], vec3(0, 1.0f, 0));
    MVP        = Projection * View * Model * transBot1 * escBot1 * rotBot1 * fixBot1;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer2, 1, uvBuffer2, textureID6, 5, carVertices.size());

    glm::mat4 escBot2 = glm::scale(mat4(1.0f), vec3(0.3f, 0.3f, 0.3f));
    glm::mat4 transBot2 = glm::translate(mat4(1.0f), vec3(bot_position[1].x, 1.5f, bot_position[1].y));
    glm::mat4 fixBot2 = glm::rotate(mat4(1.0f), 0.0f, vec3(0, 1.0f, 0));
    glm::mat4 rotBot2 = glm::rotate(mat4(1.0f), bot_angle[1], vec3(0, 1.0f, 0));
    MVP        = Projection * View * Model * transBot2 * escBot2 * rotBot2 * fixBot2;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer2, 1, uvBuffer2, textureID7, 6, carVertices.size());

    glm::mat4 escBot3 = glm::scale(mat4(1.0f), vec3(0.3f, 0.3f, 0.3f));
    glm::mat4 transBot3 = glm::translate(mat4(1.0f), vec3(bot_position[2].x, 1.5f, bot_position[2].y));
    glm::mat4 fixBot3 = glm::rotate(mat4(1.0f), 0.0f, vec3(0, 1.0f, 0));
    glm::mat4 rotBot3 = glm::rotate(mat4(1.0f), bot_angle[2], vec3(0, 1.0f, 0));
    MVP        = Projection * View * Model * transBot3 * escBot3 * rotBot3 * fixBot3;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer2, 1, uvBuffer2, textureID8, 7, carVertices.size());

    // powerups
    glm::mat4 escPower;
    glm::mat4 transPower;
    glm::mat4 rotPower;

    for(int i = 0; i < pu_nitro_pos.size(); i++){
        escPower = glm::scale(mat4(1.0f), vec3(0.6f, 0.6f, 0.6f));
        transPower = glm::translate(mat4(1.0f), vec3(pu_nitro_pos[i].x, 1.5f, pu_nitro_pos[i].y));
        rotPower = glm::rotate(mat4(1.0f), 0.0f, vec3(0, 1.0f, 0));
        MVP        = Projection * View * Model * transPower * escPower * rotPower;
        glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
        drawMesh(0, vertexBuffer6, 1, uvBuffer6, textureID9, 8, powerVertices.size());
    }

    for(int i = 0; i < pu_freeze_pos.size(); i++){
        escPower = glm::scale(mat4(1.0f), vec3(0.6f, 0.6f, 0.6f));
        transPower = glm::translate(mat4(1.0f), vec3(pu_freeze_pos[i].x, 1.5f, pu_freeze_pos[i].y));
        rotPower = glm::rotate(mat4(1.0f), 0.0f, vec3(0, 1.0f, 0));
        MVP        = Projection * View * Model * transPower * escPower * rotPower;
        glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
        drawMesh(0, vertexBuffer6, 1, uvBuffer6, textureID10, 9, powerVertices.size());
    }

    for(int i = 0; i < pu_handle_pos.size(); i++){
        escPower = glm::scale(mat4(1.0f), vec3(0.6f, 0.6f, 0.6f));
        transPower = glm::translate(mat4(1.0f), vec3(pu_handle_pos[i].x, 1.5f, pu_handle_pos[i].y));
        rotPower = glm::rotate(mat4(1.0f), 0.0f, vec3(0, 1.0f, 0));
        MVP        = Projection * View * Model * transPower * escPower * rotPower;
        glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
        drawMesh(0, vertexBuffer6, 1, uvBuffer6, textureID11, 9, powerVertices.size());
    }

    // sand
    glm::mat4 transSand = glm::translate(mat4(1.0f), vec3(0.0f, 1.0f, 0.0f));
    MVP        = Projection * View * Model * transSand;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer3, 1, uvBuffer3, textureID3, 2, areiaVertices.size());

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // arrow
    glm::mat4 escArrow = glm::scale(mat4(1.0f), vec3(1.0f, 1.0f, 1.0f));
    glm::mat4 transArrow = glm::translate(mat4(1.0f), vec3(current_checkpoint_pos.x, 5.0f, current_checkpoint_pos.y));
    glm::mat4 rotArrow = glm::rotate(mat4(1.0f), 90.0f, vec3(1.0, 0.0f, 0));
    MVP        = Projection * View * Model * transArrow * escArrow * rotArrow;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer4, 1, uvBuffer4, textureID4, 3, checkVertices.size());
    // finish
    glm::mat4 transFinish = glm::translate(mat4(1.0f), vec3(-37.1f, -0.25f, 0.5f));
    MVP        = Projection * View * Model * transFinish;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer5, 1, uvBuffer5, textureID5, 4, finishVertices.size());

    printToScreen();

    //disable
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glutSwapBuffers();
    glutPostRedisplay();

}


//libera recursos
void free_resources()
{

    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vertexID);
    glDeleteProgram(programID);

}


int main(int argc, char* argv[])
{

    //inicia janela
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Super Speed Runners");

    current_camera_pos = camera_pos[0];
    current_camera_look = camera_look[0];
    camera_index = 0;

    srand (time(NULL));

    current_checkpoint_pos = pinpoints[0];

    initPowerups();

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK)
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
        return 1;
    }

    if (init_resources() != 0)
    {
        glutDisplayFunc(onDisplay);

        glutIgnoreKeyRepeat(1);
        glutKeyboardFunc(keyboardDown);
        glutKeyboardUpFunc(keyboardUp);
        glutSpecialFunc(specialKeyboardDown);
        glutSpecialUpFunc(specialKeyboardUp);
        glutIdleFunc(idle);

        glutMainLoop();

    }

    free_resources();
    return 0;
}


