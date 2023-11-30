#version 330 core

in vec3 out_Color;
in vec3 out_Normal;
in vec2 out_Uv;

in vec3 Frag_Pos;
out vec4 Frag_Color;

uniform vec3 Light_Pos;
uniform vec3 Light_Color;

uniform vec3 View_Pos;
uniform float Distance;

uniform sampler2D out_Tex; //--- 텍스처 샘플러

uniform bool Tex_or_Color; // 텍스쳐로 출력할지 == True /  컬러로 출력할지 == False

void main ()
{
	float Ambient_Light = 0.3;
	vec3 ambient = Ambient_Light * Light_Color;

	vec3 Normal_Vector = normalize(out_Normal);

	vec3 Light_Dir = normalize(Light_Pos - Frag_Pos);

	float Diffuse_Light = max(dot(Normal_Vector, Light_Dir), 0.0); 
    vec3 diffuse = Diffuse_Light * Light_Color; 

	int shininess = 128;

	vec3 View_Dir = normalize(View_Pos - Frag_Pos);

	vec3 Reflect_Dir = reflect(-Light_Dir, Normal_Vector); 

	float Specular_Light = max(dot(View_Dir, Reflect_Dir), 0.0);
	Specular_Light = pow(Specular_Light, shininess);
	vec3 specular = Specular_Light * Light_Color;

	vec3 result = (ambient + diffuse + specular) * out_Color;

	if (Tex_or_Color)
		Frag_Color = (vec4((ambient + diffuse + specular) / Distance, 1.0) * texture(out_Tex, out_Uv));
	else
		Frag_Color = vec4(result / Distance, 1.0);
}