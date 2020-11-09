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
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
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

void normal(float* mas) {
	float x, y, z;
	x = mas[1] * 3 - 0 * mas[2];
	y = -mas[0] * 3 + 0 * mas[2];
	z = mas[0] * 0 - 0 * mas[1];
	mas[0] = x;
	mas[1] = y;
	mas[2] = z;

}

void treug() {
	float r = 2.5; //радиус
	float XYZ[3];

	glColor4d(1, 0.75, 0.53, 0.56);
	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i <= 9; i++) {
		float a = (float)i / (float)10 * 3.1415f;
		float b = (float)(i + 1) / (float)10 * 3.1415f;
		glBegin(GL_QUADS);

		XYZ[0] = (3 - cos(b) * r) - (3 - cos(a) * r);
		XYZ[1] = (7.5 - sin(b) * r) - (7.5 - sin(a) * r);
		XYZ[2] = 0;
		normal(XYZ);
		glNormal3f(XYZ[0], XYZ[1], XYZ[2]);

		glVertex3f(3 - cos(a) * r, 7.5 - sin(a) * r, 0);
		glVertex3f(3 - cos(a) * r, 7.5 - sin(a) * r, 3);
		glVertex3f(3 - cos(b) * r, 7.5 - sin(b) * r, 3);
		glVertex3f(3 - cos(b) * r, 7.5 - sin(b) * r, 0);
		glEnd();
	}
	for (int i = 0; i <= 9; i++) {
		float a = (float)i / (float)10 * 3.1415f * (-0.5);
		float b = (float)(i + 1) / (float)10 * 3.1415f * (-0.5);
		glBegin(GL_QUADS);

		XYZ[0] = 3 - cos(a) * r - (3 - cos(b) * r);
		XYZ[1] = 7.5 - sin(a) * r - (7.5 - sin(b) * r);
		XYZ[2] = 0;
		normal(XYZ);
		glNormal3f(XYZ[0], XYZ[1], XYZ[2]);

		glVertex3f(3 - cos(a) * r, 7.5 - sin(a) * r, 0);
		glVertex3f(3 - cos(a) * r, 7.5 - sin(a) * r, 3);
		glVertex3f(3 - cos(b) * r, 7.5 - sin(b) * r, 3);
		glVertex3f(3 - cos(b) * r, 7.5 - sin(b) * r, 0);
		glEnd();
	}
	glEnd();
}

void treug1() {
	float r = 2.5; //радиус
	glNormal3d(0, 0, -1);
	glColor4d(1, 0.5, 0.36, 0.38);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(3, 7.5, 0);  //центр окружности
	for (int i = 0; i <= 10; i++) {
		float a = (float)i / (float)10 * 3.1415f;
		glVertex3f(3 - cos(a) * r, 7.5 - sin(a) * r, 0);
	}
	for (int i = 0; i <= 10; i++) {
		float a = (float)i / (float)10 * 3.1415f * (-0.5);
		glVertex3f(3 - cos(a) * r, 7.5 - sin(a) * r, 0);
	}

	glEnd();
}

void treug2() {
	float r = 2.5; //радиус
	glNormal3d(0, 0, 1);
	glColor4d(1, 1, 0.7, 0.76);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(3, 7.5, 3);  //центр окружности
	for (int i = 0; i <= 10; i++) {
		float a = (float)i / (float)10 * 3.1415f;
		glVertex3f(3 - cos(a) * r, 7.5 - sin(a) * r, 3);
	}
	for (int i = 0; i <= 10; i++) {
		float a = (float)i / (float)10 * 3.1415f * (-0.5);
		glVertex3f(3 - cos(a) * r, 7.5 - sin(a) * r, 3);
	}
	glEnd();
}

void vpk() {
	float X = 5, Y = -12, R = 13;
	float XYZ[3];

	double N[3];
	double* n = N;

	glBegin(GL_QUADS);
	glColor4d(1, 0.75, 0.53, 0.56);
	for (float p = 1.1801; p <= 1.97; p += 0.001) {

		float x1 = X + cos(p) * R;
		float y1 = Y + sin(p) * R;
		float x2 = X + cos(p + 0.001) * R;
		float y2 = Y + sin(p + 0.001) * R;
		XYZ[0] = x1-x2;
		XYZ[1] = y1-y2;
		XYZ[2] = 0;
		normal(XYZ);
		glNormal3f(XYZ[0], XYZ[1], XYZ[2]);
		
		glVertex3f(x1, y1, 0);
		glVertex3f(x2, y2, 0);
		glVertex3f(x2, y2, 3);
		glVertex3f(x1, y1, 3);
	}
	glEnd();
}

void laba1() {
	{
		glBegin(GL_POLYGON);
		float X = 5, Y = -12, R = 13;
		glNormal3d(0, 0, -1);
		glColor4d(1, 0.5, 0.36, 0.38);
		glVertex3d(7, 2, 0);
		for (float a =4.317; a <= 4.9; a += 0.001) {
			float X1 = X - cos(a) * R;
			float Y1 = Y - sin(a) * R;
			glVertex3d(X1, Y1, 0);
		}
		glEnd();
		
		glBegin(GL_POLYGON);
		glNormal3d(0, 0, 1);
		glColor4d(1, 1, 0.7, 0.76);
		glVertex3d(7, 2, 3);
		for (float a = 4.317; a <= 4.9; a += 0.001) {
			float X1 = X - cos(a) * R;
			float Y1 = Y - sin(a) * R;

			glVertex3d(X1, Y1, 3);
		}
		glEnd();

		glBegin(GL_TRIANGLES);
		glNormal3d(0, 0, -1);
		//-----------------------------------------
		
		glColor4d(1, 0.5, 0.36, 0.38);
		glVertex3d(0, 0, 0);
		glVertex3d(7, 2, 0);
		glVertex3d(7, 5, 0);

		glVertex3d(7, 2, 0);
		glVertex3d(7, 5, 0);
		glVertex3d(9, 7, 0);

		glVertex3d(7, 5, 0);
		glVertex3d(9, 7, 0);
		glVertex3d(3, 10, 0);

		glVertex3d(3, 10, 0);
		glVertex3d(3, 5, 0);
		glVertex3d(7, 5, 0);

		//-------------------------------------
		glNormal3d(0, 0, 1);

		glColor4d(1, 1, 0.7, 0.76);
		glVertex3d(0, 0, 3);
		glVertex3d(7, 2, 3);
		glVertex3d(7, 5, 3);

		glVertex3d(7, 2, 3);
		glVertex3d(7, 5, 3);
		glVertex3d(9, 7, 3);

		glVertex3d(7, 5, 3);
		glVertex3d(9, 7, 3);
		glVertex3d(3, 10, 3);

		glVertex3d(3, 10, 3);
		glVertex3d(3, 5, 3);
		glVertex3d(7, 5, 3);
		//-----------------------------------
		glEnd();
	}
	{
		glBegin(GL_QUADS);

		glNormal3d(-15, 21, 0);
		glColor4d(1, 0.75, 0.53, 0.56);
		glVertex3d(0, 0, 0);
		glVertex3d(0, 0, 3);
		glVertex3d(7, 5, 3);
		glVertex3d(7, 5, 0);

		glNormal3d(0, -12, 0);
		glVertex3d(7, 5, 0);
		glVertex3d(7, 5, 3);
		glVertex3d(3, 5, 3);
		glVertex3d(3, 5, 0);

		glNormal3d(9, 18, 0);
		glVertex3d(3, 10, 0);
		glVertex3d(3, 10, 3);
		glVertex3d(9, 7, 3);
		glVertex3d(9, 7, 0);

		glNormal3d(15, -6, 0);
		glVertex3d(9, 7, 0);
		glVertex3d(9, 7, 3);
		glVertex3d(7, 2, 3);
		glVertex3d(7, 2, 0);

		glNormal3d(6, 9, 0);
		glVertex3d(7, 2, 0);
		glVertex3d(7, 2, 3);
		glVertex3d(10, 0, 3);
		glVertex3d(10, 0, 0);
		

		glEnd();
	}


	treug();
	treug1();
	treug2();
	vpk();

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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


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

	laba1();

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