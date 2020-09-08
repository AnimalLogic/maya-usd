#include "pxr/base/tf/pyModule.h"

PXR_NAMESPACE_USING_DIRECTIVE

TF_WRAP_MODULE
{
    TF_WRAP(MayaTransformAPI);
    TF_WRAP(TransformCache);
    TF_WRAP(TransformEvaluator);
    TF_WRAP(TransformManipulator);
}
