// Scott Wiedemann
// CSCI 441
// 11/11/2009

#include <math.h>
#include <stdlib.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <assert.h>
#include <string>
#include <vector>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

using namespace std;

#define VPD_MIN 200
#define VPD_DEFAULT 800
#define VPD_MAX 1024

#define MORE_SITES 1
#define FEWER_SITES 2
#define TOGGLE_SITES 3
#define RESET 4
#define MOVE_POINTS 5

#define GL_GLEXT_PROTOTYPES

// GLUT window id
GLint wid;
// (square) viewport dimensions
GLint vpd = VPD_DEFAULT;

// display list ID
GLuint sceneID;

// show the sites?
bool showsites = false;

// rotation
bool rotate = false;

// resolution of image
int x_resolution, y_resolution;

class rgb {
	public:
		unsigned char r,g,b;
};

// pixels in the image
rgb *image_pixels;

double fRand(double fMin, double fMax) {
	double f = (double)rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);
}

const unsigned int INITAL_NUMBER_OF_SITES = 128;

class Site {
	public:
		// position between -1 and 1 on the grid
		double x,y;
		// speed of the site
		double velocity;
		double currentangle;
		double r,g,b;

		Site() {
			x = fRand(-1,1);
			y = fRand(-1,1);
			velocity=fRand(-0.5,0.5);
			r = fRand(0,1);
			g = fRand(0,1);
			b = fRand(0,1);
		}
};

vector<Site> sites;

void addSites(unsigned int numberOfSitesToAdd) {
	for(unsigned int i=0; i<numberOfSitesToAdd; i++) {
		sites.push_back(Site());
	}
}

void removeSites(unsigned int numberOfSitesToRemove) {
	for(unsigned int i=0; i<numberOfSitesToRemove && sites.size() > 0; i++) {
		sites.pop_back();
	}
}

void draw_scene() {
	for(unsigned int i=0; i<sites.size(); i++) {
		glLoadIdentity();

		if(rotate) {
			sites[i].currentangle += sites[i].velocity;
		}
		glRotated(sites[i].currentangle, 0, 0, 1);
		glTranslated(sites[i].x, sites[i].y, 0);

		glBegin(GL_TRIANGLE_FAN);
		double ix = floor(((sites[i].x+1.0)/2.0)*x_resolution);
		double iy = floor(((sites[i].y+1.0)/2.0)*y_resolution);
		ix = min(max(ix, 0.0), x_resolution-1.0);
		iy = min(max(iy, 0.0), y_resolution-1.0);

		int imageIndex = ix+(y_resolution-1-iy)*x_resolution;
		rgb rgbAtxy = image_pixels[imageIndex];
		glColor3d(
			(double)rgbAtxy.r/255.0,
			(double)rgbAtxy.g/255.0,
			(double)rgbAtxy.b/255.0);
			//glColor3d(sites[i].r,sites[i].g,sites[i].b);

		glVertex3d(0,0,0);
		int res=35;
		for(int j=0; j<=res; j++) {
			double alpha = (j*360.0/res)*(M_PI / 180.0);
			glVertex4d(cos(alpha), sin(alpha), 1, 0);
		}
		glEnd();

		if(showsites) {
			glLoadIdentity();
			glRotated(sites[i].currentangle, 0, 0, 1);
			glBegin(GL_POINTS);
				glColor3d(0,0,0);
				glVertex3d(sites[i].x,sites[i].y,-0.5);
			glEnd();
		}

		if(rotate) {
			glutPostRedisplay();
		}
	}
}

void draw(void) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1,-1,1,1,1,-3);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* ensure we're drawing to the correct GLUT window */
	glutSetWindow(wid);

	/* clear the color buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* DRAW WHAT IS IN THE DISPLAY LIST */
	draw_scene();

	/* flush the pipeline */
	glFlush();

	/* look at our handiwork */
	glutSwapBuffers();
}

void reset() {
	showsites = false;
	rotate = false;
	sites.clear();
	vector<Site>().swap(sites);
	addSites(INITAL_NUMBER_OF_SITES);
}

void keyboard(GLubyte key, GLint x, GLint y) {
	switch(key) {
		case 27:
			exit(0);
		default:
			break;
	}
	glutPostRedisplay();
}

void menu(int value) {
	switch(value) {
		case MORE_SITES:
			addSites(sites.size());
			break;
		case FEWER_SITES:
			removeSites(sites.size()/2);
			break;
		case TOGGLE_SITES:
			showsites=!showsites;
			break;
		case RESET:
			reset();
			break;
		case MOVE_POINTS:
			rotate=!rotate;
			break;
		default:
			break;
	}
	glutPostRedisplay();
}

/* handle resizing the glut window */

void reshape(GLint vpw, GLint vph) {
	glutSetWindow(wid);

	/* maintain a square viewport, not too small, not too big */
	if( vpw < vph ) {
		vpd = vph;
	} else {
		vpd = vpw;
	}

	if( vpd < VPD_MIN ) vpd = VPD_MIN;
	if( vpd > VPD_MAX ) vpd = VPD_MAX;

	glViewport(0, 0, vpd, vpd);
	glutReshapeWindow(vpd, vpd);

	glutPostRedisplay();
}

GLint init_glut(GLint *argc, char **argv) {
	GLint id;

	glutInit(argc,argv);

	/* size and placement hints to the window system */
	glutInitWindowSize(vpd, vpd);
	glutInitWindowPosition(10,10);

	/* double buffered, RGB color mode */
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	/* create a GLUT window (not drawn until glutMainLoop() is entered) */
	id = glutCreateWindow("Voronoi Diagram Mosaics");

	/* window size changes */
	glutReshapeFunc(reshape);

	/* keypress handling when the current window has input focus */
	glutKeyboardFunc(keyboard);

	/* window obscured/revealed event handler */
	glutVisibilityFunc(NULL);

	/* handling of keyboard SHIFT, ALT, CTRL keys */
	glutSpecialFunc(NULL);

	/* what to do when mouse cursor enters/exits the current window */
	glutEntryFunc(NULL);

	/* what to do on each display loop iteration */
	glutDisplayFunc(draw);

	/* create menu */
	GLint menuID = glutCreateMenu(menu);

	glutAddMenuEntry("Double number of sites", MORE_SITES);
	glutAddMenuEntry("Half number of sites", FEWER_SITES);
	glutAddMenuEntry("Toggle sites", TOGGLE_SITES);
	glutAddMenuEntry("Reset", RESET);
	glutAddMenuEntry("Toggle movement", MOVE_POINTS);
	glutSetMenu(menuID);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	return id;
}

void init_opengl(void) {
	/* automatically scale normals to unit length after transformation */
	glEnable(GL_NORMALIZE);

	/* clear to white */
	glClearColor(1.0, 1.0, 1.0, 1.0);

	/* Enable depth test */
	glEnable(GL_DEPTH_TEST);
}

void readFile(string Filename) {
	ifstream ifs(Filename.c_str());
	if(!ifs) {
		cerr << "ERROR: Can't open file: " << Filename << endl;
		exit(1);
	}

	char c;
	ifs >> c;
	assert(c=='P');
	ifs >> c;
	assert(c=='6');	 // binary PPMs start with the magic code `P6'
	ifs >> x_resolution >> y_resolution;
	int i;
	ifs >> i;
	assert(i==255);	// all images we'll use have 255 color levels
	ifs.get(); // need to skip one more byte

	image_pixels = new rgb[x_resolution*y_resolution];
	ifs.read((char*)image_pixels,x_resolution*y_resolution*sizeof(rgb));
}

int main(GLint argc, char **argv) {
	string inputFileName;
	if(argc != 2) {
		cerr << "ERROR: No imput file." << endl;
		cerr << "Usage: " << argv[0] << " <input.ppm>" << endl;
		exit(1);
	} else {
		inputFileName = argv[1];
	}

	readFile(inputFileName);

	srand(time(0));
	addSites(INITAL_NUMBER_OF_SITES);
	/* initialize GLUT: register callbacks, etc */
	wid = init_glut(&argc, argv);
	/* any OpenGL state initialization we need to do */
	init_opengl();
	glutMainLoop();
	return 0;
}
