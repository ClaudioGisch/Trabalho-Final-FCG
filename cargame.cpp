
#define GLEW_STATIC

#include <stdio.h>
#include <stdlib.h>
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
#include "audiere-1.9.4-win32/include/audiere.h";
using namespace glm;
using namespace audiere;

#include "common/shader.cpp"
#include "common/texture.cpp"
#include "common/objloader.cpp"

#define pi 3.14159265

bool keystates[256];

GLuint programID;
GLuint matrixID;        // Get a handle for our "MVP" uniform

//road
GLuint vertexID;
GLuint vertexBuffer;
GLuint uvBuffer;
GLuint textureID1;                  // Get a handle for our "myTextureSampler" uniform
std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;

//car
GLuint vertexID2;
GLuint vertexBuffer2;
GLuint uvBuffer2;
GLuint textureID2;                  // Get a handle for our "myTextureSampler" uniform
std::vector<glm::vec3> handVertices;
std::vector<glm::vec2> handUV;
std::vector<glm::vec3> handNormals;

//sand
GLuint vertexID3;
GLuint vertexBuffer3;
GLuint uvBuffer3;
GLuint textureID3;
std::vector<glm::vec3> areiaVertices;
std::vector<glm::vec2> areiaUV;
std::vector<glm::vec3> areiaNormals;

/** Sound */

// background music
string background = "happyrock.mp3";
string cardrive = "carrun.mp3";
string gravel = "gravel.mp3";
AudioDevicePtr device(OpenDevice());
OutputStreamPtr sound(OpenSound(device, background.c_str(), false));
OutputStreamPtr sound2(OpenSound(device, cardrive.c_str(), false));
OutputStreamPtr sound3(OpenSound(device, gravel.c_str(), false));


//player position
double posx = 1;
double posz = 1;
double posy = 1.5f;
float angle = 180;
float camAngle = 0;
float pauseCamAngle = 0;
int upArrow = 0;
int downArrow = 1;
int rightArrow = 2;
int leftArrow = 3;
double velocity = 0;
int slowFactorAreia = 5;
double acceleration = 0.0005;
double maxSpeed = 0.3;
bool drift = false;

// terrain info
std::vector<glm::vec2> pinpoints = {vec2(88.0, 0.0), vec2(90.0, 75.0), vec2(120.0, 75.0), vec2(120.0, 31.0),
                                    vec2(196.0, 31.0), vec2(196.0, -45.0), vec2(151.0, -45.0), vec2(151.0, -95.0),
                                    vec2(132.0, -95.0), vec2(117.0, -79.0), vec2(43.0, -79.0), vec2(43.0, -45.0),
                                    vec2(-23.0, -45.0), vec2(-23.0, 0.0)
                                   };

// colliders
typedef struct sphere_col
{
    double radius;
    double x;
    double z;
} sphereCollider;

//  The number of frames
int frameCount = 0;

//  Number of frames per second
float fps = 0;

//  currentTime - previousTime is the time elapsed
//  between every call of the Idle function
int currentTime = 0, previousTime = 0;

/** Funções auxiliares */

/** Sphere Collision */

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

int init_resources()
{
    int i;
    for (i = 0; i < 256; i++)
    {
        keystates[i] = false;
    }

    glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
    glEnable(GL_DEPTH_TEST);

    //load objects
    bool res = loadOBJ("pista.obj", vertices, uvs, normals);
    bool res2 = loadOBJ("car4.obj", handVertices, handUV, handNormals);
    bool res3 = loadOBJ("areia.obj", areiaVertices, areiaUV, areiaNormals);

    //setup vertexID
    glGenVertexArrays(1, &vertexID);
    glBindVertexArray(vertexID);

    glGenVertexArrays(1, &vertexID2);
    glBindVertexArray(vertexID2);

    glGenVertexArrays(1, &vertexID3);
    glBindVertexArray(vertexID3);

    //programID = LoadShaders( "vertexshader.vs", "fragmentshader.fs" );
    programID = LoadShaders( "TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader" );
    matrixID = glGetUniformLocation(programID, "MVP");

    /**loading textures */

    //road texture
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &textureID1);
    glBindTexture(GL_TEXTURE_2D, textureID1);

    textureID1  = loadBMP_custom("road.bmp");

    //car texture
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &textureID2);
    glBindTexture(GL_TEXTURE_2D, textureID2);

    textureID2  = loadBMP_custom("texture.bmp");

    //sand texture
    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &textureID3);
    glBindTexture(GL_TEXTURE_2D, textureID3);

    textureID3  = loadBMP_custom("sand.bmp");


    /** generate and bind vertices and uvs */
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

    glGenBuffers(1, &vertexBuffer2);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer2);
    glBufferData(GL_ARRAY_BUFFER, handVertices.size() * sizeof(glm::vec3), &handVertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvBuffer2);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer2);
    glBufferData(GL_ARRAY_BUFFER, handUV.size() * sizeof(glm::vec2), &handUV[0], GL_STATIC_DRAW);

    glGenBuffers(1, &vertexBuffer3);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer3);
    glBufferData(GL_ARRAY_BUFFER, areiaVertices.size() * sizeof(glm::vec3), &areiaVertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvBuffer3);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer3);
    glBufferData(GL_ARRAY_BUFFER, areiaUV.size() * sizeof(glm::vec2), &areiaUV[0], GL_STATIC_DRAW);

    /*
    printf("Number of vertices: %d\n",vertices.size());
    for(int i = 0; i < vertices.size(); i++){

        //printf("(%d: %.2f, %.2f, %.2f)\n",i,areiaVertices[i].x,areiaVertices[i].y,areiaVertices[i].z);
        printf("(%.2f, %.2f)\n",vertices[i].x,vertices[i].z);
    }
    */

    return programID;

}


void keyboardDown(unsigned char key, int x, int y)
{
    if(key != 'p'){
      keystates[key] = true;
    }
    else{
        if(keystates[key]){
            keystates[key] = false;
            camAngle = pauseCamAngle;
        }
        else{
            keystates[key] = true;
            pauseCamAngle = camAngle;
        }
    }
}

void keyboardUp(unsigned char key, int x, int y)
{
    if(key != 'p'){
      keystates[key] = false;
      if(key == 's'){
        camAngle = 0;
      }
    }
}

void specialKeyboardDown(int key, int x, int y){
    if(key == GLUT_KEY_UP){
        keystates[upArrow] = true;
    }
    if(key == GLUT_KEY_DOWN){
        keystates[downArrow] = true;
    }
    if(key == GLUT_KEY_RIGHT){
        keystates[rightArrow] = true;
    }
    if(key == GLUT_KEY_LEFT){
        keystates[leftArrow] = true;
    }
}

void specialKeyboardUp(int key, int x, int y){
    if(key == GLUT_KEY_UP){
        keystates[upArrow] = false;
    }
    if(key == GLUT_KEY_DOWN){
        keystates[downArrow] = false;
    }
    if(key == GLUT_KEY_RIGHT){
        keystates[rightArrow] = false;
    }
    if(key == GLUT_KEY_LEFT){
        keystates[leftArrow] = false;
    }
}

unsigned long long x;
unsigned long long y;

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

bool playingGravel = false;

void playGravel(){
    if( playingGravel == false){
            playingGravel = true;
        sound3->play();
        sound3->setVolume(1.0f);
    }
}

void idle()
{
    if(!keystates['p']){

        float mindis = 10000;
        glm::vec2 car_pos;
        car_pos.x = posx;
        car_pos.y = posz;
        float dis;
        int j = 0;
        int i = 0;

        //mindis = minimum_distance(pinpoints[0], pinpoints[13], car_pos);

        for(i = 0; i < pinpoints.size()-1; i++)
        {

            dis = minimum_distance(pinpoints[i], pinpoints[i+1], car_pos);
            if ( dis < mindis)
            {
                mindis = dis;
            }
        }

        dis = minimum_distance(pinpoints[i], pinpoints[0], car_pos);
        if ( dis < mindis)
        {
            mindis = dis;
        }

        printf("\n Distance to middle: %.2f\n",mindis);
        if(mindis > 8)
        {
            printf(" Voce esta na areia!\n");
            if (abs(velocity) > maxSpeed/slowFactorAreia){
                if (velocity > 0)
                {
                    velocity -= acceleration * 10;
                }
                else{
                    velocity += acceleration * 10;
                }
            }
            if(posy > 1.5){
                posy -= 0.03;
            }
            else{
                posy += 0.015;
            }
                    // gravel sound
            sound3->play();
            sound3->setVolume(abs(velocity*1.5));
        }
        else
        {
            sound3->stop();
            printf(" Voce esta na pista!\n");
            posy = 1.5f;
        }

        // engine
        sound2->play();
        sound2->setVolume(abs(velocity)*4);

        //andar para frente ou para tras
        if (keystates[upArrow])     //-9 < z|x < 9
        {
            posz += velocity * cos(pi*angle/180);   //cos() e sin() usam radianos, então deve-se multiplicar o
            posx += velocity * sin(pi*angle/180);   //angulo por pi e dividir por 180 para ter o valor certo
            if(abs(velocity) < maxSpeed)
            {
                if(velocity >= 0){
                    velocity -= acceleration*6;
                }
                else{
                    velocity -= acceleration;
                }
            }
        }
        if (keystates[downArrow])
        {
            posz += velocity * cos(pi*angle/180);
            posx += velocity * sin(pi*angle/180);
            if(abs(velocity) < maxSpeed)
            {
                if(velocity <= 0){
                    velocity += acceleration*6;
                }
                else{
                    velocity += acceleration;
                }
            }
        }

        if(!keystates[upArrow] && !keystates[downArrow])
        {
            if(velocity > 0){
                velocity -= acceleration * 2;
                if(velocity < 0)
                {
                    velocity = 0;
                }
                posz += velocity * cos(pi*angle/180);   //cos() e sin() usam radianos, então deve-se multiplicar o
                posx += velocity * sin(pi*angle/180);   //angulo por pi e dividir por 180 para ter o valor certo
            }
            if(velocity < 0){
                velocity += acceleration * 2;
                if(velocity > 0)
                {
                    velocity = 0;
                }
                posz += velocity * cos(pi*angle/180);   //cos() e sin() usam radianos, então deve-se multiplicar o
                posx += velocity * sin(pi*angle/180);   //angulo por pi e dividir por 180 para ter o valor certo
            }
        }


        /*
        //teste de colisão
        if (posz < -9)
            posz = -9;
        if (posz > 9)
            posz = 9;
        if (posx < -9)
            posx = -9;
        if (posx > 9)
            posx = 9;
        */

        //angulo para rotacionar
        if (keystates[leftArrow]){
            drift = true;
            if(velocity < 0){
                angle += 0.5f;
                if(camAngle <= 0 || camAngle > 350){
                    camAngle -= 0.5f;
                }
            }
            if(velocity > 0){
                angle -= 0.5f;
                if(camAngle < 10){
                    camAngle += 0.5f;
                }
            }
        }
        if (keystates[rightArrow]){
            drift = true;
            if(velocity < 0){

                angle -= 0.5f;
                if(camAngle < 10){
                    camAngle += 0.5f;
                }
            }
            if(velocity > 0){
                angle += 0.5f;
                if(camAngle <= 0 || camAngle > 350){
                    camAngle -= 0.5f;
                }
            }
        }
        if(keystates['w']){
            camAngle = 0;
        }
        if(keystates['s']){
            camAngle = 180;
        }
        if(drift && !keystates[rightArrow]){
            if(camAngle == 0){
                drift = false;
            }
            else{
                if(camAngle < 20){
                    camAngle -= 0.5f;
                }
            }
        }
        if(drift && !keystates[leftArrow]){
            if(camAngle == 0){
                drift = false;
            }
            else{
                if(camAngle > 340){
                    camAngle += 0.5f;
                }
            }
        }

        if (angle == 360)
            angle = 0;
        if (angle == -10)
            angle = 350;
        if (camAngle == 360)
            camAngle = 0;
        if (camAngle == -10)
            camAngle = 350;

        printf("\n FPS: %.2f\n Angle: %.0f\n X,Y,Z = (%.1f, %.1f, %.1f)\n Velocity = %.2f\n", fps, angle, posx, 0.0, posz, velocity);

        glutPostRedisplay();

        calculateFPS();
    }
    else{
        sound2->stop();
        sound3->stop();
        if(keystates['q']){
            camAngle += 0.5f;
        }
        if(keystates['e']){
            camAngle -= 0.5f;
        }
        if(keystates['w']){
            camAngle = 0;
        }
        if(keystates['s']){
            camAngle = 180;
        }
    }
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


void onDisplay()
{

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glBegin(GL_QUADS);
    glColor3d(1,0,0);
    glVertex3f(-1,-1,-10);
    glColor3d(1,1,0);
    glVertex3f(1,-1,-10);
    glColor3d(1,1,1);
    glVertex3f(1,1,-10);
    glColor3d(0,1,1);
    glVertex3f(-1,1,-10);
    glEnd();

    glUseProgram(programID);

    glm::mat4 Projection = glm::perspective(60.0f, 16.0f / 9.0f, 0.1f, 100.0f);

    glm::mat4 View       = glm::lookAt(

                               glm::vec3(posx-3*cos(pi*(-90-angle-camAngle)/180), 4, posz-3*sin(pi*(-90-angle-camAngle)/180)),
                               glm::vec3(posx+2.8*cos(pi*(-90-angle-camAngle)/180), 1, posz+2.8*sin(pi*(-90-angle-camAngle)/180)),

                               glm::vec3(0,1,0)
                           );

    glm::mat4 Model      = glm::mat4(1.0f);

    glm::mat4 MVP;

    // road

    glm::mat4 transPista = glm::translate(mat4(1.0f), vec3(0, 0.0f, 0));

    MVP        = Projection * View * Model * transPista;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer, 1, uvBuffer, textureID1, 0, vertices.size());

    // car
    glm::mat4 escMao = glm::scale(mat4(1.0f), vec3(0.3f, 0.3f, 0.3f));
    glm::mat4 transMao = glm::translate(mat4(1.0f), vec3(posx, posy, posz));
    glm::mat4 fixRot = glm::rotate(mat4(1.0f), 90.0f, vec3(0, 1.0f, 0));
    glm::mat4 rotMao = glm::rotate(mat4(1.0f), angle, vec3(0, 1.0f, 0));
    MVP        = Projection * View * Model * transMao * escMao * rotMao * fixRot;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer2, 1, uvBuffer2, textureID2, 1, handVertices.size());

    // sand
    glm::mat4 transAreia = glm::translate(mat4(1.0f), vec3(0.0f, -0.1f, 0.0f));
    MVP        = Projection * View * Model * transAreia;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer3, 1, uvBuffer3, textureID3, 2, areiaVertices.size());

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
    glutCreateWindow("FPS Game");

    sound->play();
    sound->setRepeat(true);
    sound->setVolume(0.1f);

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


