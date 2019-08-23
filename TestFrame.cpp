#include "TestFrame.h"

int main()
{
	FT_Library ft;
	FT_Error fterror = FT_Init_FreeType(&ft);

	if (fterror)
	{
		return fterror;
	}
	
	glfwInit();
	glfwSetErrorCallback(error_callback);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	if (window == NULL)
	{
		return 1;
	}
	
	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	{ //Scope
		std::string a{ "One." }, b{ "Two!" }, c{ "Three" }, d{ "Four" }, e{ "Five0" }, f{ "TARGET ACQUIRED" }, g{ "0123456789" };
		
		glwrap::Sourcer vsc{ L"text_vs.glsl" }, gsc{ L"text_gs.glsl" }, fsc{ L"text_fs.glsl" };
		glwrap::Shader vs{ GL_VERTEX_SHADER }, gs{ GL_GEOMETRY_SHADER }, fs{ GL_FRAGMENT_SHADER };
		vs.compile(vsc.string());
		gs.compile(gsc.string());
		fs.compile(fsc.string());
		
		glwrap::Program prg{};
		prg.attach(vs);
		prg.attach(gs);
		prg.attach(fs);
		prg.link();
		vs.clear();
		gs.clear();
		fs.clear();
		prg.log();
		
		FontManager manager{ft, "Mecha.ttf", 0, 48, 32, 127};
		manager.bakeTextureAtlas();
		//Output texture atlas as image file to inspect later
		//store_image("fontatlas.png", const_cast<BYTE*>(manager.raw()), FIF_PNG, manager.mapWidth(), manager.mapHeight(), manager.mapWidth(), 8);
		
		TextEngine engine{manager, prg, 800, 600, 5};
		unsigned __int64 a_id = engine.addString(a, glm::ivec2{50, 50}, glm::vec3{1.0, 0.2, 0.2});
		unsigned __int64 b_id = engine.addString(b, glm::ivec2{50, 100}, glm::vec3{0.0, 1.0, 0.5});
		unsigned __int64 c_id = engine.addString(c, glm::ivec2{50, 150}, glm::vec3{0.5, 0.0, 0.5});
		unsigned __int64 d_id = engine.addString(d, glm::ivec2{50, 200}, glm::vec3{0.2, 1.0, 0.2});
		engine.removeString(d_id);
		engine.updateString(c_id, "Three?");
		unsigned __int64 e_id = engine.addString(e, glm::ivec2{50, 250}, glm::vec3{0.3, 0.5, 0.1});
		assert(engine.removeString(d_id) == false); //Make sure calls with invalid IDs are ignored
		unsigned __int64 f_id = engine.addString(f, glm::ivec2{ 50, 400 }, glm::vec3{ 1.0, 0.0, 0.0 });

		/*engine.render();
		engine.printVBO();
		engine.printSSBO();*/

		engine.addString(g, glm::ivec2{ 200, 350 }, glm::vec3{ 0.8, 0.8, 0.8 });

		engine.updateOrigin(f_id, glm::ivec2{ 200, 400 });
		engine.updateColor(f_id, glm::vec3{ 0.0, 1.0, 0.0 });
		
		glClearColor(0.0, 0.0, 0.0, 1.0);
		
		while(!glfwWindowShouldClose(window))
		{
			glClear(GL_COLOR_BUFFER_BIT);
			glfwPollEvents();
			
			engine.render();
			glfwSwapBuffers(window);
			
			if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
				glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
	} //FontManager must go out of scope before the library is uninitialized because the destructor calls other library routines
	
	glfwDestroyWindow(window);
	glfwTerminate();
	
	FT_Done_FreeType(ft);
	return 0;
}

BYTE* load_image(const char *path)
{
	FREE_IMAGE_FORMAT fmt = FreeImage_GetFileType(path, 0);

	if (fmt == FIF_UNKNOWN)
	{
		return nullptr;
	}
	else if (FreeImage_FIFSupportsReading(fmt))
	{
		FIBITMAP *bitmap = FreeImage_Load(fmt, path, 0);

		BYTE *buffer = new BYTE[FreeImage_GetWidth(bitmap) * FreeImage_GetHeight(bitmap) * FreeImage_GetBPP(bitmap)];
		FreeImage_ConvertToRawBits(buffer, bitmap, FreeImage_GetPitch(bitmap), FreeImage_GetBPP(bitmap), FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);

		FreeImage_Unload(bitmap);

		return buffer; //Don't forget to release memory after use!
	}

	return nullptr;
}

void unload_image(BYTE *bitmap)
{
	delete[] bitmap;
}

void store_image(const char *path, BYTE *raw, FREE_IMAGE_FORMAT fmt, int width, int height, int pitch, unsigned int bpp)
{
	if (FreeImage_FIFSupportsWriting(fmt))
	{
		FIBITMAP *bitmap = FreeImage_ConvertFromRawBits(raw, width, height, pitch, bpp, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);
		FreeImage_Save(fmt, bitmap, path);
		FreeImage_Unload(bitmap);
	}
}

void error_callback(int error, const char* description)
{
	puts(description);
}