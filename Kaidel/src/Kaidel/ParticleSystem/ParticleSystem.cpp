#include "KDpch.h"
#include "ParticleSystem.h"
namespace Kaidel {

	namespace Utils {
		static Particle GenerateParticle(ParticleSpawnShape spawnShape, SpawnShapeData shapeData) {
			Particle particle;
			
			particle.InitialVelocity = glm::vec3(Random::GenerateFloat(-1.0f, 1.0f), Random::GenerateFloat(-1.0f, 1.0f), Random::GenerateFloat(-1.0f, 1.0f));

			particle.AliveTime = 0.0f;

			particle.Status = ParticleStatus::Alive;


			switch (spawnShape) {
			case ParticleSpawnShape::Cone: {
				float radius = shapeData.Cone.Radius;
				float angle = shapeData.Cone.Angle;
				float randRadius = Random::GenerateFloat(0.0f, radius);
				float x = randRadius * std::sin(angle);
				float z = randRadius * std::cos(angle);
				particle.InitialPostition = glm::vec3(x, 0.0f, z);
				break;
			}
			case ParticleSpawnShape::Circle: {
				float radius = shapeData.Circle.Radius;
				float angle = Random::GenerateFloat(0.0f, 2 * glm::pi<float>());
				float x = radius * std::cos(angle);
				float z = radius * std::sin(angle);
				particle.InitialPostition = glm::vec3(x, 0.0f, z);
				break;
			}
			case ParticleSpawnShape::Sphere: {
				float radius = shapeData.Sphere.Radius;
				float u = Random::GenerateFloat(-1.0f, 1.0f);
				float theta = Random::GenerateFloat(0.0f, 2 * glm::pi<float>());
				float phi = std::acos(u);
				float x = radius * std::sin(phi) * std::cos(theta);
				float y = radius * std::sin(phi) * std::sin(theta);
				float z = radius * std::cos(phi);
				particle.InitialPostition = glm::vec3(x, y, z);
				break;
			}
			}
			return particle;
		}


		static void UpdateParticlePhysics(const ParticleSystemSpecification& spec, Particle& particle,float ts) {
			if (particle.AliveTime + ts > spec.DeadTime) {
				particle.Status = ParticleStatus::Dead;
				return;
			}
			particle.AliveTime += ts;
		}
	}


    ParticleSystem::ParticleSystem(const ParticleSystemSpecification& spec) {
		m_Specification = spec;
		m_AliveParticles.resize(spec.MaxParticleCount);
	}
	uint32_t ParticleSystem::Generate(uint32_t count) {
		uint32_t addedCount = std::min(m_Specification.MaxParticleCount - m_AliveParticleCount,count);

		for (uint32_t i = 0; i < addedCount;++i) {
			uint32_t particleIndex = m_AliveParticleCount++;
			m_AliveParticles[particleIndex] = Utils::GenerateParticle(m_Specification.SpawnShape,m_Specification.ShapeData);
			if (m_Specification.StartAliveTime == m_Specification.StartActiveTime)
				m_AliveParticles[particleIndex].Status = ParticleStatus::Active;
		}
		return addedCount;
	}
	void ParticleSystem::Update(float ts) {
		for (uint32_t i = 0; i < m_AliveParticleCount;++i) {
			Utils::UpdateParticlePhysics(m_Specification, m_AliveParticles[i], ts);
		}

		for (uint32_t i = 0; i < m_AliveParticleCount; ++i) {
			if (m_AliveParticles[i].Status == ParticleStatus::Dead) {
				std::swap(m_AliveParticles[m_AliveParticleCount - 1], m_AliveParticles[i]);
				--m_AliveParticleCount;
			}
		}
		
	}

}
