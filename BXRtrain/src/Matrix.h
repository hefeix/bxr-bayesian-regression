#ifndef MATRIX_VALARRAY_WRAPPER_
#define MATRIX_VALARRAY_WRAPPER_

#include <stdexcept>
#include <assert.h>
#include <valarray>
#include <vector>
#include <iostream>
#include <strstream>
#include <string>

using std::logic_error;
using std::valarray;
using std::slice;
using std::slice_array;
using std::vector;

template<class T> std::ostream& operator<<( std::ostream& s, const valarray<T>& vad ) {
    for( size_t i=0; i<vad.size(); i++ ) s<<vad[i]<<" ";
    return s;
};
/*std::ostream& operator<<( std::ostream& s, const valarray<double>& vad ) {
    for( size_t i=0; i<vad.size(); i++ ) s<<vad[i]<<" ";
    return s;
}*/

class MatrixView;

class DimensionConflict : public logic_error {
    std::string  file;
    int line;
public:
    DimensionConflict(const char* file_, int line_) 
        : std::logic_error("Dimension Conflict"), file(file_), line(line_) {}
    const char* what() const throw() {
        std::ostrstream s;
        s<<"Dimension Conflict, File: "<<file.c_str()<<" line: "<<line<<std::ends;
        return s.str(); }
    ~DimensionConflict() throw() {}
};

class MatrixExc : public std::runtime_error {
public:
    MatrixExc(const char* msg) 
        : std::runtime_error( std::string("Matrix Exception: ") + msg) {}
    ~MatrixExc() throw() {}
};

class BoolVector : public vector<bool> {
public: 
    BoolVector( bool val, size_t size ) : vector<bool>( size, val ) {};
    BoolVector( size_t size ) : vector<bool>( size ) {};
    BoolVector() {};
};
inline std::ostream& operator<<( std::ostream& s, const BoolVector& v ) {
    for( size_t i=0; i<v.size(); i++ ) s<<v[i]<<" ";
    return s;
}
//#endif
class Vector : public vector<double> {
public: 
    Vector( double val, size_t size ) : vector<double>( size, val ) {};
    Vector( size_t size ) : vector<double>( size ) {};
    Vector() {};
    Vector operator-(const Vector& v) const {
        if( v.size()!=size() ) throw DimensionConflict(__FILE__,__LINE__);
        Vector r(size());
        for( unsigned i=0; i<size(); i++ )
            r[i] = (*this)[i] - v[i];
        return r;
    }
};
inline std::ostream& operator<<( std::ostream& s, const Vector& v ) {
    for( size_t i=0; i<v.size(); i++ ) s<<v[i]<<" ";
    return s;
}
inline std::ostream& operator<<( std::ostream& s, const vector<double>& v ) {
    for( size_t i=0; i<v.size(); i++ ) s<<v.at(i)<<" ";
    return s;
}
template<class T> std::ostream& operator<<( std::ostream& s, const vector<T>& v ) {
    for( size_t i=0; i<v.size(); i++ ) s<<v.at(i)<<" ";
    return s;
}

class Matrix : public valarray<double> {
    unsigned nrows, ncols;
    friend class MatrixView;
    //friend Matrix rbfkernel( const Matrix& x, double width);
public:
    //ctor
    Matrix( unsigned nrows_, unsigned ncols_, const valarray<double>& v_ )
        : nrows(nrows_), ncols(ncols_), valarray<double>(v_)
    { assert( size()==nrows*ncols ); }
    Matrix( unsigned r, unsigned c, double _v=0.0 )
        : nrows(r), ncols(c), valarray<double>( _v, r*c )     {}
    Matrix() : nrows(0), ncols(0) {}
    Matrix( std::istream& ifs );

    //access
    unsigned nRows() const {return nrows;}
    unsigned nCols() const {return ncols;}

    valarray<double>& val() {return (*this);}
    const valarray<double>& val() const {return (*this);}
    
    double val( int r, int c ) const { return (*this)[ c + r*ncols ]; }
    double& val( int r, int c )      { return (*this)[ c + r*ncols ]; }

    slice col(int c) const { return slice( c, nrows, ncols ); }
    slice row(int r) const { return slice( r*ncols, ncols, 1 );  }

    //valarray<double> Col(int c) const { return ((*this)[slice( c, nrows, ncols )]); }
    //valarray<double> Row(int r) const { return ((*this)[slice( r*ncols, ncols, 1 )]);  }
    //slice_array<double> Col(int c) { return (*this)[slice( c, nrows, ncols )]; }
    //slice_array<double> Row(int r) { return (*this)[slice( r*ncols, ncols, 1 )];  }

    //--- matrix algebra operations ---
    class MatrixView transpose() const;
};

#define ROW(M,r) ( (M)[(M).row((r))] )
#define COL(M,c) ( (M)[(M).col((c))] )

Matrix mIdentity(int n);
Matrix mTm( const Matrix& a );
#ifdef _MSC_VER
Matrix Minor( const Matrix& a, const BoolVector& b );
#endif
Matrix horizJoin( const Matrix& a, const Matrix& b );

template<class T1, class T2> double dot( const valarray<T1>& a, const valarray<T2>& b ) {
    size_t size = a.size();
    assert( b.size()==size );
    double r=0;
    for( size_t i=0; i<size;i++ )  r += a[i] * b[i];
    return r;
}

template<class T> double norm( const valarray<T>& a ) {
    return dot(a,a); }

/* for GCC (since valarray is not supported) ==> */ 
inline double dot( const Vector& a, const Vector& b ) {
    size_t size = a.size();
    assert( b.size()==size );
    double r=0;
    for( size_t i=0; i<size;i++ )  r += a[i] * b[i];
    return r;
}
inline double norm( const Vector& a ) {
    return dot(a,a); }

inline double normdiff( const vector<double>& a, const vector<double>& b) {
    size_t size = a.size();
    assert( b.size()==size );

	double r=0;
    for( size_t i=0; i<size;i++ )  r += (a[i] - b[i]) * (a[i] - b[i]);
    return r;
}

inline double norm( const vector<double>& a) {
    size_t size = a.size();

	double r=0;
    for( size_t i=0; i<size;i++ )  r += a[i] * a[i];
    return r;
}
/*<== for GCC*/

class MatrixView {
    const Matrix& m;
    bool transpose;
public:
    MatrixView( const Matrix& m_, bool t_ = false ) : m(m_), transpose(t_) {}
    
    int nRows() const {return transpose ? m.ncols : m.nrows;}
    int nCols() const {return transpose ? m.nrows : m.ncols;}

    double val( int r, int c ) const { 
        return transpose ? m.val(c,r) : m.val(r,c); }

    //const valarray<double>& val() const {return m.val();}  ?? DANGER
    slice col(int c) const { return transpose ? m.row(c) : m.col(c); }
    slice row(int r) const { return transpose ? m.col(r) : m.row(r); }
};

std::ostream& operator<<( std::ostream& o, const MatrixView& m );
std::ostream& operator<<( std::ostream& o, const Matrix& m );

Matrix operator+( const Matrix& left, const valarray<double>& right );
Matrix operator+( const Matrix& left, const Matrix& right );

//template<class T> Matrix operator*( const Matrix& m, T a ) {
//    return Matrix( m.nRows(), m.nCols(), a*m.val() ); }

//right is considered a matrix or a vector depending on its size
template<class T> Matrix operator*( const Matrix& left, const valarray<T>& right ) {
    ldiv_t rightDim = ldiv( int(right.size()), left.nCols() );
    if( 0!=rightDim.rem )
        throw DimensionConflict(__FILE__,__LINE__);
    int ncols = rightDim.quot;
    int nrows = left.nRows();

    Matrix res( nrows, ncols );
    for( int r=0; r<nrows; r++ )
        for( int c=0; c<ncols; c++ )
            res.val( r, c ) = 
                //dot( ROW(left,r)], right[ slice(c,left.nCols(),ncols) ] );
                dot( left.val()[left.row(r)], right[ slice(c,left.nCols(),ncols) ] );

    return res;
}

Matrix operator*( const Matrix& left, const Matrix& right );

// right is a diag matrix represented by diag vector
// result = left * diag(right)
template<class T> Matrix multDiag( const MatrixView& left, const valarray<T>& right ) {
    if( right.size() != left.nCols() )
        throw DimensionConflict(__FILE__,__LINE__);

    Matrix res( left.nRows(), left.nCols() );
    for( unsigned c=0; c<res.nCols(); c++ )
        for( unsigned r=0; r<res.nRows(); r++ )
            res.val( r, c ) = left.val( r, c ) * right[c];

    return res;
}

// left is a diag matrix represented by diag vector
// result = diag(left) * right
template<class T> Matrix diagMult( const valarray<T>& left, const MatrixView& right ) {
    if( left.size() != right.nRows() )
        throw DimensionConflict(__FILE__,__LINE__);

    Matrix res( right.nRows(), right.nCols() );
    for( unsigned r=0; r<res.nRows(); r++ )
        for( unsigned c=0; c<res.nCols(); c++ )
            res.val( r, c ) = right.val( r, c ) * left[r];

    return res;
}

Matrix LeftDiv( const MatrixView& a, const MatrixView& b );
Vector VectorLeftDiv( const MatrixView& ma, const Vector& mb );

//int ntrue( const BoolVector& y );
template<class Arr> int ntrue( const Arr& y ) {
    int nn1 = 0;
    for(size_t i=0;i<y.size();i++) if(y[i]) nn1++;
    return nn1;
}
Matrix ReduceCols( const Matrix& x, const BoolVector& select );

void Normalize( valarray<double>& from, valarray<double>& to );
void Normalize( valarray<double>& v );
void Normalize( Matrix& x );
void Centralize( Matrix& x );

// probability distributions
double probnorm(double x);
double combinedProbNorm( double x );
long double normsinv(long double p);

unsigned argmax( const vector<double>& score );


#endif //MATRIX_VALARRAY_WRAPPER_

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
