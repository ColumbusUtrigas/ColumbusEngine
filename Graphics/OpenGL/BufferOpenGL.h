#pragma once

#include <Graphics/Buffer.h>

namespace Columbus
{

	class BufferOpenGL : public Buffer
	{
	private:
		uint32 ID;
		uint32 Target;
		uint32 Usage;

		void UpdateUsage();
	public:
		BufferOpenGL();

		virtual void Clear() override;

		virtual bool Create(Buffer::Type InType, const Buffer::Properties& Props) override;
		virtual bool CreateArray(const Buffer::Properties& Props) override;

		virtual bool Load(const void* Data) override;
		virtual bool Load(const Buffer::Properties& Props, const void* BufferData) override;

		uint32 GetID() const { return ID; }         //Get OpenGL-specific ID
		uint32 GetTarget() const { return Target; } //Get OpenGL-specific Target
		uint32 GetUsage() const { return Usage; }   //Get OpenGL-specific Usage

		bool Bind() const;   //OpenGL-specific binding
		bool Unbind() const; //OpenGL-specific unbinding

		virtual ~BufferOpenGL() override;
	};

}

















