#include "tests.hpp"

#include "ham/camera.h"

using namespace ham::typedefs;

bool ham_test_camera(){
	ham::camera cam;
	ham::mat4 proj, view_proj;
	glm::mat4 glm_proj, glm_view_proj;

	cam.reset_transform();
	cam.translate({0.f, 1.f, 1.f});
	cam.rotate(M_PI, {0.f, 1.f, 0.f});

	const auto glm_qp = glm::angleAxis(0.f, glm::vec3{1.f, 0.f, 0.f});
	const auto glm_qy = glm::angleAxis(float(M_PI), glm::vec3{0.f, 1.f, 0.f});
	const auto glm_qr = glm::angleAxis(0.f, glm::vec3{0.f, 0.f, 1.f});
	const auto glm_qpyr = glm_qp * glm_qy * glm_qr;
	const auto glm_qpyr_inv_len = 1.f / glm::length(glm_qpyr);
	const auto glm_qn = glm::quat(glm_qpyr.x * glm_qpyr_inv_len, glm_qpyr.y * glm_qpyr_inv_len, glm_qpyr.z * glm_qpyr_inv_len, glm_qpyr.w * glm_qpyr_inv_len);

	const auto glm_forward = glm::normalize(glm_qn * glm::vec3(0.f, 0.f, 1.f));

	glm::quatLookAtLH(glm_forward, glm::vec3(0.f, 1.f, 0.f));

	const auto glm_eye = glm::vec3(0.f, 1.f, 1.f);
	const auto glm_focus = glm_eye + glm_forward;
	const auto glm_view = glm::lookAtLH<float>(glm_eye, glm_focus, {0.f, 1.f, 0.f});

	const auto view = cam.view_matrix();

	int result = roughly_equal(view, glm_view) - 1;

	ham_test_assert_msg(
		result == -1,
		"Bad view matrix at col %d, row %d (expected %f, got %f)",
		result / 4, result % 4,
		glm::value_ptr(glm_view)[result],
		view.data()[result]
	);

	{
		cam.set_perspective(1280.f/720.f, M_PI_2 - 0.0001, 0.001f, 1000.f);

		proj = cam.projection_matrix();
		glm_proj = glm::perspectiveLH_ZO<f32>(M_PI_2 - 0.0001, 1280.f/720.f, 0.001f, 1000.f);

		view_proj = proj * cam.view_matrix();
		glm_view_proj = glm_proj * glm_view;

		result = roughly_equal(proj, glm_proj) - 1;

		ham_test_assert_msg(
			result == -1,
			"Bad perspective projection matrix at col %d, row %d (expected %f, got %f)",
			result / 4, result % 4,
			glm::value_ptr(glm_proj)[result],
			proj.data()[result]
		);

		result = roughly_equal(view_proj, glm_view_proj) - 1;

		ham_test_assert_msg(
			result == -1,
			"Bad perspective view-projection matrix at col %d, row %d (expected %f, got %f)",
			result / 4, result % 4,
			glm::value_ptr(glm_view_proj)[result],
			view_proj.data()[result]
		);
	}

	{
		cam.set_orthographic(1.f, -1.f, -1.f, 1.f, 0.f, 1000.f);

		view_proj = cam.projection_matrix() * cam.view_matrix();

		glm_view_proj = glm::orthoLH_ZO<f32>(-1.f, 1.f, -1.f, 1.f, 0.f, 1000.f) * glm_view;

		result = roughly_equal(view_proj, glm_view_proj) - 1;

		ham_test_assert_msg(
			result == -1,
			"Bad perspective projection matrix at col %d, row %d (expected %f, got %f)",
			result / 4, result % 4,
			glm::value_ptr(glm_view_proj)[result],
			view_proj.data()[result]
		);
	}

	return true;
}
