#include "stdafx.h"
#include "CurrentVersion.h"

#include "Models\AllModels.h"

using namespace Microsoft::Devices::Management::Message;

#define MODEL_NODEF(A, B, C, D) 

#define MODEL_REQDEF(A, B, C, D) DMMessageKind C##::Tag::get() { return DMMessageKind::##A; } \

#define MODEL_ALLDEF(A, B, C, D) MODEL_REQDEF(A, B, C, D) \
DMMessageKind D##::Tag::get() { return DMMessageKind::##A; } \

#include "Models\ModelsInfo.dat"

#undef MODEL_NODEF
#undef MODEL_REQDEF
#undef MODEL_ALLDEF

