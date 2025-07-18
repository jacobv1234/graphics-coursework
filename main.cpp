#include <iostream>
using namespace std;


//--- OpenGL ---
#include "GL\glew.h"
#include "GL\wglew.h"
#pragma comment(lib, "glew32.lib")
//--------------

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"
#include "glm\gtc\matrix_inverse.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/vector_angle.hpp"

#include "GL\freeglut.h"

#include "Images\FreeImage.h"

#include "shaders\Shader.h"

CShader* myShader;  ///shader object 
CShader* myBasicShader;
CShader* myTextureShader;

//MODEL LOADING
#include "3DStruct\threeDModel.h"
#include "Obj\OBJLoader.h"

float amount = 0;
float temp = 0.002f;
	
CThreeDModel base, standA, standB, standC;
CThreeDModel cupA, cupB, cupC; //A threeDModel object is needed for each model loaded
CThreeDModel lamp, lampOn;
CThreeDModel skybox;

// static models
CThreeDModel static_models[6];
int static_size = 6;

COBJLoader objLoader;	//this object is used to load the 3d models.
///END MODEL LOADING

glm::mat4 ProjectionMatrix; // matrix for the orthographic projection
glm::mat4 ModelViewMatrix;  // matrix for the modelling and viewing

glm::mat4 objectRotation;
glm::vec3 translation = glm::vec3(0.0, 0.0, 0.0);
glm::vec3 pos = glm::vec3(0.0f,0.0f,0.0f); //vector for the position of the object.

//Material properties
float Material_Ambient[4] = {0.2f, 0.2f, 0.2f, 1.0f};
float Material_Diffuse[4] = {0.8f, 0.8f, 0.5f, 1.0f};
float Material_Specular[4] = {0.9f,0.9f,0.8f,1.0f};

//float Material_Ambient[4] = {1.0f, 1.0f, 1.0f, 1.0f};
//float Material_Diffuse[4] = {0.0f, 0.0f, 0.0f, 1.0f};
//float Material_Specular[4] = { 0.0f,0.0f,0.0f,1.0f };

float Material_Shininess = 50;

//Light Properties
float Light_Ambient_And_Diffuse[4] = {0.8f, 0.8f, 0.8f, 1.0f};
float Light_Specular[4] = {1.0f,1.0f,1.0f,1.0f};
float LightPos[4] = {0.0f, 1.0f, 1.0f, 0.0f};

// Lamppost Properties
float Lamp_Ambient[4] = { 0.1f,0.0f,0.0f,1.0f };
float Lamp_Diffuse[4] = { 0.8f, 0.1f, 0.1f, 1.0f };
float Lamp_Specular[4] = { 1.0f,0.3f,0.3f,1.0f };
float LampPos[4] = { 0.0f, 20.0f, 0.0f, 0.0f};

// Cup Light Properties
float Cup_Ambient[4] = { 0.2f,0.2f,0.2f,1.0f };
float Cup_Diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float Cup_Specular[4] = { 1.0f,1.0f,1.0f,1.0f };
glm::vec4 CupPos = glm::vec4(6.0f, 1.0f, 0.0f, 1.0f);
//float CupPosArray[4] = { 6.0f,1.0f,0.0f,1.0f };
float CupPosArray[4] = { 0.0f,1.0f,0.0f,1.0f };


int	mouse_x=0, mouse_y=0;
bool LeftPressed = false;
int screenWidth=600, screenHeight=600;

//booleans to handle when keys are pressed or released.
bool Left = false;
bool Right = false;
bool Up = false;
bool Down = false;
bool Home = false;
bool End = false;
bool Space = false;
bool W = false;
bool S = false;
bool A = false;
bool D = false;
bool R = false;
bool F = false;
bool I = false;
bool J = false;
bool K = false;
bool L = false;

float spin=180;
float speed=0;

// current state
float base_rotation = 0.0;
float base_rotation_speed = 0.0;
float cup_A_rotation = 0.0;
float cup_A_rotation_speed = 0.0;
float cup_B_rotation = 0.0;
float cup_B_rotation_speed = 0.0;
float cup_C_rotation = 0.0;
float cup_C_rotation_speed = 0.0;
float min_cup_speed = 0.0;
float max_cup_speed = 0.0;

int camera_view = 0;
/* CAMERA VIEWS:
* 0: Looking down at an angle
* 1: Ground level
* 2: Top down
* 3: Ride view
* 4: Freecam
*/
// Freecam variables
glm::vec3 camera_coords = glm::vec3(0.0f, 10.0f, -20.0f);
float camera_h_rotation = 0.0;
float camera_v_rotation = 0.0;
float camera_speed = 0.01;
glm::vec3 camera_input = glm::vec3(0.0f, 0.0f, 0.0f);


//OPENGL FUNCTION PROTOTYPES
void display();				//called in winmain to draw everything to the screen
void reshape(int width, int height);				//called when the window is resized
void init();				//called in winmain when the program starts.
void processKeys();         //called in winmain to process keyboard input
void idle();		//idle function
void updateTransform(float xinc, float yinc, float zinc);

int isValidCoord(float x, float y, float z);

/*************    START OF OPENGL FUNCTIONS   ****************/
void display()									
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glUseProgram(myShader->GetProgramObjID());  // use the shader

	//Part for displacement shader.
	amount += temp;
	if(amount > 1.0f || amount < -1.5f)
		temp = -temp;
	//amount = 0;
	glUniform1f(glGetUniformLocation(myShader->GetProgramObjID(), "displacement"), amount);

	//Set the projection matrix in the shader
	GLuint projMatLocation = glGetUniformLocation(myShader->GetProgramObjID(), "ProjectionMatrix");  
	glUniformMatrix4fv(projMatLocation, 1, GL_FALSE, &ProjectionMatrix[0][0]);

	glm::mat4 viewingMatrix = glm::mat4(1.0f);
	
	//translation and rotation for view
	//viewingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0, 0, -50));

	//apply a rotation to the view
	//static float angle = 0.0f;
	//angle += 0.01;
	//viewingMatrix = glm::rotate(viewingMatrix, angle, glm::vec3(1.0f, 0.0f, 0.0));

	//use of glm::lookAt for viewing instead.
	switch (camera_view) {
	case 0:
		viewingMatrix = glm::lookAt(glm::vec3(0, 20, 20), glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0));
		break;
	case 1:
		viewingMatrix = glm::lookAt(glm::vec3(0, 5, 20), glm::vec3(0, 5, 0), glm::vec3(0.0f, 1.0f, 0.0));
		break;
	case 2:
		viewingMatrix = glm::lookAt(glm::vec3(0, 30, 0), glm::vec3(0, 0, 0), glm::vec3(0.0f, 0.0f, 1.0f));
		break;
	case 3:
		viewingMatrix = glm::rotate(viewingMatrix, glm::radians(cup_A_rotation), glm::vec3(0.0f, -1.0f, 0.0f));
		// enable to fix camera in one direction
		//viewingMatrix = glm::rotate(viewingMatrix, glm::radians(base_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		viewingMatrix = glm::translate(viewingMatrix, glm::vec3(-6.0f,-6.0f,0.0f));
		viewingMatrix = glm::rotate(viewingMatrix, glm::radians(base_rotation), glm::vec3(0.0f, -1.0f, 0.0f));
		break;
	case 4:
		// rotate input vector to match camera angle
		glm::vec3 rotated_input = glm::rotateY(camera_input, -camera_h_rotation);

		// multiply by movement speed
		rotated_input = rotated_input * camera_speed;

		// check new coords are valid
		glm::vec3 new_camera_pos = camera_coords + rotated_input;

		// add result to the camera's position if collision detection passes
		//cout << rotated_input[0] << ',' << rotated_input[1] << ',' << rotated_input[2] << endl;
		if (isValidCoord(new_camera_pos.x, new_camera_pos.y, new_camera_pos.z) == 1) {
			camera_coords = new_camera_pos;
		}
		if (pow(camera_coords.x, 2) + pow(camera_coords.z, 2) < 100 && camera_coords.y <= 8 && base_rotation_speed > 0.0f) { // move out of the way if in cup area when ride starts
			camera_coords.y = 8.1;
		}
		//cout << camera_coords[0] << ',' << camera_coords[1] << ',' << camera_coords[2] << endl;
		
		// below transformations rewritten as a lookAt
		glm::vec3 direction = glm::vec3(
			cos(camera_h_rotation - glm::half_pi<float>()) * cos(camera_v_rotation),
			sin(camera_v_rotation),
			sin(camera_h_rotation - glm::half_pi<float>()) * cos(camera_v_rotation)
		);
		viewingMatrix = glm::lookAt(camera_coords, camera_coords + direction, glm::vec3(0.0f, 1.0f, 0.0));

		//viewingMatrix = glm::rotate(viewingMatrix, camera_v_rotation, glm::vec3(1.0f, 0.0f, 0.0f));
		//viewingMatrix = glm::rotate(viewingMatrix, camera_h_rotation, glm::vec3(0.0f, 1.0f, 0.0f));
		//viewingMatrix = glm::translate(viewingMatrix, camera_coords);
		
		break;
	}

	//DEBUG
	//cout << to_string(viewingMatrix) << '\n' << endl;
	

	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ViewMatrix"), 1, GL_FALSE, &viewingMatrix[0][0]);

	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "LightPos"), 1, LightPos);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "light_ambient"), 1, Light_Ambient_And_Diffuse);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "light_diffuse"), 1, Light_Ambient_And_Diffuse);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "light_specular"), 1, Light_Specular);

	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "SpotlightCoord"), 1, LampPos);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "spotlight_ambient"), 1, Lamp_Ambient);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "spotlight_diffuse"), 1, Lamp_Diffuse);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "spotlight_specular"), 1, Lamp_Specular);


	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "CupPos"), 1, CupPosArray);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "cup_ambient"), 1, Cup_Ambient);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "cup_diffuse"), 1, Cup_Diffuse);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "cup_specular"), 1, Cup_Specular);

	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "material_ambient"), 1, Material_Ambient);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "material_diffuse"), 1, Material_Diffuse);
	glUniform4fv(glGetUniformLocation(myShader->GetProgramObjID(), "material_specular"), 1, Material_Specular);
	glUniform1f(glGetUniformLocation(myShader->GetProgramObjID(), "material_shininess"), Material_Shininess);

	//pos.x += objectRotation[2][0]*0.0003;
	//pos.y += objectRotation[2][1]*0.0003;
	//pos.z += objectRotation[2][2]*0.0003;
	
	glm::mat4 modelmatrix = glm::translate(glm::mat4(1.0f), pos);
	ModelViewMatrix = viewingMatrix * modelmatrix * objectRotation;
	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);

	
	glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelMatrix"), 1, GL_FALSE, &modelmatrix[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);
	
	//cupA.DrawElementsUsingVBO(myShader);

	//Switch to basic shader to draw the lines for the bounding boxes
	//glUseProgram(myBasicShader->GetProgramObjID());
	//projMatLocation = glGetUniformLocation(myBasicShader->GetProgramObjID(), "ProjectionMatrix");
	//glUniformMatrix4fv(projMatLocation, 1, GL_FALSE, &ProjectionMatrix[0][0]);
	//glUniformMatrix4fv(glGetUniformLocation(myBasicShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);

	//model.DrawAllBoxesForOctreeNodes(myBasicShader);
	//model.DrawBoundingBox(myBasicShader);
	//	model.CalcBoundingBox()
	//model.DrawOctreeLeaves(myBasicShader);

	//switch back to the shader for textures and lighting on the objects.
	glUseProgram(myShader->GetProgramObjID());  // use the shader

	ModelViewMatrix = glm::translate(viewingMatrix, glm::vec3(0, 0, 0));
	
	normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelMatrix"), 1, GL_FALSE, &modelmatrix[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);
	base.DrawElementsUsingVBO(myShader);

	// other static objects
	modelmatrix = glm::translate(glm::mat4(1), pos);
	ModelViewMatrix = viewingMatrix * modelmatrix;
	normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelMatrix"), 1, GL_FALSE, &modelmatrix[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);

	for (int i = 0; i < static_size; i++) {
		static_models[i].DrawElementsUsingVBO(myShader);
	}

	// skybox
	if (Light_Specular[0] == 1.0) {
		// switch to texture only shader
		// this shader ignores the lighting and only shows textures
		glUseProgram(myTextureShader->GetProgramObjID());
		projMatLocation = glGetUniformLocation(myTextureShader->GetProgramObjID(), "ProjectionMatrix");
		glUniformMatrix4fv(projMatLocation, 1, GL_FALSE, &ProjectionMatrix[0][0]);
		glUniformMatrix3fv(glGetUniformLocation(myTextureShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(myTextureShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(myTextureShader->GetProgramObjID(), "ViewMatrix"), 1, GL_FALSE, &viewingMatrix[0][0]);

		glUniform4fv(glGetUniformLocation(myTextureShader->GetProgramObjID(), "LightPos"), 1, LightPos);
		glUniform4fv(glGetUniformLocation(myTextureShader->GetProgramObjID(), "light_ambient"), 1, Light_Ambient_And_Diffuse);
		glUniform4fv(glGetUniformLocation(myTextureShader->GetProgramObjID(), "light_diffuse"), 1, Light_Ambient_And_Diffuse);
		glUniform4fv(glGetUniformLocation(myTextureShader->GetProgramObjID(), "light_specular"), 1, Light_Specular);

		glUniform4fv(glGetUniformLocation(myTextureShader->GetProgramObjID(), "material_ambient"), 1, Material_Ambient);
		glUniform4fv(glGetUniformLocation(myTextureShader->GetProgramObjID(), "material_diffuse"), 1, Material_Diffuse);
		glUniform4fv(glGetUniformLocation(myTextureShader->GetProgramObjID(), "material_specular"), 1, Material_Specular);
		glUniform1f(glGetUniformLocation(myTextureShader->GetProgramObjID(), "material_shininess"), Material_Shininess);

		skybox.DrawElementsUsingVBO(myTextureShader);

		//switch back to the shader for textures and lighting on the objects.
		glUseProgram(myShader->GetProgramObjID());  // use the shader
	}


	// temple octree
	// 
	// Switch to basic shader to draw the lines for the bounding boxes
	//glUseProgram(myBasicShader->GetProgramObjID());
	//projMatLocation = glGetUniformLocation(myBasicShader->GetProgramObjID(), "ProjectionMatrix");
	//glUniformMatrix4fv(projMatLocation, 1, GL_FALSE, &ProjectionMatrix[0][0]);
	//glUniformMatrix4fv(glGetUniformLocation(myBasicShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);

	//static_models[5].DrawAllBoxesForOctreeNodes(myBasicShader);
	
	//static_models[5].CalcBoundingBox();
	//static_models[5].DrawOctreeLeaves(myBasicShader);
	// 
	//switch back to the shader for textures and lighting on the objects.
	//glUseProgram(myShader->GetProgramObjID());  // use the shader



	// lamppost
	modelmatrix = glm::translate(glm::mat4(1), pos);
	modelmatrix = glm::rotate(modelmatrix, glm::radians(base_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
	modelmatrix = glm::scale(modelmatrix, glm::vec3(9.0f,7.0f,9.0f));
	modelmatrix = glm::translate(modelmatrix, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = viewingMatrix * modelmatrix;
	normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelMatrix"), 1, GL_FALSE, &modelmatrix[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);
	if (Lamp_Ambient[0] == 0.1f) {
		lampOn.DrawElementsUsingVBO(myShader);
	}
	else {
		lamp.DrawElementsUsingVBO(myShader);
	}

	// stands (Base sections)
	modelmatrix = glm::translate(glm::mat4(1), pos);
	modelmatrix = glm::rotate(modelmatrix, glm::radians(base_rotation), glm::vec3(0.0f,1.0f,0.0f));
	modelmatrix = glm::translate(modelmatrix, glm::vec3(6.0f, 0.0f, 0.0f));
	modelmatrix = glm::translate(modelmatrix, glm::vec3(0.0f, 0.2f, 0.0f));
	modelmatrix = glm::scale(modelmatrix, glm::vec3(0.3f,1.0f,0.3f));

	ModelViewMatrix = viewingMatrix * modelmatrix;
	normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelMatrix"), 1, GL_FALSE, &modelmatrix[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);
	standA.DrawElementsUsingVBO(myShader);


	modelmatrix = glm::translate(glm::mat4(1), pos);
	modelmatrix = glm::rotate(modelmatrix, glm::radians(base_rotation + 120.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	modelmatrix = glm::translate(modelmatrix, glm::vec3(6.0f, 0.0f, 0.0f));
	modelmatrix = glm::translate(modelmatrix, glm::vec3(0.0f, 0.2f, 0.0f));
	modelmatrix = glm::scale(modelmatrix, glm::vec3(0.3f, 1.0f, 0.3f));

	ModelViewMatrix = viewingMatrix * modelmatrix;
	normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelMatrix"), 1, GL_FALSE, &modelmatrix[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);
	standB.DrawElementsUsingVBO(myShader);


	modelmatrix = glm::translate(glm::mat4(1), pos);
	modelmatrix = glm::rotate(modelmatrix, glm::radians(base_rotation - 120.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	modelmatrix = glm::translate(modelmatrix, glm::vec3(6.0f, 0.0f, 0.0f));
	modelmatrix = glm::translate(modelmatrix, glm::vec3(0.0f, 0.2f, 0.0f));
	modelmatrix = glm::scale(modelmatrix, glm::vec3(0.3f, 1.0f, 0.3f));

	ModelViewMatrix = viewingMatrix * modelmatrix;
	normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelMatrix"), 1, GL_FALSE, &modelmatrix[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);
	standC.DrawElementsUsingVBO(myShader);


	// cups
	modelmatrix = glm::translate(glm::mat4(1), pos);
	modelmatrix = glm::rotate(modelmatrix, glm::radians(base_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
	modelmatrix = glm::translate(modelmatrix, glm::vec3(6.0f, 0.0f, 0.0f));
	modelmatrix = glm::translate(modelmatrix, glm::vec3(0.0f, 0.2f, 0.0f));
	modelmatrix = glm::scale(modelmatrix, glm::vec3(0.5f, 0.5f, 0.5f));
	modelmatrix = glm::rotate(modelmatrix, glm::radians(cup_A_rotation), glm::vec3(0.0f, 1.0f, 0.0f));

	ModelViewMatrix = viewingMatrix * modelmatrix;
	normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelMatrix"), 1, GL_FALSE, &modelmatrix[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);
	cupA.DrawElementsUsingVBO(myShader);


	modelmatrix = glm::translate(glm::mat4(1), pos);
	modelmatrix = glm::rotate(modelmatrix, glm::radians(base_rotation + 120.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	modelmatrix = glm::translate(modelmatrix, glm::vec3(6.0f, 0.0f, 0.0f));
	modelmatrix = glm::translate(modelmatrix, glm::vec3(0.0f, 0.2f, 0.0f));
	modelmatrix = glm::scale(modelmatrix, glm::vec3(0.5f, 0.5f, 0.5f));
	modelmatrix = glm::rotate(modelmatrix, glm::radians(cup_B_rotation), glm::vec3(0.0f, 1.0f, 0.0f));

	ModelViewMatrix = viewingMatrix * modelmatrix;
	normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelMatrix"), 1, GL_FALSE, &modelmatrix[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);
	cupB.DrawElementsUsingVBO(myShader);


	modelmatrix = glm::translate(glm::mat4(1), pos);
	modelmatrix = glm::rotate(modelmatrix, glm::radians(base_rotation - 120.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	modelmatrix = glm::translate(modelmatrix, glm::vec3(6.0f, 0.0f, 0.0f));
	modelmatrix = glm::translate(modelmatrix, glm::vec3(0.0f, 0.2f, 0.0f));
	modelmatrix = glm::scale(modelmatrix, glm::vec3(0.5f, 0.5f, 0.5f));
	modelmatrix = glm::rotate(modelmatrix, glm::radians(cup_C_rotation), glm::vec3(0.0f, 1.0f, 0.0f));

	ModelViewMatrix = viewingMatrix * modelmatrix;
	normalMatrix = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelMatrix"), 1, GL_FALSE, &modelmatrix[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(myShader->GetProgramObjID(), "NormalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(myShader->GetProgramObjID(), "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);
	cupC.DrawElementsUsingVBO(myShader);


	// cup position for lighting
	CupPos = glm::rotateY(glm::vec4(6.0f,-2.0f,0.0f,1.0f), glm::radians(base_rotation));
	CupPosArray[0] = CupPos.x;
	CupPosArray[1] = CupPos.y;
	CupPosArray[2] = CupPos.z;
	CupPosArray[3] = 1.0f;
	


	glFlush();
	glutSwapBuffers();


	// handle rotation over time
	base_rotation += base_rotation_speed;
	if (base_rotation >= 360) {
		base_rotation -= 360;
	}

	// random cup rotation
	if (rand() < RAND_MAX / 100) {
		if (rand() < RAND_MAX / 2) {
			cup_A_rotation_speed += max_cup_speed / 50;
		}
		else {
			cup_A_rotation_speed -= max_cup_speed / 50;
		}
	}
	if (cup_A_rotation_speed > max_cup_speed) {
		cup_A_rotation_speed = max_cup_speed;
	}
	if (cup_A_rotation_speed < min_cup_speed) {
		cup_A_rotation_speed = min_cup_speed;
	}

	if (rand() < RAND_MAX / 100) {
		if (rand() < RAND_MAX / 2) {
			cup_B_rotation_speed += max_cup_speed / 50;
		}
		else {
			cup_B_rotation_speed -= max_cup_speed / 50;
		}
	}
	if (cup_B_rotation_speed > max_cup_speed) {
		cup_B_rotation_speed = max_cup_speed;
	}
	if (cup_B_rotation_speed < min_cup_speed) {
		cup_B_rotation_speed = min_cup_speed;
	}

	if (rand() < RAND_MAX / 100) {
		if (rand() < RAND_MAX / 2) {
			cup_C_rotation_speed += max_cup_speed / 50;
		}
		else {
			cup_C_rotation_speed -= max_cup_speed / 50;
		}
	}
	if (cup_C_rotation_speed > max_cup_speed) {
		cup_C_rotation_speed = max_cup_speed;
	}
	if (cup_C_rotation_speed < min_cup_speed) {
		cup_C_rotation_speed = min_cup_speed;
	}



	cup_A_rotation += cup_A_rotation_speed;
	if (cup_A_rotation >= 360) {
		cup_A_rotation -= 360;
	}
	cup_B_rotation += cup_B_rotation_speed;
	if (cup_B_rotation >= 360) {
		cup_B_rotation -= 360;
	}
	cup_C_rotation += cup_C_rotation_speed;
	if (cup_C_rotation >= 360) {
		cup_C_rotation -= 360;
	}
}

void reshape(int width, int height)		// Resize the OpenGL window
{
	screenWidth=width; screenHeight = height;           // to ensure the mouse coordinates match 
														// we will use these values to set the coordinate system

	glViewport(0,0,width,height);						// Reset The Current Viewport

	//Set the projection matrix
	ProjectionMatrix = glm::perspective(glm::radians(60.0f), (GLfloat)screenWidth/(GLfloat)screenHeight, 1.0f, 400.0f);
}
void init()
{
	glClearColor(0.0,0.0,0.0,0.0);						//sets the clear colour to black
														//glClear(GL_COLOR_BUFFER_BIT) in the display function
														//will clear the buffer to this colour
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);


	myShader = new CShader();
	//if(!myShader->CreateShaderProgram("BasicView", "glslfiles/basicTransformationsWithDisplacement.vert", "glslfiles/basicTransformationsWithDisplacement.frag"))
	if(!myShader->CreateShaderProgram("BasicView", "glslfiles/basicTransformationsWithSpotlight.vert", "glslfiles/basicTransformationsWithSpotlight.frag"))
	{
		cout << "failed to load shader" << endl;
	}		

	myBasicShader = new CShader();
	if(!myBasicShader->CreateShaderProgram("Basic", "glslfiles/basic.vert", "glslfiles/basic.frag"))
	{
		cout << "failed to load shader" << endl;
	}

	myTextureShader = new CShader();
	if (!myTextureShader->CreateShaderProgram("TextureOnly", "glslfiles/basicTransformationsTextureOnly.vert", "glslfiles/basicTransformationsTextureOnly.frag"))
	{
		cout << "failed to load shader" << endl;
	}

	glUseProgram(myShader->GetProgramObjID());  // use the shader

	glEnable(GL_TEXTURE_2D);

	//lets initialise our object's rotation transformation 
	//to the identity matrix
	objectRotation = glm::mat4(1.0f);

	cout << " loading model " << endl;
	if(objLoader.LoadModel("TestModels/teacup.obj"))//returns true if the model is loaded
	{
		cout << " model loaded " << endl;		

		//copy data from the OBJLoader object to the threedmodel class
		cupA.ConstructModelFromOBJLoader(objLoader);

		//if you want to translate the object to the origin of the screen,
		//first calculate the centre of the object, then move all the vertices
		//back so that the centre is on the origin.
		//model.CalcCentrePoint();
		//model.CentreOnZero();

	
		cupA.InitVBO(myShader);
		cupB.ConstructModelFromOBJLoader(objLoader);
		cupB.InitVBO(myShader);
		cupC.ConstructModelFromOBJLoader(objLoader);
		cupC.InitVBO(myShader);
	}
	else
	{
		cout << " model failed to load " << endl;
	}
	

	
	if (objLoader.LoadModel("TestModels/base_disc.obj"))//returns true if the model is loaded
	{
		base.ConstructModelFromOBJLoader(objLoader);

		//Place to centre geometry before creating VBOs.

		base.InitVBO(myShader);
	}
	if (objLoader.LoadModel("TestModels/base_disc_red.obj"))//returns true if the model is loaded
	{
		standA.ConstructModelFromOBJLoader(objLoader);
		standA.InitVBO(myShader);
		standB.ConstructModelFromOBJLoader(objLoader);
		standB.InitVBO(myShader);
		standC.ConstructModelFromOBJLoader(objLoader);
		standC.InitVBO(myShader);
	}
	if (objLoader.LoadModel("TestModels/lamppost.obj"))//returns true if the model is loaded
	{
		lamp.ConstructModelFromOBJLoader(objLoader);
		lamp.InitVBO(myShader);
	}
	if (objLoader.LoadModel("TestModels/lamppostred.obj"))//returns true if the model is loaded
	{
		lampOn.ConstructModelFromOBJLoader(objLoader);
		lampOn.InitVBO(myShader);
	}
	if (objLoader.LoadModel("TestModels/floor.obj"))//returns true if the model is loaded
	{
		static_models[0].ConstructModelFromOBJLoader(objLoader);
		static_models[0].InitVBO(myShader);
	}
	if (objLoader.LoadModel("TestModels/building1.obj"))//returns true if the model is loaded
	{
		static_models[1].ConstructModelFromOBJLoader(objLoader);
		static_models[1].InitVBO(myShader);
	}
	if (objLoader.LoadModel("TestModels/building2.obj"))//returns true if the model is loaded
	{
		static_models[2].ConstructModelFromOBJLoader(objLoader);
		static_models[2].InitVBO(myShader);
	}
	if (objLoader.LoadModel("TestModels/building3a.obj"))//returns true if the model is loaded
	{
		static_models[3].ConstructModelFromOBJLoader(objLoader);
		static_models[3].InitVBO(myShader);
	}
	if (objLoader.LoadModel("TestModels/building3b.obj"))//returns true if the model is loaded
	{
		static_models[4].ConstructModelFromOBJLoader(objLoader);
		static_models[4].InitVBO(myShader);
	}
	if (objLoader.LoadModel("TestModels/temple.obj"))//returns true if the model is loaded
	{
		static_models[5].ConstructModelFromOBJLoader(objLoader);
		static_models[5].InitVBO(myShader);
	}
	if (objLoader.LoadModel("TestModels/skybox.obj"))//returns true if the model is loaded
	{
		skybox.ConstructModelFromOBJLoader(objLoader);
		skybox.InitVBO(myShader);
	}
}

// run on key down
void special(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		Left = true;
		break;
	case GLUT_KEY_RIGHT:
		Right = true;
		break;
	case GLUT_KEY_UP:
		Up = true;
		break;
	case GLUT_KEY_DOWN:
		Down = true;
		break;
	case GLUT_KEY_HOME:
		Home = true;
		break;
	case GLUT_KEY_END:
		End = true;
		break;
	}
}
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 32:
		Space = true;
		camera_view = (camera_view + 1) % 5;
		camera_coords = glm::vec3(0.0f, 10.0f, 20.0f);;
		camera_h_rotation = 0.0;
		camera_v_rotation = 0.0;
		break;

	// camera controls
	case 'w':
		W = true;
		break;
	case 'a':
		A = true;
		break;
	case 's':
		S = true;
		break;
	case 'd':
		D = true;
		break;
	case 'r':
		R = true;
		break;
	case 'f':
		F = true;
		break;
	case 'i':
		I = true;
		break;
	case 'j':
		J = true;
		break;
	case 'k':
		K = true;
		break;
	case 'l':
		L = true;
		break;

	case 'm':
		// toggle the sun
		if (Light_Specular[0] == 1.0) {
			Light_Specular[0] = 0.0;
			Light_Specular[1] = 0.0;
			Light_Specular[2] = 0.0;
			Light_Specular[3] = 0.0;
			//float Light_Ambient_And_Diffuse[4] = { 0.8f, 0.8f, 0.6f, 1.0f };
			Light_Ambient_And_Diffuse[0] = 0.0;
			Light_Ambient_And_Diffuse[1] = 0.0;
			Light_Ambient_And_Diffuse[2] = 0.0;
			Light_Ambient_And_Diffuse[3] = 0.0;
		}
		else {
			Light_Specular[0] = 1;
			Light_Specular[1] = 1;
			Light_Specular[2] = 1;
			Light_Specular[3] = 1;
			Light_Ambient_And_Diffuse[0] = 0.8;
			Light_Ambient_And_Diffuse[1] = 0.8;
			Light_Ambient_And_Diffuse[2] = 0.8;
			Light_Ambient_And_Diffuse[3] = 1.0;
		}
		break;

	case 'n':
		// toggle the spotlight
		if (Lamp_Ambient[0] == 0.1f) {
			Lamp_Ambient[0] = 0.0f;
			Lamp_Diffuse[0] = 0.0f;
			Lamp_Diffuse[1] = 0.0f;
			Lamp_Diffuse[2] = 0.0f;
			Lamp_Diffuse[3] = 0.0f;
			Lamp_Specular[0] = 0.0f;
			Lamp_Specular[1] = 0.0f;
			Lamp_Specular[2] = 0.0f;
			Lamp_Specular[3] = 0.0f;
		}
		else {
			Lamp_Ambient[0] = 0.1f;
			Lamp_Ambient[1] = 0.0f;
			Lamp_Diffuse[0] = 0.8f;
			Lamp_Diffuse[1] = 0.1f;
			Lamp_Diffuse[2] = 0.1f;
			Lamp_Diffuse[3] = 1.0f;
			Lamp_Specular[0] = 1.0f;
			Lamp_Specular[1] = 0.3f;
			Lamp_Specular[2] = 0.3f;
			Lamp_Specular[3] = 1.0f;
		}
		break;

	case 'b':
		// toggle the cup light
		if (Cup_Ambient[0] == 0.2f) {
			Cup_Ambient[0] = 0.0f;
			Cup_Ambient[1] = 0.0f;
			Cup_Ambient[2] = 0.0f;
			Cup_Ambient[3] = 0.0f;
			Cup_Diffuse[0] = 0.0f;
			Cup_Diffuse[1] = 0.0f;
			Cup_Diffuse[2] = 0.0f;
			Cup_Diffuse[3] = 0.0f;
			Cup_Specular[0] = 0.0f;
			Cup_Specular[1] = 0.0f;
			Cup_Specular[2] = 0.0f;
			Cup_Specular[3] = 0.0f;
		}
		else {
			Cup_Ambient[0] = 0.2f;
			Cup_Ambient[1] = 0.2f;
			Cup_Ambient[2] = 0.2f;
			Cup_Ambient[3] = 1.0f;
			Cup_Diffuse[0] = 1.0f;
			Cup_Diffuse[1] = 1.0f;
			Cup_Diffuse[2] = 1.0f;
			Cup_Diffuse[3] = 1.0f;
			Cup_Specular[0] = 1.0f;
			Cup_Specular[1] = 1.0f;
			Cup_Specular[2] = 1.0f;
			Cup_Specular[3] = 1.0f;
		}
		break;
	}
}

// run on key up
void specialUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		Left = false;
		break;
	case GLUT_KEY_RIGHT:
		Right = false;
		break;
	case GLUT_KEY_UP:
		Up = false;
		break;
	case GLUT_KEY_DOWN:
		Down = false;
		break;
	case GLUT_KEY_HOME:
		Home = false;
		break;
	case GLUT_KEY_END:
		End = false;
		break;		
	}
}
void keyboardUp(unsigned char key, int x, int y) {
	switch (key) {
	case 32:
		Space = false;
		break;

		// camera controls
	case 'w':
		W = false;
		break;
	case 'a':
		A = false;
		break;
	case 's':
		S = false;
		break;
	case 'd':
		D = false;
		break;
	case 'r':
		R = false;
		break;
	case 'f':
		F = false;
		break;
	case 'i':
		I = false;
		break;
	case 'j':
		J = false;
		break;
	case 'k':
		K = false;
		break;
	case 'l':
		L = false;
		break;
	}
}

// run every frame key is pressed
void processKeys()
{
	if (Left)
	{
	}
	if (Right)
	{
	}
	if (Up)
	{
		if (base_rotation_speed < 0.06f) {
			base_rotation_speed += 0.00001f;
			min_cup_speed -= 0.00005f;
			max_cup_speed += 0.00005f;
		}
	}
	if (Down)
	{
		if (base_rotation_speed > 0.0f) {
			base_rotation_speed -= 0.00001f;
			min_cup_speed += 0.00005f;
			max_cup_speed -= 0.00005f;
		}
		if (base_rotation_speed < 0.00002f) {
			base_rotation_speed = 0.0f;
		}
	}
	if (Home)
	{
	}
	if (End)
	{
	}

	camera_input = glm::vec3(0.0f, 0.0f, 0.0f);
	
	if (R && camera_view == 4) {
		camera_input[1] += 1.0f;
	}
	if (F && camera_view == 4) {
		camera_input[1] -= 1.0f;
	}
	if (I && camera_view == 4) {
		if (camera_v_rotation < 1.5f) {
			camera_v_rotation += 0.0005f;
		}
	}
	if (K && camera_view == 4) {
		if (camera_v_rotation > -1.5f) {
			camera_v_rotation -= 0.0005f;
		}
	}
	if (J && camera_view == 4) {
		camera_h_rotation -= 0.0006f;
	}
	if (L && camera_view == 4) {
		camera_h_rotation += 0.0006f;
	}
	if (W && camera_view == 4) {
		camera_input[2] -= 1.0f;
		//float camera_x_offset = sin(camera_h_rotation) * camera_speed;
		//float camera_z_offset = cos(camera_h_rotation) * -camera_speed;
		//camera_coords[0] += camera_x_offset;
		//camera_coords[2] += camera_z_offset;
	}
	if (S && camera_view == 4) {
		camera_input[2] += 1.0f;
		//float camera_x_offset = sin(camera_h_rotation) * -camera_speed;
		//float camera_z_offset = cos(camera_h_rotation) * camera_speed;
		//camera_coords[0] += camera_x_offset;
		//camera_coords[1] += camera_z_offset;
	}
	if (A && camera_view == 4) {
		camera_input[0] -= 1.0f;
		//float camera_x_offset = sin(glm::half_pi<float>() - camera_h_rotation) * -camera_speed;
		//float camera_z_offset = cos(glm::half_pi<float>() - camera_h_rotation) * camera_speed;
		//camera_coords[0] += camera_x_offset;
		//camera_coords[2] += camera_z_offset;
		//cout << "X = " << camera_x << ", Z = " << camera_z << endl;
		//camera_z += -camera_speed;
		//camera_x += -camera_speed;
	}
	if (D && camera_view == 4) {
		camera_input[0] += 1.0f;
		//float camera_z_offset = sin(camera_h_rotation) * -camera_speed;
		//float camera_x_offset = cos(camera_h_rotation) * camera_speed;
		//camera_coords[0] += camera_x_offset;
		//camera_coords[2] += camera_z_offset;
		//cout << "X = " << camera_x << ", Z = " << camera_z << endl;
		//camera_z += camera_speed;
		//camera_x += camera_speed;
	}
}


void updateTransform(float xinc, float yinc, float zinc)
{
	objectRotation = glm::rotate(objectRotation, xinc, glm::vec3(1,0,0));
	objectRotation = glm::rotate(objectRotation, yinc, glm::vec3(0,1,0));
	objectRotation = glm::rotate(objectRotation, zinc, glm::vec3(0,0,1));
}

void idle()
{
	spin += speed;
	if(spin > 360)
		spin = 0;

	processKeys();

	glutPostRedisplay();
}
/**************** END OPENGL FUNCTIONS *************************/


// COLLISION DETECTION
int isValidCoord(float x, float y, float z) {
	if (y <= 2) {	// floor
		return 0;
	}

	// get a rotated version of the coordinates
	// so that collision can be done as an on-axis cuboid for building 1
	glm::vec3 rot_coord = glm::rotateY(glm::vec3(x, y, z), (float) glm::radians(45.0));
	float proj_dist = 50 * pow(2, 0.5);
	if (rot_coord.x <= 0-proj_dist + 22 && rot_coord.x >= 0-proj_dist - 22 && rot_coord.z <= proj_dist + 22 && rot_coord.z >= proj_dist - 22 && y <= 81) {  // building 1
		return 0;
	}

	// building 2
	if (x >= -42 && x <= 42 && z >= -115 && z <= -85 && y <= 25) { // cuboid
		return 0;
	}
	if (pow(x, 2) + pow(z + 100, 2) <= 676 && y <= 25) { // cylinder
		return 0;
	}
	if (pow(x, 2) + pow(z + 100, 2) + pow(y-25, 2) <= 441) { // sphere
		return 0;
	}

	// building 3a
	if (x<=122 && x>= 78 && z>=-32 && z<=-8 && y<=71) {
		return 0;
	}
	if (x<=122 && x>=98 && z>=-42 && z<=-28 && y<= 71) {
		return 0;
	}

	// building 3b
	if (x <= 122 && x >= 78 && z >= 8 && z <= 22 && y <= 61) {
		return 0;
	}
	if (x <= 122 && x >= 108 && z >= 18 && z <= 42 && y <= 61) {
		return 0;
	}

	// temple octree
	if (static_models[5].OctreeCollision(x, y, z) == true) {
		//cout << "COLLISION" << endl;
		return 0;
	}


	// lamppost
	if (pow(x, 2) + pow(z, 2) <= 8 && y <= 18) { // cylinder
		return 0;
	}

	// edge
	if (pow(x, 2) + pow(z, 2) + pow(y, 2) >= 39000) { // sphere
		return 0;
	}

	if (base_rotation_speed != 0.0f) {
		if (pow(x, 2) + pow(z, 2) < 100 && y <= 8) { // cylinder approximating cup area
			return 0;
		}
	}
	else {
		// individual cup collision
		rot_coord = glm::rotateY(glm::vec3(x, y, z), (float)glm::radians(-base_rotation));
		if (pow(rot_coord.x-6, 2) + pow(rot_coord.z, 2) < 15 && y <= 8) { // cylinder
			return 0;
		}
		rot_coord = glm::rotateY(glm::vec3(x, y, z), (float)glm::radians(-base_rotation + 120));
		if (pow(rot_coord.x - 6, 2) + pow(rot_coord.z, 2) < 15 && y <= 8) { // cylinder
			return 0;
		}
		rot_coord = glm::rotateY(glm::vec3(x, y, z), (float)glm::radians(-base_rotation - 120));
		if (pow(rot_coord.x - 6, 2) + pow(rot_coord.z, 2) < 15 && y <= 8) { // cylinder
			return 0;
		}
	}

	return 1;
}



int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(screenWidth, screenHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("OpenGL FreeGLUT Example: Obj loading");

	//This initialises glew - it must be called after the window is created.
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cout << " GLEW ERROR" << endl;
	}

	//Check the OpenGL version being used
	int OpenGLVersion[2];
	glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);
	cout << OpenGLVersion[0] << " " << OpenGLVersion[1] << endl;


	//initialise the objects for rendering
	init();

	glutReshapeFunc(reshape);
	//specify which function will be called to refresh the screen.
	glutDisplayFunc(display);

	glutSpecialFunc(special);
	glutSpecialUpFunc(specialUp);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);

	glutIdleFunc(idle);

	//starts the main loop. Program loops and calls callback functions as appropriate.
	glutMainLoop();

	return 0;
}
