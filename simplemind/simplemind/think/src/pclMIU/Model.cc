#include "Model.h"
#include <pcl/misc/FileNameTokenizer.h>

Model::Model(const char* const file)
	: _file(file)
{
}

//Model::Model(const char* const file, const char* const roi_directory, const char* const chromosome)
Model::Model(const char* const file, const char* const chromosome)
	: _file(file)
{
	//if (roi_directory != 0) _roi_directory.append(roi_directory);
	if (chromosome != 0) _chromosome.append(chromosome);
}

/*
Model::Model(const char* const file, const char* const chromosome)
	: _file(file), _chromosome(chromosome)//, _entity(20)
{
	//for (int i=0; i<_chromosome.length(); i++) _bit_used.push_back(false);
}
*/

Model::Model(const Model& m)
	: _file(m._file), _entity(m._entity), _chromosome(m._chromosome)
{
		//for (int i=0; i<_bit_used.size(); i++) _bit_used.push_back(m._bit_used[i]);
}


Model::~Model()
{
}

const int Model::add_entity(const AnatPathEntity& ape)
{
	int ok=1;
  std::vector<AnatPathEntity>::const_iterator p;
	for(p=_entity.begin(); (p!=_entity.end())&&ok; ++p)
		ok = (p->name().compare(ape.name()) != 0);

	if (ok)
		_entity.push_back(ape);

	//for(int i=0; i<_entity.N(); i++)
	//	ok = (_entity[i].name().compare(ape.name()) != 0);

	//if (ok)
	//	_entity.push_last(ape);

	return ok;
}


const int Model::add_entity_first(const AnatPathEntity& ape)
{
	int ok=1;

  std::vector<AnatPathEntity>::const_iterator p;
	for(p=_entity.begin(); (p!=_entity.end())&&ok; ++p)
		ok = (p->name().compare(ape.name()) != 0);

	if (ok)
		_entity.insert(_entity.begin(), ape);

	//for(int i=0; i<_entity.N(); i++)
	//	ok = (_entity[i].name().compare(ape.name()) != 0);

	//if (ok)
	//	_entity.push_first(ape);

	return ok;
}

void Model::read()
{
	pcl::FileNameTokenizer fname(_file);
	std::string full_path = fname.getPath();

	cout << "Reading model from " << _file <<endl;

	Darray<std::string> entity_names(10);
	_parse_model_file(_file, entity_names);

	for(int i=0; i<entity_names.N(); i++) {
		std::string full_ape_fname = full_path;
		full_ape_fname.append(entity_names[i]);

		ifstream ape_ifs(full_ape_fname.data());
		if (!ape_ifs) {
			cerr << "ERROR: Model: could not open AnatPathEntity file for reading: " << full_ape_fname << endl;
			exit(1);
		}

		cout << "Reading entity " << entity_names[i] << endl;
		AnatPathEntity ape;
		ape_ifs >> ape;
		if (!add_entity(ape)) {
			cerr << "ERROR: Model: could not add " << ape.name() << " to model - entity already exists" << endl;
			exit(1);
		}

		ape_ifs.close();
	}
}


void Model::write(const char* const full_path) const
{
	//This function will probably not function properly as Model has been cannibalized by Pechin
	std::string model_fname(full_path);

	cout << "Writing model to " << model_fname <<endl;

	ofstream model_ofs(model_fname.data());
	if (!model_ofs) {
		cerr << "ERROR: Model: write: could not open model file for reading: " << model_fname << endl;
		//exit(1);
	}

  std::vector<AnatPathEntity>::const_iterator p;
	for(p=_entity.begin(); p!=_entity.end(); ++p) {
		model_ofs << p->name() << endl;
	//for(int i=0; i<_entity.N(); i++) {
	//	model_ofs << _entity[i].name() << endl;

		std::string full_ape_fname = full_path;
		full_ape_fname.append(p->name());
		//full_ape_fname.append(_entity[i].name());

		ofstream ape_ofs(full_ape_fname.data());
		if (!ape_ofs) {
			cerr << "ERROR: Model: write: could not open AnatPathEntity file for writing: " << full_ape_fname << endl;
			exit(1);
		}

		cout << "Writing entity " << p->name() << endl;
		//cout << "Writing entity " << _entity[i].name() << endl;
		ape_ofs << *p;
		//ape_ofs << _entity[i];

		ape_ofs.close();
	}
	//model_ofs << "End: " << _name << endl;
	model_ofs.close();
}


const int Model::n() const
{
	return _entity.size();
	//return _entity.N();
}

AnatPathEntity& Model::entity(const unsigned int i) const
{
	if ((i<0) || (i>=_entity.size())) {
	//if ((i<0) || ((int)i>=_entity.N())) {
		cerr << "ERROR: Model: index invalid" << endl;
		exit(1);
	}

	return (AnatPathEntity&)_entity[i];
}


const int Model::entity_index(const char* const name) const
{
  std::vector<AnatPathEntity>::const_iterator p;
	int ind=0, done=0;
	for(p=_entity.begin(); (p!=_entity.end())&&(done==0); ++p) {
	//for(int i=0; i<_entity.N(); ++i) {
		if (p->name().compare(name) == 0)
		//if (_entity[i].name().compare(name) == 0)
			done = 1;
		else
			ind++;
	}

	if (!done)
		ind = -1;

	return ind;
}


void Model::_parse_model_file(const std::string& model_filename, Darray<std::string>& entity_names) const
{
	ifstream model_ifs(model_filename.data());
	if (!model_ifs) {
		cerr << "ERROR: Model: could not open model file for reading: " << model_filename << endl;
		exit(1);
	}

	std::string mod_line, mod_name, dummy;
	getline(model_ifs, mod_line);
	int i=0;
	read_word(mod_line, dummy, i);
	if (dummy.compare("Model:") != 0) {
		cerr << "ERROR: starting line is different from expected!" << endl;
		exit(1);
	}

	int version_num;
	model_ifs >> version_num;
	getline(model_ifs, dummy); // skip end of line

	std::string ape_fname;
	mod_name.clear();
	int done = 0;
	while (getline(model_ifs, ape_fname) && (done==0))
	   if (ape_fname.length()>0) {
		i = 0;
		dummy.clear();
		read_word(ape_fname, dummy, i);
		if (dummy.compare("End:") == 0) {
			done = 1;
		}
		else
			entity_names.push_last(ape_fname);
	}

	model_ifs.close();
	if (!done) {
		cerr << "ERROR: Model: Expected an End statement" << endl;
		exit(1);
	}
}


ostream& operator<<(ostream& s, const Model& m)
{
	s << m._file << endl;

  std::vector<AnatPathEntity>::const_iterator p;
	for(p=m._entity.begin(); p!=m._entity.end(); ++p)
	//for(int i=0; i<m._entity.N(); ++i)
		s << *p;
		//s << m._entity[i];

	return s;
}

/*
PSmodel::PSmodel(const char* const model_name, const char* const identifier, const int version_num)
	: Model(model_name, version_num, 0), _identifier(identifier)
{
}


PSmodel::PSmodel(const Model& m, const char* const identifier)
	: Model(m),  _identifier(identifier)
{
}


PSmodel::PSmodel(const PSmodel& p)
	: Model(p), _identifier(p._identifier)
{
}


PSmodel::~PSmodel()
{
}


void PSmodel::read(const char* const path)
{
	std::string dummy;

	std::string full_path = path;
	full_path.append("/");
	full_path.append(name());
	full_path.append(".");
	char vn[100];
	sprintf(vn, "%d", version());
	full_path.append(vn);
	full_path.append("/PSmodels/");
	full_path.append(_identifier);
	full_path.append(".");

	std::string model_fname = full_path;
	model_fname.append(name());

	cout << "Reading model from " << model_fname <<endl;

	Darray<std::string> entity_names(10);
	_parse_model_file(model_fname, entity_names);

	for(int i=0; i<entity_names.N(); i++) {
		std::string full_ape_fname = full_path;
		full_ape_fname.append(entity_names[i]);

		ifstream ape_ifs(full_ape_fname.data());
		if (!ape_ifs) {
			cerr << "ERROR: Model: could not open AnatPathEntity file for reading: " << full_ape_fname << endl;
			exit(1);
		}

		cout << "Reading entity " << entity_names[i] << endl;
		AnatPathEntity ape;
		ape_ifs >> ape;
		if (!add_entity(ape)) {
			cerr << "ERROR: Model: could not add " << ape.name() << " to model - entity already exists" << endl;
			exit(1);
		}

		ape_ifs.close();
	}
}


void PSmodel::write(const char* const path) const
{
	std::string full_path = path;
	full_path.append("/");
	full_path.append(name());
	full_path.append(".");
	char vn[100];
	sprintf(vn, "%d", version());
	full_path.append(vn);
	full_path.append("/PSmodels/");
	full_path.append(_identifier);
	full_path.append(".");

	Model::write(full_path.data());
}

*/