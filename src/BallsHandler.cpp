#include <BallsHandler.hpp>

bool to_near_to_another_cube(vector<Cube> cubes, glm::vec3 position, float length)
{
    bool ret = false;

    for(auto cube: cubes)
    {
        if(cube.number == Cube::count)
            continue;

        if(glm::length(cube.position - position) < (cube.length / 2 + length / 2) * sqrt(2))
        {
            ret = true;
            break;
        }
    }

    return ret;
}

BallsHandler::BallsHandler(float boundary_size)
{
    random_device rd;

    rd_generator = mt19937(rd());

    gap = 500;

    regular_grid = vector<vector<vector<set<int>>>>(boundary_size/gap + 2, vector<vector<set<int>>>(boundary_size/gap + 2, vector<set<int>>(boundary_size/gap + 2, set<int>())));
    cube_regular_grid = vector<vector<vector<set<int>>>>(boundary_size/gap + 2, vector<vector<set<int>>>(boundary_size/gap + 2, vector<set<int>>(boundary_size/gap + 2, set<int>())));

    for(int i = 0 ; i < 5 ; i++)
    {
        add_cube(boundary_size);
    }
}

BallsHandler::~BallsHandler()
{

}

void BallsHandler::add_ball(float boundary_size, glm::vec3 position, float radius, glm::vec3 speed)
{
    _balls.push_back(Ball(position, radius, speed));
}

void BallsHandler::add_ball(float boundary_size)
{
    uniform_real_distribution<float> unif_position(-boundary_size / 2, boundary_size / 2);
    uniform_real_distribution<float> unif_radius(50, 250);
    uniform_real_distribution<float> unif_speed(-50, 50);

    float radius = unif_radius(rd_generator);

    glm::vec3 position;
    glm::vec3 speed;

    do
    {
        position.x = unif_position(rd_generator);
        position.y = unif_position(rd_generator);
        position.z = unif_position(rd_generator);

    } while (abs(position.x) + radius >= 1000 ||
             abs(position.y) + radius >= 1000 ||
             abs(position.z) + radius >= 1000);

    speed = glm::vec3(
        unif_speed(rd_generator),
        unif_speed(rd_generator),
        unif_speed(rd_generator)
    );

    add_ball(boundary_size, position, radius, speed);
}

void BallsHandler::add_cube(float boundary_size)
{
    uniform_real_distribution<float> unif_cube_length(250, 500);
    uniform_real_distribution<float> unif_cube_position(-boundary_size / 2, boundary_size / 2);
    uniform_real_distribution<float> unif_cube_rotate(0, 360);

    float length = unif_cube_length(rd_generator);
    float rotate = unif_cube_rotate(rd_generator);
    glm::vec3 position;

    do
    {
        position.x = unif_cube_position(rd_generator);
        position.y = -boundary_size / 2 + length / 2;
        position.z = unif_cube_position(rd_generator);

    } while (abs(position.x) + length / 2 * sqrt(2) >= 1000 ||
                abs(position.z) + length / 2 * sqrt(2) >= 1000 ||
                to_near_to_another_cube(_cubes, position, length));

    Cube cube = Cube(position, length, rotate);

    _cubes.push_back(cube);
}

void BallsHandler::draw_balls(Shader shader, unsigned int texture_ball, glm::vec3 player_position, float yaw, float pitch, float near, float far)
{
    for(auto & ball: _balls)
        ball.draw(shader, texture_ball, player_position, yaw, pitch, near, far);
}

void BallsHandler::move_balls(float decay, bool spatial_partition)
{
    if(spatial_partition)
    {
        for(int i = 1; i <= 2000 / gap; i++)
        for(int j = 1; j <= 2000 / gap; j++)
        for(int k = 1; k <= 2000 / gap; k++)
        for(auto one: regular_grid[i][j][k])
        {
            for(int i2 = i - 1; i2 <= i + 1; i2++)
            for(int j2 = j - 1; j2 <= j + 1; j2++)
            for(int k2 = k - 1; k2 <= k + 1; k2++)
            {
                for(auto two: regular_grid[i2][j2][k2])
                {
                    if(one == two)
                        continue;

                    _balls[one].check_ball_collision(_balls[two], decay);
                }
            }

            for(int i2 = i - 1; i2 <= i + 1; i2++)
            for(int j2 = j - 1; j2 <= j + 1; j2++)
            for(int k2 = k - 1; k2 <= k + 1; k2++)
            {
                for(auto two: cube_regular_grid[i2][j2][k2])
                {
                    _balls[one].check_cube_collision(_cubes[two], decay);
                }
            }
        }

    }
    else
    {
        for(int i = 0; i < _balls.size(); i++)
            for(int j = i + 1; j < _balls.size() ; j++)
            {
                _balls[i].check_ball_collision(_balls[j], decay);
            }

        for(int i = 0; i < _balls.size(); i++)
            for(int j = 0; j < _cubes.size() ; j++)
            {
                _balls[i].check_cube_collision(_cubes[j], decay);
            }
    }

    for(auto & ball: _balls)
    {
        if(spatial_partition)
            ball.update_regular_grid(regular_grid, gap);

        ball.move(_balls, decay);
    }

    for(auto & cube: _cubes)
    {
        if(spatial_partition)
            cube.update_regular_grid(cube_regular_grid, gap);

        // cube.move(cube, decay);
    }
}

void BallsHandler::reset_balls()
{
    _balls.clear();

    for(int i = 0; i <= 2000 / gap + 1; i++)
    for(int j = 0; j <= 2000 / gap + 1; j++)
    for(int k = 0; k <= 2000 / gap + 1; k++)
    {
        regular_grid[i][j][k].clear();
    }

    Ball::count = 0;
}

void BallsHandler::reset_cubes(float boundary_size)
{
    _cubes.clear();

    for(int i = 0; i <= 2000 / gap + 1; i++)
    for(int j = 0; j <= 2000 / gap + 1; j++)
    for(int k = 0; k <= 2000 / gap + 1; k++)
    {
        cube_regular_grid[i][j][k].clear();
    }

    Cube::count = 0;

    for(int i = 0 ; i < 5 ; i++)
    {
        add_cube(boundary_size);
    }
}

void BallsHandler::draw_cubes(Shader shader, unsigned int texture_cube)
{
    for(auto & cube: _cubes)
        cube.draw(shader, texture_cube);
}