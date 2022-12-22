#pragma once

#include <map>
#include <vector>
#include <math.h>

struct Point {
    double x, y;
    bool operator==(const Point& p) const;
    bool operator!=(const Point& p) const;
    bool operator<(const Point& p) const;
    bool operator<=(const Point& p) const;
};

double distance(Point &p1, Point &p2);

class Graph {
public:
    void add_edge(Point start_vertex, Point end_vertex);
    void delete_edge(Point start_vertex, Point end_vertex);
    std::vector<Point> get_vertices() const;
    std::vector<Point> get_adjacent_vertices(Point src_vertex);
private:
    std::map<Point, std::vector<Point>> vertices;
}; //
