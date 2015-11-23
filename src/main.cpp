#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <string>
#include <cmath>

#include "shaders.h"
#include "textfile.h"
#include "shader/ShaderManager.h"
#include "noise/Perlin.h"
#include "sky/SkyDome.h"
#include "camera/Camera.h"

#define USE_PERLIN 0
#define DEBUG_CAMERA 0

noise::Perlin * perlin_noise;
sky::SkyDome * skyDome;
Camera * camera;

bool camera_update_pending = false;
bool mouse_left_click = false;

GLfloat delta_vertical_angle = 0.0;
GLfloat delta_horizontal_angle = 0.0;

GLfloat delta_front_back = 0.0;
GLfloat delta_left_right = 0.0;

GLfloat prev_mouse_x = 0.0;
GLfloat prev_mouse_y = 0.0;

GLint loc_time;
GLint loc_octaves;
GLuint v, f, f2, p;

GLfloat dx = 0.0, dy = 0.0, dz = 0.0;

float lpos[4] = { 1.0, 0.0, 1.0, 0.0 };

void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;

	float ratio = 1.0 * w / h;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45, ratio, 1, 100);
	glMatrixMode(GL_MODELVIEW);
}

float elapsed_time = 1;

void renderScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	if (camera_update_pending) {
		camera_update_pending = false;

		camera->rotate(delta_horizontal_angle, delta_vertical_angle);
		camera->move(delta_front_back, delta_left_right);

		delta_horizontal_angle = 0.0;
		delta_vertical_angle = 0.0;
		delta_front_back = 0.0;
		delta_left_right = 0.0;
	}

	camera->update(elapsed_time);

	glLightfv(GL_LIGHT0, GL_POSITION, lpos);
	glRotatef(elapsed_time, 0, 1, 0);

	elapsed_time += 0.001;

	if (USE_PERLIN) {
		perlin_noise->update(elapsed_time);
	} else {
		skyDome->update(elapsed_time);
	}

	glutSwapBuffers();
}

void processNormalKeys(unsigned char key, int x, int y) {

	if (key == 27)
		exit(0);
}

#define printOpenGLError() printOglError(__FILE__, __LINE__)

int printOglError(char *file, int line) {
	//
	// Returns 1 if an OpenGL error occurred, 0 otherwise.
	//
	GLenum glErr;
	int retCode = 0;

	glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		std::cout << "glError in file " << file << " @ line" << line << ":"
				<< gluErrorString(glErr) << std::endl;
		retCode = 1;
		glErr = glGetError();
	}
	return retCode;
}

void mouseClickEvent(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			mouse_left_click = true;

			prev_mouse_x = x;
			prev_mouse_y = y;
		}
		else  {
			mouse_left_click = false;
		}
	}
}

void mouseMoveEvent(int x, int y) {
	if (mouse_left_click) {
		delta_horizontal_angle = (prev_mouse_x - x) * 320 / 360;
		delta_vertical_angle = (prev_mouse_y - y) * 320 / 360;

		prev_mouse_x = x;
		prev_mouse_y = y;

		camera_update_pending = true;
	}
}

void printShaderInfoLog(GLuint obj) {
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0) {
		infoLog = (char *) malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		std::cout << infoLog;

		free(infoLog);
	}
}

void printProgramInfoLog(GLuint obj) {
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0) {
		infoLog = (char *) malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		std::cout << infoLog;

		free(infoLog);
	}
}

void specialKeyFuncton(int key, int x, int y) {

	switch (key) {
		case GLUT_KEY_UP:
			delta_front_back = 1;
			camera_update_pending = true;
			break;
		case GLUT_KEY_DOWN:
			delta_front_back = -1;
			camera_update_pending = true;
			break;
		case GLUT_KEY_LEFT:
			delta_left_right = 1;
			camera_update_pending = true;
			break;
		case GLUT_KEY_RIGHT:
			delta_left_right = -1;
			camera_update_pending = true;
			break;
	}
}

void showPerlin() {

	if (USE_PERLIN) {
		perlin_noise->apply_to(skyDome);
		perlin_noise->show();
	} else {
		skyDome->show();
	}
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Procedural Cloud Generator");

	perlin_noise = new noise::Perlin(20);
	skyDome = new sky::SkyDome(1.5, 10);
	camera = new Camera(0, 0, -5, 90, 90);

	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(specialKeyFuncton);

	glutMouseFunc(mouseClickEvent);
	glutMotionFunc(mouseMoveEvent);

	glutSetCursor(GLUT_CURSOR_NONE);

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	//glEnable(GL_CULL_FACE);

	glewInit();
	if (glewIsSupported("GL_VERSION_2_0"))
		std::cout << "Ready for OpenGL 2.0\n";
	else {
		std::cout << "OpenGL 2.0 not supported!\n";
		exit(1);
	}

	showPerlin();

	glutMainLoop();

	return 0;
}

