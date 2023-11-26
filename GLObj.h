#pragma once
#include "GLShapes.h"

class GLObj :
    public GLShapes
{
public:
    GLuint v_pos;
    GLuint v_nor;
    GLint face_cnt = 0;
    glm::vec3 Object_mid{ 0.f, 0.f, 0.f };
    glm::vec3 Ani_mid{ 0.f, 0.f, 0.f };
    glm::vec3 max{}, min{};
    glm::vec3 size{ 0.f, 0.f, 0.f };
    glm::vec3 velocity{ 0.f, 0.f, 0.f };
    glm::mat4 Normal_mat{};

    GLObj(){}
    GLObj(glm::vec3 m);

    void objLoad(std::ifstream& path);

    void draw(std::string draw_Mod);

    void draw_prepare(int Location, std::string Location_str);

    void Update();
    void Normal_Update();
    void Update20();
    void Update22();
    void Update24();
};

