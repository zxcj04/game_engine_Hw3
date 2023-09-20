#pragma once

#include <iostream>
#include <vector>
#include <set>

#include <glm/glm.hpp>

#include <BuildScene.hpp>
#include <Shader.hpp>

using namespace std;

class Cube
{
    public:
        Cube(glm::vec3, float, float);
        ~Cube();

        void draw(Shader, unsigned int);

        void update_regular_grid(vector<vector<vector<set<int>>>> &regular_grid, int);

        static int count;

    // private:
        glm::vec3 position;

        float rotate;

        float length;

        glm::ivec3 grid_position;

        int number;

        unsigned int vao;

        int size;
};