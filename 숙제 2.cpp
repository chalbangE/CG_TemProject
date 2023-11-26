#include "stdafx.h"
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
void TimerFunction(int value);
void Keyboard(unsigned char key, int x, int y);
void Special_Keyboard(int key, int x, int y);
void MouseWheel(int wheel, int diretion, int x, int y);

using namespace std;

static std::uniform_real_distribution<GLfloat> VRd(0.005f, 0.01f);
static std::uniform_int_distribution<int> VVRd(0, 1);

float winSizex = 800, winSizey = 800;
GLuint vao;

vector <GLObj> Stick;
GLLine lineObj;
GLCamera Camera;
GLLight Light;
GLObj Floor;

GLfloat Speed = 1.f;

int Mod = 1;

int W_cnt = 25, H_cnt = 25;

GLfloat Zoom = 2.f;
bool Lbt = false;
glm::vec3 click_mouse{};

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(300, 100);
	glutInitWindowSize(winSizex, winSizey);
	glutCreateWindow("#숙제 2");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();
	make_shaderProgram();
	InitBuffer();
	Init();
	//Clear();
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
	int PosLocation = glGetAttribLocation(shaderProgramID, "in_Position"); //	: 0
	int ColorLocation = glGetAttribLocation(shaderProgramID, "in_Color"); //	: 1
	int NormalLocation = glGetAttribLocation(shaderProgramID, "in_Normal");
	unsigned int WorldTransLocation = glGetUniformLocation(shaderProgramID, "World_trans");
	unsigned int CameraLocation = glGetUniformLocation(shaderProgramID, "Camera_trans");
	unsigned int ProjectionLocation = glGetUniformLocation(shaderProgramID, "Projection_trans");
	glEnableVertexAttribArray(PosLocation);
	glEnableVertexAttribArray(ColorLocation);
	glEnableVertexAttribArray(NormalLocation);
	glEnableVertexAttribArray(WorldTransLocation);

	// 프래그먼트 쉐이더에게 전달
	int LightPosLocation = glGetUniformLocation(shaderProgramID, "Light_Pos");
	unsigned int LightColorLocation = glGetUniformLocation(shaderProgramID, "Light_Color");
	unsigned int ViewPosLocation = glGetUniformLocation(shaderProgramID, "View_Pos");
	glEnableVertexAttribArray(LightPosLocation);
	glEnableVertexAttribArray(LightColorLocation);
	glEnableVertexAttribArray(ViewPosLocation);

	// 카메라 변환
	glViewport(0, 0, winSizex, winSizey);
	Camera.pos = glm::vec3{ Zoom, Zoom, Zoom };
	Camera.Update();
	Camera.draw_prepare(ViewPosLocation, "View_Pos");

	// 투영 변환
	glm::mat4 Projection_Mat = glm::mat4(1.0f);
	Projection_Mat = glm::perspective(glm::radians(45.f), 1.f, 0.1f, 50.f);
	glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, &Projection_Mat[0][0]);
	glUniformMatrix4fv(CameraLocation, 1, GL_FALSE, glm::value_ptr(Camera.Camera_Mat));

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_DEPTH_TEST);

	// 광원
	Light.Update();
	Light.draw_prepare(PosLocation, "Pos");
	Light.draw_prepare(ColorLocation, "Color");
	Light.draw_prepare(NormalLocation, "Normal");
	Light.draw_prepare(WorldTransLocation, "World");
	Light.draw_prepare(LightPosLocation, "LightPos");
	Light.draw_prepare(LightColorLocation, "LightColor");
	Light.draw("solid");

	lineObj.Update();
	lineObj.draw_prepare(PosLocation, "Pos");
	lineObj.draw_prepare(ColorLocation, "Color");
	lineObj.draw_prepare(WorldTransLocation, "World");
	lineObj.draw();

	Floor.Update();
	Floor.draw_prepare(PosLocation, "Pos");
	Floor.draw_prepare(ColorLocation, "Color");
	Floor.draw_prepare(WorldTransLocation, "World");
	Floor.draw_prepare(NormalLocation, "Normal");
	Floor.draw("solid");

	for (int i = 0; i < Stick.size(); ++i) {
		Stick[i].Update();
		Stick[i].draw_prepare(PosLocation, "Pos");
		Stick[i].draw_prepare(ColorLocation, "Color");
		Stick[i].draw_prepare(WorldTransLocation, "World");
		Stick[i].draw_prepare(NormalLocation, "Normal");
		Stick[i].draw("solid");
	}

	/*glViewport(620, 620, 180, 180);
	Camera.pos = glm::vec3{ 0.f, 0.f, 3.f };
	Camera.Top_Update();
	Camera.draw_prepare(ViewPosLocation, "View_Pos");

	// 투영 변환
	Projection_Mat = glm::mat4(1.0f);
	Projection_Mat = glm::perspective(glm::radians(0.f), 1.f, 0.1f, 50.f);
	glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, &Projection_Mat[0][0]);

	// 광원
	Light.Update();
	Light.draw_prepare(PosLocation, "Pos");
	Light.draw_prepare(ColorLocation, "Color");
	Light.draw_prepare(NormalLocation, "Normal");
	Light.draw_prepare(WorldTransLocation, "World");
	Light.draw_prepare(LightPosLocation, "LightPos");
	Light.draw_prepare(LightColorLocation, "LightColor");
	Light.draw("solid");

	lineObj.Update();
	lineObj.draw_prepare(PosLocation, "Pos");
	lineObj.draw_prepare(ColorLocation, "Color");
	lineObj.draw_prepare(WorldTransLocation, "World");
	lineObj.draw();

	Floor.Update();
	Floor.draw_prepare(PosLocation, "Pos");
	Floor.draw_prepare(ColorLocation, "Color");
	Floor.draw_prepare(WorldTransLocation, "World");
	Floor.draw_prepare(NormalLocation, "Normal");
	Floor.draw("solid");

	for (int i = 0; i < Stick.size(); ++i) {
		Stick[i].Update();
		Stick[i].draw_prepare(PosLocation, "Pos");
		Stick[i].draw_prepare(ColorLocation, "Color");
		Stick[i].draw_prepare(WorldTransLocation, "World");
		Stick[i].draw_prepare(NormalLocation, "Normal");
		Stick[i].draw("solid");
	}*/

	glDisable(GL_DEPTH_TEST);

	glDisableVertexAttribArray(PosLocation);
	glDisableVertexAttribArray(ColorLocation);
	glDisableVertexAttribArray(NormalLocation);
	glDisableVertexAttribArray(WorldTransLocation);
	glDisableVertexAttribArray(LightPosLocation);
	glDisableVertexAttribArray(LightColorLocation);
	glDisableVertexAttribArray(ViewPosLocation);
	glDisableVertexAttribArray(ProjectionLocation);
	glDisableVertexAttribArray(CameraLocation);

	glutSwapBuffers();
}

void TimerFunction(int value)
{
	switch (value)
	{
	case 1: {
		for (int i = 0; i < Stick.size(); ++i) {
			if (Stick[i].scale.y + (Stick[i].velocity.y * Speed) <= 0.001f) {
				Stick[i].scale.y = 0.001f;
				Stick[i].velocity *= -1;
			}
			else if (Stick[i].scale.y + (Stick[i].velocity.y * Speed) >= 1.5f) {
				Stick[i].scale.y = 1.5f;
				Stick[i].velocity *= -1;
			}

			Stick[i].scale += (Stick[i].velocity * Speed);
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
	case 'a':
	case 'A': {
		Light.revolve_theta.y -= 5.f;
		break;
	}
	case '1': {
		for (int i = 0; i < Stick.size(); ++i) {
			Stick[i].scale.y = 0.5f;
			Stick[i].velocity.y = VRd(rd);
			if (VVRd(rd))
				Stick[i].velocity.y *= -1;
		}
		Mod = key - '0';
		break;
	}
	case '2': {
		for (int i = 0; i < Stick.size(); ++i) {
			Stick[i].scale.y = ((1.4f / float(Stick.size())) * i);
			Stick[i].velocity.y = 0.007f;
		}
		Mod = key - '0';
		break;
	}
	case '3': {
		for (int i = 0; i < Stick.size(); ++i) {

		}
		Mod = key - '0';
		break;
	}
	case 'd':
	case 'D': {
		Light.revolve_theta.y += 5.f;
		break;
	}
	case '+':
	case '=': {
		if (Speed <= 2.f)
			Speed += 0.1f;
		break;
	}
	case '-':
	case '_': {
		if (Speed >= 0.5f)
			Speed -= 0.1f;
		break;
	}
	case 'c':
	case 'C': {
		static int color = 1;
		if (color == 0)
			Light.L_color = glm::vec3{ 1.f, 1.f, 1.f };
		else if (color == 1)
			Light.L_color = glm::vec3{ 1.f, 0.1f, 0.1f };
		else if (color == 2)
			Light.L_color = glm::vec3{ 0.1f, 1.f, 0.1f };
		else if (color == 3) {
			Light.L_color = glm::vec3{ 0.1f, 0.1f, 1.f };
			color = -1;
		}
		color++;
		break;
	}
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
	default:
		break;
	}

	glutPostRedisplay(); // 화면 재 출력
}

void MouseWheel(int wheel, int diretion, int x, int y)
{
	// 줌인
	if (diretion > 0) {
		if (Zoom > 1.f) 
			Zoom -= 0.1f;
	}
	// 줌아웃
	else if (diretion < 0) {
		if (Zoom < 3.f) 
			Zoom += 0.1f;
	}
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
			Camera.revolve_theta.y += 1.f;
		else if (m.x > click_mouse.x)
			Camera.revolve_theta.y += -1.f;

		//if (m.y < click_mouse.y)
		//	Camera.rotate_theta.x += 0.1f;
		//else if (m.y > click_mouse.y)
		//	Camera.rotate_theta.x += -0.1f;

		click_mouse = m;
	}
}

void Init()
{
	glBindVertexArray(vao);
	// 카메라
	Camera.pos = glm::vec3{ Zoom, Zoom, Zoom };

	// Light
	{
		std::ifstream inputFile("./OBJ/sphere.obj");

		if (inputFile.is_open())
			Light.objLoad(inputFile);
		else
			std::cerr << "Failed to obj file" << std::endl;

		Light.pos = Camera.pos;
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

	//  dddd
	{
		//cout << "가로 몇개로 만들까용? : " << endl;
		//cin >> W_cnt;
		//cout << "세로 몇개로 만들까용? : " << endl;
		//cin >> H_cnt;

		std::ifstream inputFile("./OBJ/cube.obj");
		for (int k = 0; k < H_cnt; ++k) {
			for (int i = 0; i < W_cnt; ++i) {
				Stick.emplace_back();

				if (Stick.size() == 1)
					Stick.back().objLoad(inputFile);
				else
					Stick.back() = *Stick.begin();

				Stick.back().pos = glm::vec3{ (0.05 * float(i)), 0.f, (0.05 * float(k)) };
				Stick.back().scale = glm::vec3{ 0.05f, 0.5f, 0.05f };
				Stick.back().velocity = glm::vec3{ 0.f, VRd(rd), 0.f };
				if (VVRd(rd))
					Stick.back().velocity.y *= -1;

				std::vector<glm::vec3> color;
				glm::vec3 a{ 103 / 255.f, 153 / 255.f, 1.f };
				glm::vec3 a1{ 31 / 255.f, 81 / 255.f, 183 / 255.f };
				glm::vec3 a2{ 227 / 255.f, 196 / 255.f, 255 / 255.f };
				glm::vec3 a3{ 137 / 255.f, 106 / 255.f, 183 / 255.f };
				glm::vec3 a4{ 255 / 255.f, 214 / 255.f, 255 / 255.f };
				glm::vec3 a5{ 255 / 255.f, 36 / 255.f, 163 / 255.f };
				for (int i = 0; i < Stick.back().face_cnt * 3; ++i) {
					color.emplace_back(a);
					if (i == 5) a = a1;
					else if (i == 11) a = a2;
					else if (i == 17) a = a3;
					else if (i == 23) a = a4;
					else if (i == 29) a = a5;
				}

				glGenBuffers(1, &Stick.back().v_color);
				glBindBuffer(GL_ARRAY_BUFFER, Stick.back().v_color);
				glBufferData(GL_ARRAY_BUFFER, color.size() * sizeof(glm::vec3), color.data(), GL_STATIC_DRAW);
			}
		}
		
		glm::vec3 mid_move = (Stick.begin()->pos + Stick.back().pos) / 2.f;
		for (int i = 0; i < Stick.size(); ++i) {
			Stick[i].pos -= mid_move;
		}
	}

	// Floor
	{
		std::ifstream inputFile("./OBJ/cube.obj");

		if (inputFile.is_open())
			Floor.objLoad(inputFile);
		else
			std::cerr << "Failed to obj file" << std::endl;

		Floor.pos = glm::vec3{ 0.f, 0.f, 0.f };
		Floor.scale = glm::vec3{ 4.f, 0.001f, 4.f };

		std::vector<glm::vec3> color;
		glm::vec3 a{ 5 / 255.f, 0 / 255.f, 153 / 255.f };
		for (int i = 0; i < Floor.face_cnt * 3; ++i) {
			color.emplace_back(a);
		}

		glGenBuffers(1, &Floor.v_color);
		glBindBuffer(GL_ARRAY_BUFFER, Floor.v_color);
		glBufferData(GL_ARRAY_BUFFER, color.size() * sizeof(glm::vec3), color.data(), GL_STATIC_DRAW);
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

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h)
{
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
	vertexSource = filetobuf("3d_vertex_v3_light.glsl");
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
	fragmentSource = filetobuf("light_fragment.glsl");
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