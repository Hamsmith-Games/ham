#include "physics_backend.hpp"

namespace editor = ham::engine::editor;

editor::physics_backend::physics_backend(QObject *parent)
	: QObject(parent)
{
	if(!ham_plugin_find(HAM_PHYSICS_BULLET3_PLUGIN_NAME, ham_plugin_default_path(), &m_plug, &m_dso)){
		constexpr char error_msg[] = "Failed to find plugin '" HAM_PHYSICS_BULLET3_PLUGIN_NAME "'";
		throw std::runtime_error(error_msg);
	}

	const auto phys_vptr = (ham_physics_vptr)ham_plugin_object(m_plug, HAM_LIT_UTF8(HAM_PHYSICS_BULLET3_OBJECT_NAME));
	if(!phys_vptr){
		constexpr char error_msg[] = "Failed to find object '" HAM_PHYSICS_BULLET3_OBJECT_NAME "' in plugin '" HAM_PHYSICS_BULLET3_PLUGIN_NAME "'";
		throw std::runtime_error(error_msg);
	}

	m_handle = ham_physics_create(phys_vptr);
	if(!m_handle){
		constexpr char error_msg[] = "Error in ham_physics_create";
		throw std::runtime_error(error_msg);
	}
}

editor::physics_backend::~physics_backend(){
	m_handle = nullptr;
	ham_plugin_unload(m_plug);
	ham_dso_close(m_dso);
}
