
#define GLEW_STATIC

#include <stdio.h>
#include <stdlib.h>
//  To use functions with variables arguments
#include <stdarg.h>

//  for malloc
#include <stdlib.h>

#include "GL/glew.h"
#include "GL/freeglut.h"


// Include GLM
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
    using namespace glm;

#include "common/shader.cpp"
#include "common/texture.cpp"
#include "common/objloader.cpp"

#include "moon.c"
#include "skin.c"
#include "brickTexture.c"

#define pi 3.14159265

bool keystates[256];

GLuint programID;
GLuint matrixID;        // Get a handle for our "MVP" uniform

//plane
GLuint vertexID;
GLuint vertexBuffer;
GLuint uvBuffer;
GLuint textureID1;                  // Get a handle for our "myTextureSampler" uniform
std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;

//hands
GLuint vertexID2;
GLuint vertexBuffer2;
GLuint uvBuffer2;
GLuint textureID2;                  // Get a handle for our "myTextureSampler" uniform
std::vector<glm::vec3> handVertices;
std::vector<glm::vec2> handUV;
std::vector<glm::vec3> handNormals;


//brick texture
GLuint textureID3;

//player position
double posx = 1;
double posz = 1;
float angle = 180;
double velocity = 0.1;
double acceleration = 0.0005;
double maxSpeed = 0.3;

// colliders
typedef struct sphere_col{
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

// Funções auxiliares

// uma espécie de construtor
void getObjectSphereCollider(sphereCollider *collider, int x, int z, int radius){
    collider->x = x;
    collider->z = z;
    collider->radius = radius;
}

bool SphereColliderCmp(sphereCollider sphere1, sphereCollider sphere2){

    double distance_x = sphere2.x - sphere1.x;
    double distance_z = sphere2.z - sphere1.z;
    double distance = sqrt(distance_x*distance_x + distance_z*distance_z);
    double sum_radius = sphere1.radius + sphere2.radius;
    //printf("Sphere 1 x: %.2f\n", sphere1.x);
    //printf("Sphere 1 z: %.2f\n", sphere1.z);
    //printf("Sphere 2 x: %.2f\n", sphere2.x);
    //printf("Sphere 2 z: %.2f\n", sphere2.z);
    //printf("Sqrt: %.2f\n", sqrt(1.0 + 9.0));
    //printf("Distance x: %.2f\n", distance_x);
    //printf("Distance z: %.2f\n", distance_z);
    //printf(" Distance: %.2f\n",distance);
    //printf(" Sum Radius: %.2f\n",sum_radius);

    return sum_radius > distance;
}

void loadTexture(unsigned int width, unsigned int height, const unsigned char * data) {

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

int init_resources() {

    int i;
    for (i = 0; i < 256; i++) {
        keystates[i] = false;
    }


	glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
	glEnable(GL_DEPTH_TEST);

    //load objects
    bool res = loadOBJ("pista.obj", vertices, uvs, normals);
    bool res2 = loadOBJ("simplecar.obj", handVertices, handUV, handNormals);

    //setup vertexID
    glGenVertexArrays(1, &vertexID);
    glBindVertexArray(vertexID);

    glGenVertexArrays(1, &vertexID2);
    glBindVertexArray(vertexID2);

    //programID = LoadShaders( "vertexshader.vs", "fragmentshader.fs" );
    programID = LoadShaders( "TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader" );
    matrixID = glGetUniformLocation(programID, "MVP");


    //loading textures

    //plane texture
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &textureID1);
    glBindTexture(GL_TEXTURE_2D, textureID1);

    loadTexture(brickTexture.width, brickTexture.height, brickTexture.pixel_data);
	textureID1  = glGetUniformLocation(programID, "myTextureSampler");


    //generate and bind vertices and uvs
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

    //skin texture
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &textureID2);
    glBindTexture(GL_TEXTURE_2D, textureID2);

    loadTexture(skin.width, skin.height, skin.pixel_data);
   // loadTexture(terra.width, terra.height, terra.pixel_data);
	textureID2  = glGetUniformLocation(programID, "myTextureSampler");

    //brick texture
    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &textureID3);
    glBindTexture(GL_TEXTURE_2D, textureID3);

    loadTexture(brickTexture.width, brickTexture.height, brickTexture.pixel_data);
	textureID3  = glGetUniformLocation(programID, "myTextureSampler");


    //generate and bind vertices and uvs
	glGenBuffers(1, &vertexBuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer2);
	glBufferData(GL_ARRAY_BUFFER, handVertices.size() * sizeof(glm::vec3), &handVertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvBuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer2);
	glBufferData(GL_ARRAY_BUFFER, handUV.size() * sizeof(glm::vec2), &handUV[0], GL_STATIC_DRAW);


    return programID;

}


void keyboardDown(unsigned char key, int x, int y) {

    keystates[key] = true;

}

void keyboardUp(unsigned char key, int x, int y) {

    keystates[key] = false;

}

unsigned long long x;
unsigned long long y;


inline uint64_t rdtsc() {
    uint32_t lo, hi;
    __asm__ __volatile__ (
      "xorl %%eax, %%eax\n"
      "cpuid\n"
      "rdtsc\n"
      : "=a" (lo), "=d" (hi)
      :
      : "%ebx", "%ecx");
    return (uint64_t)hi << 32 | lo;
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

void idle() {

    // 2 2 1
    // 3 5 1

    sphereCollider sphere1;
    sphereCollider sphere2;
    sphereCollider sphere3;
    sphereCollider sphere4;

    getObjectSphereCollider(&sphere1, 2.0, 2.0, 1.0);
    getObjectSphereCollider(&sphere2, 3.0, 5.0, 1.0);
    getObjectSphereCollider(&sphere3, 4.0, 3.0, 2.0);
    getObjectSphereCollider(&sphere4, 5.0, 5.0, 1.0);

    for(int i = 0; i < 100; i++){
        printf("%d ", SphereColliderCmp(sphere1, sphere2));
        printf("%d ", SphereColliderCmp(sphere1, sphere3));
        printf("%d ", SphereColliderCmp(sphere1, sphere4));
        printf("%d ", SphereColliderCmp(sphere2, sphere3));
        printf("%d ", SphereColliderCmp(sphere2, sphere4));
        printf("%d ", SphereColliderCmp(sphere3, sphere4));
      /*
    printf("Colidindo 1 2: %d\n", SphereColliderCmp(sphere1, sphere2));
    printf("Colidindo 1 3: %d\n", SphereColliderCmp(sphere1, sphere3));
    printf("Colidindo 1 4: %d\n", SphereColliderCmp(sphere1, sphere4));
    printf("Colidindo 2 3: %d\n", SphereColliderCmp(sphere2, sphere3));
    printf("Colidindo 2 4: %d\n", SphereColliderCmp(sphere2, sphere4));
    printf("Colidindo 3 4: %d\n", SphereColliderCmp(sphere3, sphere4));
     */
    }

    x = rdtsc();

    //andar para frente ou para tras
    if (keystates['w']) {   //-9 < z|x < 9
        posz -= velocity * cos(pi*angle/180);   //cos() e sin() usam radianos, então deve-se multiplicar o
        posx -= velocity * sin(pi*angle/180);   //angulo por pi e dividir por 180 para ter o valor certo
        if(velocity < maxSpeed){
            velocity += acceleration;
        }
    }
    if (keystates['s']) {
        posz += velocity * cos(pi*angle/180);
        posx += velocity * sin(pi*angle/180);
        if(velocity < maxSpeed){
            velocity += acceleration;
        }
    }

    if(velocity > 0 && !keystates['w'] && !keystates['s']){
        posz -= velocity * cos(pi*angle/180);   //cos() e sin() usam radianos, então deve-se multiplicar o
        posx -= velocity * sin(pi*angle/180);   //angulo por pi e dividir por 180 para ter o valor certo
        velocity -= acceleration * 10;
    }

    if(velocity < 0){
        velocity = 0.000001;
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
    if (keystates['a'])
        angle += 5.0f;
    if (keystates['d'])
        angle -= 5.0f;

    if (angle == 360)
        angle = 0;
    if (angle == -10)
        angle = 350;

    printf("\n FPS: %.2f\n Angle: %.0f\n X,Y,Z = (%.1f, %.1f, %.1f)\n Velocity = %.2f\n", fps, angle, posx, 0.0, posz, velocity);
    glutPostRedisplay();

    calculateFPS();
}

//draw method
void drawMesh(int vAttri, GLuint vBuffer,
              int tAttri, GLuint tBuffer,
              GLuint texture, GLfloat uniform, int vSize) {

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


void onDisplay() {

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

								glm::vec3(posx-3*cos(pi*(-90-angle)/180), 4, posz-3*sin(pi*(-90-angle)/180)),
								glm::vec3(posx+4*cos(pi*(-90-angle)/180), 1, posz+4*sin(pi*(-90-angle)/180)),

								glm::vec3(0,1,0)
						   );

    glm::mat4 Model      = glm::mat4(1.0f);

    glm::mat4 MVP;

    glm::mat4 transPista = glm::translate(mat4(1.0f), vec3(0, 0.0f, 0));

	MVP        = Projection * View * Model * transPista;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer, 1, uvBuffer, textureID1, 0, vertices.size());

    /*

    //parede 1
    glm::mat4 bricktrans = translate(mat4(1.0f), vec3(0, 0, 10)) * rotate(mat4(1.0f), 90.0f, vec3(1,0,0));
    MVP        = Projection * View * Model * bricktrans;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer, 1, uvBuffer, textureID3, 2, vertices.size());

    //parede 2
    glm::mat4 bricktrans2 = translate(mat4(1.0f), vec3(0, 0, -10)) * rotate(mat4(1.0f), 90.0f, vec3(1,0,0));
    MVP        = Projection * View * Model * bricktrans2;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer, 1, uvBuffer, textureID3, 2, vertices.size());

    //parede 3
    glm::mat4 bricktrans3 = translate(mat4(1.0f), vec3(10, 0, 0)) * rotate(mat4(1.0f), 90.0f, vec3(1,0,0)) * rotate(mat4(1.0f), 90.0f, vec3(0,0,1));
    MVP        = Projection * View * Model * bricktrans3;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer, 1, uvBuffer, textureID3, 2, vertices.size());

    //parede 4
    glm::mat4 bricktrans4 = translate(mat4(1.0f), vec3(-10, 0, 0)) * rotate(mat4(1.0f), 90.0f, vec3(1,0,0)) * rotate(mat4(1.0f), 90.0f, vec3(0,0,1));
    MVP        = Projection * View * Model * bricktrans4;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer, 1, uvBuffer, textureID3, 2, vertices.size());
    */

    //mão
    glm::mat4 escMao = glm::scale(mat4(1.0f), vec3(0.3f, 0.3f, 0.3f));
    glm::mat4 transMao = glm::translate(mat4(1.0f), vec3(posx, 1.5f, posz));
    glm::mat4 fixRot = glm::rotate(mat4(1.0f), 90.0f, vec3(0, 1.0f, 0));
    glm::mat4 rotMao = glm::rotate(mat4(1.0f), angle, vec3(0, 1.0f, 0));
    MVP        = Projection * View * Model * transMao * escMao * rotMao * fixRot;
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    drawMesh(0, vertexBuffer2, 1, uvBuffer2, textureID2, 1, handVertices.size());

    //disable
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

	glutSwapBuffers();
    glutPostRedisplay();

}


//libera recursos
void free_resources() {

    glDeleteBuffers(1, &vertexBuffer);
	glDeleteVertexArrays(1, &vertexID);
    glDeleteProgram(programID);

}


int main(int argc, char* argv[]) {

    //inicia janela
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("FPS Game");

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
        return 1;
    }

    if (init_resources() != 0) {

        glutDisplayFunc(onDisplay);

        glutIgnoreKeyRepeat(1);
        glutKeyboardFunc(keyboardDown);
        glutKeyboardUpFunc(keyboardUp);
        glutIdleFunc(idle);

        glutMainLoop();
    }

    free_resources();
    return 0;
}


