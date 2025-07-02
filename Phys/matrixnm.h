// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef matrixnm_h
#define matrixnm_h
#pragma once

enum    mtxflags {
    mtx_invalid=1, mtx_normal=2, mtx_orthogonal=4, mtx_PSD=8, mtx_PD_flag=16, mtx_PD=mtx_PSD|mtx_PD_flag, mtx_symmetric=32,
    mtx_diagonal_flag=64, mtx_diagonal=mtx_symmetric|mtx_normal|mtx_diagonal_flag, mtx_identity_flag=128,
    mtx_identity=mtx_PD|mtx_diagonal|mtx_orthogonal|mtx_normal|mtx_symmetric|mtx_identity_flag, mtx_singular=256,
    mtx_foreign_data=1024, // this flag means that data was not allocated by matrix
    mtx_allocate=32768 // prohibts using matrix pool for data
};

template<class ftype> class matrix_product_tpl {
public:
    matrix_product_tpl(i32 nrows1,i32 ncols1,ftype *pdata1,i32 flags1, i32 ncols2,ftype *pdata2,i32 flags2) {
        data1=pdata1; data2=pdata2; nRows1=nrows1;nCols1=ncols1; nCols2=ncols2;
        flags = flags1 & flags2 & (mtx_orthogonal | mtx_PD) & ~mtx_foreign_data;
    }
    void assign_to(ftype *pdst) const {
        i32 i,j,k; ftype sum;
        for(i=0;i<nRows1;i++) for(j=0;j<nCols2;j++) {
            for(sum=0,k=0; k<nCols1; k++)
                sum += data1[i*nCols1+k]*data2[k*nCols2+j];
            pdst[i*nCols2+j] = sum;
        }
    }
    void add_assign_to(ftype *pdst) const {
        for(i32 i=0;i<nRows1;i++) for(i32 j=0;j<nCols2;j++)
            for(i32 k=0; k<nCols1; k++)
                pdst[i*nCols2+j] += data1[i*nCols1+k]*data2[k*nCols2+j];
    }
    void sub_assign_to(ftype *pdst) const {
        for(i32 i=0;i<nRows1;i++) for(i32 j=0;j<nCols2;j++)
            for(i32 k=0; k<nCols1; k++)
                pdst[i*nCols2+j] -= data1[i*nCols1+k]*data2[k*nCols2+j];
    }

    i32 nRows1,nCols1,nCols2;
    ftype *data1,*data2;
    i32 flags;
};

template <class ftype> class matrix_tpl {
public:
    matrix_tpl() { nRows=nCols=3; flags=mtx_foreign_data; data=0;   }
    void init(i32 nrows, i32 ncols, i32 _flags, ftype *pdata) {
        nRows = nrows; nCols = ncols;
        flags = _flags & ~mtx_allocate;
        i32 sz = nRows*nCols;
        if (pdata!=(ftype*)-1) {
            data = pdata; flags |= mtx_foreign_data;
#if defined(USE_MATRIX_POOLS)
        } else if (sz<=36 && !(_flags & mtx_allocate)) {
            if (mtx_pool_pos+sz > mtx_pool_size)
                mtx_pool_pos = 0;
            data = mtx_pool+mtx_pool_pos;
            mtx_pool_pos += sz;
            flags |= mtx_foreign_data;
#endif
        }
    else
            data = new ftype[sz];
    }
    ILINE matrix_tpl(i32 nrows, i32 ncols, i32 _flags=0, ftype *pdata=(ftype*)-1) { init(nrows,ncols,_flags,pdata); }
    ILINE matrix_tpl(const matrix_tpl& src) {
        if (src.flags & mtx_foreign_data) {
            nRows=src.nRows; nCols=src.nCols;
            flags=src.flags; data=src.data;
        } else {
            init(src.nRows,src.nCols,src.flags,0);
            for(i32 i=nRows*nCols-1;i>=0;i--) data[i]=src.data[i];
        }
    }
    ILINE ~matrix_tpl() {
        if (data && !(flags & mtx_foreign_data)) delete [] data;
    }

    matrix_tpl& operator=(const matrix_tpl<ftype> &src) {
        if (this == &src) {
            return *this;
        }

        if (!data || !(flags & mtx_foreign_data) && nRows*nCols<src.nRows*src.nCols) {
            delete[] data; data = new ftype[src.nRows*src.nCols];
        }
        nRows=src.nRows; nCols=src.nCols;
        flags = flags&mtx_foreign_data | src.flags&~mtx_foreign_data;
        for(i32 i=nRows*nCols-1;i>=0;i--) data[i] = src.data[i];
        return *this;
    }
    template<class ftype1> matrix_tpl& operator=(const matrix_tpl<ftype1> &src) {
        if (!data || !(flags & mtx_foreign_data) && nRows*nCols<src.nRows*src.nCols) {
            delete[] data; data = new ftype[src.nRows*src.nCols];
        }
        nRows=src.nRows; nCols=src.nCols;
        flags = flags&mtx_foreign_data | src.flags&~mtx_foreign_data;
        for(i32 i=nRows*nCols-1;i>=0;i--) data[i] = src.data[i];
        return *this;
    }

    matrix_tpl& operator=(const matrix_product_tpl<ftype> &src) {
        nRows=src.nRows1; nCols=src.nCols2;
        flags = flags&mtx_foreign_data | src.flags;
        src.assign_to(data);
        return *this;
    }
    matrix_tpl& operator+=(const matrix_product_tpl<ftype> &src) {
        src.add_assign_to(data); return *this;
    }
    matrix_tpl& operator-=(const matrix_product_tpl<ftype> &src) {
        src.sub_assign_to(data); return *this;
    }

    matrix_tpl& allocate() {
        i32 i,sz=nRows*nCols; ftype *prevdata = data;
        if (!data) data = new ftype[sz];
        if (flags & mtx_foreign_data) for(i=0;i<sz;i++) data[i] = prevdata[i];
        return *this;
    }

    matrix_tpl& zero() { for(i32 i=nRows*nCols-1;i>=0;i--) data[i]=0; return *this; }
    matrix_tpl& identity() {
        zero(); for(i32 i=min(nRows,nCols)-1;i>=0;i--) data[i*(nCols+1)] = 1;
        return *this;
    }

    matrix_tpl& invert(); // in-place inversion
    matrix_tpl operator!() const { // returns inverted matrix
        if (flags & mtx_orthogonal)
            return T();
        matrix_tpl<ftype> res = *this;
        res.invert();
        return res;
    }

    matrix_tpl& transpose() { // in-place transposition
        if (nRows==nCols) {
            if ((flags & mtx_symmetric)==0) {
        i32 i,j; ftype t; for(i=0;i<nRows;i++) for(j=0;j<i;j++) {
          t=(*this)[i][j]; (*this)[i][j]=(*this)[j][i]; (*this)[j][i]=t;
        }
      }
        } else
            *this = T();
        return *this;
    }
    matrix_tpl T() const { // returns transposed matrix
        if (flags & mtx_symmetric)
            return matrix_tpl<ftype>(*this);
        i32 i,j; matrix_tpl<ftype> res(nCols,nRows, flags & ~mtx_foreign_data);
        for(i=0;i<nRows;i++) for(j=0;j<nCols;j++) res[j][i] = (*this)[i][j];
        return res;
    }

    i32 LUdecomposition(ftype *&LUdata,i32 *&LUidx) const;
    i32 solveAx_b(ftype *x,ftype *b, ftype *LUdata=0,i32 *LUidx=0) const; // finds x that satisfies Ax=b
    ftype determinant(ftype *LUdata=0,i32 *LUidx=0) const;

    i32 jacobi_transformation(matrix_tpl &evec, ftype* eval, ftype prec=0) const;
    i32 conjugate_gradient(ftype *startx,ftype *rightside, ftype minlen=0,ftype minel=0) const;
    i32 biconjugate_gradient(ftype *startx,ftype *rightside, ftype minlen=0,ftype minel=0) const;
    i32 minimum_residual(ftype *startx,ftype *rightside, ftype minlen=0,ftype minel=0) const;
    i32 LPsimplex(i32 m1,i32 m2, ftype &objfunout,ftype *xout=0, i32 nvars=-1, ftype e=-1) const;

    ftype *operator[](i32 iRow) const { return data + iRow*nCols; }

    i32 nRows,nCols;
    i32 flags;
    ftype *data;

#if defined(USE_MATRIX_POOLS)
    static ftype mtx_pool[];
    static i32 mtx_pool_pos;
    static i32 mtx_pool_size;
#endif
};

/*template<class ftype1,class ftype2>
matrix_tpl<ftype1> operator*(const matrix_tpl<ftype1> &op1, const matrix_tpl<ftype2> &op2) {
    matrix_tpl<ftype1> res(op1.nRows, op2.nCols);
    res.flags = res.flags & mtx_foreign_data | op1.flags & op2.flags & (mtx_orthogonal | mtx_PD) & ~mtx_foreign_data;
    i32 i,j,k; ftype1 sum;
    for(i=0;i<op1.nRows;i++) for(j=0;j<op2.nCols;j++) {
        for(sum=0,k=0; k<op1.nCols; k++) sum += op1[i][k]*op2[k][j];
        res[i][j] = sum;
    }
    return res;
}*/

template<class ftype>
matrix_product_tpl<ftype> operator*(const matrix_tpl<ftype> &op1, const matrix_tpl<ftype> &op2) {
    return matrix_product_tpl<ftype>(op1.nRows,op1.nCols,op1.data,op1.flags, op2.nCols,op2.data,op2.flags);
}

/*template<class ftype1,class ftype2>
matrix_tpl<ftype1>& operator*=(matrix_tpl<ftype1> &op1, const matrix_tpl<ftype2> &op2) {
    return op1 = (op1 * op2);
}*/

template<class ftype>
matrix_tpl<ftype>& operator*=(matrix_tpl<ftype> &op1,ftype op2) {
    for(i32 i=op1.nRows*op1.nCols-1; i>=0; i--) op1.data[i]*=op2;
    op1.flags &= ~(mtx_identity_flag | mtx_PD);
    return op1;
}

template<class ftype1,class ftype2>
matrix_tpl<ftype1>& operator+=(matrix_tpl<ftype1> &op1, const matrix_tpl<ftype2> &op2) {
    for(i32 i=op1.nRows*op1.nCols-1; i>=0; i--) op1.data[i] += op2.data[i];
    op1.flags = op1.flags & mtx_foreign_data | op1.flags & op2.flags & (mtx_symmetric | mtx_PD);
    return op1;
}

template<class ftype1,class ftype2>
matrix_tpl<ftype1>& operator-=(matrix_tpl<ftype1> &op1, const matrix_tpl<ftype2> &op2) {
    for(i32 i=op1.nRows*op1.nCols-1; i>=0; i--) op1.data[i] -= op2.data[i];
    op1.flags = op1.flags & mtx_foreign_data | op1.flags & op2.flags & mtx_symmetric;
    return op1;
}

template<class ftype1,class ftype2>
matrix_tpl<ftype1> operator+(const matrix_tpl<ftype1> &op1, const matrix_tpl<ftype2> &op2) {
    matrix_tpl<ftype1> res; res=op1; res+=op2;
    return res;
}

template<class ftype1,class ftype2>
matrix_tpl<ftype1> operator-(const matrix_tpl<ftype1> &op1, const matrix_tpl<ftype2> &op2) {
    matrix_tpl<ftype1> res; res=op1; res-=op2;
    return res;
}

template<class ftype1,class ftype2,class ftype3>
ftype3 *mul_vector_by_matrix(const matrix_tpl<ftype1> &mtx, const ftype2 *psrc,ftype3 *pdst) {
    i32 i,j;
    for(i=0;i<mtx.nRows;i++) for(pdst[i]=0,j=0;j<mtx.nCols;j++)
        pdst[i] += mtx.data[i*mtx.nCols+j]*psrc[j];
    return pdst;
}

typedef matrix_tpl<real> matrix;
typedef matrix_tpl<float> matrixf;

#if defined(__GNUC__)
  #define DECLARE_MTXNxM_POOL(ftype,sz) template<> ftype matrix_tpl<ftype>::mtx_pool[sz] = {}; \
        template<> i32 matrix_tpl<ftype>::mtx_pool_pos = 0;                          \
        template<> i32 matrix_tpl<ftype>::mtx_pool_size=sz;
#else
    #define DECLARE_MTXNxM_POOL(ftype,sz) template<> ftype matrix_tpl<ftype>::mtx_pool[sz]; template<> i32 matrix_tpl<ftype>::mtx_pool_pos=0; \
        template<> i32 matrix_tpl<ftype>::mtx_pool_size=sz;
#endif //__GNUC__

extern i32 g_bHasSSE;
#ifdef PIII_SSE
void PIII_Mult00_6x6_6x6(float *src1, float *src2, float *dst);
template <>
inline void matrix_product_tpl<float>::assign_to(float *pdst) const {
    if ((g_bHasSSE^1|nRows1-6|nCols1-6|nCols2-6)==0)
        PIII_Mult00_6x6_6x6(data1,data2, pdst);
    else {
        i32 i,j,k; float sum;
        for(i=0;i<nRows1;i++) for(j=0;j<nCols2;j++) {
            for(sum=0,k=0; k<nCols1; k++)
                sum += data1[i*nCols1+k]*data2[k*nCols2+j];
            pdst[i*nCols2+j] = sum;
        }
    }
}
#endif

#endif
