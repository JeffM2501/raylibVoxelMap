/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

For a C++ project simply rename the file to .cpp and run premake 

-- Copyright (c) 2020-2024 Jeffery Myers
--
--This software is provided "as-is", without any express or implied warranty. In no event 
--will the authors be held liable for any damages arising from the use of this software.

--Permission is granted to anyone to use this software for any purpose, including commercial 
--applications, and to alter it and redistribute it freely, subject to the following restrictions:

--  1. The origin of this software must not be misrepresented; you must not claim that you 
--  wrote the original software. If you use this software in a product, an acknowledgment 
--  in the product documentation would be appreciated but is not required.
--
--  2. Altered source versions must be plainly marked as such, and must not be misrepresented
--  as being the original software.
--
--  3. This notice may not be removed or altered from any source distribution.

*/

#include "raylib.h"

#include "resource_dir.h"
#include "lighting_system.h"

#include "object_transform.h"
#include "raymath_operators.h"

int main ()
{
	SearchAndSetResourceDir("resources");


	// set up the window
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
	InitWindow(1280, 800, "Voxels");
	SetTargetFPS(500);

	Lights::SetLightingShader(LoadShader("shaders/lighting.vert", "shaders/lighting.frag"));

	Camera3D ViewCamera = { 0 };
	ViewCamera.fovy = 45;
	ObjectTransform CameraTransform(false);
	CameraTransform.SetPosition(0, 2, -10);

	CameraTransform.SetCamera(ViewCamera);


	Material cubeMat = LoadMaterialDefault();
	cubeMat.shader = Lights::GetLightingShader();
	
	Mesh cube = GenMeshCube(2, 2, 2);

	Vector3 lightPos = { 8, 10, -8 };
	Vector3 lightDirection = { -1,-1, 1 };

	float cone = 15;

	auto* light = static_cast<Lights::SpotLight*>(Lights::AddLight(Lights::LightTypes::Spot));
	light->SetPosition(lightPos);
	light->SetDirection(lightDirection);
	light->SetConeAngle(cosf(DEG2RAD * cone));

	float attenuation = 15;
	float falloff = 20;

	light->SetFalloff(falloff);
	light->SetAttenuation(attenuation);

	Vector3 light2Pos = { -3, 3, -3 };

	auto* light2 = Lights::AddLight(Lights::LightTypes::Point);
	light2->SetPosition(light2Pos);
	light2->SetIntensity(PINK);
	light2->SetFalloff(3);
	light2->SetAttenuation(1);

	// game loop
	while (!WindowShouldClose())
	{
		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		{
			constexpr float mouseFactor = 0.1f;
			CameraTransform.RotateY(GetMouseDelta().x * -mouseFactor);
			CameraTransform.RotateH(GetMouseDelta().y * -mouseFactor);
		}

		float speed = 10 * GetFrameTime();
		if (IsKeyDown(KEY_W))
			CameraTransform.MoveD(speed);
		if (IsKeyDown(KEY_S))
			CameraTransform.MoveD(-speed);
		if (IsKeyDown(KEY_A))
			CameraTransform.MoveH(speed);
		if (IsKeyDown(KEY_D))
			CameraTransform.MoveH(-speed);
		if (IsKeyDown(KEY_E))
			CameraTransform.MoveV(speed);
		if (IsKeyDown(KEY_Q))
			CameraTransform.MoveV(-speed);


		float increment = 0.5f;
		if (IsKeyPressed(KEY_UP))
		{
			attenuation += increment;
			light->SetAttenuation(attenuation);
		}
		if (IsKeyPressed(KEY_DOWN))
		{
			attenuation -= increment;
			light->SetAttenuation(attenuation);
		}

		if (IsKeyPressed(KEY_RIGHT))
		{
			falloff += increment;
			light->SetFalloff(falloff);
		}
		if (IsKeyPressed(KEY_LEFT))
		{
			falloff -= increment;
			light->SetFalloff(falloff);
		}

		if (IsKeyPressed(KEY_R))
		{
			cone += 1.0f;
			light->SetConeAngle(cosf(DEG2RAD * cone));
		}
		if (IsKeyPressed(KEY_T))
		{
			cone -= 1.0f;
			light->SetConeAngle(cosf(DEG2RAD * cone));
		}

		// drawing
		BeginDrawing();
		ClearBackground(DARKGRAY);

		CameraTransform.SetCamera(ViewCamera);
		Lights::UpdateLights(ViewCamera);
		BeginMode3D(ViewCamera);

		DrawGrid(10, 1);

		DrawSphere(lightPos, 0.25f, WHITE);
		DrawLine3D(lightPos, lightPos + (lightDirection * 20), YELLOW);

		DrawSphere(light2Pos, 0.25f, PINK);

		DrawMesh(cube, cubeMat, MatrixTranslate(0, 1, 0));

		BeginShaderMode(cubeMat.shader);
		DrawPlane(Vector3{ 0, -0.01f, 0 }, Vector2{ 20,20 }, GRAY);
		EndShaderMode();

		EndMode3D();
		DrawFPS(0, 0);
		
		EndDrawing();
	}

	// cleanup
	CloseWindow();
	return 0;
}