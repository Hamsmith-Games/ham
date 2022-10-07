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

#include "ham/buffer.h"

#include "tests.hpp"

using namespace ham::typedefs;

struct test_obj{
	ham::str8 str;
	float f;
	int i;
};

bool ham_test_buffer(){
	// 'buf.insert' test
	{
		ham::basic_buffer<test_obj> test_buf;

		const ham::str8 strs[] = {
			"hello", "world", "hi", "there"
		};

		for(int i = 0; i < (int)std::size(strs); i++){
			test_buf.insert(test_buf.size(), { strs[i], (float)i + 1.f, i + 1 });
		}

		for(int i = 0; i < (int)std::size(strs); i++){
			const auto &test_val = test_buf[i];
			if(test_val.str != strs[i]){
				std::cerr << "Insert error on " << i << "'th value: bad string\n";
				return false;
			}
			else if(test_val.f != ((float)i + 1.f)){
				std::cerr << "Insert error on " << i << "'th value: bad float\n"
					"Expected: " << ((float)i + 1.f) << "\n"
					"Got:      " << test_val.f << '\n';

				return false;
			}
			else if(test_val.i != i + 1){
				std::cerr << "Insert error on " << i << "'th value: bad int\n";
				return false;
			}
		}
	}

	// copy constructor test
	{
		ham::basic_buffer<int> test0 = { 1, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5 };
		ham::basic_buffer<int> test1;

		{
			ham::basic_buffer<int> tmp = test0;
			test1 = std::move(tmp);
		}

		if(memcmp(test0.data(), test1.data(), test0.size() * sizeof(int)) != 0){
			std::cerr << "Copy constructor error: bad values\n"
				"Expected: " <<test0[0]<<' '<<test0[1]<<' '<<test0[2]<<' '<<test0[3]<<' '<<test0[4]<<' '<<test0[5] << "\n"
				"Got:      " <<test1[0]<<' '<<test1[1]<<' '<<test1[2]<<' '<<test1[3]<<' '<<test1[4]<<' '<<test1[5] << '\n';

			return false;
		}
	}

	return true;
}
