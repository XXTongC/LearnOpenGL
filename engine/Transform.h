#pragma once

namespace GLengine
{
	struct Vector3
	{
		float x{ 0.0f };
		float y{ 0.0f };
		float z{ 0.0f };
	};

	struct Transform
	{
		Vector3 location{};
		Vector3 rotation{};
		Vector3 scale{ 1.0f, 1.0f, 1.0f };
	};
}
