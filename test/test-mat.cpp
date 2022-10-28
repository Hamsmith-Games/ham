#include "tests.hpp"

using namespace ham::typedefs;

bool ham_test_mat(){
	// TODO: write matrix math tests

	const auto forward = ham::vec3(0.f, 0.f, 1.f);

	const auto pos = ham::vec3(0.f, 1.f, 1.f);
	const auto focus = pos + forward;
	const auto up = ham::vec3(0.f, 1.f, 0.f);

	const ham::mat4 view = ham_look_at(pos, focus, up);

	const auto glm_forward = glm::vec3(0.f, 0.f, 1.f);

	const auto glm_pos = glm::vec3(0.f, 1.f, 1.f);
	const auto glm_focus = glm_pos + glm_forward;
	const auto glm_up = glm::vec3(0.f, 1.f, 0.f);

	const auto glm_view = glm::lookAtLH(glm_pos, glm_focus, glm_up);

	const int result = roughly_equal(view, glm_view) - 1;

	ham_test_assert_msg(
		result == -1,
		"Bad look at matrix at (%d, %d): got %f, expected %f",
		result / 4, result % 4,
		view.data()[result],
		glm::value_ptr(glm_view)[result]
	);

	return true;
}
