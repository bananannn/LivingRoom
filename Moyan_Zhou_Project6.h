/*
* WireFrameScene.h
*
* Author: Samuel R. Buss
*
* Bug reports: Sam Buss, sbuss@ucsd.edu.
*
* Base code for homework project #3, CSE 167, Fall 2003
*/

// Function prototypes for WireFrameScene.cpp

void myKeyboardFunc(unsigned char key, int x, int y);
void mySpecialKeyFunc(int key, int x, int y);

void drawScene(void);

void initRendering();
void resizeWindow(int w, int h);
void MoveCloser();
void MoveAway();

void myDrawSphere(bool texCoordSpherical);
void myDrawCylinder(float radius, float height);
void myDrawCylinderCaps(float radius, float height);
void myDrawSurfaceOfRotation();
void drawSurfaceVertex(int i, int j);
void myDrawTable(float x, float y, float z, float length, float height, float width);
void myDrawTableLegCaps(float radius, float height);
void myDrawTableLeg(float radius, float height);
void myDrawDoor(bool singleStep);