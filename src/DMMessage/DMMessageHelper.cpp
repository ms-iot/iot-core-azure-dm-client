#include "stdafx.h"
#include "CurrentVersion.h"

#include "Models\AllModels.h"
#include "Models\ModelsInfo.dat"

using namespace Microsoft::Devices::Management::Message;

#define MODEL_NODEF(A, B, C, D) 

#define MODEL_REQDEF(A, B, C, D) DMMessageKind C##::Tag::get() { return DMMessageKind::##A; } \

#define MODEL_ALLDEF(A, B, C, D) MODEL_REQDEF(A, B, C, D) \
DMMessageKind D##::Tag::get() { return DMMessageKind::##A; } \

MODELS_INFO

#undef MODEL_NODEF
#undef MODEL_REQDEF
#undef MODEL_ALLDEF

