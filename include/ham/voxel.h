#ifndef HAM_VOXEL_H
#define HAM_VOXEL_H 1

/**
 * @defgroup HAM_VOXEL Voxels
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

typedef union alignas(ham_u8) ham_voxel_coord{
	struct {
		ham_u8 padd: 5;
		ham_u8 x: 1;
		ham_u8 y: 1;
		ham_u8 z: 1;
	};

	ham_u8 bits;
} ham_voxel_coord;

static_assert(sizeof(ham_voxel_coord) == sizeof(ham_u8),  "Invalid ham_voxel_coord type");

#define HAM_VOXEL32_NUM_FLAG_BITS 5u
#define HAM_VOXEL32_NUM_DEPTH_BITS 3u
#define HAM_VOXEL32_MAX_DEPTH 8u

#define HAM_VOXEL32_COORD_MASK (HAM_U32_MAX >> (HAM_VOXEL32_NUM_FLAG_BITS + HAM_VOXEL32_NUM_DEPTH_BITS))

typedef union alignas(ham_u32) ham_voxel32{
	struct {
		unsigned int flags: HAM_VOXEL32_NUM_FLAG_BITS;
		unsigned int depth: HAM_VOXEL32_NUM_DEPTH_BITS;

	#define HAM_VOXEL_PACKED_COORD_DEF(n) \
		ham_u8 x##n: 1; \
		ham_u8 y##n: 1; \
		ham_u8 z##n: 1;

		HAM_VOXEL_PACKED_COORD_DEF(0)
		HAM_VOXEL_PACKED_COORD_DEF(1)
		HAM_VOXEL_PACKED_COORD_DEF(2)
		HAM_VOXEL_PACKED_COORD_DEF(3)
		HAM_VOXEL_PACKED_COORD_DEF(4)
		HAM_VOXEL_PACKED_COORD_DEF(5)
		HAM_VOXEL_PACKED_COORD_DEF(6)
		HAM_VOXEL_PACKED_COORD_DEF(7)

	#undef HAM_VOXEL_PACKED_COORD_DEF
	};

	ham_u32 bits;
} ham_voxel32;

static_assert(HAM_VOXEL32_NUM_FLAG_BITS + HAM_VOXEL32_NUM_DEPTH_BITS + (HAM_VOXEL32_MAX_DEPTH * 3u) == 32u, "Invalid ham_voxel32 configuration");
static_assert(sizeof(ham_voxel32) == sizeof(ham_u32), "Invalid ham_voxel32 type");

#define HAM_VOXEL64_NUM_FLAG_BITS 21u
#define HAM_VOXEL64_NUM_DEPTH_BITS 4u
#define HAM_VOXEL64_MAX_DEPTH 13u

#define HAM_VOXEL64_COORD_MASK (HAM_U64_MAX >> (HAM_VOXEL64_NUM_FLAG_BITS + HAM_VOXEL64_NUM_DEPTH_BITS))

typedef union alignas(ham_u64) ham_voxel64{
	struct {
		unsigned int flags: HAM_VOXEL64_NUM_FLAG_BITS;
		unsigned int depth: HAM_VOXEL64_NUM_DEPTH_BITS;

	#define HAM_VOXEL_PACKED_COORD_DEF(n) \
		ham_u8 x##n: 1; \
		ham_u8 y##n: 1; \
		ham_u8 z##n: 1;

		HAM_VOXEL_PACKED_COORD_DEF(0)
		HAM_VOXEL_PACKED_COORD_DEF(1)
		HAM_VOXEL_PACKED_COORD_DEF(2)
		HAM_VOXEL_PACKED_COORD_DEF(3)
		HAM_VOXEL_PACKED_COORD_DEF(4)
		HAM_VOXEL_PACKED_COORD_DEF(5)
		HAM_VOXEL_PACKED_COORD_DEF(6)
		HAM_VOXEL_PACKED_COORD_DEF(7)
		HAM_VOXEL_PACKED_COORD_DEF(8)
		HAM_VOXEL_PACKED_COORD_DEF(9)
		HAM_VOXEL_PACKED_COORD_DEF(10)
		HAM_VOXEL_PACKED_COORD_DEF(11)
		HAM_VOXEL_PACKED_COORD_DEF(12)

	#undef HAM_VOXEL_PACKED_COORD_DEF
	};

	ham_u64 bits;
} ham_voxel64;

static_assert(HAM_VOXEL64_NUM_FLAG_BITS + HAM_VOXEL64_NUM_DEPTH_BITS + (HAM_VOXEL64_MAX_DEPTH * 3u) == 64u, "Invalid ham_voxel64 configuration");
static_assert(sizeof(ham_voxel64) == sizeof(ham_u64), "Invalid ham_voxel64 type");

#define HAM_VOXEL_COORD_MASK 0b111u
#define HAM_VOXEL_COORD_SHIFT 3u

// 32-bit voxel helpers

ham_constexpr ham_nothrow static inline ham_u32 ham_voxel32_coord_bits(ham_voxel32 vox){
	return vox.bits & HAM_VOXEL32_COORD_MASK;
}

ham_constexpr ham_nothrow static inline ham_voxel_coord ham_voxel32_coord(ham_voxel32 vox, ham_u8 depth){
	return (ham_voxel_coord){ .bits = (ham_u8)((vox.bits >> (HAM_VOXEL_COORD_SHIFT * depth)) & HAM_VOXEL_COORD_MASK) };
}

ham_constexpr ham_nothrow static inline ham_f64 ham_voxel32_min_length(ham_f64 unit_length){
	return unit_length / ham_f64(1u << (HAM_VOXEL32_MAX_DEPTH + 1u));
}

// 64-bit voxel helpers

ham_constexpr ham_nothrow static inline ham_u64 ham_voxel64_coord_bits(ham_voxel64 vox){
	return vox.bits & HAM_VOXEL64_COORD_MASK;
}

ham_constexpr ham_nothrow static inline ham_voxel_coord ham_voxel64_coord(ham_voxel64 vox, ham_u8 depth){
	return (ham_voxel_coord){ .bits = (ham_u8)((vox.bits >> (HAM_VOXEL_COORD_SHIFT * depth)) & HAM_VOXEL_COORD_MASK) };
}

ham_constexpr ham_nothrow static inline ham_f64 ham_voxel64_min_length(ham_f64 unit_length){
	return unit_length / ham_f64(2u << HAM_VOXEL64_MAX_DEPTH);
}

// Generic voxel helper wrappers

#ifdef __cplusplus

HAM_C_API_END

template<typename Voxel>
static inline auto ham_voxel_coord_bits(Voxel vox){
	if constexpr(std::is_same_v<Voxel, ham_voxel32>){
		return ham_voxel32_coord_bits(vox);
	}
	else{
		return ham_voxel64_coord_bits(vox);
	}
}

template<typename Voxel>
static inline auto ham_voxel_get_coord(Voxel vox, ham_u8 depth){
	if constexpr(std::is_same_v<Voxel, ham_voxel32>){
		return ham_voxel32_coord(vox, depth);
	}
	else{
		return ham_voxel64_coord(vox, depth);
	}
}

HAM_C_API_BEGIN

#else // __GNUC__

#	define ham_voxel_coord_bits(vox) \
		(_Generic(ham_typeof(vox) \
			, ham_voxel32: ham_voxel32_coord_bits \
			, ham_voxel64: ham_voxel64_coord_bits\
		)((vox)))

#	define ham_voxel_get_coord(vox, depth) \
		(_Generic(ham_typeof(vox) \
			, ham_voxel32: ham_voxel32_coord \
			, ham_voxel64: ham_voxel64_coord\
		)((vox)))

#endif

HAM_C_API_END

namespace ham{
	namespace detail{
		template<usize NumFlagBits, usize NumDepthBits, usize MaxDepth, typename = void>
		union basic_voxel_data{
			constexpr static bool is_packed = false;

			constexpr static usize num_used_bits    = NumFlagBits + NumDepthBits + (MaxDepth * 3UL);
			constexpr static usize nearest_bitwidth = std::bit_ceil(num_used_bits);
			constexpr static usize num_excess_bits  = nearest_bitwidth - num_used_bits;

			using value_type = meta::nearest_nat_t<nearest_bitwidth>;

			struct {
				unsigned long excess: num_excess_bits;
				unsigned long flags: NumFlagBits;
				unsigned long depth: NumDepthBits;
				unsigned long coords: MaxDepth * 3u;
			};

			value_type bits;
		};

		template<usize NumFlagBits, usize NumDepthBits, usize MaxDepth>
		union basic_voxel_data<NumFlagBits, NumDepthBits, MaxDepth, std::enable_if_t<std::popcount(NumFlagBits + NumDepthBits + (MaxDepth * 3UL)) == 1>>{
			constexpr static bool is_packed = true;

			constexpr static usize num_used_bits    = NumFlagBits + NumDepthBits + (MaxDepth * 3UL);
			constexpr static usize nearest_bitwidth = num_used_bits;
			constexpr static usize num_excess_bits  = 0;

			using value_type = meta::nearest_nat_t<num_used_bits>;

			struct {
				unsigned long flags: NumFlagBits;
				unsigned long depth: NumDepthBits;
				unsigned long coords: MaxDepth * 3u;
			};

			value_type bits;
		};
	}

	class voxel_coord{
		public:
			constexpr voxel_coord(const ham_voxel_coord &coord_ = {}) noexcept
				: m_coord(coord_){}

			constexpr voxel_coord(const voxel_coord&) noexcept = default;

			constexpr voxel_coord &operator=(const voxel_coord&) noexcept = default;

			constexpr voxel_coord &operator=(const ham_voxel_coord &coord_) noexcept{
				m_coord = coord_;
				return *this;
			}

			constexpr bool is_valid() const noexcept{ return m_coord.padd == 0; }

			constexpr bool x() const noexcept{ return m_coord.x; }
			constexpr bool y() const noexcept{ return m_coord.y; }
			constexpr bool z() const noexcept{ return m_coord.z; }

			constexpr void set_x(bool new_x) noexcept{ m_coord.x = new_x; }
			constexpr void set_y(bool new_y) noexcept{ m_coord.y = new_y; }
			constexpr void set_z(bool new_z) noexcept{ m_coord.z = new_z; }

			constexpr u8 bits() const noexcept{ return m_coord.bits; }

		private:
			ham_voxel_coord m_coord;
	};

	template<usize NumFlagBits, usize MaxDepth, usize NumDepthBits = std::popcount(MaxDepth)>
	class basic_voxel{
		public:
			static_assert((1UL << (NumDepthBits + 1)) > MaxDepth, "Invalid voxel configuration");

			using data_type = detail::basic_voxel_data<NumFlagBits, NumDepthBits, MaxDepth>;

			constexpr static usize num_used_bits    = data_type::num_used_bits;
			constexpr static usize nearest_bitwidth = data_type::nearest_bitwidth;

			using flags_type  = meta::nearest_nat_t<NumFlagBits>;
			using depth_type  = meta::nearest_nat_t<NumDepthBits>;
			using coords_type = meta::nearest_nat_t<MaxDepth * 3UL>;
			using packed_type = typename data_type::value_type;

			constexpr static bool is_packed = data_type::is_packed;

			constexpr static bool is_voxel32 =
				(NumFlagBits == HAM_VOXEL32_NUM_FLAG_BITS) &&
				(NumDepthBits == HAM_VOXEL32_NUM_DEPTH_BITS) &&
				(MaxDepth == HAM_VOXEL32_MAX_DEPTH)
			;

			constexpr static bool is_voxel64 =
				(NumFlagBits == HAM_VOXEL64_NUM_FLAG_BITS) &&
				(NumDepthBits == HAM_VOXEL64_NUM_DEPTH_BITS) &&
				(MaxDepth == HAM_VOXEL64_MAX_DEPTH)
			;

			template<
				bool IsVoxel32 = is_voxel32,
				std::enable_if_t<IsVoxel32, int> = 0
			>
			constexpr operator ham_voxel32() const noexcept{ return (ham_voxel32){ .bits = m_data.bits }; }

			template<
				bool IsVoxel64 = is_voxel64,
				std::enable_if_t<IsVoxel64, int> = 0
			>
			constexpr operator ham_voxel64() const noexcept{ return (ham_voxel64){ .bits = m_data.bits }; }

			template<
				bool HasExcess = !is_packed,
				std::enable_if_t<HasExcess, int> = 0
			>
			constexpr auto excess() const noexcept -> meta::nearest_nat_t<data_type::num_excess_bits>{ return m_data.excess; }

			constexpr flags_type flags() const noexcept{ return m_data.flags; }
			constexpr depth_type depth() const noexcept{ return m_data.depth; }
			constexpr coords_type coord_bits() const noexcept{ return m_data.coords; }

			constexpr void set_flags(flags_type new_flags) noexcept{ m_data.flags = new_flags; }
			constexpr void set_depth(depth_type new_depth) noexcept{ m_data.depth = new_depth; }
			constexpr void set_coord_bits(coords_type new_coords) noexcept{ m_data.coords = new_coords; }

			constexpr voxel_coord coord(u32 depth) const noexcept{
				return (m_data.coords >> depth) & 0b111;
			}

			constexpr void set_coord(u32 depth, const voxel_coord &coord){
				const coords_type old_coord_mask = 0b111 << (depth * 3UL);
				const coords_type new_coord_bits = (coord.bits() & 0b111) << (depth * 3UL);
				m_data.coords &= ~old_coord_mask;
				m_data.coords |= new_coord_bits;
			}

		private:
			data_type m_data;
	};

	namespace detail{
		template<typename Voxel>
		struct min_voxel_length;

		template<usize NumFlagBits, usize MaxDepth, usize NumDepthBits>
		struct min_voxel_length<basic_voxel<NumFlagBits, MaxDepth, NumDepthBits>>{
			constexpr static f64 value(f64 base_length) noexcept{
				return base_length / (2UL << MaxDepth);
			}
		};

		template<>
		struct min_voxel_length<ham_voxel32>{
			constexpr static f64 value(f64 base_length) noexcept{ return ham_voxel32_min_length(base_length); }
		};

		template<>
		struct min_voxel_length<ham_voxel64>{
			constexpr static f64 value(f64 base_length) noexcept{ return ham_voxel64_min_length(base_length); }
		};
	}

	template<typename Voxel>
	constexpr static inline usize min_voxel_length(f64 base_length = 1.0) noexcept{
		return detail::min_voxel_length<std::remove_cvref_t<Voxel>>::value(base_length);
	}
}

/**
 * @}
 */

#endif // !HAM_VOXEL_H
