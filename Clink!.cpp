#include "stdafx.h"
#include "GLShapes.h"
#include "GLObj.h"
#include "GLLight.h"
#include "GLLIne.h"
#include "GLCamera.h"
#include "GLRay.h"

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

void ShootBall(GLRay ray);
void LoadCrashedCrystal(const int& index);
void SaveMap();
void LoadMap();

using namespace std;

float winSizex = 0, winSizey = 0;
const float g = 0.0006f;
GLuint vao;

enum ObjectList {
	ball_i, crystal_i, cube_i, fcube_i
};

vector <GLObj> Ball, Crystal, Background, CrashedCrystal, UI;
GLLine lineObj;
GLCamera Camera;
GLLight Light;
GLRay Msray; // 마우스 광선
unsigned int PosLocation, ColorLocation, NormalLocation, UvLocation;
unsigned int WorldTransLocation, CameraLocation, ProjectionLocation, TexSamplerLocation, TexorColorLocation;
unsigned int LightPosLocation, LightColorLocation, ViewPosLocation, DistanceLocation;

glm::mat4 Projection_Mat = glm::mat4(1.0f);
bool Lbt = false;
glm::vec3 click_mouse{};
GLObj obj_list[4];

bool CheckCollision(const GLObj& a, const GLObj& b) {
	return (std::abs(a.pos.x - b.pos.x) < (a.size.x + b.size.x) &&
		std::abs(a.pos.y - b.pos.y) < (a.size.y + b.size.y) &&
		std::abs(a.pos.z - b.pos.z) < (a.size.z + b.size.z));
}
float CalVectorMagnitude(glm::vec3 v) {
	return glm::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
glm::vec3 CheckCollisionDir(const GLObj& target, const GLObj& object) {
	GLObj temp;
	glm::vec3 result = glm::vec3(1.f);

	temp = object;
	temp.pos.x += object.velocity.x;
	if (CheckCollision(target, temp))
		result.x = -1.f;

	temp = object;
	temp.pos.y += object.velocity.y;
	if (CheckCollision(target, temp))
		result.y = -1.f;

	temp = object;
	temp.pos.z += object.velocity.z;
	if (CheckCollision(target, temp))
		result.z = -1.f;

	return result;
}
glm::vec3 PerpendicularInXZPlane(glm::vec3 v) {
	glm::vec3 normal(0, 1, 0);

	glm::vec3 perpendicular;
	perpendicular.x = v.y * normal.z - v.z * normal.y;
	perpendicular.y = 0;
	perpendicular.z = v.x * normal.y - v.y * normal.x;

	return perpendicular;
}
void WindowConversion(GLObj& obj, int w, int h) {
	if (winSizex && winSizey) {
		obj.scale.y /= winSizex / winSizey;
		obj.pos.y /= winSizex / winSizey;
	}
	obj.scale.y *= (float)w / (float)h;
	obj.pos.y *= (float)w / (float)h;
	obj.size = glm::vec3{ abs((obj.max - obj.min) / 2.f) * obj.scale };
}
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
	Projection_Mat = glm::mat4(1.0f);
	Projection_Mat = glm::perspective(glm::radians(45.f), 1.f, 0.1f, 50.f);
	glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, &Projection_Mat[0][0]);
	glUniformMatrix4fv(CameraLocation, 1, GL_FALSE, glm::value_ptr(Camera.Camera_Mat));

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_DEPTH_TEST);

	// 광원
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

	//lineObj.Update();
	//lineObj.draw_prepare(PosLocation, "Pos");
	//lineObj.draw_prepare(ColorLocation, "Color");
	//lineObj.draw_prepare(TexorColorLocation, "Color_bool");
	//glUniform1i(TexorColorLocation, false);
	//lineObj.draw_prepare(WorldTransLocation, "World");
	//lineObj.draw();

	for (int i = 0; i < Background.size(); ++i) {
		Background[i].Update();
		Background[i].draw_prepare(PosLocation, "Pos");
		Background[i].draw_prepare(ColorLocation, "Color");
		Background[i].draw_prepare(TexorColorLocation, "Color_bool");
		Background[i].draw_prepare(WorldTransLocation, "World");
		Background[i].draw_prepare(NormalLocation, "Normal");
		Background[i].draw_prepare(UvLocation, "UV");
		glUniform1f(DistanceLocation, distance(Light.pos, Background[i].pos));
		Background[i].draw("solid");
	}

	// 알파값 포함 객체 그리기 시작 -------

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int i = 0; i < Ball.size(); ++i) {
		Ball[i].Update();
		Ball[i].draw_prepare(PosLocation, "Pos");
		Ball[i].draw_prepare(WorldTransLocation, "World");
		Ball[i].draw_prepare(NormalLocation, "Normal");
		Ball[i].draw_prepare(UvLocation, "UV");
		Ball[i].draw_prepare(false, "Texture");
		Ball[i].draw_prepare(TexorColorLocation, "Texture_bool");
		glUniform1f(DistanceLocation, distance(Light.pos, Ball[i].pos));
		Ball[i].draw("solid");
	}
	
	for (int i = 0; i < Crystal.size(); ++i) {
		Crystal[i].Update();
		Crystal[i].draw_prepare(PosLocation, "Pos");
		Crystal[i].draw_prepare(WorldTransLocation, "World");
		Crystal[i].draw_prepare(NormalLocation, "Normal");
		Crystal[i].draw_prepare(UvLocation, "UV");
		Crystal[i].draw_prepare(false, "Texture");
		Crystal[i].draw_prepare(TexorColorLocation, "Texture_bool");
		glUniform1f(DistanceLocation, distance(Light.pos, Crystal[i].pos));
		Crystal[i].draw("solid");
	}

	for (int i = 0; i < CrashedCrystal.size(); ++i) {
		CrashedCrystal[i].Crystal_Update();
		CrashedCrystal[i].draw_prepare(PosLocation, "Pos");
		CrashedCrystal[i].draw_prepare(WorldTransLocation, "World");
		CrashedCrystal[i].draw_prepare(NormalLocation, "Normal");
		CrashedCrystal[i].draw_prepare(UvLocation, "UV");
		CrashedCrystal[i].draw_prepare(false, "Texture");
		CrashedCrystal[i].draw_prepare(TexorColorLocation, "Texture_bool");
		glUniform1f(DistanceLocation, distance(Light.pos, CrashedCrystal[i].pos));
		CrashedCrystal[i].draw("solid");
	}

	for (int i = 0; i < UI.size(); ++i) {
		UI[i].Update();
		UI[i].draw_prepare(PosLocation, "Pos");
		UI[i].draw_prepare(WorldTransLocation, "World");
		UI[i].draw_prepare(NormalLocation, "Normal");
		UI[i].draw_prepare(UvLocation, "UV");
		UI[i].draw_prepare(false, "Texture");
		UI[i].draw_prepare(TexorColorLocation, "Texture_bool");
		glUniform1f(DistanceLocation, distance(Light.pos, UI[i].pos));
		UI[i].draw("solid");
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
		for (int i = 0; i < CrashedCrystal.size(); i++) {
			if (CalVectorMagnitude(CrashedCrystal[i].velocity) != 0.f) {
				CrashedCrystal[i].pos += CrashedCrystal[i].velocity;
				CrashedCrystal[i].rotate_theta += PerpendicularInXZPlane(CrashedCrystal[i].velocity) * -200.f;
				CrashedCrystal[i].velocity.y -= g;
				//CrashedCrystal[i].rotate_theta += glm::vec3(1.f);
			}
		}

		GLObj temp;

		for (int i = 0; i < Ball.size(); ++i) {
			temp.pos = Ball[i].pos;

			if (CalVectorMagnitude(Ball[i].velocity) != 0.f) {
				Ball[i].pos += Ball[i].velocity;
				Ball[i].velocity.y -= g;

				// 충돌검사
				for (int bg_cnt = 0; bg_cnt < Background.size(); ++bg_cnt) {
					if (CalVectorMagnitude(Ball[i].velocity) && CheckCollision(Ball[i], Background[bg_cnt]) && !CheckCollision(temp, Background[bg_cnt])) {
						Ball[i].pos -= Ball[i].velocity;
						Ball[i].velocity *= CheckCollisionDir(Background[bg_cnt], Ball[i]) / 4.f;

						if (CalVectorMagnitude(Ball[i].velocity) < 0.001f)
							Ball[i].velocity = glm::vec3(0.f);
						break;
					}
				}
				for (int c_cnt = 0; c_cnt < Crystal.size(); ++c_cnt) {
					if (CalVectorMagnitude(Ball[i].velocity) && CheckCollision(Ball[i], Crystal[c_cnt]) && !CheckCollision(temp, Crystal[c_cnt])) {
						Ball[i].pos -= Ball[i].velocity;
						Ball[i].velocity *= CheckCollisionDir(Crystal[c_cnt], Ball[i]) / 4.f;

						if (CalVectorMagnitude(Ball[i].velocity) < 0.001f)
							Ball[i].velocity = glm::vec3(0.f);
						LoadCrashedCrystal(c_cnt);
						break;
					}
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
		SaveMap();
		exit(829);
	}
	case 's':
	case 'S': {
		LoadMap();
		break;
	}
	case 'c': {
		Crystal.emplace_back(obj_list[crystal_i]);
		CrashedCrystal.clear();	
		break;
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

			Msray.ScreenToWorld(x, y, Camera.Camera_Mat, Projection_Mat, winSizex, winSizey);
			ShootBall(Msray);
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

		//if (m.x < click_mouse.x)
		//	Camera.revolve_theta.y += 1.f;
		//else if (m.x > click_mouse.x)
		//	Camera.revolve_theta.y += -1.f;

		//if (m.y < click_mouse.y)
		//	Camera.revolve_theta.x += 1.f;
		//else if (m.y > click_mouse.y)
		//	Camera.revolve_theta.x += -1.f;

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

void ShootBall(GLRay ray)
{
	Ball.emplace_back(obj_list[ball_i]);
	Ball.back().pos = ray.origin;
	Ball.back().pos.z -= 0.2f;
	Ball.back().velocity = ray.direction / 15.f;
}

// 크리스탈 부서진 obj로 교체하고 기존거 삭제하는 함수
void LoadCrashedCrystal(const int& index) {
	std::uniform_real_distribution<float> rand_dir(-0.005f, 0.005f);
	std::uniform_int_distribution<int> rand_bool(0, 1);

	if (0) {
		{
			std::ifstream inputFile("./OBJ/crystal1.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/crystal2.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/crystal3.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/crystal4.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			if (rand_bool(gen))
				CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/crystal5.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			if (rand_bool(gen))
				CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/crystal6.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			if (rand_bool(gen))
				CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/crystal7.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			if (rand_bool(gen))
				CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/crystal8.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			if (rand_bool(gen))
				CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
	}
	else {
		{
			std::ifstream inputFile("./OBJ/c1.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/c2.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/c3.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/c4.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/c5.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/c6.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/c7.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/c8.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/c9.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/c10.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
		{
			std::ifstream inputFile("./OBJ/c11.obj");
			CrashedCrystal.emplace_back();

			if (inputFile.is_open())
				CrashedCrystal.back().objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			CrashedCrystal.back().scale = Crystal[index].scale;
			CrashedCrystal.back().pos = Crystal[index].pos;
			CrashedCrystal.back().midpos *= CrashedCrystal.back().scale;
			CrashedCrystal.back().velocity = glm::vec3{ rand_dir(gen), 0.f, rand_dir(gen) };

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
	}

	Crystal.erase(Crystal.begin() + index);
}

void SaveMap()
{
	// Crystal, Background;

	std::ofstream SaveFlie("./MapList.txt"/*, ios::app*/);

	if (SaveFlie.is_open()) {
		SaveFlie << "m" << endl;

		for (int i = 0; i < Crystal.size(); ++i) {
			Crystal.back().scale.y /= winSizex / winSizey;
			Crystal.back().pos.y /= winSizex / winSizey;
			SaveFlie << "c " << Crystal[i].pos.x << " " << Crystal[i].pos.y << " " << Crystal[i].pos.z << " "
				<< Crystal[i].scale.x << " " << Crystal[i].scale.y << " " << Crystal[i].scale.z << endl;
		}
		for (int i = 4; i < Background.size(); ++i) {
			Background.back().scale.y /= winSizex / winSizey;
			Background.back().pos.y /= winSizex / winSizey;
			SaveFlie << "b " << Background[i].pos.x << " " << Background[i].pos.y << " " << Background[i].pos.z << " "
				<< Background[i].scale.x << " " << Background[i].scale.y << " " << Background[i].scale.z << endl;
		}
	}
}

void LoadMap()
{
	// Crystal, Background;

	std::ifstream SaveFlie("./MapList.txt"/*, ios::app*/);

	if (SaveFlie.is_open()) {
		std::string bind;

		while (getline(SaveFlie, bind)) {
			std::stringstream ss_bind{};
			std::string a;
			ss_bind.str(bind);
			ss_bind >> a;
			cout << a << endl;

			if (bind[0] == 'c') {
				Crystal.emplace_back(obj_list[crystal_i]);
				ss_bind >> Crystal.back().pos.x >> Crystal.back().pos.y >> Crystal.back().pos.z
					>> Crystal.back().scale.x >> Crystal.back().scale.y >> Crystal.back().scale.z;
				Crystal.back().scale.y *= winSizex / winSizey;
				Crystal.back().pos.y *= winSizex / winSizey;
			}
			else if (bind[0] == 'b') {
				Background.emplace_back(obj_list[fcube_i]);
				ss_bind >> Background.back().pos.x  >> Background.back().pos.y  >> Background.back().pos.z
					>> Background.back().scale.x  >> Background.back().scale.y  >> Background.back().scale.z;
				Background.back().scale.y *= winSizex / winSizey;
				Background.back().pos.y *= winSizex / winSizey;
			}
		}
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
		Light.scale = glm::vec3(0.f);

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

		if (inputFile.is_open())
			obj_list[ball_i].objLoad(inputFile);
		else
			std::cerr << "Failed to obj file" << std::endl;

		obj_list[ball_i].pos = glm::vec3{ 0.f, 0.f, 0.f };
		obj_list[ball_i].scale = glm::vec3{ 0.03f, 0.03f, 0.03f };
		obj_list[ball_i].velocity = glm::vec3{ 0.f, -0.001f, 0.f };

		obj_list[ball_i].imgLoad("./IMG/iron.png");
	}

	//  Crystal 안깨진거
	{
		std::ifstream inputFile("./OBJ/crystal.obj");

		if (inputFile.is_open())
			obj_list[crystal_i].objLoad(inputFile);
		else
			std::cerr << "Failed to obj file" << std::endl;

		obj_list[crystal_i].scale = glm::vec3(0.13f, 0.27f, 0.13f);
		obj_list[crystal_i].pos = glm::vec3{ 0.f, -0.5f, 0.f };
		obj_list[crystal_i].midpos = glm::vec3{ 0.f, 0.f, 0.f };

		obj_list[crystal_i].imgLoad("./IMG/유리.png");
	}
	//Crystal.emplace_back(obj_list[crystal_i]);

	//  Background
	{
		{
			std::ifstream inputFile("./OBJ/cube_tex.obj");

			if (inputFile.is_open())
				obj_list[cube_i].objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			obj_list[cube_i].scale = glm::vec3{ 4.f, 4.f, 15.f };

			std::vector<glm::vec3> color;
			glm::vec3 a{ 242 / 255.f, 255 / 255.f, 237 / 255.f };
			//                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ 요기
			for (int i = 0; i < obj_list[cube_i].face_cnt * 3; ++i) {
				color.emplace_back(a);
			}

			glGenBuffers(1, &obj_list[cube_i].v_color);
			glBindBuffer(GL_ARRAY_BUFFER, obj_list[cube_i].v_color);
			glBufferData(GL_ARRAY_BUFFER, color.size() * sizeof(glm::vec3), color.data(), GL_STATIC_DRAW);
		}

		Background.emplace_back(obj_list[cube_i]);
		Background.back().pos += glm::vec3{ -2.f, 0.f, -0.25 * Background.back().scale.z } + Camera.pos;
		Background.emplace_back(obj_list[cube_i]);
		Background.back().pos += glm::vec3{ 2.f, 0.f, -0.25 * Background.back().scale.z } + Camera.pos;
		Background.emplace_back(obj_list[cube_i]);
		Background.back().pos += glm::vec3{ 0.f, -2.f, -0.25 * Background.back().scale.z } + Camera.pos;
		Background.emplace_back(obj_list[cube_i]);
		Background.back().pos += glm::vec3{ 0.f, 2.f, -0.25 * Background.back().scale.z } + Camera.pos;
	}

	// 바닥에 붙어있는 cube
	{
		std::ifstream inputFile("./OBJ/cube_floor.obj");

		if (inputFile.is_open())
			obj_list[fcube_i].objLoad(inputFile);
		else
			std::cerr << "Failed to obj file" << std::endl;

		obj_list[fcube_i].scale = glm::vec3{ 1.f, 1.f, 1.f };
		obj_list[fcube_i].pos = glm::vec3{ 0.f, -1.f, 0.f };

		std::vector<glm::vec3> color;
		glm::vec3 a{ 242 / 255.f, 255 / 255.f, 237 / 255.f };
		//                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ 요기
		for (int i = 0; i < obj_list[fcube_i].face_cnt * 3; ++i) {
			color.emplace_back(a);
		}

		glGenBuffers(1, &obj_list[fcube_i].v_color);
		glBindBuffer(GL_ARRAY_BUFFER, obj_list[fcube_i].v_color);
		glBufferData(GL_ARRAY_BUFFER, color.size() * sizeof(glm::vec3), color.data(), GL_STATIC_DRAW);
	}
	//Background.emplace_back(obj_list[fcube_i]);

	// UI
	{
		std::ifstream inputFile("./OBJ/Clink.obj");
		UI.emplace_back();

		if (inputFile.is_open())
			UI.back().objLoad(inputFile);
		else
			std::cerr << "Failed to obj file" << std::endl;

		UI.back().scale = glm::vec3{ 3.f, 3.f, 3.f };
		UI.back().pos = glm::vec3{ 0.f, 0.2f, 0.f };

		UI.back().imgLoad("./IMG/유리.png");
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
	WindowConversion(Light, w, h);
	for (int i = 0; i < 3; i++) {
		WindowConversion(obj_list[i], w, h);
	}
	for (int i = 0; i < Ball.size(); i++) {
		WindowConversion(Ball[i], w, h);
	}
	for (int i = 0; i < UI.size(); i++) {
		WindowConversion(UI[i], w, h);
	}
	for (int i = 0; i < Crystal.size(); i++) {
		WindowConversion(Crystal[i], w, h);
	}
	for (int i = 0; i < CrashedCrystal.size(); i++) {
		WindowConversion(CrashedCrystal[i], w, h);
		if (winSizex && winSizey)
			CrashedCrystal[i].midpos.y /= winSizex / winSizey;
		CrashedCrystal[i].midpos.y *= (float)w / (float)h;
	}
	for (int i = 0; i < Background.size(); i++) {
		WindowConversion(Background[i], w, h);
	}

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