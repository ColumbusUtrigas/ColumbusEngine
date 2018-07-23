#pragma once

namespace Columbus
{

	class RenderAPIResource
	{
	public:
		RenderAPIResource() = default;

		void Release() const
		{
			delete this;
		}

		virtual ~RenderAPIResource() = default;
	};

	class RenderAPIShader : public RenderAPIResource
	{
	public:
		
	};

}












