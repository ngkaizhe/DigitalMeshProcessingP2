#pragma once
#include"Common.h"

#include<string>
#include<fstream>
#include<sstream>
#include<iostream>

class Shader
{
public:
	unsigned int ID;

	Shader();
	Shader(const char* vertexPath, const char* fragmentPath);
	void use();

	// set uniform value
	void setUniformBool(const std::string &name, const bool value);
	void setUniformInt(const std::string& name, const int value);
	void setUniformFloat(const std::string& name, const float value);
	void setUniform4fv(const std::string& name, const float value[]);
	void setUniform4fv(const std::string& name, const float v1, const float v2, const float v3, const float v4);
	void setUniform3fv(const std::string& name, const float value[]);
	void setUniform3fv(const std::string& name, const glm::vec3& value);
	void setUniform3fv(const std::string& name, const float v1, const float v2, const float v3);
	void setUniformMatrix4fv(const std::string& name, const glm::mat4 trans);

	// set attribute value
	//void setAttribBool(const std::string& name, bool value) const;
	//void setAttribBool(const unsigned int& location, bool value) const;
	//void setAttribInt(const std::string& name, int value) const;
	//void setAttribInt(const unsigned int& location, int value) const;
	//void setAttribFloat(const std::string& name, float value) const;
	//void setAttribFloat(const unsigned int& location, float value) const;
	//void setAttrib4fv(const std::string& name, float value[]) const;
	//void setAttrib4fv(const unsigned int& location, float value[]) const;
};

