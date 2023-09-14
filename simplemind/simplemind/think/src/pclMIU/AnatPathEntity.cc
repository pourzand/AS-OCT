#include "AnatPathEntity.h"
#include <stdlib.h>

using std::cerr;
using std::endl;

AnatPathEntity::AnatPathEntity()
//	: _descr(5)
{
}

AnatPathEntity::AnatPathEntity(const std::string& name)
	:_name(name)//, _descr(5)
{
}

AnatPathEntity::AnatPathEntity(const AnatPathEntity& a)
	:_name(a._name), _descr(a._descr)
{
	 //std::vector<std::string>::const_iterator p;
	//	for(p=a._descr.begin(); p!=a._descr.end(); ++p)
	//	_descr.push_back(*p);
}

AnatPathEntity::~AnatPathEntity()
{
}

const int AnatPathEntity::add_descriptor(const std::string& descr)
{
	int ok=1;

	_descr.push_back(descr);
	//_descr.push_last(descr);

	return ok;
}

const std::string& AnatPathEntity::name() const
{
	return _name;
}


const int AnatPathEntity::n() const
{
	return _descr.size();
}

const std::string AnatPathEntity::descriptor(const unsigned int i) const
{
	if ((i<0) || (i>=_descr.size())) {
		cerr << "ERROR: AnatPathEntity: index invalid" << endl;
		exit(1);
	}

	return _descr[i];
}

ostream& operator<<(ostream& s, const AnatPathEntity& a)
{
	s << "AnatPathEntity: " << a._name << ";" << endl;

  std::vector<std::string>::const_iterator p;
	for(p=a._descr.begin(); p!=a._descr.end(); ++p)
		s << *p << ";" << endl;

	//for(int i=0; i<a._descr.N(); i++) s << a._descr[i] << ";" << endl;

	s << "End: " << a._name << ";" << endl;

	return s;
}


istream& operator>>(istream& s, AnatPathEntity& a)
{
	std::string istr;

	s >> istr;
	if (istr.compare("AnatPathEntity:") != 0) {
		cerr << "ERROR: AnatPathEntity input stream operator: expected \"AnatPathEntity:\", received: " << istr << endl;
		exit(1);
	}

	getline(s, istr);

	if (istr[istr.length()-1] != ';') {
		cerr << "ERROR: AnatPathEntity input stream operator: expected line to end with a semicolon: " << istr << endl;
		exit(1);
	}
	a._name.append(istr, 1, istr.length()-2);

	int end_found=0;
	while (getline(s, istr) && !end_found) {

		if (istr[istr.length()-1] != ';') {
			cerr << "ERROR: AnatPathEntity input stream operator: expected line to end with a semicolon: " << istr << endl;
			exit(1);
		}

		if (!istr.compare(0, 4, "End:")) {
			end_found = 1;
			if (istr.compare(5, istr.length()-6, a._name) != 0) {
				cerr << "ERROR: AnatPathEntity input stream operator: expected \"End: " << a._name << ";\", received: " << istr << endl;
				exit(1);
			}
		}
		else {
			std::string nsc;
			nsc.append(istr, 0, istr.length()-1);
			a.add_descriptor(nsc);
		}
	}
	if (!end_found) {
		cerr << "ERROR: AnatPathEntity input stream operator: expected end statement for " << a._name << endl;
		exit(1);
	}

	return s;
}


