 // Include standard headers
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <sstream>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

typedef struct Vertex {
	float XYZW[4];
	float RGBA[4];
	void SetCoords(float *coords) {
		XYZW[0] = coords[0];
		XYZW[1] = coords[1];
		XYZW[2] = coords[2];
		XYZW[3] = coords[3];
	}
	void SetColor(float *color) {
		RGBA[0] = color[0];
		RGBA[1] = color[1];
		RGBA[2] = color[2];
		RGBA[3] = color[3];
	}
};

// ATTN: USE POINT STRUCTS FOR EASIER COMPUTATIONS
typedef struct point {
	float x, y, z;
	point(const float x = 0, const float y = 0, const float z = 0) : x(x), y(y), z(z){};
	point(float *coords) : x(coords[0]), y(coords[1]), z(coords[2]){};
	point operator -(const point& a)const {
		return point(x - a.x, y - a.y, z - a.z);
	}
	point operator +(const point& a)const {
		return point(x + a.x, y + a.y, z + a.z);
	}
	point operator *(const float& a)const {
		return point(x*a, y*a, z*a);
	}
	point operator /(const float& a)const {
		return point(x / a, y / a, z / a);
	}
	float* toArray() {
		float array[] = { x, y, z, 1.0f };
		return array;
	}
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void createVAOs(Vertex[], unsigned short[], size_t, size_t, int);
void createObjects(void);
void pickVertex(void);
void moveVertex(void);
void drawScene(void);
void cleanup(void);
void createControlObj(Vertex[],int);
void createBezier(Vertex[],int);
static void mouseCallback(GLFWwindow*, int, int, int);

// GLOBAL VARIABLES
GLFWwindow* window;
const GLuint window_width = 1024, window_height = 768;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex;
std::string gMessage;

int gFlag=0;
int pickingFlag=0;
int k=0,b=0;

GLuint programID;
GLuint pickingProgramID;

// ATTN: INCREASE THIS NUMBER AS YOU CREATE NEW OBJECTS
const GLuint NumObjects = 15;	// number of different "objects" to be drawn
GLuint VertexArrayId[NumObjects] = { 0 , 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
GLuint VertexBufferId[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
GLuint IndexBufferId[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
size_t NumVert[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };

GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorArrayID;
GLuint pickingColorID;
GLuint LightID;

int currIndex;
// Define objects
Vertex Vertices[] =
{
    { { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // 0
    { { 0.7071f, 0.7071f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // 1
    { { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // 2
    { { 0.7071f, -0.7071f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // 3
    { { 0.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // 4
    { { -0.7071f, -0.7071f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // 5
    { { -1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // 6
    { { -0.7071f, 0.7071f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // 7
};

unsigned short Indices[] = {
    0, 1, 2, 3, 4, 5, 6, 7
};

Vertex VerticesLine[] =
{
    { { 0.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 0
    { { 0.7071f, 0.7071f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 1
    { { 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 2
    { { 0.7071f, -0.7071f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 3
    { { 0.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 4
    { { -0.7071f, -0.7071f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 5
    { { -1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 6
    { { -0.7071f, 0.7071f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 7
    { { 0.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, // 8
};

unsigned short IndicesLine[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8
};

Vertex VerticesSubA[17];
unsigned short IndicesSubA[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
};

Vertex VerticesSubB[33];
unsigned short IndicesSubB[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32
};

Vertex VerticesSubC[65];
unsigned short IndicesSubC[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64
};

Vertex VerticesSubD[129];
unsigned short IndicesSubD[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128
};

Vertex Bez[161];
unsigned short IndicesBez[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160
};

Vertex c0[5],c1[5],c2[5],c3[5],c4[5],c5[5],c6[5],c7[5];
unsigned short Indices0[]={0,1,2,3,4};
unsigned short Indices1[]={0,1,2,3,4};
unsigned short Indices2[]={0,1,2,3,4};
unsigned short Indices3[]={0,1,2,3,4};
unsigned short Indices4[]={0,1,2,3,4};
unsigned short Indices5[]={0,1,2,3,4};
unsigned short Indices6[]={0,1,2,3,4};
unsigned short Indices7[]={0,1,2,3,4};


const size_t IndexCount = sizeof(Indices) / sizeof(unsigned short);
// ATTN: DON'T FORGET TO INCREASE THE ARRAY SIZE IN THE PICKING VERTEX SHADER WHEN YOU ADD MORE PICKING COLORS
float pickingColor[IndexCount] = { 0 / 255.0f, 1 / 255.0f, 2 / 255.0f, 3 / 255.0f, 4 / 255.0f, 5 / 255.0f, 6 / 255.0f, 7 / 255.0f};

// ATTN: ADD YOU PER-OBJECT GLOBAL ARRAY DEFINITIONS HERE

void createObjects(void)
{
	// ATTN: DERIVE YOUR NEW OBJECTS HERE:
	// each has one vertices {pos;color} and one indices array (no picking needed here)
    int j=0;
    int i=0;
    do{
    
        int p,q,r;
        if(i==0)
        {
            p=6;
            q=7;
            r=0;
        }
        else if (i==1)
        {
            p=7;
            q=0;
            r=1;
        }
        else
        {
            p=i-2;
            q=i-1;
            r=i;
        }
        
        //for fiding k
        VerticesSubA[j].XYZW[0]=(Vertices[p].XYZW[0] + 10*Vertices[q].XYZW[0] + 5*Vertices[r].XYZW[0])/16;
        VerticesSubA[j].XYZW[1]=(Vertices[p].XYZW[1] + 10*Vertices[q].XYZW[1] + 5*Vertices[r].XYZW[1])/16;
        VerticesSubA[j].XYZW[2]=0.0;
        VerticesSubA[j].XYZW[3]=1.0;
        VerticesSubA[j].RGBA[0]=0.0;
        VerticesSubA[j].RGBA[1]=0.0;
        VerticesSubA[j].RGBA[2]=1.0;
        VerticesSubA[j].RGBA[3]=1.0;
        
        j++;
        //for finding k+1 
        VerticesSubA[j].XYZW[0]=(5*Vertices[q].XYZW[0] + 10*Vertices[r].XYZW[0] + Vertices[(r+1)%8].XYZW[0])/16;
        VerticesSubA[j].XYZW[1]=(5*Vertices[q].XYZW[1] + 10*Vertices[r].XYZW[1] + Vertices[(r+1)%8].XYZW[1])/16;
        VerticesSubA[j].XYZW[2]=0.0;
        VerticesSubA[j].XYZW[3]=1.0;
        VerticesSubA[j].RGBA[0]=0.0;
        VerticesSubA[j].RGBA[1]=0.0;
        VerticesSubA[j].RGBA[2]=1.0;
        VerticesSubA[j].RGBA[3]=1.0;
        
        j++;
        
        i++;
    }while(i!=8);
    
    VerticesSubA[j]=VerticesSubA[0];
    
    i=0;
    j=0;
    do{
        
        int p,q,r;
        if(i==0)
        {
            p=14;
            q=15;
            r=0;
        }
        else if (i==1)
        {
            p=15;
            q=0;
            r=1;
        }
        else
        {
            p=i-2;
            q=i-1;
            r=i;
        }
        
        
        VerticesSubB[j].XYZW[0]=(VerticesSubA[p].XYZW[0] + 10*VerticesSubA[q].XYZW[0] + 5*VerticesSubA[r].XYZW[0])/16;
        VerticesSubB[j].XYZW[1]=(VerticesSubA[p].XYZW[1] + 10*VerticesSubA[q].XYZW[1] + 5*VerticesSubA[r].XYZW[1])/16;
        VerticesSubB[j].XYZW[2]=0.0;
        VerticesSubB[j].XYZW[3]=1.0;
        VerticesSubB[j].RGBA[0]=0.0;
        VerticesSubB[j].RGBA[1]=0.0;
        VerticesSubB[j].RGBA[2]=1.0;
        VerticesSubB[j].RGBA[3]=1.0;
        
        j++;
        
        VerticesSubB[j].XYZW[0]=(5*VerticesSubA[q].XYZW[0] + 10*VerticesSubA[r].XYZW[0] + VerticesSubA[(r+1)%16].XYZW[0])/16;
        VerticesSubB[j].XYZW[1]=(5*VerticesSubA[q].XYZW[1] + 10*VerticesSubA[r].XYZW[1] + VerticesSubA[(r+1)%16].XYZW[1])/16;
        VerticesSubB[j].XYZW[2]=0.0;
        VerticesSubB[j].XYZW[3]=1.0;
        VerticesSubB[j].RGBA[0]=0.0;
        VerticesSubB[j].RGBA[1]=0.0;
        VerticesSubB[j].RGBA[2]=1.0;
        VerticesSubB[j].RGBA[3]=1.0;
        
        j++;
        
        i++;
    }while(i!=16);
    
    VerticesSubB[j]=VerticesSubB[0];
    
    i=0;
    j=0;
    do{
        
        int p,q,r;
        if(i==0)
        {
            p=30;
            q=31;
            r=0;
        }
        else if (i==1)
        {
            p=31;
            q=0;
            r=1;
        }
        else
        {
            p=i-2;
            q=i-1;
            r=i;
        }
        
        
        VerticesSubC[j].XYZW[0]=(VerticesSubB[p].XYZW[0] + 10*VerticesSubB[q].XYZW[0] + 5*VerticesSubB[r].XYZW[0])/16;
        VerticesSubC[j].XYZW[1]=(VerticesSubB[p].XYZW[1] + 10*VerticesSubB[q].XYZW[1] + 5*VerticesSubB[r].XYZW[1])/16;
        VerticesSubC[j].XYZW[2]=0.0;
        VerticesSubC[j].XYZW[3]=1.0;
        VerticesSubC[j].RGBA[0]=0.0;
        VerticesSubC[j].RGBA[1]=0.0;
        VerticesSubC[j].RGBA[2]=1.0;
        VerticesSubC[j].RGBA[3]=1.0;
        
        j++;
        
        VerticesSubC[j].XYZW[0]=(5*VerticesSubB[q].XYZW[0] + 10*VerticesSubB[r].XYZW[0] + VerticesSubB[(r+1)%32].XYZW[0])/16;
        VerticesSubC[j].XYZW[1]=(5*VerticesSubB[q].XYZW[1] + 10*VerticesSubB[r].XYZW[1] + VerticesSubB[(r+1)%32].XYZW[1])/16;
        VerticesSubC[j].XYZW[2]=0.0;
        VerticesSubC[j].XYZW[3]=1.0;
        VerticesSubC[j].RGBA[0]=0.0;
        VerticesSubC[j].RGBA[1]=0.0;
        VerticesSubC[j].RGBA[2]=1.0;
        VerticesSubC[j].RGBA[3]=1.0;
        
        j++;
        
        i++;
    }while(i!=32);
    
    VerticesSubC[j]=VerticesSubC[0];
    
    i=0;
    j=0;
    do{
        
        int p,q,r;
        if(i==0)
        {
            p=62;
            q=63;
            r=0;
        }
        else if (i==1)
        {
            p=63;
            q=0;
            r=1;
        }
        else
        {
            p=i-2;
            q=i-1;
            r=i;
        }
        
        
        VerticesSubD[j].XYZW[0]=(VerticesSubC[p].XYZW[0] + 10*VerticesSubC[q].XYZW[0] + 5*VerticesSubC[r].XYZW[0])/16;
        VerticesSubD[j].XYZW[1]=(VerticesSubC[p].XYZW[1] + 10*VerticesSubC[q].XYZW[1] + 5*VerticesSubC[r].XYZW[1])/16;
        VerticesSubD[j].XYZW[2]=0.0;
        VerticesSubD[j].XYZW[3]=1.0;
        VerticesSubD[j].RGBA[0]=0.0;
        VerticesSubD[j].RGBA[1]=0.0;
        VerticesSubD[j].RGBA[2]=1.0;
        VerticesSubD[j].RGBA[3]=1.0;
        
        j++;
        
        VerticesSubD[j].XYZW[0]=(5*VerticesSubC[q].XYZW[0] + 10*VerticesSubC[r].XYZW[0] + VerticesSubC[(r+1)%64].XYZW[0])/16;
        VerticesSubD[j].XYZW[1]=(5*VerticesSubC[q].XYZW[1] + 10*VerticesSubC[r].XYZW[1] + VerticesSubC[(r+1)%64].XYZW[1])/16;
        VerticesSubD[j].XYZW[2]=0.0;
        VerticesSubD[j].XYZW[3]=1.0;
        VerticesSubD[j].RGBA[0]=0.0;
        VerticesSubD[j].RGBA[1]=0.0;
        VerticesSubD[j].RGBA[2]=1.0;
        VerticesSubD[j].RGBA[3]=1.0;
        
        j++;
        
        i++;
    }while(i!=64);
    
    VerticesSubD[j]=VerticesSubD[0];
    
    createControlObj(c0, 0);
    createControlObj(c1, 1);
    createControlObj(c2, 2);
    createControlObj(c3, 3);
    createControlObj(c4, 4);
    createControlObj(c5, 5);
    createControlObj(c6, 6);
    createControlObj(c7, 7);
    
    createBezier(c0, 0);
    createBezier(c1, 1);
    createBezier(c2, 2);
    createBezier(c3, 3);
    createBezier(c4, 4);
    createBezier(c5, 5);
    createBezier(c6, 6);
    createBezier(c7, 7);
}

void createControlObj(Vertex v[5],int count)
{
    int p,q,r;
    if(count==0)
    {
        p=6;
        q=7;
        r=0;
    }
    else if (count==1)
    {
        p=7;
        q=0;
        r=1;
    }
    else
    {
        p=count-2;
        q=count-1;
        r=count;
    }
    v[0].XYZW[0]=(Vertices[p].XYZW[0] + Vertices[q].XYZW[0]*11.0f + Vertices[r].XYZW[0]*11.0f + Vertices[(r+1)%8].XYZW[0]) / 24.0f;
    v[0].XYZW[1]=(Vertices[p].XYZW[1] + Vertices[q].XYZW[1]*11.0f + Vertices[r].XYZW[1]*11.0f + Vertices[(r+1)%8].XYZW[1]) / 24.0f;
    v[0].XYZW[2]=0.0;
    v[0].XYZW[3]=1.0;
    v[0].RGBA[0]=1.0;
    v[0].RGBA[1]=0.0;
    v[0].RGBA[2]=0.0;
    v[0].RGBA[3]=1.0;
    
    v[1].XYZW[0]=(4.0f*(Vertices[q].XYZW[0]) + 7.0f*(Vertices[r].XYZW[0]) + (Vertices[(r+1)%8].XYZW[0]))/ 12.0f;
    v[1].XYZW[1]=(4.0f*(Vertices[q].XYZW[1]) + 7.0f*(Vertices[r].XYZW[1]) + (Vertices[(r+1)%8].XYZW[1]))/ 12.0f;
    v[1].XYZW[2]=0.0;
    v[1].XYZW[3]=1.0;
    v[1].RGBA[0]=1.0;
    v[1].RGBA[1]=0.0;
    v[1].RGBA[2]=0.0;
    v[1].RGBA[3]=1.0;
    
    v[2].XYZW[0]=(4.0*(Vertices[q].XYZW[0]) + 16.0*(Vertices[r].XYZW[0]) + 4.0*(Vertices[(r+1)%8].XYZW[0]))/24.0;
    v[2].XYZW[1]=(4.0*(Vertices[q].XYZW[1]) + 16.0*(Vertices[r].XYZW[1]) + 4.0*(Vertices[(r+1)%8].XYZW[1]))/24.0;
    v[2].XYZW[2]=0.0;
    v[2].XYZW[3]=1.0;
    v[2].RGBA[0]=1.0;
    v[2].RGBA[1]=0.0;
    v[2].RGBA[2]=0.0;
    v[2].RGBA[3]=1.0;
    
    v[3].XYZW[0]=((Vertices[(r+1)%8].XYZW[0])*4.0f + (Vertices[r].XYZW[0])*7.0f + (Vertices[q].XYZW[0]))/ 12.0f;
    v[3].XYZW[1]=((Vertices[(r+1)%8].XYZW[1])*4.0f + (Vertices[r].XYZW[1])*7.0f + (Vertices[q].XYZW[1]))/ 12.0f;
    v[3].XYZW[2]=0.0;
    v[3].XYZW[3]=1.0;
    v[3].RGBA[0]=1.0;
    v[3].RGBA[1]=0.0;
    v[3].RGBA[2]=0.0;
    v[3].RGBA[3]=1.0;
    
    v[4].XYZW[0]=(Vertices[q].XYZW[0] + Vertices[r].XYZW[0]*11.0f + Vertices[(r+1)%8].XYZW[0]*11.0f + Vertices[(r+2)%8].XYZW[0]) / 24.0f;
    v[4].XYZW[1]=(Vertices[q].XYZW[1] + Vertices[r].XYZW[1]*11.0f + Vertices[(r+1)%8].XYZW[1]*11.0f + Vertices[(r+2)%8].XYZW[1]) / 24.0f;
    v[4].XYZW[2]=0.0;
    v[4].XYZW[3]=1.0;
    v[4].RGBA[0]=1.0;
    v[4].RGBA[1]=0.0;
    v[4].RGBA[2]=0.0;
    v[4].RGBA[3]=1.0;
    
    
}

void createBezier(Vertex v[5],int num)
{
    int flagB=0;
    int index=num * 20;
    float t=0.05;
    do
    {
        
        Bez[index].XYZW[0]= pow((1-t), 4) * v[0].XYZW[0] + 4 * pow((1-t), 3) * t * v[1].XYZW[0] + 6 * pow((1-t), 2) * pow(t, 2) * v[2].XYZW[0] + 4 * pow(t, 3) * (1-t) * v[3].XYZW[0] + pow((t), 4) * v[4].XYZW[0];
        
        Bez[index].XYZW[1]= pow((1-t), 4) * v[0].XYZW[1] + 4 * pow((1-t), 3) * t * v[1].XYZW[1] + 6 * pow((1-t), 2) * pow(t, 2) * v[2].XYZW[1] + 4 * pow(t, 3) * (1-t) * v[3].XYZW[1] + pow((t), 4) * v[4].XYZW[1];
        Bez[index].XYZW[2]=0.0;
        Bez[index].XYZW[3]=1.0;
        Bez[index].RGBA[0]=1.0;
        Bez[index].RGBA[1]=1.0;
        Bez[index].RGBA[2]=0.0;
        Bez[index].RGBA[3]=1.0;
        
        index++;
        t=t + 0.05;
        if(index==num *20 + 19)
        {
            flagB=1;
        }
        
    }while(flagB==0);
}

void drawScene(void)
{
    // Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		
		glEnable(GL_PROGRAM_POINT_SIZE);

		glBindVertexArray(VertexArrayId[0]);	// draw Vertices
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);				// update buffer data
			//glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
			glDrawElements(GL_POINTS, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
		// ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
		//glBindVertexArray(VertexArrayId[<x>]); etc etc
		glBindVertexArray(0);
        
        
        
        glBindVertexArray(VertexArrayId[1]);	// draw Vertices
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VerticesLine), VerticesLine);				// update buffer data
        //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
        glDrawElements(GL_LINE_STRIP, NumVert[1], GL_UNSIGNED_SHORT, (void*)0);
        // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
        //glBindVertexArray(VertexArrayId[<x>]); etc etc
        glBindVertexArray(1);
        
        if(k==1)
        {
        glBindVertexArray(VertexArrayId[2]);	// draw Vertices
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VerticesSubA), VerticesSubA);				// update buffer data
        //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
        glDrawElements(GL_LINE_STRIP, NumVert[2], GL_UNSIGNED_SHORT, (void*)0);
        // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
        //glBindVertexArray(VertexArrayId[<x>]); etc etc
        glBindVertexArray(2);
        }

        if(k==2)
        {
            glBindVertexArray(VertexArrayId[3]);	// draw Vertices
            glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VerticesSubB), VerticesSubB);				// update buffer data
            //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
            glDrawElements(GL_LINE_STRIP, NumVert[3], GL_UNSIGNED_SHORT, (void*)0);
            // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
            //glBindVertexArray(VertexArrayId[<x>]); etc etc
            glBindVertexArray(3);
        }
        
        if(k==3)
        {
            glBindVertexArray(VertexArrayId[4]);	// draw Vertices
            glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[4]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VerticesSubC), VerticesSubC);				// update buffer data
            //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
            glDrawElements(GL_LINE_STRIP, NumVert[4], GL_UNSIGNED_SHORT, (void*)0);
            // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
            //glBindVertexArray(VertexArrayId[<x>]); etc etc
            glBindVertexArray(4);
        }
        
        if(k==4)
        {
            glBindVertexArray(VertexArrayId[5]);	// draw Vertices
            glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[5]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VerticesSubD), VerticesSubD);				// update buffer data
            //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
            glDrawElements(GL_LINE_STRIP, NumVert[5], GL_UNSIGNED_SHORT, (void*)0);
            // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
            //glBindVertexArray(VertexArrayId[<x>]); etc etc
            glBindVertexArray(5);
        }
        
        
        if(b==1)
        {
        glBindVertexArray(VertexArrayId[6]);	// draw Vertices
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[6]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(c0), c0);				// update buffer data
        //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
        glDrawElements(GL_POINTS, NumVert[6], GL_UNSIGNED_SHORT, (void*)0);
        // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
        //glBindVertexArray(VertexArrayId[<x>]); etc etc
        glBindVertexArray(6);
        
        glBindVertexArray(VertexArrayId[7]);	// draw Vertices
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[7]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(c1), c1);				// update buffer data
        //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
        glDrawElements(GL_POINTS, NumVert[7], GL_UNSIGNED_SHORT, (void*)0);
        // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
        //glBindVertexArray(VertexArrayId[<x>]); etc etc
        glBindVertexArray(7);
        
        glBindVertexArray(VertexArrayId[8]);	// draw Vertices
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[8]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(c2), c2);				// update buffer data
        //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
        glDrawElements(GL_POINTS, NumVert[8], GL_UNSIGNED_SHORT, (void*)0);
        // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
        //glBindVertexArray(VertexArrayId[<x>]); etc etc
        glBindVertexArray(8);
        
        glBindVertexArray(VertexArrayId[9]);	// draw Vertices
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[9]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(c3), c3);				// update buffer data
        //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
        glDrawElements(GL_POINTS, NumVert[9], GL_UNSIGNED_SHORT, (void*)0);
        // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
        //glBindVertexArray(VertexArrayId[<x>]); etc etc
        glBindVertexArray(9);
        
        glBindVertexArray(VertexArrayId[10]);	// draw Vertices
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[10]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(c4), c4);				// update buffer data
        //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
        glDrawElements(GL_POINTS, NumVert[10], GL_UNSIGNED_SHORT, (void*)0);
        // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
        //glBindVertexArray(VertexArrayId[<x>]); etc etc
        glBindVertexArray(10);
        
        glBindVertexArray(VertexArrayId[11]);	// draw Vertices
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[11]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(c5), c5);				// update buffer data
        //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
        glDrawElements(GL_POINTS, NumVert[11], GL_UNSIGNED_SHORT, (void*)0);
        // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
        //glBindVertexArray(VertexArrayId[<x>]); etc etc
        glBindVertexArray(11);
        
        glBindVertexArray(VertexArrayId[12]);	// draw Vertices
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[12]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(c6), c6);				// update buffer data
        //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
        glDrawElements(GL_POINTS, NumVert[12], GL_UNSIGNED_SHORT, (void*)0);
        // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
        //glBindVertexArray(VertexArrayId[<x>]); etc etc
        glBindVertexArray(12);
        
        glBindVertexArray(VertexArrayId[13]);	// draw Vertices
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[13]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(c7), c7);				// update buffer data
        //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
        glDrawElements(GL_POINTS, NumVert[13], GL_UNSIGNED_SHORT, (void*)0);
        // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
        //glBindVertexArray(VertexArrayId[<x>]); etc etc
        glBindVertexArray(13);
        
        
        
        glBindVertexArray(VertexArrayId[14]);	// draw Vertices
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[14]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Bez), Bez);				// update buffer data
        //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
        glDrawElements(GL_LINE_STRIP, NumVert[14], GL_UNSIGNED_SHORT, (void*)0);
        // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
        //glBindVertexArray(VertexArrayId[<x>]); etc etc
        glBindVertexArray(14);
        
        }

    }
    
    
    glUseProgram(0);
	// Draw GUI
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void pickVertex(void)
{
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pickingProgramID);
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1fv(pickingColorArrayID, NumVert[0], pickingColor);	// here we pass in the picking marker array

		// Draw the ponts
		glEnable(GL_PROGRAM_POINT_SIZE);
		glBindVertexArray(VertexArrayId[0]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);	// update buffer data
		glDrawElements(GL_POINTS, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
		glBindVertexArray(0);
	}
	glUseProgram(0);
	// Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow ! 
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel, 
	// because the framebuffer is on the GPU.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	// Convert the color back to an integer ID
	gPickedIndex = int(data[0]);

	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
}

// fill this function in!
void moveVertex(void)
{
	glm::mat4 ModelMatrix = glm::mat4(1.0);
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glm::vec4 vp = glm::vec4(viewport[0], viewport[1], viewport[2], viewport[3]);
	
	// retrieve your cursor position
	// get your world coordinates
	// move points
    
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);


    
    glm::vec3 abc=glm::unProject(glm::vec3(xpos,768.0-ypos,0.0), ModelMatrix, gProjectionMatrix, vp);
    float worldCoord[4]= { abc.x, abc.y, 0.0f, 1.0f };
    Vertex v={ { -abc.x, abc.y, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } };
    Vertex v1={ { -abc.x, abc.y, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
    
    
    if (gPickedIndex == 255){ // Full white, must be the background !
		gMessage = "background";
        if(pickingFlag==1)
        {
            Vertices[currIndex]=v;
            VerticesLine[currIndex]=v1;
            if(currIndex==0)
                VerticesLine[8]=v1;

        }

	}
	else {
		std::ostringstream oss;
		oss << "point " << gPickedIndex;
		gMessage = oss.str();
        currIndex=gPickedIndex;
        pickingFlag=1;
        Vertices[currIndex]=v;
        VerticesLine[currIndex]=v1;
        if(currIndex==0)
            VerticesLine[8]=v1;

        
	}
}

int initWindow(void)
{
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "Das,Abhinav(61539128)", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width, window_height);
	TwBar * GUI = TwNewBar("Picking");
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_FALSE);
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
}

void initOpenGL(void)
{
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	//glm::mat4 ProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	gViewMatrix = glm::lookAt(
		glm::vec3(0, 0, -5), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorArrayID = glGetUniformLocation(pickingProgramID, "PickingColorArray");
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	createVAOs(Vertices, Indices, sizeof(Vertices), sizeof(Indices), 0);
    createVAOs(VerticesLine, IndicesLine, sizeof(VerticesLine), sizeof(IndicesLine), 1);
	createObjects();
    
    createVAOs(VerticesSubA, IndicesSubA, sizeof(VerticesSubA), sizeof(IndicesSubA), 2);
    
    createVAOs(VerticesSubB, IndicesSubB, sizeof(VerticesSubB), sizeof(IndicesSubB), 3);

    createVAOs(VerticesSubC, IndicesSubC, sizeof(VerticesSubC), sizeof(IndicesSubC), 4);

    createVAOs(VerticesSubD, IndicesSubD, sizeof(VerticesSubD), sizeof(IndicesSubD), 5);
    
    createVAOs(c0, Indices0, sizeof(c0), sizeof(Indices0), 6);
    createVAOs(c1, Indices1, sizeof(c1), sizeof(Indices1), 7);
    createVAOs(c2, Indices2, sizeof(c2), sizeof(Indices2), 8);
    createVAOs(c3, Indices3, sizeof(c3), sizeof(Indices3), 9);
    createVAOs(c4, Indices4, sizeof(c4), sizeof(Indices4), 10);
    createVAOs(c5, Indices5, sizeof(c5), sizeof(Indices5), 11);
    createVAOs(c6, Indices6, sizeof(c6), sizeof(Indices6), 12);
    createVAOs(c7, Indices7, sizeof(c7), sizeof(Indices7), 13);
    
    createVAOs(Bez, IndicesBez, sizeof(Bez), sizeof(IndicesBez), 14);


	// ATTN: create VAOs for each of the newly created objects here:
	// createVAOs(<fill this appropriately>);

}

void createVAOs(Vertex Vertices[], unsigned short Indices[], size_t BufferSize, size_t IdxBufferSize, int ObjectId) {

	NumVert[ObjectId] = IdxBufferSize / (sizeof( GLubyte));

	GLenum ErrorCheckValue = glGetError();
	size_t VertexSize = sizeof(Vertices[0]);
	size_t RgbOffset = sizeof(Vertices[0].XYZW);

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);
	glBindVertexArray(VertexArrayId[ObjectId]);

	// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, BufferSize, Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	glGenBuffers(1, &IndexBufferId[ObjectId]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IdxBufferSize, Indices, GL_STATIC_DRAW);

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color

	// Disable our Vertex Buffer Object 
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
			);
	}
}

void cleanup(void)
{
	// Cleanup VBO and shader
	for (int i = 0; i < NumObjects; i++) {
		glDeleteBuffers(1, &VertexBufferId[i]);
		glDeleteBuffers(1, &IndexBufferId[i]);
		glDeleteVertexArrays(1, &VertexArrayId[i]);
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickVertex();
	}
}

int main(void)
{
	// initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;

	// initialize OpenGL pipeline
	initOpenGL();

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// DRAGGING: move current (picked) vertex with cursor
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
			moveVertex();
        else
            pickingFlag=0;

		// DRAWING SCENE
		createObjects();	// re-evaluate curves in case vertices have been moved
		drawScene();
        if(glfwGetKey(window, GLFW_KEY_1))
        {
            k=(k+1)%5;
            sleep(1);
        }
        if(glfwGetKey(window, GLFW_KEY_2))
        {
            b=(b+1)%2;
            sleep(1);
        }
        
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}