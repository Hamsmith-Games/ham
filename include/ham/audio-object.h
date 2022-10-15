#ifndef HAM_AUDIO_OBJECT_H
#define HAM_AUDIO_OBJECT_H 1

/**
 * @defgroup HAM_AUDIO_OBJECT Object definitions
 * @ingroup HAM_AUDIO
 * @{
 */

#include "audio.h"
#include "object.h"

HAM_C_API_BEGIN

typedef struct ham_audio_context_vtable ham_audio_context_vtable;
typedef struct ham_audio_listener_vtable ham_audio_listener_vtable;
typedef struct ham_audio_source_vtable ham_audio_source_vtable;

struct ham_audio_context{
	ham_derive(ham_object)
};

struct ham_audio_listener{
	ham_derive(ham_object)
};

struct ham_audio_source{
	ham_derive(ham_object)
};

struct ham_audio_context_vtable{
	ham_derive(ham_object_vtable)

	const ham_audio_listener_vtable*(*listener_vptr)();
	const ham_audio_source_vtable*(*source_vptr)();
};

struct ham_audio_listener_vtable{
	ham_derive(ham_object_vtable)

	void(*set_pos)(ham_audio_listener *self, ham_vec3 new_pos);
	void(*set_rotation)(ham_audio_listener *self, ham_vec3 new_pyr);
};

struct ham_audio_source_vtable{
	ham_derive(ham_object_vtable)

	void(*set_pos)(ham_audio_source *self, ham_vec3 new_pos);
	void(*set_rotation)(ham_audio_source *self, ham_vec3 new_pyr);

	bool(*play)(ham_audio_source *self);
	bool(*pause)(ham_audio_source *self);
	bool(*stop)(ham_audio_source *self);
};

//! @cond ignore

#define ham_impl_define_audio_context_object(derived_context, derived_listener, derived_source) \
	ham_expose_object_vptr(derived_listener) \
	ham_expose_object_vptr(derived_source) \
	static inline const ham_audio_listener_vtable *ham_impl_listener_vptr_##derived_context(){ return (const ham_audio_listener_vtable*)(ham_object_vptr_name(derived_listener)()); } \
	static inline const ham_audio_source_vtable *ham_impl_source_vptr_##derived_context(){ return (const ham_audio_source_vtable*)(ham_object_vptr_name(derived_source)()); } \
	ham_define_object_x( \
		2, derived_context, 1, ham_audio_context_vtable, \
		derived_context##_ctor, derived_context##_dtor, \
		( \
			.listener_vptr = ham_impl_listener_vptr_##derived_context, \
			.source_vptr   = ham_impl_source_vptr_##derived_context, \
		) \
	)

#define ham_impl_define_audio_listener_object(derived_listener) \
	static inline void ham_impl_set_pos_##derived_listener(ham_audio_listener *self, ham_vec3 new_pos){ (derived_listener##_set_pos)((derived_listener*)self, new_pos); } \
	static inline void ham_impl_set_rotation_##derived_listener(ham_audio_listener *self, ham_vec3 new_pyr){ (derived_listener##_set_rotation)((derived_listener*)self, new_pyr); } \
	ham_define_object_x( \
		2, derived_listener, 1, ham_audio_listener_vtable, \
		derived_listener##_ctor, derived_listener##_dtor, \
		( \
			.set_pos =  ham_impl_set_pos_##derived_listener, \
			.set_rotation =  ham_impl_set_rotation_##derived_listener, \
		) \
	)

#define ham_impl_define_audio_source_object(derived_source) \
	static inline void ham_impl_set_pos_##derived_source(ham_audio_source *self, ham_vec3 new_pos){ (derived_source##_set_pos)((derived_source*)self, new_pos); } \
	static inline void ham_impl_set_rotation_##derived_source(ham_audio_source *self, ham_vec3 new_pyr){ (derived_source##_set_rotation)((derived_source*)self, new_pyr); } \
	static inline bool ham_impl_play_##derived_source(ham_audio_source *self){ return (derived_source##_play)((derived_source*)self); } \
	static inline bool ham_impl_pause_##derived_source(ham_audio_source *self){ return (derived_source##_pause)((derived_source*)self); } \
	static inline bool ham_impl_stop_##derived_source(ham_audio_source *self){ return (derived_source##_stop)((derived_source*)self); } \
	ham_define_object_x( \
		2, derived_source, 1, ham_audio_source_vtable, \
		derived_source##_ctor, derived_source##_dtor, \
		( \
			.set_pos =  ham_impl_set_pos_##derived_source, \
			.set_rotation =  ham_impl_set_rotation_##derived_source, \
			.play =  ham_impl_play_##derived_source, \
			.pause =  ham_impl_pause_##derived_source, \
			.stop =  ham_impl_stop_##derived_source, \
		) \
	)

//! @endcond

#define ham_define_audio_context_object(derived_context, derived_listener, derived_source) \
	ham_impl_define_audio_context_object(derived_context, derived_listener, derived_source)

#define ham_define_audio_listener_object(derived_listener) \
	ham_impl_define_audio_listener_object(derived_listener)

#define ham_define_audio_source_object(derived_source) \
	ham_impl_define_audio_source_object(derived_source)

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_AUDIO_OBJECT_H
