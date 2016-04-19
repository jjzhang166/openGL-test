#if defined(__APPLE__)
	#include <OpenGL/OpenGL.h>
	#include <GLUT/glut.h>
#else
	#include <GL/gl.h>
	#include <GL/freeglut.h>
	#include <GL/glu.h>
#endif

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "model.h"
using namespace std;

// es normal que quan es desactiven totes les llums encara es vegi una mica el contorn dels objectes?

#define INC 0.01
#define MOV 0.05
#define PI 3.14159265358979

void initGL();
void refresh(void);
void resize(int w, int h);
void move(int x, int y);
void click(int a, int b, int x, int y);
void key(unsigned char a, int x, int y);
void pinta_model(int num_model);
void pinta_terra();
void pinta_ninot();
void calcCapsa();
void tipus_cam();
void pos_cam();

vector<double> colours(4, 0.0);
vector<double> vrp(3, 0.0);
vector<double> v_capsa(3, 0.0);

vector<double> obs(3, 0.0);
vector<double> center(3, 1.0);


int state = 1;
// 	order controls the transformation to be done and the order
int order = 0;
// 	col controls the current colour to be modified
int col = 1;
// 	fade controls if the colour must get brighter or darker
int fade = 0;

//  aux_x and aux_y help control to check if a mouse action is a click or a movement
int aux_x = 0;
int aux_y = 0;
// win_x, win_y control the size of the window in pixels
int win_x = 600;
int win_y = 600;

// model
string filename = "Patricio.obj";
Model m;
// capsa contenidora del model
double xmin, ymin, zmin;
double xmax, ymax, zmax;
// moviment del model en el pla xz
double modelx, modelz;

// capsa contenidora de l'escena
double escena_xmin, escena_ymin, escena_zmin;
double escena_xmax, escena_ymax, escena_zmax;
// esfera contenidora
double radi;
// tipus de camera: 0 --> axonometrica, 1 --> perspectiva
// walk: 0 --> 3a persona, 1 --> 1a persona
int tipus, walk;
// valors de camera axonometrica
double xleft, xright, ybottom, ytop, zNear, zFar;
// valors de la camera perspectiva
double fovy, aspect, dist, alpha_v;
double prev_fovy, prev_zNear, prev_zFar;
int prev_tipus;
// valors de la posició de la camera
double anglex, angley, anglez;
double zoom;

// parets visibles
int visibilitat;
// velocitat del moviment
int vel;
// direcció a la que apunta el patricio
double dir = 270.0;

// ILUMINACIO
int iluminacio = 1;
int mat_index = -1;
int normals_per_v = 1;
Material aux;
// llums
int llum0, llum1, llum2;
// posicions llum0
int pos_llum0;
vector<float *> posicions_llum0(5);
float pos0[] = {5.0, 1.5, 5.0, 1.0};
float pos1[] = {5.0, 1.5, -5.0, 1.0};
float pos2[] = {-5.0, 1.5, -5.0, 1.0};
float pos3[] = {-5.0, 1.5, 5.0, 1.0};
float pos4[] = {2.5, 1.5, 2.5, 1.0};
// posicio llum1
float pos_llum1[] = {0.0, 5.0, 0.0, 1.0};

// model 2
Model m2;
double xmin2, ymin2, zmin2;
double xmax2, ymax2, zmax2;


void calc_ambient()
{
	aux.ambient[0] = aux.diffuse[0] * 0.25;
	aux.ambient[1] = aux.diffuse[1] * 0.25;
	aux.ambient[2] = aux.diffuse[2] * 0.25;
	aux.ambient[3] = 1.0;
}

void esfera_contenidora()
{
	// escena amb el terra, el ninot i el legoman a escala
	// centre de l'escena
	vrp[0] = vrp[1] = vrp[2] = 0;
	// vrp[1] = 1.2 / 2;
	// vertex de la capsa contenidora de tota l'escena mes allunyat de vrp
	if (vrp[0] - escena_xmin > vrp[0] - escena_xmax) v_capsa[0] = escena_xmin;
	else v_capsa[0] = escena_xmax;
	if (vrp[1] - escena_ymin > vrp[1] - escena_ymax) v_capsa[1] = escena_ymin;
	else v_capsa[1] = escena_ymax;
	if (vrp[2] - escena_zmin > vrp[2] - escena_zmax) v_capsa[2] = escena_zmin;
	else v_capsa[2] = escena_zmax;

	radi = sqrt(pow(v_capsa[0]-vrp[0], 2) + pow(v_capsa[1]-vrp[1], 2) + pow(v_capsa[2]-vrp[2], 2));
}

void tipus_cam(int tipus)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (tipus == 0) {
		// camera axonometrica
		glOrtho(xleft, xright, ybottom, ytop, zNear, zFar);
	} else if (tipus == 1) {
		// camera perspectiva
		gluPerspective(fovy, aspect, zNear, zFar);
	}
	glMatrixMode(GL_MODELVIEW);
}

void pos_cam(int tipus)
{
	double aux;
	if (tipus == 0) aux = -radi;
	else aux = -dist;
	aux += zoom;
	glLoadIdentity();
	if (walk) {
		center[0] = modelx + 5 * sin(dir * PI / 180.0);
		center[2] = modelz + 5 * cos(dir * PI / 180.0);
		gluLookAt(modelx, 1.0, modelz, center[0], center[1], center[2], 0, 1, 0);
	} else {
		glTranslated(0.0, 0.0, aux);
		glRotated(-anglez, 0, 0, 1);
		glRotated(anglex, 1, 0, 0);
		glRotated(-angley, 0, 1, 0);
		glTranslated(-vrp[0], -vrp[1], -vrp[2]);
	}
}

void inicialitzacions()
{
	anglez = 0.0;
	angley = anglex = 45.0;
	escena_xmin = escena_zmin = -5.0;
	escena_xmax = escena_zmax = 5.0;
	escena_ymin = 0.0;
	escena_ymax = 1.5;
	esfera_contenidora();	
	// inicialitzacions camera axonometrica
	xleft = ybottom = -radi;
	xright = ytop = radi;
	// inicialitzacions camera perspectiva
	dist = 2 * radi;
	alpha_v = asin(radi / dist);
	fovy = 2 * (alpha_v * 180 / PI);
	aspect = 1;
}

void parametres_camera(int tipus)
{
	if (tipus == 0) {
		// parametres camera axonometrica
		zNear = 0;
		zFar = 2 * radi;
	} else if (tipus == 1) {
		// parametres camera perspectiva
		zNear = radi;
		zFar = 3 * radi;
	}
}

void init_llum1()
{
	float param_llum[] = {1.0, 1.0, 1.0, 1.0};
	//float param_llum2[] = {1.0, 1.0, 1.0, 1.0};
	glLightfv(GL_LIGHT1, GL_DIFFUSE, &param_llum[0]);
	glLightfv(GL_LIGHT1, GL_SPECULAR, &param_llum[0]);
	//glLightfv(GL_LIGHT1, GL_AMBIENT, &param_llum2[0]);
	glLightfv(GL_LIGHT1, GL_POSITION, &pos_llum1[0]);
}

void init_llum0()
{
	float param_llum[] = {1.0, 1.0, 0, 1.0};
	//float param_llum2[] = {1.0, 1.0, 1.0, 1.0};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, &param_llum[0]);
	glLightfv(GL_LIGHT0, GL_SPECULAR, &param_llum[0]);
	//glLightfv(GL_LIGHT0, GL_AMBIENT, &param_llum2[0]);

	glLightfv(GL_LIGHT0, GL_POSITION, posicions_llum0[pos_llum0]);
}

void init_llum2()
{
	float param_llum[] = {1.0, 1.0, 1.0, 1.0};
	//float param_llum2[] = {1.0, 1.0, 1.0, 1.0};
	glLightfv(GL_LIGHT2, GL_DIFFUSE, &param_llum[0]);
	glLightfv(GL_LIGHT2, GL_SPECULAR, &param_llum[0]);
	//glLightfv(GL_LIGHT2, GL_AMBIENT, &param_llum2[0]);

	float aux_pos[] = {modelx, 1.0, modelz, 1.0};
	glLightfv(GL_LIGHT2, GL_POSITION, &aux_pos[0]);
}

void initGL()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(colours[0], colours[1], colours[2], colours[3]);

	prev_fovy = prev_zFar = prev_zNear = 0.0;
	prev_tipus = tipus = visibilitat = vel = 1;
	zoom = walk = 0;
	modelx = modelz = 0.0;

	inicialitzacions();
	parametres_camera(tipus);
	tipus_cam(tipus);
	pos_cam(tipus);

	llum0 = llum2 = pos_llum0 = 0;
	llum1 = 1;
	init_llum1();
	if (llum1) glEnable(GL_LIGHT1);
}

void pinta_ninot()
{
	glPushMatrix();
		glPushMatrix();
			glColor3d(1, 1, 1);
			aux.specular[0] = 1.0;
			aux.specular[1] = 1.0;
			aux.specular[2] = 1.0;
			aux.specular[3] = 1.0;
			aux.diffuse[0] = 0.8;
			aux.diffuse[1] = 0.8;
			aux.diffuse[2] = 0.8;
			aux.diffuse[3] = 1.0;
			calc_ambient();
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, aux.ambient);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, aux.diffuse);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, aux.specular);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, aux.shininess);
			glutSolidSphere(0.4, 20, 20);

			glPushMatrix();
			glTranslated(0.0, 0.6, 0.0);
			glColor3d(1, 1, 1);
			glutSolidSphere(0.2, 20, 20);
			glPopMatrix();

			glPushMatrix();
			glTranslated(0.1, 0.6, 0);
			glRotated(90, 0, 1, 0);
			glColor3d(1, 0.698, 0.2353);
			aux.specular[0] = 1.0;
			aux.specular[1] = 1.0;
			aux.specular[2] = 1.0;
			aux.specular[3] = 1.0;
			aux.diffuse[0] = 1.0;
			aux.diffuse[1] = 0.6;
			aux.diffuse[2] = 0.2;
			aux.diffuse[3] = 1.0;
			calc_ambient();
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, aux.ambient);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, aux.diffuse);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, aux.specular);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, aux.shininess);
			glutSolidCone(0.1, 0.2, 20, 20);
			glPopMatrix();
		glPopMatrix();
	glPopMatrix();
}

void pinta_terra()
{
	aux.specular[0] = 1.0;
	aux.specular[1] = 1.0;
	aux.specular[2] = 1.0;
	aux.specular[3] = 1.0;
	aux.diffuse[0] = 0.0;
	aux.diffuse[1] = 0.0;
	aux.diffuse[2] = 0.8;
	aux.diffuse[3] = 1.0;
	calc_ambient();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, aux.ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, aux.diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, aux.specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, aux.shininess);
	glPushMatrix();

	// per moure el terra a la posicio que toca al dibuixar els 100 quadrats
	glTranslated(-5.0, 0, -5.0);

	glNormal3d(0, 1, 0);
	glColor3d(0.6470, 0.2980, 0);

	// per pintar 100 quadrats 1x1 en comptes de un de 10x10
	for (double k = 0; k < 10; k++){
		for (double l = 0; l < 10; l++){
			glBegin(GL_QUADS);
				if (normals_per_v) glNormal3d(0, 1, 0);
				glVertex3d(k, 0, l);
				if (normals_per_v) glNormal3d(0, 1, 0);
				glVertex3d(k + 1, 0, l);
				if (normals_per_v) glNormal3d(0, 1, 0);
				glVertex3d(k + 1, 0, l + 1);
				if (normals_per_v) glNormal3d(0, 1, 0);
				glVertex3d(k, 0, l + 1);
			glEnd();
		}
	}

/*
	glBegin(GL_QUADS);
		glColor3d(0.6470, 0.2980, 0);
		if (normals_per_v) glNormal3d(0, 1, 0);
		glVertex3d(-5.0, 0.0, -5.0);
		if (normals_per_v) glNormal3d(0, 1, 0);
		glVertex3d(-5.0, 0.0, 5.0);
		if (normals_per_v) glNormal3d(0, 1, 0);
		glVertex3d(5.0, 0.0, 5.0);
		if (normals_per_v) glNormal3d(0, 1, 0);
		glVertex3d(5.0, 0.0, -5.0);
	glEnd();
*/	
	glPopMatrix();
}

void pinta_model(int num_model)
{
	glPushMatrix();
		double altura = ymax - ymin;
		double escala, rotate, movex, movez;
		escala = rotate = movex = movez = 0.0;
		if (num_model == 0) {
			escala = 1.0 / altura;
			rotate = dir;
		} else {
			escala = 1.5 / altura;
			movex = movez = 2.5;
		}

		if (modelx > escena_xmax - ((xmax - xmin) / 2.0) * escala)
			modelx = escena_xmax - ((xmax - xmin) / 2.0) * escala;
		if (modelx < escena_xmin + ((xmax - xmin) / 2.0) * escala)
			modelx = escena_xmin + ((xmax - xmin) / 2.0) * escala;
		if (modelz > escena_zmax - ((zmax - zmin) / 2.0) * escala)
			modelz = escena_zmax - ((zmax - zmin) / 2.0) * escala;
		if (modelz < escena_zmin + ((zmax - zmin) / 2.0) * escala)
			modelz = escena_zmin + ((zmax - zmin) / 2.0) * escala;

		// moviment del patricio
		if (num_model == 0) {
			glTranslated(modelx, 0.0, modelz);
			if (llum2) init_llum2();
		}

		// escalar, desplasar
		double aux_trans = (ymin - ((ymin + ymax) / 2.0)) * escala;
		glTranslated(movex, -aux_trans, movez);
		glRotated(rotate, 0, 1, 0);
		glScaled(escala, escala, escala);
		glTranslated(-(xmax + xmin) / 2.0, -(ymin + ymax) / 2.0, -(zmax + zmin) / 2.0);
		for (int i = 0; i < m.faces().size(); i++) {
			const Face f = m.faces()[i];
			glColor4fv(Materials[f.mat].diffuse);
			glNormal3dv(&f.normalC[0]);
			if (mat_index != f.mat) {
				mat_index = f.mat;
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Materials[mat_index].ambient);
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Materials[mat_index].diffuse);
				glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Materials[mat_index].specular);
				glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, Materials[mat_index].shininess);
			}
			glBegin(GL_POLYGON);
				for (int j = 0; j < f.v.size(); j++) {
					if (normals_per_v && f.n.size() > 0) glNormal3dv(&m.normals()[f.n[j]]);
					glVertex3dv(&m.vertices()[f.v[j]]);
				}
			glEnd();
		}
	glPopMatrix();
}

void pinta_parets()
{
	if (visibilitat) {
		aux.specular[0] = 0.0;
		aux.specular[1] = 0.0;
		aux.specular[2] = 0.0;
		aux.specular[3] = 1.0;
		aux.diffuse[0] = 0.0;
		aux.diffuse[1] = 0.7;
		aux.diffuse[2] = 0.0;
		aux.diffuse[3] = 1.0;
		calc_ambient();
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, aux.ambient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, aux.diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, aux.specular);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, aux.shininess);
		glPushMatrix();
			glTranslated(0.0, 0.75, -4.9);
			glScaled(50.0, 7.5, 1.0);
			glColor3d(0.23137255, 0.74901961, 0.41960784);
			glutSolidCube(0.2);
		glPopMatrix();
		glPushMatrix();
			glTranslated(1.5, 0.75, 2.5);
			glScaled(1.0, 7.5, 20.0);
			glColor3d(0.23137255, 0.74901961, 0.41960784);		
			glutSolidCube(0.2);
		glPopMatrix();
	}
}

void pinta_model2()
{
	glPushMatrix();
		double altura = ymax2 - ymin2;
		double escala, rotate, movex, movez;
		escala = 1.0 / altura;
		rotate = -180.0;
		movex = 2.5;
		movez = 1.5;

		double aux_trans = (ymin2 - ((ymin2 + ymax2) / 2.0)) * escala;
		glTranslated(movex, -aux_trans, movez);
		glRotated(rotate, 0, 1, 0);
		glScaled(escala, escala, escala);
		glTranslated(-(xmax2 + xmin2) / 2.0, -(ymin2 + ymax2) / 2.0, -(zmax2 + zmin2) / 2.0);
		for (int i = 0; i < m2.faces().size(); i++) {
			const Face f = m2.faces()[i];
			glColor4fv(Materials[f.mat].diffuse);
			glNormal3dv(&f.normalC[0]);
			if (mat_index != f.mat) {
				mat_index = f.mat;
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Materials[mat_index].ambient);
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Materials[mat_index].diffuse);
				glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Materials[mat_index].specular);
				glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, Materials[mat_index].shininess);
			}
			glBegin(GL_POLYGON);
				for (int j = 0; j < f.v.size(); j++) {
					if (normals_per_v && f.n.size() > 0) glNormal3dv(&m2.normals()[f.n[j]]);
					glVertex3dv(&m2.vertices()[f.v[j]]);
				}
			glEnd();
		}
	glPopMatrix();
}

void refresh()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_DEPTH_TEST);

	glPushMatrix();
		if (llum0) init_llum0();
		pinta_terra();
		glPushMatrix();
			glTranslated(2.5, 0.4, -2.5);
			pinta_ninot();
		glPopMatrix();
		glPushMatrix();
			glTranslated(-2.5, 0.4, 2.5);
			pinta_ninot();
		glPopMatrix();
		glPushMatrix();
			glTranslated(-2.5, 0.4, -2.5);
			pinta_ninot();
		glPopMatrix();
		pinta_model(0);
		pinta_model(1);
		pinta_model2();
		pinta_parets();
	glPopMatrix();
	glutSwapBuffers();
}

void resize(int w, int h)
{
	win_x = w;
	win_y = h;
	double ra_v = (double)w / (double)h;
	if (tipus == 0) {
		if (ra_v >= 1) {
			xleft = -radi * ra_v;
			xright = radi * ra_v;
		} else if (ra_v < 1) {
			ybottom = -radi / ra_v;
			ytop = radi / ra_v;
		}
	} else {
		if (ra_v < 1) {
			alpha_v = atan(tan(alpha_v) / ra_v);
			fovy = 2 * alpha_v;
		}
		aspect = ra_v;
	}
	tipus_cam(tipus);
	glViewport(0, 0, w, h);
}

void click(int a, int b, int x, int y)
{
	aux_x = x;
	aux_y = y;
	if (b == GLUT_DOWN && !walk)
	{
		if (a == GLUT_LEFT_BUTTON) state = 2;
		if (a == GLUT_RIGHT_BUTTON) state = 3;
	}
	if (b == GLUT_UP)
	{
		state = 1;
		aux_x = aux_y = 0;
	}
}

void move(int x, int y)
{
	if (state == 2) {
		// fa el mateix que la rotació pero rotant la camera en comptes de l'escena
		angley += 360 * (2 * (x - aux_x) / (double)win_x);
		anglex += 360 * (2 * (aux_y - y) / (double)win_y);
		pos_cam(tipus);
	} else if (state == 3) {
		double aux = 2 * (aux_y - y) / (double)win_y;
		if (tipus == 0) {
			xleft += aux;
			xright -= aux;
			ybottom += aux;
			ytop -= aux;
		} else {
			fovy += aux * 10;
		}
		tipus_cam(tipus);
	}
	aux_x = x;
	aux_y = y;
	glutPostRedisplay();
}

void calcCapsa(bool model)
{
	if (model) {
		filename = "legoman.obj";
		m2.load(filename);
		xmin2 = xmax2 = m2.vertices()[0];
		ymin2 = ymax2 = m2.vertices()[1];
		zmin2 = zmax2 = m2.vertices()[2];

		for (int i = 0; i < m2.vertices().size(); i += 3) {
			if (m2.vertices()[i] < xmin2) xmin2 = m2.vertices()[i];
			if (m2.vertices()[i] > xmax2) xmax2 = m2.vertices()[i];
			if (m2.vertices()[i+1] < ymin2) ymin2 = m2.vertices()[i+1];
			if (m2.vertices()[i+1] > ymax2) ymax2 = m2.vertices()[i+1];
			if (m2.vertices()[i+2] < zmin2) zmin2 = m2.vertices()[i+2];
			if (m2.vertices()[i+2] > zmax2) zmax2 = m2.vertices()[i+2];
		}
		filename = "Patricio.obj";		
	}
	else {
		m.load(filename);
		xmin = xmax = m.vertices()[0];
		ymin = ymax = m.vertices()[1];
		zmin = zmax = m.vertices()[2];

		for (int i = 0; i < m.vertices().size(); i += 3) {
			if (m.vertices()[i] < xmin) xmin = m.vertices()[i];
			if (m.vertices()[i] > xmax) xmax = m.vertices()[i];
			if (m.vertices()[i+1] < ymin) ymin = m.vertices()[i+1];
			if (m.vertices()[i+1] > ymax) ymax = m.vertices()[i+1];
			if (m.vertices()[i+2] < zmin) zmin = m.vertices()[i+2];
			if (m.vertices()[i+2] > zmax) zmax = m.vertices()[i+2];
		}
		filename = "Patricio.obj";
	}
}

void key(unsigned char a, int x, int y)
{
	if (a == 'h') {
		cout << "Pitjeu 'h' per mostrar l'ajuda\n";
		cout << "Amb el boto esquerra del ratoli es pot girar el model\n";
		cout << "Pitjeu les tecles 'a, s, d, w' per moure el model.\n";
		cout << "	Amb les tecles z i x s'agumenta o disminueix la velocitat de moviment\n";
		cout << "Pitjeu 'p' per canviar el tipus de camera" << endl;
		cout << "Pitjeu 'r' per fer un reset de la imatge" << endl;
		cout << "Amb el boto dret del ratoli i arrossegant al llarg de l'eix Y es pot fer zoom" << endl;
		cout << "Pitjeu 'v' per fer les parets visibles o no" << endl;
		cout << "Pitjeu 'c' per canviar la camera a primera persona" << endl;
		cout << "Pitjeu 'i' per activar/desactivar la iluminacio" << endl;
		cout << "Pitjeu 'n' per activar/desactivar les normals per vertex" << endl;
		cout << "Pitjeu '0' per activar/desactivar la llum 0" << endl;
		cout << "Pitjeu 'm' per canviar la posicio de la llum 0" << endl;
		cout << "Pitjeu '1' per activar/desactivar la llum 1 (es la inicial)" << endl;
		cout << "Pitjeu '2' per activar/desactivar la llum 2" << endl;
		cout << "Pitjeu ESC per sortir de l'aplicacio.\n";
	}
	else if (a == 'm') {
		if (pos_llum0 == 4) pos_llum0 = 0;
		else pos_llum0++;
	}
	else if (a == '0') {
		if (llum0) {
			llum0 = 0;
			glDisable(GL_LIGHT0);
		}
		else {
			llum0 = 1;
			glEnable(GL_LIGHT0);
		}
	}
	else if (a == '1') {
		if (llum1) {
			llum1 = 0;
			glDisable(GL_LIGHT1);
		}
		else {
			llum1 = 1;
			glEnable(GL_LIGHT1);
		}
	}
	else if (a == '2') {
		if (llum2) {
			llum2 = 0;
			glDisable(GL_LIGHT2);
		}
		else {
			llum2 = 1;
			glEnable(GL_LIGHT2);
			init_llum2();
		}
	}
	else if (a == 'i') {
		if (iluminacio) {
			iluminacio = 0;
			glDisable(GL_LIGHTING);
		}
		else {
			iluminacio = 1;
			glEnable(GL_LIGHTING);
		}
	}
	else if (a == 'p') {
		if (tipus == 0) tipus = 1;
		else tipus = 0;
		parametres_camera(tipus);
		tipus_cam(tipus);
		pos_cam(tipus);
	}
	else if (a == 'v') {
		if (visibilitat) visibilitat = 0;
		else visibilitat = 1;
	}
	else if (a == 'c') {
		if (walk) {
			walk = 0;
			tipus = prev_tipus;
			fovy = prev_fovy;
			zNear = prev_zNear;
			zFar = prev_zFar;
		}
		else {
			walk = 1;
			prev_tipus = tipus;
			tipus = 1;
			prev_fovy = fovy;
			fovy = 60.0;
			prev_zNear = zNear;
			zNear = 0.1;
			prev_zFar = zFar;
			zFar = 8.0;
		}
		tipus_cam(tipus);
		pos_cam(tipus);
	}
	else if (a == 'n') {
		if (normals_per_v) normals_per_v = 0;
		else normals_per_v = 1;
	}
	else if (a == 'r') {
		glDisable(GL_LIGHT2);
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHT1);
		initGL();
	}
	else if (a == 27) exit(0);

	// moviment del patricio central
	if (state == 1) {
		if (a == 'w') {
			modelx += sin(dir * PI / 180.0) * (MOV * vel);
			modelz += cos(dir * PI / 180.0) * (MOV * vel);
		}
		if (a == 's') {
			modelx -= sin(dir * PI / 180.0) * (MOV * vel);
			modelz -= cos(dir * PI / 180.0) * (MOV * vel);
		}
		if (a == 'd') dir -= 3;
		if (a == 'a') dir += 3;
		if (a == 'z') {
			if (vel <= 1) vel = 1;
			else --vel;
		}
		if (a == 'x') ++vel;
		if (walk) pos_cam(tipus);
	}

	glutPostRedisplay();
}

void init_llum_positions()
{
	posicions_llum0[0] = &pos0[0];
	posicions_llum0[1] = &pos1[0];
	posicions_llum0[2] = &pos2[0];
	posicions_llum0[3] = &pos3[0];
	posicions_llum0[4] = &pos4[0];
}

int main(int argc, const char *argv[])
{
	glutInit(&argc, (char **)argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutCreateWindow("IDI: Practiques OpenGL");
	// inicialitza vector de posicions de llums
	init_llum_positions();
	initGL();
	calcCapsa(true);
	calcCapsa(false);
	glutDisplayFunc(refresh);
	glutMouseFunc(click);
	glutMotionFunc(move);
	glutReshapeFunc(resize);
	glutKeyboardFunc(key);
	glutMainLoop();
	return 0;
}
