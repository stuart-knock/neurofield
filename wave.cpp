#include<cmath>
#include"wave.h"
#include <iostream>
using std::cerr;
using std::endl;

void Wave::init( Configf& configf )
{
  string buffer("Steady");
  configf.optional("phi",buffer);
  double Q = prepop.Qinit(configf);
  configf.param("Deltax",deltax);
  configf.param("Tau",tau); prepop.growHistory(tau);
  if( buffer != "Steady" )
    p.resize(nodes,atof(buffer.c_str()));
  else
    p.resize(nodes,Q);

  configf.param("Range",range);
  configf.param("gamma",gamma);

  oldpval[0].resize(nodes,p[0]);
  oldpval[1].resize(nodes,p[0]);
  oldQval[0].resize(nodes,Q);
  oldQval[1].resize(nodes,Q);

  oldp[0] = new Stencil(nodes,longside,topology); *oldp[0] = oldpval[0];
  oldp[1] = new Stencil(nodes,longside,topology); *oldp[1] = oldpval[1];
  oldQ[0] = new Stencil(nodes,longside,topology); *oldQ[0] = oldQval[0];
  oldQ[1] = new Stencil(nodes,longside,topology); *oldQ[1] = oldQval[0];

  /*if( topology == "Torus" ) {
    oldp[0] = new TStencil(nodes,longside); oldp[0]->assign(&oldpval[0]);
    oldp[1] = new TStencil(nodes,longside); oldp[1]->assign(&oldpval[1]);
    oldQ[0] = new TStencil(nodes,longside); oldQ[0]->assign(&oldQval[0]);
    oldQ[1] = new TStencil(nodes,longside); oldQ[1]->assign(&oldQval[0]);
  }
  else {
    double bath = atof( topology.substr( topology.find(" ") ).c_str() );
    oldp[0] = new Stencil(nodes,longside,bath); oldp[0]->assign(&oldpval[0]);
    oldp[1] = new Stencil(nodes,longside,bath); oldp[1]->assign(&oldpval[1]);
    oldQ[0] = new Stencil(nodes,longside,bath); oldQ[0]->assign(&oldQval[0]);
    oldQ[1] = new Stencil(nodes,longside,bath); oldQ[1]->assign(&oldQval[0]);
  }*/

  dt2on12 = deltat*deltat/12.;
  dfact = dt2on12*gamma*gamma;
  dt2ondx2 = deltat*deltat/deltax/deltax;
  p2 = dt2ondx2*range*range*gamma*gamma;
  tenminus3p2 = 10.-3.*p2;
  twominus3p2 = 2.-3.*p2;
  exp1 = exp(-deltat*gamma);
  exp2 = exp(-2.*deltat*gamma);

  if( gamma*range*deltat/deltax >1.41 ) {
    cerr<<"Wave equation does not fulfill the Courant condition."<<endl;
    exit(EXIT_FAILURE);
  }
}

void Wave::restart( Restartf& restartf )
{
}

void Wave::dump( Dumpf& dumpf ) const
{
}

Wave::Wave( int nodes, double deltat, int index, Population& prepop,
        Population& postpop, int longside, string topology )
    : Propag(nodes,deltat,index,prepop,postpop,longside,topology),
        key(0), topology(topology)
{
}

Wave::~Wave(void)
{
  delete oldp[0]; delete oldp[1];
  delete oldQ[0]; delete oldQ[1];
}

void Wave::step(void)
{
  for( int i=0; i<nodes; i++,
          (*oldp[0])++, (*oldQ[0])++, (*oldp[1])++, (*oldQ[1])++ ) {
    sump     = oldp[0]->n()  +oldp[0]->s()  +oldp[0]->w()  +oldp[0]->e();
    diagsump = oldp[0]->nw() +oldp[0]->ne() +oldp[0]->sw() +oldp[0]->se();
    sumQ     = oldQ[0]->n()  +oldQ[0]->s()  +oldQ[0]->w()  +oldQ[0]->e();
    diagsumQ = oldQ[0]->nw() +oldQ[0]->ne() +oldQ[0]->sw() +oldQ[0]->se();
    drive = dfact*( tenminus3p2*exp1*oldQ[0]->c() +prepop.Q(tau)[i] +exp2*oldQ[1]->c() +exp1*.5*p2*(sumQ+.5*diagsumQ) );
    p[i] = twominus3p2*exp1*oldp[0]->c() +exp1*.5*p2*(sump+.5*diagsump) -exp2*oldp[1]->c() +drive;
  }

  oldpval[1] = oldpval[0];
  oldpval[0] = p;
  oldQval[1] = oldQval[0];
  oldQval[0] = prepop.Q(tau);

  *oldp[0] = oldpval[0];
  *oldp[1] = oldpval[1];
  *oldQ[0] = oldQval[0];
  *oldQ[1] = oldQval[1];

  /*key = !key;
  oldpval[key] = p;
  oldp[0]->assign(&oldpval[key]);  // just updated value
  oldp[1]->assign(&oldpval[!key]); // one timestep back
  oldpval[key] = prepop.Q(tau);
  oldQ[0]->assign(&oldQval[key]);
  oldQ[1]->assign(&oldQval[!key]);*/
}