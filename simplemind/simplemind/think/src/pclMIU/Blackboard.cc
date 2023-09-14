#include "Blackboard.h"

ImageCandidate::ImageCandidate(ImagePrimitive* prim, const int num_attributes)
	: _primitive(prim), _num_attributes(num_attributes), _partial_conf(1.0)
{
	if (_num_attributes>0) {
		_confidence = new float [_num_attributes];
		_feature_value = new float [_num_attributes];
		int i;
		for(i=0; i<_num_attributes; i++)
			_confidence[i] = 2.0;
	}
	else if (_num_attributes == 0) {
		_confidence = 0;
		_feature_value = 0;
	}
	else {
		cerr << "ERROR: Blackboard: ImageCandidate: Constructor failed since number of attributes negative" << endl;
		exit(1);
	}
		
}

ImageCandidate::ImageCandidate(const ImageCandidate& ic)
	: _primitive(ic._primitive),
	_num_attributes(ic._num_attributes),
	_partial_conf(ic._partial_conf)
{
	_confidence = new float [_num_attributes];
	_feature_value = new float [_num_attributes];
	int i;
	for(i=0; i<_num_attributes; i++) {
		_confidence[i] = ic._confidence[i];
		_feature_value[i] = ic._feature_value[i];
	}
}

ImageCandidate::~ImageCandidate()
{
	if (_confidence)
		delete [] _confidence;
	if (_feature_value)
		delete [] _feature_value;

	delete _primitive;
}


void ImageCandidate::conf_score(const int attribute_index, const float score)
{
	if ((attribute_index<0) || (attribute_index>_num_attributes)) {
		cerr << "ERROR: Blackboard: ImageCandidate: attribute index invalid for setting confidence score" << endl;
		exit(1);
	}
	else {
		_confidence[attribute_index] = score;
		if (score<_partial_conf)
			_partial_conf = score;
	}
}

const float ImageCandidate::conf_score(const int attribute_index) const
{
	if ((attribute_index<0) || (attribute_index>_num_attributes)) {
		cerr << "ERROR: Blackboard: ImageCandidate: attribute index invalid for getting confidence score" << endl;
		exit(1);
	}
	return _confidence[attribute_index];
}


void ImageCandidate::feature_value(const int feature_index, const float value)
{
	if ((feature_index<0) || (feature_index>_num_attributes)) {
		cerr << "ERROR: Blackboard: ImageCandidate: feature index invalid for setting value" << endl;
		exit(1);
	}
	else
		_feature_value[feature_index] = value;
}


const float ImageCandidate::feature_value(const int feature_index) const
{
	if ((feature_index<0) || (feature_index>_num_attributes) || (_confidence[feature_index]==2.0)) {
		cerr << "ERROR: Blackboard: ImageCandidate: feature index invalid for getting value" << endl;
		exit(1);
	}
	return _feature_value[feature_index];
}


void ImageCandidate::write(ostream& s) const
{
	_primitive->write(s);
	s << _num_attributes;
	for (int i=0; i<_num_attributes; i++) {
		s << " ";
		if (_confidence[i]!=2.0)
			s << _feature_value[i];
		else
			s << 0;
		s << " " << _confidence[i];
	}
	s << endl << _partial_conf << endl;
}


SolElement::SolElement(const std::string& name)
	: _name(name),
	_attribute(10),
	_candidate(20),
	_matched_cand_index(5),
	_matched_prim(0)
	/*,_num_matched_cands(0)*/
{
}


SolElement::SolElement(const SolElement& s)
	: _name(s._name),
	_attribute(10),
	_candidate(20),
	_matched_cand_index(5),
	_matched_prim(0)
{
}


SolElement::~SolElement()
{
	int i;
	for(i=0; i<_attribute.N(); i++)
		delete _attribute(i);
	for(i=0; i<_candidate.N(); i++)
		delete _candidate(i);

	if (_matched_prim)
		delete _matched_prim;
}


const std::string& SolElement::name() const
{
	return _name;
}


void SolElement::add_attribute(Attribute* a)
{
	_attribute.push_last(a);
}


const Attribute* const SolElement::attribute(const int i) const
{
	if ((i<0) || (i>=_attribute.N())) {
		cerr << "ERROR: Blackboard: SolElement: index invalid for getting attribute" << endl;
		exit(1);
	}

	return _attribute[i];
}


const Attribute* const SolElement::find_attribute(const char* const att_name) const
{
	int i;
	Attribute* ap=0;
	for(i=0; (i<_attribute.N()) && !ap; i++) {
		if (!strcmp(_attribute[i]->name(), att_name))
			ap = _attribute[i];
	}

	return ap;
}


Attribute* SolElement::find_update_attribute(const char* const att_name)
{
	int i;
	Attribute* ap=0;
	for(i=0; (i<_attribute.N()) && !ap; i++) {
		if (!strcmp(_attribute[i]->name(), att_name))
			ap = _attribute[i];
	}

	return ap;
}


void SolElement::add_candidate(ImagePrimitive* prim)
{
	ImageCandidate *ic = new ImageCandidate (prim, num_attributes());
	_candidate.push_last(ic);
}


ostream& operator<<(ostream& s, const SolElement& e)
{
	s << "SolElement: " << e.name() << endl;

	int i;
	for(i=0; i<e._attribute.N(); i++)
		e._attribute[i]->write(s);

	s << "End: " << e.name() << endl;

	return s;
}


ImageCandidate* SolElement::candidate(const int i)
{
	if ((i<0) || (i>=_candidate.N())) {
		cerr << "ERROR: Blackboard: SolElement: index invalid for getting candidate" << endl;
		exit(1);
	}
	return _candidate[i];
}


void SolElement::append_matched_cand_index(const int index)
{
	if ((index<0) || (index>=_candidate.N())) {
		cerr << "ERROR: Blackboard: SolElement: index invalid for appending matched candidate index" << endl;
		exit(1);
	}

	_matched_cand_index.push_last(index);

	//_matched_cand_comb = candidate(index)->primitive()->combine(_matched_cand_comb);
}


const int SolElement::num_matched_cands(int& num) const
{
	num = _matched_cand_index.N();
	return (_matched_prim != 0);
}


const int SolElement::matched_cand_index(const int i) const
{
	if ((i<0) || (i>=_matched_cand_index.N())) {
		cerr << "ERROR: Blackboard: SolElement: index invalid for getting matched candidate index" << endl;
		exit(1);
	}
	return _matched_cand_index[i];
}


void SolElement::create_matched_prim(const MedicalImageSequence& mis)
{
	if (_matched_cand_index.N()>0) {
		Darray<ImagePrimitive*> prims(5);
		int i;
		for(i=1; i<_matched_cand_index.N(); i++)
			prims.push_last(candidate(_matched_cand_index[i])->primitive());
		_matched_prim = candidate(_matched_cand_index[0])->primitive()->create_matched_prim(prims, mis);
	}
}


void SolElement::free_candidates()
{
	while (_candidate.N()>0) {
		delete _candidate(0);
		_candidate.delete_item(0);
	}
	while (_matched_cand_index.N()>0)
		_matched_cand_index.delete_item(0);
}


void SolElement::write(ostream& s) const
{
	s << "SolElement: " << _name << endl;

	int i;
	for(i=0; i<_attribute.N(); i++)
		_attribute[i]->write(s);

	s << "Num_Cands: " << _candidate.N() << endl;
	for(i=0; i<_candidate.N(); i++)
		_candidate[i]->write(s);

	s << _matched_cand_index.N();
	for(i=0; i<_matched_cand_index.N(); i++)
		s << " " << _matched_cand_index[i];
	s << endl;

	if (_matched_prim) {
		s << 1 << endl;
		_matched_prim->write(s);
	}
	else
		s << 0 << endl;

	s << "End: " << _name << endl;
}


GroupCandidate::GroupCandidate(const int num_solels)
	: _num_solels(num_solels), _confidence(1.0)
{
	//_im_cand_index = new int [num_solels];
	int i;
	for(i=0; i<_num_solels; i++)
		_im_cand_index.push_back(-1);
		//_im_cand_index[i] = -1;
}

GroupCandidate::GroupCandidate(const GroupCandidate& gc)
	: _num_solels(gc._num_solels), _confidence(gc._confidence)
{
	//_im_cand_index = new int [_num_solels];
	int i;
	for(i=0; i<_num_solels; i++)
		_im_cand_index.push_back(gc._im_cand_index[i]);
		//_im_cand_index[i] = gc._im_cand_index[i];
}


GroupCandidate::~GroupCandidate()
{
	/*****
	delete [] _im_cand_index;
	_im_cand_index = NULL;
	*****/
}


void GroupCandidate::im_cand_index(const int i, const int im_cand_ind)
{
	if ((i<0) || (i>_num_solels)) {
		cerr << "ERROR: Blackboard: GroupCandidate: index invalid for setting image candidate index" << endl;
		exit(1);
	}

	_im_cand_index[i] = im_cand_ind;
}


const int GroupCandidate::im_cand_index(const int i) const
{
	if ((i<0) || (i>_num_solels)) {
		cerr << "ERROR: Blackboard: GroupCandidate: index invalid for getting image candidate index" << endl;
		exit(1);
	}

	return _im_cand_index[i];
}


SEgroup::SEgroup()
	: _priority(0.0), _sol_el_index(1), _candidate(25)
{
}


SEgroup::SEgroup(const SEgroup& seg)
	: _priority(seg._priority),
	_sol_el_index(seg._sol_el_index),
	_candidate(seg._candidate)
{
}


SEgroup::~SEgroup()
{
}


void SEgroup::priority(const float p)
{
	_priority = p;
}


const float SEgroup::priority() const
{
	return _priority;
}


void SEgroup::add_sol_el(const int bb_index)
{
	if (_candidate.N()) {
		cerr << "ERROR: Blackboard: SEgroup: add_sol_el: cannot add a solution element to a group after candidates have been generated for the group" << endl;
		exit(1);
	}

	_sol_el_index.push_last(bb_index);
}


const int SEgroup::sol_el_index(const int i) const
{
	if ((i<0) || (i>_sol_el_index.N())) {
		cerr << "ERROR: Blackboard: SEgroup: sol_el_index: index invalid for getting solution element index" << endl;
		exit(1);
	}

	return _sol_el_index[i];
}


const int SEgroup::find_solel_grp_ind(const int solel_index_bb) const
{
	register int i;
	for(i=0; (i<_sol_el_index.N()) && (_sol_el_index[i]!=solel_index_bb); i++);
	if (i<_sol_el_index.N())
		return i;
	else
		return -1;
}


const int SEgroup::in_group(const int sol_el_index) const
{
	int found=0, i;
	for(i=0; (i<_sol_el_index.N()) && !found; i++)
		found = (_sol_el_index[i] == sol_el_index);

	return found;
}


void SEgroup::add_group_cand(const GroupCandidate& gc)
{
	_candidate.push_last(gc);
}


GroupCandidate& SEgroup::group_cand(const int i)
{
	if ((i<0) || (i>_candidate.N())) {
		cerr << "ERROR: Blackboard: SEgroup: group_cand: index invalid for getting group candidate" << endl;
		exit(1);
	}

	return _candidate(i);
}

void SEgroup::free_candidates()
{
	while (_candidate.N()>0)
		_candidate.delete_item(0);
}



ActivationRecord::ActivationRecord(const std::string& name, const std::string& type)
	: _name(name), _type(type), _message(3)
{
}


ActivationRecord::ActivationRecord(const ActivationRecord& a)
	: _name(a._name), _type(a._type), _message(3)
{
	int i;
	for(i=0; i<a._message.N(); i++)
		_message.push_last(a._message[i]);
}


ActivationRecord::~ActivationRecord()
{
}


const std::string& ActivationRecord::ks_name() const
{
	return _name;
}


const std::string& ActivationRecord::ks_type() const
{
	return _type;
}


void ActivationRecord::message(const std::string& mess)
{
	_message.push_last(mess);
}

const int ActivationRecord::num_messages() const
{
	return _message.N();
}

const std::string& ActivationRecord::message(const int i) const
{
	if ((i<0) || (i>=_message.N())) {
		cerr << "ERROR: Blackboard: ActivationRecord: index invalid" << endl;
		exit(1);
	}

	return _message[i];
}



const int ActivationRecord::find_message(const std::string& s) const
{
	int i;
	for(i=0; (i<_message.N()) && (_message[i].compare(s)!=0); i++);
	if (i<_message.N())
		return i;
	else
		return -1;
}

const int ActivationRecord::find_message_starting_with(const std::string& s) const
{
	int i;
	for(i=0; (i<_message.N()) && (_message[i].compare(0, s.length(), s)!=0); i++);
	if (i<_message.N())
		return i;
	else
		return -1;
}


ostream& operator<<(ostream& s, const ActivationRecord& a)
{
	s << "ActivationRecord: " << a._name << ", " << a._type << endl;
	int i;
	for(i=0; i< a._message.N(); i++)
		s << a._message[i] << endl;
	s << "ActivationRecord: End" << endl;
	return s;
}

Blackboard::Blackboard(MedicalImageSequence& ms, Model& mod, const char* const image_path, const char* const exec_path, const char* const temp_file_path)
	: _mis(ms)/*, _hu_values_column_major(0)*/, _model(mod), _solel(10), _actrec(30), _group(10), _next_group(-1), _next_solel(-1)
{
	Point tl(0, 0, 0), br(ms.xdim()-1, ms.ydim()-1, ms.zdim()-1);
	_overall_search_area.add_box(tl, br);
	_image_path = new char [strlen(image_path)+1];
	strcpy(_image_path, image_path);
	_temp_file_path = new char [strlen(temp_file_path)+1];
	strcpy(_temp_file_path, temp_file_path);
	_exec_directory.append(exec_path);
}

Blackboard::Blackboard(MedicalImageSequence& ms, Model& mod , const ROI& s_area, const char* const image_path, const char* const exec_path, const char* const temp_file_path/*, MedicalImageSequence* ss_ms*/)
	: _mis(ms)/*, _hu_values_column_major(0)*/, _model(mod), _solel(10), _actrec(30), _group(10), 
	_overall_search_area(s_area), _next_group(-1), _next_solel(-1)/*,_ss_mis(ss_ms)*/
{
	_image_path = new char [strlen(image_path)+1];
	strcpy(_image_path, image_path);
	_temp_file_path = new char [strlen(temp_file_path)+1];
	strcpy(_temp_file_path, temp_file_path);
	_exec_directory.append(exec_path);
}

Blackboard::Blackboard(MedicalImageSequence& ms, Model& mod , const ROI& s_area, const char* const image_path, const char* const exec_path, const char* const temp_file_path, const char* const roi_directory, const char* const edm_directory, const char* const stop_at_node)
	: _mis(ms)/*, _hu_values_column_major(0)*/, _model(mod), _solel(10), _actrec(30), _group(10), 
	_overall_search_area(s_area), _next_group(-1), _next_solel(-1)/*,_ss_mis(ss_ms)*/
{
	_image_path = new char [strlen(image_path)+1];
	strcpy(_image_path, image_path);
	_temp_file_path = new char [strlen(temp_file_path)+1];
	strcpy(_temp_file_path, temp_file_path);
	_exec_directory.append(exec_path);

	if (roi_directory != 0) {
		_roi_directory.append(roi_directory);
	}

	if (edm_directory != 0) {
		_edm_directory.append(edm_directory);
	}

	if (stop_at_node != 0) {
		_stop_at_node.append(stop_at_node);
	}
}


Blackboard::Blackboard(MedicalImageSequence& ms, Model& mod , const ROI& s_area, const char* const image_path, const char* const exec_path, 
                        const char* const temp_file_path, const char* const roi_directory, const char* const edm_directory,
						const char* const stop_at_node, const char* const user_resource_directory, const char* const condor_job_directory, 
						const bool skip_normalized_image_png, const bool skip_normalized_image_png_training, const bool skip_tensorboard_logging, 
						const bool predict_cpu_only)
	: _mis(ms)/*, _hu_values_column_major(0)*/, _model(mod), _solel(10), _actrec(30), _group(10), 
	_overall_search_area(s_area), _next_group(-1), _next_solel(-1)/*,_ss_mis(ss_ms)*/
{
	_image_path = new char [strlen(image_path)+1];
	strcpy(_image_path, image_path);
	_temp_file_path = new char [strlen(temp_file_path)+1];
	strcpy(_temp_file_path, temp_file_path);
	_exec_directory.append(exec_path);

	if (roi_directory != 0) {
		_roi_directory.append(roi_directory);
	}

	if (edm_directory != 0) {
		_edm_directory.append(edm_directory);
	}

	if (stop_at_node != 0) {
		_stop_at_node.append(stop_at_node);
	}

	if (user_resource_directory != 0) {
		_user_resource_directory.append(user_resource_directory);
	}

	if (condor_job_directory != 0) {
		_condor_job_directory.append(condor_job_directory);
	}

    _predict_cpu_only = predict_cpu_only;
	_skip_normalized_image_png = skip_normalized_image_png;
	_skip_normalized_image_png_training = skip_normalized_image_png_training;
	_skip_tensorboard_logging = skip_tensorboard_logging;
}

Blackboard::~Blackboard()
{
	delete [] _image_path;
	delete [] _temp_file_path;
	//if (_hu_values_column_major) delete [] _hu_values_column_major;
}

const Model& Blackboard::model() const
{
	return _model;
}

/*
const short* const Blackboard::hu_values_column_major() {
	if (!_hu_values_column_major) {
		_hu_values_column_major = short int [_mis.xdim()*_mis.ydim()*_mis.zdim()];
		for (int z = 0; z < _mis.zdim(); z++) {
			const Image& im = _mis.image_const(z);
			for (int y = 0; y < _mis.ydim(); y++)
				for (int x = 0; x < _mis.xdim(); x++)
					_hu_values_column_major[y + x * _mis.ydim() + z * _mis.ydim() * _mis.xdim()] = (short) (_mis.fast_pix_val(x, y, z) * im.rescale_slope() + im.rescale_intercept());
		}
	}
	return _hu_values_column_major;
}
*/


const char* const Blackboard::image_path() const
{
	return _image_path;
}

const char* const Blackboard::temp_file_path() const
{
	return _temp_file_path;
}

const std::string Blackboard::roi_directory() const
{
	return _roi_directory;
}

const std::string Blackboard::edm_directory() const
{
	return _edm_directory;
}

const std::string Blackboard::exec_directory() const
{
	return _exec_directory;
}
const std::string Blackboard::stop_at_node() const
{
	return _stop_at_node;
}

const std::string Blackboard::user_resource_directory() const
{
	return _user_resource_directory;
}

const std::string Blackboard::condor_job_directory() const
{
	return _condor_job_directory;
}

const bool Blackboard::predict_cpu_only() const
{
	return _predict_cpu_only;
}

const bool Blackboard::skip_normalized_image_png() const
{
	return _skip_normalized_image_png;
}

const bool Blackboard::skip_normalized_image_png_training() const
{
	return _skip_normalized_image_png_training;
}

const bool Blackboard::skip_tensorboard_logging() const
{
	return _skip_tensorboard_logging;
}

void Blackboard::add_sol_element(const std::string& name)
{
	SolElement s(name);
	_solel.push_last(s);
}

const int Blackboard::num_sol_elements() const
{
	return _solel.N();
}

SolElement& Blackboard::sol_element(const int i)
{
	if ((i<0) || (i>=_solel.N())) {
		cerr << "ERROR: Blackboard: invalid index for getting solution element" << endl;
		exit(1);
	}

	return _solel(i);
}


SolElement* Blackboard::sol_element(const std::string& name)
{
	int i;
	for(i=0; (i<_solel.N()) && (_solel[i].name() != name); i++);
	if (i>=_solel.N())
		return 0;
	else
		return &(_solel(i));
}


const int Blackboard::sol_element_index(const std::string& name) const
{
	int i;
	for(i=0; (i<_solel.N()) && (_solel[i].name() != name); i++);
	if (i>=_solel.N())
		return -1;
	else
		return i;
}


SolElement& Blackboard::last_sol_element()
{
	int i = _solel.N()-1;
	if (i<0) {
		cerr << "ERROR: Blackboard: failed to get last solution element - no elements exist" << endl;
		exit(1);
	}
	return _solel(i);
}



void Blackboard::add_ancestors(const SolElement& se, std::vector<int>& anc)
{
	//cout << "add_ancestors " << se.name() << ": ";
	int m, k;
	for(m=0; m<se.num_attributes(); m++) {
		for(k=0; k<se.attribute(m)->num_rel_solels(); k++) {
			int rsi = se.attribute(m)->rel_solel_index(k);
			if (std::find(anc.begin(), anc.end(), rsi) == anc.end()) {
				anc.push_back(rsi);
				//cout << se.attribute(m)->rel_solel_name(k) << "  ";
				add_ancestors(sol_element(rsi), anc);
			}
		}
	}
	//cout << endl;
}



const int Blackboard::num_groups() const
{
	return _group.N();
}


SEgroup& Blackboard::group(const int i)
{
	if ((i<0) || (i>=_group.N())) {
		cerr << "ERROR: Blackboard: group: invalid index for getting group" << endl;
		exit(1);
	}

	return _group(i);
}


const SEgroup& Blackboard::group_const(const int i) const
{
	if ((i<0) || (i>=_group.N())) {
		cerr << "ERROR: Blackboard: group: invalid index for getting group" << endl;
		exit(1);
	}

	return _group[i];
}


const SEgroup& Blackboard::get_group(const int i) const
{
	if ((i<0) || (i>=_group.N())) {
		cerr << "ERROR: Blackboard: group: invalid index for getting group" << endl;
		exit(1);
	}

	return _group[i];
}


const int Blackboard::find_group(const int solel_ind) const
{
	int g_ind = -1;
	int i, j;
	for(i=0; (i<_group.N())&&(g_ind==-1); i++)
	  for(j=0; j<_group[i].num_sol_els(); j++)
		if (_group[i].sol_el_index(j)==solel_ind)
			g_ind = i;
	return g_ind;
}


void Blackboard::next_group(const int index)
{
	if ((index<-1) || (index>=_group.N())) {
		cerr << "ERROR: Blackboard: next_group: invalid index for next group" << endl;
		exit(1);
	}

	_next_group = index;
}


void Blackboard::next_solel(const int index)
{
	if ((index<-1) || (index>=_solel.N())) {
		cerr << "ERROR: Blackboard: next_solel: invalid index for next solution element" << endl;
		exit(1);
	}

	_next_solel = index;
}


void Blackboard::append_act_rec(const std::string& name, const std::string& type)
{
	ActivationRecord ar(name, type);
	_actrec.push_last(ar);
}

void Blackboard::add_message_to_last_act_rec(const std::string& mess)
{
	int i = _actrec.N()-1;
	//cout << "add_message_to_last_act_rec: " << i << endl;
	//cout << mess << endl;
	if (i<0) {
		cerr << "ERROR: Blackboard: failed to add message to last activation record - no records exist" << endl;
		exit(1);
	}
	_actrec(i).message(mess);
}

const int Blackboard::num_act_recs() const
{
	return _actrec.N();
}

const ActivationRecord& Blackboard::act_rec(const int i)
{
	if ((i<0) || (i>=_actrec.N())) {
		cerr << "ERROR: Blackboard: invalid index for getting activation record" << endl;
		exit(1);
	}

	return _actrec[i];
}


const ActivationRecord* const Blackboard::find_act_rec(const std::string& name, const std::string& type, const std::string& message) const
{
	ActivationRecord* ptr=0;
	int i=_actrec.N()-1;
//if (next_solel()==sol_element_index("right_main_bronchus"))
//{
//cout << "*********In find_act_rec, looking for " << name << endl;
//cout << "SEIndex of right_main_bronchus = " << sol_element_index("right_main_bronchus") << endl;
//}
	while ((i>=0)&&(!ptr)) {
//if ((next_solel()==sol_element_index("right_main_bronchus"))&&(_actrec(i).ks_name().compare("MatchCands")==0)) {
//cout << _actrec(i).ks_name();
//int k=0;
//if (_actrec(i).num_messages()>0) cout << ": " << _actrec(i).message(k);
//cout << endl;
//}
		if ((_actrec[i].ks_name().compare(name)==0) && (_actrec[i].ks_type().compare(type)==0)) {
			if (_actrec[i].find_message(message)!=-1) {
//if (next_solel()==sol_element_index("right_main_bronchus")) {
//cout << "HEEEEEEEEEEEEEEEEEERRRRRRRRRRRRRRE" << endl;
//int k=0;
//cout << _actrec(i).message(k) << endl;
//cout << message << endl;
//}
				ptr = (ActivationRecord*)&(_actrec[i]);
			}
		}
		i--;
	}
//if (next_solel()==sol_element_index("right_main_bronchus")) cout << "*********Exit find_act_rec, looking for " << name << endl;
	return ptr;	
}


const ActivationRecord* const Blackboard::last_act_rec() const
{
	if (!_actrec.N())
		return 0;
	else
		return (ActivationRecord*)&(_actrec[_actrec.N()-1]);	
}


void Blackboard::write_sol_elements(ostream& s)
{
	s << "Number of SolElements: " << num_sol_elements() << endl;
	for(int i=0; i<num_sol_elements(); i++) {
		sol_element(i).write(s);
		s << endl;
		//s << sol_element(i) << endl;
	}
}


void Blackboard::write_groups(ostream& s)
{
	int i, j;
	for(i=0; i<_group.N(); i++) {
		const SEgroup& seg = _group[i];
		s << "Group: " << i << endl;

		for(j=0; j<seg.num_sol_els(); j++)
			cout << "Member: " << sol_element(seg.sol_el_index(j)).name() << endl;

		s << "Priority: " << seg.priority() << endl;

		s << "End: Group: " << i << endl;
	}
}


void Blackboard::write_act_recs(ostream& s)
{
	s << "Number of ActivationRecords: " << num_act_recs() << endl;
	for(int i=0; i<num_act_recs(); i++)
		s << act_rec(i);
}

void Blackboard::overall_search_area(const ROI& r)
{
	_overall_search_area.clear();
	_overall_search_area.OR(r);
}

const ROI& Blackboard::overall_search_area() const
{
	return _overall_search_area;
}


/*
ostream& operator<<(ostream& s, const Blackboard& bb)
{
}
*/
