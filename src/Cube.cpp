#include <Cube.hpp>

int Cube::count = 0;

Cube::Cube(glm::vec3 position, float length, float rotate)
{
    this->position = position;

    this->length = length;

    this->rotate = rotate;

    this->number = Cube::count++;

    this->grid_position = glm::vec3(0.0f, 0.0f, 0.0f);

    BuildScene::setup_cube(vao, size);
}

Cube::~Cube()
{

}

void Cube::draw(Shader shader, unsigned int texture_cube)
{
    BuildScene::render_cube(shader, texture_cube, vao, position, length, rotate, size);
}

void Cube::update_regular_grid(vector<vector<vector<set<int>>>> &regular_grid, int gap)
{
    glm::ivec3 now_grid = glm::ivec3((position + 1000.0f) / (float)gap + 1.0f);

    if(now_grid.x < 0 || now_grid.x > 2000 / gap + 1 ||
       now_grid.y < 0 || now_grid.y > 2000 / gap + 1 ||
       now_grid.z < 0 || now_grid.z > 2000 / gap + 1
    )
        return;

    if(grid_position != now_grid)
    {
        regular_grid[grid_position.x][grid_position.y][grid_position.z].erase(this->number);

        grid_position = now_grid;

        // cout << this->number << ": " << grid_position.x << " " << grid_position.y << " " << grid_position.z << endl;

        regular_grid[grid_position.x][grid_position.y][grid_position.z].insert(this->number);
    }
}