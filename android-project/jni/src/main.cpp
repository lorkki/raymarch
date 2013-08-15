/*
 * Raymarching demo thing
 */

#include <SDL.h>
#include <SDL_log.h>
#include <SDL_opengles2.h>

#include <android/log.h>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include <cmath>

#define CHECK_SDL(...)	checkSDL(__FILE__, __LINE__)
#define CHECK_GL(...)	checkGL(__FILE__, __LINE__)


using namespace std;


void log (const string& message)
{
	__android_log_print(ANDROID_LOG_INFO, "HackDemo", "%s", message.c_str());
}


void checkSDL (const char* file, int line)
{
	const string error = SDL_GetError();

	if (error != "")
	{
		ostringstream ss;
		ss << "SDL error in " << file << ":" << line << ": " << error;
		log(ss.str());
	}
}


void checkGL (const char* file, int line)
{
	GLuint error = glGetError();

	if (error)
	{
		ostringstream ss;
		ss << "GL error (" << error << ")" << " in " << file << ":" << line;
		log(ss.str());
	}
}


string readFile (const string& filename)
{
	SDL_RWops*	file	= SDL_RWFromFile(filename.c_str(), "r");

	if (file)
	{
		vector<char> buf (file->size(file) + 1, 0);

		if (file->read(file, &buf[0], 1, buf.size()-1))
		{
			SDL_RWclose(file);
			return string(&buf[0]);
		}
	}

	log(string(SDL_GetError()));

	return "";
}


class Demo
{
public:
				Demo		();
	virtual		~Demo		();

	void		init		();
	void		run			();

	GLuint		loadShader	(GLenum type, const string& filename);
	GLuint		loadProgram	(const string& vert_file, const string& frag_file);

	void		useProgram	(GLuint program);

	void		drawQuad	();

private:
	SDL_DisplayMode	m_mode;
	SDL_Window*		m_window;
	SDL_GLContext	m_context;

	GLint			m_aPosition;
	GLint			m_aTexCoord;

	GLint			m_uResolution;
	GLint			m_uTime;
	GLint			m_uCamPos;
};


// Constructor and destructor

Demo::Demo ()
	: m_window(0)
{
}


Demo::~Demo ()
{
	log("Destroying window and quitting.");

	if (m_window)
	{
		SDL_DestroyWindow(m_window);
		m_window = 0;
	}

	SDL_Quit();
}


// Initialize SDL and create a GLES 2 context

void Demo::init ()
{
	if (SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		log(SDL_GetError());
		return;
	}

	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);

	SDL_GetCurrentDisplayMode(0, &m_mode);

	m_window = SDL_CreateWindow(
		"Cool Demo",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		m_mode.w, m_mode.h,
		SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL
		);

	if (!m_window)
	{
		log("No window: " + string(SDL_GetError()));
		return;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES); 
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2); 
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	m_context = SDL_GL_CreateContext(m_window);

	if (!m_context)
	{
		log("No context: " + string(SDL_GetError()));
		return;
	}

	SDL_GL_SetSwapInterval(1);

	glViewport(0, 0, m_mode.w, m_mode.h);

	glClearColor(1, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	SDL_GL_SwapWindow(m_window);

	CHECK_GL();
}


GLuint Demo::loadShader (GLenum type, const string& filename)
{
	GLuint shader 	= glCreateShader(type);

	if (!shader)
	{
		log("Couldn't create shader.");
		return 0;
	}

	{
		string 			source		= readFile(filename);
		const char*		srcList[]	= { source.c_str(), 0 };
	
		glShaderSource(shader, 1, srcList, 0);
		glCompileShader(shader);
	}

	CHECK_GL();

	{
		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

		CHECK_GL();

		if (!compiled)
		{
			GLint infoLen;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

			CHECK_GL();

			if (infoLen > 1)
			{
				vector<char> logBuf (infoLen+1, 0);
				glGetShaderInfoLog(shader, infoLen, 0, &logBuf[0]);

				log("Error compiling shader: " + string(&logBuf[0]));
			}

			glDeleteShader(shader);
			return 0;
		}
	}

	return shader;
}


GLuint Demo::loadProgram (const string& vertFile, const string& fragFile)
{
	GLuint	vertShader	= loadShader(GL_VERTEX_SHADER, vertFile);
	GLuint	fragShader	= loadShader(GL_FRAGMENT_SHADER, fragFile);

	GLuint	program		= glCreateProgram();
	CHECK_GL();

	if (!program)
		return 0;

	glAttachShader(program, vertShader);
	CHECK_GL();

	glAttachShader(program, fragShader);
	CHECK_GL();

	glBindAttribLocation(program, 0, "a_position");
	glBindAttribLocation(program, 1, "a_texcoord");
	CHECK_GL();

	glLinkProgram(program);

	{
		GLint linked;
		glGetProgramiv(program, GL_LINK_STATUS, &linked);

		CHECK_GL();

		if (!linked)
		{
			GLint infoLen;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);

			CHECK_GL();

			if (infoLen > 1)
			{
				vector<char> logBuf (infoLen+1, 0);
				glGetShaderInfoLog(program, infoLen, 0, &logBuf[0]);

				log("Error linking program: " + string(&logBuf[0]));
			}

			glDeleteProgram(program);
			return 0;
		}
	}

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	CHECK_GL();
	return program;
}


void Demo::useProgram (GLuint program)
{
	glUseProgram(program);
	CHECK_GL();

	m_uResolution 	= glGetUniformLocation(program, "resolution");
	m_uTime			= glGetUniformLocation(program, "time");
	m_uCamPos		= glGetUniformLocation(program, "cam_pos");
	CHECK_GL();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	CHECK_GL();
}


void Demo::drawQuad ()
{
	const GLfloat position[] =
	{
		-1.0, -1.0, 0.0, 0.0,
		 1.0, -1.0, 0.0, 0.0,
		-1.0,  1.0, 0.0, 0.0,
		 1.0,  1.0, 0.0, 0.0
	};

	const GLfloat texcoord[] =
	{
		0.0, 0.0,
		1.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
	};

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, position);
	CHECK_GL();
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texcoord);
	CHECK_GL();

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	CHECK_GL();
}


void Demo::run ()
{
	GLuint 	program;
	Uint32	startTime;
	bool 	running 	= true;

	if (!m_window || !m_context) return;

	program		= loadProgram("default.vert", "default.frag");

	if (!program) return;

	startTime	= SDL_GetTicks();

	useProgram(program);
	CHECK_GL();

	glUniform2f(m_uResolution, m_mode.w, m_mode.h);
	CHECK_GL();

	while (running)
	{
		float currentTime = (SDL_GetTicks() - startTime) / 1000.0;

		glUniform1f(m_uTime, currentTime);
		CHECK_GL();

		glUniform3f(m_uCamPos, -sin(currentTime)*8.0, 4, cos(currentTime)*8.0);
		CHECK_GL();

		drawQuad();
		CHECK_GL();

		SDL_GL_SwapWindow(m_window);

		SDL_Event e;
		while (SDL_PollEvent(&e))
			if (e.type == SDL_QUIT)
				running = false;
	}
}


int main (int argc, char** argv)
{
	Demo demo;

	demo.init();
	demo.run();

	return 0;
}
