#pragma once


#include "VulkanBase.h"
#include "VulkanDefinitions.h"

namespace Kaidel {
	namespace Vulkan {

		struct QueueValidateResult {
			bool Validated;
		};


		struct VulkanQueueSpecification {
			std::function<QueueValidateResult(const VkQueueFamilyProperties& queueFamilyProperties,uint32_t familyIndex,VkPhysicalDevice physicalDevice)> Validator;
		};

		struct VulkanQueueGroupSpecification {
			std::unordered_map<std::string, VulkanQueueSpecification> QueueSpecifications;
		};



		struct VulkanQueue {
			VkQueue Queue;
			uint32_t FamilyIndex;
			uint32_t QueueIndex;

			operator VkQueue()const { return Queue; }
		};


		class VulkanQueueGroup {
		public:

			bool Filled()const { return m_Filled; }

			void FillQueueGroup(const VulkanQueueGroupSpecification& specification, const std::vector<VkQueueFamilyProperties> queueFamilyProperties,
				std::unordered_map<uint32_t, uint32_t>& allAcceptedFamilies, VkPhysicalDevice physicalDevice) 
			
			{
				m_Filled = true;
				m_Queues.clear();

				std::unordered_map<uint32_t, bool> acceptedFamilies;

				for (auto& [queueName, spec] : specification.QueueSpecifications) {

					uint32_t foundQueueIndex = -1;
					uint32_t currentQueueIndex = 0;

					for (auto& queueFamily : queueFamilyProperties) {

						if (!QueueFamilyHasSpace(allAcceptedFamilies, currentQueueIndex, queueFamily.queueCount))
							continue;

						QueueValidateResult validatonResult = spec.Validator(queueFamily,currentQueueIndex,physicalDevice);

						if (validatonResult.Validated) {
							acceptedFamilies[currentQueueIndex] = true;
							foundQueueIndex = currentQueueIndex;
							break;
						}
						++currentQueueIndex;
					}

					if (foundQueueIndex == -1) {
						m_Filled = false;
						return;
					}
					VulkanQueue queue{};
					queue.FamilyIndex = foundQueueIndex;
					m_Queues.insert({ queueName,queue });
				}


				for (auto& acceptedFamily : acceptedFamilies) {
					++allAcceptedFamilies[acceptedFamily.first];
				}

				for (auto& [queueName, queue] : m_Queues) {
					queue.QueueIndex = allAcceptedFamilies.at(queue.FamilyIndex) - 1;
				}
			}

			const auto& GetQueues()const { return m_Queues; }


		private:

			bool QueueFamilyHasSpace(const std::unordered_map<uint32_t, uint32_t>& acceptedFamilies, uint32_t queueFamilyIndex, uint32_t maxQueueCount) {
				auto acceptedFamily = acceptedFamilies.find(queueFamilyIndex);
				if (acceptedFamily != acceptedFamilies.end()) {
					return acceptedFamily->second < maxQueueCount;
				}

				return maxQueueCount != 0;
			}

			std::unordered_map<std::string, VulkanQueue> m_Queues;
			bool m_Filled = true;

		};


		class VulkanQueueManager {
		public:

			void FillQueueManager(const std::vector<VulkanQueueGroupSpecification>& queueGroups, VkPhysicalDevice physicalDevice) {
				m_Queues.clear();
				m_Filled = true;

				std::vector<VkQueueFamilyProperties> queueFamilies = QuerySupportedPhysicalDeviceQueueFamilies(physicalDevice);

				std::unordered_map<uint32_t, uint32_t> acceptedFamilies;

				for (auto& queueGroupSpecification : queueGroups) {
					VulkanQueueGroup queueGroup;
					queueGroup.FillQueueGroup(queueGroupSpecification, queueFamilies, acceptedFamilies, physicalDevice);
					if (queueGroup.Filled()) {
						AddQueuesFromGroup(queueGroup);
					}
					else {
						m_Filled = false;
						return;
					}
				}
			}

			bool Filled() const { return m_Filled; }

			std::vector<uint32_t> GetUniqueFamilyIndices()const {
				std::vector<uint32_t> families;

				for (auto& queue : m_Queues) {
					families.push_back(queue.second.FamilyIndex);
				}

				auto uniqueEnd = std::unique(families.begin(), families.end());

				return { families.begin(),uniqueEnd };
			}

			const auto& GetQueues()const { return m_Queues; }
			auto begin() { return m_Queues.begin(); }
			auto end() { return m_Queues.end(); }
			auto begin()const { return m_Queues.begin(); }
			auto end()const { return m_Queues.end(); }

			VulkanQueue& operator[](const std::string& queueName) { return m_Queues.at(queueName); }
			const VulkanQueue& operator[](const std::string& queueName)const { return m_Queues.at(queueName); }

		private:
			void AddQueuesFromGroup(const VulkanQueueGroup& queueGroup) {
				for (auto& queue : queueGroup.GetQueues()) {
					KD_CORE_ASSERT(m_Queues.find(queue.first) == m_Queues.end(), "Multiple queues with the same name cannot exist");
					m_Queues.insert(queue);
				}
			}

			std::vector<VkQueueFamilyProperties> QuerySupportedPhysicalDeviceQueueFamilies(VkPhysicalDevice physicalDevice) {
				uint32_t supportedPhysicalDeviceQueueFamilyCount = 0;
				std::vector<VkQueueFamilyProperties> supportedQueueFamilies;
				vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &supportedPhysicalDeviceQueueFamilyCount, nullptr);
				supportedQueueFamilies.resize(supportedPhysicalDeviceQueueFamilyCount);
				vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &supportedPhysicalDeviceQueueFamilyCount, supportedQueueFamilies.data());
				return supportedQueueFamilies;
			}

			std::unordered_map<std::string, VulkanQueue> m_Queues;
			
			bool m_Filled = false;
		};

	}
}


