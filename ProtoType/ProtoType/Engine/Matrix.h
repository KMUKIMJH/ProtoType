/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Matrix.h
Project:    CS230 Engine
Author:     Minchan Cho
Created:    April 2, 2025
*/

#pragma once
#include "Vec2.h"
#include <cmath>

namespace Math {
    class TransformationMatrix {
    public:
        TransformationMatrix();

        const double* operator[](int index) const { return matrix[index]; }
        TransformationMatrix operator * (TransformationMatrix m) const;
        TransformationMatrix& operator *= (TransformationMatrix m);
        void Reset();
        vec2 operator * (vec2 v) const;

    protected:
        double matrix[3][3];
    };

    class TranslationMatrix : public TransformationMatrix {
    public:
        TranslationMatrix(ivec2 translate);
        TranslationMatrix(vec2 translate);
    };
    class ScaleMatrix : public TransformationMatrix {
    public:
        ScaleMatrix(double scale);
        ScaleMatrix(vec2 scale);
    };


    class RotationMatrix : public TransformationMatrix {
    public:
        RotationMatrix(double theta);
    };

}
