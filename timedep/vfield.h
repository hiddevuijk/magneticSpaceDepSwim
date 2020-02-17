#ifndef GUARD_VFIELD_H
#define GUARD_VFIELD_H

#include "xyz.h"

#include <iostream>
#include <vector>

class VfieldAll {
public:
	virtual double f(const XYZ& r, double t) = 0; 
    virtual double get_tau() const = 0;
protected:
	double w;
	double L;

    double tau;
};

class VNone: public VfieldAll {
public:
	VNone() {w = 0; L = 0; tau = 0;}
	double f(const XYZ& r, double t)
			{return 1;}

    double get_tau() const { return tau;}
};


class VSine: public VfieldAll {
public:
	VSine() {L = 0; }
	VSine(double ww, double ttau) { w=ww; tau = ttau; }

	double f(const XYZ& r, double t) {
			return std::sin(w*(r.x-t*tau) ); }

    double get_tau() const { return tau;}

};

class VSine2: public VfieldAll {
public:
	VSine2() {L = 0; }
	VSine2(double ww, double ttau) { w=ww; tau = ttau;}

	double f(const XYZ& r, double t) {
			return 1+std::sin(w*(r.x - t*tau) ); }
    double get_tau() const { return tau;}

};

class VGauss: public VfieldAll {
public:
	VGauss() {L = 0; }
	VGauss(double ww, double LL, double ttau) {L = LL; w=ww; tau=ttau;}

	double f(const XYZ& r, double t)
    {
		return std::exp(-w*(r.x-0.5*L - t*tau)*(r.x-0.5*L - t*tau) ) - std::exp(-w*L*L/4);
    }
    double get_tau() const { return tau;}

};


class VStep: public VfieldAll {
public:
	VStep() {L = 0; }
	VStep(double ww, double LL, double ttau) {L = LL; w=ww; tau=ttau; }

	double f(const XYZ& r, double t)
    {
		return 0.5*(std::tanh( w*( r.x - 0.25*L - t*tau ) )*std::tanh( w*(0.75*L - r.x  - t*tau) ) - std::tanh( -0.25*w*L)*std::tanh(0.75*w*L) );
    }
    double get_tau() const { return tau;}

};


class Vfield {
public:
	Vfield();

	Vfield(double w, double L, double tau,
			std::string VType);

	double get_field(const XYZ& r, double t) const
		{return vfield_ptr->f(r, t);}

    double get_tau() const { return vfield_ptr->get_tau(); }

private:
	VNone vnone;
	VSine vsine;
	VSine2 vsine2;
    VGauss vgauss;
    VStep vstep;
	
	VfieldAll *vfield_ptr;
};



Vfield::Vfield()
: vnone(), vsine(), vsine2(), vgauss(), vstep()
{
	vfield_ptr = &vnone;
}


Vfield::Vfield(double w, double L, double tau,
			std::string VType)
:
	vnone(), vsine(w, tau), vsine2(w, tau), vgauss(w,L, tau), vstep(w,L, tau)
{
	if( VType == "none") {
		vfield_ptr = &vnone;	
	} else if( VType == "sine" ) {
		vfield_ptr = &vsine;
	} else if( VType == "sine_shift" ) {
		vfield_ptr = &vsine2;
	} else if( VType == "gauss" ) {
		vfield_ptr = &vgauss;
	} else if( VType == "step" ) {
		vfield_ptr = &vstep;
	} else {
		std::cerr << "ERROR: " << VType
			<< " is not a valid option.\n";
	}


}

#endif

