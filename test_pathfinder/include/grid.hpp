#ifndef GRID_H
#define GRID_H
#include "gvd.hpp"
#include "vehicle.hpp"

class Grid : public GVD
{
    vector<Coordinates> obstacleList;
public:
    Grid(int width, int height);
    void addObstacle(int x, int y);
    void removeObstacle(int x, int y);
    void refreshMap();
    vector<vector<float>> returnDistMap();
    vector<vector<float>> returnVoroDistMap();
    bool isSafe(float safetyFactor);
};

#endif