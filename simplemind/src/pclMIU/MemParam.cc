#include "MemParam.h"


MemParam::MemParam()
	: Attribute()
{
}


MemParam::MemParam(const MemParam& ip)
	: Attribute(ip)
{
}


MemParam::~MemParam()
{
}


RetainCands::RetainCands()
	: MemParam()
{
}


RetainCands::RetainCands(const RetainCands& m)
	: MemParam(m)
{
}


RetainCands::~RetainCands()
{
}


void RetainCands::write(ostream& s) const
{
        _write_start_attribute(s);
        _write_end_attribute(s);
}
