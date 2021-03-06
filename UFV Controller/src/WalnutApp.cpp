#include "WalnutApp.h"
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Layers/Layers.h"

ufv_state g_ufvState = { 0, 0, 0, false, 200 };
angle_preset g_anglePresets[NUM_PRESETS] = {};


void ApplyPreset(int index)
{
	if (index < 0 || index > NUM_PRESETS - 1) return;

	g_ufvState.tiltAngle = g_anglePresets[index].tilt;
	g_ufvState.panAngle = g_anglePresets[index].pan;
}

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "UFV Controller";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<InputLayer>();
	app->PushLayer<DriveLayer>();
	app->PushLayer<FluidLayer>();
	app->PushLayer<GamepadControlsLayer>();
	app->PushLayer<CommsLayer>();
	app->PushLayer<LoggingLayer>();
	app->PushLayer<PresetsLayer>();
	app->SetMenubarCallback(
		[app]()
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit"))
				{
					app->Close();
				}
				ImGui::EndMenu();
			}
		});
	return app;
}
