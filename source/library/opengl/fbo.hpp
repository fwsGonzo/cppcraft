#ifndef FBO_HPP
#define FBO_HPP

#include <string>
#include <vector>

typedef unsigned int GLenum;
typedef int          GLint;
typedef unsigned int GLuint;

namespace library
{
	class Texture;
	
	class FBO
	{
		GLuint fbo;
		
		static GLuint lastFBO;
		
	public:
		FBO();
		
		// creates VBO handle, allowing further usage
		void create();
		// binds this FBO
		void bind();
		void unbind();
		
		// attaches a color target to this FBO, but will only work
		// if the texture isn't being read at the same time as being rendered to!
		void attachColor(GLenum index, Texture& texture);
		void attachColor(GLenum index, GLuint texture);
		void removeColor(GLenum index);
		// resolve a multisampling texture and remove it
		void resolve(GLenum index);
		// attach a depth texture to this FBO
		void attachDepth(Texture& texture);
		void attachDepth(GLuint texture);
		void removeDepth();
		
		// create depth renderbuffer
		void createDepthRBO(int width, int height);
		
		// select which attachments to draw to
		static void drawBuffers(std::vector<int> buffers);
		// back to normal (default = attachment 0)
		static void drawBuffers();
		
		// returns true if the framebuffer is OK, and can be used
		// returns false if the framebuffer is incomplete, and thus cannot be used yet
		static bool isComplete();
		static std::string errorString();
	};
}

#endif
