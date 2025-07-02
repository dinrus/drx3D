#ifndef DRX3D_MATRIX_X_H
#define DRX3D_MATRIX_X_H

#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <stdio.h>

//#define DRX3D_DEBUG_OSTREAM
#ifdef DRX3D_DEBUG_OSTREAM
#include <iostream>
#include <iomanip>  // std::setw
#endif              //DRX3D_DEBUG_OSTREAM

class IntSortPredicate
{
public:
    bool operator()(i32k& a, i32k& b) const
    {
        return a < b;
    }
};

template <typename T>
struct VectorX
{
    AlignedObjectArray<T> m_storage;

    VectorX()
    {
    }
    VectorX(i32 numRows)
    {
        m_storage.resize(numRows);
    }

    void resize(i32 rows)
    {
        m_storage.resize(rows);
    }
    i32 cols() const
    {
        return 1;
    }
    i32 rows() const
    {
        return m_storage.size();
    }
    i32 size() const
    {
        return rows();
    }

    T nrm2() const
    {
        T norm = T(0);

        i32 nn = rows();

        {
            if (nn == 1)
            {
                norm = Fabs((*this)[0]);
            }
            else
            {
                T scale = 0.0;
                T ssq = 1.0;

                /* The following loop is equivalent to this call to the LAPACK
                 auxiliary routine:   CALL SLASSQ( N, X, INCX, SCALE, SSQ ) */

                for (i32 ix = 0; ix < nn; ix++)
                {
                    if ((*this)[ix] != 0.0)
                    {
                        T absxi = Fabs((*this)[ix]);
                        if (scale < absxi)
                        {
                            T temp;
                            temp = scale / absxi;
                            ssq = ssq * (temp * temp) + DRX3D_ONE;
                            scale = absxi;
                        }
                        else
                        {
                            T temp;
                            temp = absxi / scale;
                            ssq += temp * temp;
                        }
                    }
                }
                norm = scale * sqrt(ssq);
            }
        }
        return norm;
    }
    void setZero()
    {
        if (m_storage.size())
        {
            //  for (i32 i=0;i<m_storage.size();i++)
            //      m_storage[i]=0;
            //memset(&m_storage[0],0,sizeof(T)*m_storage.size());
            SetZero(&m_storage[0], m_storage.size());
        }
    }
    const T& operator[](i32 index) const
    {
        return m_storage[index];
    }

    T& operator[](i32 index)
    {
        return m_storage[index];
    }

    T* getBufferPointerWritable()
    {
        return m_storage.size() ? &m_storage[0] : 0;
    }

    const T* getBufferPointer() const
    {
        return m_storage.size() ? &m_storage[0] : 0;
    }
};
/*
 template <typename T>
 void setElem(MatrixX<T>& mat, i32 row, i32 col, T val)
 {
 mat.setElem(row,col,val);
 }
 */

template <typename T>
struct MatrixX
{
    i32 m_rows;
    i32 m_cols;
    i32 m_operations;
    i32 m_resizeOperations;
    i32 m_setElemOperations;

    AlignedObjectArray<T> m_storage;
    mutable AlignedObjectArray<AlignedObjectArray<i32> > m_rowNonZeroElements1;

    T* getBufferPointerWritable()
    {
        return m_storage.size() ? &m_storage[0] : 0;
    }

    const T* getBufferPointer() const
    {
        return m_storage.size() ? &m_storage[0] : 0;
    }
    MatrixX()
        : m_rows(0),
          m_cols(0),
          m_operations(0),
          m_resizeOperations(0),
          m_setElemOperations(0)
    {
    }
    MatrixX(i32 rows, i32 cols)
        : m_rows(rows),
          m_cols(cols),
          m_operations(0),
          m_resizeOperations(0),
          m_setElemOperations(0)
    {
        resize(rows, cols);
    }
    void resize(i32 rows, i32 cols)
    {
        m_resizeOperations++;
        m_rows = rows;
        m_cols = cols;
        {
            DRX3D_PROFILE("m_storage.resize");
            m_storage.resize(rows * cols);
        }
    }
    i32 cols() const
    {
        return m_cols;
    }
    i32 rows() const
    {
        return m_rows;
    }
    ///we don't want this read/write operator(), because we cannot keep track of non-zero elements, use setElem instead
    /*T& operator() (i32 row,i32 col)
    {
        return m_storage[col*m_rows+row];
    }
    */

    void addElem(i32 row, i32 col, T val)
    {
        if (val)
        {
            if (m_storage[col + row * m_cols] == 0.f)
            {
                setElem(row, col, val);
            }
            else
            {
                m_storage[row * m_cols + col] += val;
            }
        }
    }

    void setElem(i32 row, i32 col, T val)
    {
        m_setElemOperations++;
        m_storage[row * m_cols + col] = val;
    }

    void mulElem(i32 row, i32 col, T val)
    {
        m_setElemOperations++;
        //mul doesn't change sparsity info

        m_storage[row * m_cols + col] *= val;
    }

    void copyLowerToUpperTriangle()
    {
        i32 count = 0;
        for (i32 row = 0; row < rows(); row++)
        {
            for (i32 col = 0; col < row; col++)
            {
                setElem(col, row, (*this)(row, col));
                count++;
            }
        }
        //printf("copyLowerToUpperTriangle copied %d elements out of %dx%d=%d\n", count,rows(),cols(),cols()*rows());
    }

    const T& operator()(i32 row, i32 col) const
    {
        return m_storage[col + row * m_cols];
    }

    void setZero()
    {
        {
            DRX3D_PROFILE("storage=0");
            if (m_storage.size())
            {
                SetZero(&m_storage[0], m_storage.size());
            }
            //memset(&m_storage[0],0,sizeof(T)*m_storage.size());
            //for (i32 i=0;i<m_storage.size();i++)
            //          m_storage[i]=0;
        }
    }

    void setIdentity()
    {
        Assert(rows() == cols());

        setZero();
        for (i32 row = 0; row < rows(); row++)
        {
            setElem(row, row, 1);
        }
    }

    void printMatrix(tukk msg) const
    {
        printf("%s ---------------------\n", msg);
        for (i32 i = 0; i < rows(); i++)
        {
            printf("\n");
            for (i32 j = 0; j < cols(); j++)
            {
                printf("%2.1f\t", (*this)(i, j));
            }
        }
        printf("\n---------------------\n");
    }

    void rowComputeNonZeroElements() const
    {
        m_rowNonZeroElements1.resize(rows());
        for (i32 i = 0; i < rows(); i++)
        {
            m_rowNonZeroElements1[i].resize(0);
            for (i32 j = 0; j < cols(); j++)
            {
                if ((*this)(i, j) != 0.f)
                {
                    m_rowNonZeroElements1[i].push_back(j);
                }
            }
        }
    }
    MatrixX transpose() const
    {
        //transpose is optimized for sparse matrices
        MatrixX tr(m_cols, m_rows);
        tr.setZero();
        for (i32 i = 0; i < m_cols; i++)
            for (i32 j = 0; j < m_rows; j++)
            {
                T v = (*this)(j, i);
                if (v)
                {
                    tr.setElem(i, j, v);
                }
            }
        return tr;
    }

    MatrixX operator*(const MatrixX& other)
    {
        //MatrixX*MatrixX implementation, brute force
        Assert(cols() == other.rows());

        MatrixX res(rows(), other.cols());
        res.setZero();
        //      DRX3D_PROFILE("MatrixX mul");
        for (i32 i = 0; i < rows(); ++i)
        {
            {
                for (i32 j = 0; j < other.cols(); ++j)
                {
                    T dotProd = 0;
                    {
                        {
                            i32 c = cols();

                            for (i32 k = 0; k < c; k++)
                            {
                                T w = (*this)(i, k);
                                if (other(k, j) != 0.f)
                                {
                                    dotProd += w * other(k, j);
                                }
                            }
                        }
                    }
                    if (dotProd)
                        res.setElem(i, j, dotProd);
                }
            }
        }
        return res;
    }

    // this assumes the 4th and 8th rows of B and C are zero.
    void multiplyAdd2_p8r(const Scalar* B, const Scalar* C, i32 numRows, i32 numRowsOther, i32 row, i32 col)
    {
        const Scalar* bb = B;
        for (i32 i = 0; i < numRows; i++)
        {
            const Scalar* cc = C;
            for (i32 j = 0; j < numRowsOther; j++)
            {
                Scalar sum;
                sum = bb[0] * cc[0];
                sum += bb[1] * cc[1];
                sum += bb[2] * cc[2];
                sum += bb[4] * cc[4];
                sum += bb[5] * cc[5];
                sum += bb[6] * cc[6];
                addElem(row + i, col + j, sum);
                cc += 8;
            }
            bb += 8;
        }
    }

    void multiply2_p8r(const Scalar* B, const Scalar* C, i32 numRows, i32 numRowsOther, i32 row, i32 col)
    {
        Assert(numRows > 0 && numRowsOther > 0 && B && C);
        const Scalar* bb = B;
        for (i32 i = 0; i < numRows; i++)
        {
            const Scalar* cc = C;
            for (i32 j = 0; j < numRowsOther; j++)
            {
                Scalar sum;
                sum = bb[0] * cc[0];
                sum += bb[1] * cc[1];
                sum += bb[2] * cc[2];
                sum += bb[4] * cc[4];
                sum += bb[5] * cc[5];
                sum += bb[6] * cc[6];
                setElem(row + i, col + j, sum);
                cc += 8;
            }
            bb += 8;
        }
    }

    void setSubMatrix(i32 rowstart, i32 colstart, i32 rowend, i32 colend, const T value)
    {
        i32 numRows = rowend + 1 - rowstart;
        i32 numCols = colend + 1 - colstart;

        for (i32 row = 0; row < numRows; row++)
        {
            for (i32 col = 0; col < numCols; col++)
            {
                setElem(rowstart + row, colstart + col, value);
            }
        }
    }

    void setSubMatrix(i32 rowstart, i32 colstart, i32 rowend, i32 colend, const MatrixX& block)
    {
        Assert(rowend + 1 - rowstart == block.rows());
        Assert(colend + 1 - colstart == block.cols());
        for (i32 row = 0; row < block.rows(); row++)
        {
            for (i32 col = 0; col < block.cols(); col++)
            {
                setElem(rowstart + row, colstart + col, block(row, col));
            }
        }
    }
    void setSubMatrix(i32 rowstart, i32 colstart, i32 rowend, i32 colend, const VectorX<T>& block)
    {
        Assert(rowend + 1 - rowstart == block.rows());
        Assert(colend + 1 - colstart == block.cols());
        for (i32 row = 0; row < block.rows(); row++)
        {
            for (i32 col = 0; col < block.cols(); col++)
            {
                setElem(rowstart + row, colstart + col, block[row]);
            }
        }
    }

    MatrixX negative()
    {
        MatrixX neg(rows(), cols());
        for (i32 i = 0; i < rows(); i++)
            for (i32 j = 0; j < cols(); j++)
            {
                T v = (*this)(i, j);
                neg.setElem(i, j, -v);
            }
        return neg;
    }
};

typedef MatrixX<float> MatrixXf;
typedef VectorX<float> VectorXf;

typedef MatrixX<double> MatrixXd;
typedef VectorX<double> VectorXd;

#ifdef DRX3D_DEBUG_OSTREAM
template <typename T>
std::ostream& operator<<(std::ostream& os, const MatrixX<T>& mat)
{
    os << " [";
    //printf("%s ---------------------\n",msg);
    for (i32 i = 0; i < mat.rows(); i++)
    {
        for (i32 j = 0; j < mat.cols(); j++)
        {
            os << std::setw(12) << mat(i, j);
        }
        if (i != mat.rows() - 1)
            os << std::endl
               << "  ";
    }
    os << " ]";
    //printf("\n---------------------\n");

    return os;
}
template <typename T>
std::ostream& operator<<(std::ostream& os, const VectorX<T>& mat)
{
    os << " [";
    //printf("%s ---------------------\n",msg);
    for (i32 i = 0; i < mat.rows(); i++)
    {
        os << std::setw(12) << mat[i];
        if (i != mat.rows() - 1)
            os << std::endl
               << "  ";
    }
    os << " ]";
    //printf("\n---------------------\n");

    return os;
}

#endif  //DRX3D_DEBUG_OSTREAM

inline void setElem(MatrixXd& mat, i32 row, i32 col, double val)
{
    mat.setElem(row, col, val);
}

inline void setElem(MatrixXf& mat, i32 row, i32 col, float val)
{
    mat.setElem(row, col, val);
}

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define VectorXu VectorXd
#define MatrixXu MatrixXd
#else
#define VectorXu VectorXf
#define MatrixXu MatrixXf
#endif  //DRX3D_USE_DOUBLE_PRECISION

#endif  //DRX3D_MATRIX_H_H
