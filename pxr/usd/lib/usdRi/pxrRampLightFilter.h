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
#ifndef USDRI_GENERATED_PXRRAMPLIGHTFILTER_H
#define USDRI_GENERATED_PXRRAMPLIGHTFILTER_H

/// \file usdRi/pxrRampLightFilter.h

#include "pxr/pxr.h"
#include "pxr/usd/usdRi/api.h"
#include "pxr/usd/usdLux/lightFilter.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdRi/tokens.h"

#include "pxr/usd/usdRi/splineAPI.h"


#include "pxr/base/vt/value.h"

#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4d.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// PXRRAMPLIGHTFILTER                                                         //
// -------------------------------------------------------------------------- //

/// \class UsdRiPxrRampLightFilter
///
/// A ramp to modulate how a light falls off with distance.
///
/// For any described attribute \em Fallback \em Value or \em Allowed \em Values below
/// that are text/tokens, the actual token is published and defined in \ref UsdRiTokens.
/// So to set an attribute to the value "rightHanded", use UsdRiTokens->rightHanded
/// as the value.
///
class UsdRiPxrRampLightFilter : public UsdLuxLightFilter
{
public:
    /// Compile-time constant indicating whether or not this class corresponds
    /// to a concrete instantiable prim type in scene description.  If this is
    /// true, GetStaticPrimDefinition() will return a valid prim definition with
    /// a non-empty typeName.
    static const bool IsConcrete = true;

    /// Compile-time constant indicating whether or not this class inherits from
    /// UsdTyped. Types which inherit from UsdTyped can impart a typename on a
    /// UsdPrim.
    static const bool IsTyped = true;

    /// Construct a UsdRiPxrRampLightFilter on UsdPrim \p prim .
    /// Equivalent to UsdRiPxrRampLightFilter::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit UsdRiPxrRampLightFilter(const UsdPrim& prim=UsdPrim())
        : UsdLuxLightFilter(prim)
    {
    }

    /// Construct a UsdRiPxrRampLightFilter on the prim held by \p schemaObj .
    /// Should be preferred over UsdRiPxrRampLightFilter(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit UsdRiPxrRampLightFilter(const UsdSchemaBase& schemaObj)
        : UsdLuxLightFilter(schemaObj)
    {
    }

    /// Destructor.
    USDRI_API
    virtual ~UsdRiPxrRampLightFilter();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    USDRI_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a UsdRiPxrRampLightFilter holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// UsdRiPxrRampLightFilter(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    USDRI_API
    static UsdRiPxrRampLightFilter
    Get(const UsdStagePtr &stage, const SdfPath &path);

    /// Attempt to ensure a \a UsdPrim adhering to this schema at \p path
    /// is defined (according to UsdPrim::IsDefined()) on this stage.
    ///
    /// If a prim adhering to this schema at \p path is already defined on this
    /// stage, return that prim.  Otherwise author an \a SdfPrimSpec with
    /// \a specifier == \a SdfSpecifierDef and this schema's prim type name for
    /// the prim at \p path at the current EditTarget.  Author \a SdfPrimSpec s
    /// with \p specifier == \a SdfSpecifierDef and empty typeName at the
    /// current EditTarget for any nonexistent, or existing but not \a Defined
    /// ancestors.
    ///
    /// The given \a path must be an absolute prim path that does not contain
    /// any variant selections.
    ///
    /// If it is impossible to author any of the necessary PrimSpecs, (for
    /// example, in case \a path cannot map to the current UsdEditTarget's
    /// namespace) issue an error and return an invalid \a UsdPrim.
    ///
    /// Note that this method may return a defined prim whose typeName does not
    /// specify this schema class, in case a stronger typeName opinion overrides
    /// the opinion at the current EditTarget.
    ///
    USDRI_API
    static UsdRiPxrRampLightFilter
    Define(const UsdStagePtr &stage, const SdfPath &path);

private:
    // needs to invoke _GetStaticTfType.
    friend class UsdSchemaRegistry;
    USDRI_API
    static const TfType &_GetStaticTfType();

    static bool _IsTypedSchema();

    // override SchemaBase virtuals.
    USDRI_API
    virtual const TfType &_GetTfType() const;

public:
    // --------------------------------------------------------------------- //
    // RAMPMODE 
    // --------------------------------------------------------------------- //
    /// Specifies the direction in which the ramp is applied
    ///
    /// \n  C++ Type: TfToken
    /// \n  Usd Type: SdfValueTypeNames->Token
    /// \n  Variability: SdfVariabilityVarying
    /// \n  Fallback Value: distanceToLight
    /// \n  \ref UsdRiTokens "Allowed Values": [distanceToLight, linear, spherical, radial]
    USDRI_API
    UsdAttribute GetRampModeAttr() const;

    /// See GetRampModeAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDRI_API
    UsdAttribute CreateRampModeAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // FALLOFFRAMPBEGINDISTANCE 
    // --------------------------------------------------------------------- //
    /// 
    ///
    /// \n  C++ Type: float
    /// \n  Usd Type: SdfValueTypeNames->Float
    /// \n  Variability: SdfVariabilityVarying
    /// \n  Fallback Value: 0.0
    USDRI_API
    UsdAttribute GetFalloffRampBeginDistanceAttr() const;

    /// See GetFalloffRampBeginDistanceAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDRI_API
    UsdAttribute CreateFalloffRampBeginDistanceAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // FALLOFFRAMPENDDISTANCE 
    // --------------------------------------------------------------------- //
    /// 
    ///
    /// \n  C++ Type: float
    /// \n  Usd Type: SdfValueTypeNames->Float
    /// \n  Variability: SdfVariabilityVarying
    /// \n  Fallback Value: 10.0
    USDRI_API
    UsdAttribute GetFalloffRampEndDistanceAttr() const;

    /// See GetFalloffRampEndDistanceAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDRI_API
    UsdAttribute CreateFalloffRampEndDistanceAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // ===================================================================== //
    // Feel free to add custom code below this line, it will be preserved by 
    // the code generator. 
    //
    // Just remember to: 
    //  - Close the class declaration with }; 
    //  - Close the namespace with PXR_NAMESPACE_CLOSE_SCOPE
    //  - Close the include guard with #endif
    // ===================================================================== //
    // --(BEGIN CUSTOM CODE)--

    /// Return the UsdRiSplineAPI interface used for examining and modifying
    /// the falloff ramp.
    USDRI_API
    UsdRiSplineAPI GetFalloffRampAPI() const;

    /// Return the UsdRiSplineAPI interface used for examining and modifying
    /// the color ramp.
    USDRI_API
    UsdRiSplineAPI GetColorRampAPI() const;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
