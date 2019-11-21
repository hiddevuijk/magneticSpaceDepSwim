#ifndef GUARD_VFIELD_H
#define GUARD_VFIELD_H

#include "xyz.h"

#include <iostream>
#include <vector>

class VfieldAll {
public:
	virtual double f(const XYZ& r) = 0; 

protected:
	double w;
	double L;
};

class VNone: public VfieldAll {
public:
	VNone() {w = 0; L = 0;}
	double f(const XYZ& r)
			{return 1;}
};


class VSine: public VfieldAll {
public:
	VSine() {L = 0; }
	VSine(double ww) { w=ww; }

	double f(const XYZ& r) {
			return std::sin(w*r.x); }

};


class Vfield {
public:
	Vfield();

	Vfield(double w, double L,
			std::string VType);

	double get_field(const XYZ& r) const
		{return vfield_ptr->f(r);}

private:
	VNone vnone;
	VSine vsine;
	
	VfieldAll *vfield_ptr;
};



Vfield::Vfield()
:
	vnone(), vsine()
{
	vfield_ptr = &vnone;
}


Vfield::Vfield(double w, double L,
			std::string VType)
:
	vnone(), vsine(w)
{
	if( VType == "none") {
		vfield_ptr = &vnone;	
	} else if( VType == "sine" ) {
		vfield_ptr = &vsine;
	} else {
		std::cerr << "ERROR: " << VType
			<< " is not a valid option.\n";
	}


}

#endif

