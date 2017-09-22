/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "ValueConverter.hpp"
#include "ValueConverter_impl.hpp"

namespace helics
{
	template ValueConverter<double>;
	template ValueConverter<int64_t>;
	template ValueConverter<char>;
	template ValueConverter<std::complex<double>>;
	template ValueConverter<float>;
	template ValueConverter<short>;
}


