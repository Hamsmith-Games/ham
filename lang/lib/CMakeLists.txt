ham_add_library(
	ham-lang
	bytecode.cpp
)

add_library(ham::lang ALIAS ham-lang)
