#pragma once
#include <math.h>
#include <vector>
#include "graph.h"



class Circle {
private:
	Point coords;
	double r;
	int col[3];
public:
	Circle();
	Circle(double x, double y, double rad);
	Point get_point();
	double get_r();
	int* get_color();
	void set_color(int color[3]);
	void move_to(double x, double y);
	void move_by(double x, double y);
	void set_r(double rad);
};