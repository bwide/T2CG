// NAVEGCAO
//   W
// A S D  -> Move a visao
//
// SETAS -> Movem o jogador
//
// 8 -> PARA CIMA
// 2 -> PARA BAIXO

#ifdef WIN32
#include <windows.h>
#include "gl\glut.h"
    static DWORD last_idle_time;
#else
    #include <sys/time.h>
    static struct timeval last_idle_time;
#endif
#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>

using namespace std;
GLfloat AspectRatio;

int WINDOW_WIDTH = 1200;
int WINDOW_HEIGHT = 800;

int NUM_COLORS;
int colors[10][3];

int DIVISION_SPACE = 30;

typedef struct {
    int x, y, z;
    int alpha;
} Pos;

float PI = 3.14159;

Pos user;
Pos target;
Pos zero;

float degrees(int angle) {
    return angle * 180 / PI;
}

float radians(int angle) {
    return angle * PI / 180;
}


float USER_RADIUS = 20.0;

int CITY_MAXWIDTH = 100;
int CITY_MAXDEPTH = 100;

int OBJ_WIDTH = 5;
int OBJ_DEPTH = 5;

int CULLING_BACKFACE = 0;
int CULLING_INTERSECTION = 0;
int CULLING_SUBDIVISION = 1;

ifstream inFile;
int x;

typedef struct{
    float x, y, z;
}VOP;

typedef struct{
    int width, depth;
    int form[500][500];
}City;

City simCity;

void open(string file) {
    inFile.open(file);
    
    if (inFile.is_open() == 0) {
        cout << "Couldn't open " + file + " ";
        exit(1);
    }else{
        cout << "Opened " + file + " " << endl;
    }
}

void readColors(char fileName[50]){
    open(fileName);

    while(inFile >> x){
        NUM_COLORS = x;
        for(int i = 0; i < NUM_COLORS; i++){
            int id, r, g, b;
            inFile >> id >> r >> g >> b;
            colors[i][0] = r;
            colors[i][1] = g;
            colors[i][2] = b;
        }
    }
    inFile.close();
}

void readCity(char fileName[50]){
    inFile.open(fileName);
    if(!inFile){
        cout << "Não consegui abrir o arquivo " << fileName << endl;
        exit(1);
    }else{
        cout << "Arquivo "<< fileName <<" aberto" << endl;
    }


    inFile >> simCity.depth >> simCity.width;

    for(int i = 0; i < simCity.width; i++){
        for(int j = 0; j < simCity.depth; j++){
            inFile >> simCity.form[i][j];
        }
    }

    inFile.close();

    cout << "City: " << endl;
    cout << simCity.depth << endl;
    cout << simCity.width << endl;
}

void DefineLuz(void){
  // Define cores para um objeto dourado
  GLfloat LuzAmbiente[]   = {0.24725f, 0.1995f, 0.07f } ;
  GLfloat LuzDifusa[]   = {0.75164f, 0.60648f, 0.22648f, 1.0f };
  GLfloat LuzEspecular[] = {0.626281f, 0.555802f, 0.366065f, 1.0f };
  GLfloat PosicaoLuz0[]  = {0.0f, 100.0f, 0.0f, 1.0f };
  GLfloat PosicaoLuz1[]  = {-3.0f, -3.0f, 0.0f, 1.0f };
  GLfloat Especularidade[] = {1.0f, 1.0f, 1.0f, 1.0f };

   // ****************  Fonte de Luz 0

	glEnable ( GL_COLOR_MATERIAL );


   // Habilita o uso de ilumina��o
  glEnable(GL_LIGHTING);

  // Ativa o uso da luz ambiente
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LuzAmbiente);
  // Define os parametros da Luz n�mero Zero
  glLightfv(GL_LIGHT0, GL_AMBIENT, LuzAmbiente);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, LuzDifusa  );
  glLightfv(GL_LIGHT0, GL_SPECULAR, LuzEspecular  );
  glLightfv(GL_LIGHT0, GL_POSITION, PosicaoLuz0 );
  glEnable(GL_LIGHT0);

  // Ativa o "Color Tracking"
  glEnable(GL_COLOR_MATERIAL);

  // Define a reflectancia do material
  glMaterialfv(GL_FRONT,GL_SPECULAR, Especularidade);

  // Define a concentra��o do brilho.
  // Quanto maior o valor do Segundo parametro, mais
  // concentrado ser� o brilho. (Valores v�lidos: de 0 a 128)
  glMateriali(GL_FRONT,GL_SHININESS,51);

  // ****************  Fonte de Luz 1

  // Ativa o uso da luz ambiente
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LuzAmbiente);
  // Define os parametros da Luz n�mero Zero
  glLightfv(GL_LIGHT1, GL_AMBIENT, LuzAmbiente);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, LuzDifusa  );
  glLightfv(GL_LIGHT1, GL_SPECULAR, LuzEspecular  );
  glLightfv(GL_LIGHT1, GL_POSITION, PosicaoLuz1 );
  glEnable(GL_LIGHT1);

  // Ativa o "Color Tracking"
  glEnable(GL_COLOR_MATERIAL);

  // Define a reflectancia do material
  glMaterialfv(GL_FRONT,GL_SPECULAR, Especularidade);

  // Define a concentra��o do brilho.
  // Quanto maior o valor do Segundo parametro, mais
  // concentrado ser� o brilho. (Valores v�lidos: de 0 a 128)
  glMateriali(GL_FRONT,GL_SHININESS,20);

}

float distance(float x1, float y1, float z1, float x2, float y2, float z2){
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2));
}

bool calculateIntersection(VOP cubePos){
    //r1 = raio do cubo
    float cube_radius = 1;
    float r1 = distance(cubePos.x, cubePos.y, cubePos.z, cubePos.x + cube_radius, cubePos.y + cube_radius, cubePos.z + cube_radius);

    //r2 = raio do user
    float r2 = 2;

    // distancia do centro do cubo até o user
    float d1 = 3;

    // Soma dos raios
    float d2 = r1 + r2;

    return d1 <= d2;
}

float dotProduct(VOP p1, VOP p2){
    float x = p1.x * p2.x;
    float y = p1.y * p2.y;
    float z = p1.z * p2.z;
    return x + y + z;
}


void DrawObject(float x, float y, float z){
	glBegin ( GL_QUADS );
        // Front Face
        glNormal3f(0,0,1);
        glVertex3f(-1.0f*x, -1.0f*y,  1.0f*z);
        glVertex3f( 1.0f*x, -1.0f*y,  1.0f*z);
        glVertex3f( 1.0f*x,  1.0f*y,  1.0f*z);
        glVertex3f(-1.0f*x,  1.0f*y,  1.0f*z);
        // Back Face
        glNormal3f(0,0,-1);
        glVertex3f(-1.0f*x, -1.0f*y, -1.0f*z);
        glVertex3f(-1.0f*x,  1.0f*y, -1.0f*z);
        glVertex3f( 1.0f*x,  1.0f*y, -1.0f*z);
        glVertex3f( 1.0f*x, -1.0f*y, -1.0f*z);
        // Top Face
        glNormal3f(0,1,0);
        glVertex3f(-1.0f*x,  1.0f*y, -1.0f*z);
        glVertex3f(-1.0f*x,  1.0f*y,  1.0f*z);
        glVertex3f( 1.0f*x,  1.0f*y,  1.0f*z);
        glVertex3f( 1.0f*x,  1.0f*y, -1.0f*z);
        // Bottom Face
        glNormal3f(0,-1,0);
        glVertex3f(-1.0f*x, -1.0f*y, -1.0f*z);
        glVertex3f( 1.0f*x, -1.0f*y, -1.0f*z);
        glVertex3f( 1.0f*x, -1.0f*y,  1.0f*z);
        glVertex3f(-1.0f*x, -1.0f*y,  1.0f*z);
        // Right face
        glNormal3f(1,0,0);
        glVertex3f( 1.0f*x, -1.0f*y, -1.0f*z);
        glVertex3f( 1.0f*x,  1.0f*y, -1.0f*z);
        glVertex3f( 1.0f*x,  1.0f*y,  1.0f*z);
        glVertex3f( 1.0f*x, -1.0f*y,  1.0f*z);
        // Left Face
        glNormal3f(-1,0,0);
        glVertex3f(-1.0f*x, -1.0f*y, -1.0f*z);
        glVertex3f(-1.0f*x, -1.0f*y,  1.0f*z);
        glVertex3f(-1.0f*x,  1.0f*y,  1.0f*z);
        glVertex3f(-1.0f*x,  1.0f*y, -1.0f*z);
	glEnd();
}

void DrawCitySubDivision(City c){
    for(int i = abs(user.x)-DIVISION_SPACE; i < abs(user.x)+DIVISION_SPACE; i++){
        for(int j = abs(user.z)-DIVISION_SPACE; j < abs(user.z)+DIVISION_SPACE; j++){
            int objHeight = c.form[i][j];

            if( objHeight > 0){
                if(j%2 == 0){
                    glColor3f(255.0, 0.0, 0.0);
                } else {
                    glColor3f(0.0, 0.0, 255.0);
                }

                VOP Position;
                Position.x = -i;
                Position.y = objHeight/2.0;
                Position.z = -j;

                glPushMatrix();
                    glTranslatef(Position.x, Position.y, Position.z);
                    glScalef(1, objHeight, 1);
                    DrawObject(0.5, 0.5, 0.5);
                glPopMatrix();
            }
        }
    }
}

void DrawCity(City c){

    for(int i = 0; i < c.width; i++){
        for(int j = 0; j < c.depth; j++){
            int objHeight = c.form[i][j];

            if( objHeight > 0){
                if(j%2 == 0){
                    glColor3f(255.0, 0.0, 0.0);
                } else {
                    glColor3f(0.0, 0.0, 255.0);
                }

                VOP Position;
                Position.x = -i;
                Position.y = objHeight/2.0;
                Position.z = -j;

                glPushMatrix();
                    glTranslatef(Position.x, Position.y, Position.z);
                    glScalef(1, objHeight, 1);
                    if(CULLING_INTERSECTION){
                        if(calculateIntersection(Position)){
                            DrawObject(0.5, 0.5, 0.5);
                        }

                    } else {
                        DrawObject(0.5, 0.5, 0.5);
                    }
                glPopMatrix();
            }
        }
    }
}

void PosicUser(){

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90,AspectRatio,0.01,200);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(user.x, user.y, user.z,
		      target.x, target.y, target.z,
			  0.0f,1.0f,0.0f);
}

void reshape( int w, int h ){

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0)
		h = 1;

	AspectRatio = 1.0f * w / h;
	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	// Set the viewport to be the entire window
    glViewport(0, 0, w, h);

	PosicUser();

}

void display( void ){

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	DefineLuz();

	PosicUser();

	glMatrixMode(GL_MODELVIEW);

    if(CULLING_SUBDIVISION){
        DrawCitySubDivision(simCity);
    } else {
        DrawCity(simCity);
    }

	glutSwapBuffers();
}

void animate(){
    static float dt;
    static float AccumTime=0;

#ifdef _WIN32
    DWORD time_now;
    time_now = GetTickCount();
    dt = (float) (time_now - last_idle_time) / 1000.0;
#else
    struct timeval time_now;
    gettimeofday(&time_now, NULL);
    dt = (float)(time_now.tv_sec  - last_idle_time.tv_sec) +
    1.0e-6*(time_now.tv_usec - last_idle_time.tv_usec);
#endif
    AccumTime +=dt;
    if (AccumTime >=3)
    {
        cout << 1.0/dt << " FPS"<< endl;
        AccumTime = 0;
    }

    last_idle_time = time_now;

    glutPostRedisplay();
}

Pos subtractPoints(Pos x, Pos y) {
    Pos ans;
    ans.x = x.x - y.x;
    ans.y = x.y - y.y;
    ans.z = x.z - y.z;
    return ans;
}

Pos sum(Pos x, Pos y) {
    Pos ans;
    
    ans.x = x.x + y.x;
    ans.y = x.y + y.y;
    ans.z = x.z + y.z;
    
    return ans;
}

void keyboard ( unsigned char key, int x, int y ){
    
    cout << "Obs: " << user.x << " " << user.y << " " << user.z << endl;
    cout << "Target:" << target.x << " " << target.y << " " << target.z << " " << endl;
    cout << user.alpha << endl;
    
    
    int difX = (target.x - user.x) * 0.2;
    int difY = (target.y - user.y) * 0.2;
    int difZ = (target.z - user.z) * 0.2;
    
    Pos VETOR_ALVO;
    Pos NOVO_ALVO;
    
    float ALPHA_RADIANS;
    
	switch ( key ){
        case 27:
            exit(0);
            break;
        case 'w':
            user.x = user.x + difX;
            user.y = user.y + difY;
            user.z = user.z + difZ + 1;
            
            target.x = target.x + difX;
            target.y = target.y + difY;
            target.z = target.z + difZ + 1;
            
            glutPostRedisplay();
            break;
        case 's':
            user.x = user.x - difX;
            user.y = user.y - difY;
            user.z = user.z - difZ - 1;
            
            target.x = target.x - difX;
            target.y = target.y - difY;
            target.z = target.z - difZ - 1;
            
            glutPostRedisplay();
            break;
        case 'd':
            VETOR_ALVO = subtractPoints(target, user);
            
            user.alpha = (user.alpha-1) % 360;
            if (user.alpha<0) { user.alpha = 360; }
            
            ALPHA_RADIANS = radians(user.alpha);
            
            NOVO_ALVO.x = VETOR_ALVO.x*cos(ALPHA_RADIANS) + VETOR_ALVO.z*sin(ALPHA_RADIANS);
            NOVO_ALVO.y = VETOR_ALVO.y;
            NOVO_ALVO.z = -VETOR_ALVO.x*cos(ALPHA_RADIANS) + VETOR_ALVO.z*cos(ALPHA_RADIANS);
            
            target = sum(user, NOVO_ALVO);
            
            glutPostRedisplay();
            break;
        case 'a':
            VETOR_ALVO = subtractPoints(target, user); // vetor obs-alvo
            
            user.alpha = (user.alpha+1) % 360;
            ALPHA_RADIANS = radians(user.alpha);
            
            NOVO_ALVO.x = VETOR_ALVO.x*cos(ALPHA_RADIANS) + VETOR_ALVO.z*sin(ALPHA_RADIANS);
            NOVO_ALVO.y = VETOR_ALVO.y;
            NOVO_ALVO.z = -VETOR_ALVO.x*cos(ALPHA_RADIANS) + VETOR_ALVO.z*cos(ALPHA_RADIANS);
            
            target = sum(user, NOVO_ALVO);
            
            glutPostRedisplay();
            break;
        case '+':
            USER_RADIUS += 1;
            glutPostRedisplay();
            break;
        case '-':
            USER_RADIUS -= 1;
            glutPostRedisplay();
            break;
        default:
            cout << key;
          break;
      }
}

void arrow_keys ( int a_keys, int x, int y ){
    int JUMP = 5;
	switch ( a_keys )
	{
		case GLUT_KEY_UP:
			user.z -= JUMP;
            target.z -= JUMP;
            glutPostRedisplay();
            break;
	    case GLUT_KEY_DOWN:
            user.z += JUMP;
            target.z += JUMP;
            glutPostRedisplay();
            break;
        case GLUT_KEY_RIGHT:
            user.x += JUMP;
            target.x += JUMP;
            glutPostRedisplay();
            break;
        case GLUT_KEY_LEFT:
            user.x -= JUMP;
            target.x -= JUMP;
            glutPostRedisplay();
            break;
		default:
			break;
	}
}

void init(void){
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glShadeModel(GL_SMOOTH);
	glColorMaterial (GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_DEPTH_TEST);
	//glEnable (GL_CULL_FACE);

    // Obtem o tempo inicial
    #ifdef WIN32
        last_idle_time = GetTickCount();
    #else
        gettimeofday (&last_idle_time, NULL);
    #endif

    readColors("colors.txt");
    readCity("city01.txt");
}

int main(int argc, char** argv){
	glutInit            ( &argc, argv );
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );

	glutInitWindowPosition (0,0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("SimCity");

	init ();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(arrow_keys);
	glutIdleFunc(animate);

	glutMainLoop();

	return 0;
}
