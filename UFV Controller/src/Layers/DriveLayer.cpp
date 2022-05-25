#include "DriveLayer.h"
#include "../WalnutApp.h"

void DriveLayer::OnUIRender()
{
	ImGui::Begin("Drive");
	ImGui::Text("Drive state"); ImGui::SameLine(115);
	ImGui::SliderInt("##it's about drive", &g_ufvState.drive, -1, 1, g_ufvState.drive == 0 ? "Brake" : (g_ufvState.drive == 1 ? "Forward" : "Reverse"));
	ImGui::Text("Motor power"); ImGui::SameLine(115);
	ImGui::SliderInt("##it's about power", &g_ufvState.motorPower, 0, 255);
	ImGui::End();
}