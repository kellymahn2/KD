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
		ParticleRendererType RendererType = ParticleRendererType::BillboardQuad;
		ParticleSpawnShape SpawnShape = ParticleSpawnShape::Circle;

		float StartAliveTime = 0.0f, DeadTime = 0.0f;
		float StartActiveTime = 0.0f, InactiveTime = 0.0f;



	};


	class ParticleSystem {
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
	};


}
