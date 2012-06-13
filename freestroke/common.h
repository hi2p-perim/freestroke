#define NOMINMAX

#include <boost/format.hpp>
#include <boost/unordered_map.hpp>
#include <boost/foreach.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/norm.hpp>

#include <QtGui>

#include "exception.h"

#define SAFE_DELETE(val) if ((val) != NULL ) { delete (val); (val) = NULL; }
#define SAFE_DELETE_ARRAY(val) if ((val) != NULL ) { delete[] (val); (val) = NULL; }
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
	TypeName(const TypeName&); \
	void operator=(const TypeName&)