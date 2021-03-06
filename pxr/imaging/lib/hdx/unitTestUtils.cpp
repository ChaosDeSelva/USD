//
// Copyright 2017 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
//
#include "pxr/pxr.h"
#include "pxr/imaging/hdx/unitTestUtils.h"

#include "pxr/imaging/hd/rprimCollection.h"
#include "pxr/imaging/hdx/intersector.h"
#include "pxr/imaging/hdx/selectionTracker.h"

#include <boost/functional/hash.hpp>

PXR_NAMESPACE_OPEN_SCOPE

namespace HdxUnitTestUtils {

struct HitHash {
    // make a partial hash excluding elementId, ndcDepth, wsHitPoint,
    // allowing us to group hits to different elements of the same object
    // instance
    size_t operator()(HdxIntersector::Hit const& hit) const {
        size_t hash = 0;
        boost::hash_combine(hash, hit.delegateId.GetHash());
        boost::hash_combine(hash, hit.objectId.GetHash());
        boost::hash_combine(hash, hit.instancerId.GetHash());
        boost::hash_combine(hash, hit.instanceIndex);
        return hash;
    }
};

struct HitEq {
    bool operator()(HdxIntersector::Hit const& a, 
                    HdxIntersector::Hit const& b) const {
        return a.delegateId    == b.delegateId    &&
               a.objectId      == b.objectId      &&
               a.instancerId   == b.instancerId   &&
               a.instanceIndex == b.instanceIndex;
    }
};

Picker::Picker() : _selectionTracker(new HdxSelectionTracker()) {}

Picker::~Picker() {}

void
Picker::InitIntersector(HdRenderIndex* renderIndex)
{
    _intersector.reset(new HdxIntersector(renderIndex));
}

void
Picker::SetPickParams(PickParams const& pp)
{
    _pParams = pp;

    #if 1
    using namespace std;
    cout << "PickParams" << endl;
    cout << "width = " << _pParams.screenWidth << endl;
    cout << "height  = " << _pParams.screenHeight << endl;
    #endif
}

void
Picker::SetHighlightMode(HdxSelectionHighlightMode mode)
{
    _pParams.highlightMode = mode;
}

void
Picker::Pick(GfVec2i const& startPos,
              GfVec2i const& endPos)
{
    if (!_intersector)
        return;

    // for readability
    GfVec2i const& pickRadius               = _pParams.pickRadius;
    float const& width                      = _pParams.screenWidth;
    float const& height                     = _pParams.screenHeight;
    GfFrustum const& frustum                = _pParams.viewFrustum;
    GfMatrix4d const& viewMatrix            = _pParams.viewMatrix;
    HdxSelectionHighlightMode const& mode   = _pParams.highlightMode;

    int fwidth  = std::max(pickRadius[0], std::abs(startPos[0] - endPos[0]));
    int fheight = std::max(pickRadius[1], std::abs(startPos[1] - endPos[1]));

    _intersector->SetResolution(GfVec2i(fwidth, fheight));
    
    GfVec2d min(2*startPos[0]/width-1, 1-2*startPos[1]/height);
    GfVec2d max(2*(endPos[0]+1)/width-1, 1-2*(endPos[1]+1)/height);
    // scale window
    GfVec2d origin = frustum.GetWindow().GetMin();
    GfVec2d scale = frustum.GetWindow().GetMax() - frustum.GetWindow().GetMin();
    min = origin + GfCompMult(scale, 0.5 * (GfVec2d(1.0, 1.0) + min));
    max = origin + GfCompMult(scale, 0.5 * (GfVec2d(1.0, 1.0) + max));
    
    GfFrustum pickFrustum(frustum);
    pickFrustum.SetWindow(GfRange2d(min, max));

    HdxIntersector::Params iParams;
    iParams.hitMode          = HdxIntersector::HitFirst;
    iParams.projectionMatrix = pickFrustum.ComputeProjectionMatrix();
    iParams.viewMatrix       = viewMatrix;

    std::cout << "Pick " << startPos << " - " << endPos << "\n";

    HdxIntersector::Result result;
    _intersector->Query(iParams,
                        *_pParams.pickablesCol,
                        _pParams.engine, 
                        &result);

    HdxIntersector::HitSet hits;
    HdxSelectionSharedPtr selection(new HdxSelection);
    if (result.ResolveUnique(&hits)) {
        std::unordered_set<HdxIntersector::Hit, HitHash, HitEq> aggregatedHits;

        // Aggregate hits to the same object instance (see HitHash)
        TF_FOR_ALL(it, hits) {
            aggregatedHits.insert(*it);
            std::cout << "object: " << it->objectId << " "
                      << "instancer: " << it->instancerId << " "
                      << "instanceIndex: " << it->instanceIndex << " "
                      << "elementIndex: " << it->elementIndex << " "
                      << "hit: " << it->worldSpaceHitPoint << " "
                      << "ndcDepth: " << it->ndcDepth << "\n";

        }

        for(const auto& hit : aggregatedHits) {
            if (!hit.instancerId.IsEmpty()) {
                // XXX :this doesn't work for nested instancing.
                VtIntArray instanceIndex;
                instanceIndex.push_back(hit.instanceIndex);
                selection->AddInstance(mode, hit.objectId, instanceIndex);
                // we should use GetPathForInstanceIndex instead of it->objectId
                //SdfPath path = _delegate->GetPathForInstanceIndex(it->objectId, it->instanceIndex);
                // and also need to add some APIs to compute VtIntArray instanceIndex.
            } else {
                selection->AddRprim(mode, hit.objectId);
            }
        }
    }

    _selectionTracker->SetSelection(selection);
}

HdxSelectionTrackerSharedPtr
Picker::GetSelectionTracker() const
{
    return _selectionTracker;
}

HdxSelectionSharedPtr
Picker::GetSelection() const
{
    return _selectionTracker->GetSelectionMap();
}

//------------------------------------------------------------------------------
Marquee::Marquee()
{

}

Marquee::~Marquee()
{

}

void
Marquee::InitGLResources()
{
    glGenBuffers(1, &_vbo);
    _program = glCreateProgram();
    const char *sources[1];
    sources[0] =
        "#version 430                                         \n"
        "in vec2 position;                                    \n"
        "void main() {                                        \n"
        "  gl_Position = vec4(position.x, position.y, 0, 1);  \n"
        "}                                                    \n";

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, sources, NULL);
    glCompileShader(vShader);

    sources[0] =
        "#version 430                                         \n"
        "out vec4 outColor;                                   \n"
        "void main() {                                        \n"
        "  outColor = vec4(1);                                \n"
        "}                                                    \n";

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, sources, NULL);
    glCompileShader(fShader);

    glAttachShader(_program, vShader);
    glAttachShader(_program, fShader);

    glLinkProgram(_program);

    glDeleteShader(vShader);
    glDeleteShader(fShader);
}

void
Marquee::DestroyGLResources()
{
    glDeleteProgram(_program);
    glDeleteBuffers(1, &_vbo);
}

void
Marquee::Draw(float width, float height, 
              GfVec2f const& startPos, GfVec2f const& endPos)
{
    glDisable(GL_DEPTH_TEST);
    glUseProgram(_program);

    GfVec2f s(2*startPos[0]/width-1,
              1-2*startPos[1]/height);
    GfVec2f e(2*endPos[0]/width-1,
              1-2*endPos[1]/height);
    float pos[] = { s[0], s[1], e[0], s[1],
                    e[0], e[1], s[0], e[1],
                    s[0], s[1] };

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_LINE_STRIP, 0, 5);

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glUseProgram(0);
    glEnable(GL_DEPTH_TEST);
}

} // namespace HdxUnitTestHelper

PXR_NAMESPACE_CLOSE_SCOPE
