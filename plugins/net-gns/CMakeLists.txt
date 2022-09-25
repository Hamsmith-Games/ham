ham_add_library(ham-net-gns SHARED net-gns.cpp)

target_link_libraries(ham-net-gns PRIVATE GameNetworkingSockets::shared)
