#include "MyImGuiPanel.h"



MyImGuiPanel::MyImGuiPanel()
{
	this->m_avgFPS = 0.0;
	this->m_avgFrameTime = 0.0;
}


MyImGuiPanel::~MyImGuiPanel()
{
}

void MyImGuiPanel::update() {
	// performance information
	const std::string FPS_STR = "FPS: " + std::to_string(this->m_avgFPS);
	ImGui::TextColored(ImVec4(0, 220, 0, 255), FPS_STR.c_str());
	const std::string FT_STR = "Frame: " + std::to_string(this->m_avgFrameTime);
	ImGui::TextColored(ImVec4(0, 220, 0, 255), FT_STR.c_str());
    if (ImGui::Button("Teleport 0"))
        Teleport[0] = true;
    if (ImGui::Button("Teleport 1"))
        Teleport[1] = true;
	if (ImGui::Button("Teleport 2"))
		Teleport[2] = true;
	if (ImGui::Button("toggle magic stone normal map"))
		StoneNormalMap = !StoneNormalMap;
    if (ImGui::Button("mode none"))
        deferredMode = 0;
    if (ImGui::Button("mode world space vertex"))
        deferredMode = 1;
    if (ImGui::Button("mode world space normal"))
        deferredMode = 2;
    if (ImGui::Button("mode ambient/diffuse"))
        deferredMode = 3;
    if (ImGui::Button("mode specular"))
        deferredMode = 4;
}

void MyImGuiPanel::setAvgFPS(const double avgFPS){
	this->m_avgFPS = avgFPS;
}
void MyImGuiPanel::setAvgFrameTime(const double avgFrameTime){
	this->m_avgFrameTime = avgFrameTime;
}