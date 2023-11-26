#include "GLObj.h"

GLObj::GLObj(glm::vec3 m) : GLShapes(m) {

}

void GLObj::objLoad(std::ifstream& path) {
	char count[100];
	int vertexNum = 0;
	int faceNum = 0;
	int normalNum = 0;
	int textureNum = 0;

	// 끝날때까지로 수정하기
	while (path >> count) {
		if (count[0] == 'v' && count[1] == '\0')
			vertexNum += 1;
		else if (count[0] == 'v' && count[1] == 'n' && count[2] == '\0')
			normalNum += 1;
		else if (count[0] == 'f' && count[1] == '\0')
			faceNum += 1;
		else if (count[0] == 'v' && count[1] == 't' && count[2] == '\0')
			textureNum += 1;
		memset(count, '\0', sizeof(count));
	}

	glm::vec4* v_face = new glm::vec4[faceNum * 2];
	glm::vec4* vn_face = new glm::vec4[faceNum * 2];
	glm::vec4* vt_face = new glm::vec4[faceNum * 2];
	glm::vec4* vertex = new glm::vec4[vertexNum * 2];
	glm::vec4* normal = new glm::vec4[normalNum * 2];
	glm::vec4* texture = new glm::vec4[textureNum * 2];
	std::string bind;
	int vertIndex = 0;
	int faceIndex = 0;
	int normalIndex = 0;
	int textureIndex = 0;
	glm::vec3 sum{}, avg{};
	std::vector<glm::vec3> objpos;
	std::vector<glm::vec3> objnor;
	std::vector<glm::vec2> objtex;

	path.clear();
	path.seekg(0, std::ios::beg);
	while (getline(path, bind)) {
		if (bind[0] == 'v' && bind[1] == ' ') {
			std::stringstream ss_bind{};

			for (int i = 0; i < bind.size(); ++i) {
				if (bind[i] == '/')
					bind[i] = ' ';
			}

			ss_bind.str(bind);

			std::string a;
			ss_bind >> a;

			ss_bind >> vertex[vertIndex].x >> vertex[vertIndex].y >> vertex[vertIndex].z;

			if (vertex[vertIndex].x > max.x)	max.x = vertex[vertIndex].x;
			if (vertex[vertIndex].y > max.y)	max.y = vertex[vertIndex].y;
			if (vertex[vertIndex].z > max.z)	max.z = vertex[vertIndex].z;

			if (vertex[vertIndex].x < min.x)	min.x = vertex[vertIndex].x;
			if (vertex[vertIndex].y < min.y)	min.y = vertex[vertIndex].y;
			if (vertex[vertIndex].z < min.z)	min.z = vertex[vertIndex].z;

			sum.x += vertex[vertIndex].x;
			sum.y += vertex[vertIndex].y;
			sum.z += vertex[vertIndex].z;

			vertIndex++;
		}
		if (bind[0] == 'v' && bind[1] == 'n' && bind[2] == ' ') {
			std::stringstream ss_bind{};

			for (int i = 0; i < bind.size(); ++i) {
				if (bind[i] == '/')
					bind[i] = ' ';
			}

			ss_bind.str(bind);

			std::string a;
			ss_bind >> a;

			ss_bind >> normal[normalIndex].x >> normal[normalIndex].y >> normal[normalIndex].z;

			normalIndex++;
		}
		else if (bind[0] == 'f' && bind[1] == ' ') {
			std::stringstream ss_bind{};

			for (int i = 0; i < bind.size(); ++i) {
				if (bind[i] == '/')
					bind[i] = ' ';
			}

			ss_bind.str(bind);

			std::string a;
			// 'f' 빼기
			ss_bind >> a;

			int fcnt = 0;
			while (ss_bind >> a) {
				fcnt++;
			}	
			ss_bind.clear();
			ss_bind.str(bind);

			ss_bind >> a;

			if (fcnt == 6) {
				for (int i = 0; i < 3; i++) {
					ss_bind >> v_face[faceIndex][i];
					ss_bind >> vn_face[faceIndex][i];
				}

				glm::vec3 a[3]{
					{ vertex[static_cast<int>(v_face[faceIndex][0] - 1)].x, vertex[static_cast<int>(v_face[faceIndex][0] - 1)].y, vertex[static_cast<int>(v_face[faceIndex][0] - 1)].z},
					{ vertex[static_cast<int>(v_face[faceIndex][1] - 1)].x, vertex[static_cast<int>(v_face[faceIndex][1] - 1)].y, vertex[static_cast<int>(v_face[faceIndex][1] - 1)].z },
					{ vertex[static_cast<int>(v_face[faceIndex][2] - 1)].x, vertex[static_cast<int>(v_face[faceIndex][2] - 1)].y, vertex[static_cast<int>(v_face[faceIndex][2] - 1)].z}
				};

				glm::vec3 n[3]{
					{ normal[static_cast<int>(vn_face[faceIndex][0] - 1)].x, normal[static_cast<int>(vn_face[faceIndex][0] - 1)].y, normal[static_cast<int>(vn_face[faceIndex][0] - 1)].z},
					{ normal[static_cast<int>(vn_face[faceIndex][1] - 1)].x, normal[static_cast<int>(vn_face[faceIndex][1] - 1)].y, normal[static_cast<int>(vn_face[faceIndex][1] - 1)].z },
					{ normal[static_cast<int>(vn_face[faceIndex][2] - 1)].x, normal[static_cast<int>(vn_face[faceIndex][2] - 1)].y, normal[static_cast<int>(vn_face[faceIndex][2] - 1)].z}
				};

				objpos.push_back(a[0]);
				objpos.push_back(a[1]);
				objpos.push_back(a[2]);

				objnor.push_back(n[0]);
				objnor.push_back(n[1]);
				objnor.push_back(n[2]);
			}
			else if (fcnt == 9) {
				for (int i = 0; i < 3; i++) {
					ss_bind >> v_face[faceIndex][i];
					ss_bind >> vt_face[faceIndex][i];
					ss_bind >> vn_face[faceIndex][i];
				}

				glm::vec3 a[3]{
					{ vertex[static_cast<int>(v_face[faceIndex][0] - 1)].x, vertex[static_cast<int>(v_face[faceIndex][0] - 1)].y, vertex[static_cast<int>(v_face[faceIndex][0] - 1)].z},
					{ vertex[static_cast<int>(v_face[faceIndex][1] - 1)].x, vertex[static_cast<int>(v_face[faceIndex][1] - 1)].y, vertex[static_cast<int>(v_face[faceIndex][1] - 1)].z },
					{ vertex[static_cast<int>(v_face[faceIndex][2] - 1)].x, vertex[static_cast<int>(v_face[faceIndex][2] - 1)].y, vertex[static_cast<int>(v_face[faceIndex][2] - 1)].z}
				};

				glm::vec3 n[3]{
					{ normal[static_cast<int>(vn_face[faceIndex][0] - 1)].x, normal[static_cast<int>(vn_face[faceIndex][0] - 1)].y, normal[static_cast<int>(vn_face[faceIndex][0] - 1)].z},
					{ normal[static_cast<int>(vn_face[faceIndex][1] - 1)].x, normal[static_cast<int>(vn_face[faceIndex][1] - 1)].y, normal[static_cast<int>(vn_face[faceIndex][1] - 1)].z },
					{ normal[static_cast<int>(vn_face[faceIndex][2] - 1)].x, normal[static_cast<int>(vn_face[faceIndex][2] - 1)].y, normal[static_cast<int>(vn_face[faceIndex][2] - 1)].z}
				};

				glm::vec3 t[3]{
					{ texture[static_cast<int>(vt_face[faceIndex][0] - 1)].x, texture[static_cast<int>(vt_face[faceIndex][0] - 1)].y, texture[static_cast<int>(vt_face[faceIndex][0] - 1)].z},
					{ texture[static_cast<int>(vt_face[faceIndex][1] - 1)].x, texture[static_cast<int>(vt_face[faceIndex][1] - 1)].y, texture[static_cast<int>(vt_face[faceIndex][1] - 1)].z },
					{ texture[static_cast<int>(vt_face[faceIndex][2] - 1)].x, texture[static_cast<int>(vt_face[faceIndex][2] - 1)].y, texture[static_cast<int>(vt_face[faceIndex][2] - 1)].z}
				};

				objpos.push_back(a[0]);
				objpos.push_back(a[1]);
				objpos.push_back(a[2]);

				objnor.push_back(n[0]);
				objnor.push_back(n[1]);
				objnor.push_back(n[2]);

				objtex.push_back(t[0]);
				objtex.push_back(t[1]);
				objtex.push_back(t[2]);
			}
			else if (fcnt == 12) {
				for (int i = 0; i < 4; i++) {
					ss_bind >> v_face[faceIndex][i];
					ss_bind >> vt_face[faceIndex][i];
					ss_bind >> vn_face[faceIndex][i];
				}

				glm::vec3 a[4]{
					{ vertex[static_cast<int>(v_face[faceIndex].x - 1)].x, vertex[static_cast<int>(v_face[faceIndex].x - 1)].y, vertex[static_cast<int>(v_face[faceIndex].x - 1)].z },
					{ vertex[static_cast<int>(v_face[faceIndex].y - 1)].x, vertex[static_cast<int>(v_face[faceIndex].y - 1)].y, vertex[static_cast<int>(v_face[faceIndex].y - 1)].z },
					{ vertex[static_cast<int>(v_face[faceIndex].z - 1)].x, vertex[static_cast<int>(v_face[faceIndex].z - 1)].y, vertex[static_cast<int>(v_face[faceIndex].z - 1)].z },
					{ vertex[static_cast<int>(v_face[faceIndex].a - 1)].x, vertex[static_cast<int>(v_face[faceIndex].a - 1)].y, vertex[static_cast<int>(v_face[faceIndex].a - 1)].z}
				};

				glm::vec3 n[4]{
					{ normal[static_cast<int>(vn_face[faceIndex].x - 1)].x, normal[static_cast<int>(vn_face[faceIndex].x - 1)].y, normal[static_cast<int>(vn_face[faceIndex].x - 1)].z },
					{ normal[static_cast<int>(vn_face[faceIndex].y - 1)].x, normal[static_cast<int>(vn_face[faceIndex].y - 1)].y, normal[static_cast<int>(vn_face[faceIndex].y - 1)].z },
					{ normal[static_cast<int>(vn_face[faceIndex].z - 1)].x, normal[static_cast<int>(vn_face[faceIndex].z - 1)].y, normal[static_cast<int>(vn_face[faceIndex].z - 1)].z },
					{ normal[static_cast<int>(vn_face[faceIndex].a - 1)].x, normal[static_cast<int>(vn_face[faceIndex].a - 1)].y, normal[static_cast<int>(vn_face[faceIndex].a - 1)].z}
				};
				
				glm::vec2 t[4]{
					{ texture[static_cast<int>(vt_face[faceIndex].x - 1)].x, texture[static_cast<int>(vt_face[faceIndex].x - 1)].y },
					{ texture[static_cast<int>(vt_face[faceIndex].y - 1)].x, texture[static_cast<int>(vt_face[faceIndex].y - 1)].y },
					{ texture[static_cast<int>(vt_face[faceIndex].z - 1)].x, texture[static_cast<int>(vt_face[faceIndex].z - 1)].y },
					{ texture[static_cast<int>(vt_face[faceIndex].a - 1)].x, texture[static_cast<int>(vt_face[faceIndex].a - 1)].y }
				};

				objpos.push_back(a[0]);
				objpos.push_back(a[1]);
				objpos.push_back(a[2]);

				objpos.push_back(a[0]);
				objpos.push_back(a[2]);
				objpos.push_back(a[3]);

				objnor.push_back(n[0]);
				objnor.push_back(n[1]);
				objnor.push_back(n[2]);

				objnor.push_back(n[0]);
				objnor.push_back(n[2]);
				objnor.push_back(n[3]);

				objtex.push_back(t[0]);
				objtex.push_back(t[1]);
				objtex.push_back(t[2]);
					
				objtex.push_back(t[0]);
				objtex.push_back(t[2]);
				objtex.push_back(t[3]);

				faceIndex++;
			}

			faceIndex++;
		}
	}
	midpos = sum / float(vertIndex);
	pos = glm::vec3{ 0.f, 0.f, 0.f };
	size = glm::vec3{ abs((max - min) / 2.f) };

	glGenBuffers(1, &v_pos);
	glBindBuffer(GL_ARRAY_BUFFER, v_pos);
	glBufferData(GL_ARRAY_BUFFER, objpos.size() * sizeof(glm::vec3), objpos.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &v_nor);
	glBindBuffer(GL_ARRAY_BUFFER, v_nor);
	glBufferData(GL_ARRAY_BUFFER, objnor.size() * sizeof(glm::vec3), objnor.data(), GL_STATIC_DRAW);

	face_cnt = faceIndex;

	delete[] v_face;
	delete[] vn_face;
	delete[] vt_face;
	delete[] vertex;
	delete[] normal;
	delete[] texture;
}

void GLObj::draw(std::string draw_Mod) {
	if ("solid" == draw_Mod)
		glDrawArrays(GL_TRIANGLES, 0, face_cnt * 3);
	if ("polygon" == draw_Mod)
		glDrawArrays(GL_POLYGON, 0, face_cnt * 3);
	else if ("line" == draw_Mod) {
		for (int i = 0; i < face_cnt * 3; i += 3)
			glDrawArrays(GL_LINE_LOOP, i, 3);
	}
}

void GLObj::draw_prepare(int Location, std::string Location_str) {
	if ("Pos" == Location_str) {
		glBindBuffer(GL_ARRAY_BUFFER, v_pos);
		glVertexAttribPointer(Location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
	else if ("Color" == Location_str) {
		glBindBuffer(GL_ARRAY_BUFFER, v_color);
		glVertexAttribPointer(Location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
	else if ("World" == Location_str) {
		glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(World_mat));
	}
	else if ("Normal" == Location_str) {
		glBindBuffer(GL_ARRAY_BUFFER, v_nor);
		glVertexAttribPointer(Location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
	else if ("Normal_mat" == Location_str) {
		glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(Normal_mat));
	}
}

void GLObj::Update()
{
	World_mat = glm::mat4(1.0);

	World_mat = glm::scale(World_mat, Oscale);

	World_mat = glm::rotate(World_mat, glm::radians(revolve_theta.x), glm::vec3(1.0f, 0.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(revolve_theta.y), glm::vec3(0.0f, 1.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(revolve_theta.z), glm::vec3(0.0f, 0.0f, 1.0f));

	World_mat = glm::translate(World_mat, pos);

	World_mat = glm::rotate(World_mat, glm::radians(rotate_theta.x), glm::vec3(1.0f, 0.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(rotate_theta.y), glm::vec3(0.0f, 1.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(rotate_theta.z), glm::vec3(0.0f, 0.0f, 1.0f));

	World_mat = glm::scale(World_mat, scale);

	World_mat = glm::translate(World_mat, -midpos);
}

void GLObj::Normal_Update()
{
	Normal_mat = glm::mat4(1.0);

	Normal_mat = glm::rotate(Normal_mat, glm::radians(revolve_theta.x), glm::vec3(1.0f, 0.0f, 0.0f));
	Normal_mat = glm::rotate(Normal_mat, glm::radians(revolve_theta.y), glm::vec3(0.0f, 1.0f, 0.0f));
	Normal_mat = glm::rotate(Normal_mat, glm::radians(revolve_theta.z), glm::vec3(0.0f, 0.0f, 1.0f));

	Normal_mat = glm::translate(Normal_mat, pos);

	Normal_mat = glm::rotate(Normal_mat, glm::radians(rotate_theta.x), glm::vec3(1.0f, 0.0f, 0.0f));
	Normal_mat = glm::rotate(Normal_mat, glm::radians(rotate_theta.y), glm::vec3(0.0f, 1.0f, 0.0f));
	Normal_mat = glm::rotate(Normal_mat, glm::radians(rotate_theta.z), glm::vec3(0.0f, 0.0f, 1.0f));

	Normal_mat = glm::translate(Normal_mat, -midpos);
}

void GLObj::Update20()
{
	World_mat = glm::mat4(1.0);

	World_mat = glm::translate(World_mat, pos - Object_mid);
	World_mat = glm::rotate(World_mat, glm::radians(revolve_theta.x), glm::vec3(1.0f, 0.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(revolve_theta.y), glm::vec3(0.0f, 1.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(revolve_theta.z), glm::vec3(0.0f, 0.0f, 1.0f));
	World_mat = glm::translate(World_mat, -(pos - Object_mid));

	World_mat = glm::translate(World_mat, pos);

	World_mat = glm::rotate(World_mat, glm::radians(rotate_theta.x), glm::vec3(1.0f, 0.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(rotate_theta.y), glm::vec3(0.0f, 1.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(rotate_theta.z), glm::vec3(0.0f, 0.0f, 1.0f));

	World_mat = glm::scale(World_mat, scale);

	World_mat = glm::translate(World_mat, -midpos);
}

void GLObj::Update22()
{
	World_mat = glm::mat4(1.0);

	World_mat = glm::scale(World_mat, Oscale);

	World_mat = glm::translate(World_mat, Ani_mid);
	World_mat = glm::rotate(World_mat, glm::radians(revolve_theta.x), glm::vec3(1.0f, 0.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(revolve_theta.y), glm::vec3(0.0f, 1.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(revolve_theta.z), glm::vec3(0.0f, 0.0f, 1.0f));
	World_mat = glm::translate(World_mat, -Ani_mid);

	World_mat = glm::translate(World_mat, pos);

	World_mat = glm::translate(World_mat, -Object_mid);
	World_mat = glm::rotate(World_mat, glm::radians(rotate_theta.x), glm::vec3(1.0f, 0.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(rotate_theta.y), glm::vec3(0.0f, 1.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(rotate_theta.z), glm::vec3(0.0f, 0.0f, 1.0f));
	World_mat = glm::translate(World_mat, Object_mid);

	World_mat = glm::scale(World_mat, scale);

	World_mat = glm::translate(World_mat, -midpos);
}

void GLObj::Update24()
{
	World_mat = glm::mat4(1.0);

	World_mat = glm::scale(World_mat, Oscale);

	World_mat = glm::translate(World_mat, Ani_mid);
	World_mat = glm::rotate(World_mat, glm::radians(revolve_theta.x), glm::vec3(1.0f, 0.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(revolve_theta.y), glm::vec3(0.0f, 1.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(revolve_theta.z), glm::vec3(0.0f, 0.0f, 1.0f));
	World_mat = glm::translate(World_mat, -Ani_mid);

	World_mat = glm::translate(World_mat, pos);

	World_mat = glm::translate(World_mat, -Object_mid);
	World_mat = glm::rotate(World_mat, glm::radians(rotate_theta.x), glm::vec3(1.0f, 0.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(rotate_theta.y), glm::vec3(0.0f, 1.0f, 0.0f));
	World_mat = glm::rotate(World_mat, glm::radians(rotate_theta.z), glm::vec3(0.0f, 0.0f, 1.0f));
	World_mat = glm::translate(World_mat, Object_mid);

	World_mat = glm::scale(World_mat, scale);

	World_mat = glm::translate(World_mat, -midpos);
}