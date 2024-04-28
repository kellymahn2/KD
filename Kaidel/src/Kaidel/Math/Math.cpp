#include "KDpch.h"
#include "Math.h"
#include "Kaidel/Scene/Entity.h"
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
namespace Math {
	
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
	{
		// From glm::decompose in matrix_decompose.inl

		using namespace glm;
		using T = float;

		mat4 LocalMatrix(transform);

		// Normalize the matrix.
		if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
			return false;

		// First, isolate perspective.  This is the messiest.
		if (
			epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
			LocalMatrix[3][3] = static_cast<T>(1);
		}

		// Next take care of translation (easy).
		translation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3];

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		scale.x = length(Row[0]);
		Row[0] = detail::scale(Row[0], static_cast<T>(1));
		scale.y = length(Row[1]);
		Row[1] = detail::scale(Row[1], static_cast<T>(1));
		scale.z = length(Row[2]);
		Row[2] = detail::scale(Row[2], static_cast<T>(1));

		// At this point, the matrix (in rows[]) is orthonormal.
		// Check for a coordinate system flip.  If the determinant
		// is -1, then negate the matrix and the scaling factors.
#if 0
		Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
		if (dot(Row[0], Pdum3) < 0)
		{
			for (length_t i = 0; i < 3; i++)
			{
				scale[i] *= static_cast<T>(-1);
				Row[i] *= static_cast<T>(-1);
			}
		}
#endif

		rotation.y = asin(-Row[0][2]);
		if (cos(rotation.y) != 0) {
			rotation.x = atan2(Row[1][2], Row[2][2]);
			rotation.z = atan2(Row[0][1], Row[0][0]);
		}
		else {
			rotation.x = atan2(-Row[2][0], Row[1][1]);
			rotation.z = 0;
		}


		return true;
	}

	static glm::mat4 Multiply(const glm::mat4& lhs, const glm::mat4& rhs) {
		glm::mat4 ret;

		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				ret[i][j] = glm::dot(lhs[i], rhs[j]);
			}
		}

		return ret;
	}
	static void Print(const glm::mat4& m) {
		for (int i = 0; i < 4; ++i) {
			std::cout<<fmt::format("{},{},{},{}", m[i][0], m[i][1], m[i][2], m[i][3])<<std::endl;
		}
	}

	static glm::mat4 mult(const glm::mat4& a, const glm::mat4& b){
		glm::mat4 res;

		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				res[j][i] = glm::dot({ a[0][i],a[1][i] ,a[2][i] ,a[3][i] }, b[j]);
			}
		}

		return res;
	}
	glm::mat4 Rotate(const glm::vec3& pos, const glm::vec3& rotationVector) {
		using namespace glm;
		glm::mat4 mat2;
		mat2 = glm::translate(glm::mat4(1.0f), pos)* glm::toMat4(glm::quat(rotationVector))/*glm::rotate(glm::mat4(1.0f), rotationVector.z, glm::vec3(0, 0, 1.0f))*glm::rotate(glm::mat4(1.0f), rotationVector.y, glm::vec3(0, 1.0f, 0))*
			glm::rotate(glm::mat4(1.0f), rotationVector.x, glm::vec3(1.0f, 0, 0)) */*glm::translate(glm::mat4(1.0f), -pos);
		//glm::mat4 mat;
		//mat = glm::translate(glm::mat4(1.0f), pos) * glm::rotate(glm::mat4(1.0f), rotationVector.z, glm::vec3(0, 0, 1.0f))*glm::rotate(glm::mat4(1.0f), rotationVector.y, glm::vec3(0, 1.0f, 0))*
		//	glm::rotate(glm::mat4(1.0f), rotationVector.x, glm::vec3(1.0f, 0, 0)) * glm::translate(glm::mat4(1.0f), -pos);
		//Print(mat);
		return mat2;

	}
	void Rotate(Kaidel::Entity other, Kaidel::Entity origin, const glm::vec3& rotationVector)
	{
		using namespace Kaidel;
		auto& otherTC = other.GetComponent<TransformComponent>();
		DecomposeTransform((Rotate(origin.GetComponent<TransformComponent>().Translation, rotationVector)) * otherTC.GetTransform()
			, otherTC.Translation, otherTC.Rotation, otherTC.Scale);

	}


	glm::vec3 Rotate(const glm::mat4& a, const glm::mat4& origin, const glm::vec3& rotationVector)
	{
		return glm::vec3{};
	}

}
