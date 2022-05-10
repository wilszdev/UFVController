#include "DriveLayer.h"
#include "../WalnutApp.h"

void DriveLayer::OnUIRender()
{
	ImGui::Begin("Drive");
	ImGui::SliderInt("##it's about drive, it's about power", &g_ufvState.drive, -1, 1, g_ufvState.drive == 0 ? "Brake" : (g_ufvState.drive == 1 ? "Forward" : "Reverse"));
	ImGui::End();
}