#include "stdafx.h"
#include "GLShapes.h"
#include "GLObj.h"
#include "GLLight.h"
#include "GLLIne.h"
#include "GLCamera.h"
#include "GLRay.h"
#include "GLUi.h"
#include "fmod.hpp"
#include "fmod_errors.h" 

using namespace std;

GLchar* vertexSource, * fragmentSource; //--- 소스코드 저장 변수
GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint shaderProgramID; //--- 셰이더 프로그램

void make_shaderProgram();
void make_vertexShaders();
void make_fragmentShaders();
void InitBuffer();
char* filetobuf(const char* file);

GLvoid Mouse(int button, int state, int x, int y);
GLvoid Motion(int x, int y);
GLvoid drawScene();
GLvoid Reshape(int w, int h);
void TimerFunction(int value);
void Keyboard(unsigned char key, int x, int y);
void Special_Keyboard(int key, int x, int y);
void MouseWheel(int wheel, int diretion, int x, int y);
void Init();
void Mapping();

unsigned int PosLocation, ColorLocation, NormalLocation, UvLocation;
unsigned int WorldTransLocation, CameraLocation, ProjectionLocation, TexSamplerLocation, TexorColorLocation, UiboolLocation;
unsigned int LightPosLocation, LightColorLocation, ViewPosLocation, DistanceLocation;
glm::mat4 Projection_Mat = glm::mat4(1.0f);
float winSizex = 0, winSizey = 0;
const float g = 0.0006f;
GLuint vao;
GLLine lineObj;
GLCamera Camera;
GLLight Light;
GLRay Msray; // 마우스 광선

enum ObjectList {
	ball_i, crystal_i, cube_i, fcube_i, obstacle_i
};

enum BallDecoList {
	sunglass1, sunglass2, sunglass3, hat1, hat2, hat3, non_deco
};

enum SoundChannelList {
	bgm_cn, ball_cn, crash_cn
};

enum GameStateList {
	title_s, option_s, custom_s, play_s
};

vector <GLObj> Ball, BallDeco, Crystal, Background, MakeCrashedCrystal, CrashedCrystal, Obstacle, CrashedObstacle;
vector <GLUi> Ui[4];
GLObj Clink;
GLObj obj_list[5], deco_list[6];
bool Lbt = false;
glm::vec3 click_mouse{};
int ball_num = 1; // 한번에 쏘는 공 개수
int GameState = title_s;
GLfloat volumeSize = 1.f, Speed = 0.03f;
int Whatdeco = non_deco;

static FMOD::System* ssystem;
static FMOD::Sound* Crach_Sound[3], * BallShoot_Sound, *Bgm_Sound;
static FMOD::Channel* channel[3] = { 0, 0, 0 };
static FMOD_RESULT result;
static void* extradriverdata = 0;

void UiClick(int what)
{
	switch (GameState)
	{
	case title_s: {
		if (what == 0) {
			GameState = option_s;
		}
		else if (what == 1) {
			GameState = custom_s;

			Ball.clear();
			Ball.emplace_back(obj_list[ball_i]);
			Ball.back().pos = glm::vec3{ 0.f, 0.f, 0.f };
			Ball.back().velocity = glm::vec3{ 0.f, 0.f, 0.f };

			BallDeco.clear();
			
			if (Whatdeco != non_deco)
				BallDeco.emplace_back(deco_list[Whatdeco]);

			Camera.pos = glm::vec3{ 0.f, 0.f, 0.3f };
			Light.pos = Camera.pos + glm::vec3{ 0.f, 0.f, 2.f };
		}
		break;
	}
	case option_s: {
		if (what == 1) {
			GameState = title_s;
		}
		else if (what == 2) {
			if (volumeSize < 2.f)
				volumeSize += 0.2;
		}
		else if (what == 3) {
			if (volumeSize > 0.1f)
				volumeSize -= 0.2;
		}
		else if (what == 4) {
			if (volumeSize == 0.f)
				volumeSize = 1.f;
			else
				volumeSize = 0.f;
		}

		if (volumeSize < 0.1f)
			Ui[option_s][4].now_img = 1;
		else
			Ui[option_s][4].now_img = 0;

		channel[bgm_cn]->setVolume(0.08 * volumeSize);
		break;
	}
	case custom_s: {
		if (what >= 0 && what <= 5) {
			BallDeco.clear();
			BallDeco.emplace_back(deco_list[what]);
			Whatdeco = what;
		}
		else if (what == 6) {
			BallDeco.clear();
			Whatdeco = what;
		}
		else if (what == 7) {
			Ball.clear();
			BallDeco.clear();

			Camera.pos = glm::vec3{ 0.f, 0.f, 3.f };
			Light.pos = Camera.pos + glm::vec3{ 0.f, 0.f, 2.f };
			GameState = title_s;
		}
		break;
	}
	case play_s: {
		break;
	}
	default:
		break;
	}
}

bool CheckCollision(const GLObj& a, const GLObj& b, int what) {
	if (what == crystal_i) {
		GLObj temp_b = b;
		temp_b.pos.y += (temp_b.max.y - temp_b.min.y) / 4.f;

		return (std::abs(a.pos.x - temp_b.pos.x) < (a.size.x + temp_b.size.x) &&
			std::abs(a.pos.y - temp_b.pos.y) < (a.size.y + temp_b.size.y) &&
			std::abs(a.pos.z - temp_b.pos.z) < (a.size.z + temp_b.size.z));
	}
	
	return (std::abs(a.pos.x - b.pos.x) < (a.size.x + b.size.x) &&
			std::abs(a.pos.y - b.pos.y) < (a.size.y + b.size.y) &&
			std::abs(a.pos.z - b.pos.z) < (a.size.z + b.size.z));
}
glm::vec3 CheckCollisionDir(const GLObj& target, const GLObj& object, int what) {
	GLObj temp;
	glm::vec3 result = glm::vec3(1.f);

	temp = object;
	temp.pos.x += object.velocity.x;
	if (CheckCollision(target, temp, what))
		result.x = -1.f;

	temp = object;
	temp.pos.y += object.velocity.y;
	if (CheckCollision(target, temp, what))
		result.y = -1.f;

	temp = object;
	temp.pos.z += object.velocity.z;
	if (CheckCollision(target, temp, what))
		result.z = -1.f;

	return result;
}
bool isPointInsideRectangle(glm::vec2 p, glm::vec2 rt, glm::vec2 lb) {
	//cout << rt.x << ' ' << rt.y << ' ' << lb.x << ' ' << lb.y << ' ' << p.x << ' ' << p.y << endl;
	return (rt.x > p.x) && (p.x > lb.x) && (rt.y > p.y) && (p.y > lb.y);
}
bool FindIntersection(glm::vec2 a1, glm::vec2 a2, glm::vec2 b1, glm::vec2 b2, glm::vec2* intersection) {
	float x1 = a1.x, y1 = a1.y;
	float x2 = a2.x, y2 = a2.y;
	float x3 = b1.x, y3 = b1.y;
	float x4 = b2.x, y4 = b2.y;

	// 두 선분의 방정식 계수
	float A1 = y2 - y1;
	float B1 = x1 - x2;
	float C1 = (x2 * y1) - (x1 * y2);

	float A2 = y4 - y3;
	float B2 = x3 - x4;
	float C2 = (x4 * y3) - (x3 * y4);

	// 두 선분이 평행하거나 일치하는 경우
	if ((A1 * B2 - A2 * B1) == 0)
		return false;

	// 교점의 x, y 좌표 계산
	float det = A1 * B2 - A2 * B1;
	float intersectionX = (B2 * C1 - B1 * C2) / -det;
	float intersectionY = (A1 * C2 - A2 * C1) / -det;

	// 교점이 두 선분 사이에 있는지 확인
	if ((intersectionX >= fmin(x1, x2) && intersectionX <= fmax(x1, x2)) &&
		(intersectionX >= fmin(x3, x4) && intersectionX <= fmax(x3, x4)) &&
		(intersectionY >= fmin(y1, y2) && intersectionY <= fmax(y1, y2)) &&
		(intersectionY >= fmin(y3, y4) && intersectionY <= fmax(y3, y4))) {
		intersection->x = intersectionX;
		intersection->y = intersectionY;
		return true;
	}
	else {
		return false;
	}
}

float CalVectorMagnitude(glm::vec3 v) {
	return glm::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
glm::vec3 PerpendicularInXZPlane(glm::vec3 v) {
	glm::vec3 normal(0, 1, 0);

	glm::vec3 perpendicular;
	perpendicular.x = v.y * normal.z - v.z * normal.y;
	perpendicular.y = 0;
	perpendicular.z = v.x * normal.y - v.y * normal.x;

	return perpendicular;
}
glm::vec3 CalVector(glm::vec3 start, glm::vec3 arrival) {
	return arrival - start;
}
glm::vec3 NormalizeVector(glm::vec3 vector) {
	return vector / CalVectorMagnitude(vector);
}
glm::vec3 CrossProduct(const glm::vec3& A, const glm::vec3& B) {
	return glm::vec3(A.y * B.z - A.z * B.y, A.z * B.x - A.x * B.z, A.x * B.y - A.y * B.x);
}
glm::vec3 CalNormalVector(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C) {
	glm::vec3 AB = CalVector(A, B);
	glm::vec3 AC = CalVector(A, C);

	glm::vec3 normal = CrossProduct(AC, AB);
	normal = NormalizeVector(normal);

	return normal;
}
glm::mat4 CalInverseMatrix(glm::mat4 matrix) {
	using Matrix = std::vector<std::vector<float>>;
	Matrix input = {
		{matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3]},
		{matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3]},
		{matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3]},
		{matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]}
	};
	
	int n = input.size();

	// 확장된 행렬 생성 (원래 행렬과 단위 행렬을 합침)
	Matrix augmented(2 * n, std::vector<float>(2 * n, 0.0f));

	// 원래 행렬 복사
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			augmented[i][j] = input[i][j];
		}
	}

	// 단위 행렬 추가
	for (int i = 0; i < n; ++i) {
		augmented[i][i + n] = 1.0f;
	}

	// 가우스 소거법 수행
	for (int i = 0; i < n; ++i) {
		// 대각원소를 1로 만들기
		float pivot = augmented[i][i];
		for (int j = 0; j < 2 * n; ++j) {
			augmented[i][j] /= pivot;
		}

		// 다른 행들의 대각원소를 0으로 만들기
		for (int k = 0; k < n; ++k) {
			if (k != i) {
				float factor = augmented[k][i];
				for (int j = 0; j < 2 * n; ++j) {
					augmented[k][j] -= factor * augmented[i][j];
				}
			}
		}
	}

	// 역행렬 부분 추출
	Matrix result(n, std::vector<float>(n, 0.0f));
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			result[i][j] = augmented[i][j + n];
		}
	}

	glm::mat4 result_matrix = {
	{result[0][0], result[0][1], result[0][2], result[0][3]},
	{result[1][0], result[1][1], result[1][2], result[1][3]},
	{result[2][0], result[2][1], result[2][2], result[2][3]},
	{result[3][0], result[3][1], result[3][2], result[3][3]}
	};

	return result_matrix;
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
void DeleteObject(GLObj& obj) {
	if (obj.v_pos)
		glDeleteBuffers(1, &obj.v_pos);
	if (obj.v_nor)
		glDeleteBuffers(1, &obj.v_nor);
	if (obj.v_uv)
		glDeleteBuffers(1, &obj.v_uv);
	if (obj.v_color)
		glDeleteBuffers(1, &obj.v_color);
}
glm::vec3 CalFragmentVelocity(GLObj& ball, GLObj& fragment, const char* mode) {
	glm::vec3 velocity = {};
	std::uniform_real_distribution<float> rand_magnitude(0.5f, 0.8f);
	std::uniform_int_distribution<int> rand_bool(0, 1);

	if (mode == "c") {
		velocity = CalVector(ball.pos, fragment.pos + fragment.midpos); // 공에서 조각으로의 벡터 구하기
		//cout << fragment.pos.y + fragment.midpos.y << endl;
		velocity = NormalizeVector(velocity); // 벡터 정규화
		velocity *= CalVectorMagnitude(ball.velocity) * rand_magnitude(gen); // 벡터에 속력 곱하기

		if (fragment.pos.y + fragment.midpos.y < ball.pos.y - 0.25f)
			velocity = glm::vec3(0.f);
		else if (fragment.pos.y + fragment.midpos.y < ball.pos.y - 0.15f && rand_bool(gen))
			velocity = glm::vec3(0.f);
	}
	else if (mode == "ho") {
		//cout << "공\t(" << ball.pos.x << ", " << ball.pos.y << ", " << ball.pos.z << ")" << endl;
		//cout << "장애물\t(" << fragment.pos.x + fragment.midpos.x << ", " << fragment.pos.y + fragment.midpos.y << ", " << fragment.pos.z + fragment.midpos.z << ")" << endl;
		//cout << endl;
		velocity = CalVector(ball.pos, glm::vec3(fragment.pos.x + fragment.midpos.x, fragment.pos.y + fragment.midpos.y, ball.pos.z)); // 공에서 조각으로의 벡터 구하기
		cout << fragment.pos.y + fragment.midpos.y - ball.pos.y << endl;
		//velocity.z *= 2.f; // 공에서 조각으로의 벡터 구하기
		velocity = NormalizeVector(velocity); // 벡터 정규화
		velocity *= CalVectorMagnitude(ball.velocity) / (glm::abs(glm::distance(ball.pos, fragment.pos + fragment.midpos)) * 10.f) / 20.f; // 벡터에 속력 곱하기

		if (fragment.pos.y + fragment.midpos.y > ball.pos.y + 0.25f)
			velocity = glm::vec3(0.f);
		else if (fragment.pos.y + fragment.midpos.y > ball.pos.y + 0.15f && rand_bool(gen))
			velocity = glm::vec3(0.f);
	}
	else if (mode == "wo") {
		//cout << "공\t(" << ball.pos.x << ", " << ball.pos.y << ", " << ball.pos.z << ")" << endl;
		//cout << "장애물\t(" << fragment.pos.x + fragment.midpos.x << ", " << fragment.pos.y + fragment.midpos.y << ", " << fragment.pos.z + fragment.midpos.z << ")" << endl;
		//cout << endl;
		velocity = CalVector(ball.pos, glm::vec3(fragment.pos.x + fragment.midpos.x, fragment.pos.y + fragment.midpos.y, ball.pos.z)); // 공에서 조각으로의 벡터 구하기
		cout << fragment.pos.y + fragment.midpos.y - ball.pos.y << endl;
		//velocity.z *= 2.f; // 공에서 조각으로의 벡터 구하기
		velocity = NormalizeVector(velocity); // 벡터 정규화
		velocity *= CalVectorMagnitude(ball.velocity) / (glm::abs(glm::distance(ball.pos, fragment.pos + fragment.midpos)) * 10.f) / 20.f; // 벡터에 속력 곱하기

		if (fragment.pos.x + fragment.midpos.x > ball.pos.x + 0.25f)
			velocity = glm::vec3(0.f);
		else if (fragment.pos.x + fragment.midpos.x < ball.pos.x - 0.25f)
			velocity = glm::vec3(0.f);
	}

	return velocity;
}
void LoadCrashedCrystal(GLObj& ball, const int& index) {
	std::uniform_real_distribution<float> rand_dir(-0.005f, 0.005f);
	std::uniform_int_distribution<int> rand_bool(0, 1);
	std::uniform_int_distribution<int> rand_sound(0, 2);

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
			CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
			CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
			CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
				CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
				CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
				CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
				CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
				CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
			CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
			CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
			CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
			CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
			CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
			CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
			CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
			CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
			CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
			CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

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
			CrashedCrystal.back().velocity = CalFragmentVelocity(ball, CrashedCrystal.back(), "c");

			CrashedCrystal.back().imgLoad("./IMG/유리.png");
		}
	}

	ssystem->playSound(Crach_Sound[rand_sound(rd)], 0, false, &channel[crash_cn]);
	channel[crash_cn]->setVolume(0.35 * volumeSize);

	Crystal.erase(Crystal.begin() + index);
}
void ShootBall(GLRay ray)
{
	switch (ball_num)
	{
	case 1: {
		Ball.emplace_back(obj_list[ball_i]);
		Ball.back().pos = ray.origin;
		Ball.back().pos.z -= 0.2f;
		Ball.back().velocity = ray.direction / 15.f;

		if (Whatdeco != non_deco) {
			BallDeco.emplace_back(deco_list[Whatdeco]);
			BallDeco.back().pos = Ball.back().pos + BallDeco.back().velocity;
		}
		break;
	}
	case 2: {
		Ball.emplace_back(obj_list[ball_i]);
		Ball.back().pos = ray.origin;
		Ball.back().pos.z -= 0.2f;
		Ball.back().velocity = ray.direction / 15.f;
		Ball.back().velocity.x -= 0.002f;

		if (Whatdeco != non_deco) {
			BallDeco.emplace_back(deco_list[Whatdeco]);
			BallDeco.back().pos = Ball.back().pos + BallDeco.back().velocity;
		}

		Ball.emplace_back(obj_list[ball_i]);
		Ball.back().pos = ray.origin;
		Ball.back().pos.z -= 0.2f;
		Ball.back().velocity = ray.direction / 15.f;
		Ball.back().velocity.x += 0.002f;

		if (Whatdeco != non_deco) {
			BallDeco.emplace_back(deco_list[Whatdeco]);
			BallDeco.back().pos = Ball.back().pos + BallDeco.back().velocity;
		}
		break;
	}
	case 3: {
		Ball.emplace_back(obj_list[ball_i]);
		Ball.back().pos = ray.origin;
		Ball.back().pos.z -= 0.2f;
		Ball.back().velocity = ray.direction / 15.f;
		Ball.back().velocity.y += 0.003f;

		if (Whatdeco != non_deco) {
			BallDeco.emplace_back(deco_list[Whatdeco]);
			BallDeco.back().pos = Ball.back().pos + BallDeco.back().velocity;
		}

		Ball.emplace_back(obj_list[ball_i]);
		Ball.back().pos = ray.origin;
		Ball.back().pos.z -= 0.2f;
		Ball.back().velocity = ray.direction / 15.f;
		Ball.back().velocity.x -= 0.002f;
		Ball.back().velocity.y -= 0.002f;

		if (Whatdeco != non_deco) {
			BallDeco.emplace_back(deco_list[Whatdeco]);
			BallDeco.back().pos = Ball.back().pos + BallDeco.back().velocity;
		}

		Ball.emplace_back(obj_list[ball_i]);
		Ball.back().pos = ray.origin;
		Ball.back().pos.z -= 0.2f;
		Ball.back().velocity = ray.direction / 15.f;
		Ball.back().velocity.x += 0.002f;
		Ball.back().velocity.y -= 0.002f;

		if (Whatdeco != non_deco) {
			BallDeco.emplace_back(deco_list[Whatdeco]);
			BallDeco.back().pos = Ball.back().pos + BallDeco.back().velocity;
		}
		break;
	}
	case 4: {
		Ball.emplace_back(obj_list[ball_i]);
		Ball.back().pos = ray.origin;
		Ball.back().pos.z -= 0.2f;
		Ball.back().velocity = ray.direction / 15.f;
		Ball.back().velocity.x -= 0.002f;
		Ball.back().velocity.y -= 0.003f;

		if (Whatdeco != non_deco) {
			BallDeco.emplace_back(deco_list[Whatdeco]);
			BallDeco.back().pos = Ball.back().pos + BallDeco.back().velocity;
		}

		Ball.emplace_back(obj_list[ball_i]);
		Ball.back().pos = ray.origin;
		Ball.back().pos.z -= 0.2f;
		Ball.back().velocity = ray.direction / 15.f;
		Ball.back().velocity.x += 0.002f;
		Ball.back().velocity.y -= 0.003f;

		if (Whatdeco != non_deco) {
			BallDeco.emplace_back(deco_list[Whatdeco]);
			BallDeco.back().pos = Ball.back().pos + BallDeco.back().velocity;
		}

		Ball.emplace_back(obj_list[ball_i]);
		Ball.back().pos = ray.origin;
		Ball.back().pos.z -= 0.2f;
		Ball.back().velocity = ray.direction / 15.f;
		Ball.back().velocity.x -= 0.002f;
		Ball.back().velocity.y += 0.003f;

		if (Whatdeco != non_deco) {
			BallDeco.emplace_back(deco_list[Whatdeco]);
			BallDeco.back().pos = Ball.back().pos + BallDeco.back().velocity;
		}

		Ball.emplace_back(obj_list[ball_i]);
		Ball.back().pos = ray.origin;
		Ball.back().pos.z -= 0.2f;
		Ball.back().velocity = ray.direction / 15.f;
		Ball.back().velocity.x += 0.002f;
		Ball.back().velocity.y += 0.003f;

		if (Whatdeco != non_deco) {
			BallDeco.emplace_back(deco_list[Whatdeco]);
			BallDeco.back().pos = Ball.back().pos + BallDeco.back().velocity;
		}
		break;
	}
	case 5: {
		Ball.emplace_back(obj_list[ball_i]);
		Ball.back().pos = ray.origin;
		Ball.back().pos.z -= 0.2f;
		Ball.back().velocity = ray.direction / 15.f;
		Ball.emplace_back(obj_list[ball_i]);

		if (Whatdeco != non_deco) {
			BallDeco.emplace_back(deco_list[Whatdeco]);
			BallDeco.back().pos = Ball.back().pos + BallDeco.back().velocity;
		}

		Ball.back().pos = ray.origin;
		Ball.back().pos.z -= 0.2f;
		Ball.back().velocity = ray.direction / 15.f;
		Ball.back().velocity.x -= 0.002f;
		Ball.back().velocity.y -= 0.003f;

		if (Whatdeco != non_deco) {
			BallDeco.emplace_back(deco_list[Whatdeco]);
			BallDeco.back().pos = Ball.back().pos + BallDeco.back().velocity;
		}

		Ball.emplace_back(obj_list[ball_i]);
		Ball.back().pos = ray.origin;
		Ball.back().pos.z -= 0.2f;
		Ball.back().velocity = ray.direction / 15.f;
		Ball.back().velocity.x += 0.002f;
		Ball.back().velocity.y -= 0.003f;

		if (Whatdeco != non_deco) {
			BallDeco.emplace_back(deco_list[Whatdeco]);
			BallDeco.back().pos = Ball.back().pos + BallDeco.back().velocity;
		}

		Ball.emplace_back(obj_list[ball_i]);
		Ball.back().pos = ray.origin;
		Ball.back().pos.z -= 0.2f;
		Ball.back().velocity = ray.direction / 15.f;
		Ball.back().velocity.x -= 0.002f;
		Ball.back().velocity.y += 0.003f;

		if (Whatdeco != non_deco) {
			BallDeco.emplace_back(deco_list[Whatdeco]);
			BallDeco.back().pos = Ball.back().pos + BallDeco.back().velocity;
		}

		Ball.emplace_back(obj_list[ball_i]);
		Ball.back().pos = ray.origin;
		Ball.back().pos.z -= 0.2f;
		Ball.back().velocity = ray.direction / 15.f;
		Ball.back().velocity.x += 0.002f;
		Ball.back().velocity.y += 0.003f;

		if (Whatdeco != non_deco) {
			BallDeco.emplace_back(deco_list[Whatdeco]);
			BallDeco.back().pos = Ball.back().pos + BallDeco.back().velocity;
		}
		break;
	}
	default:
		break;
	}

	if (GameState == play_s) // 앞으로 가는거 상쇄
		Ball.back().velocity.z -= Speed;
	ssystem->playSound(BallShoot_Sound, 0, false, &channel[ball_cn]);
	channel[ball_cn]->setVolume(0.35 * volumeSize);
}
void SaveMap()
{
	// Crystal, Background;

	std::ofstream SaveFlie("./MapList.txt", ios::app);

	if (SaveFlie.is_open()) {
		SaveFlie << "m" << endl;

		for (int i = 0; i < Crystal.size(); ++i) {
			Crystal[i].scale.y /= winSizex / winSizey;
			Crystal[i].pos.y /= winSizex / winSizey;
			SaveFlie << "c " << Crystal[i].pos.x << " " << Crystal[i].pos.y << " " << Crystal[i].pos.z << " "
				<< Crystal[i].scale.x << " " << Crystal[i].scale.y << " " << Crystal[i].scale.z << endl;
		}
		for (int i = 4; i < Background.size(); ++i) {
			Background[i].scale.y /= winSizex / winSizey;
			Background[i].pos.y /= winSizex / winSizey;
			SaveFlie << "b " << Background[i].pos.x << " " << Background[i].pos.y << " " << Background[i].pos.z << " "
				<< Background[i].scale.x << " " << Background[i].scale.y << " " << Background[i].scale.z << endl;
		}
		for (int i = 0; i < Obstacle.size(); ++i) {
			Obstacle[i].scale.y /= winSizex / winSizey;
			Obstacle[i].pos.y /= winSizex / winSizey;
			SaveFlie << "o " << Obstacle[i].pos.x << " " << Obstacle[i].pos.y << " " << Obstacle[i].pos.z << " "
				<< Obstacle[i].scale.x << " " << Obstacle[i].scale.y << " " << Obstacle[i].scale.z  << " "
				<< Obstacle[i].velocity.x << " " << Obstacle[i].velocity.y << " " << Obstacle[i].velocity.z << endl;
		}
	}
}
void LoadMap(int randint)
{
	// Crystal, Background;

	std::ifstream SaveFlie("./MapList.txt"/*, ios::app*/);

	if (SaveFlie.is_open()) {
		std::string bind;

		int mcnt = 0;
		bool select = false;
		while (getline(SaveFlie, bind)) {
			std::stringstream ss_bind{};
			std::string a;
			ss_bind.str(bind);
			ss_bind >> a;

			if (bind[0] == 'm') {
				mcnt++;
				if (mcnt == randint)
					select = true;
				else
					select = false;
			}
			
			if (select) {
				if (bind[0] == 'c') {
					Crystal.emplace_back(obj_list[crystal_i]);
					ss_bind >> Crystal.back().pos.x >> Crystal.back().pos.y >> Crystal.back().pos.z
						>> Crystal.back().scale.x >> Crystal.back().scale.y >> Crystal.back().scale.z;
					Crystal.back().scale.y *= winSizex / winSizey;
					Crystal.back().pos.y *= winSizex / winSizey;
					Crystal.back().pos.z -= 7.f;
					Crystal.back().velocity.z  = Speed;
					WindowConversion(Crystal.back(), winSizex, winSizey);
				}
				else if (bind[0] == 'b') {
					Background.emplace_back(obj_list[fcube_i]);
					ss_bind >> Background.back().pos.x >> Background.back().pos.y >> Background.back().pos.z
						>> Background.back().scale.x >> Background.back().scale.y >> Background.back().scale.z;
					Background.back().scale.y *= winSizex / winSizey;
					Background.back().pos.y *= winSizex / winSizey;
					Background.back().pos.z -= 7.f;
					Background.back().velocity.z = Speed;
					WindowConversion(Background.back(), winSizex, winSizey);
				}
				else if (bind[0] == 'o') {
					Obstacle.emplace_back(obj_list[obstacle_i]);
					ss_bind >> Obstacle.back().pos.x >> Obstacle.back().pos.y >> Obstacle.back().pos.z
						>> Obstacle.back().scale.x >> Obstacle.back().scale.y >> Obstacle.back().scale.z
						>> Obstacle.back().velocity.x >> Obstacle.back().velocity.y >> Obstacle.back().velocity.z;
					Obstacle.back().scale.y *= winSizex / winSizey;
					Obstacle.back().pos.y *= winSizex / winSizey;
					Obstacle.back().pos.z -= 7.f;
					Obstacle.back().velocity.z = Speed;
					WindowConversion(Obstacle.back(), winSizex, winSizey);
				}
			}
		}
	}
}
std::vector <glm::vec3> ClampCrashVertex(std::vector <glm::vec3>& vertex, GLObj& ball, GLObj& obstacle, bool import_angle) {
	std::vector <glm::vec3> result;
	glm::vec2 intersection;
	std::vector <glm::vec2> additional;
	int index;
	glm::vec2 target_vertex[4] = {
		glm::vec2(obstacle.pos.x + obstacle.size.x, obstacle.pos.y + obstacle.size.y),
		glm::vec2(obstacle.pos.x - obstacle.size.x, obstacle.pos.y + obstacle.size.y),
		glm::vec2(obstacle.pos.x - obstacle.size.x, obstacle.pos.y - obstacle.size.y),
		glm::vec2(obstacle.pos.x + obstacle.size.x, obstacle.pos.y - obstacle.size.y)
	};
	float angle[4] = {
		atan2(target_vertex[0].y - ball.pos.y, target_vertex[0].x - ball.pos.x),
		atan2(target_vertex[1].y - ball.pos.y, target_vertex[1].x - ball.pos.x),
		atan2(target_vertex[2].y - ball.pos.y, target_vertex[2].x - ball.pos.x),
		atan2(target_vertex[3].y - ball.pos.y, target_vertex[3].x - ball.pos.x)
	};
	//int dir[2];

	for (int i = 0; i < vertex.size(); i++) {
		index = i - 1 < 0 ? vertex.size() - 1 : i - 1;

		if (isPointInsideRectangle(vertex[index], obstacle.pos + obstacle.size, obstacle.pos - obstacle.size) && isPointInsideRectangle(vertex[i], obstacle.pos + obstacle.size, obstacle.pos - obstacle.size)) {
			result.emplace_back(vertex[i]);
			//cout << "내부 점\t(" << result.back().x << ", " << result.back().y << ")" << endl;
		}
		else {
			//cout << "외부 점\t(" << vertex[index].x << ", " << vertex[index].y << ")\t(" << vertex[i].x << ", " << vertex[i].y << ")" << endl;

			for (int dir = 0; dir < 4; dir++) {
				if (dir == 0) {
					if (FindIntersection(
						vertex[index], vertex[i],
						obstacle.pos + obstacle.size, glm::vec2(obstacle.pos.x + obstacle.size.x, obstacle.pos.y - obstacle.size.y), &intersection)) {
						result.emplace_back(glm::vec3(intersection, vertex[i].z));
						//cout << "교점\t(" << result.back().x << ", " << result.back().y << ")" << endl;
						break;
					}
				}
				else if (dir == 1) {
					if (FindIntersection(
						vertex[index], vertex[i],
						obstacle.pos - obstacle.size, glm::vec2(obstacle.pos.x - obstacle.size.x, obstacle.pos.y + obstacle.size.y), &intersection)) {
						result.emplace_back(glm::vec3(intersection, vertex[i].z));
						//cout << "교점\t(" << result.back().x << ", " << result.back().y << ")" << endl;
						break;
					}
				}
				else if (dir == 2) {
					if (FindIntersection(
						vertex[index], vertex[i],
						obstacle.pos + obstacle.size, glm::vec2(obstacle.pos.x - obstacle.size.x, obstacle.pos.y + obstacle.size.y), &intersection)) {
						result.emplace_back(glm::vec3(intersection, vertex[i].z));
						//cout << "교점\t(" << result.back().x << ", " << result.back().y << ")" << endl;
						break;
					}
				}
				else if (dir == 3) {
					if (FindIntersection(
						vertex[index], vertex[i],
						obstacle.pos - obstacle.size, glm::vec2(obstacle.pos.x + obstacle.size.x, obstacle.pos.y - obstacle.size.y), &intersection)) {
						result.emplace_back(glm::vec3(intersection, vertex[i].z));
						//cout << "교점\t(" << result.back().x << ", " << result.back().y << ")" << endl;
						break;
					}
				}
			}
			if (isPointInsideRectangle(vertex[i], obstacle.pos + obstacle.size, obstacle.pos - obstacle.size)) {
				result.emplace_back(vertex[i]);
				//cout << "내부점\t(" << result.back().x << ", " << result.back().y << ")" << endl;
			}
		}
		//cout << endl;
	}

	return result;
}
std::vector <glm::vec3>  InputVertexIndex(std::vector <glm::vec3>& vertex) {
	std::vector <glm::vec3> objpos;

	if (vertex.size() == 6) {
		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[1]);
		objpos.emplace_back(vertex[2]);

		objpos.emplace_back(vertex[3]);
		objpos.emplace_back(vertex[5]);
		objpos.emplace_back(vertex[4]);

		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[3]);
		objpos.emplace_back(vertex[4]);

		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[4]);
		objpos.emplace_back(vertex[1]);

		objpos.emplace_back(vertex[1]);
		objpos.emplace_back(vertex[4]);
		objpos.emplace_back(vertex[5]);

		objpos.emplace_back(vertex[1]);
		objpos.emplace_back(vertex[5]);
		objpos.emplace_back(vertex[2]);

		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[2]);
		objpos.emplace_back(vertex[5]);

		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[5]);
		objpos.emplace_back(vertex[3]);
	}
	else if (vertex.size() == 8) {
		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[1]);
		objpos.emplace_back(vertex[2]);

		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[2]);
		objpos.emplace_back(vertex[3]);

		objpos.emplace_back(vertex[6]);
		objpos.emplace_back(vertex[5]);
		objpos.emplace_back(vertex[4]);

		objpos.emplace_back(vertex[7]);
		objpos.emplace_back(vertex[6]);
		objpos.emplace_back(vertex[4]);

		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[4]);
		objpos.emplace_back(vertex[5]);

		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[5]);
		objpos.emplace_back(vertex[1]);

		objpos.emplace_back(vertex[1]);
		objpos.emplace_back(vertex[2]);
		objpos.emplace_back(vertex[6]);

		objpos.emplace_back(vertex[1]);
		objpos.emplace_back(vertex[6]);
		objpos.emplace_back(vertex[5]);

		objpos.emplace_back(vertex[2]);
		objpos.emplace_back(vertex[3]);
		objpos.emplace_back(vertex[7]);

		objpos.emplace_back(vertex[2]);
		objpos.emplace_back(vertex[7]);
		objpos.emplace_back(vertex[6]);

		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[3]);
		objpos.emplace_back(vertex[7]);

		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[7]);
		objpos.emplace_back(vertex[4]);
	}
	else if (vertex.size() == 10) {
		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[1]);
		objpos.emplace_back(vertex[2]);

		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[2]);
		objpos.emplace_back(vertex[3]);

		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[3]);
		objpos.emplace_back(vertex[4]);

		objpos.emplace_back(vertex[7]);
		objpos.emplace_back(vertex[6]);
		objpos.emplace_back(vertex[5]);

		objpos.emplace_back(vertex[8]);
		objpos.emplace_back(vertex[7]);
		objpos.emplace_back(vertex[5]);

		objpos.emplace_back(vertex[9]);
		objpos.emplace_back(vertex[8]);
		objpos.emplace_back(vertex[5]);

		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[5]);
		objpos.emplace_back(vertex[6]);

		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[6]);
		objpos.emplace_back(vertex[1]);

		objpos.emplace_back(vertex[1]);
		objpos.emplace_back(vertex[6]);
		objpos.emplace_back(vertex[7]);

		objpos.emplace_back(vertex[1]);
		objpos.emplace_back(vertex[7]);
		objpos.emplace_back(vertex[2]);

		objpos.emplace_back(vertex[2]);
		objpos.emplace_back(vertex[7]);
		objpos.emplace_back(vertex[8]);

		objpos.emplace_back(vertex[2]);
		objpos.emplace_back(vertex[8]);
		objpos.emplace_back(vertex[3]);

		objpos.emplace_back(vertex[3]);
		objpos.emplace_back(vertex[8]);
		objpos.emplace_back(vertex[9]);

		objpos.emplace_back(vertex[3]);
		objpos.emplace_back(vertex[9]);
		objpos.emplace_back(vertex[4]);

		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[4]);
		objpos.emplace_back(vertex[9]);

		objpos.emplace_back(vertex[0]);
		objpos.emplace_back(vertex[4]);
		objpos.emplace_back(vertex[5]);
	}
	else {
		//cout << "버그입니다~ ㅎ" << endl;
	}

	return objpos;
}
void CrashObstacle(GLObj& ball, GLObj& obstacle) {
	uniform_real_distribution<float> rand_range[6];
	std::uniform_real_distribution<float> rand_angle_cnt(glm::radians(15.f), glm::radians(60.f));
	std::vector <float> angle;
	std::vector <float> r[6];
	std::vector <glm::vec3> vertex;
	std::vector <glm::vec3> vertex_temp;
	std::vector<glm::vec3> objnor;
	std::vector <glm::vec3> objpos;
	std::vector<glm::vec2> objtex;
	float range[6] = { 0.03125f, 0.0625f, 0.125f, 0.25f, 0.5f, 1.f };
	float essential_angle[4] = {
		atan2((obstacle.pos.y + obstacle.size.y - ball.pos.y) / (winSizex / winSizey), obstacle.pos.x + obstacle.size.x - ball.pos.x),
		atan2((obstacle.pos.y + obstacle.size.y - ball.pos.y) / (winSizex / winSizey), obstacle.pos.x - obstacle.size.x - ball.pos.x),
		atan2((obstacle.pos.y - obstacle.size.y - ball.pos.y) / (winSizex / winSizey), obstacle.pos.x - obstacle.size.x - ball.pos.x),
		atan2((obstacle.pos.y - obstacle.size.y - ball.pos.y) / (winSizex / winSizey), obstacle.pos.x + obstacle.size.x - ball.pos.x)
	};
	int size;
	glm::vec3 sum{};

	// 깨질 반지름 랜덤생성기 만들기
	for (int i = 0; i < 6; i++) {
		rand_range[i] = uniform_real_distribution<float>(range[i], range[i] * 1.5f);
	}

	// 깨질 각도 생성
	for (int i = 0; i < 4; i++) {
		angle.emplace_back(essential_angle[i]);
	}

	angle.emplace_back(rand_angle_cnt(gen));
	while (angle.back() < glm::radians(360.f)) {
		//cout << glm::degrees(angle.back()) << endl;
		angle.emplace_back(angle.back() + rand_angle_cnt(gen));
	};
	angle.pop_back();
	for (int i = 0; i < angle.size(); i++) {
		while (angle[i] < 0.0) {
			angle[i] += 2 * 3.14f;
		}
		angle[i] = fmod(angle[i], 2 * 3.14f);
	}
	std::sort(angle.begin(), angle.end());

	// 점 얻기

	//cout << "버텍스 정보" << endl;
	//cout << "(" << vertex.back().x << ", " << vertex.back().y << ")" << endl;
	//cout << "v " << vertex.back().x << " " << vertex.back().y << " " << vertex.back().z << endl;
	//cout << "vn " << objnor.back().x << " " << objnor.back().y << " " << objnor.back().z << endl;

	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < angle.size(); j++) {
			r[i].emplace_back(rand_range[i](gen));
		}
	}

	// 원점 포함 조각
	//cout << "------------------------------------원점 포함 조각------------------------------------" << endl;
	for (int j = 0; j < angle.size(); j++) {
		vertex.clear();
		vertex_temp.clear();
		objpos.clear();
		objnor.clear();
		objtex.clear();
		sum = {};

		// 찍어서 나오는 좌표 넣기
		{
			vertex_temp.emplace_back(ball.pos);
			vertex_temp.back().z = obstacle.pos.z + obstacle.size.z;

			vertex_temp.emplace_back();
			vertex_temp.back().x = ball.pos.x + glm::cos(angle[j]) * r[0][j];
			vertex_temp.back().y = ball.pos.y + glm::sin(angle[j]) * r[0][j] * ((float)winSizex / (float)winSizey);
			vertex_temp.back().z = obstacle.pos.z + obstacle.size.z;

			vertex_temp.emplace_back();
			if (j + 1 == angle.size()) {
				vertex_temp.back().x = ball.pos.x + glm::cos(angle[0]) * r[0][0];
				vertex_temp.back().y = ball.pos.y + glm::sin(angle[0]) * r[0][0] * ((float)winSizex / (float)winSizey);
			}
			else {
				vertex_temp.back().x = ball.pos.x + glm::cos(angle[j + 1]) * r[0][j + 1];
				vertex_temp.back().y = ball.pos.y + glm::sin(angle[j + 1]) * r[0][j + 1] * ((float)winSizex / (float)winSizey);
			}
			vertex_temp.back().z = obstacle.pos.z + obstacle.size.z;
			std::reverse(vertex_temp.begin(), vertex_temp.end());
		}
		//for (int i = 0; i < vertex_temp.size(); i++) {
		//	cout << vertex_temp[i].x << ' ' << vertex_temp[i].y << ' ' << vertex_temp[i].z << endl;
		//}
		//cout << endl;

		// 진짜 좌표 넣기
		vertex = ClampCrashVertex(vertex_temp, ball, obstacle, false);
		size = vertex.size();
		for (int i = 0; i < size; i++) {
			vertex.emplace_back(glm::vec3(vertex[i].x, vertex[i].y, obstacle.pos.z - obstacle.size.z));
		}
		for (int i = 0; i < vertex.size(); i++) {
			vertex[i] -= obstacle.pos;
			sum += vertex[i];
		}
		//cout << vertex.size() << endl;
		objpos = InputVertexIndex(vertex);

		//if (objpos.size() == 0)
		//	cout << "버근디?" << endl;
		//for (int i = 0; i < objpos.size(); i++) {
		//	cout << objpos[i].x << ' ' << objpos[i].y << ' ' << objpos[i].z << endl;
		//}
		//cout << endl;

		if (objpos.size()) {
			CrashedObstacle.emplace_back();

			for (int i = 0; i < objpos.size() / 3; i++) {
				objnor.emplace_back(CalNormalVector(objpos[i * 3], objpos[i * 3 + 1], objpos[i * 3 + 2]));
				objnor.emplace_back(objnor.back());
				objnor.emplace_back(objnor.back());
			}

			for (int i = 0; i < objpos.size(); i++) {
				objtex.emplace_back();
			}

			glGenBuffers(1, &CrashedObstacle.back().v_pos);
			glBindBuffer(GL_ARRAY_BUFFER, CrashedObstacle.back().v_pos);
			glBufferData(GL_ARRAY_BUFFER, objpos.size() * sizeof(glm::vec3), objpos.data(), GL_STATIC_DRAW);

			glGenBuffers(1, &CrashedObstacle.back().v_nor);
			glBindBuffer(GL_ARRAY_BUFFER, CrashedObstacle.back().v_nor);
			glBufferData(GL_ARRAY_BUFFER, objnor.size() * sizeof(glm::vec3), objnor.data(), GL_STATIC_DRAW);

			glGenBuffers(1, &CrashedObstacle.back().v_uv);
			glBindBuffer(GL_ARRAY_BUFFER, CrashedObstacle.back().v_uv);
			glBufferData(GL_ARRAY_BUFFER, objtex.size() * sizeof(glm::vec2), objtex.data(), GL_STATIC_DRAW);

			//cout << CrashedObstacle.back().v_pos << ' ' << CrashedObstacle.back().v_nor << ' ' << CrashedObstacle.back().v_uv << endl;

			CrashedObstacle.back().pos = obstacle.pos;
			CrashedObstacle.back().midpos = sum / float(size * 2);
			CrashedObstacle.back().midpos *= CrashedObstacle.back().scale;
			CrashedObstacle.back().face_cnt = objnor.size();

			CrashedObstacle.back().imgLoad("./IMG/장애물.png");
			if (obstacle.scale.x > obstacle.scale.y)
				CrashedObstacle.back().velocity = CalFragmentVelocity(ball, CrashedObstacle.back(), "wo");
			else
				CrashedObstacle.back().velocity = CalFragmentVelocity(ball, CrashedObstacle.back(), "ho");
		}
	}

	// 일반 조각
	//cout << "------------------------------------일반 조각------------------------------------" << endl;
	for (int cnt = 0; cnt < 5; cnt++) {
		for (int j = 0; j < angle.size(); j++) {
			vertex.clear();
			vertex_temp.clear();
			objpos.clear();
			objnor.clear();
			objtex.clear();
			sum = {};

			// 찍어서 나오는 좌표 넣기
			{
				vertex_temp.emplace_back();
				vertex_temp.back().x = ball.pos.x + glm::cos(angle[j]) * r[cnt][j];
				vertex_temp.back().y = ball.pos.y + glm::sin(angle[j]) * r[cnt][j] * ((float)winSizex / (float)winSizey);
				vertex_temp.back().z = obstacle.pos.z + obstacle.size.z;

				vertex_temp.emplace_back();
				if (j + 1 == angle.size()) {
					vertex_temp.back().x = ball.pos.x + glm::cos(angle[0]) * r[cnt][0];
					vertex_temp.back().y = ball.pos.y + glm::sin(angle[0]) * r[cnt][0] * ((float)winSizex / (float)winSizey);
				}
				else {
					vertex_temp.back().x = ball.pos.x + glm::cos(angle[j + 1]) * r[cnt][j + 1];
					vertex_temp.back().y = ball.pos.y + glm::sin(angle[j + 1]) * r[cnt][j + 1] * ((float)winSizex / (float)winSizey);
				}
				vertex_temp.back().z = obstacle.pos.z + obstacle.size.z;

				vertex_temp.emplace_back();
				if (j + 1 == angle.size()) {
					vertex_temp.back().x = ball.pos.x + glm::cos(angle[0]) * r[cnt + 1][0];
					vertex_temp.back().y = ball.pos.y + glm::sin(angle[0]) * r[cnt + 1][0] * ((float)winSizex / (float)winSizey);
				}
				else {
					vertex_temp.back().x = ball.pos.x + glm::cos(angle[j + 1]) * r[cnt + 1][j + 1];
					vertex_temp.back().y = ball.pos.y + glm::sin(angle[j + 1]) * r[cnt + 1][j + 1] * ((float)winSizex / (float)winSizey);
				}
				vertex_temp.back().z = obstacle.pos.z + obstacle.size.z;

				vertex_temp.emplace_back();
				vertex_temp.back().x = ball.pos.x + glm::cos(angle[j]) * r[cnt + 1][j];
				vertex_temp.back().y = ball.pos.y + glm::sin(angle[j]) * r[cnt + 1][j] * ((float)winSizex / (float)winSizey);
				vertex_temp.back().z = obstacle.pos.z + obstacle.size.z;
			}
			//for (int i = 0; i < vertex_temp.size(); i++) {
			//	cout << vertex_temp[i].x << ' ' << vertex_temp[i].y << ' ' << vertex_temp[i].z << endl;
			//}
			//cout << endl;

			// 진짜 좌표 넣기
			vertex = cnt == 2 ? ClampCrashVertex(vertex_temp, ball, obstacle, true) : ClampCrashVertex(vertex_temp, ball, obstacle, false);
			//for (int i = 0; i < vertex_temp.size(); i++) {
			//	cout << vertex_temp[i].x << ' ' << vertex_temp[i].y << ' ' << vertex_temp[i].z << endl;
			//}
			//cout << endl;
			size = vertex.size();
			for (int i = 0; i < size; i++) {
				vertex.emplace_back(glm::vec3(vertex[i].x, vertex[i].y, obstacle.pos.z - obstacle.size.z));
			}
			for (int i = 0; i < vertex.size(); i++) {
				vertex[i] -= obstacle.pos;
				sum += vertex[i];
			}
			//cout << vertex.size() << endl;
			objpos = InputVertexIndex(vertex);
			//if (objpos.size() == 0)
			//	cout << "버근디?" << endl;
			//for (int i = 0; i < objpos.size(); i++) {
			//	cout << objpos[i].x << ' ' << objpos[i].y << ' ' << objpos[i].z << endl;
			//}
			//cout << endl;

			for (int i = 0; i < objpos.size() / 3; i++) {
				objnor.emplace_back(CalNormalVector(objpos[i * 3], objpos[i * 3 + 1], objpos[i * 3 + 2]));
				//cout << "vn " << objnor.back().x << " " << objnor.back().y << " " << objnor.back().z << endl;
				objnor.emplace_back(objnor.back());
				objnor.emplace_back(objnor.back());
			}

			for (int i = 0; i < objpos.size(); i++) {
				objtex.emplace_back();
			}

			if (objpos.size()) {
				CrashedObstacle.emplace_back();
				glGenBuffers(1, &CrashedObstacle.back().v_pos);
				glBindBuffer(GL_ARRAY_BUFFER, CrashedObstacle.back().v_pos);
				glBufferData(GL_ARRAY_BUFFER, objpos.size() * sizeof(glm::vec3), objpos.data(), GL_STATIC_DRAW);

				glGenBuffers(1, &CrashedObstacle.back().v_nor);
				glBindBuffer(GL_ARRAY_BUFFER, CrashedObstacle.back().v_nor);
				glBufferData(GL_ARRAY_BUFFER, objnor.size() * sizeof(glm::vec3), objnor.data(), GL_STATIC_DRAW);

				glGenBuffers(1, &CrashedObstacle.back().v_uv);
				glBindBuffer(GL_ARRAY_BUFFER, CrashedObstacle.back().v_uv);
				glBufferData(GL_ARRAY_BUFFER, objtex.size() * sizeof(glm::vec2), objtex.data(), GL_STATIC_DRAW);

				//cout << CrashedObstacle.back().v_pos << ' ' << CrashedObstacle.back().v_nor << ' ' << CrashedObstacle.back().v_uv << endl;

				CrashedObstacle.back().pos = obstacle.pos;
				CrashedObstacle.back().midpos = sum / float(size * 2);
				CrashedObstacle.back().midpos *= CrashedObstacle.back().scale;
				CrashedObstacle.back().face_cnt = objnor.size();

				CrashedObstacle.back().imgLoad("./IMG/장애물.png");
				if (obstacle.scale.x > obstacle.scale.y)
					CrashedObstacle.back().velocity = CalFragmentVelocity(ball, CrashedObstacle.back(), "wo");
				else
					CrashedObstacle.back().velocity = CalFragmentVelocity(ball, CrashedObstacle.back(), "ho");
			}
		}
	}
}

void MakeCrystal(glm::vec3 pos, glm::vec3 scale) {
	Background.emplace_back(obj_list[fcube_i]);
	Background.back().scale = scale;
	Background.back().pos = pos;
	Crystal.emplace_back(obj_list[crystal_i]);
	Crystal.back().pos = pos;
	Crystal.back().pos.y = pos.y + (scale.y * 0.5f);
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

	glUniform1i(UiboolLocation, false);

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

	if (GameState != custom_s) {
		for (int i = 0; i < Background.size(); ++i) {
			Background[i].Update();
			Background[i].draw_prepare(PosLocation, "Pos");
			Background[i].draw_prepare(ColorLocation, "Color");
			Background[i].draw_prepare(TexorColorLocation, "Color_bool");
			Background[i].draw_prepare(WorldTransLocation, "World");
			Background[i].draw_prepare(NormalLocation, "Normal");
			Background[i].draw_prepare(UvLocation, "UV");
			if (i < 4)
				glUniform1f(DistanceLocation, distance(Light.pos, Background[i].pos) / 3.f);
			else
				glUniform1f(DistanceLocation, distance(Light.pos, Background[i].pos));
			Background[i].draw("solid");
		}
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

	for (int i = 0; i < BallDeco.size(); ++i) {
		BallDeco[i].Update();
		BallDeco[i].draw_prepare(PosLocation, "Pos");
		BallDeco[i].draw_prepare(WorldTransLocation, "World");
		BallDeco[i].draw_prepare(NormalLocation, "Normal");
		BallDeco[i].draw_prepare(UvLocation, "UV");
		BallDeco[i].draw_prepare(false, "Texture");
		BallDeco[i].draw_prepare(TexorColorLocation, "Texture_bool");
		glUniform1f(DistanceLocation, distance(Light.pos, BallDeco[i].pos));
		BallDeco[i].draw("solid");
	}

	if (GameState != custom_s) {
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

		Clink.Update();
		Clink.draw_prepare(PosLocation, "Pos");
		Clink.draw_prepare(WorldTransLocation, "World");
		Clink.draw_prepare(NormalLocation, "Normal");
		Clink.draw_prepare(UvLocation, "UV");
		Clink.draw_prepare(false, "Texture");
		Clink.draw_prepare(TexorColorLocation, "Texture_bool");
		glUniform1f(DistanceLocation, distance(Light.pos, Clink.pos));
		Clink.draw("solid");

		for (int i = Obstacle.size() - 1; i >= 0; --i) {
			Obstacle[i].Update();
			Obstacle[i].draw_prepare(PosLocation, "Pos");
			Obstacle[i].draw_prepare(WorldTransLocation, "World");
			Obstacle[i].draw_prepare(NormalLocation, "Normal");
			Obstacle[i].draw_prepare(UvLocation, "UV");
			Obstacle[i].draw_prepare(false, "Texture");
			Obstacle[i].draw_prepare(TexorColorLocation, "Texture_bool");
			glUniform1f(DistanceLocation, distance(Light.pos, Obstacle[i].pos));
			Obstacle[i].draw("solid");
		}

		for (int i = 0; i < CrashedObstacle.size(); ++i) {
			CrashedObstacle[i].Crystal_Update();
			CrashedObstacle[i].draw_prepare(PosLocation, "Pos");
			CrashedObstacle[i].draw_prepare(WorldTransLocation, "World");
			CrashedObstacle[i].draw_prepare(NormalLocation, "Normal");
			CrashedObstacle[i].draw_prepare(UvLocation, "UV");
			CrashedObstacle[i].draw_prepare(false, "Texture");
			CrashedObstacle[i].draw_prepare(TexorColorLocation, "Texture_bool");
			glUniform1f(DistanceLocation, distance(Light.pos, CrashedObstacle[i].pos));
			CrashedObstacle[i].draw("solid");
		}
	}

	for (int i = 0; i < Ui[GameState].size(); ++i) {
		Ui[GameState][i].draw_prepare(PosLocation, "Pos");
		Ui[GameState][i].draw_prepare(UvLocation, "UV");
		Ui[GameState][i].draw_prepare(Ui[GameState][i].now_img, "Texture");
		Ui[GameState][i].draw_prepare(UiboolLocation, "UI_bool");
		Ui[GameState][i].draw("solid");
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
		if (GameState == play_s) {
			for (int i = 0; i < Crystal.size(); ++i) {
				Crystal[i].pos += Crystal[i].velocity;
				if (Crystal[i].pos.z > Camera.pos.z + 1.f) {
					Crystal.erase(Crystal.begin() + i);
					--i;
				}
			}
			for (int i = 0; i < CrashedCrystal.size(); ++i) {
				CrashedCrystal[i].pos.z += Speed;
				if (CrashedCrystal[i].pos.z > Camera.pos.z + 1.f) {
					DeleteObject(CrashedCrystal[i]);
					CrashedCrystal.erase(CrashedCrystal.begin() + i);
					--i;
				}
			}
			for (int i = 0; i < Ball.size(); ++i) {
				Ball[i].pos.z += Speed;
				if (Ball[i].pos.z > Camera.pos.z + 1.f) {
					if (Whatdeco != non_deco) 
						BallDeco.erase(BallDeco.begin() + i);
					Ball.erase(Ball.begin() + i);
					--i;
				}
			}
			for (int i = 4; i < Background.size(); ++i) {
				Background[i].pos += Background[i].velocity;
				if (Background[i].pos.z > Camera.pos.z + 1.f) {
					Background.erase(Background.begin() + i);
					--i;
				}
			}
			for (int i = 0; i < Obstacle.size(); ++i) {
				Obstacle[i].pos += Obstacle[i].velocity;
				if (Obstacle[i].pos.z > Camera.pos.z + 1.f) {
					Obstacle.erase(Obstacle.begin() + i);
					--i;
				}
			}
			for (int i = 0; i < CrashedObstacle.size(); ++i) {
				CrashedObstacle[i].pos.z += Speed;
				if (CrashedObstacle[i].pos.z > Camera.pos.z + 1.f) {
					DeleteObject(CrashedObstacle[i]);
					CrashedObstacle.erase(CrashedObstacle.begin() + i);
					--i;
				}
			}
			Clink.pos += Clink.velocity;
		}

		// Ball, BallDeco, Crystal, Background, CrashedCrystal, Obstacle, CrashedObstacle;

		// 깨진 크리스탈 움직이기
		for (int i = 0; i < CrashedCrystal.size(); i++) {
			if (CalVectorMagnitude(CrashedCrystal[i].velocity) != 0.f) {
				CrashedCrystal[i].pos += CrashedCrystal[i].velocity;
				CrashedCrystal[i].rotate_theta += PerpendicularInXZPlane(CrashedCrystal[i].velocity) * -200.f;
				CrashedCrystal[i].velocity.y -= g;
				//CrashedCrystal[i].rotate_theta += glm::vec3(1.f);
			}
		}

		// 깨진 오브젝트 움직이기
		for (int i = 0; i < CrashedObstacle.size(); i++) {
			if (CalVectorMagnitude(CrashedObstacle[i].velocity) != 0.f) {
				CrashedObstacle[i].pos += CrashedObstacle[i].velocity;
				CrashedObstacle[i].rotate_theta += NormalizeVector(PerpendicularInXZPlane(CrashedObstacle[i].velocity)) * -2.f;
				CrashedObstacle[i].velocity.y -= g;
				//CrashedObstacle[i].rotate_theta += glm::vec3(1.f);
			}
		}

		GLObj temp;

		for (int i = 0; i < Ball.size(); ++i) {
			temp.pos = Ball[i].pos;

			if (CalVectorMagnitude(Ball[i].velocity) != 0.f) {
				Ball[i].pos += Ball[i].velocity;
				Ball[i].velocity.y -= g;

				// 충돌검사
				// 배경
				for (int bg_cnt = 0; bg_cnt < Background.size(); ++bg_cnt) {
					if (CalVectorMagnitude(Ball[i].velocity) && CheckCollision(Ball[i], Background[bg_cnt], fcube_i) && !CheckCollision(temp, Background[bg_cnt], fcube_i)) {
						Ball[i].pos -= Ball[i].velocity;
						Ball[i].velocity *= CheckCollisionDir(Background[bg_cnt], Ball[i], fcube_i) / 4.f;

						if (CalVectorMagnitude(Ball[i].velocity) < 0.001f)
							Ball[i].velocity = glm::vec3(0.f);
						break;
					}
				}
				// 크리스탈
				for (int c_cnt = 0; c_cnt < Crystal.size(); ++c_cnt) {
					if (CalVectorMagnitude(Ball[i].velocity) && CheckCollision(Ball[i], Crystal[c_cnt], crystal_i) && !CheckCollision(temp, Crystal[c_cnt], crystal_i)) {
						Ball[i].pos -= Ball[i].velocity;
						Ball[i].velocity *= CheckCollisionDir(Crystal[c_cnt], Ball[i], crystal_i) / 4.f;
						Ball[i].velocity.y = glm::abs(Ball[i].velocity.y);

						if (CalVectorMagnitude(Ball[i].velocity) < 0.001f)
							Ball[i].velocity = glm::vec3(0.f);
						LoadCrashedCrystal(Ball[i], c_cnt);

						if (GameState == title_s) {
							GameState = play_s;
							glutTimerFunc(2000, TimerFunction, 2);

							//vector <GLObj> Ball, Crystal, Background, CrashedCrystal, Obstacle, CrashedObstacle;
							for (int i = 0; i < Ball.size(); ++i)
								Ball[i].velocity.z += Speed;
							for (int i = 0; i < Crystal.size(); ++i)
								Crystal[i].velocity.z += Speed;
							for (int i = 0; i < Background.size(); ++i)
								Background[i].velocity.z += Speed;
							for (int i = 0; i < CrashedCrystal.size(); ++i)
								CrashedCrystal[i].velocity.z += Speed;
							for (int i = 0; i < Obstacle.size(); ++i)
								Obstacle[i].velocity.z += Speed;
							for (int i = 0; i < CrashedObstacle.size(); ++i)
								CrashedObstacle[i].velocity.z += Speed;
							Clink.velocity.z += Speed;
						}
						break;
					}
				}
				// 장애물
				for (int o_cnt = 0; o_cnt < Obstacle.size(); ++o_cnt) {
					if (CalVectorMagnitude(Ball[i].velocity) && CheckCollision(Ball[i], Obstacle[o_cnt], cube_i) && !CheckCollision(temp, Obstacle[o_cnt], cube_i)) {
						CrashObstacle(Ball[i], Obstacle[o_cnt]);
						
						Ball[i].pos -= Ball[i].velocity;
						Ball[i].velocity *= CheckCollisionDir(Obstacle[o_cnt], Ball[i], cube_i) / 4.f;

						if (CalVectorMagnitude(Ball[i].velocity) < 0.001f)
							Ball[i].velocity = glm::vec3(0.f);
						Obstacle.erase(Obstacle.begin() + o_cnt);
						break;
					}
				}
			}
		}


		if (Whatdeco != non_deco) {
			for (int i = 0; i < Ball.size(); ++i)
				BallDeco[i].pos = BallDeco[i].velocity + Ball[i].pos;
		}

		glutTimerFunc(10, TimerFunction, 1);
		break;
	}
	case 2: {
		static std::uniform_int_distribution<int> LoadMapRd(1, 5);
		
		LoadMap(LoadMapRd(rd));

		if (GameState == play_s)
			glutTimerFunc(2000, TimerFunction, 2);
		break;
	}
	default:
		break;
	}

	glutPostRedisplay(); // 화면 재 출력
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
	case 'o': {
		for (int i = 0; i < CrashedObstacle.size(); i++) {
			DeleteObject(CrashedObstacle[i]);
		}
		CrashedObstacle.clear();
		Obstacle.emplace_back(obj_list[obstacle_i]);
		Obstacle.back().scale = glm::vec3(1.f, 1.f * winSizex / winSizey, 0.1f);
		break;
	}
	case 's': {
		MakeCrystal({ 0.f, -0.5f, 0.1f }, { 0.5f, 0.5, 0.5 });
		break;
	}
	case '+':
	case '=': {
		ball_num++;
		if (ball_num == 6)
			ball_num = 1;
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
			bool skip = false;
			int Now_state = GameState;

			// 현재 출력중인 ui와의 상호작용 확인
			for (int i = 0; i < Ui[GameState].size(); ++i) {
				if (m.x >= Ui[GameState][i].leftbottom.x && m.y >= Ui[GameState][i].leftbottom.y 
					&& m.x <= Ui[GameState][i].righttop.x && m.y <= Ui[GameState][i].righttop.y) {
					UiClick(i);
					skip = true;
					if (GameState != Now_state || GameState != option_s || i != 0)
						break;
				}
			}

			if ((GameState == title_s || GameState == play_s) && !skip) {
				Msray.ScreenToWorld(x, y, Camera.Camera_Mat, Projection_Mat, winSizex, winSizey);
				ShootBall(Msray);
			}
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

		if (GameState == custom_s) {
			if (m.x < click_mouse.x)
				Ball.back().rotate_theta.y += 1.f;
			else if (m.x > click_mouse.x)
				Ball.back().rotate_theta.y += -1.f;

			if (BallDeco.size())
				BallDeco.back().revolve_theta = Ball.back().rotate_theta;
		}

		click_mouse = m;
	}
}
void MouseWheel(int wheel, int diretion, int x, int y)
{
	static int control = 0;
	// 줌인
	if (GameState == custom_s) {
		if (diretion > 0 && control < 2) {
			control++;
			Camera.pos.z -= 0.06f;
			Light.pos.z -= 0.06f;
		}
		// 줌아웃
		else if (diretion < 0 && control > -2) {
			control--;
			Camera.pos.z += 0.06f;
			Light.pos.z += 0.06f;
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

		Light.pos = Camera.pos + glm::vec3{ 0.f, 0.f, 2.f };
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

	// Obstacle
	{
		std::ifstream inputFile("./OBJ/cube_tex.obj");

		if (inputFile.is_open())
			obj_list[obstacle_i].objLoad(inputFile);
		else
			std::cerr << "Failed to obj file" << std::endl;

		obj_list[obstacle_i].imgLoad("./IMG/장애물.png");
	}

	//  Crystal 안깨진거
	{
		std::ifstream inputFile("./OBJ/crystal.obj");

		if (inputFile.is_open())
			obj_list[crystal_i].objLoad(inputFile);
		else
			std::cerr << "Failed to obj file" << std::endl;

		obj_list[crystal_i].scale = glm::vec3(0.13f, 0.27f, 0.13f);
		obj_list[crystal_i].pos = glm::vec3{ 0.5f, -0.5f, 0.f };
		obj_list[crystal_i].midpos = glm::vec3{ 0.f, 0.f, 0.f };

		obj_list[crystal_i].imgLoad("./IMG/유리.png");
	}

	//  Background
	{
		{
			std::ifstream inputFile("./OBJ/cube_tex.obj");

			if (inputFile.is_open())
				obj_list[cube_i].objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			obj_list[cube_i].scale = glm::vec3{ 4.f, 4.f, 30.f };

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

	//MakeCrystal(glm::vec3{ -0.3, -0.6, 0.f }, glm::vec3{ 0.5, 0.3, 0.5 });
	//MakeCrystal(glm::vec3{ 0.8f, -1.f, -3.f }, glm::vec3{ 0.5, 0.3, 0.5 });
	

	Obstacle.emplace_back(obj_list[obstacle_i]);
	Obstacle.back().pos = glm::vec3(0.f, 0.2, -0.5f);
	Obstacle.back().scale = glm::vec3(0.3, 1.5f, 0.1f);
	Obstacle.back().velocity = glm::vec3(0.002f, 0.f, 0.f);
	Obstacle.emplace_back(obj_list[obstacle_i]);
	Obstacle.back().pos = glm::vec3(0.f, 0.2, -0.5f);
	Obstacle.back().scale = glm::vec3(0.3, 1.5f, 0.1f);
	Obstacle.back().velocity = glm::vec3(-0.002f, 0.f, 0.f);
	Background.emplace_back(obj_list[fcube_i]);
	Background.back().pos = glm::vec3(0.f, 0.5, 0.f);
	Background.back().scale = glm::vec3(1.f, 0.2, 0.3f);

	
	// Clink
	{
		std::ifstream inputFile("./OBJ/Clink.obj");

		if (inputFile.is_open())
			Clink.objLoad(inputFile);
		else
			std::cerr << "Failed to obj file" << std::endl;

		Clink.scale = glm::vec3{ 3.f, 3.f, 3.f };
		Clink.pos = glm::vec3{ 0.f, 0.2f, 0.f };

		Clink.imgLoad("./IMG/유리.png");
	}

	// Ui
	{
		Ui[title_s].emplace_back(GLUi(-0.8f - 0.15f, -0.8f - 0.15f, -0.363f - 0.15f, -0.5f - 0.15f));
		Ui[title_s].back().imgLoad("./IMG/Option_ui.png");
		Ui[title_s].emplace_back(GLUi(0.363f + 0.15f, -0.8f - 0.15f, 0.8 + 0.15f, -0.5f - 0.15f));
		Ui[title_s].back().imgLoad("./IMG/Customizing_ui.png");

		Ui[option_s].emplace_back(GLUi(-1.f, -1.f, 1.f, 1.f, 0.1f));
		Ui[option_s].back().imgLoad("./IMG/gray_background.png");
		Ui[option_s].emplace_back(GLUi(-0.97, 0.7, -0.8, 0.95));
		Ui[option_s].back().imgLoad("./IMG/return.png");
		Ui[option_s].emplace_back(GLUi(0.35f, 0.1f, 0.55f, 0.3f));
		Ui[option_s].back().imgLoad("./IMG/sound_up.png");
		Ui[option_s].emplace_back(GLUi(0.35f, -0.15, 0.55f, 0.05));
		Ui[option_s].back().imgLoad("./IMG/sound_down.png");
		Ui[option_s].emplace_back(GLUi(-0.5f, -0.1, -0.2, 0.25f));
		Ui[option_s].back().imgLoad("./IMG/volume_on.png", "./IMG/volume_off.png");

		Ui[custom_s].emplace_back(GLUi(0.7f, 0.75f, 0.95f, 0.9f));
		Ui[custom_s].back().imgLoad("./IMG/Sunglass1.png");
		Ui[custom_s].emplace_back(GLUi(0.7f, 0.55f, 0.95f, 0.7f));
		Ui[custom_s].back().imgLoad("./IMG/Sunglass2.png");
		Ui[custom_s].emplace_back(GLUi(0.7f, 0.35f, 0.95f, 0.5f));
		Ui[custom_s].back().imgLoad("./IMG/Sunglass3.png");
		Ui[custom_s].emplace_back(GLUi(0.7f, 0.15f, 0.95f, 0.3f));
		Ui[custom_s].back().imgLoad("./IMG/Hat1.png");
		Ui[custom_s].emplace_back(GLUi(0.7f, -0.05f, 0.95f, 0.1f));
		Ui[custom_s].back().imgLoad("./IMG/Hat2.png");
		Ui[custom_s].emplace_back(GLUi(0.7f, -0.25f, 0.95f, -0.1f));
		Ui[custom_s].back().imgLoad("./IMG/Hat3.png");
		Ui[custom_s].emplace_back(GLUi(0.7f, -0.45f, 0.95f, -0.3f));
		Ui[custom_s].back().imgLoad("./IMG/초기화.png");
		Ui[custom_s].emplace_back(GLUi(0.7f, -0.9f, 0.95f, -0.75f));
		Ui[custom_s].back().imgLoad("./IMG/완료.png");
	}

	//  Deco
	{
		{
			std::ifstream inputFile("./OBJ/Sunglass1/Gafas4.obj");

			if (inputFile.is_open())
				deco_list[sunglass1].objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			deco_list[sunglass1].velocity = glm::vec3{ 0.f, 0.01f, 0.03f };
			deco_list[sunglass1].scale = glm::vec3{ 0.024f, 0.024f, 0.024f };
			deco_list[sunglass1].pos = glm::vec3{ 0.f, 0.f, 0.f };

			deco_list[sunglass1].imgLoad("./OBJ/Sunglass1/Environment_Tropical.png");
		}
		{
			std::ifstream inputFile("./OBJ/Sunglass2/Gafas2.obj");

			if (inputFile.is_open())
				deco_list[sunglass2].objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			deco_list[sunglass2].velocity = glm::vec3{ 0.f, 0.008f, 0.03f };
			deco_list[sunglass2].scale = glm::vec3{ 0.022f, 0.022f, 0.022f };
			deco_list[sunglass2].pos = glm::vec3{ 0.f, 0.f, 0.f };

			deco_list[sunglass2].imgLoad("./OBJ/Sunglass2/Environment_Tropical.png");
		}
		{
			std::ifstream inputFile("./OBJ/sunglass3/Gafas3.obj");

			if (inputFile.is_open())
				deco_list[sunglass3].objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			deco_list[sunglass3].velocity = glm::vec3{ 0.f, 0.008f, 0.03f };
			deco_list[sunglass3].scale = glm::vec3{ 0.022f, 0.022f, 0.022f };
			deco_list[sunglass3].pos = glm::vec3{ 0.f, 0.f, 0.f };

			deco_list[sunglass3].imgLoad("./OBJ/sunglass3/Environment_Tropical.png");
		}
		{
			std::ifstream inputFile("./OBJ/Hat1/Accesorios 22.obj");

			if (inputFile.is_open())
				deco_list[hat1].objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			deco_list[hat1].velocity = glm::vec3{ 0.f, 0.06f, -0.04f };
			deco_list[hat1].scale = glm::vec3{ 0.035f, 0.035f, 0.035f };
			deco_list[hat1].pos = glm::vec3{ 0.f, 0.f, 0.f };

			deco_list[hat1].imgLoad("./OBJ/Hat1/mat_SharkHat01_Albedo.png");
		}
		{
			std::ifstream inputFile("./OBJ/Hat2/Gorro_navidad.obj");

			if (inputFile.is_open())
				deco_list[hat2].objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			deco_list[hat2].velocity = glm::vec3{ 0.f, 0.04f, -0.015f };
			deco_list[hat2].scale = glm::vec3{ 0.035f, 0.035f, 0.035f };
			deco_list[hat2].pos = glm::vec3{ 0.f, 0.f, 0.f };

			deco_list[hat2].imgLoad("./OBJ/Hat2/Colores.png");
		}
		{
			std::ifstream inputFile("./OBJ/Hat3/hat_elephant_model.obj");

			if (inputFile.is_open())
				deco_list[hat3].objLoad(inputFile);
			else
				std::cerr << "Failed to obj file" << std::endl;

			deco_list[hat3].velocity = glm::vec3{ 0.f, 0.035f, 0.01f };
			deco_list[hat3].scale = glm::vec3{ 0.0011f, 0.0011f, 0.0011f };
			deco_list[hat3].pos = glm::vec3{ 0.f, 0.f, 0.f };
						  
			deco_list[hat3].imgLoad("./OBJ/Hat3/hat_elephant_color.png");
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

	// 사운드 로드
	{
		result = FMOD::System_Create(&ssystem); //--- 사운드 시스템 생성
		if (result != FMOD_OK)
			exit(255);
		ssystem->init(32, FMOD_INIT_NORMAL, extradriverdata); //--- 사운드 시스템 초기화
		ssystem->createSound("WAV/GlassCrash1.wav", FMOD_LOOP_OFF, 0, &Crach_Sound[0]); //--- 유리 깨지는 소리 1
		ssystem->createSound("WAV/GlassCrash2.wav", FMOD_LOOP_OFF, 0, &Crach_Sound[1]); //--- 유리 깨지는 소리 2
		ssystem->createSound("WAV/GlassCrash3.wav", FMOD_LOOP_OFF, 0, &Crach_Sound[2]); //--- 유리 깨지는 소리 3
		ssystem->createSound("WAV/BallShoot.wav", FMOD_LOOP_OFF, 0, &BallShoot_Sound); //--- 공 쏘는 소리
		ssystem->createSound("WAV/Bgm.mp3", FMOD_LOOP_NORMAL, 0, &Bgm_Sound); //--- BGM
	}
	ssystem->playSound(Bgm_Sound, 0, false, &channel[bgm_cn]);
	channel[bgm_cn]->setVolume(0.08 * volumeSize);
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
	UiboolLocation = glGetUniformLocation(shaderProgramID, "Ui_bool");

	LightPosLocation = glGetUniformLocation(shaderProgramID, "Light_Pos");
	LightColorLocation = glGetUniformLocation(shaderProgramID, "Light_Color");
	ViewPosLocation = glGetUniformLocation(shaderProgramID, "View_Pos");
	DistanceLocation = glGetUniformLocation(shaderProgramID, "Distance");
}
GLvoid Reshape(int w, int h)
{
	WindowConversion(Light, w, h);
	for (int i = 0; i < 3; i++) {
		WindowConversion(obj_list[i], w, h);
	}
	for (int i = 0; i < 3; i++) {
		WindowConversion(deco_list[i], w, h);
	}
	for (int i = 0; i < Ball.size(); i++) {
		WindowConversion(Ball[i], w, h);
	}
	for (int i = 0; i < BallDeco.size(); i++) {
		WindowConversion(BallDeco[i], w, h);
	}
	WindowConversion(Clink, w, h);
	for (int i = 0; i < Crystal.size(); i++) {
		WindowConversion(Crystal[i], w, h);
	}
	for (int i = 0; i < CrashedCrystal.size(); i++) {
		WindowConversion(CrashedCrystal[i], w, h);
		if (winSizex && winSizey)
			CrashedCrystal[i].midpos.y /= winSizex / winSizey;
		CrashedCrystal[i].midpos.y *= (float)w / (float)h;
	}
	for (int i = 0; i < Obstacle.size(); i++) {
		WindowConversion(Obstacle[i], w, h);
	}
	//for (int i = 0; i < CrashedObstacle.size(); i++) {
	//	WindowConversion(CrashedObstacle[i], w, h);
	//	if (winSizex && winSizey)
	//		CrashedObstacle[i].midpos.y /= winSizex / winSizey;
	//	CrashedObstacle[i].midpos.y *= (float)w / (float)h;
	//}
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