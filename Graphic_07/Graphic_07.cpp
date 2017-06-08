// Graphic_07.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <GL/glut.h>
#include <stdlib.h>
#include "colors.h"
#include "gltools_extracted.h"
#include <array>
#include <vector>

// Adds two vectors together
void gltAddVectors(const GLTVector3 vFirst, const GLTVector3 vSecond, GLTVector3 vResult) {
	vResult[0] = vFirst[0] + vSecond[0];
	vResult[1] = vFirst[1] + vSecond[1];
	vResult[2] = vFirst[2] + vSecond[2];
}

// Subtract one vector from another
void gltSubtractVectors(const GLTVector3 vFirst, const GLTVector3 vSecond, GLTVector3 vResult)
{
	vResult[0] = vFirst[0] - vSecond[0];
	vResult[1] = vFirst[1] - vSecond[1];
	vResult[2] = vFirst[2] - vSecond[2];
}

// Scales a vector by a scalar
void gltScaleVector(GLTVector3 vVector, const GLfloat fScale)
{
	vVector[0] *= fScale; vVector[1] *= fScale; vVector[2] *= fScale;
}

// Gets the length of a vector squared
GLfloat gltGetVectorLengthSqrd(const GLTVector3 vVector)
{
	return (vVector[0] * vVector[0]) + (vVector[1] * vVector[1]) + (vVector[2] * vVector[2]);
}

// Gets the length of a vector
GLfloat gltGetVectorLength(const GLTVector3 vVector)
{
	return (GLfloat)sqrt(gltGetVectorLengthSqrd(vVector));
}

// Scales a vector by it's length - creates a unit vector
void gltNormalizeVector(GLTVector3 vNormal)
{
	GLfloat fLength = 1.0f / gltGetVectorLength(vNormal);
	gltScaleVector(vNormal, fLength);
}

// Copies a vector
void gltCopyVector(const GLTVector3 vSource, GLTVector3 vDest)
{
	memcpy(vDest, vSource, sizeof(GLTVector3));
}

// Get the dot product between two vectors
GLfloat gltVectorDotProduct(const GLTVector3 vU, const GLTVector3 vV)
{
	return vU[0] * vV[0] + vU[1] * vV[1] + vU[2] * vV[2];
}

// Calculate the cross product of two vectors
void gltVectorCrossProduct(const GLTVector3 vU, const GLTVector3 vV, GLTVector3 vResult)
{
	vResult[0] = vU[1] * vV[2] - vV[1] * vU[2];
	vResult[1] = -vU[0] * vV[2] + vV[0] * vU[2];
	vResult[2] = vU[0] * vV[1] - vV[0] * vU[1];
}



// Given three points on a plane in counter clockwise order, calculate the unit normal
void gltGetNormalVector(const GLTVector3 vP1, const GLTVector3 vP2, const GLTVector3 vP3, GLTVector3 vNormal)
{
	GLTVector3 vV1, vV2;

	gltSubtractVectors(vP2, vP1, vV1);
	gltSubtractVectors(vP3, vP1, vV2);

	gltVectorCrossProduct(vV1, vV2, vNormal);
	gltNormalizeVector(vNormal);
}



// Transform a point by a 4x4 matrix
void gltTransformPoint(const GLTVector3 vSrcVector, const GLTMatrix mMatrix, GLTVector3 vOut)
{
	vOut[0] = mMatrix[0] * vSrcVector[0] + mMatrix[4] * vSrcVector[1] + mMatrix[8] * vSrcVector[2] + mMatrix[12];
	vOut[1] = mMatrix[1] * vSrcVector[0] + mMatrix[5] * vSrcVector[1] + mMatrix[9] * vSrcVector[2] + mMatrix[13];
	vOut[2] = mMatrix[2] * vSrcVector[0] + mMatrix[6] * vSrcVector[1] + mMatrix[10] * vSrcVector[2] + mMatrix[14];
}

// Rotates a vector using a 4x4 matrix. Translation column is ignored
void gltRotateVector(const GLTVector3 vSrcVector, const GLTMatrix mMatrix, GLTVector3 vOut)
{
	vOut[0] = mMatrix[0] * vSrcVector[0] + mMatrix[4] * vSrcVector[1] + mMatrix[8] * vSrcVector[2];
	vOut[1] = mMatrix[1] * vSrcVector[0] + mMatrix[5] * vSrcVector[1] + mMatrix[9] * vSrcVector[2];
	vOut[2] = mMatrix[2] * vSrcVector[0] + mMatrix[6] * vSrcVector[1] + mMatrix[10] * vSrcVector[2];
}


// Gets the three coefficients of a plane equation given three points on the plane.
void gltGetPlaneEquation(GLTVector3 vPoint1, GLTVector3 vPoint2, GLTVector3 vPoint3, GLTVector3 vPlane)
{
	// Get normal vector from three points. The normal vector is the first three coefficients
	// to the plane equation...
	gltGetNormalVector(vPoint1, vPoint2, vPoint3, vPlane);

	// Final coefficient found by back substitution
	vPlane[3] = -(vPlane[0] * vPoint3[0] + vPlane[1] * vPoint3[1] + vPlane[2] * vPoint3[2]);
}

// Determine the distance of a point from a plane, given the point and the
// equation of the plane.
GLfloat gltDistanceToPlane(GLTVector3 vPoint, GLTVector4 vPlane)
{
	return vPoint[0] * vPlane[0] + vPoint[1] * vPlane[1] + vPoint[2] * vPlane[2] + vPlane[3];
}

const GLint N = 10;

// rozmiary bryły obcinania
const GLdouble left = -2.0;
const GLdouble right = 2.0;
const GLdouble bottom = -2.0;
const GLdouble top = 2.0;
const GLdouble near_ = 2.0;
const GLdouble far_ = 7.0;
// stałe do obsługi menu podręcznego
enum
{
	CUTTING_PLANE, // płaszczyzna przekroju
	POLYGON_OFFSET, // przesunięcie wartości głębi
	EXIT // wyjście
};
// kąt obrotu kuli
GLfloat angle = 0.0;
// kąty obrotu sześcianu
GLfloat rotatex = 0.0;
GLfloat rotatey = 0.0;
// wskaŸnik rysowania płaszczyzna przekroju
bool cutting_plane = true;
// wskaŸnik przesunięcia wartości głębi
bool polygon_offset = true;
// wskaŸnik naciśnięcia lewego przycisku myszki
int button_state = GLUT_UP;
// położenie kursora myszki
int button_x, button_y;


void drawPyramid(GLuint n, GLfloat size, GLenum type, bool drawBase)
{
	GLTVector3 topVertice{ 0.0f, +size, 0.0f };
	GLTVector3 baseCenter{ 0.0f, -size / 2.0, 0.0f };

	std::vector<GLTVector3> bottomVertices{ n };

	for (int i = 0; i < n; ++i)
	{
		const GLfloat angle = (GLfloat)i / n * 3.1415926 * 2.0;

		bottomVertices[i][0] = size * sin(angle);
		bottomVertices[i][1] = -size / 2;
		bottomVertices[i][2] = size * cos(angle);
	}

	if (drawBase)
	{
		for (int i = 0; i <= n; ++i)
		{
			glBegin(type);
			glNormal3f(0.0f, -1.0f, 0.0f);
			glVertex3fv(baseCenter);
			glVertex3fv(bottomVertices[(i + 1) % n]);
			glVertex3fv(bottomVertices[i % n]);
			glEnd();
		}
	}
	for (int i = 0; i <= n; ++i)
	{
		glBegin(type);
		GLTVector3 normal;
		gltGetNormalVector(bottomVertices[i % n], bottomVertices[(i + 1) % n], topVertice, normal);
		glNormal3fv(normal);

		glVertex3fv(bottomVertices[i % n]);
		glVertex3fv(bottomVertices[(i + 1) % n]);
		glVertex3fv(topVertice);

		glEnd();
	}
}

void drawSolidPyramid(GLint n, GLdouble size)
{
	drawPyramid(n, size, GL_TRIANGLES, true);
}

void drawWirePyramid(GLint n, GLdouble size)
{
	drawPyramid(n, size, GL_LINE_LOOP, false);
}

// funkcja generująca scenę 3D
void Display()
{
	// kolor tła - zawartośæ bufora koloru
	glClearColor(1.0, 1.0, 1.0, 1.0);
	// czyszczenie bufora koloru i bufora głębokości
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// wybór macierzy modelowania
	glMatrixMode(GL_MODELVIEW);
	// macierz modelowania = macierz jednostkowa
	glLoadIdentity();
	// przesunięcie układu współrzędnych sześcianu do środka bryły odcinania
	glTranslatef(0, 0, -(near_ + far_) / 2);
	// obroty sześcianu
	glRotatef(rotatex, 1.0, 0, 0);
	glRotatef(rotatey, 0, 1.0, 0);
	// niewielkie powiększenie sześcianu
	//glScalef(1.15, 1.15, 1.15);
	// włączenie testu bufora głębokości
	glEnable(GL_DEPTH_TEST);
	// najpierw rysujemy kulę obracającą się wewnątrz sześcianu;
	// z uwagi na celowy brak efektów oświetlenia, obrót kuli
	// podkreśla druga kula w wersji "szkieletowej"
	glPushMatrix();
	angle += 0.2;
	glRotatef(angle, 1.0, 1.0, 0.0);
	glColor3fv(Yellow);
	if (polygon_offset)
		glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1.0);
	glutSolidSphere(0.5, 10, 10);
	glColor3fv(Black);
	glutWireSphere(0.5, 10, 10);
	if (polygon_offset)
		glDisable(GL_POLYGON_OFFSET_FILL);
	glPopMatrix();


	const GLfloat a = 0.5;

	// w drugiej kolejności rysujemy wnętrze sześcianu;
	// rysowane są tylko przednie strony wewnętrznych ścian
	// no i nie jest rysowana ściana przednia, w której będzie otwór
	glEnable(GL_CULL_FACE);
	glBegin(GL_QUADS);
	glColor3fv(Blue);
	glVertex3f(-a, -2 * a, -a);
	glVertex3f(a, -2 * a, -a);
	glVertex3f(a, a, -a);
	glVertex3f(-a, a, -a);
	glColor3fv(Magenta);
	glVertex3f(-a, a, a);
	glVertex3f(a, a, a);
	glVertex3f(a, -2 * a, a);
	glVertex3f(-a, -2 * a, a);
	glColor3fv(Lime);
	glVertex3f(-a, a, a);
	glVertex3f(-a, -2 * a, a);
	glVertex3f(-a, -2 * a, -a);
	glVertex3f(-a, a, -a);
	glColor3fv(Cyan);
	glVertex3f(a, a, a);
	glVertex3f(-a, a, a);
	glVertex3f(-a, a, -a);
	glVertex3f(a, a, -a);
	glColor3fv(Green);
	glVertex3f(a, -2 * a, a);
	glVertex3f(a, a, a);
	glVertex3f(a, a, -a);
	glVertex3f(a, -2 * a, -a);
	glColor3fv(Cyan);
	glVertex3f(-a, -2 * a, a);
	glVertex3f(a, -2 * a, a);
	glVertex3f(a, -2 * a, -a);
	glVertex3f(-a, -2 * a, -a);
	glEnd();
	glDisable(GL_CULL_FACE);
	// rysowanie płaszczyzny otworu w sześcianie
	if (cutting_plane)
	{
		// wyłączenie rysowania w buforze kolorów
		glDrawBuffer(GL_NONE);
		// rysowanie kwadratu częściowo odsłaniającego wnętrze sześcianu
		// (kwadrat jest położony o 0,001 jednostki nad bokiem sześcianu)
		glBegin(GL_QUADS);
		glVertex3f(-0.5, -1.001, -0.5);
		glVertex3f(0.5, -1.001, -0.5);
		glVertex3f(0.5, -1.001, 0.5);
		glVertex3f(-0.5, -1.001, 0.5);
		glEnd();
		// włączenie rysowania w buforze kolorów
		glDrawBuffer(GL_BACK);
	}
	// właściwy sześcian z obramowaniem, którego rysowanie wymusza brak oświetlenia
	glColor3fv(Red);
	if (polygon_offset)
		glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(1.0, 1.0);
	drawSolidPyramid(N, 2.0);
	glColor3fv(Black);
	drawWirePyramid(N, 2.0);
	if (polygon_offset)
		glDisable(GL_POLYGON_OFFSET_FILL);
	// skierowanie poleceñ do wykonania
	glFlush();
	// zamiana buforów koloru
	glutSwapBuffers();
}
// zmiana wielkości okna
void Reshape(int width, int height)
{
	// obszar renderingu - całe okno
	glViewport(0, 0, width, height);
	// wybór macierzy rzutowania
	glMatrixMode(GL_PROJECTION);
	// macierz rzutowania = macierz jednostkowa
	glLoadIdentity();
	// parametry bryły obcinania
	glFrustum(left, right, bottom, top, near_, far_);
	// generowanie sceny 3D
	Display();
}
// obsługa klawiszy funkcyjnych i klawiszy kursora
void SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
		// kursor w lewo
	case GLUT_KEY_LEFT:
		rotatey -= 1;
		break;
		// kursor w górę
	case GLUT_KEY_UP:
		rotatex -= 1;
		break;
		// kursor w prawo
	case GLUT_KEY_RIGHT:
		rotatey += 1;
		break;
		// kursor w dół
	case GLUT_KEY_DOWN:
		rotatex += 1;
		break;
	}
	// odrysowanie okna
	Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}
// obsługa przycisków myszki
void MouseButton(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		// zapamiętanie stanu lewego przycisku myszki
		button_state = state;
		// zapamiętanie położenia kursora myszki
		if (state == GLUT_DOWN)
		{
			button_x = x;
			button_y = y;
		}
	}
}
// obsługa ruchu kursora myszki
void MouseMotion(int x, int y)
{
	if (button_state == GLUT_DOWN)
	{
		rotatey += 30 * (right - left) / glutGet(GLUT_WINDOW_WIDTH) *(x - button_x);
		button_x = x;
		rotatex -= 30 * (top - bottom) / glutGet(GLUT_WINDOW_HEIGHT) *(button_y - y);
		button_y = y;
		glutPostRedisplay();
	}
}
// obsługa menu podręcznego
void Menu(int value)
{
	switch (value)
	{
		// płaszczyzna przekroju
	case CUTTING_PLANE:
		cutting_plane = !cutting_plane;
		Display();
		break;
		// przesunięcie wartości głębi
	case POLYGON_OFFSET:
		polygon_offset = !polygon_offset;
		Display();
		break;
		// wyjście
	case EXIT:
		exit(0);
	}
}
int main(int argc, char * argv[])
{
	// inicjalizacja biblioteki GLUT
	glutInit(&argc, argv);
	// inicjalizacja bufora ramki
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	// rozmiary głównego okna programu
	glutInitWindowSize(500, 500);
	// utworzenie głównego okna programu
	glutCreateWindow("Z-bufor");
	// dołączenie funkcji generującej scenę 3D
	glutDisplayFunc(Display);
	// dołączenie funkcji wywoływanej przy zmianie rozmiaru okna
	glutReshapeFunc(Reshape);
	// dołączenie funkcji obsługi klawiszy funkcyjnych i klawiszy kursora
	glutSpecialFunc(SpecialKeys);
	// obsługa przycisków myszki
	glutMouseFunc(MouseButton);
	// obsługa ruchu kursora myszki
	glutMotionFunc(MouseMotion);
	// utworzenie menu podręcznego
	glutCreateMenu(Menu);
	// menu główne
	glutCreateMenu(Menu);
#ifdef WIN32
	glutAddMenuEntry("Płaszczyzna przekroju: rysowana/nierysowana", CUTTING_PLANE);
	glutAddMenuEntry("Przesunięcie wartości głębi: włącz/wyłącz", POLYGON_OFFSET);
	glutAddMenuEntry("Wyjście", EXIT);
#else
	glutAddMenuEntry("Plaszczyzna przekroju: rysowana/nierysowana", CUTTING_PLANE);
	glutAddMenuEntry("Przesuniecie wartosci glebi: wlacz/wylacz", POLYGON_OFFSET);
	glutAddMenuEntry("Wyjscie", EXIT);
#endif
	// określenie przycisku myszki obsługującej menu podręczne
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	// dołączenie funkcji wywoływanej podczas "bezczynności" systemu
	glutIdleFunc(Display);
	// wprowadzenie programu do obsługi pętli komunikatów
	glutMainLoop();
	return 0;
}