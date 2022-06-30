#pragma once

namespace Math
{
	template<class data_type>
	data_type clamp(data_type x, data_type min, data_type max)
	{
		if (x < min) x = min;
		else if (x > max) x = max;

		return x;
	}
}
