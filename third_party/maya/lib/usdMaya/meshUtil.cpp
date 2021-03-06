//
// Copyright 2016 Pixar
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
#include "pxr/pxr.h"
#include "usdMaya/meshUtil.h"

#include "pxr/base/gf/vec3f.h"
#include "pxr/base/tf/staticTokens.h"
#include "pxr/base/tf/token.h"
#include "pxr/base/vt/array.h"
#include "pxr/usd/usdGeom/mesh.h"

#include <maya/MFloatVector.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MItMeshFaceVertex.h>
#include <maya/MPlug.h>
#include <maya/MStatus.h>

PXR_NAMESPACE_OPEN_SCOPE


TF_DEFINE_PUBLIC_TOKENS(PxrUsdMayaMeshColorSetTokens,
    PXRUSDMAYA_MESH_COLOR_SET_TOKENS);

// These tokens are supported Maya attributes used for Mesh surfaces
TF_DEFINE_PRIVATE_TOKENS(
    _meshTokens,

    // we capitalize this because it doesn't correspond to an actual attribute
    (USD_EmitNormals)

    // This is a value for face varying interpolate boundary from OpenSubdiv 2
    // that we translate to face varying linear interpolation for OpenSubdiv 3.
    (alwaysSharp)
);

// These tokens are supported Maya attributes used for SDiv surfaces
TF_DEFINE_PRIVATE_TOKENS(
    _subdivTokens,

    (USD_subdivisionScheme)
    (USD_interpolateBoundary)
    (USD_faceVaryingLinearInterpolation)

    // This token is deprecated as it is from OpenSubdiv 2 and the USD
    // schema now conforms to OpenSubdiv 3, but we continue to look for it
    // and translate to the equivalent new value for backwards compatibility.
    (USD_faceVaryingInterpolateBoundary)
);


// This can be customized for specific pipelines.
bool
PxrUsdMayaMeshUtil::getEmitNormals(
        const MFnMesh& mesh,
        const TfToken& subdivScheme)
{
    MPlug plug = mesh.findPlug(MString(_meshTokens->USD_EmitNormals.GetText()));
    if (!plug.isNull()) {
        return plug.asBool();
    }

    // We only emit normals by default if it wasn't explicitly set (above) and
    // the subdiv scheme is "polygonal".  note, we currently only ever call this
    // function with subdivScheme == none...
    return subdivScheme == UsdGeomTokens->none;
}

TfToken
PxrUsdMayaMeshUtil::setEmitNormals(
        const UsdGeomMesh& primSchema,
        MFnMesh& meshFn,
        TfToken defaultValue)
{
    MStatus status;

    TfToken normalInterp = defaultValue;
    //primSchema.GetSubdivisionSchemeAttr().Get(&subdScheme, UsdTimeCode::Default());

    //primSchema.GetNormalsAttr().Set(meshNormals, UsdTimeCode::Default());
    normalInterp = primSchema.GetNormalsInterpolation();

    // If normals are not present, don't create the attribute
    if (normalInterp == UsdGeomTokens->faceVarying) {
        MFnNumericAttribute nAttr;
        MObject attr = nAttr.create(_meshTokens->USD_EmitNormals.GetText(),
                                 "", MFnNumericData::kBoolean, 1, &status);
        if (status == MS::kSuccess) {
            meshFn.addAttribute(attr);
        }
    }
    return normalInterp;
}

bool
PxrUsdMayaMeshUtil::GetMeshNormals(
        const MFnMesh& mesh,
        VtArray<GfVec3f>* normalsArray,
        TfToken* interpolation)
{
    MStatus status;

    // Sanity check first to make sure we can get this mesh's normals.
    int numNormals = mesh.numNormals(&status);
    if (status != MS::kSuccess || numNormals == 0) {
        return false;
    }

    // Using itFV.getNormal() does not always give us the right answer, so
    // instead we have to use itFV.normalId() and use that to index into the
    // normals.
    MFloatVectorArray mayaNormals;
    status = mesh.getNormals(mayaNormals);
    if (status != MS::kSuccess) {
        return false;
    }

    const unsigned int numFaceVertices = mesh.numFaceVertices(&status);
    if (status != MS::kSuccess) {
        return false;
    }

    normalsArray->resize(numFaceVertices);
    *interpolation = UsdGeomTokens->faceVarying;

    MItMeshFaceVertex itFV(mesh.object());
    unsigned int fvi = 0;
    for (itFV.reset(); !itFV.isDone(); itFV.next(), ++fvi) {
        int normalId = itFV.normalId();
        if (normalId < 0 ||
                static_cast<size_t>(normalId) >= mayaNormals.length()) {
            return false;
        }

        MFloatVector normal = mayaNormals[normalId];
        (*normalsArray)[fvi][0] = normal[0];
        (*normalsArray)[fvi][1] = normal[1];
        (*normalsArray)[fvi][2] = normal[2];
    }

    return true;
}

// This can be customized for specific pipelines.
// We first look for the USD string attribute, and if not present we look for
// the RenderMan for Maya int attribute.
// XXX Maybe we should come up with a OSD centric nomenclature ??
TfToken
PxrUsdMayaMeshUtil::getSubdivScheme(
        const MFnMesh& mesh,
        const TfToken& defaultValue)
{
    TfToken schemeToken = defaultValue;

    MPlug plug = mesh.findPlug(MString(_subdivTokens->USD_subdivisionScheme.GetText()));
    if (!plug.isNull()) {
        schemeToken = TfToken(plug.asString().asChar());
    } else {
        plug = mesh.findPlug(MString("rman__torattr___subdivScheme"));
        if (!plug.isNull()) {
            switch (plug.asInt()) {
                case 0:
                    schemeToken = UsdGeomTokens->catmullClark;
                    break;
                case 1:
                    schemeToken = UsdGeomTokens->loop;
                    break;
                default:
                    break;
            }
        }
    }

    if (schemeToken.IsEmpty()) {
        return defaultValue;
    } else if (schemeToken != UsdGeomTokens->none &&
               schemeToken != UsdGeomTokens->catmullClark &&
               schemeToken != UsdGeomTokens->loop &&
               schemeToken != UsdGeomTokens->bilinear) {
        MGlobal::displayError("Unsupported subdivision scheme: " +
            MString(schemeToken.GetText()) + " on mesh: " +
            MString(mesh.fullPathName()) + ". Defaulting to: " +
            MString(defaultValue.GetText()));
        return defaultValue;
    }

    return schemeToken;
}

// This can be customized for specific pipelines.
// We first look for the USD string attribute, and if not present we look for
// the RenderMan for Maya int attribute.
// XXX Maybe we should come up with a OSD centric nomenclature ??
TfToken PxrUsdMayaMeshUtil::getSubdivInterpBoundary(
        const MFnMesh& mesh,
        const TfToken& defaultValue)
{
    TfToken interpBoundaryToken = defaultValue;

    MPlug plug = mesh.findPlug(MString(_subdivTokens->USD_interpolateBoundary.GetText()));
    if (!plug.isNull()) {
        interpBoundaryToken = TfToken(plug.asString().asChar());
    } else {
        plug = mesh.findPlug(MString("rman__torattr___subdivInterp"));
        if (!plug.isNull()) {
            switch (plug.asInt()) {
                case 0:
                    interpBoundaryToken = UsdGeomTokens->none;
                    break;
                case 1:
                    interpBoundaryToken = UsdGeomTokens->edgeAndCorner;
                    break;
                case 2:
                    interpBoundaryToken = UsdGeomTokens->edgeOnly;
                    break;
                default:
                    break;
            }
        }
    }

    if (interpBoundaryToken.IsEmpty()) {
        return defaultValue;
    } else if (interpBoundaryToken != UsdGeomTokens->none &&
               interpBoundaryToken != UsdGeomTokens->edgeAndCorner &&
               interpBoundaryToken != UsdGeomTokens->edgeOnly) {
        MGlobal::displayError("Unsupported interpolate boundary setting: " +
            MString(interpBoundaryToken.GetText()) + " on mesh: " +
            MString(mesh.fullPathName()) + ". Defaulting to: " +
            MString(defaultValue.GetText()));
        return defaultValue;
    }

    return interpBoundaryToken;
}

// XXX: Note that this function is not exposed publicly since the USD schema
// has been updated to conform to OpenSubdiv 3. We still look for this attribute
// on Maya nodes specifying this value from OpenSubdiv 2, but we translate the
// value to OpenSubdiv 3. This is to support legacy assets authored against
// OpenSubdiv 2.
static
TfToken
_getSubdivFVInterpBoundary(const MFnMesh& mesh)
{
    TfToken sdFVInterpBound;

    MPlug plug = mesh.findPlug(MString(_subdivTokens->USD_faceVaryingInterpolateBoundary.GetText()));
    if (!plug.isNull()) {
        sdFVInterpBound = TfToken(plug.asString().asChar());

        // Translate OSD2 values to OSD3.
        if (sdFVInterpBound == UsdGeomTokens->bilinear) {
            sdFVInterpBound = UsdGeomTokens->all;
        } else if (sdFVInterpBound == UsdGeomTokens->edgeAndCorner) {
            sdFVInterpBound = UsdGeomTokens->cornersPlus1;
        } else if (sdFVInterpBound == _meshTokens->alwaysSharp) {
            sdFVInterpBound = UsdGeomTokens->boundaries;
        } else if (sdFVInterpBound == UsdGeomTokens->edgeOnly) {
            sdFVInterpBound = UsdGeomTokens->none;
        }
    } else {
        plug = mesh.findPlug(MString("rman__torattr___subdivFacevaryingInterp"));
        if (!plug.isNull()) {
            switch(plug.asInt()) {
                case 0:
                    sdFVInterpBound = UsdGeomTokens->all;
                    break;
                case 1:
                    sdFVInterpBound = UsdGeomTokens->cornersPlus1;
                    break;
                case 2:
                    sdFVInterpBound = UsdGeomTokens->none;
                    break;
                case 3:
                    sdFVInterpBound = UsdGeomTokens->boundaries;
                    break;
                default:
                    break;
            }
        }
    }

    return sdFVInterpBound;
}

TfToken PxrUsdMayaMeshUtil::getSubdivFVLinearInterpolation(const MFnMesh& mesh)
{
    TfToken sdFVLinearInterpolation;
    MPlug plug = mesh.findPlug(MString(_subdivTokens->USD_faceVaryingLinearInterpolation.GetText()));
    if (!plug.isNull()) {
        sdFVLinearInterpolation = TfToken(plug.asString().asChar());
    } else {
        // If the OpenSubdiv 3-style face varying linear interpolation value
        // wasn't specified, fall back to the old OpenSubdiv 2-style face
        // varying interpolate boundary value if we have that.
        sdFVLinearInterpolation = _getSubdivFVInterpBoundary(mesh);
    }

    if (!sdFVLinearInterpolation.IsEmpty() &&
            sdFVLinearInterpolation != UsdGeomTokens->all &&
            sdFVLinearInterpolation != UsdGeomTokens->none &&
            sdFVLinearInterpolation != UsdGeomTokens->boundaries &&
            sdFVLinearInterpolation != UsdGeomTokens->cornersOnly &&
            sdFVLinearInterpolation != UsdGeomTokens->cornersPlus1 &&
            sdFVLinearInterpolation != UsdGeomTokens->cornersPlus2) {
        MGlobal::displayError("Unsupported Face Varying Linear Interpolation Attribute: " +
            MString(sdFVLinearInterpolation.GetText()) + " on mesh: " + MString(mesh.fullPathName()));
        sdFVLinearInterpolation = TfToken();
    }

    return sdFVLinearInterpolation;
}

TfToken PxrUsdMayaMeshUtil::setSubdivScheme(const UsdGeomMesh &primSchema, MFnMesh &meshFn, TfToken defaultValue)
{
    MStatus status;

    // Determine if PolyMesh or SubdivMesh
    TfToken subdScheme;
    primSchema.GetSubdivisionSchemeAttr().Get(&subdScheme, UsdTimeCode::Default());

    // If retrieved scheme is default, don't create the attribute
    if (subdScheme != defaultValue) {
        MFnTypedAttribute stringAttr;
        MFnStringData stringData;
        MObject stringVal = stringData.create(subdScheme.GetText());
        MObject attr = stringAttr.create(_subdivTokens->USD_subdivisionScheme.GetText(),
                                         "", MFnData::kString, stringVal, &status);
        if (status == MS::kSuccess) {
            meshFn.addAttribute(attr);
        }
    }
    return subdScheme;
}

TfToken PxrUsdMayaMeshUtil::setSubdivInterpBoundary(const UsdGeomMesh &primSchema, MFnMesh &meshFn, TfToken defaultValue)
{
    MStatus status;

    TfToken interpBoundary;
    primSchema.GetInterpolateBoundaryAttr().Get(&interpBoundary, UsdTimeCode::Default());

    // XXXX THIS IS FOR OPENSUBDIV IN MAYA ?
    if (interpBoundary != UsdGeomTokens->none) {
        MPlug boundRulePlug = meshFn.findPlug("boundaryRule", &status);
        if (status == MS::kSuccess) {
            int value=0;
            if(interpBoundary == UsdGeomTokens->edgeAndCorner) value=1;
            else if(interpBoundary == UsdGeomTokens->edgeOnly) value=2;
            boundRulePlug.setValue(value);
        }
    }

    if (interpBoundary != defaultValue) {
        MFnTypedAttribute stringAttr;
        MFnStringData stringData;
        MObject stringVal = stringData.create(interpBoundary.GetText());
        MObject attr = stringAttr.create(_subdivTokens->USD_interpolateBoundary.GetText(),
                         "", MFnData::kString, stringVal, &status);
        if (status == MS::kSuccess) {
            meshFn.addAttribute(attr);
        }
    }
    return interpBoundary;
}

TfToken PxrUsdMayaMeshUtil::setSubdivFVLinearInterpolation(const UsdGeomMesh &primSchema, MFnMesh &meshFn)
{
    MStatus status;

    TfToken fvLinearInterpolation;
    auto fvLinearInterpolationAttr = primSchema.GetFaceVaryingLinearInterpolationAttr();
    fvLinearInterpolationAttr.Get(&fvLinearInterpolation);

    if (fvLinearInterpolation != UsdGeomTokens->cornersPlus1) {
        MFnTypedAttribute stringAttr;
        MFnStringData stringData;
        MObject stringVal = stringData.create(fvLinearInterpolation.GetText());
        MObject attr = stringAttr.create(_subdivTokens->USD_faceVaryingInterpolateBoundary.GetText(),
                                         "", MFnData::kString, stringVal, &status);
        if (status == MS::kSuccess) {
            meshFn.addAttribute(attr);
        }
    }

    return fvLinearInterpolation;
}

PXR_NAMESPACE_CLOSE_SCOPE

