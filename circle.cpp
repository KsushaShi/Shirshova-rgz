#include "circle.h";


Circle::Circle() {
	coords.x = coords.y = 50;
	r = 15;
	col[2] = 0;
	col[0] = col[1] = 255;
}

Circle::Circle(double x, double y, double rad) {
	coords.x = x;
	coords.y = y;
	r = rad;
	col[0] = 200;
	col[1] = col[2] = 50;
}

Point Circle::get_point() {
	return coords;
}

double Circle::get_r() {
	return r;
}

int* Circle::get_color() {
	return col;
}

void Circle::set_color(int color[3]) {
	col[0] = color[0];
	col[1] = color[1];
	col[2] = color[2];
}

void Circle::move_to(double x, double y) {
	coords.x = x;
	coords.y = y;
}

void Circle::move_by(double x, double y) {
	coords.x += x;
	coords.y += y;
}

void Circle::set_r(double rad) {
	r = rad;
}