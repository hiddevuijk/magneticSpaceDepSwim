#ifndef GUARD_SYSTEM_H
#define GUARD_SYSTEM_H



#include "ConfigFile.h"

#include "xyz.h"
#include "bfield.h"
#include "walls.h"

#include <iostream>
#include <vector>
#include <boost/random.hpp>

namespace system_func {
template<class RNDIST> 
inline void xyz_random_normal(XYZ &r, RNDIST &rndist) {
	r.x = rndist();
	r.y = rndist();
	r.z = rndist();
}

};

struct System {
public:
	// initialize from ConfigFile object
	System(ConfigFile config);

	// random number generator
	const boost::normal_distribution<double> ndist;
	const boost::uniform_real<double> udist;

	int seed;
	boost::mt19937 rng;		
	boost::variate_generator<boost::mt19937&,
		boost::normal_distribution<double> > rndist;
	boost::variate_generator<boost::mt19937&,
		boost::uniform_real<double> > rudist;

	// fixed system parameters
	unsigned int N;
	double L;
	double m;
	double v0;
	double Dr;
	double dt;

	double sqrt_2dt;
	double sqrt_2dt_Dr;

	// state of the system
	double t;
	std::vector<XYZ> r;
	std::vector<XYZ> dr;
	std::vector<XYZ> v;
	std::vector<XYZ> p;
	
	// magnetic field	
	Bfield bfield;

	// walls 
	Wall wall;

	// force on particle i due to the walls
	std::vector<XYZ> Fwall;

	// initialize with random coordinates.
	void init_random();
	void init_random(double);

	// increment time
	void step();

	bool check_x_in_box();
	bool check_y_in_box();
	bool check_z_in_box();

	void write(const char* outname);
	
	// temporary containers
	double Bri;
	XYZ xi,eta,dp,dv;	


};

void System::step()
{

	for(unsigned int i=0;i<N;++i) {
		
		Bri = bfield.get_field(r[i]);
		
		r[i] += v[i]*dt;

		system_func::xyz_random_normal(xi,rndist);
		xi *= sqrt_2dt;

		dv.x = ( Bri*v[i].y*dt - v[i].x*dt + 
				Fwall[i].x*dt + v0*p[i].x*dt + xi.x)/m;	
		dv.y = (-Bri*v[i].x*dt - v[i].y*dt + 
				Fwall[i].y*dt + v0*p[i].y*dt + xi.y)/m;	
		dv.z = (-v[i].z*dt + Fwall[i].z*dt +
				v0*p[i].z*dt + xi.z)/m;

		v[i] += dv;
	
		Fwall[i] = wall.wallForce(r[i]);


		if( v0 > 0) {
			
			system_func::xyz_random_normal(eta,rndist);
			eta *= sqrt_2dt_Dr;

			dp = xyz::cross(eta,p[i]);

			p[i] += dp;
			p[i].normalize();
		}
	}

	t += dt;
}



System::System(ConfigFile config)
:
	ndist(0.,1.),udist(0,1),
	seed(config.read<unsigned int>("seed")),
	rng(seed), rndist(rng,ndist), rudist(rng,udist),
	bfield(config.read<double>("B"),
				config.read<double>("w"),
				config.read<double>("L"),
				config.read<std::string>("BType") ),	
	wall(config.read<double>("sigmaW"),
		config.read<double>("epsilonW"),
		config.read<double>("L"),
		config.read<string>("WallType"))
{
	XYZ rr;

	// init parameters
	N = config.read<unsigned int>("N");
	L = config.read<double>("L");
	m = config.read<double>("m");
	v0 = config.read<double>("v0");
	Dr = config.read<double>("Dr");
	dt = config.read<double>("dt");
	sqrt_2dt = std::sqrt(2*dt);
	sqrt_2dt_Dr = std::sqrt(2*dt*Dr);

	// init state
	t = 0.0;
	r = std::vector<XYZ>(N);
	dr = std::vector<XYZ>(N);
	v = std::vector<XYZ>(N);
	p = std::vector<XYZ>(N);


	// wall force object
	Fwall = std::vector<XYZ>(N,XYZ(0,0,0));
}

void System::init_random()
{
	double l = wall.get_sigma()*pow(2.,1./6.);
	XYZ zeta;
	double d;

	// node_pd: nodes per dim
	int node_pd = ceil(pow(1.*N,1./3));
	int Nnodes = node_pd*node_pd*node_pd;
	double node_dist = 0;

	if( wall.get_sigma() > 0.0001) {
		node_dist = (L-2*l)/(node_pd-1);
	} else {
		node_dist = (L-2*l)/node_pd;
	}

	std::vector<XYZ> nodes(Nnodes);
	int i=0;
	for(int xi =0;xi<node_pd;++xi) {
		for(int yi=0;yi<node_pd;++yi) {
			for(int zi=0;zi<node_pd;++zi) {
				nodes[i].x = l+xi*node_dist;
				nodes[i].y = l+yi*node_dist;
				nodes[i].z = l+zi*node_dist;
				++i;
			}
		}
	}
	for(unsigned int i=0;i<N; ++i) {
		// pick a random node
		int ni =  (int)( rudist()*nodes.size() );			
		r[i] = nodes[ni];
		nodes.erase(nodes.begin()+ni); // remove node


		// check!!!!!!	
		v[i].x = (rudist()-1.)/std::sqrt(m);
		v[i].y = (rudist()-1.)/std::sqrt(m);
		v[i].z = (rudist()-1.)/std::sqrt(m);

		do {
			zeta.x = 2*rudist() - 1.;
			zeta.y = 2*rudist() - 1.;
			zeta.z = 2*rudist() - 1.;
			d = zeta.length_sq();	
		} while (d > 1.);
		zeta.normalize();
		p[i] = zeta;
	}

}

bool System::check_x_in_box()
{
	XYZ temp;
	bool inside = true;
	for(unsigned int i=0;i<N;++i){
		temp = r[i];
		if(temp.x < 0 or temp.x >L)
			inside = false;

		if(!inside) break;
	}
	return inside;
}


bool System::check_y_in_box()
{
	XYZ temp;
	bool inside = true;
	for(unsigned int i=0;i<N;++i){
		temp = r[i];
		if(temp.y < 0 or temp.y>L)
			inside = false;

		if(!inside) break;
	}
	return inside;
}

bool System::check_z_in_box()
{
	XYZ temp;
	bool inside = true;
	for(unsigned int i=0;i<N;++i){
		temp = r[i];
		if(temp.z < 0 or temp.z>L)
			inside = false;

		if(!inside) break;
	}
	return inside;
}




void System::write(const char* outname)
{
	std::ofstream out;
	out.open(outname);
	XYZ temp;
	for(unsigned int i=0;i<N;++i) {
		temp = r[i];
		temp.pbc(L);	
		out << temp.x << '\t';
		out << temp.y << '\t';
		out << temp.z;
		if(i<(N-1)) out << '\n';
	}

	out.close();

}

class Integration {
public:
	Integration( ConfigFile config)
	{
		Nt_init = config.read<unsigned int>("Nt_init");
		Nt = config.read<unsigned int>("Nt");
		sample_freq = config.read<unsigned int>("sample_freq");
		print_freq = config.read<unsigned int>("print_freq");
		t_unit = config.read<unsigned int>("t_unit");
		dt = config.read<double>("dt");
		bs = config.read<double>("bs");
	}

	unsigned int Nt_init;
	unsigned int Nt;
	unsigned int sample_freq;
	unsigned int print_freq;
	unsigned int t_unit;
	double dt;
	double bs;
};


#endif
