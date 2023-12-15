#version 330 core

layout (location = 0) in vec3 in_Position; //--- 위치 변수: attribute position 0
layout (location = 1) in vec3 in_Color; //--- 컬러 변수: attribute position 1
layout (location = 2) in vec3 in_Normal; //--- 컬러 변수: attribute position 2
layout (location = 3) in vec2 in_Uv;

out vec3 out_Color; //--- 프래그먼트 세이더에게 전달
out vec3 out_Normal; //--- 프래그먼트 세이더에게 전달
out vec2 out_Uv ;//--- 프래그먼트 세이더에게 전달
out vec3 Frag_Pos; //--- 프래그먼트 세이더에게 전달

uniform mat4 World_trans;
uniform mat4 Camera_trans;
uniform mat4 Projection_trans;

uniform vec3 Light_Pos;
uniform vec3 Light_Color;

uniform vec3 View_Pos;

uniform bool Ui_bool; // ui == True

void main(void) 
{

	if (Ui_bool){
        gl_Position = World_trans * vec4 (in_Position.x, in_Position.y, in_Position.z, 1.0);
        Frag_Pos = in_Position;
    }
    else { 
        gl_Position = Projection_trans * Camera_trans * World_trans * vec4 (in_Position.x, in_Position.y, in_Position.z, 1.0);
        Frag_Pos = vec3(World_trans * vec4(in_Position, 1.0));
    }

    out_Normal = mat3(transpose(inverse(World_trans))) * in_Normal;
    out_Color = in_Color;
    out_Uv = in_Uv;
}