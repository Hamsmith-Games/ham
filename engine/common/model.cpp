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

#include "ham/engine/model.h"
#include "ham/image.h"

#include "ham/check.h"
#include "ham/fs.h"
#include "ham/buffer.h"

#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"

#include "robin_hood.h"

using namespace ham::typedefs;

HAM_C_API_BEGIN

struct ham_model{
	ham::basic_buffer<ham_shape*> shapes;
	ham::basic_buffer<ham_image*> images;
};

ham_model *ham_model_load_from_mem(ham_usize len, const void *data){
	if(!ham_check(len > 0) || !ham_check(data != NULL)){
		return nullptr;
	}

	Assimp::Importer importer;

	const ham::str8 mdl_mime = ham_mime_from_mem(len, data);

	constexpr ham::str8 glb_prefix = HAM_MIME_TYPE_GLB;

	const char *file_hint = nullptr;

	if(mdl_mime.substr(0, glb_prefix.size()) == glb_prefix){
		file_hint = "glb";
	}

	const auto scene = importer.ReadFileFromMemory(
		data, len,
		aiProcess_MakeLeftHanded |
		aiProcessPreset_TargetRealtime_Fast,
		file_hint
	);

	if(!scene){
		ham::logapierror("Error in Assimp::Importer::ReadFile: {}", importer.GetErrorString());
		return nullptr;
	}

	ham::basic_buffer<ham_shape*> shapes;
	ham::basic_buffer<ham_image*> images;

	struct mesh_data{
		ham::basic_buffer<ham_vec3> verts;
		ham::basic_buffer<ham_vec3> norms;
		ham::basic_buffer<ham_vec2> uvs;
		ham::basic_buffer<ham_vec4i> bone_indices;
		ham::basic_buffer<ham_vec4> bone_weights;
		ham::basic_buffer<ham_u32> indices;
		ham::basic_buffer<ham_shape_bone> bones;

		ham::basic_buffer<ham_u32> bone_index_counters;
	};

	robin_hood::unordered_flat_map<aiMesh*, mesh_data> shape_data_map;

	shapes.resize(scene->mNumMeshes);
	images.resize(scene->mNumTextures);

	memset(shapes.data(), 0, sizeof(void*) * scene->mNumMeshes);
	memset(images.data(), 0, sizeof(void*) * scene->mNumTextures);

	for(unsigned int mesh_idx = 0; mesh_idx < scene->mNumMeshes; mesh_idx++){
		const auto mesh = scene->mMeshes[mesh_idx];

		const auto num_points  = mesh->mNumVertices;
		const auto num_indices = mesh->mNumFaces * 3u;

		ham::basic_buffer<ham_vec3> verts, norms;
		ham::basic_buffer<ham_vec2> uvs;
		ham::basic_buffer<ham_u32> indices;
		ham::basic_buffer<ham_u32> bone_index_counters;

		verts.resize(num_points);
		norms.resize(num_points);
		uvs.resize(num_points);
		indices.resize(num_indices);
		bone_index_counters.resize(num_points);

		memset(bone_index_counters.data(), 0, num_points * sizeof(ham_u32));

		for(unsigned int point_idx = 0; point_idx < mesh->mNumVertices; point_idx++){
			const auto vert = mesh->mVertices + point_idx;
			const auto norm = mesh->mNormals + point_idx;
			const auto uv = mesh->mTextureCoords[0] + point_idx;

			verts[point_idx] = ham_vec3{vert->x, vert->y, vert->z};
			norms[point_idx] = ham_vec3{norm->x, norm->y, norm->z};
			uvs[point_idx]   = ham_vec2{uv->x, uv->y};
		}

		for(unsigned int face_idx = 0; face_idx < mesh->mNumFaces; face_idx++){
			const auto face = mesh->mFaces + face_idx;

			for(unsigned int k = 0; k < 3; k++){
				indices[(face_idx * 3) + k] = face->mIndices[k];
			}
		}

		auto &shape_data = shape_data_map[mesh];
		shape_data.verts = std::move(verts);
		shape_data.norms = std::move(norms);
		shape_data.uvs = std::move(uvs);
		shape_data.indices = std::move(indices);
		shape_data.bone_index_counters = std::move(bone_index_counters);
	}

	const auto allocator = ham_current_allocator();

	for(unsigned int skel_i = 0; skel_i < scene->mNumSkeletons; skel_i++){
		const auto ai_skel = scene->mSkeletons[skel_i];

		for(unsigned int bone_i = 0; bone_i < ai_skel->mNumBones; bone_i++){
			const auto ai_bone = ai_skel->mBones[bone_i];

			auto &shape_data = shape_data_map[ai_bone->mMeshId];

			const ham_u32 bone_idx = shape_data.bones.size();
			ham_shape_bone &bone = shape_data.bones.emplace_back();

			// TODO: set bone matrix to good value

			bone.transform = ham_mat4_identity();

			for(unsigned int weight_i = 0; weight_i < ai_bone->mNumnWeights; weight_i++){
				const auto ai_weight = ai_bone->mWeights + weight_i;

				ham_u32 &vert_bone_counter = shape_data.bone_index_counters[ai_weight->mVertexId];
				if(vert_bone_counter >= 4){
					ham::logapiwarn("Skipping bone '{}' for vertex {}", ai_bone->mNode->mName.C_Str(), ai_weight->mVertexId);
					continue;
				}

				shape_data.bone_indices[ai_weight->mVertexId].data[vert_bone_counter] = bone_idx;
				shape_data.bone_weights[ai_weight->mVertexId].data[vert_bone_counter] = f32(ai_weight->mWeight);

				++vert_bone_counter;
			}
		}
	}

	ham_u32 shape_counter = 0;
	for(auto &&shape_data_p : shape_data_map){
		const auto &shape_data = shape_data_p.second;
		shapes[shape_counter] = ham_shape_create_triangle_mesh(
			(ham_u32)shape_data.verts.size(),
			shape_data.verts.data(),
			shape_data.norms.data(),
			shape_data.uvs.data(),
			shape_data.bone_indices.data(),
			shape_data.bone_weights.data(),
			(ham_u32)shape_data.indices.size(),
			shape_data.indices.data(),
			(ham_u32)shape_data.bones.size(),
			shape_data.bones.data()
		);
		++shape_counter;
	}

	// TODO: load from central resource manager when not embedded/filename given

	for(unsigned int img_idx = 0; img_idx < scene->mNumTextures; img_idx++){
		const auto tex = scene->mTextures[img_idx];

		// TODO: default texture for when a texture format is unrecognized
		ham_image *img = nullptr;

		if(tex->mHeight == 0){
			img = ham_image_load_from_mem(HAM_RGBA8U, tex->mWidth, tex->pcData);
			if(!img){
				ham_logapiwarnf("Unrecognized compressed embedded texture format");
				continue;
			}
		}
		else{
			u32 num_comps;
			ham_color_format format;

			if(tex->CheckFormat("r8")){
				num_comps = 1;
				format = HAM_R8U;
			}
			if(tex->CheckFormat("rg88")){
				num_comps = 2;
				format = HAM_RG8U;
			}
			else if(tex->CheckFormat("rgb888")){
				num_comps = 3;
				format = HAM_RGB8U;
			}
			else if(tex->CheckFormat("rgba8888")){
				num_comps = 4;
				format = HAM_RGBA8U;
			}
			else{
				ham_logapiwarnf("Unrecognized embedded texture format");
				continue;
			}

			ham::basic_buffer<std::byte> bytes;
			bytes.resize(tex->mWidth * tex->mHeight * num_comps);

			for(u32 y = 0; y < tex->mHeight; y++){
				for(u32 x = 0; x < tex->mWidth; x++){
					const auto idx = ((y * tex->mWidth) + x) * num_comps;
					const auto ai_pix = tex->pcData + idx;

					ham_u8 pix[4] = { ai_pix->r, ai_pix->g, ai_pix->b, ai_pix->a };

					memcpy(bytes.data() + idx, pix, num_comps);
				}
			}

			img = ham_image_create(format, tex->mWidth, tex->mHeight, bytes.data());
			if(!img){
				ham_logapiwarnf("Error in ham_image_create");
				continue;
			}
		}

		images[img_idx] = img;
	}

	const auto mdl = ham_allocator_new(allocator, ham_model);
	if(!mdl) return nullptr;

	mdl->shapes = std::move(shapes);
	mdl->images = std::move(images);

	return mdl;
}

ham_model *ham_model_load(ham_str8 filepath){
	if(
	   !ham_check(filepath.len && filepath.ptr) ||
	   !ham_check(filepath.len < HAM_PATH_BUFFER_SIZE)
	){
		return nullptr;
	}

	const auto file = ham_file_open_utf8(filepath, HAM_OPEN_READ);
	if(!file){
		ham::logapierror("Error in ham_file_open_utf8");
		return nullptr;
	}

	ham_file_info file_info;
	if(!ham_file_get_info(file, &file_info)){
		ham::logapierror("Error in ham_file_get_info");
		ham_file_close(file);
		return nullptr;
	}

	const auto mapping = ham_file_map(file, HAM_OPEN_READ, 0, file_info.size);
	if(!mapping){
		ham::logapierror("Error in ham_file_map");
		ham_file_close(file);
		return nullptr;
	}

	const auto ret = ham_model_load_from_mem(file_info.size, mapping);

	if(!ham_file_unmap(file, mapping, file_info.size)){
		ham::logapiwarn("Error in ham_file_unmap");
	}

	ham_file_close(file);

	return ret;
}

ham_nothrow void ham_model_destroy(ham_model *mdl){
	if(ham_unlikely(!mdl)) return;

	const auto allocator = mdl->shapes.allocator();

	for(auto shape : mdl->shapes){
		ham_shape_destroy(shape);
	}

	for(auto img : mdl->images){
		ham_image_destroy(img);
	}

	ham_allocator_delete(allocator, mdl);
}

void ham_model_fill_blank_images(ham_model *mdl, const ham_image *img){
	if(!ham_check(mdl != NULL) || !ham_check(img != NULL)) return;

	for(auto &img_ptr : mdl->images){
		if(img_ptr) continue;
		img_ptr = ham_image_create_view(img);
	}
}

ham_nothrow ham_usize ham_model_num_shapes(const ham_model *mdl){
	if(!ham_check(mdl != NULL)) return (ham_usize)-1;
	return mdl->shapes.size();
}

ham_nothrow const ham_shape *const *ham_model_shapes(const ham_model *mdl){
	if(!ham_check(mdl != NULL)) return nullptr;
	return mdl->shapes.data();
}

ham_nothrow const ham_image *const *ham_model_images(const ham_model *mdl){
	if(!ham_check(mdl != NULL)) return nullptr;
	return mdl->images.data();
}

HAM_C_API_END
