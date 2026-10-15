// build 14191

namespace CS2Dumper.Schemas {
    public static class LibresourcesystemSo {
        public enum FuseVariableAccess_t : byte {
            WRITABLE = 0x0,
            READ_ONLY = 0x1
        }
        public enum FuseVariableType_t : byte {
            INVALID = 0x0,
            BOOL = 0x1,
            INT8 = 0x2,
            INT16 = 0x3,
            INT32 = 0x4,
            UINT8 = 0x5,
            UINT16 = 0x6,
            UINT32 = 0x7,
            FLOAT32 = 0x8
        }
        public enum NoiseStreamModifier_t : uint {
            NOISE_STREAM_MODIFIER_NONE = 0x0,
            NOISE_STREAM_MODIFIER_LINES = 0x1,
            NOISE_STREAM_MODIFIER_CLUMPS = 0x2,
            NOISE_STREAM_MODIFIER_RINGS = 0x3
        }
        public enum NoiseStreamTurbulence_t : uint {
            NOISE_STREAM_TURB_NONE = 0x0,
            NOISE_STREAM_TURB_HIGHLIGHT = 0x1,
            NOISE_STREAM_TURB_FEEDBACK = 0x2,
            NOISE_STREAM_TURB_LOOPY = 0x3,
            NOISE_STREAM_TURB_CONTRAST = 0x4,
            NOISE_STREAM_TURB_ALTERNATE = 0x5
        }
        public enum NoiseStreamType_t : uint {
            NOISE_STREAM_TYPE_PERLIN = 0x0,
            NOISE_STREAM_TYPE_SIMPLEX = 0x1,
            NOISE_STREAM_TYPE_WORLEY = 0x2,
            NOISE_STREAM_TYPE_CURL = 0x3,
            NOISE_STREAM_TYPE_NONE = 0x4
        }
        public static class AABBWS_t {
            public const nint m_vMinBounds = 0x0; // VectorWS
            public const nint m_vMaxBounds = 0xC; // VectorWS
        }
        public static class AABB_t {
            public const nint m_vMinBounds = 0x0; // Vector
            public const nint m_vMaxBounds = 0xC; // Vector
        }
        public static class CFuseProgram {
            public const nint m_programBuffer = 0x0; // CUtlVector<uint8>
            public const nint m_variablesRead = 0x18; // CUtlVector<FuseVariableIndex_t>
            public const nint m_variablesWritten = 0x30; // CUtlVector<FuseVariableIndex_t>
            public const nint m_nMaxTempVarsUsed = 0x48; // int32
        }
        public static class CFuseSymbolTable {
            public const nint m_constants = 0x0; // CUtlVector<ConstantInfo_t>
            public const nint m_variables = 0x18; // CUtlVector<VariableInfo_t>
            public const nint m_functions = 0x30; // CUtlVector<FunctionInfo_t>
            public const nint m_constantMap = 0x48; // CUtlHashtable<CUtlStringToken,int32>
            public const nint m_variableMap = 0x68; // CUtlHashtable<CUtlStringToken,int32>
            public const nint m_functionMap = 0x88; // CUtlHashtable<CUtlStringToken,int32>
        }
        public static class ConstantInfo_t {
            public const nint m_name = 0x0; // CUtlString
            public const nint m_nameToken = 0x8; // CUtlStringToken
            public const nint m_flValue = 0xC; // float32
        }
        public static class FourQuaternions {
            public const nint x = 0x0; // fltx4
            public const nint y = 0x10; // fltx4
            public const nint z = 0x20; // fltx4
            public const nint w = 0x30; // fltx4
        }
        public static class FunctionInfo_t {
            public const nint m_name = 0x8; // CUtlString
            public const nint m_nameToken = 0x10; // CUtlStringToken
            public const nint m_nParamCount = 0x14; // int32
            public const nint m_nIndex = 0x18; // FuseFunctionIndex_t
            public const nint m_bIsPure = 0x1A; // bool
        }
        public static class FuseFunctionIndex_t {
            public const nint m_Value = 0x0; // uint16
        }
        public static class FuseVariableIndex_t {
            public const nint m_Value = 0x0; // uint16
        }
        public static class InfoForResourceTypeCAnimData {
        }
        public static class InfoForResourceTypeCAnimationGroup {
        }
        public static class InfoForResourceTypeCCSGOEconItem {
        }
        public static class InfoForResourceTypeCChoreoSceneResource {
        }
        public static class InfoForResourceTypeCCompositeMaterialKit {
        }
        public static class InfoForResourceTypeCDOTANovelsList {
        }
        public static class InfoForResourceTypeCDOTAPatchNotesList {
        }
        public static class InfoForResourceTypeCDotaItemDefinitionResource {
        }
        public static class InfoForResourceTypeCEntityLump {
        }
        public static class InfoForResourceTypeCGcExportableExternalData {
        }
        public static class InfoForResourceTypeCJavaScriptResource {
        }
        public static class InfoForResourceTypeCModel {
        }
        public static class InfoForResourceTypeCMorphSetData {
        }
        public static class InfoForResourceTypeCNmClip {
        }
        public static class InfoForResourceTypeCNmGraphDefinition {
        }
        public static class InfoForResourceTypeCNmSkeleton {
        }
        public static class InfoForResourceTypeCPanoramaDynamicImages {
        }
        public static class InfoForResourceTypeCPanoramaLayout {
        }
        public static class InfoForResourceTypeCPanoramaStyle {
        }
        public static class InfoForResourceTypeCPhysAggregateData {
        }
        public static class InfoForResourceTypeCPostProcessingResource {
        }
        public static class InfoForResourceTypeCRenderMesh {
        }
        public static class InfoForResourceTypeCResponseRulesList {
        }
        public static class InfoForResourceTypeCSequenceGroupData {
        }
        public static class InfoForResourceTypeCSurfaceGraph {
        }
        public static class InfoForResourceTypeCTestResourceData {
        }
        public static class InfoForResourceTypeCTextureBase {
        }
        public static class InfoForResourceTypeCTypeScriptResource {
        }
        public static class InfoForResourceTypeCVDSPResource {
        }
        public static class InfoForResourceTypeCVDataItemDefs {
        }
        public static class InfoForResourceTypeCVDataResource {
        }
        public static class InfoForResourceTypeCVMixListResource {
        }
        public static class InfoForResourceTypeCVPhysXSurfacePropertiesList {
        }
        public static class InfoForResourceTypeCVSoundEventScriptList {
        }
        public static class InfoForResourceTypeCVSoundStackScriptList {
        }
        public static class InfoForResourceTypeCVoiceContainerBase {
        }
        public static class InfoForResourceTypeCVoxelVisibility {
        }
        public static class InfoForResourceTypeCWorldNode {
        }
        public static class InfoForResourceTypeIAnimGraphModelBinding {
        }
        public static class InfoForResourceTypeIMaterial2 {
        }
        public static class InfoForResourceTypeIParticleSnapshot {
        }
        public static class InfoForResourceTypeIParticleSystemDefinition {
        }
        public static class InfoForResourceTypeIPulseGraphDef {
        }
        public static class InfoForResourceTypeISmartProp {
        }
        public static class InfoForResourceTypeIVectorGraphic {
        }
        public static class InfoForResourceTypeManifestTestResource_t {
        }
        public static class InfoForResourceTypeProceduralTestResource_t {
        }
        public static class InfoForResourceTypeWorld_t {
        }
        public static class ManifestTestResource_t {
            public const nint m_name = 0x0; // CUtlString
            public const nint m_child = 0x8; // CStrongHandle<InfoForResourceTypeManifestTestResource_t>
        }
        public static class NoiseOscillatorDef_t {
            public const nint m_flPhase = 0x0; // float32
            public const nint m_flFrequency = 0x4; // float32
            public const nint m_flAmplitude = 0x8; // float32
        }
        public static class NoiseStreamDef_t {
            public const nint m_nType = 0x0; // NoiseStreamType_t
            public const nint m_nModifier = 0x4; // NoiseStreamModifier_t
            public const nint m_nTurbulence = 0x8; // NoiseStreamTurbulence_t
            public const nint m_flOutputMin = 0xC; // float32
            public const nint m_flOutputMax = 0x10; // float32
            public const nint m_flScale = 0x14; // float32
            public const nint m_vOffsetRate = 0x18; // Vector
            public const nint m_flOffset = 0x24; // float32
            public const nint m_nOctaves = 0x28; // int32
            public const nint m_flTurbulenceScale = 0x2C; // float32
            public const nint m_flTurbulenceMix = 0x30; // float32
            public const nint m_Oscillators = 0x38; // CUtlVector<NoiseOscillatorDef_t>
        }
        public static class PackedAABB_t {
            public const nint m_nPackedMin = 0x0; // uint32
            public const nint m_nPackedMax = 0x4; // uint32
        }
        public static class VariableInfo_t {
            public const nint m_name = 0x0; // CUtlString
            public const nint m_nameToken = 0x8; // CUtlStringToken
            public const nint m_nIndex = 0xC; // FuseVariableIndex_t
            public const nint m_nNumComponents = 0xE; // uint8
            public const nint m_eVarType = 0xF; // FuseVariableType_t
            public const nint m_eAccess = 0x10; // FuseVariableAccess_t
        }
    }
}
