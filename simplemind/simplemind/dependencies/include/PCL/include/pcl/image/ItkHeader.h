#ifndef PCL_ITKHEADER
#define PCL_ITKHEADER

#ifndef NO_ITK

#include "itkImage.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"

#include "itkTextOutput.h"
#define REDIRECT_ITK_TO_CONSOLE itk::OutputWindow::SetInstance(itk::TextOutput::New())

#if ITK_VERSION_MAJOR>=4
#include<itkBioRadImageIOFactory.h>
#include<itkBMPImageIOFactory.h>
#include<itkBruker2dseqImageIOFactory.h>
#include<itkGDCMImageIOFactory.h>
#include<itkGE4ImageIOFactory.h>
#include<itkGE5ImageIOFactory.h>
#include<itkGEAdwImageIOFactory.h>
#include<itkGiplImageIOFactory.h>
#include<itkHDF5ImageIOFactory.h>
#include<itkJPEG2000ImageIOFactory.h>
#include<itkJPEGImageIOFactory.h>
#include<itkLSMImageIOFactory.h>
#include<itkMetaImageIOFactory.h>
#include<itkMRCImageIOFactory.h>
#include<itkNiftiImageIOFactory.h>
#include<itkNrrdImageIOFactory.h>
#include<itkPNGImageIOFactory.h>
#include<itkSiemensVisionImageIOFactory.h>
#include<itkStimulateImageIOFactory.h>
#include<itkTIFFImageIOFactory.h>
#include<itkVoxBoCUBImageIOFactory.h>
#include<itkVTKImageIOFactory.h>
#endif

static void RegisterImageIoFactory()
{
#if ITK_VERSION_MAJOR>=4
	static bool init = false;
	if (init) return;
	itk::BioRadImageIOFactory::RegisterOneFactory();
	itk::BMPImageIOFactory::RegisterOneFactory();
	itk::Bruker2dseqImageIOFactory::RegisterOneFactory();
	itk::GDCMImageIOFactory::RegisterOneFactory();
	itk::GE4ImageIOFactory::RegisterOneFactory();
	itk::GE5ImageIOFactory::RegisterOneFactory();
	itk::GEAdwImageIOFactory::RegisterOneFactory();
	itk::GiplImageIOFactory::RegisterOneFactory();
	itk::HDF5ImageIOFactory::RegisterOneFactory();
	itk::JPEG2000ImageIOFactory::RegisterOneFactory();
	itk::JPEGImageIOFactory::RegisterOneFactory();
	itk::LSMImageIOFactory::RegisterOneFactory();
	itk::MetaImageIOFactory::RegisterOneFactory();
	itk::MRCImageIOFactory::RegisterOneFactory();
	itk::NiftiImageIOFactory::RegisterOneFactory();
	itk::NrrdImageIOFactory::RegisterOneFactory();
	itk::PNGImageIOFactory::RegisterOneFactory();
	itk::SiemensVisionImageIOFactory::RegisterOneFactory();
	itk::StimulateImageIOFactory::RegisterOneFactory();
	itk::TIFFImageIOFactory::RegisterOneFactory();
	itk::VoxBoCUBImageIOFactory::RegisterOneFactory();
	itk::VTKImageIOFactory::RegisterOneFactory();
	init = true;
#endif
}

#endif
#endif