#ifndef PCL_ITERATOR_MACRO
#define PCL_ITERATOR_MACRO

#define pcl_ForX(var, minp, maxp) for (int var=minp.x(); var<=maxp.x(); ++var)
#define pcl_ForY(var, minp, maxp) for (int var=minp.y(); var<=maxp.y(); ++var)
#define pcl_ForZ(var, minp, maxp) for (int var=minp.z(); var<=maxp.z(); ++var)
#define pcl_ForXY(var_x, var_y, minp, maxp) for (int var_y=minp.y(); var_y<=maxp.y(); ++var_y) for (int var_x=minp.x(); var_x<=maxp.x(); ++var_x)
#define pcl_ForXYZ(var_x, var_y, var_z, minp, maxp) for (int var_z=minp.z(); var_z<=maxp.z(); ++var_z) for (int var_y=minp.y(); var_y<=maxp.y(); ++var_y) for (int var_x=minp.x(); var_x<=maxp.x(); ++var_x)

#define pcl_ForEX(var, minp, maxp) for (var=minp.x(); var<=maxp.x(); ++var)
#define pcl_ForEY(var, minp, maxp) for (var=minp.y(); var<=maxp.y(); ++var)
#define pcl_ForEZ(var, minp, maxp) for (var=minp.z(); var<=maxp.z(); ++var)
#define pcl_ForEXY(var_x, var_y, minp, maxp) for (var_y=minp.y(); var_y<=maxp.y(); ++var_y) for (var_x=minp.x(); var_x<=maxp.x(); ++var_x)
#define pcl_ForEXYZ(var_x, var_y, var_z, minp, maxp) for (var_z=minp.z(); var_z<=maxp.z(); ++var_z) for (var_y=minp.y(); var_y<=maxp.y(); ++var_y) for (var_x=minp.x(); var_x<=maxp.x(); ++var_x)

#define pcl_ForIterator(iter) for ((iter).begin(); !(iter).end(); (iter).next())
#define pcl_ForIterator2(iter1,iter2) for ((iter1).begin(), (iter2).begin(); !(iter1).end() && !(iter2).end(); (iter1).next(), (iter2).next())
#define pcl_ForIterator3(iter1,iter2,iter3) for ((iter1).begin(), (iter2).begin(), (iter3).begin(); !(iter1).end() && !(iter2).end() && !(iter3).end(); (iter1).next(), (iter2).next(), (iter3).next())

#endif