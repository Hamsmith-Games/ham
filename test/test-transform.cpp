#include "tests.hpp"

#include "ham/transform.h"

using namespace ham::typedefs;

bool ham_test_transform(){
	const glm::mat4 glm_ident = glm::mat4(1.f);

	ham_test_assert_msg(ham::mat4() == glm::mat4(1.f), "Bad identity matrix");

	ham::transform trans;
	trans.translate({0.f, 1.f, 0.f});
	trans.rotate(M_PI, {0.5f, 1.f, 0.f});
	trans.scale({10.f, 5.f, 2.5f});

	const auto glm_scale = glm::scale(glm::mat4(1.f), {10.f, 5.f, 2.5f});
	const auto scale = ham::mat4(1.f).scaled({10.f, 5.f, 2.5f});

	int result = roughly_equal(scale, glm_scale) - 1;

	ham_test_assert_msg(
		result == -1,
		"Bad scale matrix at (%d, %d): got %f, expected %f",
		result / 4, result % 4,
		scale.data()[result],
		glm::value_ptr(glm_scale)[result]
	);

	const auto qp = ham::quat::from_pitch(M_PI * 0.5f);
	const auto qy = ham::quat::from_yaw(M_PI);
	const auto qr = ham::quat::from_roll(0.f);

	const auto qpyr = qp * qy * qr;
	const auto qn = qpyr.normalized();

	const auto glm_qp = glm::angleAxis(float(M_PI * 0.5f), glm::vec3{1.f, 0.f, 0.f});
	const auto glm_qy = glm::angleAxis(float(M_PI), glm::vec3{0.f, 1.f, 0.f});
	const auto glm_qr = glm::angleAxis(0.f, glm::vec3{0.f, 0.f, 1.f});

	const auto glm_qpyr = glm_qp * glm_qy * glm_qr;
	const auto glm_qpyr_inv_len = glm::length(glm_qpyr);
	const auto glm_qn = glm::quat(
		glm_qpyr.x * glm_qpyr_inv_len,
		glm_qpyr.y * glm_qpyr_inv_len,
		glm_qpyr.z * glm_qpyr_inv_len,
		glm_qpyr.w * glm_qpyr_inv_len
	);

	ham_test_assert_msg(
		roughly_equal(qn, glm_qn) == 0,
		"Bad quat orientation: got (%f, %f, %f, %f), expected (%f, %f, %f, %f)",
		qn.x(), qn.y(), qn.z(), qn.w(),
		glm_qn.x, glm_qn.y, glm_qn.z, glm_qn.w
	);

	const auto rot = ham::mat4::from_quat(qn);
	const auto glm_rot = glm::mat4_cast(glm_qn);

	result = roughly_equal(rot, glm_rot) - 1;

	ham_test_assert_msg(
		result == -1,
		"Bad quaternion matrix conversion at (%d, %d): got %f, expected %f",
		result / 4, result % 4,
		rot.data()[result], glm::value_ptr(glm_rot)[result]
	);

	const auto rot_inv = rot.inverse();
	const auto glm_rot_inv = glm::inverse(glm_rot);

	result = roughly_equal(rot_inv, glm_rot_inv) - 1;

	ham_test_assert_msg(
		result == -1,
		"Bad matrix inversion at (%d, %d): got %f, expected %f",
		result / 4, result % 4,
		rot_inv.data()[result], glm::value_ptr(glm_rot_inv)[result]
	);

	const glm::mat4 glm_trans = glm::translate(glm_ident, {0.f, -1.f, 0.f}) * glm_rot_inv * glm_scale;
	const ham::mat4 trans_m = ham::mat4(1.f).translated({0.f, -1.f, 0.f}) * rot_inv * scale;

	result = roughly_equal(trans_m, glm_trans) - 1;

	ham_test_assert_msg(
		result == -1,
		"Bad matrix multiplication at (%d, %d) got %f, expected %f",
		result / 4, result % 4,
		trans_m.data()[result],
		glm::value_ptr(glm_trans)[result]
	);

	const auto trans_mat = trans.matrix();

	result = roughly_equal(trans_mat, trans_m) - 1;

	ham_test_assert_msg(
		result == -1,
		"Bad transformation matrix at (%d, %d) got %f, expected %f",
		result / 4, result % 4,
		trans_mat.data()[result],
		trans_m.data()[result]
	);

	return true;
}
