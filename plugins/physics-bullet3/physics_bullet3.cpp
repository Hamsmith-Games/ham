/*
 * Ham Runtime Plugins
 * Copyright (C) 2022 Keith Hammond
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "physics_bullet3.h"

#include "ham/plugin.h"

#include "Bullet3OpenCL/Initialize/b3OpenCLUtils.h"

using namespace ham::typedefs;

HAM_C_API_BEGIN

HAM_PLUGIN(
	ham_physics_bullet3,
	HAM_PHYSICS_BULLET3_PLUGIN_UUID,
	HAM_PHYSICS_BULLET3_PLUGIN_NAME,
	HAM_VERSION,
	"Bullet3 Physics",
	"Keith Hammond",
	"LGPLv3+",
	HAM_PHYSICS_PLUGIN_CATEGORY,
	"Physics simulation using Bullet3",

	ham_plugin_init_pass,
	ham_plugin_fini_pass
)

ham_def_ctor(ham_physics_bullet3, nargs, va){
	(void)nargs; (void)va;

	cl_int err_no;
	const auto cl_ctx = b3OpenCLUtils::createContextFromType(CL_DEVICE_TYPE_GPU, &err_no);
	if(!cl_ctx){
		self->use_gpu = false;
	}
	else{
		self->use_gpu = true;
		self->cl_ctx  = cl_ctx;
		self->cl_dev_id = b3OpenCLUtils::getDevice(cl_ctx, 0);
		self->cl_queue = clCreateCommandQueueWithProperties(cl_ctx, self->cl_dev_id, nullptr, &err_no);
	}

	return self;
}

ham_def_dtor(ham_physics_bullet3){
	if(self->use_gpu){
		clReleaseCommandQueue(self->cl_queue);
		clReleaseContext(self->cl_ctx);
	}
}

void ham_def_method(ham_physics_bullet3, tick, f64 dt){
	(void)self; (void)dt;
}

ham_def_physics_object(ham_physics_bullet3, ham_physics_world_bullet3, ham_physics_shape_bullet3, ham_physics_body_bullet3)

HAM_C_API_END
