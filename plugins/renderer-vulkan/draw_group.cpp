/*
 * Ham Runtime Plugins
 * Copyright (C) 2022  Hamsmith Ltd.
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

#include "renderer.hpp"

HAM_C_API_BEGIN

ham_draw_group_vulkan *ham_draw_group_vulkan_ctor(ham_draw_group_vulkan *mem, ham_u32 nargs, va_list va){
	if(nargs != 2){
		ham::logapierror("Wrong number of args {}, expected 2 (ham_usize num_shapes, const ham_shape *const *shapes)");
		return nullptr;
	}

	const ham_usize num_shapes = va_arg(va, ham_usize);
	const ham_shape *const *const shapes = va_arg(va, const ham_shape* const*);

	const auto r = (ham_renderer_vulkan*)ham_super(mem)->r;
	const auto group = new(mem) ham_draw_group_vulkan;

	ham_usize total_num_points = 0, total_num_indices = 0;

	for(ham_usize i = 0; i < num_shapes; i++){
		total_num_points  += ham_super(group)->num_shape_points[i];
		total_num_indices += ham_super(group)->num_shape_indices[i];
	}

	const ham_usize total_vbo_size = total_num_points  * (sizeof(ham_vec3) + sizeof(ham_vec3) + sizeof(ham_vec2));
	const ham_usize total_ibo_size = total_num_indices * sizeof(ham_u32);

	VmaAllocationCreateInfo alloc_create_info = {
		.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.preferredFlags = 0,
		.memoryTypeBits = 0,
		.pool = 0,
		.pUserData = nullptr,
		.priority = 1.f
	};

	VkBufferCreateInfo buffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
	};

	buffer_create_info.size  = total_vbo_size;
	buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	ham::vk::result res = vmaCreateBuffer(r->vma_allocator, &buffer_create_info, &alloc_create_info, &group->vbo, &group->vbo_alloc, nullptr);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vmaCreateBuffer: %s", res.to_str());
		std::destroy_at(group);
		return nullptr;
	}

	buffer_create_info.size  = total_ibo_size;
	buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	res = vmaCreateBuffer(r->vma_allocator, &buffer_create_info, &alloc_create_info, &group->ibo, &group->ibo_alloc, nullptr);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vmaCreateBuffer: %s", res.to_str());
		vmaDestroyBuffer(r->vma_allocator, group->vbo, group->vbo_alloc);
		std::destroy_at(group);
		return nullptr;
	}

	buffer_create_info.size  = num_shapes * sizeof(VkDrawIndexedIndirectCommand);
	buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

	res = vmaCreateBuffer(r->vma_allocator, &buffer_create_info, &alloc_create_info, &group->cbo, &group->cbo_alloc, nullptr);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vmaCreateBuffer: %s", res.to_str());
		vmaDestroyBuffer(r->vma_allocator, group->ibo, group->vbo_alloc);
		vmaDestroyBuffer(r->vma_allocator, group->vbo, group->vbo_alloc);
		std::destroy_at(group);
		return nullptr;
	}

	void *vbo_mem, *ibo_mem, *cbo_mem;
	vmaMapMemory(r->vma_allocator, group->vbo_alloc, &vbo_mem);
	vmaMapMemory(r->vma_allocator, group->ibo_alloc, &ibo_mem);
	vmaMapMemory(r->vma_allocator, group->cbo_alloc, &cbo_mem);

	VkDrawIndexedIndirectCommand draw_cmd;

	ham::std_vector<VkVertexInputBindingDescription> bindings;

	ham_i32 vert_off = 0, vert_idx_off = 0;

	ham_usize vbo_off = 0, ibo_off = 0;
	for(ham_usize i = 0; i < num_shapes; i++){
		const auto shape = shapes[i];

		const auto num_points  = ham_super(group)->num_shape_points[i];
		const auto num_indices = ham_super(group)->num_shape_indices[i];

		const auto indices_size = num_indices * sizeof(ham_u32);

		const auto verts   = ham_shape_vertices(shape);
		const auto norms   = ham_shape_normals(shape);
		const auto uvs     = ham_shape_uvs(shape);
		const auto indices = ham_shape_indices(shape);

		draw_cmd.indexCount    = num_indices;
		draw_cmd.instanceCount = 1;
		draw_cmd.firstIndex    = ibo_off / sizeof(ham_u32);
		draw_cmd.vertexOffset  = vert_idx_off;
		draw_cmd.firstInstance = 0;

		// write interlaced points: (pos, norm, uv)+
		for(ham_usize i = 0; i < num_points; i++){
			memcpy((char*)vbo_mem + vbo_off, verts + i, sizeof(ham_vec3));
			vbo_off += sizeof(ham_vec3);

			memcpy((char*)vbo_mem + vbo_off, norms + i, sizeof(ham_vec3));
			vbo_off += sizeof(ham_vec3);

			memcpy((char*)vbo_mem + vbo_off, uvs + i, sizeof(ham_vec2));
			vbo_off += sizeof(ham_vec2);
		}

		memcpy((char*)ibo_mem + ibo_off, indices, indices_size);
		ibo_off += indices_size;

		memcpy((char*)cbo_mem + (i * sizeof(VkDrawIndexedIndirectCommand)), &draw_cmd, sizeof(VkDrawIndexedIndirectCommand));

		vert_idx_off += num_points;
	}

	vmaUnmapMemory(r->vma_allocator, group->vbo_alloc);
	vmaUnmapMemory(r->vma_allocator, group->ibo_alloc);
	vmaUnmapMemory(r->vma_allocator, group->cbo_alloc);

	return group;
}

void ham_draw_group_vulkan_dtor(ham_draw_group_vulkan *group){
	const auto r = (ham_renderer_vulkan*)ham_super(group)->r;

	vmaDestroyBuffer(r->vma_allocator, group->cbo, group->cbo_alloc);
	vmaDestroyBuffer(r->vma_allocator, group->ibo, group->ibo_alloc);
	vmaDestroyBuffer(r->vma_allocator, group->vbo, group->vbo_alloc);

	std::destroy_at(group);
}

ham_define_draw_group(ham_draw_group_vulkan)

HAM_C_API_END
