/*
* Napisati kratko uputstvo za koriscenje igrice
*
*/

#include <math.h>		
#include <stdio.h>
#include <stdlib.h>		
#include "Glut.h"
#include <time.h>
#include "RgbImage.h"
#include <vector>
#include <iterator>
#include <algorithm>


constexpr auto FPS = 80.0;

static GLuint textureName[4];

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
bool player_shoot_key = false;

bool is_colliding = false;
bool check_coll_called_move_towards = false;



class Player {
public:
	float x, y;  // position
	float speed; // movement speed
	float accel = 5; // accelleration
	float velocity_x, velocity_y; // velocity
	float collition_width = 0.2, collition_height = 0.2;
	float gravity = -0.0003f;
	bool jump = false;
	int jump_amount = 0;
	int jump_steps_after = 0;
	int jump_steps_after_duration = 16;
	float jump_height = 0.017;
	int jump_amount_max = 2;
	float jump_force = 0.02;
	int dir = 1; // 1 right -1 left

	float min_speed_y = 0.001;
	int on_ground = 0;


	Player(float start_x, float start_y, float player_speed) {
		x = start_x;
		y = start_y;
		speed = player_speed;
	}

	void move(float dt, float dx, float dy) {

		// Update the velocity based on the acceleration
		velocity_x += dx * accel * dt;
		velocity_y += dy * accel * dt;

		// Add friction to movment
		float friction = 0.01f;
		velocity_x -= friction * velocity_x * dt;
		velocity_y -= friction * velocity_y * dt;

		// Add gravity to the velocity_y
		velocity_y += gravity * dt;
		
		// Limit the velocity to the maximum speed
		velocity_x = std::clamp(velocity_x, -speed, speed);
		velocity_y = std::clamp(velocity_y, -min_speed_y, jump_height);
		
		x += velocity_x * dt;
		y += velocity_y * dt;
		
	}

	void move_towards(float target_x, float target_y) {
		// Calculate the vector from current position to target position
		float dx = target_x - x;
		float dy = target_y - y;
		float distance = sqrt(dx * dx + dy * dy);

		// Calculate the movement needed to reach the target position
		if (distance <= speed) {
			// If the distance is less than or equal to the speed, we can reach the target position in one step
			return;
		}
		else {
			// Otherwise, we need to move towards the target position by the specified speed
			float ratio = (speed ) / distance;
			
			x = x + dx * ratio;
			y = y + dy * ratio;
			
			move_towards(target_x, target_y);
		}
	}

	void draw() {

		glColor3f(1, 0, 0);
		glRectf(x - 0.1, y - 0.1, x + 0.1, y + 0.1);
	}
};

class Enemy {

public:
	float x, y;  // position
	float speed; // movement speed
	float accel = 5; // accelleration
	float velocity_x, velocity_y; // velocity
	float collition_width = 0.2, collition_height = 0.2;
	float gravity = -0.1f;
	int dir = 1; // 1 up -1 down // for tree enemy
	int type;
	bool is_alive = 1;

	Enemy(float start_x, float start_y, float enemy_speed, int type_e) {
		x = start_x;
		y = start_y;
		speed = enemy_speed;
		//type side enemy going up down on trees 0
		//type walking enemy going left right 1
		type = type_e;
		if (type == 0) {
			gravity = 0.0;
		}
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

	bool checkCollision(float box_x, float box_y, float box_w, float box_h) {
		// Calculate the half-widths and half-heights of the player and the box
		float half_w = collition_width / 2;
		float half_h = collition_height / 2;
		float box_half_w = box_w / 2;
		float box_half_h = box_h / 2;

		// Calculate the centers of the player and the box
		float player_center_x = x;
		float player_center_y = y;
		float box_center_x = box_x + box_half_w;
		float box_center_y = box_y + box_half_h;

		// Calculate the minimum and maximum distances between the centers of the player and the box
		float dx = fabs(player_center_x - box_center_x);
		float dy = fabs(player_center_y - box_center_y);
		float min_dist_x = half_w + box_half_w;
		float min_dist_y = half_h + box_half_h;

		// Check if there is an overlap between the player and the box
		if (dx <= min_dist_x && dy <= min_dist_y) {
			// Calculate the amount to move the player out of the box
			float overlap_x = min_dist_x - dx;
			float overlap_y = min_dist_y - dy;

			// Move the player out of the box
			if (overlap_x < overlap_y) {
				if (player_center_x < box_center_x) {
					x -= overlap_x;
					velocity_x = 0;
					dir = -1;
				}
				else {
					x += overlap_x;
					velocity_x = 0;
					dir = 1;
				}
			}
			else {
				if (player_center_y < box_center_y) {
					y -= overlap_y;
					velocity_y = 0;
				}
				else {
					y += overlap_y;
					velocity_y = 0;
				}
			}

			// Collision detected
			return true;
		}

		// No collision
		return false;
	}

	void draw() {
		glColor3f(1, 0, 0);
		glRectf(x - 0.1, y - 0.1, x + 0.1, y + 0.1);
	}
};

class Projectile {
public:
	float x, y;  // position
	float speed; // movement speed
	float collition_width = 0.05, collition_height = 0.05;
	int dir = 1; // 1 right -1 left

	Projectile(float start_x, float start_y, float projectile_speed, int direction) {
		x = start_x;
		y = start_y;
		speed = projectile_speed;
		dir = direction;
	}

	void move(void) {
		x += dir * speed;
	}

	void draw() {
		glColor3f(1, 1, 0);
		glRectf(x, y, x + collition_width, y +collition_height);
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

	Box(Enemy enemy) {
		x = enemy.x - (enemy.collition_width / 2);
		y = enemy.y - (enemy.collition_height / 2);
		width = enemy.collition_width;
		height = enemy.collition_height;
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



Player player(1.5, 2, 0.0007);
Box player_collition(player);

Enemy enemy_1(2.85, 1.5, 0.0003, 0);
Box enemy_1_collition(enemy_1);

Enemy enemy_2(0.15, 1.5, 0.0003, 0);
Box enemy_2_collition(enemy_2);

Enemy enemy_3(1.1, 1.5, 0.001, 1);
Box enemy_3_collition(enemy_3);

Box boxes[8];
Enemy* enemies[3] = {&enemy_1, &enemy_2, &enemy_3};


std::vector<Projectile*> projectiles = {};
std::vector<Projectile*> player_projectiles = {};




void loadTextureFromFile(const char* filename)
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	//glShadeModel(GL_FLAT);
	//glEnable(GL_DEPTH_TEST);

	RgbImage theTexMap(filename);

	// Pixel alignment: each row is word aligned.  Word alignment is the default. 
	// glPixelStorei(GL_UNPACK_ALIGNMENT, 4);		

	// Set the interpolation settings to best quality.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB,
		theTexMap.GetNumCols(), theTexMap.GetNumRows(),
		GL_RGB, GL_UNSIGNED_BYTE, theTexMap.ImageData());

}

void initFour(const char* filenames[])
{
	glGenTextures(4, textureName);	// Load four texture names into array
	for (int i = 0; i < 4; i++) {
		glBindTexture(GL_TEXTURE_2D, textureName[i]);	// Texture #i is active now
		loadTextureFromFile(filenames[i]);			// Load texture #i
	}
}

void drawTextureQuad(int i) {
	glBindTexture(GL_TEXTURE_2D, textureName[i]);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, 1.0, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(1.0, 1.0, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(1.0, -1.0, 0.0);
	glEnd();

}

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
		player.dir = -1;
		break;
	case 'd':
		right_key_pressed = true;
		player.dir = 1;
		break;
	case 'k':
		player_shoot_key  = true;
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
				if (!check_coll_called_move_towards) {
					player.x -= overlap_x;
					player.velocity_x = 0;
				}
			}
			else {
				if (!check_coll_called_move_towards) {
					player.x += overlap_x;
					player.velocity_x = 0;
				}
			}
		}
		else {
			if (player_center_y < box_center_y) {
				if (!check_coll_called_move_towards) {
					player.y -= overlap_y;
					player.velocity_y = 0;
				}
			}
			else {
				if (!check_coll_called_move_towards) {
					player.y += overlap_y;
					player.velocity_y = 0;
					player.on_ground = true;
				}
			}
		}
		// Collision detected
		return true;
	}
	else {
		return false;
	}
	// No collision
	player.on_ground = false;
}

void move_towards(float target_x, float target_y) {
	// Calculate the vector from current position to target position
	float dx = target_x - player.x;
	float dy = target_y - player.y;
	float distance = sqrt(dx * dx + dy * dy);

	// Calculate the movement needed to reach the target position
	if (distance <= player.speed) {
		// If the distance is less than or equal to the speed, we can reach the target position in one step
		check_coll_called_move_towards = false;
		return;
	}
	else {
		// Otherwise, we need to move towards the target position by the specified speed
		float ratio = (player.speed) / distance;
		if (!player.on_ground) {
			for (int i = 0; i < 8; i++) {
				check_coll_called_move_towards = false;
				if (checkCollision(player.x + dx * ratio, player.y + dy * ratio, player.collition_width, player.collition_height, boxes[j].x, boxes[j].y, boxes[j].width, boxes[j].height)) {
					check_coll_called_move_towards = false;
					return;
				}
			}
		}
		player.x = player.x + dx * ratio;
		player.y = player.y + dy * ratio;
		move_towards(target_x, target_y);
		

		
	}
}

void update(int) {  //something like physics proccess in godot

	// Compute dt (time since last frame)
	static double prev_time = 0;
	double current_time = glutGet(GLUT_ELAPSED_TIME);
	double dt = current_time - prev_time;
	prev_time = current_time;
	bool old_ground = player.on_ground;
	
	float dx = 0, dy = 0;
	
	if (left_key_pressed) dx -= 0.01;
	if (right_key_pressed) dx += 0.01;
	//if (down_key_pressed) dy -= 0.01;
	//if (up_key_pressed) dy += 0.01;
	
	for (int i = 0; i < 8; i++) {
		checkCollision(player.x + dx, player.y + dy, player.collition_width, player.collition_height, boxes[j].x, boxes[j].y, boxes[j].width, boxes[j].height);
	}

	if (!player.on_ground) {
		if (player.jump_steps_after > 0)
			player.jump_steps_after -= 1;
	}
	else if (player.jump_steps_after != player.jump_steps_after_duration)
		player.jump_steps_after = player.jump_steps_after_duration;

	// Ground jump:
		if (up_key_pressed) {
			if (player.on_ground or player.jump_steps_after > 0) {
				move_towards(player.x + dx, player.y + player.jump_height);
				dy += 0.01;
				player.on_ground = false;
				player.jump_amount = 1;
				player.jump_steps_after = 0;

				// Air jump ("double-jump"):
			}
			else {
				printf("%i\n", player.jump_amount++);
				if (player.jump_amount++ <= player.jump_amount_max) {

					move_towards(player.x + dx, player.y + player.jump_height);
					dy += 0.01;
					printf(" ");
				}
			}
			up_key_pressed = false;
		}

	player.move(dt, dx, dy);

	for (int j = 0; j < 8; j++) {
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

	if (player_shoot_key) {
		Projectile* project_inst_1 = new Projectile(player.x, player.y, 0.01, player.dir);
		player_projectiles.push_back(project_inst_1);
		player_shoot_key = false;
	}
	
	for (int i = 0; i < sizeof(enemies) / sizeof(enemies[0]); i++) {
		float dxe = 0, dye = 0;
		if (enemies[i]->is_alive) {
			if (enemies[i]->type == 0) {
				dye += enemies[i]->dir * 0.01;
				if (enemies[i]->y + (enemies[i]->collition_height / 2) + dye > Ymax || (enemies[i]->y - (enemies[i]->collition_height / 2) + dye < Ymin)) {
					enemies[i]->dir = enemies[i]->dir * -1;
				}
				if (enemies[i]->y > player.y - 0.02 && enemies[i]->y < player.y + 0.02) {
					if (enemies[i]->x > player.x) {
						Projectile* project_inst = new Projectile(enemies[i]->x, enemies[i]->y, 0.008, -1);
						projectiles.push_back(project_inst);
					}
					else {
						Projectile* project_inst = new Projectile(enemies[i]->x, enemies[i]->y, 0.008, 1);
						projectiles.push_back(project_inst);
					}
				}
			}
			if (enemies[i]->type == 1) {
				dxe += enemies[i]->dir * 0.01;
				if (enemies[i]->x + (enemies[i]->collition_width / 2) + dxe > Xmax || (enemies[i]->x - (enemies[i]->collition_width / 2) + dxe < Xmin)) {
					enemies[i]->dir = enemies[i]->dir * -1;
				}
			}

			enemies[i]->move(dt, dxe, dye);

			if (enemies[i]->type == 1) {
				for (j = 0; j < sizeof(boxes) / sizeof(boxes[0]); j++) {
					enemies[i]->checkCollision(boxes[j].x, boxes[j].y, boxes[j].width, boxes[j].height);
					if (checkCollision(player.x, player.y, player.collition_width, player.collition_height, enemies[i]->x - (enemies[i]->collition_width / 2), enemies[i]->y - (enemies[i]->collition_height / 2), enemies[i]->collition_width, enemies[i]->collition_height)) {
						enemies[i]->is_alive = false; //logic for "combat"
					}
				}
			}
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

	glPushMatrix();

	for (int i = 0; i < sizeof(enemies) / sizeof(enemies[0]); i++) {
		if (enemies[i]->is_alive) {
			enemies[i]->draw();
		}
	}

	glPopMatrix();

	for (int i = 0; i < 8; i++) {  //level
		boxes[i].draw();
	}

	glBegin(GL_POINTS);
	glColor3f(0, 0, 0);
	glVertex2f(player.x, player.y);
	glEnd();

	glPushMatrix();

	player.draw();

	glPopMatrix();

	glPushMatrix();
	if (!player_projectiles.empty()) {
		for (auto it = player_projectiles.begin(); it != player_projectiles.end();) {
			int index = std::distance(player_projectiles.begin(), it);
			(*it)->move();
			(*it)->draw();
			float off = (*it)->speed * (*it)->dir;
			bool projectile_deleted = false;
			if ((*it)->x + off > Xmax || (*it)->x + off < Xmin) {
				delete (*it);
				it = player_projectiles.erase(it);
				projectile_deleted = true;
			}
			else {
				for (int i = 0; i < sizeof(enemies) / sizeof(enemies[0]); i++) {
					if (enemies[i]->is_alive) {
						if (enemies[i]->checkCollision((*it)->x + off, (*it)->y, (*it)->collition_width, (*it)->collition_height)) {
							enemies[i]->is_alive = false;
							delete (*it);
							it = player_projectiles.erase(it);
							projectile_deleted = true;
							break;
						}
					}
				}
			}
			if (!projectile_deleted) {
				++it;
			}
		}
	}

	glPopMatrix();

	glPushMatrix();

	if (!projectiles.empty()) {
		for (auto it = projectiles.begin(); it != projectiles.end();) {
			int index = std::distance(projectiles.begin(), it);
			(*it)->move();
			(*it)->draw();
			float off = (*it)->speed * (*it)->dir;
			bool projectile_deleted = false;
			if ((*it)->x + off > Xmax || (*it)->x + off < Xmin) {
				delete (*it);
				it = projectiles.erase(it);
				projectile_deleted = true;
			}
			else if (checkCollision(player.x, player.y, player.collition_width, player.collition_height, (*it)->x + off, (*it)->y, (*it)->collition_width, (*it)->collition_height)) {
				delete (*it);
				it = projectiles.erase(it);
				projectile_deleted = true;
			}
			if (!projectile_deleted) {
				++it;
			}
		}
	}

	glPopMatrix();

	
	// Flush the pipeline, swap the buffers
	glFlush();
	//glDisable(GL_TEXTURE_2D);
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

const char* filenameArray[4] = {
		"kangaro_idle-export.bmp",
		"LightningTexture.bmp",
		"IvyTexture.bmp",
		"RedLeavesTexture.bmp"
};

int main(int argc, char** argv)
{

	boxes[0] = Box(0, 0, 3, 0.1);
	boxes[1] = Box(0.3, 1.4, 0.2, 0.2);
	boxes[2] = Box(0.7, 2.5, 0.5, 0.3);
	boxes[3] = Box(0, 2.9, 3, 0.1);  // screen edges
	boxes[4] = Box(0, 0, 0.3, 3);	// screen edges
	boxes[5] = Box(0, 0, 3, 0.1);   // screen edges
	boxes[6] = Box(2.7, 0, 0.3, 3); // screen edges
	boxes[7] = Box(0, 1, 3, 0.1);

	//Projectile*  project_inst = new Projectile(1.7, 1.7, 0.001, -1);
	//projectiles.push_back(project_inst);

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowPosition(10, 60);
	glutInitWindowSize(360, 360);

	glutCreateWindow("Projektni zadatak");
	
	initRendering();
	//initFour(filenameArray);
	glutKeyboardFunc(myKeyboardFunc);
	glutSpecialFunc(mySpecialKeyFunc);
	glutKeyboardUpFunc(input_released);

	glutReshapeFunc(resizeWindow);


	glutDisplayFunc(drawScene);

	glutTimerFunc(1000 / FPS, update, 1);

	glutMainLoop();

	return(0);
}

