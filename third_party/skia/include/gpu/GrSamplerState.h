
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrSamplerState_DEFINED
#define GrSamplerState_DEFINED

#include "GrTypes.h"
#include "GrMatrix.h"

#define MAX_KERNEL_WIDTH 25

class GrSamplerState {
public:
    enum Filter {
        /**
         * Read the closest src texel to the sample position
         */
        kNearest_Filter,
        /**
         * Blend between closest 4 src texels to sample position (tent filter)
         */
        kBilinear_Filter,
        /**
         * Average of 4 bilinear filterings spaced +/- 1 texel from sample
         * position in x and y. Intended for averaging 16 texels in a downsample
         * pass. (rasterizing such that texture samples fall exactly halfway
         * between texels in x and y spaced 4 texels apart.) Only supported
         * on shader backends.
         */
        k4x4Downsample_Filter,
        /**
         * Apply a separable convolution kernel.
         */
        kConvolution_Filter
    };

    /**
     * The intepretation of the texture matrix depends on the sample mode. The
     * texture matrix is applied both when the texture coordinates are explicit
     * and  when vertex positions are used as texture  coordinates. In the latter
     * case the texture matrix is applied to the pre-view-matrix position 
     * values.
     *
     * kNormal_SampleMode
     *  The post-matrix texture coordinates are in normalize space with (0,0) at
     *  the top-left and (1,1) at the bottom right.
     * kRadial_SampleMode
     *  The matrix specifies the radial gradient parameters.
     *  (0,0) in the post-matrix space is center of the radial gradient.
     * kRadial2_SampleMode
     *   Matrix transforms to space where first circle is centered at the
     *   origin. The second circle will be centered (x, 0) where x may be 
     *   0 and is provided by setRadial2Params. The post-matrix space is 
     *   normalized such that 1 is the second radius - first radius.
     * kSweepSampleMode
     *  The angle from the origin of texture coordinates in post-matrix space
     *  determines the gradient value.
     */
    enum SampleMode {
        kNormal_SampleMode,     //!< sample color directly
        kRadial_SampleMode,     //!< treat as radial gradient
        kRadial2_SampleMode,    //!< treat as 2-point radial gradient
        kSweep_SampleMode,      //!< treat as sweep gradient
    };

    /**
     * Describes how a texture is sampled when coordinates are outside the
     * texture border
     */
    enum WrapMode {
        kClamp_WrapMode,
        kRepeat_WrapMode,
        kMirror_WrapMode
    };

    /**
     * Default sampler state is set to clamp, use normal sampling mode, be
     * unfiltered, and use identity matrix.
     */
    GrSamplerState()
    : fRadial2CenterX1()
    , fRadial2Radius0()
    , fRadial2PosRoot() {
        this->setClampNoFilter();
    }

    explicit GrSamplerState(Filter filter)
    : fRadial2CenterX1()
    , fRadial2Radius0()
    , fRadial2PosRoot() {
        fWrapX = kClamp_WrapMode;
        fWrapY = kClamp_WrapMode;
        fSampleMode = kNormal_SampleMode;
        fFilter = filter;
        fMatrix.setIdentity();
        fTextureDomain.setEmpty();
    }

    GrSamplerState(WrapMode wx, WrapMode wy, Filter filter)
    : fRadial2CenterX1()
    , fRadial2Radius0()
    , fRadial2PosRoot() {
        fWrapX = wx;
        fWrapY = wy;
        fSampleMode = kNormal_SampleMode;
        fFilter = filter;
        fMatrix.setIdentity();
        fSwapRAndB = false;
        fTextureDomain.setEmpty();
    }

    GrSamplerState(WrapMode wx, WrapMode wy, 
                   const GrMatrix& matrix, Filter filter)
    : fRadial2CenterX1()
    , fRadial2Radius0()
    , fRadial2PosRoot() {
        fWrapX = wx;
        fWrapY = wy;
        fSampleMode = kNormal_SampleMode;
        fFilter = filter;
        fMatrix = matrix;
        fSwapRAndB = false;
        fTextureDomain.setEmpty();
    }

    GrSamplerState(WrapMode wx, WrapMode wy, SampleMode sample, 
                   const GrMatrix& matrix, Filter filter)
    : fRadial2CenterX1()
    , fRadial2Radius0()
    , fRadial2PosRoot() {
        fWrapX = wx;
        fWrapY = wy;
        fSampleMode = sample;
        fMatrix = matrix;
        fFilter = filter;
        fSwapRAndB = false;
        fTextureDomain.setEmpty();
    }

    WrapMode getWrapX() const { return fWrapX; }
    WrapMode getWrapY() const { return fWrapY; }
    SampleMode getSampleMode() const { return fSampleMode; }
    const GrMatrix& getMatrix() const { return fMatrix; }
    const GrRect& getTextureDomain() const { return fTextureDomain; }
    bool hasTextureDomain() const {return SkIntToScalar(0) != fTextureDomain.right();}
    Filter getFilter() const { return fFilter; }
    int getKernelWidth() const { return fKernelWidth; }
    const float* getKernel() const { return fKernel; }
    const float* getImageIncrement() const { return fImageIncrement; }
    bool swapsRAndB() const { return fSwapRAndB; }

    bool isGradient() const {
        return  kRadial_SampleMode == fSampleMode ||
                kRadial2_SampleMode == fSampleMode ||
                kSweep_SampleMode == fSampleMode;
    }

    void setWrapX(WrapMode mode) { fWrapX = mode; }
    void setWrapY(WrapMode mode) { fWrapY = mode; }
    void setSampleMode(SampleMode mode) { fSampleMode = mode; }
    
    /**
     * Sets the sampler's matrix. See SampleMode for explanation of
     * relationship between the matrix and sample mode.
     * @param matrix the matrix to set
     */
    void setMatrix(const GrMatrix& matrix) { fMatrix = matrix; }
    
    /**
     * Sets the sampler's texture coordinate domain to a 
     * custom rectangle, rather than the default (0,1).
     * This option is currently only supported with kClamp_WrapMode
     */
    void setTextureDomain(const GrRect& textureDomain) { fTextureDomain = textureDomain; }

    /**
     * Swaps the R and B components when reading from the texture. Has no effect
     * if the texture is alpha only.
     */
    void setRAndBSwap(bool swap) { fSwapRAndB = swap; }

    /**
     *  Multiplies the current sampler matrix  a matrix
     *
     *  After this call M' = M*m where M is the old matrix, m is the parameter
     *  to this function, and M' is the new matrix. (We consider points to
     *  be column vectors so tex cood vector t is transformed by matrix X as 
     *  t' = X*t.)
     *
     *  @param matrix   the matrix used to modify the matrix.
     */
    void preConcatMatrix(const GrMatrix& matrix) { fMatrix.preConcat(matrix); }

    /**
     * Sets filtering type.
     * @param filter    type of filtering to apply
     */
    void setFilter(Filter filter) { fFilter = filter; }

    void setClampNoFilter() {
        fWrapX = kClamp_WrapMode;
        fWrapY = kClamp_WrapMode;
        fSampleMode = kNormal_SampleMode;
        fFilter = kNearest_Filter;
        fMatrix.setIdentity();
        fTextureDomain.setEmpty();
        fSwapRAndB = false;
    }

    GrScalar getRadial2CenterX1() const { return fRadial2CenterX1; }
    GrScalar getRadial2Radius0() const { return fRadial2Radius0; }
    bool     isRadial2PosRoot() const { return SkToBool(fRadial2PosRoot); }
    // do the radial gradient params lead to a linear (rather than quadratic)
    // equation.
    bool radial2IsDegenerate() const { return GR_Scalar1 == fRadial2CenterX1; }

    /**
     * Sets the parameters for kRadial2_SampleMode. The texture
     * matrix must be set so that the first point is at (0,0) and the second
     * point lies on the x-axis. The second radius minus the first is 1 unit.
     * The additional parameters to define the gradient are specified by this
     * function.
     */
    void setRadial2Params(GrScalar centerX1, GrScalar radius0, bool posRoot) {
        fRadial2CenterX1 = centerX1;
        fRadial2Radius0 = radius0;
        fRadial2PosRoot = posRoot;
    }

    void setConvolutionParams(int kernelWidth, const float* kernel, float imageIncrement[2]) {
        GrAssert(kernelWidth >= 0 && kernelWidth <= MAX_KERNEL_WIDTH);
        fKernelWidth = kernelWidth;
        if (NULL != kernel) {
            memcpy(fKernel, kernel, kernelWidth * sizeof(float));
        }
        if (NULL != imageIncrement) {
            memcpy(fImageIncrement, imageIncrement, sizeof(fImageIncrement));
        } else {
            memset(fImageIncrement, 0, sizeof(fImageIncrement));
        }
    }

    static const GrSamplerState& ClampNoFilter() {
        return gClampNoFilter;
    }

private:
    WrapMode    fWrapX : 8;
    WrapMode    fWrapY : 8;
    SampleMode  fSampleMode : 8;
    Filter      fFilter : 8;
    GrMatrix    fMatrix;
    bool        fSwapRAndB;
    GrRect      fTextureDomain;

    // these are undefined unless fSampleMode == kRadial2_SampleMode
    GrScalar    fRadial2CenterX1;
    GrScalar    fRadial2Radius0;
    SkBool8     fRadial2PosRoot;

    // These are undefined unless fFilter == kConvolution_Filter
    uint8_t     fKernelWidth;
    float       fImageIncrement[2];
    float       fKernel[MAX_KERNEL_WIDTH];

    static const GrSamplerState gClampNoFilter;
};

#endif

