#include "tests.hpp"

#include "ham/typedefs.h"

#include "glm/gtc/type_ptr.hpp"

using namespace ham::typedefs;

inline bool operator==(const glm::quat &a, const ham::quat &b) noexcept{
	return std::memcmp(glm::value_ptr(a), b.data(), sizeof(f32) * 4) == 0;
}

inline bool operator==(const ham::quat &a, const glm::quat &b) noexcept{
	return std::memcmp(a.data(), glm::value_ptr(b), sizeof(f32) * 4) == 0;
}

inline bool operator!=(const glm::quat &a, const ham::quat &b) noexcept{
	return std::memcmp(glm::value_ptr(a), b.data(), sizeof(f32) * 4) != 0;
}

inline bool operator!=(const ham::quat &a, const glm::quat &b) noexcept{
	return std::memcmp(a.data(), glm::value_ptr(b), sizeof(f32) * 4) != 0;
}

bool ham_test_quat(){
	const auto pyr = ham::vec3{M_PI, 0.5 * M_PI, 1.9 * M_PI}; // glm::sin && glm::cos do some sort of funny business at/after 2.0 * M_PI

	const auto qp     = ham::quat::from_pitch(pyr.x());
	const auto glm_qp = glm::angleAxis(pyr.x(), glm::vec3{1.f, 0.f, 0.f});
	ham_test_assert_msg(roughly_equal(qp, glm_qp) == 0, "Bad pitch");

	const auto qy = ham::quat::from_yaw(pyr.y());
	const auto glm_qy = glm::angleAxis(pyr.y(), glm::vec3{0.f, 1.f, 0.f});
	ham_test_assert_msg(roughly_equal(qy, glm_qy) == 0, "Bad yaw");

	const auto qr     = ham::quat::from_roll(pyr.z());
	const auto glm_qr = glm::angleAxis(pyr.z(), glm::vec3{0.f, 0.f, 1.f});
	ham_test_assert_msg(roughly_equal(qr, glm_qr) == 0, "Bad roll");

	const auto q = qp * qy * qr;
	const auto glm_q = glm_qp * glm_qy * glm_qr;
	ham_test_assert_msg(roughly_equal(q, glm_q) == 0, "Bad multiplication");

	// glm (the dirty dog!) swaps components in normalization iff using xyzw order
	// see: glm::normalize<glm::quat> and glm::qua<...>::qua(x, y, z, w)

	const auto glm_qlen_inv = 1.f / glm::length(glm_q);

	const auto qn = q.normalized();
	const auto glm_qn = glm::quat(glm_q.x * glm_qlen_inv, glm_q.y * glm_qlen_inv, glm_q.z * glm_qlen_inv, glm_q.w * glm_qlen_inv);

	ham_test_assert_msg(
		roughly_equal(qn, glm_qn) == 0,
		"Bad normalization, got (%f, %f, %f, %f) expected (%f, %f, %f, %f)",
		qn.x(), qn.y(), qn.z(), qn.w(),
		glm_qn.x, glm_qn.y, glm_qn.z, glm_qn.w
	);

	const auto mat = qn.to_mat4();
	const auto glm_mat = glm::mat4_cast(glm_qn);

	int result = roughly_equal(mat, glm_mat) - 1;

	ham_test_assert_msg(
		result == -1,
		"Bad matrix conversion at (%d, %d): got %f, expected %f",
		result / 4, result % 4,
		mat.data[result],
		glm::value_ptr(glm_mat)[result]
	);

	return true;
}
