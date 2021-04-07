#include <WindowManagement.hpp>

#define GLFW_MINOR_VERSION 6

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
    ImGui_ImplOpenGL3_Init("#version 460");

    system_init();

    this->shader = Shader("./src/shaders/tri.vert", "./src/shaders/tri.frag");

    cout << this->shader.ID << endl;

    this->camera = Camera();

    this->light_color = glm::vec3(1.0f, 1.0f, 1.0f);
    this->clear_color = glm::vec4(0.75f, 0.75f, 0.75f, 1.0f);

    this->enable_cursor = true;

    BuildScene::setup_boundary(vao_boundary);
    BuildScene::setup_player(vao_player);

    BuildScene::setup_texture(texture_wood, "assets/wood.png");

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
    render_scene(SCENE::ORTHO_X);
    render_scene(SCENE::ORTHO_Y);
    render_scene(SCENE::ORTHO_Z);
}

void WindowManagement::mainloop()
{
    while (!glfwWindowShouldClose(this->window))
    {
        this->display();

        this->check_keyboard_pressing();

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
    // cout << clip << endl;
    static bool is_load = false;
    static bool is_show = false;
    static string selected_inf = "engine";
    static string selected_raw = "engine";
    static int iso_value = 80;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(250, 325), ImGuiCond_Once);

    ImGui::Begin("Is that a bird?");
    {

    }
    ImGui::End();
}


//-----------------------------------------


void WindowManagement::render_scene(SCENE scene)
{
    this->shader.use();

    glm::mat4 projection;

    BuildScene::set_viewport(scene, width, height);
    BuildScene::set_projection(scene, projection, this->camera, width, height);

    shader.set_uniform("projection", projection);
    shader.set_uniform("light_color", light_color);
    camera.use(shader, scene);

    BuildScene::render_boundary(scene, vao_boundary, shader, texture_wood);
    BuildScene::render_player(vao_player, shader, this->camera.position);
}

//-------------------------------------------------



void WindowManagement::keyboard_down(int key)
{
    // cout << (char) tolower(key) << endl;

    static double x, y;

    switch(key)
    {
        case GLFW_KEY_ESCAPE:  // ESC
            exit(0);

        case GLFW_KEY_R:
            this->shader.reload();

            break;

        case GLFW_KEY_Q:
            this->enable_cursor = !this->enable_cursor;

            if(this->enable_cursor)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                glfwGetCursorPos(window, &x, &y);

                this->last_x = x;
                this->last_y = y;
            }
            else
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            break;
    }
}

void WindowManagement::check_keyboard_pressing()
{
    // if(ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
    //     return;


    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        this->camera.position +=  this->camera.direction * 10.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        this->camera.position += -this->camera.direction * 10.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        this->camera.position +=  glm::cross(this->camera.direction, glm::vec3(0.0f, 1.0f, 0.0f)) * 10.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        this->camera.position += -glm::cross(this->camera.direction, glm::vec3(0.0f, 1.0f, 0.0f)) * 10.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        this->camera.position += -glm::vec3(0.0f, 1.0f, 0.0f) * 10.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        this->camera.position +=  glm::vec3(0.0f, 1.0f, 0.0f) * 10.0f;
    }

    if(this->camera.position.x > 900)
        this->camera.position.x = 900;
    if(this->camera.position.x < -900)
        this->camera.position.x = -900;

    if(this->camera.position.y > 900)
        this->camera.position.y = 900;
    if(this->camera.position.y < -900)
        this->camera.position.y = -900;

    if(this->camera.position.z > 900)
        this->camera.position.z = 900;
    if(this->camera.position.z < -900)
        this->camera.position.z = -900;
}

void WindowManagement::mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {

    }

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
        this->camera.update_pitch(y_offset);
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