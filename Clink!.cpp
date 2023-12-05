#include "stdafx.h"
#include "GLShapes.h"
#include "GLObj.h"
#include "GLLight.h"
#include "GLLIne.h"
#include "GLCamera.h"

GLchar* vertexSource, * fragmentSource; //--- 소스코드 저장 변수
GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint shaderProgramID; //--- 셰이더 프로그램

void make_shaderProgram();
void make_vertexShaders();
void make_fragmentShaders();
void InitBuffer();
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Motion(int x, int y);
GLvoid drawScene();
GLvoid Reshape(int w, int h);
char* filetobuf(const char* file);

void Init();
void Mapping();
void TimerFunction(int value);
void Keyboard(unsigned char key, int x, int y);
void Special_Keyboard(int key, int x, int y);
void MouseWheel(int wheel, int diretion, int x, int y);

using namespace std;

bool CheckCollision(const GLObj& a, const GLObj& b) {
	return (std::abs(a.pos.x - b.pos.x) < (a.size.x + b.size.x) &&
		std::abs(a.pos.y - b.pos.y) < (a.size.y + b.size.y) &&
		std::abs(a.pos.z - b.pos.z) < (a.size.z + b.size.z));
}

float winSizex = 0, winSizey = 0;
GLuint vao;

vector <GLObj> Ball, Crystal, Background;
GLLine lineObj;
GLCamera Camera;
GLLight Light;
int PosLocation, ColorLocation, NormalLocation, UvLocation;
unsigned int WorldTransLocation, CameraLocation, ProjectionLocation, TexSamplerLocation, TexorColorLocation;
int LightPosLocation;
unsigned int LightColorLocation, ViewPosLocation, DistanceLocation;

bool Lbt = false;
glm::vec3 click_mouse{};

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(300, 100);
	glutInitWindowSize(winSizex, winSizey);
	glutCreateWindow("Clink!");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();
	make_shaderProgram();
	Mapping();
	InitBuffer();
	Init();
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutMouseWheelFunc(MouseWheel);
	glutKeyboardFunc(Keyboard); // 키보드 입력 콜백함수
	glutSpecialFunc(Special_Keyboard); // 키보드 입력 콜백함수
	glutTimerFunc(10, TimerFunction, 1);
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMainLoop();
}

GLvoid drawScene()
{
	glUseProgram(shaderProgramID);

	// 버텍스 쉐이더에게 전달
	glEnableVertexAttribArray(PosLocation);
	glEnableVertexAttribArray(ColorLocation);
	glEnableVertexAttribArray(NormalLocation);
	glEnableVertexAttribArray(UvLocation);

	// 프래그먼트 쉐이더에게 전달
	glUniform1i(TexSamplerLocation, 0);

	// 카메라 변환
	Camera.Update();
	Camera.draw_prepare(ViewPosLocation, "View_Pos");

	// 투영 변환
	glm::mat4 Projection_Mat = glm::mat4(1.0f);
	Projection_Mat = glm::perspective(glm::radians(45.f), 1.f, 0.1f, 50.f);
	glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, &Projection_Mat[0][0]);
	glUniformMatrix4fv(CameraLocation, 1, GL_FALSE, glm::value_ptr(Camera.Camera_Mat));

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_DEPTH_TEST);

	// 광원
	Light.scale.y *= winSizex / winSizey;
	Light.Update();
	Light.draw_prepare(PosLocation, "Pos");
	Light.draw_prepare(ColorLocation, "Color");
	Light.draw_prepare(TexorColorLocation, "Color_bool");
	Light.draw_prepare(NormalLocation, "Normal");
	Light.draw_prepare(WorldTransLocation, "World");
	Light.draw_prepare(UvLocation, "UV");
	Light.draw_prepare(LightPosLocation, "LightPos");
	Light.draw_prepare(LightColorLocation, "LightColor");
	Light.draw("solid");
	Light.scale.y /= winSizex / winSizey;

	lineObj.Update();
	lineObj.draw_prepare(PosLocation, "Pos");
	lineObj.draw_prepare(ColorLocation, "Color");
	lineObj.draw_prepare(TexorColorLocation, "Color_bool");
	glUniform1i(TexorColorLocation, false);
	lineObj.draw_prepare(WorldTransLocation, "World");
	lineObj.draw();

	for (int i = 0; i < Background.size(); ++i) {
		Background[i].scale.y *= winSizex / winSizey;
		Background[i].Update();
		Background[i].draw_prepare(PosLocation, "Pos");
		Background[i].draw_prepare(ColorLocation, "Color");
		Background[i].draw_prepare(TexorColorLocation, "Color_bool");
		Background[i].draw_prepare(WorldTransLocation, "World");
		Background[i].draw_prepare(NormalLocation, "Normal");
		Background[i].draw_prepare(UvLocation, "UV");
		glUniform1f(DistanceLocation, distance(Light.pos, Background[i].pos));
		Background[i].draw("solid");
		Background[i].scale.y /= winSizex / winSizey;
	}

	// 알파값 포함 객체 그리기 시작 -------

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int i = 0; i < Ball.size(); ++i) {
		Ball[i].scale.y *= winSizex / winSizey;
		Ball[i].Update();
		Ball[i].draw_prepare(PosLocation, "Pos");
		Ball[i].draw_prepare(WorldTransLocation, "World");
		Ball[i].draw_prepare(NormalLocation, "Normal");
		Ball[i].draw_prepare(UvLocation, "UV");
		Ball[i].draw_prepare(false, "Texture");
		Ball[i].draw_prepare(TexorColorLocation, "Texture_bool");
		glUniform1f(DistanceLocation, distance(Light.pos, Ball[i].pos));
		Ball[i].draw("solid");
		Ball[i].scale.y /= winSizex / winSizey;
	}
	
	for (int i = 0; i < Crystal.size(); ++i) {
		Crystal[i].scale.y *= winSizex / winSizey;
		Crystal[i].Update();
		Crystal[i].draw_prepare(PosLocation, "Pos");
		Crystal[i].draw_prepare(WorldTransLocation, "World");
		Crystal[i].draw_prepare(NormalLocation, "Normal");
		Crystal[i].draw_prepare(UvLocation, "UV");
		Crystal[i].draw_prepare(false, "Texture");
		Crystal[i].draw_prepare(TexorColorLocation, "Texture_bool");
		glUniform1f(DistanceLocation, distance(Light.pos, Crystal[i].pos));
		Crystal[i].draw("solid");
		Crystal[i].scale.y /= winSizex / winSizey;
	}

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	glDisableVertexAttribArray(PosLocation);
	glDisableVertexAttribArray(ColorLocation);
	glDisableVertexAttribArray(NormalLocation);
	glDisableVertexAttribArray(UvLocation);

	glutSwapBuffers();
}

void TimerFunction(int value)
{
	switch (value)
	{
	case 1: {
		for (int bg_cnt = 0; bg_cnt < Background.size(); ++bg_cnt) {

			for (int i = 0; i < Ball.size(); ++i) {
				if (bg_cnt == 0) {
					if (Ball[i].pos.x - Ball[i].size.x <= -1.f || Ball[i].pos.x + Ball[i].size.x >= 1.f)
						Ball[i].velocity = glm::vec3{ 0.f, 0.f, 0.f };
					if (Ball[i].pos.y - Ball[i].size.y <= -1.f || Ball[i].pos.y + Ball[i].size.y >= 1.f)
						Ball[i].velocity = glm::vec3{ 0.f, 0.f, 0.f };
				}

				Ball[i].pos += Ball[i].velocity;

				if (Ball[i].velocity.y != 0.f) {
					Ball[i].velocity.y -= 0.0005f;
				}
			}

		}
		break;
	}
	default:
		break;
	}

	glutPostRedisplay(); // 화면 재 출력
	glutTimerFunc(10, TimerFunction, 1);
}

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
	case 'Q': {
		exit(829);
	}
	default:
		break;
	}

	glutPostRedisplay(); // 화면 재 출력
}

void Special_Keyboard(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP: {
		if (Light.L_color.x < 2.f)
			Light.L_color += glm::vec3{ 0.1f, 0.1f, 0.1f };
		break;
	}
	case GLUT_KEY_DOWN: {
		if (Light.L_color.x > 0.3f)
			Light.L_color -= glm::vec3{ 0.1f, 0.1f, 0.1f };
		break;
	}
	case GLUT_KEY_F11: {
		glutFullScreenToggle();
		break;
	}
	default:
		break;
	}

	glutPostRedisplay(); // 화면 재 출력
}

GLvoid Mouse(int button, int state, int x, int y)
{
	glm::vec3 m = { (x - (winSizex / 2)) / (winSizex / 2), -(y - (winSizey / 2)) / (winSizey / 2), 0.0f };

	if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON) {
			Lbt = true;
			click_mouse = m;
		}
	}
	else if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON) {
			Lbt = false;
			click_mouse = glm::vec3{ 0.f, 0.f, 0.f };
		}
	}
}

GLvoid Motion(int x, int y)
{
	if (Lbt) {
		glm::vec3 m = { (x - (winSizex / 2)) / (winSizex / 2), -(y - (winSizey / 2)) / (winSizey / 2), 0.0f };

		if (m.x < click_mouse.x)
			Ball[0].rotate_theta.y += 1.f;
		else if (m.x > click_mouse.x)
			Ball[0].rotate_theta.y += -1.f;

		if (m.y < click_mouse.y)
			Camera.revolve_theta.x += 1.f;
		else if (m.y > click_mouse.y)
			Camera.revolve_theta.x += -1.f;

		click_mouse = m;
	}
}

void MouseWheel(int wheel, int diretion, int x, int y)
{
	// 줌인
	if (diretion > 0) {
		Camera.pos.z -= 0.1f;
	}
	// 줌아웃
	else if (diretion < 0) {
		Camera.pos.z += 0.1f;
	}
}

void Init()
{
	glBindVertexArray(vao);
	// 카메라
	Camera.pos = glm::vec3{ 0.f, 0.f, 3.f };

	// Light
	{
		std::ifstream inputFile("./OBJ/sphere.obj");

		if (inputFile.is_open())
			Light.objLoad(inputFile);
		else
			std::cerr << "Failed to obj file" << std::endl;

		Light.pos = Camera.pos + glm::vec3{ 0.f, 0.f, 0.3f };
		Light.scale = glm::vec3{ 0.05f, 0.05f, 0.05f };

		std::vector<glm::vec3> color;
		glm::vec3 a{ 0.23f, 0.52f, 1.0f };
		for (int i = 0; i < Light.face_cnt * 3; ++i) {
			color.emplace_back(a);
		}
		Light.L_color = glm::vec3{ 1.f, 1.f, 1.f };

		glGenBuffers(1, &Light.v_color);
		glBindBuffer(GL_ARRAY_BUFFER, Light.v_color);
		glBufferData(GL_ARRAY_BUFFER, color.size() * sizeof(glm::vec3), color.data(), GL_STATIC_DRAW);
	}

	//  Ball
	{
		std::ifstream inputFile("./OBJ/sphere.obj");
		Ball.emplace_back();

		if (inputFile.is_open())
			Ball.back().objLoad(inputFile);
		else
			std::cerr << "Failed to obj file" << std::endl;

		Ball.back().pos = glm::vec3{ 0.f, 0.f, -1.f };
		Ball.back().scale = glm::vec3{ 0.07f, 0.07f, 0.07f };
		Ball.back().size *= Ball.back().scale;
		Ball.back().velocity = glm::vec3{ 0.f, -0.001f, 0.f };

		Ball.back().imgLoad("./IMG/모몽가.png");
	}

	//  Crystal
	{
		std::ifstream inputFile("./OBJ/pyramid.obj");
		Crystal.emplace_back();

		if (inputFile.is_open())
			Crystal.back().objLoad(inputFile);
		else
			std::cerr << "Failed to obj file" << std::endl;

		Crystal.back().scale = glm::vec3{ 0.01f, 0.01f, 0.01f };
		// 얘는 중앙에 맞춰서 나오게 할람 일케해야댐 걍 얘만 이럼 왜인지는 몰?루겟음 걍 저번 실습에서 대충 만든거 긁어와서 그런듯
		Crystal.back().midpos = glm::vec3{ 0.f, 0.f, 0.f }; 

		Crystal.back().imgLoad("./IMG/유리.png");
	}

	//  Background
	{
		{
			std::ifstream inputFile("./OBJ/skycube.txt");
			Background.emplace_back();

			if (inputFile.is_open())
				Background.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			Background.back().scale = glm::vec3{ 4.f, 4.f, 15.f };
			Background.back().pos += glm::vec3{ 0.f, 0.f, -0.25 * Background.back().scale.z } + Camera.pos;

			std::vector<glm::vec3> color;
			glm::vec3 a{ 242 / 255.f, 255 / 255.f, 237 / 255.f };
			//                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ 요기
			for (int i = 0; i < Background.back().face_cnt * 3; ++i) {
				color.emplace_back(a);
			}

			glGenBuffers(1, &Background.back().v_color);
			glBindBuffer(GL_ARRAY_BUFFER, Background.back().v_color);
			glBufferData(GL_ARRAY_BUFFER, color.size() * sizeof(glm::vec3), color.data(), GL_STATIC_DRAW);
		}
		{
			/*std::ifstream inputFile("./OBJ/cube_tex.obj");
			Background.emplace_back();

			if (inputFile.is_open())
				Background.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			// Background.back().scale = glm::vec3{ 4.f, 4.f, 15.f };
			Background.back().scale = glm::vec3{ 1.f, 1.f, 1.f };
			Background.back().pos = glm::vec3{ 0.f, -1.f, 0.f };
			Background.back().size *= Background.back().scale;

			std::vector<glm::vec3> color;
			glm::vec3 a{ 242 / 255.f, 255 / 255.f, 237 / 255.f };
			//                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ 요기
			for (int i = 0; i < Background.back().face_cnt * 3; ++i) {
				color.emplace_back(a);
			}

			glGenBuffers(1, &Background.back().v_color);
			glBindBuffer(GL_ARRAY_BUFFER, Background.back().v_color);
			glBufferData(GL_ARRAY_BUFFER, color.size() * sizeof(glm::vec3), color.data(), GL_STATIC_DRAW);*/
		}
	}

	// X축 Y축
	{
		glm::vec3 line[6]{
			{1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f}, {0.0f, -1.0f, 0.0f},
			{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}
		};

		glGenBuffers(1, &V_pos_Line);
		glBindBuffer(GL_ARRAY_BUFFER, V_pos_Line);
		glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);

		lineObj = GLLine({ 0.0f, 0.0f, 0.0f });
	}
}

void Mapping() {
	PosLocation = glGetAttribLocation(shaderProgramID, "in_Position"); //	: 0
	ColorLocation = glGetAttribLocation(shaderProgramID, "in_Color"); //	: 1
	NormalLocation = glGetAttribLocation(shaderProgramID, "in_Normal");
	UvLocation = glGetAttribLocation(shaderProgramID, "in_Uv");
	WorldTransLocation = glGetUniformLocation(shaderProgramID, "World_trans");
	CameraLocation = glGetUniformLocation(shaderProgramID, "Camera_trans");
	ProjectionLocation = glGetUniformLocation(shaderProgramID, "Projection_trans");
	TexSamplerLocation = glGetUniformLocation(shaderProgramID, "out_Tex");
	TexorColorLocation = glGetUniformLocation(shaderProgramID, "Tex_or_Color");

	LightPosLocation = glGetUniformLocation(shaderProgramID, "Light_Pos");
	LightColorLocation = glGetUniformLocation(shaderProgramID, "Light_Color");
	ViewPosLocation = glGetUniformLocation(shaderProgramID, "View_Pos");
	DistanceLocation = glGetUniformLocation(shaderProgramID, "Distance");
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h)
{
	winSizex = w;
	winSizey = h;
	glViewport(0, 0, w, h);
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao); //--- VAO 를 지정하고 할당하기
	glBindVertexArray(vao); //--- VAO를 바인드하기
}

void make_shaderProgram()
{
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
	//-- shader Program
	shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);
	glLinkProgram(shaderProgramID);
	//--- 세이더 삭제하기
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	//--- Shader Program 사용하기
	glUseProgram(shaderProgramID);
}

void make_vertexShaders()
{
	vertexSource = filetobuf("vertex.glsl");
	//--- 버텍스 세이더 객체 만들기
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
	//--- 버텍스 세이더 컴파일하기
	glCompileShader(vertexShader);
	//--- 컴파일이 제대로 되지 않은 경우: 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cout << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

void make_fragmentShaders()
{
	fragmentSource = filetobuf("fragment.glsl");
	//--- 프래그먼트 세이더 객체 만들기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);
	//--- 프래그먼트 세이더 컴파일
	glCompileShader(fragmentShader);
	//--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cout << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // Open file for reading 
	if (!fptr) // Return NULL on failure 
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file 
	length = ftell(fptr); // Find out how many bytes into the file we are 
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator 
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file 
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer 
	fclose(fptr); // Close the file 
	buf[length] = 0; // Null terminator 
	return buf; // Return the buffer 
}