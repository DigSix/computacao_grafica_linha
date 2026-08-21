#include <iostream>
#include <GL/freeglut.h>
#include <cmath>



#define PI 3.14159

#define WWIDTH  500
#define WHEIGHT 500


void display();
void keyboard(unsigned char key, int x, int y);
double degrees_to_radians(double degrees);

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	
	glutInitWindowSize(WWIDTH, WHEIGHT);

	glutCreateWindow("Criando poligonos");
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glOrtho(0, WWIDTH, 0, WHEIGHT, -1, 1);

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);

	glutMainLoop();

	return 0;
}


void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_LINES);

	int    sides  = 5;
	float  radius = 30;
	float  x   	  = WWIDTH  / 2 - radius / 2;
	float  y   	  = WHEIGHT / 2 - radius / 2;
	float  angle  = 360.0f / sides;
	double de     = 0 ;

	for(int i = 0; i < sides; i++){

		glColor3f(1.0, 0.0, 1.0);

		glVertex2f(x, y);

		x  += radius * std::cos(degrees_to_radians(de));
		y  += radius * std::sin(degrees_to_radians(de));
		de += angle;
		
		glVertex2f(x, y);

	}

	glEnd();

	glFlush();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:                                         
		exit(0);
		break;
	}
}


double degrees_to_radians(double degrees) {
	return degrees * (PI / 180.0);
}