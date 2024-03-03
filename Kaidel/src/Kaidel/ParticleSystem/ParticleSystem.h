#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace Kaidel {

	enum class ParticleStatus {
		None = 0,
		Alive,
		Active,
		Inactive,
		Dead
	};

	enum class ParticleRendererType {
		None,
		BillboardQuad,
		BillboardCircle
	};

	enum class ParticleSpawnShape {
		None,
		Cone,
		Circle,
		Sphere
	};

	enum class ParticleSystemState {
		None,
		Active,
		Inactive
	};


	struct ConeSpawnShape{
		float Radius;
		// Radians
		float Angle;
	};

	struct CircleSpawnShape {
		float Radius;
	};
	struct SphereSpawnShape {
		float Radius;
	};


	union SpawnShapeData {
		ConeSpawnShape Cone;
		CircleSpawnShape Circle;
		SphereSpawnShape Sphere;
	};

	struct Particle {
		glm::vec3 InitialPostition{ 0.0f };
		glm::vec3 InitialVelocity{ 0.0f };
		float AliveTime = 0.0f;
		ParticleStatus Status = ParticleStatus::None;
	};


	struct ParticleSystemSpecification {
		glm::vec3 ParticleAcceleration{ 0.0f,-9.81f,0.0f };
		SpawnShapeData ShapeData;
		uint32_t MaxParticleCount = 0;
		uint32_t ParticleSpawnCountPerTick = 10;
		ParticleRendererType RendererType = ParticleRendererType::BillboardQuad;
		ParticleSpawnShape SpawnShape = ParticleSpawnShape::Circle;

		//Particle System
		float ParticleSpawningStartTime = 0.0f;
		float ParticleSpawingEndTime = 0.0f;
		float ActiveTime = 0.0f;
		float TickTime = 1 / 60.0f;

		//Particles
		float ParticleStartAliveTime = 0.0f, ParticleDeadTime = 0.0f;
		float ParticleStartActiveTime = 0.0f, ParticleInactiveTime = 0.0f;

		ParticleSystemState SystemState = ParticleSystemState::Inactive;
	};


	class ParticleSystem : public IRCCounter<false> {
	public:
		ParticleSystem() {}
		ParticleSystem(const ParticleSystemSpecification& spec);

		uint32_t Generate(uint32_t count);
		void Update(float ts);

		Particle* begin() {
			return m_AliveParticles.data();
		}
		Particle* end() {
			return begin() + m_AliveParticleCount;
		}

		const ParticleSystemSpecification& GetSpecification()const { return m_Specification; }

	private:
		ParticleSystemSpecification m_Specification;
		std::vector<Particle> m_AliveParticles;
		uint32_t m_AliveParticleCount = 0;
		float m_TimeSinceLastTick = 0.0f;
	};


}
