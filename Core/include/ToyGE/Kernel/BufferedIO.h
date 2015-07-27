#pragma once
#ifndef BUFFEREDIO_H
#define BUFFEREDIO_H

#include "ToyGE\Kernel\IOHelper.h"

namespace ToyGE
{
	class TOYGE_CORE_API BufferedReader : public Reader
	{
	public:
		virtual ~BufferedReader() = default;

		BufferedReader(const std::shared_ptr<uint8_t> & data, size_t size)
			: _data(data),
			_size(size),
			_cur(data.get())
		{
			
		}

	protected:
		void DoReadBytes(void * dst, size_t numBytes) override
		{
			memcpy(dst, _cur, numBytes);
			_cur += numBytes;
		}

	private:
		std::shared_ptr<uint8_t> _data;
		size_t _size;
		uint8_t * _cur;
	};
}

#endif