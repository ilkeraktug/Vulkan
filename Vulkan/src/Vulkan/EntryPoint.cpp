#pragma once
#include "pch.h"
#include "Application.h"

int main()
{
	auto* app = Application::CreateApplication();

	app->Run();

	delete app;
	return 0;
}