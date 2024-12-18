#include "KDpch.h"
#include "glm/glm.hpp"

struct Basis;
struct Vector2;

typedef float real_t;

using Vector3 = glm::vec3;
template<typename T>
using Vector = std::vector<T>;

#define MAX(a,b) a > b ? a : b
#define MIN(a,b) a > b ? b : a
#define cofac(row1, col1, row2, col2) \
	(rows[row1][col1] * rows[row2][col2] - rows[row1][col2] * rows[row2][col1])
#define CMP_EPSILON 0.00001

struct Plane {
	glm::vec3 normal = glm::vec3(0.0f);
	real_t d = 0;
//
//	void set_normal(const glm::vec3& p_normal);
//	glm::vec3 get_normal() const { return normal; };
//
	void normalize() {
		real_t l = glm::length(normal);
		if (l == 0) {
			*this = Plane(0, 0, 0, 0);
			return;
		}
		normal /= l;
		d /= l;
	}

//	Plane normalized() const;
//
//	/* Plane-Point operations */
//
//	glm::vec3 get_center() const { return normal * d; }
//	glm::vec3 get_any_perpendicular_normal() const;
//
//	bool is_point_over(const glm::vec3& p_point) const; ///< Point is over plane
//	real_t distance_to(const glm::vec3& p_point) const;
//	bool has_point(const glm::vec3& p_point, real_t p_tolerance = CMP_EPSILON) const;
//
//	/* intersections */
//

	static bool is_zero_approx(float s) {
		return abs(s) < (float)CMP_EPSILON;
	}

	bool intersect_3(const Plane& p_plane1, const Plane& p_plane2, glm::vec3* r_result = nullptr) const {
		const Plane& p_plane0 = *this;
		Vector3 normal0 = p_plane0.normal;
		Vector3 normal1 = p_plane1.normal;
		Vector3 normal2 = p_plane2.normal;

		real_t denom = glm::dot(glm::cross(normal0, normal1),normal2);

		if (is_zero_approx(denom)) {
			return false;
		}

		if (r_result) {
			*r_result = ((glm::cross(normal1, normal2) * p_plane0.d) +
				(glm::cross(normal2, normal0) * p_plane1.d) +
				(glm::cross(normal0, normal1) * p_plane2.d)) /
				denom;
		}

		return true;
	}
//	bool intersects_ray(const glm::vec3& p_from, const glm::vec3& p_dir, glm::vec3* p_intersection) const;
//	bool intersects_segment(const glm::vec3& p_begin, const glm::vec3& p_end, glm::vec3* p_intersection) const;
//
//	// For Variant bindings.
//	Variant intersect_3_bind(const Plane& p_plane1, const Plane& p_plane2) const;
//	Variant intersects_ray_bind(const glm::vec3& p_from, const glm::vec3& p_dir) const;
//	Variant intersects_segment_bind(const glm::vec3& p_begin, const glm::vec3& p_end) const;
//
//	glm::vec3 project(const glm::vec3& p_point) const {
//		return p_point - normal * distance_to(p_point);
//	}
//
//	/* misc */
//
//	Plane operator-() const { return Plane(-normal, -d); }
//	bool is_equal_approx(const Plane& p_plane) const;
//	bool is_equal_approx_any_side(const Plane& p_plane) const;
//	bool is_finite() const;
//
//	bool operator==(const Plane& p_plane) const;
//	bool operator!=(const Plane& p_plane) const;
//	operator String() const;
//
	Plane() {}
	Plane(real_t p_a, real_t p_b, real_t p_c, real_t p_d) :
		normal(p_a, p_b, p_c),
		d(p_d) {}
	
	Plane(const glm::vec3& p_normal, real_t p_d = 0.0) :
		normal(p_normal),
		d(p_d) {
	}
//	Plane(const glm::vec3& p_normal, const glm::vec3& p_point);
//	Plane(const glm::vec3& p_point1, const glm::vec3& p_point2, const glm::vec3& p_point3, ClockDirection p_dir = CLOCKWISE);
};


//bool Plane::is_point_over(const glm::vec3& p_point) const {
//	return (normal.dot(p_point) > d);
//}
//
//real_t Plane::distance_to(const glm::vec3& p_point) const {
//	return (normal.dot(p_point) - d);
//}
//
//bool Plane::has_point(const glm::vec3& p_point, real_t p_tolerance) const {
//	real_t dist = normal.dot(p_point) - d;
//	dist = ABS(dist);
//	return (dist <= p_tolerance);
//}
//
//
//Plane::Plane(const glm::vec3& p_normal, const glm::vec3& p_point) :
//	normal(p_normal),
//	d(p_normal.dot(p_point)) {
//}
//
//Plane::Plane(const glm::vec3& p_point1, const glm::vec3& p_point2, const glm::vec3& p_point3, ClockDirection p_dir) {
//	if (p_dir == CLOCKWISE) {
//		normal = (p_point1 - p_point3).cross(p_point1 - p_point2);
//	}
//	else {
//		normal = (p_point1 - p_point2).cross(p_point1 - p_point3);
//	}
//
//	normal.normalize();
//	d = normal.dot(p_point1);
//}
//
//bool Plane::operator==(const Plane& p_plane) const {
//	return normal == p_plane.normal && d == p_plane.d;
//}
//
//bool Plane::operator!=(const Plane& p_plane) const {
//	return normal != p_plane.normal || d != p_plane.d;
//}
//
//


struct Basis {
	glm::vec3 rows[3] = {
		glm::vec3(1, 0, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 0, 1)
	};
//
//	const glm::vec3& operator[](int p_axis) const {
//		return rows[p_axis];
//	}
//	glm::vec3& operator[](int p_axis) {
//		return rows[p_axis];
//	}
//
	void invert() {
		real_t co[3] = {
			cofac(1, 1, 2, 2), cofac(1, 2, 2, 0), cofac(1, 0, 2, 1)
		};
		real_t det = rows[0][0] * co[0] +
			rows[0][1] * co[1] +
			rows[0][2] * co[2];
		
		real_t s = 1.0f / det;

		set(co[0] * s, cofac(0, 2, 2, 1) * s, cofac(0, 1, 1, 2) * s,
			co[1] * s, cofac(0, 0, 2, 2) * s, cofac(0, 2, 1, 0) * s,
			co[2] * s, cofac(0, 1, 2, 0) * s, cofac(0, 0, 1, 1) * s);
	}
	void transpose() {
		std::swap(rows[0][1], rows[1][0]);
		std::swap(rows[0][2], rows[2][0]);
		std::swap(rows[1][2], rows[2][1]);
	}

	Basis inverse() const {
		Basis inv = *this;
		inv.invert();
		return inv;
	}
//	Basis transposed() const;
//
//	real_t determinant() const;
//
//	void rotate(const glm::vec3& p_axis, real_t p_angle);
//	Basis rotated(const glm::vec3& p_axis, real_t p_angle) const;
//
//	void rotate_local(const glm::vec3& p_axis, real_t p_angle);
//	Basis rotated_local(const glm::vec3& p_axis, real_t p_angle) const;
//
//	void rotate(const glm::vec3& p_euler);
//	Basis rotated(const glm::vec3& p_euler) const;
//
//	//void rotate(const Quaternion& p_quaternion);
//	//Basis rotated(const Quaternion& p_quaternion) const;
//
//	glm::vec3 get_euler_normalized(EulerOrder p_order = EulerOrder::YXZ) const;
//	void get_rotation_axis_angle(glm::vec3& p_axis, real_t& p_angle) const;
//	void get_rotation_axis_angle_local(glm::vec3& p_axis, real_t& p_angle) const;
//	Quaternion get_rotation_quaternion() const;
//
//	void rotate_to_align(glm::vec3 p_start_direction, glm::vec3 p_end_direction);
//
//	glm::vec3 rotref_posscale_decomposition(Basis& rotref) const;
//
//	glm::vec3 get_euler(EulerOrder p_order = EulerOrder::YXZ) const;
//	void set_euler(const glm::vec3& p_euler, EulerOrder p_order = EulerOrder::YXZ);
//	static Basis from_euler(const glm::vec3& p_euler, EulerOrder p_order = EulerOrder::YXZ) {
//		Basis b;
//		b.set_euler(p_euler, p_order);
//		return b;
//	}
//
//	Quaternion get_quaternion() const;
//	void set_quaternion(const Quaternion& p_quaternion);
//
//	void get_axis_angle(glm::vec3& r_axis, real_t& r_angle) const;
//	void set_axis_angle(const glm::vec3& p_axis, real_t p_angle);
//
//	void scale(const glm::vec3& p_scale);
//	Basis scaled(const glm::vec3& p_scale) const;
//
//	void scale_local(const glm::vec3& p_scale);
//	Basis scaled_local(const glm::vec3& p_scale) const;
//
//	void scale_orthogonal(const glm::vec3& p_scale);
//	Basis scaled_orthogonal(const glm::vec3& p_scale) const;
//	float get_uniform_scale() const;
//
//	glm::vec3 get_scale() const;
//	glm::vec3 get_scale_abs() const;
//	glm::vec3 get_scale_local() const;
//
//	void set_axis_angle_scale(const glm::vec3& p_axis, real_t p_angle, const glm::vec3& p_scale);
//	void set_euler_scale(const glm::vec3& p_euler, const glm::vec3& p_scale, EulerOrder p_order = EulerOrder::YXZ);
//	void set_quaternion_scale(const Quaternion& p_quaternion, const glm::vec3& p_scale);
//
//	// transposed dot products
//	real_t tdotx(const glm::vec3& p_v) const {
//		return rows[0][0] * p_v[0] + rows[1][0] * p_v[1] + rows[2][0] * p_v[2];
//	}
//	real_t tdoty(const glm::vec3& p_v) const {
//		return rows[0][1] * p_v[0] + rows[1][1] * p_v[1] + rows[2][1] * p_v[2];
//	}
//	real_t tdotz(const glm::vec3& p_v) const {
//		return rows[0][2] * p_v[0] + rows[1][2] * p_v[1] + rows[2][2] * p_v[2];
//	}
//
//	bool is_equal_approx(const Basis& p_basis) const;
//	bool is_finite() const;
//
//	bool operator==(const Basis& p_matrix) const;
//	bool operator!=(const Basis& p_matrix) const;
//
	glm::vec3 xform(const glm::vec3& p_vector) const {
		return Vector3(
			glm::dot(rows[0],p_vector),
			glm::dot(rows[1],p_vector),
			glm::dot(rows[2],p_vector));
	}

	//glm::vec3 xform_inv(const glm::vec3& p_vector) const;
//	void operator*=(const Basis& p_matrix);
//	Basis operator*(const Basis& p_matrix) const;
//	void operator+=(const Basis& p_matrix);
//	Basis operator+(const Basis& p_matrix) const;
//	void operator-=(const Basis& p_matrix);
//	Basis operator-(const Basis& p_matrix) const;
//	void operator*=(real_t p_val);
//	Basis operator*(real_t p_val) const;
//	void operator/=(real_t p_val);
//	Basis operator/(real_t p_val) const;
//
//	bool is_orthogonal() const;
//	bool is_orthonormal() const;
//	bool is_conformal() const;
//	bool is_diagonal() const;
//	bool is_rotation() const;
//
//	Basis lerp(const Basis& p_to, real_t p_weight) const;
//	Basis slerp(const Basis& p_to, real_t p_weight) const;
//	void rotate_sh(real_t* p_values);
//
//	operator String() const;
//
//	/* create / set */
//
	void set(real_t p_xx, real_t p_xy, real_t p_xz, real_t p_yx, real_t p_yy, real_t p_yz, real_t p_zx, real_t p_zy, real_t p_zz) {
		rows[0][0] = p_xx;
		rows[0][1] = p_xy;
		rows[0][2] = p_xz;
		rows[1][0] = p_yx;
		rows[1][1] = p_yy;
		rows[1][2] = p_yz;
		rows[2][0] = p_zx;
		rows[2][1] = p_zy;
		rows[2][2] = p_zz;
	}
	void set_columns(const glm::vec3& p_x, const glm::vec3& p_y, const glm::vec3& p_z) {
		set_column(0, p_x);
		set_column(1, p_y);
		set_column(2, p_z);
	}
//
	glm::vec3 get_column(int p_index) const {
		// Get actual basis axis column (we store transposed as rows for performance).
		return glm::vec3(rows[0][p_index], rows[1][p_index], rows[2][p_index]);
	}
//
	void set_column(int p_index, const glm::vec3& p_value) {
		// Set actual basis axis column (we store transposed as rows for performance).
		rows[0][p_index] = p_value.x;
		rows[1][p_index] = p_value.y;
		rows[2][p_index] = p_value.z;
	}
//
//	glm::vec3 get_main_diagonal() const {
//		return glm::vec3(rows[0][0], rows[1][1], rows[2][2]);
//	}
//
//	void set_zero() {
//		rows[0].zero();
//		rows[1].zero();
//		rows[2].zero();
//	}
//
//	Basis transpose_xform(const Basis& p_m) const {
//		return Basis(
//			rows[0].x * p_m[0].x + rows[1].x * p_m[1].x + rows[2].x * p_m[2].x,
//			rows[0].x * p_m[0].y + rows[1].x * p_m[1].y + rows[2].x * p_m[2].y,
//			rows[0].x * p_m[0].z + rows[1].x * p_m[1].z + rows[2].x * p_m[2].z,
//			rows[0].y * p_m[0].x + rows[1].y * p_m[1].x + rows[2].y * p_m[2].x,
//			rows[0].y * p_m[0].y + rows[1].y * p_m[1].y + rows[2].y * p_m[2].y,
//			rows[0].y * p_m[0].z + rows[1].y * p_m[1].z + rows[2].y * p_m[2].z,
//			rows[0].z * p_m[0].x + rows[1].z * p_m[1].x + rows[2].z * p_m[2].x,
//			rows[0].z * p_m[0].y + rows[1].z * p_m[1].y + rows[2].z * p_m[2].y,
//			rows[0].z * p_m[0].z + rows[1].z * p_m[1].z + rows[2].z * p_m[2].z);
//	}
//	Basis(real_t p_xx, real_t p_xy, real_t p_xz, real_t p_yx, real_t p_yy, real_t p_yz, real_t p_zx, real_t p_zy, real_t p_zz) {
//		set(p_xx, p_xy, p_xz, p_yx, p_yy, p_yz, p_zx, p_zy, p_zz);
//	}
//
	void orthonormalize() {
		// Gram-Schmidt Process

		Vector3 x = get_column(0);
		Vector3 y = get_column(1);
		Vector3 z = get_column(2);

		x = glm::normalize(x);
		y = (y - x * glm::dot(x,y));
		y = glm::normalize(y);
		z = (z - x * glm::dot(x,z) - y * glm::dot(y,z));
		z = glm::normalize(z);

		set_column(0, x);
		set_column(1, y);
		set_column(2, z);
	}
//	Basis orthonormalized() const;
//
//	void orthogonalize();
//	Basis orthogonalized() const;
//
//#ifdef MATH_CHECKS
//	bool is_symmetric() const;
//#endif
//	Basis diagonalize();
//
//	//operator Quaternion() const { return get_quaternion(); }
//
//	static Basis looking_at(const glm::vec3& p_target, const glm::vec3& p_up = glm::vec3(0, 1, 0), bool p_use_model_front = false);
//
//	//Basis(const Quaternion& p_quaternion) { set_quaternion(p_quaternion); };
//	//Basis(const Quaternion& p_quaternion, const glm::vec3& p_scale) { set_quaternion_scale(p_quaternion, p_scale); }
//
//	Basis(const glm::vec3& p_axis, real_t p_angle) { set_axis_angle(p_axis, p_angle); }
//	Basis(const glm::vec3& p_axis, real_t p_angle, const glm::vec3& p_scale) { set_axis_angle_scale(p_axis, p_angle, p_scale); }
//	static Basis from_scale(const glm::vec3& p_scale);
//
//	Basis(const glm::vec3& p_x_axis, const glm::vec3& p_y_axis, const glm::vec3& p_z_axis) {
//		set_columns(p_x_axis, p_y_axis, p_z_axis);
//	}
//
//	Basis() {}
//
//private:
//	// Helper method.
//	void _set_diagonal(const glm::vec3& p_diag);
};


//void Basis::operator*=(const Basis& p_matrix) {
//	set(
//		p_matrix.tdotx(rows[0]), p_matrix.tdoty(rows[0]), p_matrix.tdotz(rows[0]),
//		p_matrix.tdotx(rows[1]), p_matrix.tdoty(rows[1]), p_matrix.tdotz(rows[1]),
//		p_matrix.tdotx(rows[2]), p_matrix.tdoty(rows[2]), p_matrix.tdotz(rows[2]));
//}
//
//Basis Basis::operator*(const Basis& p_matrix) const {
//	return Basis(
//		p_matrix.tdotx(rows[0]), p_matrix.tdoty(rows[0]), p_matrix.tdotz(rows[0]),
//		p_matrix.tdotx(rows[1]), p_matrix.tdoty(rows[1]), p_matrix.tdotz(rows[1]),
//		p_matrix.tdotx(rows[2]), p_matrix.tdoty(rows[2]), p_matrix.tdotz(rows[2]));
//}
//
//void Basis::operator+=(const Basis& p_matrix) {
//	rows[0] += p_matrix.rows[0];
//	rows[1] += p_matrix.rows[1];
//	rows[2] += p_matrix.rows[2];
//}
//
//Basis Basis::operator+(const Basis& p_matrix) const {
//	Basis ret(*this);
//	ret += p_matrix;
//	return ret;
//}
//
//void Basis::operator-=(const Basis& p_matrix) {
//	rows[0] -= p_matrix.rows[0];
//	rows[1] -= p_matrix.rows[1];
//	rows[2] -= p_matrix.rows[2];
//}
//
//Basis Basis::operator-(const Basis& p_matrix) const {
//	Basis ret(*this);
//	ret -= p_matrix;
//	return ret;
//}
//
//void Basis::operator*=(real_t p_val) {
//	rows[0] *= p_val;
//	rows[1] *= p_val;
//	rows[2] *= p_val;
//}
//
//Basis Basis::operator*(real_t p_val) const {
//	Basis ret(*this);
//	ret *= p_val;
//	return ret;
//}
//
//void Basis::operator/=(real_t p_val) {
//	rows[0] /= p_val;
//	rows[1] /= p_val;
//	rows[2] /= p_val;
//}
//
//Basis Basis::operator/(real_t p_val) const {
//	Basis ret(*this);
//	ret /= p_val;
//	return ret;
//}
//
//glm::vec3 Basis::xform(const glm::vec3& p_vector) const {
//	return glm::vec3(
//		rows[0].dot(p_vector),
//		rows[1].dot(p_vector),
//		rows[2].dot(p_vector));
//}
//
//glm::vec3 Basis::xform_inv(const glm::vec3& p_vector) const {
//	return glm::vec3(
//		(rows[0][0] * p_vector.x) + (rows[1][0] * p_vector.y) + (rows[2][0] * p_vector.z),
//		(rows[0][1] * p_vector.x) + (rows[1][1] * p_vector.y) + (rows[2][1] * p_vector.z),
//		(rows[0][2] * p_vector.x) + (rows[1][2] * p_vector.y) + (rows[2][2] * p_vector.z));
//}
//
//real_t Basis::determinant() const {
//	return rows[0][0] * (rows[1][1] * rows[2][2] - rows[2][1] * rows[1][2]) -
//		rows[1][0] * (rows[0][1] * rows[2][2] - rows[2][1] * rows[0][2]) +
//		rows[2][0] * (rows[0][1] * rows[1][2] - rows[1][1] * rows[0][2]);
//}
//


struct Transform3D {
	Basis basis;
	glm::vec3 origin;
//
//	void invert();
//	Transform3D inverse() const;
//
//	void affine_invert();
//	Transform3D affine_inverse() const;
//
//	Transform3D rotated(const glm::vec3& p_axis, real_t p_angle) const;
//	Transform3D rotated_local(const glm::vec3& p_axis, real_t p_angle) const;
//
//	void rotate(const glm::vec3& p_axis, real_t p_angle);
//	void rotate_basis(const glm::vec3& p_axis, real_t p_angle);
//
//	void set_look_at(const glm::vec3& p_eye, const glm::vec3& p_target, const glm::vec3& p_up = glm::vec3(0, 1, 0), bool p_use_model_front = false);
//	Transform3D looking_at(const glm::vec3& p_target, const glm::vec3& p_up = glm::vec3(0, 1, 0), bool p_use_model_front = false) const;
//
//	void scale(const glm::vec3& p_scale);
//	Transform3D scaled(const glm::vec3& p_scale) const;
//	Transform3D scaled_local(const glm::vec3& p_scale) const;
//	void scale_basis(const glm::vec3& p_scale);
//	void translate_local(real_t p_tx, real_t p_ty, real_t p_tz);
//	void translate_local(const glm::vec3& p_translation);
//	Transform3D translated(const glm::vec3& p_translation) const;
//	Transform3D translated_local(const glm::vec3& p_translation) const;
//
//	const Basis& get_basis() const { return basis; }
//	void set_basis(const Basis& p_basis) { basis = p_basis; }
//
//	const glm::vec3& get_origin() const { return origin; }
//	void set_origin(const glm::vec3& p_origin) { origin = p_origin; }
//
	void orthonormalize() {
		basis.orthonormalize();
	}
//	Transform3D orthonormalized() const;
//	void orthogonalize();
//	Transform3D orthogonalized() const;
//	bool is_equal_approx(const Transform3D& p_transform) const;
//	bool is_finite() const;
//
//	bool operator==(const Transform3D& p_transform) const;
//	bool operator!=(const Transform3D& p_transform) const;
//
//	glm::vec3 xform(const glm::vec3& p_vector) const;
//	AABB xform(const AABB& p_aabb) const;
//	Vector<glm::vec3> xform(const Vector<glm::vec3>& p_array) const;
//
//	// NOTE: These are UNSAFE with non-uniform scaling, and will produce incorrect results.
//	// They use the transpose.
//	// For safe inverse transforms, xform by the affine_inverse.
//	glm::vec3 xform_inv(const glm::vec3& p_vector) const;
//	AABB xform_inv(const AABB& p_aabb) const;
//	Vector<glm::vec3> xform_inv(const Vector<glm::vec3>& p_array) const;
//
//	// Safe with non-uniform scaling (uses affine_inverse).
	Plane xform(const Plane& p_plane) const {
		Basis b = basis.inverse();
		b.transpose();
		return xform_fast(p_plane, b);
	}

	Vector3 xform(const Vector3& p_vector) const {
		return Vector3(
			glm::dot(basis.rows[0],p_vector) + origin.x,
			glm::dot(basis.rows[1],p_vector) + origin.y,
			glm::dot(basis.rows[2],p_vector) + origin.z);
	}

//	Plane xform_inv(const Plane& p_plane) const;
//
//	// These fast versions use precomputed affine inverse, and should be used in bottleneck areas where
//	// multiple planes are to be transformed.
	Plane xform_fast(const Plane& p_plane, const Basis& p_basis_inverse_transpose) const {
		// Transform a single point on the plane.
		Vector3 point = p_plane.normal * p_plane.d;
		point = xform(point);

		// Use inverse transpose for correct normals with non-uniform scaling.
		Vector3 normal = p_basis_inverse_transpose.xform(p_plane.normal);
		normal = glm::normalize(normal);

		real_t d = glm::dot(normal,point);
		return Plane(normal, d);
	}
//	static Plane xform_inv_fast(const Plane& p_plane, const Transform3D& p_inverse, const Basis& p_basis_transpose);
//
//	void operator*=(const Transform3D& p_transform);
//	Transform3D operator*(const Transform3D& p_transform) const;
//	void operator*=(real_t p_val);
//	Transform3D operator*(real_t p_val) const;
//	void operator/=(real_t p_val);
//	Transform3D operator/(real_t p_val) const;
//
//	Transform3D interpolate_with(const Transform3D& p_transform, real_t p_c) const;
//
//	Transform3D inverse_xform(const Transform3D& t) const {
//		glm::vec3 v = t.origin - origin;
//		return Transform3D(basis.transpose_xform(t.basis),
//			basis.xform(v));
//	}
//
//	void set(real_t p_xx, real_t p_xy, real_t p_xz, real_t p_yx, real_t p_yy, real_t p_yz, real_t p_zx, real_t p_zy, real_t p_zz, real_t p_tx, real_t p_ty, real_t p_tz) {
//		basis.set(p_xx, p_xy, p_xz, p_yx, p_yy, p_yz, p_zx, p_zy, p_zz);
//		origin.x = p_tx;
//		origin.y = p_ty;
//		origin.z = p_tz;
//	}
//
//	operator String() const;
//
//	Transform3D() {}
//	Transform3D(const Basis& p_basis, const glm::vec3& p_origin = glm::vec3());
//	Transform3D(const glm::vec3& p_x, const glm::vec3& p_y, const glm::vec3& p_z, const glm::vec3& p_origin);
//	Transform3D(real_t p_xx, real_t p_xy, real_t p_xz, real_t p_yx, real_t p_yy, real_t p_yz, real_t p_zx, real_t p_zy, real_t p_zz, real_t p_ox, real_t p_oy, real_t p_oz);
};

//
//glm::vec3 Transform3D::xform(const glm::vec3& p_vector) const {
//	return glm::vec3(
//		basis[0].dot(p_vector) + origin.x,
//		basis[1].dot(p_vector) + origin.y,
//		basis[2].dot(p_vector) + origin.z);
//}
//
//glm::vec3 Transform3D::xform_inv(const glm::vec3& p_vector) const {
//	glm::vec3 v = p_vector - origin;
//
//	return glm::vec3(
//		(basis.rows[0][0] * v.x) + (basis.rows[1][0] * v.y) + (basis.rows[2][0] * v.z),
//		(basis.rows[0][1] * v.x) + (basis.rows[1][1] * v.y) + (basis.rows[2][1] * v.z),
//		(basis.rows[0][2] * v.x) + (basis.rows[1][2] * v.y) + (basis.rows[2][2] * v.z));
//}
//
//// Neither the plane regular xform or xform_inv are particularly efficient,
//// as they do a basis inverse. For xforming a large number
//// of planes it is better to pre-calculate the inverse transpose basis once
//// and reuse it for each plane, by using the 'fast' version of the functions.
//Plane Transform3D::xform(const Plane& p_plane) const {
//	Basis b = basis.inverse();
//	b.transpose();
//	return xform_fast(p_plane, b);
//}
//
//Plane Transform3D::xform_inv(const Plane& p_plane) const {
//	Transform3D inv = affine_inverse();
//	Basis basis_transpose = basis.transposed();
//	return xform_inv_fast(p_plane, inv, basis_transpose);
//}
//
//AABB Transform3D::xform(const AABB& p_aabb) const {
//	/* https://dev.theomader.com/transform-bounding-boxes/ */
//	glm::vec3 min = p_aabb.position;
//	glm::vec3 max = p_aabb.position + p_aabb.size;
//	glm::vec3 tmin, tmax;
//	for (int i = 0; i < 3; i++) {
//		tmin[i] = tmax[i] = origin[i];
//		for (int j = 0; j < 3; j++) {
//			real_t e = basis[i][j] * min[j];
//			real_t f = basis[i][j] * max[j];
//			if (e < f) {
//				tmin[i] += e;
//				tmax[i] += f;
//			}
//			else {
//				tmin[i] += f;
//				tmax[i] += e;
//			}
//		}
//	}
//	AABB r_aabb;
//	r_aabb.position = tmin;
//	r_aabb.size = tmax - tmin;
//	return r_aabb;
//}
//
//AABB Transform3D::xform_inv(const AABB& p_aabb) const {
//	/* define vertices */
//	glm::vec3 vertices[8] = {
//		glm::vec3(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z + p_aabb.size.z),
//		glm::vec3(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z),
//		glm::vec3(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y, p_aabb.position.z + p_aabb.size.z),
//		glm::vec3(p_aabb.position.x + p_aabb.size.x, p_aabb.position.y, p_aabb.position.z),
//		glm::vec3(p_aabb.position.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z + p_aabb.size.z),
//		glm::vec3(p_aabb.position.x, p_aabb.position.y + p_aabb.size.y, p_aabb.position.z),
//		glm::vec3(p_aabb.position.x, p_aabb.position.y, p_aabb.position.z + p_aabb.size.z),
//		glm::vec3(p_aabb.position.x, p_aabb.position.y, p_aabb.position.z)
//	};
//
//	AABB ret;
//
//	ret.position = xform_inv(vertices[0]);
//
//	for (int i = 1; i < 8; i++) {
//		ret.expand_to(xform_inv(vertices[i]));
//	}
//
//	return ret;
//}
//
//Vector<glm::vec3> Transform3D::xform(const Vector<glm::vec3>& p_array) const {
//	Vector<glm::vec3> array;
//	array.resize(p_array.size());
//
//	const glm::vec3* r = p_array.ptr();
//	glm::vec3* w = array.ptrw();
//
//	for (int i = 0; i < p_array.size(); ++i) {
//		w[i] = xform(r[i]);
//	}
//	return array;
//}
//
//Vector<glm::vec3> Transform3D::xform_inv(const Vector<glm::vec3>& p_array) const {
//	Vector<glm::vec3> array;
//	array.resize(p_array.size());
//
//	const glm::vec3* r = p_array.ptr();
//	glm::vec3* w = array.ptrw();
//
//	for (int i = 0; i < p_array.size(); ++i) {
//		w[i] = xform_inv(r[i]);
//	}
//	return array;
//}
//
//Plane Transform3D::xform_fast(const Plane& p_plane, const Basis& p_basis_inverse_transpose) const {
//	// Transform a single point on the plane.
//	glm::vec3 point = p_plane.normal * p_plane.d;
//	point = xform(point);
//
//	// Use inverse transpose for correct normals with non-uniform scaling.
//	glm::vec3 normal = p_basis_inverse_transpose.xform(p_plane.normal);
//	normal.normalize();
//
//	real_t d = normal.dot(point);
//	return Plane(normal, d);
//}
//
//Plane Transform3D::xform_inv_fast(const Plane& p_plane, const Transform3D& p_inverse, const Basis& p_basis_transpose) {
//	// Transform a single point on the plane.
//	glm::vec3 point = p_plane.normal * p_plane.d;
//	point = p_inverse.xform(point);
//
//	// Note that instead of precalculating the transpose, an alternative
//	// would be to use the transpose for the basis transform.
//	// However that would be less SIMD friendly (requiring a swizzle).
//	// So the cost is one extra precalced value in the calling code.
//	// This is probably worth it, as this could be used in bottleneck areas. And
//	// where it is not a bottleneck, the non-fast method is fine.
//
//	// Use transpose for correct normals with non-uniform scaling.
//	glm::vec3 normal = p_basis_transpose.xform(p_plane.normal);
//	normal.normalize();
//
//	real_t d = normal.dot(point);
//	return Plane(normal, d);
//}
//
//
//

static real_t get_fovy(real_t p_fovx, real_t p_aspect) {
	return glm::degrees(glm::atan(p_aspect * glm::tan(glm::radians(p_fovx) * 0.5)) * 2.0);
}

struct Projection {
	enum Planes {
		PLANE_NEAR,
		PLANE_FAR,
		PLANE_LEFT,
		PLANE_TOP,
		PLANE_RIGHT,
		PLANE_BOTTOM
	};
	//
	glm::vec4 columns[4];
	//
	//	const glm::vec4& operator[](int p_axis) const {
	//		DEV_ASSERT((unsigned int)p_axis < 4);
	//		return columns[p_axis];
	//	}
	//
	//	glm::vec4& operator[](int p_axis) {
	//		DEV_ASSERT((unsigned int)p_axis < 4);
	//		return columns[p_axis];
	//	}
	//
	//	float determinant() const;
	void set_identity() {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				columns[i][j] = (i == j) ? 1 : 0;
			}
		}
	}
	//	void set_zero();
	//	void set_light_bias();
	//	void set_depth_correction(bool p_flip_y = true);
	//
	//	void set_light_atlas_rect(const Rect2& p_rect);
	void set_perspective(real_t p_fovy_degrees, real_t p_aspect, real_t p_z_near, real_t p_z_far, bool p_flip_fov = false) {
		if (p_flip_fov) {
			p_fovy_degrees = get_fovy(p_fovy_degrees, 1.0 / p_aspect);
		}

		real_t sine, cotangent, deltaZ;
		real_t radians = glm::radians(p_fovy_degrees / 2.0);

		deltaZ = p_z_far - p_z_near;
		sine = glm::sin(radians);

		if ((deltaZ == 0) || (sine == 0) || (p_aspect == 0)) {
			return;
		}
		cotangent = glm::cos(radians) / sine;

		set_identity();

		columns[0][0] = cotangent / p_aspect;
		columns[1][1] = cotangent;
		columns[2][2] = -(p_z_far + p_z_near) / deltaZ;
		columns[2][3] = -1;
		columns[3][2] = -2 * p_z_near * p_z_far / deltaZ;
		columns[3][3] = 0;
	}

	//	void set_perspective(real_t p_fovy_degrees, real_t p_aspect, real_t p_z_near, real_t p_z_far, bool p_flip_fov, int p_eye, real_t p_intraocular_dist, real_t p_convergence_dist);
	//	void set_for_hmd(int p_eye, real_t p_aspect, real_t p_intraocular_dist, real_t p_display_width, real_t p_display_to_lens, real_t p_oversample, real_t p_z_near, real_t p_z_far);
	void set_orthogonal(real_t p_left, real_t p_right, real_t p_bottom, real_t p_top, real_t p_znear, real_t p_zfar) {
		set_identity();

		columns[0][0] = 2.0 / (p_right - p_left);
		columns[3][0] = -((p_right + p_left) / (p_right - p_left));
		columns[1][1] = 2.0 / (p_top - p_bottom);
		columns[3][1] = -((p_top + p_bottom) / (p_top - p_bottom));
		columns[2][2] = -2.0 / (p_zfar - p_znear);
		columns[3][2] = -((p_zfar + p_znear) / (p_zfar - p_znear));
		columns[3][3] = 1.0;
	}

	//	void set_orthogonal(real_t p_size, real_t p_aspect, real_t p_znear, real_t p_zfar, bool p_flip_fov = false);
	//	void set_frustum(real_t p_left, real_t p_right, real_t p_bottom, real_t p_top, real_t p_near, real_t p_far);
	//	void set_frustum(real_t p_size, real_t p_aspect, Vector2 p_offset, real_t p_near, real_t p_far, bool p_flip_fov = false);
	//	void adjust_perspective_znear(real_t p_new_znear);
	//
	//	static Projection create_depth_correction(bool p_flip_y);
	//	static Projection create_light_atlas_rect(const Rect2& p_rect);
	//	static Projection create_perspective(real_t p_fovy_degrees, real_t p_aspect, real_t p_z_near, real_t p_z_far, bool p_flip_fov = false);
	//	static Projection create_perspective_hmd(real_t p_fovy_degrees, real_t p_aspect, real_t p_z_near, real_t p_z_far, bool p_flip_fov, int p_eye, real_t p_intraocular_dist, real_t p_convergence_dist);
	//	static Projection create_for_hmd(int p_eye, real_t p_aspect, real_t p_intraocular_dist, real_t p_display_width, real_t p_display_to_lens, real_t p_oversample, real_t p_z_near, real_t p_z_far);
	//	static Projection create_orthogonal(real_t p_left, real_t p_right, real_t p_bottom, real_t p_top, real_t p_znear, real_t p_zfar);
	//	static Projection create_orthogonal_aspect(real_t p_size, real_t p_aspect, real_t p_znear, real_t p_zfar, bool p_flip_fov = false);
	//	static Projection create_frustum(real_t p_left, real_t p_right, real_t p_bottom, real_t p_top, real_t p_near, real_t p_far);
	//	static Projection create_frustum_aspect(real_t p_size, real_t p_aspect, Vector2 p_offset, real_t p_near, real_t p_far, bool p_flip_fov = false);
	//	static Projection create_fit_aabb(const AABB& p_aabb);
	//	Projection perspective_znear_adjusted(real_t p_new_znear) const;
	//	Plane get_projection_plane(Planes p_plane) const;
	//	Projection flipped_y() const;
	//	Projection jitter_offseted(const Vector2& p_offset) const;
	//
	//	static real_t get_fovy(real_t p_fovx, real_t p_aspect) {
	//		return Math::rad_to_deg(Math::atan(p_aspect * Math::tan(Math::deg_to_rad(p_fovx) * 0.5)) * 2.0);
	//	}
	//
	//	real_t get_z_far() const;
	//	real_t get_z_near() const;
	//	real_t get_aspect() const;
	//	real_t get_fov() const;
	//	bool is_orthogonal() const;
	//
		Vector<Plane> get_projection_planes(const Transform3D& p_transform) const {
			/** Fast Plane Extraction from combined modelview/projection matrices.
			 * References:
			 * https://web.archive.org/web/20011221205252/https://www.markmorley.com/opengl/frustumculling.html
			 * https://web.archive.org/web/20061020020112/https://www2.ravensoft.com/users/ggribb/plane%20extraction.pdf
			 */

			Vector<Plane> planes;
			planes.resize(6);

			const real_t* matrix = (const real_t*)columns;

			Plane new_plane;

			///////--- Near Plane ---///////
			new_plane = Plane(matrix[3] + matrix[2],
				matrix[7] + matrix[6],
				matrix[11] + matrix[10],
				matrix[15] + matrix[14]);

			new_plane.normal = -new_plane.normal;
			new_plane.normalize();

			planes[0] = p_transform.xform(new_plane);

			///////--- Far Plane ---///////
			new_plane = Plane(matrix[3] - matrix[2],
				matrix[7] - matrix[6],
				matrix[11] - matrix[10],
				matrix[15] - matrix[14]);

			new_plane.normal = -new_plane.normal;
			new_plane.normalize();

			planes[1] = p_transform.xform(new_plane);

			///////--- Left Plane ---///////
			new_plane = Plane(matrix[3] + matrix[0],
				matrix[7] + matrix[4],
				matrix[11] + matrix[8],
				matrix[15] + matrix[12]);

			new_plane.normal = -new_plane.normal;
			new_plane.normalize();

			planes[2] = p_transform.xform(new_plane);

			///////--- Top Plane ---///////
			new_plane = Plane(matrix[3] - matrix[1],
				matrix[7] - matrix[5],
				matrix[11] - matrix[9],
				matrix[15] - matrix[13]);

			new_plane.normal = -new_plane.normal;
			new_plane.normalize();

			planes[3] = p_transform.xform(new_plane);

			///////--- Right Plane ---///////
			new_plane = Plane(matrix[3] - matrix[0],
				matrix[7] - matrix[4],
				matrix[11] - matrix[8],
				matrix[15] - matrix[12]);

			new_plane.normal = -new_plane.normal;
			new_plane.normalize();

			planes[4] = p_transform.xform(new_plane);

			///////--- Bottom Plane ---///////
			new_plane = Plane(matrix[3] + matrix[1],
				matrix[7] + matrix[5],
				matrix[11] + matrix[9],
				matrix[15] + matrix[13]);

			new_plane.normal = -new_plane.normal;
			new_plane.normalize();

			planes[5] = p_transform.xform(new_plane);

			return planes;
		}
	//
	bool get_endpoints(const Transform3D& p_transform, glm::vec3* p_8points) const {
		std::vector<Plane> planes = get_projection_planes(Transform3D());
		const Planes intersections[8][3] = {
			{ PLANE_FAR, PLANE_LEFT, PLANE_TOP },
			{ PLANE_FAR, PLANE_LEFT, PLANE_BOTTOM },
			{ PLANE_FAR, PLANE_RIGHT, PLANE_TOP },
			{ PLANE_FAR, PLANE_RIGHT, PLANE_BOTTOM },
			{ PLANE_NEAR, PLANE_LEFT, PLANE_TOP },
			{ PLANE_NEAR, PLANE_LEFT, PLANE_BOTTOM },
			{ PLANE_NEAR, PLANE_RIGHT, PLANE_TOP },
			{ PLANE_NEAR, PLANE_RIGHT, PLANE_BOTTOM },
		};

		for (int i = 0; i < 8; i++) {
			Vector3 point;
			Plane a = planes[intersections[i][0]];
			Plane b = planes[intersections[i][1]];
			Plane c = planes[intersections[i][2]];
			bool res = a.intersect_3(b, c, &point);
			
			p_8points[i] = p_transform.xform(point);
		}

		return true;
	}
	//	Vector2 get_viewport_half_extents() const;
	//	Vector2 get_far_plane_half_extents() const;
	//
	//	void invert();
	//	Projection inverse() const;
	//
	//	Projection operator*(const Projection& p_matrix) const;
	//
	//	Plane xform4(const Plane& p_vec4) const;
	//	glm::vec3 xform(const glm::vec3& p_vec3) const;
	//
	//	glm::vec4 xform(const glm::vec4& p_vec4) const;
	//	glm::vec4 xform_inv(const glm::vec4& p_vec4) const;
	//
	//	operator String() const;
	//
	//	void scale_translate_to_fit(const AABB& p_aabb);
	//	void add_jitter_offset(const Vector2& p_offset);
	//	void make_scale(const glm::vec3& p_scale);
	//	int get_pixels_per_meter(int p_for_pixel_width) const;
	//	operator Transform3D() const;
	//
	//	void flip_y();
	//
	//	bool operator==(const Projection& p_cam) const {
	//		for (uint32_t i = 0; i < 4; i++) {
	//			for (uint32_t j = 0; j < 4; j++) {
	//				if (columns[i][j] != p_cam.columns[i][j]) {
	//					return false;
	//				}
	//			}
	//		}
	//		return true;
	//	}
	//
	//	bool operator!=(const Projection& p_cam) const {
	//		return !(*this == p_cam);
	//	}
	//
	//	float get_lod_multiplier() const;
	//
	//	Projection();
	//	Projection(const glm::vec4& p_x, const glm::vec4& p_y, const glm::vec4& p_z, const glm::vec4& p_w);
	//	Projection(const Transform3D& p_transform);
	//	~Projection();
};
	//
	//glm::vec3 Projection::xform(const glm::vec3& p_vec3) const {
	//	glm::vec3 ret;
	//	ret.x = columns[0][0] * p_vec3.x + columns[1][0] * p_vec3.y + columns[2][0] * p_vec3.z + columns[3][0];
	//	ret.y = columns[0][1] * p_vec3.x + columns[1][1] * p_vec3.y + columns[2][1] * p_vec3.z + columns[3][1];
	//	ret.z = columns[0][2] * p_vec3.x + columns[1][2] * p_vec3.y + columns[2][2] * p_vec3.z + columns[3][2];
	//	real_t w = columns[0][3] * p_vec3.x + columns[1][3] * p_vec3.y + columns[2][3] * p_vec3.z + columns[3][3];
	//	return ret / w;
	//}

static double snapped(double p_value, double p_step) {
	if (p_step != 0) {
		p_value = glm::floor(p_value / p_step + 0.5) * p_step;
	}
	return p_value;
}


std::vector<glm::mat4> cascadeCalculate(Transform3D light_transform, Transform3D p_cam_transform, float aspect, float fov, float shadowNear, float shadowFar, float textureSize) {
	// For later tight culling, the light culler needs to know the details of the directional light.

	light_transform.orthonormalize(); //scale does not count on lights
	//adssa

	float pancake_size = 0.0f;

	float min_distance = shadowNear;
	float max_distance = shadowFar;

	float range = max_distance - min_distance;

	int splits = 4;

	float distances[5];

	distances[0] = min_distance;

	static float d[4] = { .1f,.2f,.5f,1.0f };

	for (int i = 0; i < splits; i++) {
		distances[i + 1] = min_distance + d[4] * range;
	};

	distances[splits] = max_distance;

	float texture_size = textureSize;

	std::vector<glm::mat4> matrices;

	for (int i = 0; i < splits; i++) {
			// setup a camera matrix for that range!
			Projection camera_matrix;

			camera_matrix.set_perspective(fov, aspect, distances[(i == 0) ? i : i - 1], distances[i + 1], true);

			//obtain the frustum endpoints

			glm::vec3 endpoints[8]; // frustum plane endpoints
			bool res = camera_matrix.get_endpoints(p_cam_transform, endpoints);

			// obtain the light frustum ranges (given endpoints)

			Transform3D transform = light_transform; //discard scale and stabilize light

			glm::vec3 x_vec = glm::normalize(transform.basis.get_column(0));
			glm::vec3 y_vec = glm::normalize(transform.basis.get_column(1));
			glm::vec3 z_vec = glm::normalize(transform.basis.get_column(2));
			//z_vec points against the camera, like in default opengl

			float x_min = 0.f, x_max = 0.f;
			float y_min = 0.f, y_max = 0.f;
			float z_min = 0.f, z_max = 0.f;

			// FIXME: z_max_cam is defined, computed, but not used below when setting up
			// ortho_camera. Commented out for now to fix warnings but should be investigated.
			float x_min_cam = 0.f, x_max_cam = 0.f;
			float y_min_cam = 0.f, y_max_cam = 0.f;
			float z_min_cam = 0.f;
			//float z_max_cam = 0.f;

			//float bias_scale = 1.0;
			//float aspect_bias_scale = 1.0;

			//used for culling

			for (int j = 0; j < 8; j++) {
				float d_x = glm::dot(x_vec, endpoints[j]);
				float d_y = glm::dot(y_vec, endpoints[j]);
				float d_z = glm::dot(z_vec, endpoints[j]);

				if (j == 0 || d_x < x_min) {
					x_min = d_x;
				}
				if (j == 0 || d_x > x_max) {
					x_max = d_x;
				}

				if (j == 0 || d_y < y_min) {
					y_min = d_y;
				}
				if (j == 0 || d_y > y_max) {
					y_max = d_y;
				}

				if (j == 0 || d_z < z_min) {
					z_min = d_z;
				}
				if (j == 0 || d_z > z_max) {
					z_max = d_z;
				}
			}

			float radius = 0;
			float soft_shadow_expand = 0;
			glm::vec3 center;

			{
				//camera viewport stuff

				for (int j = 0; j < 8; j++) {
					center += endpoints[j];
				}
				center /= 8.0;

				//center=x_vec*(x_max-x_min)*0.5 + y_vec*(y_max-y_min)*0.5 + z_vec*(z_max-z_min)*0.5;

				for (int j = 0; j < 8; j++) {
					float d = glm::length(center - endpoints[j]);
					if (d > radius) {
						radius = d;
					}
				}

				radius *= texture_size / (texture_size - 2.0); //add a texel by each side

				z_min_cam = glm::dot(z_vec, center) - radius;


				// This trick here is what stabilizes the shadow (make potential jaggies to not move)
				// at the cost of some wasted resolution. Still, the quality increase is very well worth it.
				const float unit = (radius + soft_shadow_expand) * 4.0 / texture_size;
				x_max_cam = snapped(glm::dot(x_vec, center) + radius + soft_shadow_expand, unit);
				x_min_cam = snapped(glm::dot(x_vec, center) - radius - soft_shadow_expand, unit);
				y_max_cam = snapped(glm::dot(y_vec, center) + radius + soft_shadow_expand, unit);
				y_min_cam = snapped(glm::dot(y_vec, center) - radius - soft_shadow_expand, unit);
			}

			//now that we know all ranges, we can proceed to make the light frustum planes, for culling octree

			std::vector<Plane> light_frustum_planes;
			light_frustum_planes.resize(6);

			//right/left
			light_frustum_planes[0] = Plane(x_vec, x_max);
			light_frustum_planes[1] = Plane(-x_vec, -x_min);
			//top/bottom
			light_frustum_planes[2] = Plane(y_vec, y_max);
			light_frustum_planes[3] = Plane(-y_vec, -y_min);
			//near/far
			light_frustum_planes[4] = Plane(z_vec, z_max + 1e6);
			light_frustum_planes[5] = Plane(-z_vec, -z_min); // z_min is ok, since casters further than far-light plane are not needed

			// a pre pass will need to be needed to determine the actual z-near to be used

			z_max = glm::dot(z_vec, center) + radius + pancake_size;

			{
				Projection ortho_camera;
				float half_x = (x_max_cam - x_min_cam) * 0.5;
				float half_y = (y_max_cam - y_min_cam) * 0.5;

				ortho_camera.set_orthogonal(-half_x, half_x, -half_y, half_y, 0, (z_max - z_min_cam));

				glm::vec2 uv_scale(1.0 / (x_max_cam - x_min_cam), 1.0 / (y_max_cam - y_min_cam));

				Transform3D ortho_transform;
				ortho_transform.basis = transform.basis;
				ortho_transform.origin = x_vec * (x_min_cam + half_x) + y_vec * (y_min_cam + half_y) + z_vec * z_max;

				glm::mat4 orthoTransformMat =
					glm::mat4(
						glm::vec4(ortho_transform.basis.rows[0], 0.0),
						glm::vec4(ortho_transform.basis.rows[1], 0.0),
						glm::vec4(ortho_transform.basis.rows[2], 0.0),
						glm::vec4(ortho_transform.origin, 1.0));
				glm::mat4 orthoProjMat =
					glm::mat4(ortho_camera.columns[0], ortho_camera.columns[1], ortho_camera.columns[2], ortho_camera.columns[3]);

				matrices.push_back(orthoProjMat * orthoTransformMat);
			}
		}

		return matrices;
}

