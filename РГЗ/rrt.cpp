#include "rrt.h";

double getDouble(const double B, const double A) {
    static std::random_device randDev;
    static std::mt19937 twister(randDev());
    static std::uniform_real_distribution<double> dist;
    dist.param(std::uniform_real_distribution<double>::param_type(A, B));
    return dist(twister);
}

Point random_point() {
    Point res = { getDouble(right_border - robot_r, robot_r), getDouble(bottom_border - robot_r, robot_r) };
    return res;
}

Point nearest(Point& a, Point& b, Point& c) {
    Point vectAB = { b.x - a.x, b.y - a.y };
    Point vectAC = { c.x - a.x, c.y - a.y };
    double lenAB = vectAB.x * vectAB.x + vectAB.y * vectAB.y;
    double relD = (vectAB.x * vectAC.x + vectAB.y * vectAC.y) / lenAB;
    if (relD <= 0) return a;
    if (relD >= 1) return b;
    Point res = { a.x + vectAB.x * relD, a.y + vectAB.y * relD };
    return res;
}

bool collision(Point& a, Point& b, Circle& c) {
    Point o = c.get_point();
    Point x = nearest(a, b, o);
    if (distance(x, o) > c.get_r() + robot_r) return 0;
    return 1;
}

Point steer(Point& a, Point& b) {
    Point x = b;
    for (auto it = obstacles.begin(); it != obstacles.end(); it++) {
        if (collision(a, x, *it)) {
            Point vectAB = { x.x - a.x, x.y - a.y };
            double lenAB = sqrt(vectAB.x * vectAB.x + vectAB.y * vectAB.y);
            if (lenAB == 0) return a;
            Point o = (*it).get_point();
            Point vectAO = { o.x - a.x, o.y - a.y };
            double relN = (vectAB.x * vectAO.x + vectAB.y * vectAO.y) / (lenAB*lenAB);
            Point c = { a.x + vectAB.x * relN, a.y + vectAB.y * relN };
            double lenR2 = (robot_r + (*it).get_r()) * (robot_r + (*it).get_r());
            double lenCO2 = distance(o, c) * distance(o, c);
            double lenAC = distance(a, c);
            double relD = (lenAC - sqrt(lenR2 - lenCO2)) / lenAC;
            x = { a.x + (c.x - a.x) * relD, a.y + (c.y - a.y) * relD };
        }
    }
    return x;
}

Graph rrt(int n, Point start, Point goal) {
    Graph result;
    if (right_border < robot_r || bottom_border < robot_r ||
        start.x > right_border - robot_r || start.x < robot_r || start.y > bottom_border - robot_r || start.y < robot_r ||
        goal.x > right_border - robot_r || goal.x < robot_r || goal.y > bottom_border - robot_r || goal.y < robot_r || n < 1)
        throw "Incorrect starting position.";
    for (auto it = obstacles.begin(); it != obstacles.end(); it++) {
        Point c = (*it).get_point();
        if (distance(start, c) < (*it).get_r()) throw "Некорректная стартовая позиция";
    }
    Point randP = random_point();
    Point endP = steer(start, randP);
    result.add_edge(start, endP);
    for (int i = 1; i < n; i++) {
        std::vector<Point> vertices = result.get_vertices();
        randP = random_point();
        Point nearP = start, temp1, temp2;
        for (Point p1 : vertices) {
            std::vector<Point> adj_vertices = result.get_adjacent_vertices(p1);
            for (Point p2 : adj_vertices) {
                Point temp_near = nearest(p1, p2, randP);
                if (distance(nearP, randP) > distance(temp_near, randP)) {
                    nearP = temp_near;
                    temp1 = p1;
                    temp2 = p2;
                }
            }
        }
        if (nearP != start) {
            if ((nearP!=temp1) && (nearP!=temp2)) {
                result.delete_edge(temp1, temp2);
                result.add_edge(temp1, nearP);
                result.add_edge(nearP, temp2);
            }
        }
        if (nearP != randP) {
            endP = steer(nearP, randP);
            result.add_edge(nearP, steer(nearP, randP));
        }
    }
    std::vector<Point> vertices = result.get_vertices();
    Point nearP = start, temp1, temp2;
    for (Point p1 : vertices) {
        std::vector<Point> adj_vertices = result.get_adjacent_vertices(p1);
        for (Point p2 : adj_vertices) {
            Point temp_near = nearest(p1, p2, goal);
            bool hit = 0;
            for (Circle i : obstacles) if (collision(temp_near, goal, i)) {
                hit = 1;
                break;
            }
            if (!hit && distance(nearP, goal) > distance(temp_near, goal)) {
                nearP = temp_near;
                temp1 = p1;
                temp2 = p2;
            }
        }
    }
    if (nearP != start) {
        if (nearP != temp1 && nearP != temp2) {
            result.delete_edge(temp1, temp2);
            result.add_edge(temp1, nearP);
            result.add_edge(nearP, temp2);
        }
    }
    endP = steer(nearP, goal);
    if (endP == goal) result.add_edge(nearP, endP);
    return result;
}

std::vector<Point> find_path(Graph& g, Point& start, Point& end) {
    std::vector<Point> result;
    for (Point& v : g.get_adjacent_vertices(start)) {
        if (v == end) {
            result.push_back(v);
            return result;
        }
        result = find_path(g, v, end);
        if (!result.empty()) {
            result.insert(result.begin(), v);
            return result;
        }
    }
    return {};
}
