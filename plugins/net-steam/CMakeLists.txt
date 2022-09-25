ham_add_library(ham-net-steam SHARED net-steam.cpp)

target_link_libraries(ham-net-steam PRIVATE Steamworks::Steamworks)
