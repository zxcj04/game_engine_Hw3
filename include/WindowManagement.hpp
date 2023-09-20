#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <set>
#include <map>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <constants.hpp>
#include <Shader.hpp>
#include <Camera.hpp>
#include <Volume.hpp>
#include <BuildScene.hpp>
#include <BallsHandler.hpp>

#ifndef M_PI
#define M_PI acos(-1)
#endif

using namespace std;

#define   TSIZE  64   /* define texture dimension */

class WindowManagement
{
    public:

        WindowManagement();
        ~WindowManagement();

        static void error_callback(int, const char *);

        bool init(string);
        void generate_combo();
        void set_callback_functions();
        bool system_init();
        bool light_init();
        bool set_callback_function();

        void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        void keyboard_down(int key);

        void check_keyboard_pressing();

        void mouse_callback(GLFWwindow* window, int button, int action, int mods);

        void cursor_callback(GLFWwindow * window, double x, double y);

        void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

        void framebuffer_callback(GLFWwindow * w, int width, int height);

        void imgui();

        void render_scene(SCENE);
        void display();

        void mainloop();

    private:
        GLFWwindow *window;

        int width;
        int height;

        glm::vec4 clear_color;

        float near_distance;
        float far_distance;

        float last_x, last_y;

        /*-----Create image space for textures -----*/
        unsigned int   textName[5];

        Shader shader;
        Shader shader_texture;
        Camera camera;

        unsigned int vao_boundary;
        unsigned int vao_player;
        unsigned int vao_view_volume;

        float boundary_size;

        unsigned int texture_wood;
        unsigned int texture_ball;
        unsigned int texture_cube;

        glm::vec3 light_color;

        bool enable_cursor;

        float decay;

        bool spatial_partition;

        float ball_radius;

        bool only_first;

        BallsHandler *balls_handler;
};