/**
 * The Ham World Engine
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

#include "ham/engine.h"

#include <getopt.h>

HAM_C_API_BEGIN

static struct option ham_impl_long_options[] = {
	{0, 0, 0, 0}
};

void ham_engine_init(int argc, char **argv){
	(void)argc; (void)argv;


}

int ham_exec(){ return 0; }

HAM_C_API_END
