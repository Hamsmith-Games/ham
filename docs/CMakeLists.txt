#
# Documentation
#

set(HAM_DOCS_COLOR_TEXT "white" CACHE STRING "Color of regular text throughout the documentation")

set(HAM_DOCS_TITLE       "Ham - Docs"                   CACHE STRING "Title prefix on all generated documentation pages.")
set(HAM_DOCS_AUTHOR      "Hamsmith Ltd."                CACHE STRING "Author shown in generated documentation.")
set(HAM_DOCS_DESCRIPTION "The Ham Programming Language" CACHE STRING "Description line for generated documentation.")
set(HAM_DOCS_URL         "https://ham.hamsmith.dev/"    CACHE STRING "URL used in generated documentation.")

configure_file(index.html.in ${CMAKE_CURRENT_BINARY_DIR}/index.html @ONLY)

add_custom_target(
    ham-docs
    SOURCES
	    main.css.in
		index.html.in
)
