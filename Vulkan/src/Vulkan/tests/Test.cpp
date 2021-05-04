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
		/*for (auto& test : m_Tests)
		{
			if (ImGui::Button(test.first))
			{
				delete m_CurrentTest;
				m_CurrentTest = test.second();
			}
		}*/
	}
}