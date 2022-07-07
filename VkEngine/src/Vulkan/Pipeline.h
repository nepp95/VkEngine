#pragma once

#include "Vulkan/Base.h"

class Pipeline
{
public:
	Pipeline();
	~Pipeline();

	void Create();
	void Shutdown();

	VkRenderPass& GetRenderPass() { return m_renderPass; }
	VkPipeline& GetPipeline() { return m_pipeline; }

private:
	VkShaderModule CreateShaderModule(const std::vector<char>& bytecode);

private:
	VkRenderPass m_renderPass;
	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_pipeline;
};