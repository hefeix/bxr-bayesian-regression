#include <iostream>
#include <fstream>
#include <vector>
#include <limits>

#define  _USE_MATH_DEFINES
#include <math.h>
using namespace std;

#include "Matrix.h"
#include "logging.h"

Matrix mIdentity(int n) {
    Matrix I(n,n,0.0);
    for(int i=0; i<n*n; i+=n+1)  I[i] = 1.;
    return I;
}

Matrix mTm( const Matrix& a ) {
    Matrix aTa( a.nCols(), a.nCols() );
    for(unsigned c1=0; c1<a.nCols(); c1++)
        for(unsigned c2=c1; c2<a.nCols(); c2++)
        {
            double d=0;
            for(unsigned r=0;r<a.nRows();r++)
                d += a.val(r,c1) * a.val(r,c2);
            aTa.val(c1,c2) = aTa.val(c2,c1) = d;
        }
            //aTa.val(c1,c2) = aTa.val(c2,c1) = dot( a[ a.col(c1) ], a[ a.col(c2) ] );
    return aTa;
}
/*#ifdef _MSC_VER
// get minor out of a square matrix
// according to row/column indicator vector
Matrix Minor( const Matrix& a, const valarray<bool>& b ) {
    size_t size = a.nRows();
    if( size != a.nCols() ) //should be square
        throw DimensionConflict(__FILE__,__LINE__);
    if( size != b.size() ) //should comply
        throw DimensionConflict(__FILE__,__LINE__);

    Matrix mminor( ntrue(b), ntrue(b) );
    valarray<double> tmprow;
    size_t newrow = 0;
    for( unsigned r=0; r<size; r++ )
    if( b[r] )
    {
        tmprow = ROW( a, r );
        ROW( mminor, newrow++ ) = tmprow[ b ];
    }
    return mminor;
}
#endif //_MSC_VER
*/
Matrix horizJoin( const Matrix& a, const Matrix& b ) {
    if( a.nRows() != b.nRows() )
        throw DimensionConflict(__FILE__,__LINE__);
    Matrix ab( a.nRows(), a.nCols()+b.nCols() );
    unsigned c;
    for( c=0; c<a.nCols(); c++) {
        valarray<double> tmp = a.val()[ a.col(c) ];
        ab.val()[ ab.col(c) ] = tmp;
    }
    for(unsigned cb=0; cb<b.nCols(); cb++) {
        valarray<double> tmp = b.val()[ b.col(cb) ];
        ab.val()[ ab.col(c+cb) ] = tmp;
    }
    return ab;
}

Matrix operator+( const Matrix& left, const valarray<double>& right ) {
    Matrix res = left;
    res += right;
    return res;
}
Matrix operator+( const Matrix& left, const Matrix& right ) {
    return left + right.val();
}
Matrix operator*( const Matrix& left, const Matrix& right ) {
//Matrix operator*( const MatrixView& left, const MatrixView& right ) {
    if( left.nCols() != right.nRows() )
        throw DimensionConflict(__FILE__,__LINE__);
    unsigned ncols = right.nCols();
    unsigned nrows = left.nRows();
    unsigned n = left.nCols(); //inner

    Matrix res( nrows, ncols );
    for( unsigned r=0; r<nrows; r++ )
        for( unsigned c=0; c<ncols; c++ ) {
            double d=0;
            for(unsigned i=0;i<n;i++)
                d += left.val(r,i) * right.val(i,c);
            res.val( r, c ) = d;
                //dot( left.val()[left.row(r)], right.val()[ right.col(c) ] );
        }

    return res;
}
//Matrix operator*( const Matrix& left, const Matrix& right ) {
//    return( MatrixView(left) * MatrixView(right) );  }

ostream& operator<<( ostream& o, const MatrixView& m ) {
    o<<endl<<m.nRows()<<" "<<m.nCols();
    for( int r=0; r<m.nRows(); r++ ) {
        o<<endl<<"-> ";
        for( int c=0; c<m.nCols(); c++ )
            o<<m.val(r,c)<<" ";
    }
    return o;
}
std::ostream& operator<<( std::ostream& o, const Matrix& m ) {
    o<<MatrixView(m);
    return o;
}
/*int ntrue( const BoolVector& y ) {
    int nn1 = 0;
    for(size_t i=0;i<y.size();i++) if(y[i]) nn1++;
    return nn1;
}*/
MatrixView Matrix::transpose() const {
    return MatrixView( *this, true ); }

void Normalize( valarray<double>& from, valarray<double>& to )
{
    if( from.size() != to.size() )
        throw DimensionConflict(__FILE__,__LINE__);  //----->>--
    double mean=0.0, meansqu=0.0;
    for( unsigned int r=0; r<from.size(); r++ ) {
        mean = mean*r/(r+1) + from[r]/(r+1);
        meansqu = meansqu*r/(r+1) + from[r]*from[r]/(r+1);
    }
    double stddev = sqrt( meansqu - mean*mean );
    if( stddev>0 )
        for( unsigned int r=0; r<to.size(); r++ )
            to[r] = (from[r]-mean)/stddev;
    else //assume is constant
        to = 0.0;
}
void Normalize( valarray<double>& v ) {
    Normalize( v, v ); }

void Normalize( Matrix& x )
{
    for( unsigned c=0; c<x.nCols(); c++ ) {
        double sum=0.0, ss=0.0;
        double mean=0.0, meansqu=0.0;
        for( unsigned r=0; r<x.nRows(); r++ ) {
            //bad sum += x.val(r,c);
            //bad ss += x.val(r,c) * x.val(r,c);
            mean = mean*r/(r+1) + x.val(r,c)/(r+1);
            meansqu = meansqu*r/(r+1) + x.val(r,c)*x.val(r,c)/(r+1);
        }
        //bad mean = sum/x.nRows();
        //bad double stddev = sqrt( ss/x.nRows() - mean*mean );
        double stddev = sqrt( meansqu - mean*mean );
        for( unsigned r=0; r<x.nRows(); r++ ) {
            x.val(r,c) = (x.val(r,c)-mean)/stddev;
        }
    }
}

void Centralize( Matrix& x )
{
    for( unsigned c=0; c<x.nCols(); c++ ) {
        double sum=0.0;
        double mean=0.0;
        for( unsigned r=0; r<x.nRows(); r++ ) {
            mean = mean*r/(r+1) + x.val(r,c)/(r+1);
        }
        for( unsigned r=0; r<x.nRows(); r++ ) {
            x.val(r,c) = x.val(r,c)-mean;
        }
    }
}

Matrix::Matrix( std::istream& ifs )
{
    nrows=0; ncols=0;

    const int bufsize=1000;
    char buf[bufsize+1];
    double dbuf[bufsize];

    while( ifs.getline( buf, bufsize ) && ifs.good() )
    {
        istrstream rowbuf( buf );
        int i;
        for( i=0; rowbuf >> dbuf[i] && !rowbuf.fail(); i++ );
        
        if( ncols==0 )//just started
            ncols = i;
        else if( 0==i )
            break;  //----->>--
        else if( ncols != i )
            throw DimensionConflict(__FILE__,__LINE__);  //----->>--

        valarray<double> rowarr( dbuf, ncols );

        resize( size() + ncols );
        val()[ row(nrows) ] = rowarr;
        nrows ++;
    }
}

/*not ported to GCC
Matrix ReduceCols( const Matrix& x, const valarray<bool>& select )
{
    if( x.nCols() != select.size() )
        throw DimensionConflict(__FILE__,__LINE__);  //----->>--

    Matrix clean( x.nRows(), ntrue(select) );
    for( int r=0; r<x.nRows(); r++ )
        clean[clean.row(r)] = x[x.row(r)][select];
    //int c1=0;
    //for( int c=0; c<clean.nCols(); c++ )
    //    if( select[c] )
    //        clean.Col(c1++) = x.Col(c);

    return clean;
}*/


//--------------------------- from MATV package, DIST.CPP ------------------->>
// Version: 2.1
// Author: Mark Von Tress, Ph.D.
// Date: 01/07/96

// Copyright(c) Mark Von Tress 1996


// DISCLAIMER: THIS PROGRAM IS PROVIDED AS IS, WITHOUT ANY
// WARRANTY, EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO FITNESS FOR A PARTICULAR PURPOSE. THE AUTHOR DISCLAIMS
// ALL LIABILITY FOR DIRECT OR CONSEQUENTIAL DAMAGES RESULTING
// FROM USE OF THIS PROGRAM.

///////////////////////// probability calculations

double erfc(double x)
{
    double t, z, ans; // AG: was float

  z = fabs(x);
  t = 1.0 / (1.0+0.5*z);
  ans = t*exp(- z*z - 1.26551223+ t*(1.00002368+ t*(0.37409196
         + t*(  0.09678418 +
         + t*(- 0.18628806 + t*(0.27886807+ t*(- 1.13520398
         + t*(  1.48851587
         + t*(- 0.82215223 + t*0.17087277)))))))));
  ans = (x >= 0.0) ? ans : 2.0-ans;
  return (1.0 - ans);
}
double probnorm(double x)
{
  return ((1.0+erfc((x) / 1414213562373095E-15)) * 0.5);
}
//<<-------------------------- end from MATV package ---------------------

//--------- from INFOSCOPE - (C) P.N.Dubner  ==>
#define sign(x) (x == 0 ? 0 : (x < 0 ? -1 : 1))
const double pi_const=0.3989422804014;
int N;
double
L(double x, double eps)
{
   double x2=x*x, y=exp(-x2/2.)*pi_const,
          sum, term, r, rho, t, s;
   N=0;
   if (x == 0) return 0.5;
   x2=1./x2; term=sum=y/fabs(x); r=s=0; rho=1;
   do {
      r+=x2; rho=1./(1+r*rho); term*=rho-1;
      t=s; s=sum; sum+=term;
      N++; /* ??????? ??????????! */
   } while(fabs(s-t) > eps || fabs(s-sum) > eps);
   return (x > 0 ? 1-sum : sum);
  /* 16+10*N ???????? */
}
//--<==--------- from INFOSCOPE - (C) P.N.Dubner-------------

static const double MATVLimit = 7.5;  //switch from MATV package to P.N.Dubner's code
static const double prec = 1.0e-10;
double combinedProbNorm( double x ) {
    if( -MATVLimit<=x && x<=MATVLimit ) return probnorm(x);
    else return L(x,prec);
}

/*
http://home.online.no/~pjacklam/notes/invnorm/impl/natarajan/
found through reference at http://home.online.no/~pjacklam/notes/invnorm/
*/
#define  A1  (-3.969683028665376e+01)
#define  A2   2.209460984245205e+02
#define  A3  (-2.759285104469687e+02)
#define  A4   1.383577518672690e+02
#define  A5  (-3.066479806614716e+01)
#define  A6   2.506628277459239e+00

#define  B1  (-5.447609879822406e+01)
#define  B2   1.615858368580409e+02
#define  B3  (-1.556989798598866e+02)
#define  B4   6.680131188771972e+01
#define  B5  (-1.328068155288572e+01)

#define  C1  (-7.784894002430293e-03)
#define  C2  (-3.223964580411365e-01)
#define  C3  (-2.400758277161838e+00)
#define  C4  (-2.549732539343734e+00)
#define  C5   4.374664141464968e+00
#define  C6   2.938163982698783e+00

#define  D1   7.784695709041462e-03
#define  D2   3.224671290700398e-01
#define  D3   2.445134137142996e+00
#define  D4   3.754408661907416e+00

#define P_LOW   0.02425
/* P_high = 1 - p_low*/
#define P_HIGH  0.97575

long double normsinv(long double p)
{
long double x;
long double q, r;
if ((0 < p )  && (p < P_LOW)){
   q = sqrt(-2*log(p));
   x = (((((C1*q+C2)*q+C3)*q+C4)*q+C5)*q+C6) / ((((D1*q+D2)*q+D3)*q+D4)*q+1);
}
else{
        if ((P_LOW <= p) && (p <= P_HIGH)){
           q = p - 0.5;
           r = q*q;
           x = (((((A1*r+A2)*r+A3)*r+A4)*r+A5)*r+A6)*q /(((((B1*r+B2)*r+B3)*r+B4)*r+B5)*r+1);
        }
        else{
                if ((P_HIGH < p)&&(p < 1)){
                   q = sqrt(-2*log(1-p));
                   x = -(((((C1*q+C2)*q+C3)*q+C4)*q+C5)*q+C6) / ((((D1*q+D2)*q+D3)*q+D4)*q+1);
                }
        }
}

/* If you are compiling this under UNIX OR LINUX, you may uncomment this block for better accuracy.
if(( 0 < p)&&(p < 1)){
   e = 0.5 * erfc(-x/sqrt(2)) - p;
   u = e * sqrt(2*M_PI) * exp(x*x/2);
   x = x - u/(1 + x*u/2);
}
*/

return x;
}

unsigned argmax( const vector<double>& score ) {
    unsigned ret;
    double maxval = - numeric_limits<double>::max();
    for( vector<double>::const_iterator itr=score.begin(); itr!=score.end(); itr++ )
        if( *itr > maxval ) {
            maxval = *itr;
            ret = itr - score.begin();
        }
    return ret;
}


/*
    Copyright 2005, Rutgers University, New Brunswick, NJ.

    All Rights Reserved

    Permission to use, copy, and modify this software and its documentation for any purpose 
    other than its incorporation into a commercial product is hereby granted without fee, 
    provided that the above copyright notice appears in all copies and that both that 
    copyright notice and this permission notice appear in supporting documentation, and that 
    the names of Rutgers University, DIMACS, and the authors not be used in advertising or 
    publicity pertaining to distribution of the software without specific, written prior 
    permission.

    RUTGERS UNIVERSITY, DIMACS, AND THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO 
    THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
    ANY PARTICULAR PURPOSE. IN NO EVENT SHALL RUTGERS UNIVERSITY, DIMACS, OR THE AUTHORS 
    BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
    RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, 
    NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
    PERFORMANCE OF THIS SOFTWARE.
*/
