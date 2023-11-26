#version 330 core

layout (location = 0) in vec3 in_Position; //--- 위치 변수: attribute position 0
layout (location = 1) in vec3 in_Color; //--- 컬러 변수: attribute position 1
layout (location = 2) in vec3 in_Normal; //--- 컬러 변수: attribute position 2

out vec3 out_Color; //--- 프래그먼트 세이더에게 전달
out vec3 out_Normal; //--- 프래그먼트 세이더에게 전달
out vec3 Frag_Pos; //--- 프래그먼트 세이더에게 전달

uniform mat4 World_trans;
uniform mat4 Normal_trans;
uniform mat4 Camera_trans;
uniform mat4 Projection_trans;

uniform vec3 Light_Pos;
uniform vec3 Light_Color;

uniform vec3 View_Pos;

void main(void) 
{
    gl_Position = Projection_trans * Camera_trans * World_trans * vec4 (in_Position.x, in_Position.y, in_Position.z, 1.0);
    Frag_Pos = vec3(World_trans * vec4(in_Position, 1.0));

    // 노멀 변환 스케일 쓰면 이상할 수도 있대 문제 생기면 그거 확인해보기
    out_Normal = vec3(Normal_trans * vec4(in_Normal, 1.0));
    out_Color = in_Color;
}