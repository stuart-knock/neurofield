#include<iostream>
using std::cerr;
using std::endl;
#include"stencil.h"

Stencil::Stencil( int nodes, int longside, const string& boundary )
  : nodes(nodes), longside(longside), boundary(boundary),
    m( (nodes/longside+2)*(longside+2) ), ptr(0)
{
  if( boundary != "Torus" && boundary != "Sphere" ) {
    cerr<<"Stencil boundary must be Torus or Sphere."<<endl;
    exit(EXIT_FAILURE);
  }
}

Stencil::~Stencil(void)
{
}

const vector<double>& Stencil::operator= ( const vector<double>& field )
{
  if( boundary == "Torus" )
  {
    // copy centre square
    for( int j=0; j< nodes/longside; j++ )
      for( int i=0; i<longside; i++ )
        m[(j+1)*(longside+2)+i+1] = field[j*longside+i];

    // copy right edge into left boundary
    for( int i=0; i<=nodes/longside; i++ )
      m[i*(longside+2)] = field[i*longside-1];

    // copy left edge into right boundary
    for( int i=0; i<=nodes/longside; i++ )
      m[(i+2)*(longside+2)-1] = field[i*longside];

    // copy bottom edge into top boundary
    for( int i=0; i<longside; i++ )
      m[i+1] = field[nodes-longside+i];

    // copy top edge into bottom boundary
    for( int i=0; i<longside; i++ )
      m[(nodes/longside+1)*(longside+2)+1+i] = field[i];

    // copy 4 corners
    m[0] = field[nodes-1];
    m[longside+1] = field[nodes-longside];
    m[(longside+2)*(nodes/longside+1)] = field[longside-1];
    m[(longside+2)*(nodes/longside+2)-1] = field[0];
  }

  set(0);
  return field;
}

void Stencil::operator++ (int i) const
{
  ptr++;
  if( ( ptr%(longside+2)==longside+1 ) )
    ptr += 2;
  if( ptr == (longside+1)*(nodes/longside+2)+1 )
    set(0);
}

void Stencil::set( int node ) const
{
  if( node>=0 && node<nodes ) {
    int x = node%longside;
    int y = node/longside;
    ptr = (y+1)*(longside+2) +x+1;
  }
  else {
    cerr<<"Stencil node setting out of bound: "<<node<<endl;
    exit(EXIT_FAILURE);
  }
}

int Stencil::get(void) const
{
  int x = ptr%(longside+2)-1;
  int y = ptr/(longside+2)-1;
  return x+y*longside;
}

double Stencil::nw(void) const { return m[ ptr-longside-2 -1]; }
double Stencil::n (void) const { return m[ ptr-longside-2   ]; }
double Stencil::ne(void) const { return m[ ptr-longside-2 +1]; }
double Stencil:: w(void) const { return m[ ptr            -1]; }
double Stencil:: c(void) const { return m[ ptr              ]; }
double Stencil:: e(void) const { return m[ ptr            +1]; }
double Stencil::sw(void) const { return m[ ptr+longside+2 -1]; }
double Stencil::s (void) const { return m[ ptr+longside+2   ]; }
double Stencil::se(void) const { return m[ ptr+longside+2 +1]; }