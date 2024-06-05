#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture2.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}

double* normal(double A[], double B[], double C[]) {
	double a[3];
	double b[3];
	double n[3];
	a[0] = B[0] - A[0];
	a[1] = B[1] - A[1];
	a[2] = B[2] - A[2];

	b[0] = C[0] - A[0];
	b[1] = C[1] - A[1];
	b[2] = C[2] - A[2];

	n[0] = -(a[1] * b[2] - b[1] * a[2]);
	n[1] = -(a[0] * b[2] + b[0] * a[2]);
	n[2] = -(a[0] * b[1] - b[0] * a[1]);

	return n;
}

float alpha(const double A[3], const double B[3]) {
	float x1 = A[1] - B[1];
	float x2 = 1;
	float y1 = A[0] - B[0];
	float y2 = 0;
	return (180 / 3.14) * acos((x1 * x2 + y1 * y2) / (sqrt(x1 * x1 + y1 * y1) * sqrt(x2 * x2 + y2 * y2)));
}

void Сonvex(int step, double r, const double start[3], const double start1[3], float a) {
	double A[] = { 0, 0, 0 };
	double B[] = { 0, 0, 0 };
	double C[] = { 0, 0, 0 };
	double D[] = { 0, 0, 0 };

	double A1[3][3];
	double B1[] = { 0, 0, 0 };
	double B2[] = { 1, 0, 0 };
	double B3[] = { 0, 1, 0 };

	float angle_offset = 3.14 / step;
	double n1, n2, n3, c = 0, cc;
	double* n;

	glPushMatrix();
	glTranslated(-4.5, 0.5, 0);
	glTranslated(11, 12, 0);
	glRotated(a, 0, 0, 1);
	glTranslated(-11, -12, -0);
	glBegin(GL_QUADS);
	glColor3d(0.2, 1, 1);
	B[2] = start1[2];
	for (int i = 0; i < 3; i++) {
		C[i] = start[i];
		D[i] = start1[i];
	}


	//крышка выпуклости
	for (int j = 0; j < step; j++) {
		A[1] = start[1] + r * cos(angle_offset * j);
		A[0] = start[0] + r * 1 / 2 * sin(angle_offset * j);
		B[1] = A[1];
		B[0] = A[0];


		C[1] = start[1] + r * cos(angle_offset * (j + 1));
		C[0] = start[0] + r * 1 / 2 * sin(angle_offset * (j + 1));
		D[1] = C[1];
		D[0] = C[0];
		// нормали
		n = normal(B, A, C);
		n1 = n[0];
		n2 = n[1];
		n3 = n[2];
		glNormal3d(n1, n2, n3);
		//
		cc = 1.0 / step;
		glTexCoord2d(0, c);
		glVertex3dv(B);
		glTexCoord2d(1, c);
		glVertex3dv(A);
		glTexCoord2d(1, c+cc);
		glVertex3dv(C);
		glTexCoord2d(0, c+cc);
		glVertex3dv(D);

		c = c + cc;
	}
	glEnd();

	//боковые части выпуклости
	glBegin(GL_POLYGON);
	n = normal(B1, B2, B3);
	n1 = n[0];
	n2 = n[1];
	n3 = n[2];
	glNormal3d(n1, n2, n3);
	glVertex3dv(start);
	for (int j = 0; j <= step; j++) {
		A[1] = start[1] + r * cos(angle_offset * j);
		A[0] = start[0] + r * 1 / 2 * sin(angle_offset * j);
		//
		if (j < 3) {
			A1[j][0] = A[0];
			A1[j][1] = A[1];
			A1[j][2] = A[2];
		}
		else
		{
			n = normal(A1[0], A1[1], A1[2]);
			n1 = n[0];
			n2 = n[1];
			n3 = n[2];
			glNormal3d(-n1, -n2, -n3);
		}
		
		//

		glVertex3dv(A);
	}
	//

	//
	glEnd();
	glBegin(GL_POLYGON);
	n = normal(B1, B2, B3);
	n1 = n[0];
	n2 = n[1];
	n3 = n[2];
	glNormal3d(-n1, -n2, -n3);
	glVertex3dv(start1);
	for (int j = 0; j <= step; j++) {
		B[1] = start[1] + r * cos(angle_offset * j);
		B[0] = start[0] + r * 1 / 2 * sin(angle_offset * j);

		n = normal(A1[0], A1[1], A1[2]);
		n1 = n[0];
		n2 = n[1];
		n3 = n[2];
		glNormal3d(n1, n2, n3);
		//
		glVertex3dv(B);
	}
	
	glEnd();
	glPopMatrix();
}



void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  





	double a, b, c;
	double* n;
	double A[] = { 1, 0, 0 };
	double B[] = { 5, 4, 0 };
	double C[] = { 9, 2, 0 };
	double D[] = { 6, 6, 0 };
	double E[] = { 11, 12, 0 };
	double F[] = { 2, 13, 0 };
	double G[] = { 4, 6, 0 };
	double H[] = { 0, 5, 0 };
	double I[] = { 1, 0, 0 };

	double A2[] = { 1, 0, 3 };
	double B2[] = { 5, 4, 3 };
	double C2[] = { 9, 2, 3 };
	double D2[] = { 6, 6, 3 };
	double E2[] = { 11, 12, 3 };
	double F2[] = { 2, 13, 3 };
	double G2[] = { 4, 6, 3 };
	double H2[] = { 0, 5, 3 };
	double I2[] = { 1, 0, 3 };
	glBindTexture(GL_TEXTURE_2D, texId);
	float alp = alpha(F, E);
	double Length = sqrt(pow(F[0] - E[0], 2) + pow(F[1] - E[1], 2));
	Сonvex(10, Length / 2, E, E2, alp);
	
	glBegin(GL_QUADS);

	glColor3d(1, 0.8, 1);

	n = normal(A, A2, B2);
	a = n[0];
	b = n[1];
	c = n[2];
	glNormal3d(a, b, c);
	glTexCoord2d(0, 0);
	glVertex3dv(A);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 0);
	glVertex3dv(A2);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 1);
	glVertex3dv(B2);
	glNormal3d(a, b, c);
	glTexCoord2d(0, 1);
	glVertex3dv(B);

	glColor3d(0.4, 0.9, 0.2);

	n = normal(B, B2, C2);
	a = n[0];
	b = n[1];
	c = n[2];
	glNormal3d(a, b, c);
	glTexCoord2d(0, 0);
	glVertex3dv(B);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 0);
	glVertex3dv(B2);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 1);
	glVertex3dv(C2);
	glNormal3d(a, b, c);
	glTexCoord2d(0, 1);
	glVertex3dv(C);

	glColor3d(0, 0.7, 0.2);

	n = normal(C, C2, D);
	a = n[0];
	b = n[1];
	c = n[2];
	glNormal3d(a, b, c);
	glTexCoord2d(0, 0);
	glVertex3dv(C);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 0);
	glVertex3dv(C2);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 1);
	glVertex3dv(D2);
	glNormal3d(a, b, c);
	glTexCoord2d(0, 1);
	glVertex3dv(D);

	glColor3d(0, 0.3, 0);

	n = normal(D, D2, E2);
	a = n[0];
	b = n[1];
	c = n[2];
	glNormal3d(a, b, c);
	glTexCoord2d(0, 0);
	glVertex3dv(D);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 0);
	glVertex3dv(D2);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 1);
	glVertex3dv(E2);
	glNormal3d(a, b, c);
	glTexCoord2d(0, 1);
	glVertex3dv(E);

	glColor3d(0.3, 0, 0.3);

	n = normal(E, E2, F2);
	a = n[0];
	b = n[1];
	c = n[2];
	glNormal3d(a, b, c);
	glTexCoord2d(0, 0);
	glVertex3dv(E);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 0);
	glVertex3dv(E2);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 1);
	glVertex3dv(F2);
	glNormal3d(a, b, c);
	glTexCoord2d(0, 1);
	glVertex3dv(F);

	glColor3d(0, 0.2, 0.5);

	n = normal(F, F2, G2);
	a = n[0];
	b = n[1];
	c = n[2];
	glNormal3d(a, b, c);
	glTexCoord2d(0, 0);
	glVertex3dv(F);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 0);
	glVertex3dv(F2);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 1);
	glVertex3dv(G2);
	glNormal3d(a, b, c);
	glTexCoord2d(0, 1);
	glVertex3dv(G);

	glColor3d(0.3, 0.6, 0);

	n = normal(G, G2, H2);
	a = n[0];
	b = n[1];
	c = n[2];
	glNormal3d(a, b, c);
	glTexCoord2d(0, 0);
	glVertex3dv(G);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 0);
	glVertex3dv(G2);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 1);
	glVertex3dv(H2);
	glNormal3d(a, b, c);
	glTexCoord2d(0, 1);
	glVertex3dv(H);

	glColor3d(1, 1, 0);

	n = normal(H, H2, I2);
	a = n[0];
	b = n[1];
	c = n[2];
	glNormal3d(a, b, c);
	glTexCoord2d(0, 0);
	glVertex3dv(H);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 0);
	glVertex3dv(H2);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 1);
	glVertex3dv(I2);
	glNormal3d(a, b, c);
	glTexCoord2d(0, 1);
	glVertex3dv(I);

	glEnd();
	
	glBegin(GL_QUADS);

	glColor3d(0, 0.2, 0);

	n = normal(A, B, G);
	a = n[0];
	b = n[1];
	c = n[2];
	glNormal3d(a, b, c);
	glTexCoord2d(0, 0);
	glVertex3dv(A);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 0);
	glVertex3dv(B);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 1);
	glVertex3dv(G);
	glNormal3d(a, b, c);
	glTexCoord2d(0, 1);
	glVertex3dv(H);

	glTexCoord2d(0, 0);
	glVertex3dv(B);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 0);
	glVertex3dv(C);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 1);
	glVertex3dv(D);
	glNormal3d(a, b, c);
	glTexCoord2d(0, 1);
	glVertex3dv(G);

	glTexCoord2d(0, 0);
	glVertex3dv(D);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 0);
	glVertex3dv(E);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 1);
	glVertex3dv(F);
	glNormal3d(a, b, c);
	glTexCoord2d(0, 1);
	glVertex3dv(G);

	glEnd();

	glBegin(GL_QUADS);

	glColor3d(0, 0, 0.9);

	n = normal(A2, B2, G2);
	a = -(n[0]);
	b = -(n[1]);
	c = -(n[2]);
	glNormal3d(a, b, c);

	glTexCoord2d(0, 0);
	glVertex3dv(A2);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 0);
	glVertex3dv(B2);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 1);
	glVertex3dv(G2);
	glNormal3d(a, b, c);
	glTexCoord2d(0, 1);
	glVertex3dv(H2);

	glNormal3d(a, b, c);
	glTexCoord2d(0, 0);
	glVertex3dv(B2);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 0);
	glVertex3dv(C2);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 1);
	glVertex3dv(D2);
	glNormal3d(a, b, c);
	glTexCoord2d(0, 1);
	glVertex3dv(G2);


	glNormal3d(a, b, c);
	glTexCoord2d(0, 0);
	glVertex3dv(D2);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 0);
	glVertex3dv(E2);
	glNormal3d(a, b, c);
	glTexCoord2d(1, 1);
	glVertex3dv(F2);
	glNormal3d(a, b, c);
	glTexCoord2d(0, 1);
	glVertex3dv(G2);

	glColor3d(0, 1, 0);

	glEnd();

	glBegin(GL_LINES);



	glEnd();


	//конец рисования квадратика станкина


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}