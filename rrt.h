#pragma once
#include "circle.h"
#include <random>
#include <queue>

extern int right_border, bottom_border;
extern double robot_r;
extern std::vector<Circle> obstacles;

double getDouble(const double B, const double A = 0);
Point random_point();
Point nearest(Point& a, Point& b, Point& c);
bool collision(Point& a, Point& b, Circle& c);
Point steer(Point& a, Point& b);
Graph rrt(int n, Point start, Point goal);
std::vector<Point> find_path(Graph& g, Point& start, Point& end);