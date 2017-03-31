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
#pragma once

#include "DMMessageKind.h"
#include "Models\AllModels.h"

using namespace Platform;
using namespace Platform::Metadata;
using namespace Windows::Data::Json;
using namespace Microsoft::Devices::Management::Message;

namespace Microsoft { namespace Devices { namespace Management { namespace Message
{
    private class DMMessageDeserializer
    {
    public:
        typedef IDataPayload^ (DMRequestDeserialize)(Blob^ bytes);
        typedef IDataPayload^ (DMResponseDeserialize)(Blob^ bytes);
        typedef std::pair<DMRequestDeserialize*, DMResponseDeserialize*> DMSerializationPair;
        typedef std::map<DMMessageKind, DMSerializationPair> DMDeserializer;

        DMDeserializer Deserializer = {
#define MODEL_NODEF(A, B, C, D) { DMMessageKind::##A, { (DMRequestDeserialize*)##C##::Deserialize, (DMResponseDeserialize*)##D##::Deserialize } },
#define MODEL_REQDEF(A, B, C, D) MODEL_NODEF(A, B, C, D)
#define MODEL_ALLDEF(A, B, C, D) MODEL_NODEF(A, B, C, D)
#define MODEL_TAGONLY(A, B, C, D) MODEL_NODEF(A, B, C, D)
#include "Models\ModelsInfo.dat"
#undef MODEL_NODEF
#undef MODEL_REQDEF
#undef MODEL_ALLDEF
#undef MODEL_TAGONLY
        };
    };
}}}}
