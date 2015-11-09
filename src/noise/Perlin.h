/*
 * Perlin.h
 *
 *  Created on: 9 de nov de 2015
 *      Author: cassiano
 */

#include <GL/glew.h>
#include <GL/glut.h>

#include "../shader/ShaderManager.h"

#ifndef PERLIN_H_
#define PERLIN_H_

namespace noise {

class Perlin {
public:
	Perlin(int);
	virtual ~Perlin();

	void update(float);
	void show();

private:
	int octaves;
	GLint time_location;
	GLint octaves_location;
	GLuint shader;
	GLuint program;

	static std::string TIME_UNIFORM;
	static std::string OCTAVES_UNIFORM;
	static std::string FRAGMENT_FILE;
};

} /* namespace words */

#endif /* PERLIN_H_ */