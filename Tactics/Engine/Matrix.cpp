#include "stdafx.h"
#include "Matrix.h"
#include "Coordinate.h"
#include "Vector.h"
#include <gl/GL.h>

namespace Engine
{


Matrix::Matrix(const std::initializer_list<double>& v)
{
    unsigned i = 0;
    for (const auto& element : v)
    {
        m[i >> 2][i & 3] = element;
        if (++i > 15)
            break;
    }
}

Matrix::Matrix(const Vector& x, const Vector& y, const Vector& z, const Vector& t)
{
    m[0][0] = x.x;  m[1][0] = y.x;  m[2][0] = z.x; m[3][0] = t.x;
    m[0][1] = x.y;  m[1][1] = y.y;  m[2][1] = z.y; m[3][1] = t.y;
    m[0][2] = x.z;  m[1][2] = y.z;  m[2][2] = z.z; m[3][2] = t.z;
    m[0][3] = 0;    m[1][3] = 0;    m[2][3] = 0;   m[3][3] = 1;
}

Matrix Matrix::Rotation() const
{
    Matrix result;
    auto xl = X().Length();
    auto yl = Y().Length();
    auto zl = Z().Length();
    result[0][0] = m[0][0] / xl;  result[1][0] = m[1][0] / xl; result[2][0] = m[2][0] / xl; result[3][0] = 0;
    result[0][1] = m[0][1] / yl;  result[1][1] = m[1][1] / yl; result[2][1] = m[2][1] / yl; result[3][1] = 0;
    result[0][2] = m[0][2] / zl;  result[1][2] = m[1][2] / zl; result[2][2] = m[2][2] / zl; result[3][2] = 0;
    result[0][3] = 0;             result[1][3] = 0;            result[2][3] = 0;            result[3][3] = 1;

    return result;
}

bool Matrix::IsAffine() const
{
    return m[0][3] == 0 &&
        m[1][3] == 0 &&
        m[2][3] == 0;
}

bool Matrix::IsIdentity() const
{
    for (unsigned col = 0; col < 4; ++col)
    {
        if (m[col][col] != 1)
            return false;
        for (unsigned row = 0; row < col; ++row)
        {
            if (m[col][row] != 0)
                return false;
        }
        for (unsigned row = col + 1; row < 4; ++row)
        {
            if (m[col][row] != 0)
                return false;
        }
    }
    return true;
}

Matrix Matrix::Inverse() const
{
    Matrix result;
    if (false) // IsAffine())
    {   // this also doesn't invert scaling, which would require sqrt and probably make it slower than the elaborate way
        // transpose the rotation
        result[0][0] = m[0][0];  result[1][0] = m[0][1]; result[2][0] = m[0][2];
        result[0][1] = m[1][0];  result[1][1] = m[1][1]; result[2][1] = m[1][2];
        result[0][2] = m[2][0];  result[1][2] = m[2][1]; result[2][2] = m[2][2];
        // rotate and inverse the translation 
        result[3][0] = -(result[0][0] * m[3][0] + result[1][0] * m[3][1] + result[2][0] * m[3][2]);
        result[3][1] = -(result[0][1] * m[3][0] + result[1][1] * m[3][1] + result[2][1] * m[3][2]);
        result[3][2] = -(result[0][2] * m[3][0] + result[1][2] * m[3][1] + result[2][2] * m[3][2]);

        result[0][3] = result[1][3] = result[2][3] = 0;
        result[3][3] = m[3][3];
    /*    // http://bookofhook.com/mousepick.pdf
        result[0][3] = result[1][3] = 0;
        double perspective = m[2][3];
        double w = m[3][3];

        result[3][2] /= perspective;
        result[2][3] = 1 / m[3][2];
        result[3][3] = m[3][3] / -(m[2][2] / (perspective*w));
    */
    }
    else
    {
        result[0][0] = m[1][1] * m[2][2] * m[3][3] -
            m[1][1] * m[2][3] * m[3][2] -
            m[2][1] * m[1][2] * m[3][3] +
            m[2][1] * m[1][3] * m[3][2] +
            m[3][1] * m[1][2] * m[2][3] -
            m[3][1] * m[1][3] * m[2][2];

        result[1][0] = -m[1][0] * m[2][2] * m[3][3] +
            m[1][0] * m[2][3] * m[3][2] +
            m[2][0] * m[1][2] * m[3][3] -
            m[2][0] * m[1][3] * m[3][2] -
            m[3][0] * m[1][2] * m[2][3] +
            m[3][0] * m[1][3] * m[2][2];

        result[2][0] = m[1][0] * m[2][1] * m[3][3] -
            m[1][0] * m[2][3] * m[3][1] -
            m[2][0] * m[1][1] * m[3][3] +
            m[2][0] * m[1][3] * m[3][1] +
            m[3][0] * m[1][1] * m[2][3] -
            m[3][0] * m[1][3] * m[2][1];

        result[3][0] = -m[1][0] * m[2][1] * m[3][2] +
            m[1][0] * m[2][2] * m[3][1] +
            m[2][0] * m[1][1] * m[3][2] -
            m[2][0] * m[1][2] * m[3][1] -
            m[3][0] * m[1][1] * m[2][2] +
            m[3][0] * m[1][2] * m[2][1];

        result[0][1] = -m[0][1] * m[2][2] * m[3][3] +
            m[0][1] * m[2][3] * m[3][2] +
            m[2][1] * m[0][2] * m[3][3] -
            m[2][1] * m[0][3] * m[3][2] -
            m[3][1] * m[0][2] * m[2][3] +
            m[3][1] * m[0][3] * m[2][2];

        result[1][1] = m[0][0] * m[2][2] * m[3][3] -
            m[0][0] * m[2][3] * m[3][2] -
            m[2][0] * m[0][2] * m[3][3] +
            m[2][0] * m[0][3] * m[3][2] +
            m[3][0] * m[0][2] * m[2][3] -
            m[3][0] * m[0][3] * m[2][2];

        result[2][1] = -m[0][0] * m[2][1] * m[3][3] +
            m[0][0] * m[2][3] * m[3][1] +
            m[2][0] * m[0][1] * m[3][3] -
            m[2][0] * m[0][3] * m[3][1] -
            m[3][0] * m[0][1] * m[2][3] +
            m[3][0] * m[0][3] * m[2][1];

        result[3][1] = m[0][0] * m[2][1] * m[3][2] -
            m[0][0] * m[2][2] * m[3][1] -
            m[2][0] * m[0][1] * m[3][2] +
            m[2][0] * m[0][2] * m[3][1] +
            m[3][0] * m[0][1] * m[2][2] -
            m[3][0] * m[0][2] * m[2][1];

        result[0][2] = m[0][1] * m[1][2] * m[3][3] -
            m[0][1] * m[1][3] * m[3][2] -
            m[1][1] * m[0][2] * m[3][3] +
            m[1][1] * m[0][3] * m[3][2] +
            m[3][1] * m[0][2] * m[1][3] -
            m[3][1] * m[0][3] * m[1][2];

        result[1][2] = -m[0][0] * m[1][2] * m[3][3] +
            m[0][0] * m[1][3] * m[3][2] +
            m[1][0] * m[0][2] * m[3][3] -
            m[1][0] * m[0][3] * m[3][2] -
            m[3][0] * m[0][2] * m[1][3] +
            m[3][0] * m[0][3] * m[1][2];

        result[2][2] = m[0][0] * m[1][1] * m[3][3] -
            m[0][0] * m[1][3] * m[3][1] -
            m[1][0] * m[0][1] * m[3][3] +
            m[1][0] * m[0][3] * m[3][1] +
            m[3][0] * m[0][1] * m[1][3] -
            m[3][0] * m[0][3] * m[1][1];

        result[3][2] = -m[0][0] * m[1][1] * m[3][2] +
            m[0][0] * m[1][2] * m[3][1] +
            m[1][0] * m[0][1] * m[3][2] -
            m[1][0] * m[0][2] * m[3][1] -
            m[3][0] * m[0][1] * m[1][2] +
            m[3][0] * m[0][2] * m[1][1];

        result[0][3] = -m[0][1] * m[1][2] * m[2][3] +
            m[0][1] * m[1][3] * m[2][2] +
            m[1][1] * m[0][2] * m[2][3] -
            m[1][1] * m[0][3] * m[2][2] -
            m[2][1] * m[0][2] * m[1][3] +
            m[2][1] * m[0][3] * m[1][2];

        result[1][3] = m[0][0] * m[1][2] * m[2][3] -
            m[0][0] * m[1][3] * m[2][2] -
            m[1][0] * m[0][2] * m[2][3] +
            m[1][0] * m[0][3] * m[2][2] +
            m[2][0] * m[0][2] * m[1][3] -
            m[2][0] * m[0][3] * m[1][2];

        result[2][3] = -m[0][0] * m[1][1] * m[2][3] +
            m[0][0] * m[1][3] * m[2][1] +
            m[1][0] * m[0][1] * m[2][3] -
            m[1][0] * m[0][3] * m[2][1] -
            m[2][0] * m[0][1] * m[1][3] +
            m[2][0] * m[0][3] * m[1][1];

        result[3][3] = m[0][0] * m[1][1] * m[2][2] -
            m[0][0] * m[1][2] * m[2][1] -
            m[1][0] * m[0][1] * m[2][2] +
            m[1][0] * m[0][2] * m[2][1] +
            m[2][0] * m[0][1] * m[1][2] -
            m[2][0] * m[0][2] * m[1][1];

        double det = m[0][0] * result[0][0] + m[0][1] * result[1][0] + m[0][2] * result[2][0] + m[0][3] * result[3][0];

        if (det == 0)
            throw std::runtime_error("Uninvertable matrix");

        for (unsigned col = 0; col < 4; ++col)
        {
            for (unsigned row = 0; row < 4; ++row)
            {
                result[col][row] /= det;
            }
        }
    }
    return result;
}

void Multiply(Matrix& o, const Matrix& l, const Matrix& r)
{
    for (unsigned col = 0; col < 4; ++col)
    {
        for (unsigned row = 0; row < 4; ++row)
        {
            o[col][row] =
                l[0][row] * r[col][0] +
                l[1][row] * r[col][1] +
                l[2][row] * r[col][2] +
                l[3][row] * r[col][3];
        }
    }
}

Matrix Matrix::NormalScale() const
{
    Vector x = X().Normal();
    Vector y = Y().Normal();
    Vector z = Z().Normal();
    return Matrix(x, y, z, Translation());
}

Matrix& Matrix::operator*=(const Matrix& o)
{
    Multiply(*this, Matrix(*this), o);

    return *this;
}

const double& Matrix::data() const
{
    return m[0][0];
}

/*
Vector Matrix::X() const
{
    return Vector(m[0][0], m[0][1], m[0][2]);
}
Vector Matrix::Y() const
{
    return Vector(m[1][0], m[1][1], m[1][2]);
}

Vector Matrix::Z() const
{
    return Vector(m[2][0], m[2][1], m[2][2]);
}
*/
Vector Matrix::X() const
{
    return Vector(m[0][0], m[1][0], m[2][0]);
}
Vector Matrix::Y() const
{
    return Vector(m[0][1], m[1][1], m[2][1]);
}

Vector Matrix::Z() const
{
    return Vector(m[0][2], m[1][2], m[2][2]);
}

Vector Matrix::Scale() const
{
    return Vector(double(X().Length()),
        double(Y().Length()),
        double(Z().Length()));
}

Vector Matrix::Translation() const
{
    return Vector(m[3][0], m[3][1], m[3][2]);
}

void Matrix::SetTranslation(const Vector& translation)
{
    m[3][0] = translation.x;
    m[3][1] = translation.y;
    m[3][2] = translation.z;
}


Matrix Matrix::Projection()
{
    Matrix matrix;
    assert(sizeof(double) == sizeof(GLdouble));
    glGetDoublev(GL_PROJECTION_MATRIX, &matrix[0][0]);
    return matrix;

}

Matrix Matrix::Identity()
{
    return Scale(Vector(1, 1, 1));
}

Matrix Matrix::Scale(const Vector& scale)
{
    Matrix matrix;
    memset(&matrix[0][0], 0, sizeof(matrix[0][0]) * 4 * 4);
    matrix[0][0] = scale.x;
    matrix[1][1] = scale.y;
    matrix[2][2] = scale.z;
    matrix[3][3] = 1;
    return matrix;
}

Matrix Matrix::Translation(const Vector& vector)
{
    Matrix matrix = Identity();
    matrix[3][0] = vector.x;
    matrix[3][1] = vector.y;
    matrix[3][2] = vector.z;
    return matrix;
}

Matrix Matrix::Perspective(double n, double f)
{
    Matrix matrix = Matrix::Identity();
    matrix[2][3] = -1;
    matrix[3][3] = 0;
    matrix[2][2] = -f / (f - n);
    matrix[3][2] = -f * n / (f - n);
    return matrix;
}




Matrix operator*(const Matrix& l, const Matrix& r)
{
    Matrix m;
    Multiply(m, l, r);
    return m;
}

}