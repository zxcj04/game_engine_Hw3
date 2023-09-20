#pragma once

#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <constants.hpp>
#include <Camera.hpp>
#include <Shader.hpp>

#ifndef M_PI
#define M_PI acos(-1)
#endif

using namespace std;

class BuildScene
{
    public:
        static void set_viewport(SCENE, int, int, bool only_first = false);
        static void set_projection(SCENE, glm::mat4 &, Camera, int, int, float, float, bool only_first = false);

        static void setup_boundary(unsigned int &);
        static void render_boundary(SCENE, unsigned int, Shader, unsigned int, float);

        static void setup_player(unsigned int &);
        static void render_player(SCENE, unsigned int, Shader, glm::vec3);

        static void setup_ball(unsigned int &, int &);
        static void render_ball(Shader, unsigned int, unsigned int, glm::vec3, glm::vec3, float, int, CULLING);

        static void setup_cube(unsigned int &, int &);
        static void render_cube(Shader, unsigned int, unsigned int, glm::vec3, float, float, int);

        static void setup_view_volume(unsigned int &);
        static void render_view_volume(SCENE, unsigned int, Shader, glm::vec3, float, float, float, float);

        static void setup_texture(unsigned int &, string);

        static int first_width;
        static int first_height;
};