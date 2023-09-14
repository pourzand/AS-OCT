#include "SegParam.h"

using std::endl;

SegParam::SegParam()
	: Attribute()
{
}

SegParam::SegParam(const SegParam& s)
	: Attribute(s)
{
}

SegParam::~SegParam()
{
}


AddMatchedCandidates::AddMatchedCandidates(const std::string& rel_solel_name, const int rel_solel_ind)
	: SegParam()
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}


AddMatchedCandidates::AddMatchedCandidates(const Darray<std::string*>& rel_solel_name, const Darray<int>& rel_solel_ind)
	: SegParam()
{
	for(int i=0; i<rel_solel_ind.N(); i++)
		add_rel_solel(*(rel_solel_name[i]), rel_solel_ind[i]);
}


AddMatchedCandidates::AddMatchedCandidates(const AddMatchedCandidates& sca)
	: SegParam (sca)
{
}


AddMatchedCandidates::~AddMatchedCandidates()
{
}


void AddMatchedCandidates::write(ostream& s) const
{
	_write_start_attribute(s);
	_write_end_attribute(s);
}



DistanceMap25DPercMax::DistanceMap25DPercMax(const std::string& rel_solel_name, const int rel_solel_ind, const float dist_threshold, const float percentage, const int hu_diff_threshold, const float max_z_dist)
	: SegParam(), _dist_threshold(dist_threshold), _perc(percentage), _hu_diff_threshold(hu_diff_threshold), _max_z_dist(max_z_dist)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

DistanceMap25DPercMax::DistanceMap25DPercMax(const DistanceMap25DPercMax& tr)
	: SegParam(tr), _dist_threshold(tr._dist_threshold), _perc(tr._perc), _hu_diff_threshold(tr._hu_diff_threshold), _max_z_dist(tr._max_z_dist)
{
}


DistanceMap25DPercMax::~DistanceMap25DPercMax()
{
}

const float DistanceMap25DPercMax::dist_threshold() const
{
	return _dist_threshold;
}

const float DistanceMap25DPercMax::percentage() const
{
	return _perc;
}

const int DistanceMap25DPercMax::hu_diff_threshold() const
{
	return _hu_diff_threshold;
}

const float DistanceMap25DPercMax::max_z_distance() const
{
	return _max_z_dist;
}

void DistanceMap25DPercMax::write(ostream& s) const
{
	_write_start_attribute(s);
	s << "Distance threshold: " << _dist_threshold << endl;
	s << "Percentage: " << _perc << endl;
	s << "Maximum z-distance: " << _max_z_dist << endl;
	_write_end_attribute(s);
}



DistanceMapRegionGrowing::DistanceMapRegionGrowing(const std::string& rel_solel_name, const int rel_solel_ind, const float dist_threshold, const float percentage)
	: SegParam(), _dist_threshold(dist_threshold), _perc(percentage)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

DistanceMapRegionGrowing::DistanceMapRegionGrowing(const DistanceMapRegionGrowing& tr)
	: SegParam(tr), _dist_threshold(tr._dist_threshold), _perc(tr._perc)
{
}


DistanceMapRegionGrowing::~DistanceMapRegionGrowing()
{
}

const float DistanceMapRegionGrowing::dist_threshold() const
{
	return _dist_threshold;
}

const float DistanceMapRegionGrowing::percentage() const
{
	return _perc;
}

void DistanceMapRegionGrowing::write(ostream& s) const
{
	_write_start_attribute(s);
	s << "Distance threshold: " << _dist_threshold << endl;
	s << "Percentage: " << _perc << endl;
	_write_end_attribute(s);
}



DistanceMapWatershed::DistanceMapWatershed(const std::string& rel_solel_name, const int rel_solel_ind, const float dist_threshold)
	: SegParam(), _dist_threshold(dist_threshold)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

DistanceMapWatershed::DistanceMapWatershed(const DistanceMapWatershed& tr)
	: SegParam(tr), _dist_threshold(tr._dist_threshold)
{
}


DistanceMapWatershed::~DistanceMapWatershed()
{
}

const float DistanceMapWatershed::dist_threshold() const
{
	return _dist_threshold;
}

void DistanceMapWatershed::write(ostream& s) const
{
	_write_start_attribute(s);
	s << "Distance threshold: " << _dist_threshold << endl;
	_write_end_attribute(s);
}



GrowPartSolid::GrowPartSolid(const std::string& rel_solel_name, const int rel_solel_ind, const float percentage)
	: SegParam(), _perc(percentage)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

GrowPartSolid::GrowPartSolid(const GrowPartSolid& tr)
	: SegParam(tr), _perc(tr._perc)
{
}

GrowPartSolid::~GrowPartSolid()
{
}

const float GrowPartSolid::percentage() const
{
	return _perc;
}

void GrowPartSolid::write(ostream& s) const
{
	_write_start_attribute(s);
	s << "Percentage: " << _perc << endl;
	_write_end_attribute(s);
}



IncludeAllVoxels::IncludeAllVoxels()
	: SegParam()
{
}


IncludeAllVoxels::IncludeAllVoxels(const IncludeAllVoxels& u)
	: SegParam(u)
{
}


IncludeAllVoxels::~IncludeAllVoxels()
{
}


void IncludeAllVoxels::write(ostream& s) const
{
	_write_start_attribute(s);
	_write_end_attribute(s);
}




IntensitySubregion::IntensitySubregion(const std::string& rel_solel_name, const int rel_solel_ind)
	: SegParam()
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

IntensitySubregion::IntensitySubregion(const IntensitySubregion& tr)
	: SegParam(tr)
{
}

IntensitySubregion::~IntensitySubregion()
{
}

void IntensitySubregion::write(ostream& s) const
{
	_write_start_attribute(s);
	_write_end_attribute(s);
}



LineToDots::LineToDots(const std::string& direction, const float spacing, const float check_distance)
	: SegParam(), _direction(direction), _spacing(spacing), _check_distance(check_distance)
{
}

LineToDots::LineToDots(const LineToDots& tr)
	: SegParam(tr), _direction(tr._direction), _spacing(tr._spacing), _check_distance(tr._check_distance)
{
}


LineToDots::~LineToDots()
{
}


const float LineToDots::spacing() const
{
	return _spacing;
}

const float LineToDots::check_distance() const
{
	return _check_distance;
}

void LineToDots::write(ostream& s) const
{
	_write_start_attribute(s);
	s << "direction: " << _direction << endl;
	s << "spacing: " << _spacing << endl;
	s << "check_distance: " << _check_distance << endl;
	_write_end_attribute(s);
}



MaxCostPath::MaxCostPath()
	: SegParam()
{
}


MaxCostPath::MaxCostPath(const MaxCostPath& u)
	: SegParam(u)
{
}


MaxCostPath::~MaxCostPath()
{
}


void MaxCostPath::write(ostream& s) const
{
	_write_start_attribute(s);
	_write_end_attribute(s);
}



MinNumVoxels::MinNumVoxels(const int minNum)
	: SegParam(), _min_num_voxels(minNum)
{
}

MinNumVoxels::MinNumVoxels(const MinNumVoxels& tr)
	: SegParam(tr), _min_num_voxels(tr._min_num_voxels)
{
}


MinNumVoxels::~MinNumVoxels()
{
}


const int MinNumVoxels::minNumVoxels() const
{
	return _min_num_voxels;
}


void MinNumVoxels::write(ostream& s) const
{
	_write_start_attribute(s);
	s << "Min_num_voxels: " << _min_num_voxels << endl;
	_write_end_attribute(s);
}


/*
NeuralNetKeras::NeuralNetKeras(const std::string& cnn_arch, const std::string& weights_path, const int image_rows, const int image_cols, const int num_channels, const std::string* intensity_normalization, const float learning_rate)
	: SegParam(), _cnn_arch(cnn_arch), _weights_path(weights_path), _image_rows(image_rows), _image_cols(image_cols), _num_channels(num_channels), _learning_rate(learning_rate)
{
	_intensity_normalization = new std::string [_num_channels];
	int j;
	for (j=0; j<_num_channels; j++)
		_intensity_normalization[j].assign(intensity_normalization[j]);
}
*/

NeuralNetKeras::NeuralNetKeras(const std::string& cnn_arch, const std::string& weights_path, const int image_rows, const int image_cols)
	: SegParam(), _cnn_arch(cnn_arch), _weights_path(weights_path), _image_rows(image_rows), _image_cols(image_cols)
{
	//youngwon edited
	_image_slices = 0;
	set_parameter("model_info", "architecture", _cnn_arch);
	_update_img_shape_ini_param();
}

//youngwon edited
NeuralNetKeras::NeuralNetKeras(const std::string& cnn_arch, const std::string& weights_path, const int image_rows, const int image_cols, const int image_slices)
	: SegParam(), _cnn_arch(cnn_arch), _weights_path(weights_path), _image_rows(image_rows), _image_cols(image_cols), _image_slices(image_slices)
{
	set_parameter("model_info", "architecture", _cnn_arch);
	_update_img_shape_ini_param();
}

NeuralNetKeras::NeuralNetKeras(const float learning_rate)
	: SegParam(), _learning_rate(learning_rate)
{
	std::ostringstream ss;
	ss << _learning_rate;
	set_parameter("model_info", "lr", ss.str());
}

//youngwon : TOOD: copy constructure for 3D input how to..?
NeuralNetKeras::NeuralNetKeras(const NeuralNetKeras& tr)
	: SegParam(tr), _cnn_arch(tr._cnn_arch), _weights_path(tr._weights_path), _image_rows(tr._image_rows), _image_cols(tr._image_cols), _normalization_bits_used(tr._normalization_bits_used), _learning_rate(tr._learning_rate), _intensity_normalization(tr._intensity_normalization), _parameters(tr._parameters)
{
	set_parameter("model_info", "architecture", _cnn_arch);
	std::ostringstream ss;
	ss << _learning_rate;
	set_parameter("model_info", "lr", ss.str());
	_update_intensity_norm_ini_param();
	_update_img_shape_ini_param();
}
/*
NeuralNetKeras::NeuralNetKeras(const NeuralNetKeras& tr)
	: SegParam(tr), _cnn_arch(tr._cnn_arch), _weights_path(tr._weights_path), _image_rows(tr._image_rows), _image_cols(tr._image_cols), _num_channels(tr._num_channels), _normalization_bits_used(tr._normalization_bits_used), _learning_rate(tr._learning_rate)
{
	_intensity_normalization = new std::string [_num_channels];
	int j;
	for (j=0; j<_num_channels; j++)
		_intensity_normalization[j].assign(tr._intensity_normalization[j]);
}
*/

NeuralNetKeras::~NeuralNetKeras()
{
}

void NeuralNetKeras::set_cnn_arch(const std::string& cnn_arch) { 
	_cnn_arch.assign(cnn_arch); 
	set_parameter("model_info", "architecture", _cnn_arch);
}

void NeuralNetKeras::set_weights_path(const std::string& weights_path) { 
	_weights_path.assign(weights_path); 
}

void NeuralNetKeras::set_rows_cols(const int rows, const int cols) {
	_image_rows = rows;
	_image_cols = cols;
	_update_img_shape_ini_param();
}

//youngwon: needed?
// void NeuralNetKeras::set_rows_cols_slices(const int rows, const int cols, const int slices) {
// 	_image_rows = rows;
// 	_image_cols = cols;
// 	_image_slices = slices;
// 	_update_img_shape_ini_param();
// }

const int NeuralNetKeras::image_rows() const
{
	return _image_rows;
}

const int NeuralNetKeras::image_cols() const
{
	return _image_cols;
}

//youngwon edited
const int NeuralNetKeras::image_slices() const
{
	return _image_slices;
}

void NeuralNetKeras::add_to_channel(const int channel_index, const std::string& intensity_normalization) 
{
	while (channel_index >= _intensity_normalization.size()) _intensity_normalization.push_back("empty");
	if (_intensity_normalization[channel_index].compare("empty")==0)
		_intensity_normalization[channel_index].assign(intensity_normalization);
	else {
		_intensity_normalization[channel_index].append("_");
		_intensity_normalization[channel_index].append(intensity_normalization);
	}
	_update_intensity_norm_ini_param();
	_update_img_shape_ini_param();
}
/*
void NeuralNetKeras::set_channels(const int num_channels, const std::string* intensity_normalization) {
	_num_channels = num_channels;
	_intensity_normalization = new std::string [_num_channels];
	int j;
	for (j=0; j<_num_channels; j++)
		_intensity_normalization[j].assign(intensity_normalization[j]);
}
*/

const int NeuralNetKeras::num_channels() const
{
	//return _num_channels;
	return _intensity_normalization.size();
}

const int NeuralNetKeras::num_augmentations() const
{
	return _augmentation.size();
}

// const std::string& const NeuralNetKeras::intensity_normalization(const int j) const //MWW 082920 
const std::string& NeuralNetKeras::intensity_normalization(const int j) const //MWW 082920 
{ 
	assert((j>=0) && (j<_intensity_normalization.size()));
	//assert((j>=0) && (j<_num_channels));
	return _intensity_normalization[j]; 
}

void NeuralNetKeras::set_normalization_bits_used(const std::vector<bool> bits) {
	int i;
	for (i=0; i<bits.size(); i++) {
		if (i>=_normalization_bits_used.size()) {
			_normalization_bits_used.push_back(bits[i]);
		}
		else if (bits[i]) _normalization_bits_used[i] = true;
	}
}

const bool NeuralNetKeras::no_normalization_bits_used() const {
	bool used = false;
	int i;
	for (i=0; (i<_normalization_bits_used.size()) && !used; i++)
		used = _normalization_bits_used[i];
	return !used;
}


void NeuralNetKeras::add_to_augmentation(const int stack_index, const std::string& augmentation) 
{
	while (stack_index >= _augmentation.size()) _augmentation.push_back("empty");
	if (_augmentation[stack_index].compare("empty")==0)
		_augmentation[stack_index].assign(augmentation);
	else {
		_augmentation[stack_index].append("_");
		_augmentation[stack_index].append(augmentation);
	}
	_update_augmentation_ini_param();
}

const std::string& NeuralNetKeras::augmentation(const int j) const 
{ 
	assert((j>=0) && (j<_augmentation.size()));
	return _augmentation[j]; 
}

void NeuralNetKeras::set_augmentation_bits_used(const std::vector<bool> bits) {
	int i;
	for (i=0; i<bits.size(); i++) {
		if (i>=_augmentation_bits_used.size()) {
			_augmentation_bits_used.push_back(bits[i]);
		}
		else if (bits[i]) _augmentation_bits_used[i] = true;
	}
}

const bool NeuralNetKeras::no_augmentation_bits_used() const {
	bool used = false;
	int i;
	for (i=0; (i<_augmentation_bits_used.size()) && !used; i++)
		used = _augmentation_bits_used[i];
	return !used;
}

void NeuralNetKeras::set_learning_rate(const float learning_rate)
{
	_learning_rate = learning_rate;
	std::ostringstream ss;
	ss << _learning_rate;
	set_parameter("model_info", "lr", ss.str());
}

const float NeuralNetKeras::learning_rate() const
{
	return _learning_rate;
}

void NeuralNetKeras::set_parameter(const std::string group, const std::string name, const std::string value)
{
	_parameters[group][name] = value;
}


void NeuralNetKeras::append_parameter(const std::string group, const std::string name, const std::string value) {
	std::string& s = _parameters[group][name];
	if (!s.empty())
		s.append(", ");
	s.append(value);
	//std::cout << _parameters[group][name] << endl;
}

const void NeuralNetKeras::write_ini(const std::string output_filename, const std::string default_param_filename) const {
	//std::cout << "NeuralNetKeras::write_ini " << default_param_filename << endl;
	mINI::INIFile def_file(default_param_filename);
	mINI::INIStructure ini;
	def_file.read(ini);

	for (auto pi = _parameters.begin() ; pi != _parameters.end(); pi++) {
		auto const& pv = *pi;
		auto const& section = pv.first;
		auto const& collection = pv.second;
		for (auto kvi = collection.begin() ; kvi != collection.end(); kvi++) {
			auto const& kvv = *kvi;
			auto key = kvv.first;
			auto value = kvv.second;
			ini[section][key] = value;
		}
	}

	mINI::INIFile file(output_filename);
	file.generate(ini);
}

const void NeuralNetKeras::set_ini(mINI::INIStructure& ini) const {
	for (auto pi = _parameters.begin() ; pi != _parameters.end(); pi++) {
		auto const& pv = *pi;
		auto const& section = pv.first;
		auto const& collection = pv.second;
		for (auto kvi = collection.begin() ; kvi != collection.end(); kvi++) {
			auto const& kvv = *kvi;
			auto key = kvv.first;
			auto value = kvv.second;
			ini[section][key] = value;
		}
	}
}

void NeuralNetKeras::write(ostream& s) const
{
	_write_start_attribute(s);
	s << "cnn_arch: " << _cnn_arch << endl;
	s << "image_rows: " << _image_rows << endl;
	s << "image_cols: " << _image_cols << endl;
	//s << "num_channels: " << _num_channels << endl;
	s << "num_channels: " << _intensity_normalization.size() << endl;
	s << "intensity_normalization: ";
	int j;
	//for (j=0; j<_num_channels; j++)
	for (j=0; j<_intensity_normalization.size(); j++)
		s << _intensity_normalization[j] << " ";
	s << endl;
	s << "learning_rate: " << _learning_rate << endl;
	
	for (auto di = _parameters.begin() ; di != _parameters.end(); di++) {
		auto const& dv = *di;
		auto const& section = dv.first;
		auto const& collection = dv.second;
		//s << ""[" << section << "]" << endl;
		for (auto kvi = collection.begin() ; kvi != collection.end(); kvi++) {
			auto const& kvv = *kvi;
			auto key = kvv.first;
			auto value = kvv.second;
			s << "parameter: " << section << " " << key << " " << value << endl;
		}
	}

	_write_end_attribute(s);
}

void NeuralNetKeras::_update_intensity_norm_ini_param() {
	std::string instr;
	for (int k=0; k<num_channels(); k++) {
		// youngwon edited
		// if ((k>0) && (k<(num_channels()-1))) instr = instr + ", ";
		if ((k>0) && (k<(num_channels()))) instr = instr + ", ";
		instr = instr + intensity_normalization(k);
	}
	set_parameter("model_info", "intensity_norm", instr);
}

void NeuralNetKeras::_update_img_shape_ini_param() {
	std::ostringstream ss;
	if (_image_slices <= 0) {
		ss << _image_rows << ", " << _image_cols << ", " << num_channels();
		// std::cout << "update 2D input" << endl;
	} else {
		ss << _image_rows << ", " << _image_cols << ", " << _image_slices << ", " << num_channels();
		// std::cout << "update 3D input" << endl;
	}
	set_parameter("model_info", "img_shape", ss.str());
}

void NeuralNetKeras::_update_augmentation_ini_param() {
	std::string instr;
	for (int k=0; k<num_augmentations(); k++) {
		if ((k>0) && (k<(num_augmentations()))) instr = instr + ", ";
		instr = instr + augmentation(k);
	}
	set_parameter("model_info", "augmentation", instr);
}

PlatenessThreshRegGrow::PlatenessThreshRegGrow(const float low, const float high)
	: SegParam(), _low(low), _high(high)
{
}

PlatenessThreshRegGrow::PlatenessThreshRegGrow(const PlatenessThreshRegGrow& tr)
	: SegParam(tr), _low(tr._low), _high(tr._high)
{
}


PlatenessThreshRegGrow::~PlatenessThreshRegGrow()
{
}

const float PlatenessThreshRegGrow::low() const
{
	return _low;
}

const float PlatenessThreshRegGrow::high() const
{
	return _high;
}

void PlatenessThreshRegGrow::write(ostream& s) const
{
	_write_start_attribute(s);
	s << "Range (low high): " << _low << " " << _high << endl;
	_write_end_attribute(s);
}



SameCandidatesAs::SameCandidatesAs(const std::string& rel_solel_name, const int rel_solel_ind)
	: SegParam()
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}


SameCandidatesAs::SameCandidatesAs(const Darray<std::string*>& rel_solel_name, const Darray<int>& rel_solel_ind)
	: SegParam()
{
	for(int i=0; i<rel_solel_ind.N(); i++)
		add_rel_solel(*(rel_solel_name[i]), rel_solel_ind[i]);
}


SameCandidatesAs::SameCandidatesAs(const SameCandidatesAs& sca)
	: SegParam (sca)
{
}


SameCandidatesAs::~SameCandidatesAs()
{
}


void SameCandidatesAs::write(ostream& s) const
{
	_write_start_attribute(s);
	_write_end_attribute(s);
}


SegmentIn2D::SegmentIn2D()
	: SegParam()
{
}


SegmentIn2D::SegmentIn2D(const SegmentIn2D& u)
	: SegParam(u)
{
}


SegmentIn2D::~SegmentIn2D()
{
}


void SegmentIn2D::write(ostream& s) const
{
	_write_start_attribute(s);
	_write_end_attribute(s);
}


ThreshRangeGL::ThreshRangeGL(const int lowGL, const int highGL)
	: SegParam(), _low_GL(lowGL), _high_GL(highGL)
{
}

ThreshRangeGL::ThreshRangeGL(const ThreshRangeGL& tr)
	: SegParam(tr), _low_GL(tr._low_GL), _high_GL(tr._high_GL)
{
}


ThreshRangeGL::~ThreshRangeGL()
{
}


const float ThreshRangeGL::lowGL() const
{
	return _low_GL;
}

const float ThreshRangeGL::highGL() const
{
	return _high_GL;
}


void ThreshRangeGL::write(ostream& s) const
{
	_write_start_attribute(s);
	s << "GL_range: " << _low_GL << " to " << _high_GL << endl;
	_write_end_attribute(s);
}


UseSubSampledImage::UseSubSampledImage(const int x_step, const int y_step, const int z_step)
	: SegParam(), _x_step(x_step), _y_step(y_step), _z_step(z_step)
{
}


UseSubSampledImage::UseSubSampledImage(const UseSubSampledImage& u)
	: SegParam(u), _x_step(u._x_step), _y_step(u._y_step), _z_step(u._z_step)
{
}


UseSubSampledImage::~UseSubSampledImage()
{
}


void UseSubSampledImage::write(ostream& s) const
{
	_write_start_attribute(s);
	s << "X-step: " << _x_step << endl;
	s << "Y-step: " << _y_step << endl;
	s << "Z-step: " << _z_step << endl;
	_write_end_attribute(s);
}

