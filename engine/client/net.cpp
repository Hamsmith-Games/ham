#include "client.h"

#include "ham/log.h"

using namespace ham::typedefs;

namespace engine = ham::engine;

bool engine::client_net_subsystem::init(ham_engine *engine){
	set_min_dt(1.f/60.f);

	m_net = ham_net_create(HAM_NET_STEAMWORKS_PLUGIN_NAME, HAM_NET_STEAMWORKS_OBJECT_NAME);
	if(!m_net){
		ham::logapierror("Error creating ham_net");
		return false;
	}

	m_serv_sock = ham_net_socket_create(m_net, 1476, nullptr, nullptr, nullptr, nullptr);
	if(!m_serv_sock){
		ham::logapierror("Error create listen ham_net_socket");
		ham_net_destroy(m_net);
		m_net = nullptr;
		return false;
	}

	ham::logapiverbose("Net subsystem initialized");
	return true;
}

void engine::client_net_subsystem::fini(ham_engine *engine){
	(void)engine;

	ham_net_socket_destroy(m_serv_sock);
	ham_net_destroy(m_net);

	ham::logapiverbose("Net subsystem finished");
}

void engine::client_net_subsystem::loop(ham_engine *engine, f64 dt){
	(void)engine;

	ham_net_loop(m_net, dt);
}
