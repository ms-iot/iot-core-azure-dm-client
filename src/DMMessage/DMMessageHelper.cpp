/*
Copyright 2017 Microsoft
Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "stdafx.h"
#include "CurrentVersion.h"

#include "Models\AllModels.h"

using namespace Microsoft::Devices::Management::Message;

#define MODEL_NODEF(A, B, C, D) 
#define MODEL_REQDEF(A, B, C, D) DMMessageKind C##::Tag::get() { return DMMessageKind::##A; }
#define MODEL_ALLDEF(A, B, C, D) MODEL_REQDEF(A, B, C, D) DMMessageKind D##::Tag::get() { return DMMessageKind::##A; }
#define MODEL_TAGONLY(A, B, C, D)
#include "Models\ModelsInfo.dat"
#undef MODEL_NODEF
#undef MODEL_REQDEF
#undef MODEL_ALLDEF
#undef MODEL_TAGONLY
