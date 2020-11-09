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

//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;

	
	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	void SetUpCamera()
	{
		//�������� �� ������� ������ ������
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
		//������� ��������� ������
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}

	
	//������ ����� � ����� ��� ���������� �����, ���������� �������
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
			//����� �� ��������� ����� �� ����������
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
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

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����




//������ ���������� ����
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//������� ���� �� ���������, � ����� ��� ����
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

//����������� ����� ������ ��������
void initRender(OpenGL *ogl)
{
	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);
	

	//������ ����������� ���������  (R G B)
	RGBTRIPLE *texarray;

	//������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//���������� �� ��� ��������
	glGenTextures(1, &texId);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//�������� ������
	free(texCharArray);
	free(texarray);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH); 


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

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
	float r = 2.5; //������
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
	float r = 2.5; //������
	glNormal3d(0, 0, -1);
	glColor4d(1, 0.5, 0.36, 0.38);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(3, 7.5, 0);  //����� ����������
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
	float r = 2.5; //������
	glNormal3d(0, 0, 1);
	glColor4d(1, 1, 0.7, 0.76);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(3, 7.5, 3);  //����� ����������
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


	//��������������
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//��������� ���������
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//������ �����
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//���� ���� �������, ��� ����������� (����������� ���������)
	glShadeModel(GL_SMOOTH);
	//===================================
	//������� ���  

	laba1();

   //��������� ������ ������

	
	glMatrixMode(GL_PROJECTION);	//������ �������� ������� ��������. 
	                                //(���� ��������� ��������, ����� �� ������������.)
	glPushMatrix();   //��������� ������� ������� ������������� (������� ��������� ������������� ��������) � ���� 				    
	glLoadIdentity();	  //��������� ��������� �������
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //������� ����� ������������� ��������

	glMatrixMode(GL_MODELVIEW);		//������������� �� �����-��� �������
	glPushMatrix();			  //��������� ������� ������� � ���� (��������� ������, ����������)
	glLoadIdentity();		  //���������� �� � ������

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //������� ����� ��������� ��� ������� ������ � �������� ������.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - ���/���� �������" << std::endl;
	ss << "L - ���/���� ���������" << std::endl;
	ss << "F - ���� �� ������" << std::endl;
	ss << "G - ������� ���� �� �����������" << std::endl;
	ss << "G+��� ������� ���� �� ���������" << std::endl;
	ss << "�����. �����: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "�����. ������: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "��������� ������: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //��������������� ������� �������� � �����-��� �������� �� �����.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}