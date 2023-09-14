#include "KnowledgeSource.h"

KnowledgeSource::KnowledgeSource(
				const std::string name,
				const std::string type,
				float (*activation_score) (Blackboard&),
				void (*activate) (Blackboard&),
				void (*activation_record) (Blackboard&))
      :	_name(name),
	_type(type),
	_activation_score(activation_score),
	_activate(activate),
	_activation_record(activation_record)
{
}

KnowledgeSource::KnowledgeSource(const KnowledgeSource& ks)
      :	_name(ks._name),
	_type(ks._type),
	_activation_score(ks._activation_score),
	_activate(ks._activate),
	_activation_record(ks._activation_record)
{
}

KnowledgeSource::~KnowledgeSource()
{
}

const std::string& KnowledgeSource::name() const
{
	return _name;
}

const std::string& KnowledgeSource::type() const
{
	return _type;
}

void KnowledgeSource::add_activation_rec(Blackboard& bb) const
{
	bb.append_act_rec(_name, _type);
	if (_activation_record)
		(*_activation_record)(bb);
}

const float KnowledgeSource::activation_score(Blackboard& bb) const
{
	return (*_activation_score)(bb);
}

void KnowledgeSource::activate(Blackboard& bb) const
{
	(*_activate)(bb);
}
