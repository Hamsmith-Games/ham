#ifndef HAM_SETTINGS_H
#define HAM_SETTINGS_H 1

/**
 * @defgroup HAM_SETTINGS Settings/Configuration storage
 * @ingroup HAM
 * @{
 */

#include "check.h"

HAM_C_API_BEGIN

typedef struct ham_settings ham_settings;
typedef struct ham_settings_value ham_settings_value;

typedef enum ham_settings_value_type{
	HAM_SETTINGS_VALUE_BOOL,
	HAM_SETTINGS_VALUE_NAT,
	HAM_SETTINGS_VALUE_INT,
	HAM_SETTINGS_VALUE_FLOAT,
	HAM_SETTINGS_VALUE_STR,

	HAM_SETTINGS_VALUE_TYPE_COUNT,
} ham_settings_value_type;

ham_api ham_settings *ham_settings_create(ham_usize num_vals, const ham_settings_value *vals);
ham_api void ham_settings_destroy(ham_settings *settings);

ham_api const ham_settings_value *ham_settings_get(ham_settings *settings, ham_str8 key);

ham_api const ham_settings_value *ham_settings_set(ham_settings *settings, ham_str8 key, ham_settings_value_type type, void *value_ptr);

typedef struct ham_settings_value{
	ham_str8 key;
	ham_settings_value_type type;
	void *value;
} ham_settings_value;

//! @cond ignore

#ifdef __GNUC__
#	define ham_impl_settings_value_extract(val_, type_) \
		({	static_assert(sizeof(type_) <= sizeof(void*), ""); \
			type_ ret; \
			memcpy(&ret, &(val_)->value, sizeof(type_)); \
			ret; \
		})
#else // __cplusplus
#	define ham_impl_settings_value_extract(val_, type_) \
		([](const auto val__){ \
			static_assert(sizeof(type_) <= sizeof(void*), ""); \
			type_ ret; \
			memcpy(&ret, &(val_)->value, sizeof(type_)); \
			return ret; \
		}((val_)))
#endif

//! @endcond

ham_nonnull_args(1)
static inline void ham_settings_value_init_bool(ham_settings_value *ret, ham_str8 key, bool val){
	ret->key  = key;
	ret->type = HAM_SETTINGS_VALUE_BOOL;
	memcpy(&ret->value, &val, sizeof(bool));
}

ham_nonnull_args(1)
static inline void ham_settings_value_init_nat(ham_settings_value *ret, ham_str8 key, ham_uptr val){
	ret->key  = key;
	ret->type = HAM_SETTINGS_VALUE_NAT;
	memcpy(&ret->value, &val, sizeof(ham_uptr));
}

ham_nonnull_args(1)
static inline void ham_settings_value_init_int(ham_settings_value *ret, ham_str8 key, ham_iptr val){
	ret->key  = key;
	ret->type = HAM_SETTINGS_VALUE_INT;
	memcpy(&ret->value, &val, sizeof(ham_iptr));
}

ham_nonnull_args(1)
static inline void ham_settings_value_init_float(ham_settings_value *ret, ham_str8 key, ham_f64 val){
	ret->key  = key;
	ret->type = HAM_SETTINGS_VALUE_FLOAT;
	memcpy(&ret->value, &val, sizeof(ham_f64));
}

ham_nonnull_args(1)
static inline void ham_settings_value_init_str(ham_settings_value *ret, ham_str8 key, const char *val){
	ret->key  = key;
	ret->type = HAM_SETTINGS_VALUE_STR;
	memcpy(&ret->value, &val, sizeof(const char*));
}

static inline bool ham_settings_value_bool(const ham_settings_value *val){
	if(!ham_check(val != NULL) || !ham_check(val->type == HAM_SETTINGS_VALUE_BOOL)){
		return false;
	}

	return ham_impl_settings_value_extract(val, bool);
}

static inline ham_uptr ham_settings_value_nat(const ham_settings_value *val){
	if(!ham_check(val != NULL) || !ham_check(val->type == HAM_SETTINGS_VALUE_NAT)){
		return 0;
	}

	return ham_impl_settings_value_extract(val, ham_uptr);
}

static inline ham_iptr ham_settings_value_int(const ham_settings_value *val){
	if(!ham_check(val != NULL) || !ham_check(val->type == HAM_SETTINGS_VALUE_INT)){
		return 0;
	}

	return ham_impl_settings_value_extract(val, ham_iptr);
}

static inline ham_f64 ham_settings_value_float(const ham_settings_value *val){
	if(!ham_check(val != NULL) || !ham_check(val->type == HAM_SETTINGS_VALUE_FLOAT)){
		return 0.0;
	}

	return ham_impl_settings_value_extract(val, ham_f64);
}

static inline ham_str8 ham_settings_value_str(const ham_settings_value *val){
	if(!ham_check(val != NULL) || !ham_check(val->type == HAM_SETTINGS_VALUE_STR)){
		return HAM_EMPTY_STR8;
	}

	const char *const c_str = ham_impl_settings_value_extract(val, const char*);

	return ham_str8{ c_str, strlen(c_str) };
}

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_SETTINGS_H
