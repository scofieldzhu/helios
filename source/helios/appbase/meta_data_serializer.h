/*******************************************************
* All Copyright (C) by Sysbot Co. ltd (2022-2026)
* author: zhucg
* time:2026/2/28
*******************************************************/
#ifndef __meta_data_serializer_h__
#define __meta_data_serializer_h__

#include <iosfwd>
#include "helios/basic/helios_basic_typedef.h"

HELIOS_NAMESPACE_BEGIN

class MetaDataSerializer
{
public:
	virtual bool serializeToOstream(std::ostream* output) const = 0;
	virtual bool serializeToByteData(ByteBuffer& data)const = 0;
	virtual bool parseFromByteData(const ByteBuffer& data) = 0;
	virtual bool parseFromIstream(std::istream* input) = 0;
	virtual bool isContentModified()const = 0;
	virtual ~MetaDataSerializer() = default;
};

NAMESPACE_END

#endif
