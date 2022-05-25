#pragma once
#include "GamepadControlsLayer.h"
#include "imgui.h"

void GamepadControlsLayer::OnUIRender()
{
	ImGui::Begin("Gamepad Controls");

	if (ImGui::BeginTable("ctrltbl", 2, 0))
	{
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Button");
			ImGui::Dummy({ 0,5 });
			ImGui::Separator();
			ImGui::Dummy({ 0,5 });

			ImGui::TableNextColumn();
			ImGui::Text("Function");
			ImGui::Dummy({ 0,5 });
			ImGui::Separator();
		}

		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("RT");
			ImGui::Text("LT");
			ImGui::Dummy({ 0,5 });
			ImGui::Separator();
			ImGui::Dummy({ 0,5 });

			ImGui::TableNextColumn();
			ImGui::Text("forward (hold)");
			ImGui::Text("reverse (hold)");
			ImGui::Dummy({ 0,5 });
			ImGui::Separator();
		}

		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("DPAD LEFT");
			ImGui::Text("DPAD RIGHT");
			ImGui::Text("DPAD UP");
			ImGui::Dummy({ 0,5 });
			ImGui::Separator();
			ImGui::Dummy({ 0,5 });

			ImGui::TableNextColumn();
			ImGui::Text("previous preset");
			ImGui::Text("next preset");
			ImGui::Text("apply preset");
			ImGui::Dummy({ 0,5 });
			ImGui::Separator();
		}

		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("LS (Y)");
			ImGui::Text("RS (X)");
			ImGui::Text("RS (Y)");
			ImGui::Dummy({ 0,5 });
			ImGui::Separator();
			ImGui::Dummy({ 0,5 });

			ImGui::TableNextColumn();
			ImGui::Text("adjust motor power");
			ImGui::Text("adjust nozzle pan");
			ImGui::Text("adjust nozzle tilt");
			ImGui::Dummy({ 0,5 });
			ImGui::Separator();
		}

		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("A");
			ImGui::Text("Y");
			ImGui::Dummy({ 0,5 });
			ImGui::Separator();
			ImGui::Dummy({ 0,5 });

			ImGui::TableNextColumn();
			ImGui::Text("pump on (hold)");
			ImGui::Text("log current angle settings");
			ImGui::Dummy({ 0,5 });
			ImGui::Separator();
		}

		ImGui::EndTable();
	}

	ImGui::End();
}
