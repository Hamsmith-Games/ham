/*
 * Ham Runtime Tests
 * Copyright (C) 2022  Hamsmith Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "tests.hpp"

#include "ham/octree.h"

using namespace ham::typedefs;

struct test_type{
	explicit test_type(u32 u) noexcept{
		const f64 f = 1.0 / ((f64)u + 1.0);
		u0 = u;
		u1 = u+1;
		u2 = u+2;
		f0 = f;
		f1 = f+1;
		f2 = f+2;
	}

	bool check(u32 expected_u0) const noexcept{
		const f64 expected_f0 = 1.0 / ((f64)expected_u0 + 1.0);
		if(u0 != expected_u0) return false;
		else if(u1 != expected_u0+1) return false;
		else if(u2 != expected_u0+2) return false;
		else if(f0 != expected_f0) return false;
		else if(f1 != expected_f0+1) return false;
		else if(f2 != expected_f0+2) return false;
		else return true;
	}

	u32 u0, u1, u2;
	f64 f0, f1, f2;
};

bool ham_test_octree(){
	ham::octree<test_type> tree;

	const auto root = tree.root();

#define check_node(node_, u_) \
	if(!node_){ \
		std::cerr << "Could not emplace node " #node_ "\n"; \
		return false; \
	} \
	else if(!node_.value_ptr()){ \
		std::cerr << "Failed to emplace value in node " #node_ "\n"; \
		return false; \
	} \
	else if(!node_.value().check((u_))){ \
		std::cerr << "Bad node value for " #node_ "\n"; \
		return false; \
	}

	const auto t0 = root.emplace_leaf(0, 1);
	const auto t1 = root.emplace_leaf(1, 2);

	const auto branch0 = root.emplace(3);

	const auto t2 = branch0.emplace_leaf(0, 3);
	const auto t3 = branch0.emplace_leaf(1, 4);

	const auto branch1 = root.emplace(5);

	const auto t4 = branch1.emplace_leaf(0, 5);
	const auto t5 = branch1.emplace_leaf(1, 6);

	const auto moved_tree = std::move(tree);
	(void)moved_tree;

	check_node(t0, 1);
	check_node(t1, 2);
	check_node(t2, 3);
	check_node(t3, 4);
	check_node(t4, 5);
	check_node(t5, 6);

	return true;
}
