#include "DeviceVulkan.h"
#include "AccelerationStructureVulkan.h"
#include <vulkan/vulkan_core.h>

namespace Columbus
{

	static VkAccelerationStructureGeometryTrianglesDataKHR PrepareTrianglesBLAS(const AccelerationStructureDesc& Desc, DeviceVulkan* Device)
	{
		// Setup identity transform matrix
		VkTransformMatrixKHR transformMatrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f
		};
		auto transformBuffer = Device->CreateBuffer({sizeof(transformMatrix), BufferType::AccelerationStructureInput}, &transformMatrix);

		VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
		VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
		VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

		vertexBufferDeviceAddress.deviceAddress = Device->GetBufferDeviceAddress(Desc.Vertices);
		transformBufferDeviceAddress.deviceAddress = Device->GetBufferDeviceAddress(transformBuffer);
		indexBufferDeviceAddress.deviceAddress = Device->GetBufferDeviceAddress(Desc.Indices);

		VkAccelerationStructureGeometryTrianglesDataKHR result;
		result.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		result.pNext = nullptr;
		result.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		result.vertexData = vertexBufferDeviceAddress;
		result.vertexStride = sizeof(Columbus::Vector3);
		result.maxVertex = Desc.VerticesCount;
		result.indexType = VK_INDEX_TYPE_UINT32;
		result.indexData = indexBufferDeviceAddress;
		result.transformData = transformBufferDeviceAddress;

		return result;
	}

	static VkAccelerationStructureGeometryInstancesDataKHR PrepareInstancesTLAS(const AccelerationStructureDesc& Desc, DeviceVulkan* Device)
	{
		std::vector<VkAccelerationStructureInstanceKHR> instances(Desc.Instances.size());

		for (int i = 0; i < Desc.Instances.size(); i++)
		{
			VkTransformMatrixKHR transformMatrix;
			*((Columbus::Vector4*)transformMatrix.matrix[0]) = Desc.Instances[i].Transform.GetRow(0);
			*((Columbus::Vector4*)transformMatrix.matrix[1]) = Desc.Instances[i].Transform.GetRow(1);
			*((Columbus::Vector4*)transformMatrix.matrix[2]) = Desc.Instances[i].Transform.GetRow(2);

			instances[i].transform = transformMatrix;
			instances[i].instanceCustomIndex = i;
			instances[i].mask = 0xFF;
			instances[i].instanceShaderBindingTableRecordOffset = 0;
			instances[i].flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			instances[i].accelerationStructureReference = static_cast<AccelerationStructureVulkan*>(Desc.Instances[i].Blas)->_DeviceAddress; // TODO
		}

		auto instancesBuffer = Device->CreateBuffer({instances.size() * sizeof(VkAccelerationStructureInstanceKHR), BufferType::AccelerationStructureInput}, instances.data());

		VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
		instanceDataDeviceAddress.deviceAddress = Device->GetBufferDeviceAddress(instancesBuffer);

		VkAccelerationStructureGeometryInstancesDataKHR result;
		result.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		result.pNext = nullptr;
		result.arrayOfPointers = VK_FALSE;
		result.data = instanceDataDeviceAddress;

		return result;
	}

	AccelerationStructure* DeviceVulkan::CreateAccelerationStructure(const AccelerationStructureDesc& Desc)
	{
		auto result = new AccelerationStructureVulkan(Desc);

		VkAccelerationStructureTypeKHR type;
		uint32_t numPrimitives = 0;

		// Build info
		VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
		accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

		if (Desc.Type == AccelerationStructureType::BLAS)
		{
			accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			accelerationStructureGeometry.geometry.triangles = PrepareTrianglesBLAS(Desc, this);
			type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			numPrimitives = Desc.IndicesCount / 3;
		} else
		{
			accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
			accelerationStructureGeometry.geometry.instances = PrepareInstancesTLAS(Desc, this);
			type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
			numPrimitives = Desc.Instances.size();
		}

		VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
		accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationStructureBuildGeometryInfo.type = type;
		accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		accelerationStructureBuildGeometryInfo.geometryCount = 1;
		accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

		// Get size info
		VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
		accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		VkFunctions.vkGetAccelerationStructureBuildSizes(_Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &accelerationStructureBuildGeometryInfo, &numPrimitives, &accelerationStructureBuildSizesInfo);

		// Allocate AS storage buffer
		result->_Buffer = CreateBuffer({accelerationStructureBuildSizesInfo.accelerationStructureSize, BufferType::AccelerationStructureStorage}, nullptr);

		// AS create info
		VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
		accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		accelerationStructureCreateInfo.buffer = static_cast<BufferVulkan*>(result->_Buffer)->_Buffer;
		accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
		accelerationStructureCreateInfo.type = type;
		VkFunctions.vkCreateAccelerationStructure(_Device, &accelerationStructureCreateInfo, nullptr, &result->_Handle);

		// Create a small scratch buffer used during build of the acceleration structure
		auto scratchBuffer = CreateBuffer({accelerationStructureBuildSizesInfo.buildScratchSize, BufferType::UAV, true}, nullptr);
		auto scratchBufferDeviceAddress = GetBufferDeviceAddress(scratchBuffer);

		accelerationStructureBuildGeometryInfo.dstAccelerationStructure = result->_Handle;
		accelerationStructureBuildGeometryInfo.scratchData.deviceAddress = scratchBufferDeviceAddress;

		// Build ranges
		VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
		accelerationStructureBuildRangeInfo.primitiveCount = numPrimitives;
		accelerationStructureBuildRangeInfo.primitiveOffset = 0;
		accelerationStructureBuildRangeInfo.firstVertex = 0;
		accelerationStructureBuildRangeInfo.transformOffset = 0;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

		// Execute build
		auto commandBuffer = CreateCommandBufferShared();
		commandBuffer->Begin();
		VkFunctions.vkCmdBuildAccelerationStructures(commandBuffer->_CmdBuf, 1, &accelerationStructureBuildGeometryInfo, accelerationBuildStructureRangeInfos.data());
		commandBuffer->End();
		Submit(commandBuffer.get());
		QueueWaitIdle();

		// Get AS device memory address
		VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
		accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		accelerationDeviceAddressInfo.accelerationStructure = result->_Handle;
		result->_DeviceAddress = VkFunctions.vkGetAccelerationStructureDeviceAddress(_Device, &accelerationDeviceAddressInfo);

		return result;
	}

}
