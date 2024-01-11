#pragma once

#include <memory>

#include <glm/glm.hpp>

namespace VkApp
{

	// Note(Jorben): All I want this class to do is setup the rendering API with logging etc
	class API
	{
	public:
		void Init();
		void Destroy();

		static std::shared_ptr<API> Create() { return std::make_shared<API>(); }

	};

}