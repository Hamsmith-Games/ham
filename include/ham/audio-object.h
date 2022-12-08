#ifndef HAM_AUDIO_OBJECT_H
#define HAM_AUDIO_OBJECT_H 1

/**
 * @defgroup HAM_AUDIO_OBJECT Object definitions
 * @ingroup HAM_AUDIO
 * @{
 */

#include "audio.h"
#include "object.h"
#include "memory.h"

HAM_C_API_BEGIN

typedef struct ham_audio_vtable ham_audio_vtable;
typedef struct ham_audio_stream_vtable ham_audio_stream_vtable;

struct ham_audio{
	ham_derive(ham_object)
	const ham_allocator *allocator;
	ham_vec3 pos, vel;
};

struct ham_audio_stream{
	ham_derive(ham_object)
	ham_audio *ctx;
	const ham_audio_data *data;
};

struct ham_audio_vtable{
	ham_derive(ham_object_vtable)

	const ham_audio_stream_vtable*(*stream_vptr)();

	void(*update)(ham_audio *self, ham_f64 dt);

	ham_vec3(*position)(const ham_audio *self);
	ham_vec3(*rotation)(const ham_audio *self);
	ham_vec3(*velocity)(const ham_audio *self);

	void(*set_position)(ham_audio *self, ham_vec3 new_pos);
	void(*set_rotation)(ham_audio *self, ham_vec3 new_pyr);
	void(*set_velocity)(ham_audio *self, ham_vec3 new_vel);
};

struct ham_audio_stream_vtable{
	ham_derive(ham_object_vtable)

	bool(*set_data)(ham_audio_stream *self, const ham_audio_data *data);

	ham_vec3(*position)(const ham_audio_stream *self);
	ham_vec3(*rotation)(const ham_audio_stream *self);
	ham_vec3(*velocity)(const ham_audio_stream *self);

	void(*set_position)(ham_audio_stream *self, ham_vec3 new_pos);
	void(*set_rotation)(ham_audio_stream *self, ham_vec3 new_pyr);
	void(*set_velocity)(ham_audio_stream *self, ham_vec3 new_vel);

	bool(*play)(ham_audio_stream *self);
	bool(*pause)(ham_audio_stream *self);
	bool(*stop)(ham_audio_stream *self);
};

//! @cond ignore

#define ham_impl_define_audio_object(derived_obj, derived_stream) \
	ham_expose_object_vptr(derived_stream) \
	static inline const ham_audio_stream_vtable *ham_impl_stream_vptr_##derived_obj(){ return (const ham_audio_stream_vtable*)(ham_object_vptr_name(derived_stream)()); } \
	static inline void ham_impl_update_##derived_obj(ham_audio *self, ham_f64 dt){ (derived_obj##_update)((derived_obj*)self, dt); } \
	static inline ham_vec3 ham_impl_position_##derived_obj(const ham_audio *self){ return (derived_obj##_position)((derived_obj*)self); } \
	static inline ham_vec3 ham_impl_rotation_##derived_obj(const ham_audio *self){ return (derived_obj##_rotation)((derived_obj*)self); } \
	static inline ham_vec3 ham_impl_velocity_##derived_obj(const ham_audio *self){ return (derived_obj##_velocity)((derived_obj*)self); } \
	static inline void ham_impl_set_position_##derived_obj(ham_audio *self, ham_vec3 new_pos){ (derived_obj##_set_position)((derived_obj*)self, new_pos); } \
	static inline void ham_impl_set_rotation_##derived_obj(ham_audio *self, ham_vec3 new_pyr){ (derived_obj##_set_rotation)((derived_obj*)self, new_pyr); } \
	static inline void ham_impl_set_velocity_##derived_obj(ham_audio *self, ham_vec3 new_vel){ (derived_obj##_set_velocity)((derived_obj*)self, new_vel); } \
	ham_define_object_x( \
		2, derived_obj, 1, ham_audio_vtable, \
		derived_obj##_ctor, derived_obj##_dtor, \
		( \
			.stream_vptr   = ham_impl_stream_vptr_##derived_obj, \
			.update = ham_impl_update_##derived_obj, \
			.position = ham_impl_position_##derived_obj, \
			.rotation = ham_impl_rotation_##derived_obj, \
			.velocity = ham_impl_velocity_##derived_obj, \
			.set_position = ham_impl_set_position_##derived_obj, \
			.set_rotation = ham_impl_set_rotation_##derived_obj, \
			.set_velocity = ham_impl_set_velocity_##derived_obj, \
		) \
	)

#define ham_impl_define_audio_stream_object(derived_stream) \
	static inline ham_vec3 ham_impl_position_##derived_stream(const ham_audio_stream *self){ return (derived_stream##_position)((derived_stream*)self); } \
	static inline ham_vec3 ham_impl_rotation_##derived_stream(const ham_audio_stream *self){ return (derived_stream##_rotation)((derived_stream*)self); } \
	static inline ham_vec3 ham_impl_velocity_##derived_stream(const ham_audio_stream *self){ return (derived_stream##_velocity)((derived_stream*)self); } \
	static inline bool ham_impl_set_data_##derived_stream(ham_audio_stream *self, const ham_audio_data *data){ return (derived_stream##_set_data)((derived_stream*)self, data); } \
	static inline void ham_impl_set_position_##derived_stream(ham_audio_stream *self, ham_vec3 new_pos){ (derived_stream##_set_position)((derived_stream*)self, new_pos); } \
	static inline void ham_impl_set_rotation_##derived_stream(ham_audio_stream *self, ham_vec3 new_pyr){ (derived_stream##_set_rotation)((derived_stream*)self, new_pyr); } \
	static inline void ham_impl_set_velocity_##derived_stream(ham_audio_stream *self, ham_vec3 new_vel){ (derived_stream##_set_velocity)((derived_stream*)self, new_vel); } \
	static inline bool ham_impl_play_##derived_stream(ham_audio_stream *self){ return (derived_stream##_play)((derived_stream*)self); } \
	static inline bool ham_impl_pause_##derived_stream(ham_audio_stream *self){ return (derived_stream##_pause)((derived_stream*)self); } \
	static inline bool ham_impl_stop_##derived_stream(ham_audio_stream *self){ return (derived_stream##_stop)((derived_stream*)self); } \
	ham_define_object_x( \
		2, derived_stream, 1, ham_audio_stream_vtable, \
		derived_stream##_ctor, derived_stream##_dtor, \
		( \
			.set_data = ham_impl_set_data_##derived_stream, \
			.position = ham_impl_position_##derived_stream, \
			.rotation = ham_impl_rotation_##derived_stream, \
			.velocity = ham_impl_velocity_##derived_stream, \
			.set_position = ham_impl_set_position_##derived_stream, \
			.set_rotation = ham_impl_set_rotation_##derived_stream, \
			.set_velocity = ham_impl_set_velocity_##derived_stream, \
			.play  = ham_impl_play_##derived_stream, \
			.pause = ham_impl_pause_##derived_stream, \
			.stop  = ham_impl_stop_##derived_stream, \
		) \
	)

//! @endcond

#define ham_define_audio_object(derived_obj, derived_stream) \
	ham_impl_define_audio_object(derived_obj, derived_stream)

#define ham_define_audio_stream_object(derived_stream) \
	ham_impl_define_audio_stream_object(derived_stream)

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_AUDIO_OBJECT_H
