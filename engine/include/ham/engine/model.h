/*
 * Ham World Engine Runtime
 * Copyright (C) 2022 Hamsmith Ltd.
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

#ifndef HAM_ENGINE_MODEL_H
#define HAM_ENGINE_MODEL_H 1

/**
 * @defgroup HAM_ENGINE_MODEL Models
 * @ingroup HAM_ENGINE
 * @{
 */

#include "ham/engine/config.h"

#include "ham/shape.h"
#include "ham/image.h"

HAM_C_API_BEGIN

typedef struct ham_model ham_model;

ham_engine_api ham_model *ham_model_load(ham_str8 filepath);

ham_engine_api ham_model *ham_model_load_from_mem(ham_usize len, const void *data);

ham_engine_api ham_nothrow void ham_model_destroy(ham_model *mdl);

ham_engine_api ham_nothrow ham_usize ham_model_num_shapes(const ham_model *mdl);
ham_engine_api ham_nothrow const ham_shape *const *ham_model_shapes(const ham_model *mdl);
ham_engine_api ham_nothrow const ham_image *const *ham_model_images(const ham_model *mdl);

ham_engine_api void ham_model_fill_blank_images(ham_model *mdl, const ham_image *img);

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	class model_exception: public exception{};

	class model_load_error: public model_exception{
		public:
			const char *api() const noexcept override{ return "ham::model::model"; }
			const char *what() const noexcept override{ return "Error in ham_model_load"; }
	};

	class model{
		public:
			explicit model(const ham::str8 &filepath)
				: m_handle(ham_model_load(filepath))
			{
				if(!m_handle){
					throw model_load_error();
				}
			}

			model(ham_usize len, const void *data)
				: m_handle(ham_model_load_from_mem(len, data))
			{
				if(!m_handle){
					throw model_load_error();
				}
			}

			usize num_shapes() const noexcept{ return ham_model_num_shapes(m_handle.get()); }
			const ham_shape *const *shapes() const noexcept{ return ham_model_shapes(m_handle.get()); }
			const ham_image *const *images() const noexcept{ return ham_model_images(m_handle.get()); }

			void fill_blank_images(const ham_image *img){ ham_model_fill_blank_images(m_handle.get(), img); }

		private:
			unique_handle<ham_model*, ham_model_destroy> m_handle;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // HAM_ENGINE_MODEL_H
