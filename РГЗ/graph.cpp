#include "graph.h"

#include <stdexcept>

using namespace std;

namespace {
    static const double EPS = 1e-8;

    bool equal(double a, double b) {
        return abs(a - b) < EPS;
    }
}

bool Point::operator==(const Point& p) const {
    return equal(x, p.x) && equal(y, p.y);
}

bool Point::operator!=(const Point& p) const {
    return !equal(x, p.x) || !equal(y, p.y);
}

bool Point::operator<(const Point& p) const {
    return (x < p.x) || (equal(x, p.x) && y < p.y);
}

bool Point::operator<=(const Point& p) const {
    return (x < p.x) || (equal(x, p.x) && ((y < p.y) || equal(y, p.y)));
}

double distance(Point &p1, Point &p2) {
    return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

void Graph::add_edge(Point start_vertex, Point end_vertex) {
    if (start_vertex != end_vertex) vertices[start_vertex].push_back(end_vertex);
}

void Graph::delete_edge(Point start_vertex, Point end_vertex) {
    for (auto it = vertices[start_vertex].begin(); it != vertices[start_vertex].end(); it++) {
        if (end_vertex == (*it)) vertices[start_vertex].erase(it);
        break;
    }
}

std::vector<Point> Graph::get_vertices() const {
    std::vector<Point> result;
    for (const auto& p : vertices) {
        result.push_back(p.first);
    }
    return result;
}

std::vector<Point> Graph::get_adjacent_vertices(Point src_vertex) {
    return vertices[src_vertex];
}
