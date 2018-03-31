/*
* TexturedScene2012.c
*
* Author: Samuel R. Buss
*
* Bug reports: Sam Buss, sbuss@ucsd.edu.
*
* Base code for homework project #5, Math 155A, 2012
*
* USAGE:   (Please keep these controls in your homework assignment.)
*
*    Press arrow keys to control view position.
*	  left and right keys rotate the viewpoint left and right
*	  Up and down keys rotate viewpoint up and down (up to 80 degrees).
*
*    PAGE-UP, or +, more the viewpoint closer
*    PAGE-DOWN or -, move the viewpoint farther away
*
*    Press "R" key to make step size bigger (if moving too slowly)
*    Press "r" key to make step size smaller (if moving too fast).
*
*    Press "w" key to toggle wireframe mode on and off
*
*    Press "M" , "m" to increase, decrease Mesh Resolution
*
*    Press "1", "2", "3" to toggle the three lights being off or on.
*
*	  Press ESCAPE to exit.
*
*/

#include <math.h>			// For math routines (such as sqrt & trig).
#include <stdio.h>
//#include <stdlib.h>		// For the "exit" function
#include <GL/glut.h>		// OpenGL Graphics Utility Library
#include "Moyan_Zhou_Project6.h"
#include "RgbImage.h"

// Not available in the header file, but needed anyway.
#define GL_LIGHT_MODEL_COLOR_CONTROL      0x81F8
#define GL_SINGLE_COLOR                   0x81F9
#define GL_SEPARATE_SPECULAR_COLOR        0x81FA

const int NumLoadedTextures = 16;

static GLuint textureName[NumLoadedTextures];		// Holds OpenGL's internal texture names (not filenames)
													//static GLuint noMiptextureName[NumLoadedTextures];

char* filenames[NumLoadedTextures] = {
	"soil.bmp",
	"wallpaper.bmp",
	"KalachakraSera.bmp",
	"RoughWood.bmp",
	"forest.bmp",
	"wood3.bmp",
	"caarpet.bmp",
	"wood.bmp",
	"wood2.bmp",
	"bedleg.bmp",
	"table.bmp",
	"bookshelf.bmp",
	"towel.bmp",
	"map.bmp",
	"window.bmp",
	"door.bmp"
};

// The next global variable controls the animation's state and speed.
float RotateAngle = 0.0f;		// Angle in degrees of rotation around y-axis
float Azimuth = 0.0;			// Rotated up or down by this amount
float AngleStepSize = 3.0f;		// Step three degrees at a time
const float AngleStepMax = 10.0f;
const float AngleStepMin = 0.1f;
float DistanceAway = 20.0f;
float MinDistance = 5.0f;
float MaxDistance = 80.0f;
float DistanceStepSize = 0.1f;

float radius = 0.5f;	// Dimensions of the cylinder
float height = 1.5f;

// Some global state variables
int MeshCount = 6;				// The mesh resolution for the mushroom top
int WireFrameOn = 1;
const int MeshCountMin = 3;

// A texture
int TextureWrapVert = 6;
int TextureWrapHoriz = 6;
bool textureFlag = true;

bool UseMipmapping = true;
bool OtherMode = false;
bool singleStep = true;
bool lightMode = true;
// Light controls
float GlobalAmbient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
bool Light0On = true;
bool Light1On = true;
bool Light2On = true;
float Light0AmbientDiffuse[4] = { 1.0f, 0.2f, 0.2f, 1.0f };
float Light0Specular[4] = { 1.0f, 0.2f, 0.2f, 1.0f };
float Light1AmbientDiffuse[4] = { 1.0, 1.0, 1.0, 1.0 };
float Light1Specular[4] = { 1.0, 1.0, 1.0, 1.0 };
float Light2AmbientDiffuse[4] = { 0.2f, 1.0f, 0.2f, 1.0f };
float Light2Specular[4] = { 0.2f, 1.0f, 0.2f, 1.0f };
float Light0Pos[4] = { 0.2, 5.0, 0.0, 1.0 };
float Light1Pos[4] = { -0.2, 5.0, 0.0, 1.0 };
float Light2Pos[4] = { 0.0, 5.0, -0.5, 1.0 };

float blackColor[4] = { 0.0, 0.0, 0.0, 1.0 };
float whiteColor[4] = { 1.0, 1.0, 1.0, 1.0 };

static float HourOfDay = 0.0;
static float DayOfYear = 0.0;
static float AnimateIncrement = 24.0;
// Material colors
float SpecularExponent = 90;
float PlaneBaseAmbientDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0 };
float UnderTextureAmbientDiffuse[4] = { 1.0, 1.0, 1.0, 1.0 };

/*
* Read a texture map from a BMP bitmap file.
*/
void loadTextureFromFile(char *filename, bool UseMip)
{
	RgbImage theTexMap(filename);
	// Pixel alignment: each row is word aligned (aligned to a 4 byte boundary)
	//    Therefore, no need to call glPixelStore( GL_UNPACK_ALIGNMENT, ... );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Use bilinear interpolation between texture pixels.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// Don't use bilinear interpolation
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	if (!UseMip) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// This sets up the texture may without mipmapping
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, theTexMap.GetNumCols(), theTexMap.GetNumRows(),
			0, GL_RGB, GL_UNSIGNED_BYTE, theTexMap.ImageData());
	}
	else {
		//Keep the next line for best mipmap linear and bilinear interpolation.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, theTexMap.GetNumCols(), theTexMap.GetNumRows(),
			GL_RGB, GL_UNSIGNED_BYTE, theTexMap.ImageData());
	}
}


// glutKeyboardFunc is called below to set this function to handle
//		all "normal" key presses.
void myKeyboardFunc(unsigned char key, int x, int y)
{
	switch (key) {
	case 'm':
		MeshCount = (MeshCount>MeshCountMin) ? MeshCount - 1 : MeshCount;
		glutPostRedisplay();
		break;
	case 'M':
		MeshCount++;
		glutPostRedisplay();
		break;
	case 'w':
		WireFrameOn = 1 - WireFrameOn;
		if (WireFrameOn) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		// Just show wireframes
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);		// Show solid polygons
		}
		glutPostRedisplay();
		break;
	case 'R':
		AngleStepSize *= 1.5;
		if (AngleStepSize>AngleStepMax) {
			AngleStepSize = AngleStepMax;
		}
		break;
	case 'r':
		AngleStepSize /= 1.5;
		if (AngleStepSize<AngleStepMin) {
			AngleStepSize = AngleStepMin;
		}
		break;
	case '1':
		Light0On = !Light0On;
		if (Light0On) {
			glEnable(GL_LIGHT0);
		}
		else {
			glDisable(GL_LIGHT0);
		}
		glutPostRedisplay();
		break;
	case '2':
		Light1On = !Light1On;
		if (Light1On) {
			glEnable(GL_LIGHT1);
		}
		else {
			glDisable(GL_LIGHT1);
		}
		glutPostRedisplay();
		break;
	case '3':
		Light2On = !Light2On;
		if (Light2On) {
			glEnable(GL_LIGHT2);
		}
		else {
			glDisable(GL_LIGHT2);
		}
		glutPostRedisplay();
		break;
	case '+':
		MoveCloser();
		glutPostRedisplay();
		break;
	case '-':
		MoveAway();
		glutPostRedisplay();
		break;
	case 'b':
		if (OtherMode == true) {
			OtherMode = false;
		}
		else {
			OtherMode = true;
		}
		glutPostRedisplay();
		break;
	case 'c':
		if (lightMode == true) {
			//UseMipmapping == false;
			lightMode = false;
		}
		else {
			//UseMipmapping == true;
			lightMode = true;
		}
		glutPostRedisplay();
		break;
	case 27:	// Escape key
		exit(1);
	}
}

// glutSpecialFunc is called below to set this function to handle
//		all "special" key presses.  See glut.h for the names of
//		special keys.
void mySpecialKeyFunc(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP:
		Azimuth += AngleStepSize;
		if (Azimuth>80.0f) {
			Azimuth = 80.0f;
		}
		break;
	case GLUT_KEY_DOWN:
		Azimuth -= AngleStepSize;
		if (Azimuth < -80.0f) {
			Azimuth = -80.0f;
		}
		break;
	case GLUT_KEY_LEFT:
		RotateAngle += AngleStepSize;
		if (RotateAngle > 180.0f) {
			RotateAngle -= 360.0f;
		}
		break;
	case GLUT_KEY_RIGHT:
		RotateAngle -= AngleStepSize;
		if (RotateAngle < -180.0f) {
			RotateAngle += 360.0f;
		}
		break;
	case GLUT_KEY_PAGE_UP:
		MoveCloser();
		break;
	case GLUT_KEY_PAGE_DOWN:
		MoveAway();
		break;
	}

	glutPostRedisplay();

}

void MoveCloser() {
	DistanceAway = DistanceAway - DistanceStepSize;
	if (DistanceAway < MinDistance) {
		DistanceAway = MinDistance;
	}
}

void MoveAway() {
	DistanceAway = DistanceAway + DistanceStepSize;
	if (DistanceAway > MaxDistance) {
		DistanceAway = MaxDistance;
	}
}

/*
* drawScene() handles the animation and the redrawing of the
*		graphics window contents.
*/
void drawScene(void)
{
	// Clear the rendering window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// Rotate the image
	glMatrixMode(GL_MODELVIEW);			// Current matrix affects objects positions
	glLoadIdentity();						// Initialize to the identity
	glTranslatef(0.0, -1.0, -DistanceAway);	// Translate from origin (in front of viewer)
	glRotatef(Azimuth, 1.0, 0.0, 0.0);			// Set Azimuth angle
	glRotatef(RotateAngle, 0.0, 1.0, 0.0);		// Rotate around y-axis

												// Set lights' positions (directions)
	glLightfv(GL_LIGHT0, GL_POSITION, Light0Pos);
	glLightfv(GL_LIGHT1, GL_POSITION, Light1Pos);
	glLightfv(GL_LIGHT2, GL_POSITION, Light2Pos);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, blackColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, blackColor);
	float lightrad = 0.25f;
	float lighthei = 0.5f;
	if (Light0Pos[3] != 0.0f && Light0On) {
		if (lightMode == true) {
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Light0AmbientDiffuse);
		}
		else {
			Light0Pos[3] = 0;
			glLightfv(GL_LIGHT0, GL_POSITION, Light0Pos);
			glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 10.0);
			glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 6.0);
			glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, Light0Pos);
			Light0Pos[3] = 1;
		}
		glPushMatrix();
		glRotatef(60, 0, 1, 0);
		glTranslatef(Light0Pos[0], Light0Pos[1], Light0Pos[2]);
		// Draw the cylinder
	    glBindTexture(GL_TEXTURE_2D, textureName[3]);	// Texture 3 is active now

		myDrawCylinder(lightrad - 0.2, lighthei + 1);				// Project 5: This already has the needed texture coordinates.  NOTHING TO DO NOW.

														// Undo texture mode and specular highlights once spheres are drawn (so will affect base plane)
		// Draw the left and right caps of the cylinder
		myDrawCylinderCaps(lightrad - 0.2, lighthei + 1);
		//glutWireSphere(0.125, 10, 10);
		glPopMatrix();

	}
	if (Light1Pos[3] != 0.0f && Light1On) {
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Light1AmbientDiffuse);
	
		glPushMatrix();
		glRotatef(-60, 0, 1, 0);
		glTranslatef(Light1Pos[0], Light1Pos[1], Light1Pos[2]);
		// Draw the cylinder
		glBindTexture(GL_TEXTURE_2D, textureName[3]);	// Texture 3 is active now

		myDrawCylinder(lightrad - 0.2, lighthei + 1);				// Project 5: This already has the needed texture coordinates.  NOTHING TO DO NOW.

														// Undo texture mode and specular highlights once spheres are drawn (so will affect base plane)
														// Draw the left and right caps of the cylinder
		myDrawCylinderCaps(lightrad - 0.2, lighthei + 1);
		//glutWireSphere(0.125, 10, 10);
		glPopMatrix();
	}
	if (Light2Pos[3] != 0.0f && Light2On) {
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Light2AmbientDiffuse);
		glPushMatrix();
		glTranslatef(Light2Pos[0], Light2Pos[1], Light2Pos[2]);
		// Draw the cylinder
		glBindTexture(GL_TEXTURE_2D, textureName[3]);	// Texture 3 is active now

		myDrawCylinder(lightrad - 0.2, lighthei + 1);				// Project 5: This already has the needed texture coordinates.  NOTHING TO DO NOW.

														// Undo texture mode and specular highlights once spheres are drawn (so will affect base plane)
														// Draw the left and right caps of the cylinder
		myDrawCylinderCaps(lightrad - 0.2, lighthei + 1);
		//glutWireSphere(0.125, 10, 10);
		glPopMatrix();
	}
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, blackColor);

	// Material and texture mode
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, UnderTextureAmbientDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, whiteColor);

	// Draw the back wall (textured brick wall). Back Wall
	if (OtherMode == true) {
		glBindTexture(GL_TEXTURE_2D, textureName[4]);	// Texture 0 is active now
	}
	else {
		glBindTexture(GL_TEXTURE_2D, textureName[1]);
	}
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);		// Same normal for the next four vertices.
	glTexCoord2f(0.0, 0.0);			// Texture coordinates for the back wall vertex
	glVertex3f(-6.0, 0.0, -5.0);
	glTexCoord2f(1.0, 0.0);			// Texture coordinates for the back wall vertex
	glVertex3f(6.0, 0.0, -5.0);
	glTexCoord2f(1.0, 1.0);			// Texture coordinates for the back wall vertex
	glVertex3f(6.0, 5.0, -5.0);
	glTexCoord2f(0.0, 1.0);			// Texture coordinates for the back wall vertex
	glVertex3f(-6.0, 5.0, -5.0);
	glEnd();

	// Draw the back wall (textured brick wall). Left wall
	if (OtherMode == true) {
		glBindTexture(GL_TEXTURE_2D, textureName[4]);	// Texture 0 is active now
	}
	else {
		glBindTexture(GL_TEXTURE_2D, textureName[1]);
	}

	glBegin(GL_QUADS);
	//glTranslatef(0.0, 0.0, -3);
	glNormal3f(1.0, 0.0, 0.0);		// Same normal for the next four vertices.
	glTexCoord2f(0.0, 0.0);			// Texture coordinates for the back wall vertex
	glVertex3f(-6.0, 0.0, 2.0);
	glTexCoord2f(1.0, 0.0);			// Texture coordinates for the back wall vertex
	glVertex3f(-6.0, 0.0, -5.0);
	glTexCoord2f(1.0, 1.0);			// Texture coordinates for the back wall vertex
	glVertex3f(-6.0, 5.0, -5.0);
	glTexCoord2f(0.0, 1.0);			// Texture coordinates for the back wall vertex
	glVertex3f(-6.0, 5.0, 2.0);
	glEnd();

	/*glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);		// Same normal for the next four vertices.
	glTexCoord2f(0.0, 0.0);			// Texture coordinates for the back wall vertex
	glVertex3f(-6.0, 0.0, 5.0);
	glTexCoord2f(1.0, 0.0);			// Texture coordinates for the back wall vertex
	glVertex3f(-6.0, 0.0, -5.0);
	glTexCoord2f(1.0, 1.0);			// Texture coordinates for the back wall vertex
	glVertex3f(-6.0, 5.0, -5.0);
	glTexCoord2f(0.0, 1.0);			// Texture coordinates for the back wall vertex
	glVertex3f(-6.0, 5.0, 5.0);
	glEnd();*/

	// Draw the back wall (textured brick wall). Right wall
	if (OtherMode == true) {
		glBindTexture(GL_TEXTURE_2D, textureName[4]);	// Texture 0 is active now
	}
	else {
		glBindTexture(GL_TEXTURE_2D, textureName[1]);
	}
	//glTranslatef(0.0f, 0.0f, -3.0f);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);		// Same normal for the next four vertices.
	glTexCoord2f(0.0, 0.0);			// Texture coordinates for the back wall vertex
	glVertex3f(6.0, 0.0, -5.0);
	glTexCoord2f(1.0, 0.0);			// Texture coordinates for the back wall vertex
	glVertex3f(6.0, 0.0, 5.0);
	glTexCoord2f(1.0, 1.0);			// Texture coordinates for the back wall vertex
	glVertex3f(6.0, 5.0, 5.0);
	glTexCoord2f(0.0, 1.0);			// Texture coordinates for the back wall vertex
	glVertex3f(6.0, 5.0, -5.0);
	glEnd();


	// Draw the back wall (textured brick wall). Front wall
	if (OtherMode == true) {
		glBindTexture(GL_TEXTURE_2D, textureName[4]);	// Texture 0 is active now
	}
	else {
		glBindTexture(GL_TEXTURE_2D, textureName[1]);
	}
	
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);		// Same normal for the next four vertices.
	glTexCoord2f(0.0, 0.0);			// Texture coordinates for the back wall vertex
	glVertex3f(6.0, 0.0, 5.0);
	glTexCoord2f(1.0, 0.0);			// Texture coordinates for the back wall vertex
	glVertex3f(-6.0, 0.0, 5.0);
	glTexCoord2f(1.0, 1.0);			// Texture coordinates for the back wall vertex
	glVertex3f(-6.0, 5.0, 5.0);
	glTexCoord2f(0.0, 1.0);			// Texture coordinates for the back wall vertex
	glVertex3f(6.0, 5.0, 5.0);
	glEnd();


	// Draw the first sphere
	glBindTexture(GL_TEXTURE_2D, textureName[13]);	// Texture 1 is active now
	glPushMatrix();
	glTranslatef(5, 4.3, 3.5);		// Sphere slightly repositioned in revived Project 5
	glScalef(0.5, 0.5, 0.5);
	myDrawSphere(true);				// Project 5: You will need to update the texture coordinates in this function!
	glPopMatrix();

	// Draw the second sphere
	/*glBindTexture(GL_TEXTURE_2D, textureName[4]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(2.5, 1.0, 0.0);		// Sphere slightly repositioned in revived Project 5
	myDrawSphere(false);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();*/

	// Draw the table
	glBindTexture(GL_TEXTURE_2D, textureName[10]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(2, 1.0, 0.0);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0.0, 0.3, 0.0, 4.0, 0.3, 5.0);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();
	
	// NEW IN REVISED VERSION OF PROJECT 5.
	glBindTexture(GL_TEXTURE_2D, textureName[2]);	// Texture 4 is active now
	glPushMatrix();
	glTranslatef(4.0, 1.85, 1.75);
	glScalef(0.08f, 0.35f, 0.08f);
	myDrawSurfaceOfRotation();			// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();
    
	// draw the table legs: the left and back one
	float tableLegh = 1.35;
	float tableLegr = 0.3;
	glPushMatrix();
	glTranslatef(2.85, 0.65, 0.85);
	// Draw the cylinder
	glBindTexture(GL_TEXTURE_2D, textureName[10]);	// Texture 3 is active now

	myDrawTableLeg(tableLegr, tableLegh);				// Project 5: This already has the needed texture coordinates.  NOTHING TO DO NOW.
													// Draw the left and right caps of the cylinder
	myDrawTableLegCaps(tableLegr, tableLegh);
	//glutWireSphere(0.125, 10, 10);
	glPopMatrix();

	// draw the table leg: the left and front one
	glPushMatrix();
	glTranslatef(2.85, 0.65, 4.15);
	// Draw the cylinder
	glBindTexture(GL_TEXTURE_2D, textureName[10]);	// Texture 3 is active now

	myDrawTableLeg(tableLegr, tableLegh);				// Project 5: This already has the needed texture coordinates.  NOTHING TO DO NOW.
														// Draw the left and right caps of the cylinder
	myDrawTableLegCaps(tableLegr, tableLegh);
	//glutWireSphere(0.125, 10, 10);
	glPopMatrix();

	// draw the table leg: the right and back one
	glPushMatrix();
	glTranslatef(5.2, 0.65, 0.85);
	// Draw the cylinder
	glBindTexture(GL_TEXTURE_2D, textureName[10]);	// Texture 3 is active now

	myDrawTableLeg(tableLegr, tableLegh);				// Project 5: This already has the needed texture coordinates.  NOTHING TO DO NOW.
														// Draw the left and right caps of the cylinder
	myDrawTableLegCaps(tableLegr, tableLegh);
	//glutWireSphere(0.125, 10, 10);
	glPopMatrix();

	// draw the table leg: the left and front one
	glPushMatrix();
	glTranslatef(5.2, 0.65, 4.15);
	// Draw the cylinder
	glBindTexture(GL_TEXTURE_2D, textureName[10]);	// Texture 3 is active now

	myDrawTableLeg(tableLegr, tableLegh);				// Project 5: This already has the needed texture coordinates.  NOTHING TO DO NOW.
														// Draw the left and right caps of the cylinder
	myDrawTableLegCaps(tableLegr, tableLegh);
	//glutWireSphere(0.125, 10, 10);
	glPopMatrix();

	// draw the bookshelf: the back 
	glBindTexture(GL_TEXTURE_2D, textureName[11]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(4, 1.3, 0.5);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0.0, 0.3, 0.0, 2, 3, 0.2);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();
	// draw the bookshelf: the front
	glBindTexture(GL_TEXTURE_2D, textureName[11]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(4, 1.3, 4.2);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0.0, 0.3, 0.0, 2, 3, 0.2);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	// draw the bookshelf: the first level
	glBindTexture(GL_TEXTURE_2D, textureName[11]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(4, 2.3, 0.7);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0.0, 0.3, 0.0, 2, 0.2, 3.5);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	// draw the bookshelf: the first level
	glBindTexture(GL_TEXTURE_2D, textureName[11]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(4, 3.3, 0.7);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0.0, 0.3, 0.0, 2, 0.2, 3.5);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();



	// draw the bed
	glBindTexture(GL_TEXTURE_2D, textureName[5]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(-1.3, 0.6, -5);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0.0, 0.1, 0.0, 7.0, 0.5, 3.5);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, textureName[12]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(-1.05, 1.1, -4.75);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0.0, 0.1, 0.0, 6.5, 0.2, 3);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();
	
	// the back of the bed
	glBindTexture(GL_TEXTURE_2D, textureName[5]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(5.65, 0.05, -10);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0,0, 5, 0.3, 3, 3.5);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	// the leg of the bed: the back one
	float bedLegr = 0.2;
	float bedLegh = 0.65;
	glPushMatrix();
	glTranslatef(-0.3, 0.35, -4.6);
	// Draw the cylinder
	glBindTexture(GL_TEXTURE_2D, textureName[9]);	// Texture 3 is active now

	myDrawTableLeg(bedLegr, bedLegh);				// Project 5: This already has the needed texture coordinates.  NOTHING TO DO NOW.
														// Draw the left and right caps of the cylinder
	myDrawTableLegCaps(bedLegr, bedLegh);
	//glutWireSphere(0.125, 10, 10);
	glPopMatrix();
	
	// the leg of the bed: the front one
	glPushMatrix();
	glTranslatef(-0.3, 0.35, -2.2);
	// Draw the cylinder
	glBindTexture(GL_TEXTURE_2D, textureName[9]);	// Texture 3 is active now

	myDrawTableLeg(bedLegr, bedLegh);				// Project 5: This already has the needed texture coordinates.  NOTHING TO DO NOW.
													// Draw the left and right caps of the cylinder
	myDrawTableLegCaps(bedLegr, bedLegh);
	//glutWireSphere(0.125, 10, 10);
	glPopMatrix();

	// the closet
	// the back of the closet
	glBindTexture(GL_TEXTURE_2D, textureName[7]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(-5.99, 0.15, -9.8);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0, 0, 5, 0.2, 2.4, 4.5);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	// the front of the closet
	glBindTexture(GL_TEXTURE_2D, textureName[7]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(-3.99, 0.15, -9.8);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0, 0, 5, 0.2, 2.4, 4.5);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	// the left of the closet
	glBindTexture(GL_TEXTURE_2D, textureName[7]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(-6, 0.15, -0.3);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0, 0, 0, 2.2, 2.4, 0.2);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	// the left of the closet
	glBindTexture(GL_TEXTURE_2D, textureName[7]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(-6, 0.15, -5);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0, 0, 0, 2.2, 2.4, 0.2);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	// the top of the closet
	glBindTexture(GL_TEXTURE_2D, textureName[7]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(-6, 2.55, -5);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0, 0, 0, 2.2, 0.2, 4.9);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	// the bottom of the closet
	glBindTexture(GL_TEXTURE_2D, textureName[7]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(-6, 0, -5);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0, 0, 0, 2.2, 0.15, 4.9);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	// the drawers of the closet
	glBindTexture(GL_TEXTURE_2D, textureName[8]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(-3.75, 0.35, -9.6);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0, 0, 5, 0.2, 0.9, 4.1);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();


	glBindTexture(GL_TEXTURE_2D, textureName[8]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(-3.75, 1.35, -9.6);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0, 0, 5, 0.2, 0.9, 4.1);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	// the handle on the drawers: lower one
	glBindTexture(GL_TEXTURE_2D, textureName[1]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(-3.4, 0.8, -2.5);		// Sphere slightly repositioned in revived Project 5
	glScalef(0.15, 0.15, 0.15);
	myDrawSphere(false);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	// upper one
	glBindTexture(GL_TEXTURE_2D, textureName[1]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(-3.4, 1.8, -2.5);		// Sphere slightly repositioned in revived Project 5
	glScalef(0.15, 0.15, 0.15);
	myDrawSphere(false);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	// the chair
	glBindTexture(GL_TEXTURE_2D, textureName[3]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(0.5, 0.4, 1.3);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0.0, 0.2, 0.0, 2, 0.1, 2);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	// the back of the chair
	glBindTexture(GL_TEXTURE_2D, textureName[3]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(0.5, 0.5, 1.3);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0.0, 0.2, 0.0, 0.1, 1.7, 0.5);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();


	glBindTexture(GL_TEXTURE_2D, textureName[3]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(0.5, 0.5, 2);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0.0, 0.2, 0.0, 0.1, 1.7, 0.5);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, textureName[3]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(0.5, 0.5, 2.8);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0.0, 0.2, 0.0, 0.1, 1.7, 0.5);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, textureName[3]);	// Texture 2 is active now
	glPushMatrix();
	glTranslatef(0.5, 2.2, 1.3);		// Sphere slightly repositioned in revived Project 5
	myDrawTable(0.0, 0.2, 0.0, 0.1, 0.2, 2);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	// Draw the base plane
	if (OtherMode == true) {
		glBindTexture(GL_TEXTURE_2D, textureName[0]);	// Texture 0 is active now
	}
	else {
		glBindTexture(GL_TEXTURE_2D, textureName[6]);
	}
	// Draw the base plane
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);		// Same normal for the next four vertices.
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-6.0, 0.0, 5.0);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(6.0, 0.0, 5.0);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(6.0, 0.0, -5.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-6.0, 0.0, -5.0);
	glEnd();


	glPushMatrix();
	glTranslatef(1.5, 0.3, 1.5);
	//glRotatef(90, 0.0, 1.0, 0.0);
	// Draw the cylinder
	glBindTexture(GL_TEXTURE_2D, textureName[3]);	// Texture 3 is active now
	myDrawCylinder(radius-0.1, height+0.5);				// Project 5: This already has the needed texture coordinates.  NOTHING TO DO NOW.

									// Undo texture mode and specular highlights once spheres are drawn (so will affect base plane)
	//glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
	//glDisable(GL_TEXTURE_2D);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, PlaneBaseAmbientDiffuse);

	// Draw the left and right caps of the cylinder
	myDrawCylinderCaps(radius-0.1, height+0.5);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.5, 0.3, 3);
	//glRotatef(90, 0.0, 1.0, 0.0);
	// Draw the cylinder
	glBindTexture(GL_TEXTURE_2D, textureName[3]);	// Texture 3 is active now
	myDrawCylinder(radius - 0.1, height + 0.5);				// Project 5: This already has the needed texture coordinates.  NOTHING TO DO NOW.

															// Undo texture mode and specular highlights once spheres are drawn (so will affect base plane)
	//glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
	//glDisable(GL_TEXTURE_2D);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, PlaneBaseAmbientDiffuse);

	// Draw the left and right caps of the cylinder
	myDrawCylinderCaps(radius - 0.1, height + 0.5);
	glPopMatrix();

	// the window 
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, PlaneBaseAmbientDiffuse);
	if (OtherMode == false) {
		glBindTexture(GL_TEXTURE_2D, textureName[14]);	// Texture 0 is active now
		glBegin(GL_QUADS);
		glNormal3f(0.0, 0.0, 1.0);		// Same normal for the next four vertices.
		glTexCoord2f(0.0, 0.0);			// Texture coordinates for the back wall vertex
		glVertex3f(-2.5, 1.8, -4.9);
		glTexCoord2f(1.0, 0.0);			// Texture coordinates for the back wall vertex
		glVertex3f(2.5, 1.8, -4.9);
		glTexCoord2f(1.0, 1.0);			// Texture coordinates for the back wall vertex
		glVertex3f(2.5, 4.5, -4.9);
		glTexCoord2f(0.0, 1.0);			// Texture coordinates for the back wall vertex
		glVertex3f(-2.5, 4.5, -4.9);
		glEnd();
	}

	// my draw door
	//glBindTexture(GL_TEXTURE_2D, textureName[5]);	// Texture 0 is active now
	glBindTexture(GL_TEXTURE_2D, textureName[15]);	// Texture 2 is active now
	glPushMatrix();
	//glTranslatef(4, 1.3, 4.2);		// Sphere slightly repositioned in revived Project 5
	myDrawDoor(singleStep);				// Project 5:  You will need to update the texture coordinates in this function!
	glPopMatrix();

	// Flush the pipeline, swap the buffers
	glFlush();
	glutSwapBuffers();

}

void myDrawDoor(bool singleStep) {
	if (singleStep) {
		// Update the animation state
		HourOfDay += AnimateIncrement;
		HourOfDay = HourOfDay - ((int)(HourOfDay / 24)) * 24;
	}
	// Draw the Earth
	// First position it around the sun
	//		Use DayOfYear to determine its position
	glRotatef(360.0*DayOfYear / 365.0, 0.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);		// Same normal for the next four vertices.
	glTexCoord2f(0.0, 0.0);			// Texture coordinates
	glVertex3f(-6.0, 0.0, 5.0);
	glTexCoord2f(1.0, 0.0);			// Texture coordinates
	glVertex3f(-6.0, 0.0, 2.0);
	glTexCoord2f(1.0, 1.0);			// Texture coordinates
	glVertex3f(-6.0, 5.0, 2.0);
	glTexCoord2f(0.0, 1.0);			// Texture coordinates
	glVertex3f(-6.0, 5.0, 5.0);
	glEnd();

	// Flush the pipeline, and swap the buffers
	//glFlush();
	//glutSwapBuffers();

	if (singleStep) {
		singleStep = false;
	}

	glutPostRedisplay();		// Request a re-draw for animation purposes


}

// Parameter texCoordSpherical controls whether spherical or cylindrical texture coordinates are used.
void myDrawSphere(bool texCoordSpherical)
{
	// For Project 5: Add texture coordinates to this sphere.
	//   The boolean variable texCoordSpherical controls which type of
	//   texture coordinates are used: spherical or cylindrical.
	//   See page 131 in the textbook for the way these are computed.

	// The sphere is drawn with the center of the texture map at the front of the sphere.
	for (int i = 0; i<MeshCount; i++) {
		glBegin(GL_TRIANGLE_STRIP);
		// Draw i-th triangle strip of sphere

		// Draw south pole vertex
		glNormal3f(0.0, -1.0, 0.0);
		glTexCoord2f((i + 0.5f) / (float)MeshCount, 1.0);	// Texture coord's at poles to be discussed in class
		glVertex3f(0.0, -1.0, 0.0);

		// PROJECT 5: MUST ADD TEXTURE COORDINATES FOR VERTICES BELOW
		float thetaLeft = (((float)i) / (float)MeshCount)*2.0f*3.14159f;
		float thetaRight = (((float)((i + 1) % MeshCount)) / (float)MeshCount)*2.0f*3.14159f;
		for (int j = 1; j<MeshCount; j++) {
			float phi = ((((float)j) / (float)MeshCount) - 0.5f)*3.14159f;
			float x = -sinf(thetaRight)*cosf(phi);
			float y = sinf(phi);
			float z = -cosf(thetaRight)*cosf(phi);
			glNormal3f(x, y, z);
			float tempR = thetaRight;
			if (i == MeshCount - 1) {
				thetaRight = 2.0f*3.14159f;
			}
			float s1 = thetaRight / (2.0f*3.14159f);
			float t1 = phi / 3.14159f + 0.5;
			if (texCoordSpherical == false) {
				glTexCoord2f(s1, t1);
			}
			else {
				float t2 = sin(phi) / 3.14159f + 0.5;
				glTexCoord2f(s1, t2);
			}
			thetaRight = tempR;
			glVertex3f(x, y, z);

			x = -sinf(thetaLeft)*cosf(phi);
			z = -cosf(thetaLeft)*cosf(phi);
			glNormal3f(x, y, z);
			float s2 = thetaLeft / (2.0f*3.14159f);
			float tempL = thetaLeft;
			if (i == MeshCount - 1) {
				thetaLeft = 2.0f*3.14159f;
			}
			if (texCoordSpherical == false) {
				glTexCoord2f(s2, t1);
			}
			else {
				float t2 = sin(phi) / 3.14159f + 0.5;
				glTexCoord2f(s2, t2);
			}
			thetaLeft = tempL;
			glVertex3f(x, y, z);
		}

		// Draw north pole vertex
		glNormal3f(0.0, 1.0, 0.0);
		glTexCoord2f((i + 0.5f) / (float)MeshCount, 1.0);	// Texture coord's at poles to be discussed in class
		glVertex3f(0.0, 1.0, 0.0);

		glEnd();		// End of one triangle strip going up the sphere.
	}
}

void myDrawTable(float x, float y, float z, float length, float height, float width) {
	//glBegin(GL_TRIANGLE_STRIP);

	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);		// Same normal for the next four vertices.
	glTexCoord2f(0.0, 0.0);
	glVertex3f(x, y, z+width);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(x+length, y, z + width);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(x + length, y + height, z + width);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(x, y + height, z + width);
	/*
	glVertex3f(0.0, 0.3, 5.0); // take as the original x and y and z
	glVertex3f(4.0, 0.3, 5.0);
	glVertex3f(4.0, 0.7, 5.0);
	glVertex3f(0.0, 0.7, 5.0); // checked*/

	glTexCoord2f(0.0, 0.0);
	glVertex3f(x + length, y, z + width);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(x + length, y, z); 
	glTexCoord2f(1.0, 1.0);
	glVertex3f(x + length, y + height, z);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(x + length, y + height, z + width);
	/*glVertex3f(4.0, 0.3, 5.0);
	glVertex3f(4.0, 0.3, 0.0);
	glVertex3f(4.0, 0.7, 0.0);
	glVertex3f(4.0, 0.7, 5.0);*/

	glTexCoord2f(0.0, 0.0);
	glVertex3f(x + length, y, z);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(x, y, z);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(x, y + height, z);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(x + length, y + height, z);
	/*glVertex3f(4.0, 0.3, 0.0);
	glVertex3f(0.0, 0.3, 0.0);
	glVertex3f(0.0, 0.7, 0.0);
	glVertex3f(4.0, 0.7, 0.0); */
	
	glTexCoord2f(0.0, 0.0);
	glVertex3f(x, y, z);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(x, y, z + width);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(x, y + height, z + width);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(x, y + height, z);
	/*glVertex3f(0.0, 0.3, 0.0);
	glVertex3f(0.0, 0.3, 5.0);
	glVertex3f(0.0, 0.7, 5.0);
	glVertex3f(0.0, 0.7, 0.0);*/

	glTexCoord2f(0.0, 0.0);
	glVertex3f(x, y + height, z);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(x, y + height, z + width);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(x + length, y + height, z + width);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(x + length, y + height, z);
	/*glVertex3f(0.0, 0.7, 0.0);
	glVertex3f(0.0, 0.7, 5.0);
	glVertex3f(4.0, 0.7, 5.0);
	glVertex3f(4.0, 0.7, 0.0);*/

	glTexCoord2f(0.0, 0.0);
	glVertex3f(x, y, z);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(x + length, y, z);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(x + length, y, z + width);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(x, y, z + width);
	/*glVertex3f(0.0, 0.3, 0.0);
	glVertex3f(4.0, 0.3, 0.0);
	glVertex3f(4.0, 0.3, 5.0);
	glVertex3f(0.0, 0.3, 5.0);*/

	glEnd();
}


// Project 5:  You will need to add texture coordinates to drawSurfaceVertex below.
void myDrawSurfaceOfRotation() {
	glScalef(0.8f, 8.0f, 0.8f);
	// Draw triangle at the center
	glBegin(GL_TRIANGLE_FAN);
	drawSurfaceVertex(0, 0);
	for (int j = 0; j <= MeshCount; j++) {
		drawSurfaceVertex(1, j);
	}
	glEnd();

	// Draw circular quad strips
	for (int i = 1; i<MeshCount; i++) {
		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j <= MeshCount; j++) {
			drawSurfaceVertex(i, j);
			drawSurfaceVertex(i + 1, j);
		}
		glEnd();
	}
}

// Parameters to drawSurfaceVertex for "cos(r)/5+r*r)" surface:
// i = 0 ... MeshCount, distance from center
// j = 0 ... MeshCount, rotation around
void drawSurfaceVertex(int i, int j) {
	// PROJECT 5: YOU WILL NEED TO ADD TEXTURE COORDINATES HERE.
	float r = 4.0f*3.1415926f*((float)i) / (float)MeshCount;
	float theta = 2.0f*3.1415926f*((float)(j%MeshCount)) / (float)MeshCount;
	float fPrime = -((5.0f + r*r)*sinf(r) + 2.0f*r*cosf(r)) / (5.0f + r*r);
	float s = r*sinf(theta) / (4.0f*3.1415926f*2.0f) + 0.5f;
	float t = r*cosf(theta) / (4.0f*3.1415926f*2.0f) + 0.5f;
	glNormal3f(-fPrime*sinf(theta), 1.0f, -fPrime*cosf(theta));
	glTexCoord2f(s, t);
	glVertex3f(r*sinf(theta), cosf(r) / (5 + r*r), r*cosf(theta));
}

//float radius = 0.5f;	// Dimensions of the cylinder
//float height = 1.5f;
void myDrawCylinder(float radius, float height)
{
	for (int i = 0; i<MeshCount; i++) {
		float theta1 = 2.0f*3.14159f*((float)i) / (float)MeshCount;
		float theta2 = 2.0f*3.14159f*((float)((i + 1) % MeshCount)) / (float)MeshCount;
		float z1 = radius*cosf(theta1);
		float z2 = radius*cosf(theta2);
		float y1 = radius*sinf(theta1);
		float y2 = radius*sinf(theta2);
		int ii = i;
		float t1 = ((float)((ii) % MeshCount)) / (float)MeshCount;
		float t2 = ((float)((ii + 1))) / (float)MeshCount;
		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j <= MeshCount; j++) {
			float s = ((float)j) / (float)MeshCount;
			float x = -0.5f*height + ((float)j) / (float)MeshCount*height;
			glNormal3f(0.0, y2, z2);
			glTexCoord2f(s, t2);		// The texture coordinates are already here.
			glVertex3f(x, y2, z2);
			glNormal3f(0.0, y1, z1);
			glTexCoord2f(s, t1);		// The texture coordinates are already here.
			glVertex3f(x, y1, z1);
		}
		glEnd();

	}
}


void myDrawTableLeg(float radius, float height) {
	for (int i = 0; i < MeshCount; i++) {
		float theta1 = 2.0f*3.14159f*((float)i) / (float)MeshCount;
		float theta2 = 2.0f*3.14159f*((float)((i + 1) % MeshCount)) / (float)MeshCount;
		float z1 = radius*cosf(theta1);
		float z2 = radius*cosf(theta2);
		float y1 = radius*sinf(theta1);
		float y2 = radius*sinf(theta2);
		int ii = i;
		float t1 = ((float)((ii) % MeshCount)) / (float)MeshCount;
		float t2 = ((float)((ii + 1))) / (float)MeshCount;
		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j <= MeshCount; j++) {
			float s = ((float)j) / (float)MeshCount;
			float x = -0.5f*height + ((float)j) / (float)MeshCount*height;
			glNormal3f(y1, 0.0, z1);
			glTexCoord2f(s, t1);		// The texture coordinates are already here.
			glVertex3f(y1, x, z1);
			glNormal3f(y2, 0.0, z2);
			glTexCoord2f(s, t2);		// The texture coordinates are already here.
			glVertex3f(y2, x, z2);
		}
		glEnd();
	}
}

void myDrawTableLegCaps(float radius, float height) {
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0, -1.0, 0.0);
	glVertex3f(0, height*0.5f, 0);
	for (int i = 0; i <= MeshCount; i++) {
		float theta = 2.0f*3.14159f*((float)(i%MeshCount)) / (float)MeshCount;
		glVertex3f(radius*sinf(theta), height*0.5f, radius*cosf(theta));
	}
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(0, -height*0.5f, 0);
	for (int i = 0; i <= MeshCount; i++) {
		float theta = 2.0f*3.14159f*((float)(i%MeshCount)) / (float)MeshCount;
		glVertex3f(-radius*sinf(theta), -height*0.5f, radius*cosf(theta));
	}
	glEnd();
}

void myDrawCylinderCaps(float radius, float height)
{
	// Draw the two end faces of the cylinder
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(height*0.5f, 0, 0);
	for (int i = 0; i <= MeshCount; i++) {
		float theta = 2.0f*3.14159f*((float)(i%MeshCount)) / (float)MeshCount;
		glVertex3f(height*0.5f, -radius*sinf(theta), radius*cosf(theta));
	}
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(-height*0.5f, 0, 0);
	for (int i = 0; i <= MeshCount; i++) {
		float theta = 2.0f*3.14159f*((float)(i%MeshCount)) / (float)MeshCount;
		glVertex3f(-height*0.5f, radius*sinf(theta), radius*cosf(theta));
	}
	glEnd();
}


// Initialize OpenGL's rendering modes
void initRendering()
{
	glEnable(GL_DEPTH_TEST);	// Depth testing must be turned on

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);		// These two commands will cause backfaces to not be drawn

								// Possibly turn on wireframe mode.
	if (WireFrameOn) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		// Just show wireframes
	}

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, GlobalAmbient);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, SpecularExponent);
	if (Light0On) {
		glEnable(GL_LIGHT0);
	}
	else {
		glDisable(GL_LIGHT0);
	}
	glLightfv(GL_LIGHT0, GL_AMBIENT, Light0AmbientDiffuse);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, Light0AmbientDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, Light0Specular);

	if (Light1On) {
		glEnable(GL_LIGHT1);
	}
	else {
		glDisable(GL_LIGHT1);
	}
	glLightfv(GL_LIGHT1, GL_AMBIENT, Light1AmbientDiffuse);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, Light1AmbientDiffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, Light1Specular);

	if (Light2On) {
		glEnable(GL_LIGHT2);
	}
	else {
		glDisable(GL_LIGHT2);
	}
	glLightfv(GL_LIGHT2, GL_AMBIENT, Light2AmbientDiffuse);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, Light2AmbientDiffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, Light2Specular);

	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
	glShadeModel(GL_SMOOTH);		// Use smooth shading


}

// Called when the window is resized
//		w, h - width and height of the window in pixels.
void resizeWindow(int w, int h)
{
	double aspectRatio;

	// Define the portion of the window used for OpenGL rendering.
	glViewport(0, 0, w, h);	// View port uses whole window

							// Set up the projection view matrix: perspective projection
							// Determine the min and max values for x and y that should appear in the window.
							// The complication is that the aspect ratio of the window may not match the
							//		aspect ratio of the scene we want to view.
	w = (w == 0) ? 1 : w;
	h = (h == 0) ? 1 : h;
	aspectRatio = (double)w / (double)h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(25.0, aspectRatio, 5.0, MaxDistance + 10.0);

}


// Main routine
// Set up OpenGL, define the callbacks and start the main loop
int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	// We're going to animate it, so double buffer 
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// Window position (from top corner), and size (width% and hieght)
	glutInitWindowPosition(10, 60);
	glutInitWindowSize(640, 360);
	glutCreateWindow("Lighted Scene");

	// Initialize OpenGL as we like it..
	initRendering();

	// Set up callback functions for key presses
	glutKeyboardFunc(myKeyboardFunc);			// Handles "normal" ascii symbols
	glutSpecialFunc(mySpecialKeyFunc);		// Handles "special" keyboard keys

											// Set up the callback function for resizing windows
	glutReshapeFunc(resizeWindow);

	// Call this for background processing
	// glutIdleFunc( myIdleFunction );

	// call this whenever window needs redrawing
	glutDisplayFunc(drawScene);

	fprintf(stdout, "Arrow keys control viewpoint.n");
	fprintf(stdout, "Press Page-Up/Page-Down or +/- to move closer to or away from the scene.\n");
	fprintf(stdout, "Press \"b\" to toggle mip-mapping on the brick wall.\n");
	fprintf(stdout, "Press \"w\" to toggle wireframe mode.\n");
	fprintf(stdout, "Press \"M\" or \"m\" to increase or decrease resolution of mushroom cap.\n");
	fprintf(stdout, "Press \"R\" or \"r\" to increase or decrease rate of movement (respectively).\n");
	fprintf(stdout, "Press \"1\", \"2\", \"3\" to toggle the three lights.\n");
	fprintf(stdout, "Press \"ESCAPE\" to exit.\n");


	// Load the three texture maps.
	glGenTextures(NumLoadedTextures, textureName);	// Load three (internal) texture names into array
	for (int i = 0; i<NumLoadedTextures; i++) {
		glBindTexture(GL_TEXTURE_2D, textureName[i]);	// Texture #i is active now
		loadTextureFromFile(filenames[i], true);			// Load texture #i
	}
	//glGenTextures(1, textureName);
	glBindTexture(GL_TEXTURE_2D, textureName[5]);	// Texture #i is active now
	loadTextureFromFile(filenames[5], false);			// Load texture #i
														// Start the main loop.  glutMainLoop never returns.
	glutMainLoop();

	return(0);	// This line is never reached.
}