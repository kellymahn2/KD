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

		vec3 Row[3], Pdum3;

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
			KD_CORE_INFO("{},{},{},{}", m[i][0], m[i][1], m[i][2], m[i][3]);
		}
	}
	void Rotate(Kaidel::Entity other, Kaidel::Entity origin, const glm::vec3& rotationVector)
	{
		using namespace Kaidel;
		using namespace glm;
		auto& oTC = origin.GetComponent<TransformComponent>();
		auto x = oTC.Translation.x;
		auto y = oTC.Translation.y;
		auto z = oTC.Translation.z;

		float a = sin(rotationVector.x);
		float b = cos(rotationVector.x);
		float c = sin(rotationVector.y);
		float D = cos(rotationVector.y);
		float e = sin(rotationVector.z);
		float f = cos(rotationVector.z);
		if (epsilonEqual(a, 0.0f, epsilon<float>()))
			a = 0.0f;
		if (epsilonEqual(b, 0.0f, epsilon<float>()))
			b = 0.0f;
		if (epsilonEqual(c, 0.0f, epsilon<float>()))
			c = 0.0f;
		if (epsilonEqual(D, 0.0f, epsilon<float>()))
			D = 0.0f;
		if (epsilonEqual(e, 0.0f, epsilon<float>()))
			e = 0.0f;
		if (epsilonEqual(f, 0.0f, epsilon<float>()))
			f = 0.0f;
		glm::mat4 mat;
		mat[0][0] = D * f;
		mat[0][1] = a * c * f -e * b;
		mat[0][2] = e * a + b * c * f;
		mat[0][3] = -y * (a * c * f - e * b) - z*(e * a + b * c * f) - D * f * x + x;

		mat[1][0] = e * D;
		mat[1][1] = a * c * e + f * b;
		mat[1][2] = b * e * c - f * a  ;
		mat[1][3] = -y * (a * c * e + f * b) - z * (b * c * e - f * a) + e*(-D) * x + y;

		mat[2][0] = -c;
		mat[2][1] = a*D;
		mat[2][2] = b*D;
		mat[2][3] = -a*D*y-b*D*z+c*x*z;

		mat[3][0] = 0;
		mat[3][1] = 0;
		mat[3][2] = 0;
		mat[3][3] = 1;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				if (epsilonEqual(mat[i][j], 0.0f, epsilon<float>()))
				mat[i][j] = 0.0f;
			}
		}
		auto& otherTC = other.GetComponent<TransformComponent>();
		//Row major
		mat = glm::transpose(mat)*otherTC.GetTransform();
		DecomposeTransform(mat,otherTC.Translation,otherTC.Rotation,otherTC.Scale);

	}

}
