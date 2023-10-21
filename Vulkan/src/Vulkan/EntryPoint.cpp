#include "pch.h"
#include "Application.h"

int main(int argc, char** argv)
{
	for(int i = 0; i < argc; i++)
	{
		if(std::strcmp("-waitforattach", argv[i]) == 0)
		{
			//VK_CORE_INFO("-waitforattach var");
			while(!IsDebuggerPresent()) {}
		}
	}
	
	auto* app = Application::CreateApplication();

	app->Run();

	delete app;
}