#
# CMake string helper library
#

macro(string_camel_to_snake STR OUTPUT_VAR)
	string(
		REGEX

		REPLACE
			"^([a-zA-Z][a-z]*)([A-Z][a-z]*|[0-9]+)$"
			"\\1_\\2"

		${OUTPUT_VAR}
		${STR}
	)
endmacro()
