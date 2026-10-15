// build 14191

#pragma once

#include <cstddef>
#include <cstdint>

namespace cs2_dumper {
    namespace schemas {
        namespace libresourcesystem_so {
            enum class FuseVariableAccess_t : uint8_t {
                WRITABLE = 0x0,
                READ_ONLY = 0x1
            };
            enum class FuseVariableType_t : uint8_t {
                INVALID = 0x0,
                BOOL = 0x1,
                INT8 = 0x2,
                INT16 = 0x3,
                INT32 = 0x4,
                UINT8 = 0x5,
                UINT16 = 0x6,
                UINT32 = 0x7,
                FLOAT32 = 0x8
            };
            enum class NoiseStreamModifier_t : uint32_t {
                NOISE_STREAM_MODIFIER_NONE = 0x0,
                NOISE_STREAM_MODIFIER_LINES = 0x1,
                NOISE_STREAM_MODIFIER_CLUMPS = 0x2,
                NOISE_STREAM_MODIFIER_RINGS = 0x3
            };
            enum class NoiseStreamTurbulence_t : uint32_t {
                NOISE_STREAM_TURB_NONE = 0x0,
                NOISE_STREAM_TURB_HIGHLIGHT = 0x1,
                NOISE_STREAM_TURB_FEEDBACK = 0x2,
                NOISE_STREAM_TURB_LOOPY = 0x3,
                NOISE_STREAM_TURB_CONTRAST = 0x4,
                NOISE_STREAM_TURB_ALTERNATE = 0x5
            };
            enum class NoiseStreamType_t : uint32_t {
                NOISE_STREAM_TYPE_PERLIN = 0x0,
                NOISE_STREAM_TYPE_SIMPLEX = 0x1,
                NOISE_STREAM_TYPE_WORLEY = 0x2,
                NOISE_STREAM_TYPE_CURL = 0x3,
                NOISE_STREAM_TYPE_NONE = 0x4
            };
            namespace AABBWS_t {
                constexpr std::ptrdiff_t m_vMinBounds = 0x0; // VectorWS
                constexpr std::ptrdiff_t m_vMaxBounds = 0xC; // VectorWS
            }
            namespace AABB_t {
                constexpr std::ptrdiff_t m_vMinBounds = 0x0; // Vector
                constexpr std::ptrdiff_t m_vMaxBounds = 0xC; // Vector
            }
            namespace CFuseProgram {
                constexpr std::ptrdiff_t m_programBuffer = 0x0; // CUtlVector<uint8>
                constexpr std::ptrdiff_t m_variablesRead = 0x18; // CUtlVector<FuseVariableIndex_t>
                constexpr std::ptrdiff_t m_variablesWritten = 0x30; // CUtlVector<FuseVariableIndex_t>
                constexpr std::ptrdiff_t m_nMaxTempVarsUsed = 0x48; // int32
            }
            namespace CFuseSymbolTable {
                constexpr std::ptrdiff_t m_constants = 0x0; // CUtlVector<ConstantInfo_t>
                constexpr std::ptrdiff_t m_variables = 0x18; // CUtlVector<VariableInfo_t>
                constexpr std::ptrdiff_t m_functions = 0x30; // CUtlVector<FunctionInfo_t>
                constexpr std::ptrdiff_t m_constantMap = 0x48; // CUtlHashtable<CUtlStringToken,int32>
                constexpr std::ptrdiff_t m_variableMap = 0x68; // CUtlHashtable<CUtlStringToken,int32>
                constexpr std::ptrdiff_t m_functionMap = 0x88; // CUtlHashtable<CUtlStringToken,int32>
            }
            namespace ConstantInfo_t {
                constexpr std::ptrdiff_t m_name = 0x0; // CUtlString
                constexpr std::ptrdiff_t m_nameToken = 0x8; // CUtlStringToken
                constexpr std::ptrdiff_t m_flValue = 0xC; // float32
            }
            namespace FourQuaternions {
                constexpr std::ptrdiff_t x = 0x0; // fltx4
                constexpr std::ptrdiff_t y = 0x10; // fltx4
                constexpr std::ptrdiff_t z = 0x20; // fltx4
                constexpr std::ptrdiff_t w = 0x30; // fltx4
            }
            namespace FunctionInfo_t {
                constexpr std::ptrdiff_t m_name = 0x8; // CUtlString
                constexpr std::ptrdiff_t m_nameToken = 0x10; // CUtlStringToken
                constexpr std::ptrdiff_t m_nParamCount = 0x14; // int32
                constexpr std::ptrdiff_t m_nIndex = 0x18; // FuseFunctionIndex_t
                constexpr std::ptrdiff_t m_bIsPure = 0x1A; // bool
            }
            namespace FuseFunctionIndex_t {
                constexpr std::ptrdiff_t m_Value = 0x0; // uint16
            }
            namespace FuseVariableIndex_t {
                constexpr std::ptrdiff_t m_Value = 0x0; // uint16
            }
            namespace InfoForResourceTypeCAnimData {
            }
            namespace InfoForResourceTypeCAnimationGroup {
            }
            namespace InfoForResourceTypeCCSGOEconItem {
            }
            namespace InfoForResourceTypeCChoreoSceneResource {
            }
            namespace InfoForResourceTypeCCompositeMaterialKit {
            }
            namespace InfoForResourceTypeCDOTANovelsList {
            }
            namespace InfoForResourceTypeCDOTAPatchNotesList {
            }
            namespace InfoForResourceTypeCDotaItemDefinitionResource {
            }
            namespace InfoForResourceTypeCEntityLump {
            }
            namespace InfoForResourceTypeCGcExportableExternalData {
            }
            namespace InfoForResourceTypeCJavaScriptResource {
            }
            namespace InfoForResourceTypeCModel {
            }
            namespace InfoForResourceTypeCMorphSetData {
            }
            namespace InfoForResourceTypeCNmClip {
            }
            namespace InfoForResourceTypeCNmGraphDefinition {
            }
            namespace InfoForResourceTypeCNmSkeleton {
            }
            namespace InfoForResourceTypeCPanoramaDynamicImages {
            }
            namespace InfoForResourceTypeCPanoramaLayout {
            }
            namespace InfoForResourceTypeCPanoramaStyle {
            }
            namespace InfoForResourceTypeCPhysAggregateData {
            }
            namespace InfoForResourceTypeCPostProcessingResource {
            }
            namespace InfoForResourceTypeCRenderMesh {
            }
            namespace InfoForResourceTypeCResponseRulesList {
            }
            namespace InfoForResourceTypeCSequenceGroupData {
            }
            namespace InfoForResourceTypeCSurfaceGraph {
            }
            namespace InfoForResourceTypeCTestResourceData {
            }
            namespace InfoForResourceTypeCTextureBase {
            }
            namespace InfoForResourceTypeCTypeScriptResource {
            }
            namespace InfoForResourceTypeCVDSPResource {
            }
            namespace InfoForResourceTypeCVDataItemDefs {
            }
            namespace InfoForResourceTypeCVDataResource {
            }
            namespace InfoForResourceTypeCVMixListResource {
            }
            namespace InfoForResourceTypeCVPhysXSurfacePropertiesList {
            }
            namespace InfoForResourceTypeCVSoundEventScriptList {
            }
            namespace InfoForResourceTypeCVSoundStackScriptList {
            }
            namespace InfoForResourceTypeCVoiceContainerBase {
            }
            namespace InfoForResourceTypeCVoxelVisibility {
            }
            namespace InfoForResourceTypeCWorldNode {
            }
            namespace InfoForResourceTypeIAnimGraphModelBinding {
            }
            namespace InfoForResourceTypeIMaterial2 {
            }
            namespace InfoForResourceTypeIParticleSnapshot {
            }
            namespace InfoForResourceTypeIParticleSystemDefinition {
            }
            namespace InfoForResourceTypeIPulseGraphDef {
            }
            namespace InfoForResourceTypeISmartProp {
            }
            namespace InfoForResourceTypeIVectorGraphic {
            }
            namespace InfoForResourceTypeManifestTestResource_t {
            }
            namespace InfoForResourceTypeProceduralTestResource_t {
            }
            namespace InfoForResourceTypeWorld_t {
            }
            namespace ManifestTestResource_t {
                constexpr std::ptrdiff_t m_name = 0x0; // CUtlString
                constexpr std::ptrdiff_t m_child = 0x8; // CStrongHandle<InfoForResourceTypeManifestTestResource_t>
            }
            namespace NoiseOscillatorDef_t {
                constexpr std::ptrdiff_t m_flPhase = 0x0; // float32
                constexpr std::ptrdiff_t m_flFrequency = 0x4; // float32
                constexpr std::ptrdiff_t m_flAmplitude = 0x8; // float32
            }
            namespace NoiseStreamDef_t {
                constexpr std::ptrdiff_t m_nType = 0x0; // NoiseStreamType_t
                constexpr std::ptrdiff_t m_nModifier = 0x4; // NoiseStreamModifier_t
                constexpr std::ptrdiff_t m_nTurbulence = 0x8; // NoiseStreamTurbulence_t
                constexpr std::ptrdiff_t m_flOutputMin = 0xC; // float32
                constexpr std::ptrdiff_t m_flOutputMax = 0x10; // float32
                constexpr std::ptrdiff_t m_flScale = 0x14; // float32
                constexpr std::ptrdiff_t m_vOffsetRate = 0x18; // Vector
                constexpr std::ptrdiff_t m_flOffset = 0x24; // float32
                constexpr std::ptrdiff_t m_nOctaves = 0x28; // int32
                constexpr std::ptrdiff_t m_flTurbulenceScale = 0x2C; // float32
                constexpr std::ptrdiff_t m_flTurbulenceMix = 0x30; // float32
                constexpr std::ptrdiff_t m_Oscillators = 0x38; // CUtlVector<NoiseOscillatorDef_t>
            }
            namespace PackedAABB_t {
                constexpr std::ptrdiff_t m_nPackedMin = 0x0; // uint32
                constexpr std::ptrdiff_t m_nPackedMax = 0x4; // uint32
            }
            namespace VariableInfo_t {
                constexpr std::ptrdiff_t m_name = 0x0; // CUtlString
                constexpr std::ptrdiff_t m_nameToken = 0x8; // CUtlStringToken
                constexpr std::ptrdiff_t m_nIndex = 0xC; // FuseVariableIndex_t
                constexpr std::ptrdiff_t m_nNumComponents = 0xE; // uint8
                constexpr std::ptrdiff_t m_eVarType = 0xF; // FuseVariableType_t
                constexpr std::ptrdiff_t m_eAccess = 0x10; // FuseVariableAccess_t
            }
        }
    }
}
