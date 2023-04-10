/*
* Napisati kratko uputstvo za koriscenje igrice
*
*/

#include <math.h>		
#include <stdio.h>
#include <stdlib.h>		
#include "Glut.h"
#include <time.h>





constexpr auto FPS = 80.0;



int frame_count = 0;
float fps = 0.0;
int init_time = time(NULL);
int final_time = 0;


int run_mode = 1;

const double Xmin = 0.0, Xmax = 3.0;
const double Ymin = 0.0, Ymax = 3.0;

float x_speed = 0.01;
float y_speed = -0.01;

float i = 0.0f;


float M_PI = 3.14;
int j = 0;


bool up_key_pressed = false;
bool down_key_pressed = false;
bool right_key_pressed = false;
bool left_key_pressed = false;

bool is_colliding = false;




class Player {
public:
	float x, y;  // position
	float speed; // movement speed
	float accel = 5; // accelleration
	float velocity_x, velocity_y; // velocity
	float collition_width = 0.2, collition_height = 0.2;
	float gravity = -0.01f;

	Player(float start_x, float start_y, float player_speed) {
		x = start_x;
		y = start_y;
		speed = player_speed;
	}

	void move(float dt, float dx, float dy) {



		// Update the velocity based on the acceleration
		velocity_x += dx * accel * dt;
		velocity_y += dy * accel * dt;

		float friction = 0.01f;
		velocity_x -= friction * velocity_x * dt;
		velocity_y -= friction * velocity_y * dt;

		// Add gravity to the velocity_y
		velocity_y += gravity * dt;

		// Limit the velocity to the maximum speed
		float velocity_mag = sqrt(velocity_x * velocity_x + velocity_y * velocity_y);
		if (velocity_mag > speed) {
			velocity_x *= speed / velocity_mag;
			velocity_y *= speed / velocity_mag;
		}



		x += velocity_x * dt;
		y += velocity_y * dt;
	}


	void draw() {

		glColor3f(1, 0, 0);
		glRectf(x - 0.1, y - 0.1, x + 0.1, y + 0.1);
	}
};

class Box {
public:
	float x, y;
	float width, height;
	float rc = 1, gc = 0, bc = 1;

	Box() {
		x = 0;
		y = 0;
		width = 0.3;
		height = 0.3;
	}

	Box(float start_x, float start_y, float box_width, float box_height) {
		x = start_x;
		y = start_y;
		width = box_width;
		height = box_height;
	}

	Box(Player player) {
		x = player.x - (player.collition_width / 2);
		y = player.y - (player.collition_height / 2);
		width = player.collition_width;
		height = player.collition_height;

	}

	void draw(void) {
		glColor3f(rc, gc, bc);
		glRectf(x, y, x + width, y + height);
	}

	bool overlaps(Box other) {
		return x < other.x + other.width && x + width > other.x
			&& y < other.y + other.height && y + height > other.y;
	}
};


Player player(1.5, 1.5, 0.0007);
Box player_collition(player);

Box boxes[8];





void updateFPS() {
	final_time = time(NULL);
	frame_count++;
	if (final_time - init_time > 0) {
		fps = frame_count / (final_time - init_time);
		char title[128];
		sprintf_s(title, "Projektni zadatak - FPS: %.0f", fps);
		glutSetWindowTitle(title);

		init_time = final_time;
		frame_count = 0;
	}
}

void myKeyboardFunc(unsigned char key, int x, int y)
{
	switch (key) {
	case 'w':
		up_key_pressed = true;
		break;
	case 's':
		down_key_pressed = true;
		break;
	case 'a':
		left_key_pressed = true;
		break;
	case 'd':
		right_key_pressed = true;
		break;
	}

}

void input_released(unsigned char key, int x, int y) {

	switch (key) {
	case 'w':
		up_key_pressed = false;
		break;
	case 's':
		down_key_pressed = false;
		break;
	case 'a':
		left_key_pressed = false;
		break;
	case 'd':
		right_key_pressed = false;
		break;
	}
}

void mySpecialKeyFunc(int key, int x, int y)
{

}


void draw_player(void) {

}

bool checkCollision(float player_x, float player_y, float player_w, float player_h, float box_x, float box_y, float box_w, float box_h) {
	// Calculate the half-widths and half-heights of the player and the box
	float player_half_w = player_w / 2;
	float player_half_h = player_h / 2;
	float box_half_w = box_w / 2;
	float box_half_h = box_h / 2;

	// Calculate the centers of the player and the box
	float player_center_x = player_x;
	float player_center_y = player_y;
	float box_center_x = box_x + box_half_w;
	float box_center_y = box_y + box_half_h;

	// Calculate the minimum and maximum distances between the centers of the player and the box
	float dx = fabs(player_center_x - box_center_x);
	float dy = fabs(player_center_y - box_center_y);
	float min_dist_x = player_half_w + box_half_w;
	float min_dist_y = player_half_h + box_half_h;

	// Check if there is an overlap between the player and the box
	if (dx <= min_dist_x && dy <= min_dist_y) {
		// Calculate the amount to move the player out of the box
		float overlap_x = min_dist_x - dx;
		float overlap_y = min_dist_y - dy;

		// Move the player out of the box
		if (overlap_x < overlap_y) {
			if (player_center_x < box_center_x) {
				player.x -= overlap_x;
				player.velocity_x = 0;
			}
			else {
				player.x += overlap_x;
				player.velocity_x = 0;
			}
		}
		else {
			if (player_center_y < box_center_y) {
				player.y -= overlap_y;
				player.velocity_y = 0;
			}
			else {
				player.y += overlap_y;
				player.velocity_y = 0;
			}
		}

		// Collision detected
		return true;
	}

	// No collision
	return false;
}


void update(int) {

	// Compute dt (time since last frame)
	static double prev_time = 0;
	double current_time = glutGet(GLUT_ELAPSED_TIME);
	double dt = current_time - prev_time;
	prev_time = current_time;

	float dx = 0, dy = 0;
	if (left_key_pressed) dx -= 0.01;
	if (right_key_pressed) dx += 0.01;
	if (down_key_pressed) dy -= 0.01;
	if (up_key_pressed) dy += 0.01;

	player.move(dt, dx, dy);

	for (int j = i + 1; j < 8; j++) {
		if (checkCollision(player.x + dx, player.y + dy, player.collition_width, player.collition_height, boxes[j].x, boxes[j].y, boxes[j].width, boxes[j].height)) {
			boxes[j].rc = 1;
			boxes[j].gc = 1;
			boxes[j].bc = 1;
		}
		else {
			boxes[j].rc = 1;
			boxes[j].gc = 0;
			boxes[j].bc = 1;
		}
	}

	if (run_mode == 1) {
		glutPostRedisplay();	// Trigger an automatic redraw for animation
		//j++;
	}
	glutTimerFunc(1000 / FPS, update, 1);

}



void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	player_collition.x = player.x - (player.collition_width / 2);
	player_collition.y = player.y - (player.collition_height / 2);
	player_collition.draw();



	for (int i = 0; i < 8; i++) {
		boxes[i].draw();
	}


	//glRectf(0, 0, 3, 1); 
	/*glColor3f(1, 0, 1);
	glRectf(boxes[0].x, boxes[0].y, boxes[0].x + boxes[0].width, boxes[0].y + boxes[0].height);

	glColor3f(1, 0, 1);
	glRectf(0.3, 1.4, 0.5, 1.6);

	glColor3f(1, 0, 1);
	glRectf(0.6, 2.6, 0.7, 2.9);*/

	glBegin(GL_POINTS);
	glColor3f(0, 0, 0);
	glVertex2f(player.x, player.y);
	glEnd();


	glPushMatrix();

	player.draw();

	glPopMatrix();
	// Flush the pipeline, swap the buffers
	glFlush();
	glutSwapBuffers();
	updateFPS();

}

void initRendering()
{
	glShadeModel(GL_FLAT);	// The default value of GL_SMOOTH is usually better - flat or smooth shading
	glEnable(GL_DEPTH_TEST);	// Depth testing must be turned on
}

void resizeWindow(int w, int h)
{
	double scale, center;
	double windowXmin, windowXmax, windowYmin, windowYmax;

	glViewport(0, 0, w, h);

	w = (w == 0) ? 1 : w;
	h = (h == 0) ? 1 : h;
	if ((Xmax - Xmin) / w < (Ymax - Ymin) / h) {
		scale = ((Ymax - Ymin) / h) / ((Xmax - Xmin) / w);
		center = (Xmax + Xmin) / 2;
		windowXmin = center - (center - Xmin) * scale;
		windowXmax = center + (Xmax - center) * scale;
		windowYmin = Ymin;
		windowYmax = Ymax;
	}
	else {
		scale = ((Xmax - Xmin) / w) / ((Ymax - Ymin) / h);
		center = (Ymax + Ymin) / 2;
		windowYmin = center - (center - Ymin) * scale;
		windowYmax = center + (Ymax - center) * scale;
		windowXmin = Xmin;
		windowXmax = Xmax;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(windowXmin, windowXmax, windowYmin, windowYmax, -1, 1);

}


int main(int argc, char** argv)
{


	boxes[0] = Box(0, 0, 3, 0.1);
	boxes[1] = Box(0.3, 1.4, 0.2, 0.2);
	boxes[2] = Box(0.7, 2.5, 0.5, 0.3);
	boxes[3] = Box(0, 2.9, 3, 0.1);  // screen edges
	boxes[4] = Box(0, 0, 0.1, 3);	// screen edges
	boxes[5] = Box(0, 0, 3, 0.1);   // screen edges
	boxes[6] = Box(2.9, 0, 0.1, 3); // screen edges
	boxes[7] = Box(0, 1, 3, 0.1);

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowPosition(10, 60);
	glutInitWindowSize(360, 360);

	//preimenovati u Kolokvijum_ime_prezime (npr. Kolokvijum_Tijana_Sustersic)

	glutCreateWindow("Projektni zadatak");

	initRendering();

	glutKeyboardFunc(myKeyboardFunc);
	glutSpecialFunc(mySpecialKeyFunc);
	glutKeyboardUpFunc(input_released);

	glutReshapeFunc(resizeWindow);


	glutDisplayFunc(drawScene);

	glutTimerFunc(1000 / FPS, update, 1);

	glutMainLoop();

	return(0);
}

