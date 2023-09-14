#include "InfParam.h"

using std::endl;

InfParam::InfParam()
	: Attribute()
{
}


InfParam::InfParam(const InfParam& ip)
	: Attribute(ip)
{
}


InfParam::~InfParam()
{
}


MatchAboveConf::MatchAboveConf(const float conf_thresh)
	: InfParam(), _conf_thresh(conf_thresh)
{
}


MatchAboveConf::MatchAboveConf(const MatchAboveConf& m)
	: InfParam(m), _conf_thresh(m._conf_thresh)
{
}


MatchAboveConf::~MatchAboveConf()
{
}


void MatchAboveConf::write(ostream& s) const
{
        _write_start_attribute(s);
        s << "Confidence_threshold: " << _conf_thresh << endl;
        _write_end_attribute(s);
}
