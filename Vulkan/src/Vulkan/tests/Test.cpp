#include "pch.h"
#include "Test.h"

namespace test {

	void TestMenu::OnUpdate(float deltaTime)
	{
	}

	void TestMenu::OnRender()
	{
	}

	void TestMenu::OnImGuiRender()
	{
		//ImGui::NewFrame();

		ImGui::Begin("ILKR", &isOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
		
		for (auto& test : m_Tests)
		{
			if (ImGui::Button(test.first.c_str()))
			{
				delete m_CurrentTest;
				m_CurrentTest = test.second();
			}
		}
		
		ImGui::End();
	}
}