#include "Attribute.h"

using std::cerr;
using std::endl;
using std::cout;

Attribute::Attribute()
	: _b_flag(0), _e_flag(0), _index_rel_solel(1), _name_rel_solel(1)
{
}

Attribute::Attribute(const Attribute& a)
	: _b_flag(a._b_flag), _e_flag(a._e_flag),
  _index_rel_solel(a._index_rel_solel),
	_name_rel_solel(a._name_rel_solel), _chromosome_bits_used(a._chromosome_bits_used)
{
}

Attribute::~Attribute()
{
}

void Attribute::add_rel_solel(const std::string& rel_name, const int rel_index)
{
	_name_rel_solel.push_last(rel_name);
	_index_rel_solel.push_last(rel_index);
}

const int Attribute::num_rel_solels() const
{
	return _name_rel_solel.N();
}

const std::string& Attribute::rel_solel_name(const int i) const
{
	if ((i<0) || (i>=_name_rel_solel.N())) {
		cerr << "ERROR: Attribute: rel_solel_name: index invalid for getting related solution element name" << endl;
		exit(1);
	}

	return _name_rel_solel[i];
}


const int Attribute::rel_solel_index(const int i) const
{
	if ((i<0) || (i>=_index_rel_solel.N())) {
		cerr << "ERROR: Attribute: rel_solel_index: index invalid for getting related solution element index" << endl;
		exit(1);
	}

	return _index_rel_solel[i];
}


void Attribute::set_b_flag()
{
	_b_flag = 1;
}


const int Attribute::b_flag() const
{
	return _b_flag;
}


void Attribute::set_chromosome_bits_used(const std::vector<bool> bits) {
	int i;
	for (i=0; i<bits.size(); i++) {
		if (i>=_chromosome_bits_used.size()) {
			_chromosome_bits_used.push_back(bits[i]);
		}
		else if (bits[i]) _chromosome_bits_used[i] = true;
	}
}


const bool Attribute::no_chromosome_bits_used() const {
	bool used = false;
	int i;
	for (i=0; (i<_chromosome_bits_used.size()) && !used; i++)
		used = _chromosome_bits_used[i];
	return !used;
}


void Attribute::write(ostream& s) const
{
	_write_start_attribute(s);
	_write_end_attribute(s);
}

void Attribute::_write_start_attribute(ostream& s) const
{
	s << "Attribute: " << type() << ": " << name() << endl;

	int i;
	for(i=0; i<num_rel_solels(); i++) {
		s << "Related SolElement: " << _name_rel_solel[i];
		//s << ", " << _index_rel_solel[i];
		s << endl;
	}
}

void Attribute::_write_end_attribute(ostream& s) const
{
	s << "End: " << name() << endl;
}

