#ifndef __CYTHON_ELEMENT__
#define __CYTHON_ELEMENT__

#include <pcl/image.h>
#include <pcl/iterator.h>

pcl::iterator::ImageNeighborIterator::OffsetListPointer getEllipsoidElement(int rx, int ry, int rz, bool skip_origin)
{
	return pcl::iterator::ImageNeighborIterator::CreateEllipsoidOffset(rx, ry, rz, skip_origin);
}

pcl::iterator::ImageNeighborIterator::OffsetListPointer getCubeElement(int x, int y, int z, bool skip_origin)
{
	return pcl::iterator::ImageNeighborIterator::CreateOffsetFromRegion(pcl::Region3D<int>(
		pcl::Point3D<int>(-x,-y,-z),
		pcl::Point3D<int>(x,y,z)
	), skip_origin);
}

pcl::iterator::ImageNeighborIterator::OffsetListPointer getConnect6Element(bool skip_origin)
{
	return pcl::iterator::ImageNeighborIterator::CreateConnect6Offset(skip_origin);
}

#endif