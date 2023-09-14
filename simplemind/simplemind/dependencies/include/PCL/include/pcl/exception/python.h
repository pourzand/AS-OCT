#include <pcl/exception.h>
#include <Python.h>

void raisePyError()
{
	PyGILState_STATE gstate = PyGILState_Ensure();
	try {
		throw;
	} catch (pcl::Exception& e) {
		std::stringstream ss;
		ss << e;
		PyErr_SetString(PyExc_RuntimeError, ss.str().c_str());
	} catch (const std::exception& e) {
		PyErr_SetString(PyExc_RuntimeError, e.what());
	} catch (...) {
		PyErr_SetString(PyExc_RuntimeError, "Unknown error");
	}
	PyGILState_Release(gstate);
}