
#ifndef __VEDNN__
#define __VEDNN__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    VEDNN_SUCCESS = 0,
    VEDNN_ERROR_INVALID_PARAM   = 1,
    VEDNN_ERROR_MEMORY_EXHAUST  = 2,
} vednnError_t;

typedef enum {
    DTYPE_FLOAT,
    // DTYPE_DOUBLE,
} dataType_t;

typedef enum {
    VEDNN_ACTIVATION_RELU = 0,
} vednnActivationMode_t;

typedef enum {
    VEDNN_SOFTMAX_FAST = 0,
    VEDNN_SOFTMAX_ACCURATE,
    VEDNN_SOFTMAX_LOG
} vednnSoftmaxMode_t;

/* Convolution Parametes */

typedef enum {
    VEDNN_CONV_ALGORITHM_DIRECT,
//    VEDNN_CONV_ALGORITHM_GEMM,
} vednnConvolutionAlgorithm_t;

typedef struct {
    dataType_t	dtype;
    int		batch;
    int		channel;
    int		width;
    int		height;
} vednnTensorParam_t;

typedef struct {
    dataType_t	dtype;
    int		channel;
} vednnBiasParam_t;

typedef struct {
    dataType_t	dtype;
    int		inChannel;	// inChannel / group
    int		outChannel;	// outChannel / group
    int		width;
    int		height;
} vednnFilterParam_t;

typedef struct {
    int		group;
    int		strideWidth;
    int		strideHeight;
    int		padWidth;
    int		padHeight;
    int		dilationWidth;
    int		dilationHeight;
} vednnConvolutionParam_t ;

typedef struct {
    int		windowWidth;
    int		windowHeight;
    int		strideWidth;
    int		strideHeight;
    int		padWidth;
    int		padHeight;
} vednnPoolingParam_t ;

vednnError_t vednnConvolutionForward(
    const vednnTensorParam_t 		*pParamIn,
    const void 				*pDataIn,
    const vednnFilterParam_t		*pParamKernel,
    const void 				*pDataKernel,
    const vednnTensorParam_t 		*pParamOut,
    void 				*pDataOut,
    const vednnConvolutionParam_t	*pParamConv,
    vednnConvolutionAlgorithm_t 	algo
) ;

vednnError_t vednnConvolutionForwardAddBias(
    const vednnTensorParam_t 		*pParamIn,
    const void 				*pDataIn,
    const vednnFilterParam_t		*pParamKernel,
    const void 				*pDataKernel,
    const vednnBiasParam_t 		*pParamBias,
    const void 				*pDataBias,
    const vednnTensorParam_t 		*pParamOut,
    void 				*pDataOut,
    const vednnConvolutionParam_t	*pParamConv,
    vednnConvolutionAlgorithm_t 	algo
) ;

vednnError_t vednnConvolutionBackwardData(
    const vednnTensorParam_t 		*pParamGradIn,
    const void 				*pDataGradIn,
    const vednnFilterParam_t		*pParamKernel,
    const void 				*pDataKernel,
    const vednnTensorParam_t 		*pParamGradOut,
    void 				*pDataGradOut,
    const vednnConvolutionParam_t	*pParamConv,
    vednnConvolutionAlgorithm_t 	algo
) ;

vednnError_t vednnConvolutionBackwardFilter(
    const vednnTensorParam_t 		*pParamIn,
    const void 				*pDataIn,
    const vednnTensorParam_t 		*pParamGradOut,
    const void 				*pDataGradOut,
    const vednnFilterParam_t		*pParamGradKernel,
    void 				*pDataGradKernel,
    const vednnConvolutionParam_t	*pParamConv,
    vednnConvolutionAlgorithm_t 	algo
) ;

vednnError_t vednnLinearForward(
    const unsigned long			inDim,
    const unsigned long			outDim,
    const unsigned long			nBatch,
    const void 				*pDataIn,
    const void 				*pDataWeight,
    void 				*pDataOut
) ;

vednnError_t vednnLinearBackwardData(
    const unsigned long			inDim,
    const unsigned long			outDim,
    const unsigned long			nBatch,
    const void 				*pDataGradOut,
    const void 				*pDataWeight,
    void 				*pData
) ;

vednnError_t vednnMaxPoolingForward(
    const vednnTensorParam_t 		*pParamIn,
    const void 				*pDataIn,
    const vednnTensorParam_t 		*pParamOut,
    void 				*pDataOut,
    const vednnPoolingParam_t		*pParamPool
) ;

vednnError_t vednnMaxPoolingBackward(
    const vednnTensorParam_t 		*pParamGradOut,
    const void 				*pDataGradOut,
    const vednnTensorParam_t 		*pParamOut,
    const void 				*pDataOut,
    const vednnTensorParam_t 		*pParamIn,
    const void 				*pDataIn,
    const vednnTensorParam_t 		*pParamGradIn,
    void 				*pDataGradIn,
    const vednnPoolingParam_t		*pParamPool
) ;

vednnError_t vednnActivationForward(
    const vednnActivationMode_t		mode,
    const void 				*pDataIn,
    void 				*pDataOut,
    const unsigned long			nElements
) ;

vednnError_t vednnActivationBackward(
    const vednnActivationMode_t		mode,
    const void 				*pDataGradOut,
    const void 				*pDataIn,
    void 				*pDataGradIn,
    const unsigned long			nElements
) ;

vednnError_t vednnSoftmaxForward(
    const vednnSoftmaxMode_t		mode,
    const void 				*pDataIn,
    void 				*pDataOut,
    const unsigned long			nBatch,
    const unsigned long			nClass
) ;

#ifdef __cplusplus
}
#endif

#endif
