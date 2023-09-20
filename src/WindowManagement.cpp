#include <WindowManagement.hpp>

#define GLFW_MINOR_VERSION 1

WindowManagement::WindowManagement()
{
    cout << "WindowManagement init" << endl;
}

WindowManagement::~WindowManagement()
{
	glfwDestroyWindow(this->window);

    cout << "terminate" << endl;

    glfwTerminate();
}

void WindowManagement::error_callback(int error, const char * description)
{
	cout << "Error: " << description << endl;
}

bool WindowManagement::init(string window_name)
{
    glfwSetErrorCallback(WindowManagement::error_callback);

    GLuint err = !glfwInit();
    if (err)
    {
        cout << "glfwInit ERROR" << endl;

        return false;
    }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GLFW_MINOR_VERSION);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifndef __APPLE__
        // dedug context
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    #endif
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    width = 1200;
    height = 900;

    this->last_x = width/2;
    this->last_y = height/2;

    this->near_distance = 0.2f;
    this->far_distance = 4000.0f;

    this->window = glfwCreateWindow(width, height, window_name.c_str(), NULL, NULL);

    GLFWmonitor* primary = glfwGetPrimaryMonitor();

    const GLFWvidmode *mode = glfwGetVideoMode(primary);
    if (!mode)
        return 0;

    int monitorX, monitorY;
    glfwGetMonitorPos(primary, &monitorX, &monitorY);

    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    glfwSetWindowPos(window,
                     monitorX + (mode->width - windowWidth) / 2,
                     monitorY + (mode->height - windowHeight) / 2);

    if(!this->window)
    {
        cout << "glfwCreateWindow ERROR" << endl;

        glfwTerminate();

        return false;
    }
    else
    {
        cout << "Create window Success" << endl;
    }

    glfwMakeContextCurrent(this->window);

    glfwSwapInterval(1);

    // glfwSetWindowAspectRatio(window, 1, 1);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        cout << "Failed to initialize GLAD" << endl;

        return false;
    }

    cout << "OpenGL shader language version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    set_callback_functions();

    // IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(this->window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);

    system_init();

    this->shader = Shader("./src/shaders/tri.vert", "./src/shaders/tri.frag");
    this->shader_texture = Shader("./src/shaders/tri_texture.vert", "./src/shaders/tri_texture.frag");

    cout << this->shader.ID << " " << this->shader_texture.ID << endl;

    this->boundary_size = 2000.0f;

    this->camera = Camera(boundary_size);

    this->light_color = glm::vec3(1.0f, 1.0f, 1.0f);
    this->clear_color = glm::vec4(0.75f, 0.75f, 0.75f, 1.0f);

    this->enable_cursor = true;

    this->decay = 1.0f;

    this->spatial_partition = true;

    this->ball_radius = 150.0f;

    this->only_first = true;

    BuildScene::setup_boundary(vao_boundary);
    BuildScene::setup_player(vao_player);
    BuildScene::setup_view_volume(vao_view_volume);

    BuildScene::setup_texture(texture_wood, "assets/wood.png");
    BuildScene::setup_texture(texture_ball, "assets/brick.jpg");
    BuildScene::setup_texture(texture_cube, "assets/container.jpg");

    balls_handler = new BallsHandler(this->boundary_size);

    return true;
}

void WindowManagement::set_callback_functions()
{
    glfwSetWindowUserPointer(this->window, this);

    auto keyboardCb = [](GLFWwindow * w, int key, int scan, int act, int mod){
        static_cast<WindowManagement*>(glfwGetWindowUserPointer(w))->keyboard_callback(w, key, scan, act, mod);
    };
    auto mouseCb = [](GLFWwindow * w, int button, int action, int mods){
        static_cast<WindowManagement*>(glfwGetWindowUserPointer(w))->mouse_callback(w, button, action, mods);
    };
    auto scrollCb = [](GLFWwindow * w, double x_offset, double y_offset){
        static_cast<WindowManagement*>(glfwGetWindowUserPointer(w))->scroll_callback(w, x_offset, y_offset);
    };
    auto cursorPosCb = [](GLFWwindow * w, double x_pos, double y_pos){
        static_cast<WindowManagement*>(glfwGetWindowUserPointer(w))->cursor_callback(w, x_pos, y_pos);
    };
    auto viewportCb = [](GLFWwindow * w, int width, int height){
        static_cast<WindowManagement*>(glfwGetWindowUserPointer(w))->framebuffer_callback(w, width, height);
    };

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetKeyCallback(this->window, keyboardCb);
    glfwSetMouseButtonCallback(window, mouseCb);
    glfwSetScrollCallback(this->window, scrollCb);
    glfwSetCursorPosCallback(this->window, cursorPosCb);
    glfwSetFramebufferSizeCallback(this->window, viewportCb);
}

bool WindowManagement::system_init()
{
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_PROGRAM_POINT_SIZE);

    light_init();

    return true;
}

bool WindowManagement::light_init()
{
    float  lit_diffuse[] = {0.75, 0.75, 0.75, 1.0};
    float  lit_specular[] = {0.7, 0.7, 0.7, 1.0};

    // float  lit1_diffuse[] = {0.075, 0.10, 0.075, 1.0};
    float  lit1_ambient[] = {0.0, 0.0, 0.0, 0.0};

    float  lit2_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    float  lit2_ambient[] = {0.0, 0.0, 0.0, 0.0};

    return true;
}

void WindowManagement::display()
{
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

    glfwGetFramebufferSize(window, &(this->width), &(this->height));

    glViewport(0, 0, this->width, this->height);

    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);

    render_scene(SCENE::FIRST);

    if(!only_first)
    {
        render_scene(SCENE::ORTHO_X);
        render_scene(SCENE::ORTHO_Y);
        render_scene(SCENE::ORTHO_Z);
    }
}

void WindowManagement::mainloop()
{
    while (!glfwWindowShouldClose(this->window))
    {
        this->display();

        this->check_keyboard_pressing();

        this->balls_handler->move_balls(decay, spatial_partition);

        imgui();

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(this->window);

        /* Poll for and process events */
        glfwPollEvents();
    }
}

//-----------------------------------------

void WindowManagement::imgui()
{
    static glm::vec3 ball_position;
    static vector<float> ball_speed = {0.0f, 0.0f, 0.0f};
    static bool overflow_decay = false;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(width - 275, height - 500), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(250, 475), ImGuiCond_Once);

    ImGui::Begin("Is that a bird?");
    {
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::Text("Clipping");

        // ImGui::SliderFloat("Far", &far_distance, near_distance, 5000.0f, "%.1f");
        ImGui::InputFloat("-Far-",  &far_distance, 1.0f, 100.0f, "%.1f", ImGuiInputTextFlags_AutoSelectAll);
        // ImGui::SliderFloat("Near", &near_distance, 0.1f, far_distance  , "%.1f");
        ImGui::InputFloat("-Near-", &near_distance, 1.0f, 100.0f, "%.1f", ImGuiInputTextFlags_AutoSelectAll);

        ImGui::Checkbox("perspective only", &only_first);

        ImGui::Text("----------------------------");

        ImGui::Text("Game Setting");

        if(overflow_decay)
            ImGui::SliderFloat("Decay", &decay, 0.1f, 2.0f, "%.1f");
        else
        {
            if(decay > 1.0f)
                decay = 1.0f;

            ImGui::SliderFloat("Decay", &decay, 0.1f, 1.0f, "%.1f");
        }

        ImGui::Checkbox("Spatial Partition", &spatial_partition);
        ImGui::Checkbox("Decay Overflow (danger)", &overflow_decay);

        ImGui::Text("----------------------------");

        ImGui::Text("Generate Ball");

        if(ImGui::Button("Random Generate", ImVec2(200, 25)))
        {
            this->balls_handler->add_ball(boundary_size);
        }

        ImGui::Text("----------------------------");

        // void BallsHandler::add_ball(float boundary_size, glm::vec3 position, float radius, glm::vec3 speed)

        ImGui::InputFloat("radius", &ball_radius, 1.0f, 50.0f, "%.1f", ImGuiInputTextFlags_AutoSelectAll);

        ImGui::InputFloat3("init speed", ball_speed.data(), "%.1f");

        if(ImGui::Button("Generate at now position", ImVec2(200, 25)))
        {
            ball_position = this->camera.position;

            this->balls_handler->add_ball(boundary_size, ball_position, ball_radius, glm::vec3(ball_speed[0], ball_speed[1], ball_speed[2]));
        }

        ImGui::Text("----------------------------");

        if(ImGui::Button("Clear \"ALL\" Balls", ImVec2(200, 25)))
        {
            this->balls_handler->reset_balls();
        }
    }
    ImGui::End();
}


//-----------------------------------------


void WindowManagement::render_scene(SCENE scene)
{
    this->shader_texture.use();

    glm::mat4 projection;

    BuildScene::set_viewport(scene, width, height, only_first);
    BuildScene::set_projection(scene, projection, this->camera, width, height, this->near_distance, this->far_distance, only_first);

    shader_texture.set_uniform("projection", projection);
    shader_texture.set_uniform("light_color", light_color);
    camera.use(this->shader_texture, scene);

    BuildScene::render_boundary(scene, vao_boundary, this->shader_texture, texture_wood, boundary_size);

    this->balls_handler->draw_balls(this->shader_texture, texture_ball
                                  , this->camera.position, this->camera.yaw, this->camera.pitch
                                  , this->near_distance, this->far_distance);

    this->balls_handler->draw_cubes(this->shader_texture, texture_cube);

    this->shader.use();

    shader.set_uniform("projection", projection);
    shader.set_uniform("light_color", light_color);
    shader.set_uniform("enable_light", true);
    camera.use(this->shader, scene);

    BuildScene::render_player(scene, vao_player, this->shader, this->camera.position);

    shader.set_uniform("enable_light", false);
    BuildScene::render_view_volume(scene, vao_view_volume, this->shader
                                 , this->camera.position, this->camera.yaw, this->camera.pitch
                                 , this->near_distance, this->far_distance);
}

//-------------------------------------------------



void WindowManagement::keyboard_down(int key)
{
    // cout << (char) tolower(key) << endl;

    static double x, y;

    ImGuiIO& io = ImGui::GetIO();

    switch(key)
    {
        case GLFW_KEY_ESCAPE:  // ESC
            exit(0);

        case GLFW_KEY_R:
            this->shader.reload();
            this->shader_texture.reload();

            break;

        case GLFW_KEY_Q:
            this->enable_cursor = !this->enable_cursor;

            if(this->enable_cursor)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                glfwGetCursorPos(window, &x, &y);

                this->last_x = x;
                this->last_y = y;


                io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
                ImGui::SetMouseCursor(ImGuiMouseCursor_None);
            }
            else
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

                io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            }

            break;

        case GLFW_KEY_G:
            this->balls_handler->reset_balls();

            break;

        case GLFW_KEY_H:
            this->balls_handler->reset_cubes(this->boundary_size);

            break;

        case GLFW_KEY_T:
            this->balls_handler->add_ball(boundary_size);

            break;
    }
}

void WindowManagement::check_keyboard_pressing()
{
    // if(ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
    //     return;
    static float cooldown = 0.0f;

    glm::vec3 front = glm::normalize(glm::vec3(this->camera.direction.x, 0, this->camera.direction.z));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        this->camera.position +=  front * 20.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        this->camera.position += -front * 20.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        this->camera.position +=  glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f))) * 20.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        this->camera.position += -glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f))) * 20.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        this->camera.position += -glm::vec3(0.0f, 1.0f, 0.0f) * 20.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        this->camera.position +=  glm::vec3(0.0f, 1.0f, 0.0f) * 20.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
    {
        this->boundary_size += 10;

        this->camera.update_boundary_size(this->boundary_size);
    }
    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
    {
        this->boundary_size -= 10;

        this->camera.update_boundary_size(this->boundary_size);
    }

    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
    {
        this->balls_handler->add_ball(boundary_size);
    }

    if (enable_cursor && cooldown >= this->ball_radius / 50.0f && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        this->balls_handler->add_ball(boundary_size, this->camera.position + this->camera.direction * 5.0f, this->ball_radius, 25.0f * this->camera.direction);

        cooldown = 0.0f;
    }

    if(cooldown < this->ball_radius / 50.0f)
        cooldown += 0.1f;

    if(this->camera.position.x > boundary_size/2 - 5)
        this->camera.position.x = boundary_size/2 - 5;
    if(this->camera.position.x < -(boundary_size/2 - 5))
        this->camera.position.x = -(boundary_size/2 - 5);

    if(this->camera.position.y > boundary_size/2 - 5)
        this->camera.position.y = boundary_size/2 - 5;
    if(this->camera.position.y < -(boundary_size/2 - 5))
        this->camera.position.y = -(boundary_size/2 - 5);

    if(this->camera.position.z > boundary_size/2 - 5)
        this->camera.position.z = boundary_size/2 - 5;
    if(this->camera.position.z < -(boundary_size/2 - 5))
        this->camera.position.z = -(boundary_size/2 - 5);
}

void WindowManagement::mouse_callback(GLFWwindow* window, int button, int action, int mods)
{

}

void WindowManagement::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // if(ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
    //     return;
    // cout << xoffset << " " << yoffset << endl;

    this->camera.zoom(yoffset);
}

void WindowManagement::cursor_callback(GLFWwindow * window, double x, double y)
{
    float x_offset = x - this->last_x;
    float y_offset = y - this->last_y;

    this->last_x = x;
    this->last_y = y;

    // if(ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
    //     return;

    if(this->enable_cursor)
    {
        this->camera.update_yaw(x_offset);
        this->camera.update_pitch(-y_offset);
    }
}

void WindowManagement::keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS)
        keyboard_down(key);
}

void WindowManagement::framebuffer_callback(GLFWwindow * w, int new_w, int new_h)
{
    this->width = new_w;
    this->height = new_h;
}
