#pragma once

#include "VulkanCore/Core/Layer.hpp"

namespace VkApp
{

	class InstanceManager;

	class BaseImGuiLayer : public Layer
	{
	public:
		BaseImGuiLayer();
		virtual ~BaseImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void Begin();
		void End();

	private:
		friend class InstanceManager;
	};

}