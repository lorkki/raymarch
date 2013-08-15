/*
 * Raymarching demo thing
 */

#include <SDL.h>
#include <SDL_log.h>
#include <SDL_opengles2.h>

#include <android/log.h>

#include <iostream>


using namespace std;


void log (const string& message)
{
	__android_log_print(ANDROID_LOG_INFO, "HackDemo", "%s", message.c_str());
}


class Demo
{
public:
				Demo	();
	virtual		~Demo	();

	void		init	();

	void		run		();

private:
	SDL_Window*		m_window;
	SDL_GLContext	m_context;
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
	SDL_DisplayMode mode;

	if (SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		log(SDL_GetError());
		return;
	}

	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);

	SDL_GetCurrentDisplayMode(0, &mode);

	m_window = SDL_CreateWindow(
		"Cool Demo",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		mode.w, mode.h,
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

	glViewport(0, 0, mode.w, mode.h);

	glClearColor(1, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	SDL_GL_SwapWindow(m_window);
}


void Demo::run ()
{
	bool running = true;

	if (!m_window || !m_context) return;

	while (running)
	{
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
