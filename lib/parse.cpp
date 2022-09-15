#include "ham/parse.h"
#include "ham/hash.h"

#include "ham/std_vector.hpp"

#include "robin_hood.h"

#include <uchar.h>

#define HAM_PARSE_IMPL_X_UTF 8
#include "parse-impl.x.h"

#define HAM_PARSE_IMPL_X_UTF 16
#include "parse-impl.x.h"

#define HAM_PARSE_IMPL_X_UTF 32
#include "parse-impl.x.h"
