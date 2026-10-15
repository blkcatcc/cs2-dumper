// build 14191

#![allow(non_upper_case_globals, non_camel_case_types, non_snake_case, unused)]

pub mod cs2_dumper {
    pub mod schemas {
        pub mod libresourcesystem_so {
            #[repr(u8)]
            pub enum FuseVariableAccess_t {
                WRITABLE = 0x0,
                READ_ONLY = 0x1
            }
            #[repr(u8)]
            pub enum FuseVariableType_t {
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
            #[repr(u32)]
            pub enum NoiseStreamModifier_t {
                NOISE_STREAM_MODIFIER_NONE = 0x0,
                NOISE_STREAM_MODIFIER_LINES = 0x1,
                NOISE_STREAM_MODIFIER_CLUMPS = 0x2,
                NOISE_STREAM_MODIFIER_RINGS = 0x3
            }
            #[repr(u32)]
            pub enum NoiseStreamTurbulence_t {
                NOISE_STREAM_TURB_NONE = 0x0,
                NOISE_STREAM_TURB_HIGHLIGHT = 0x1,
                NOISE_STREAM_TURB_FEEDBACK = 0x2,
                NOISE_STREAM_TURB_LOOPY = 0x3,
                NOISE_STREAM_TURB_CONTRAST = 0x4,
                NOISE_STREAM_TURB_ALTERNATE = 0x5
            }
            #[repr(u32)]
            pub enum NoiseStreamType_t {
                NOISE_STREAM_TYPE_PERLIN = 0x0,
                NOISE_STREAM_TYPE_SIMPLEX = 0x1,
                NOISE_STREAM_TYPE_WORLEY = 0x2,
                NOISE_STREAM_TYPE_CURL = 0x3,
                NOISE_STREAM_TYPE_NONE = 0x4
            }
            pub mod AABBWS_t {
                pub const m_vMinBounds: usize = 0x0; // VectorWS
                pub const m_vMaxBounds: usize = 0xC; // VectorWS
            }
            pub mod AABB_t {
                pub const m_vMinBounds: usize = 0x0; // Vector
                pub const m_vMaxBounds: usize = 0xC; // Vector
            }
            pub mod CFuseProgram {
                pub const m_programBuffer: usize = 0x0; // CUtlVector<uint8>
                pub const m_variablesRead: usize = 0x18; // CUtlVector<FuseVariableIndex_t>
                pub const m_variablesWritten: usize = 0x30; // CUtlVector<FuseVariableIndex_t>
                pub const m_nMaxTempVarsUsed: usize = 0x48; // int32
            }
            pub mod CFuseSymbolTable {
                pub const m_constants: usize = 0x0; // CUtlVector<ConstantInfo_t>
                pub const m_variables: usize = 0x18; // CUtlVector<VariableInfo_t>
                pub const m_functions: usize = 0x30; // CUtlVector<FunctionInfo_t>
                pub const m_constantMap: usize = 0x48; // CUtlHashtable<CUtlStringToken,int32>
                pub const m_variableMap: usize = 0x68; // CUtlHashtable<CUtlStringToken,int32>
                pub const m_functionMap: usize = 0x88; // CUtlHashtable<CUtlStringToken,int32>
            }
            pub mod ConstantInfo_t {
                pub const m_name: usize = 0x0; // CUtlString
                pub const m_nameToken: usize = 0x8; // CUtlStringToken
                pub const m_flValue: usize = 0xC; // float32
            }
            pub mod FourQuaternions {
                pub const x: usize = 0x0; // fltx4
                pub const y: usize = 0x10; // fltx4
                pub const z: usize = 0x20; // fltx4
                pub const w: usize = 0x30; // fltx4
            }
            pub mod FunctionInfo_t {
                pub const m_name: usize = 0x8; // CUtlString
                pub const m_nameToken: usize = 0x10; // CUtlStringToken
                pub const m_nParamCount: usize = 0x14; // int32
                pub const m_nIndex: usize = 0x18; // FuseFunctionIndex_t
                pub const m_bIsPure: usize = 0x1A; // bool
            }
            pub mod FuseFunctionIndex_t {
                pub const m_Value: usize = 0x0; // uint16
            }
            pub mod FuseVariableIndex_t {
                pub const m_Value: usize = 0x0; // uint16
            }
            pub mod InfoForResourceTypeCAnimData {
            }
            pub mod InfoForResourceTypeCAnimationGroup {
            }
            pub mod InfoForResourceTypeCCSGOEconItem {
            }
            pub mod InfoForResourceTypeCChoreoSceneResource {
            }
            pub mod InfoForResourceTypeCCompositeMaterialKit {
            }
            pub mod InfoForResourceTypeCDOTANovelsList {
            }
            pub mod InfoForResourceTypeCDOTAPatchNotesList {
            }
            pub mod InfoForResourceTypeCDotaItemDefinitionResource {
            }
            pub mod InfoForResourceTypeCEntityLump {
            }
            pub mod InfoForResourceTypeCGcExportableExternalData {
            }
            pub mod InfoForResourceTypeCJavaScriptResource {
            }
            pub mod InfoForResourceTypeCModel {
            }
            pub mod InfoForResourceTypeCMorphSetData {
            }
            pub mod InfoForResourceTypeCNmClip {
            }
            pub mod InfoForResourceTypeCNmGraphDefinition {
            }
            pub mod InfoForResourceTypeCNmSkeleton {
            }
            pub mod InfoForResourceTypeCPanoramaDynamicImages {
            }
            pub mod InfoForResourceTypeCPanoramaLayout {
            }
            pub mod InfoForResourceTypeCPanoramaStyle {
            }
            pub mod InfoForResourceTypeCPhysAggregateData {
            }
            pub mod InfoForResourceTypeCPostProcessingResource {
            }
            pub mod InfoForResourceTypeCRenderMesh {
            }
            pub mod InfoForResourceTypeCResponseRulesList {
            }
            pub mod InfoForResourceTypeCSequenceGroupData {
            }
            pub mod InfoForResourceTypeCSurfaceGraph {
            }
            pub mod InfoForResourceTypeCTestResourceData {
            }
            pub mod InfoForResourceTypeCTextureBase {
            }
            pub mod InfoForResourceTypeCTypeScriptResource {
            }
            pub mod InfoForResourceTypeCVDSPResource {
            }
            pub mod InfoForResourceTypeCVDataItemDefs {
            }
            pub mod InfoForResourceTypeCVDataResource {
            }
            pub mod InfoForResourceTypeCVMixListResource {
            }
            pub mod InfoForResourceTypeCVPhysXSurfacePropertiesList {
            }
            pub mod InfoForResourceTypeCVSoundEventScriptList {
            }
            pub mod InfoForResourceTypeCVSoundStackScriptList {
            }
            pub mod InfoForResourceTypeCVoiceContainerBase {
            }
            pub mod InfoForResourceTypeCVoxelVisibility {
            }
            pub mod InfoForResourceTypeCWorldNode {
            }
            pub mod InfoForResourceTypeIAnimGraphModelBinding {
            }
            pub mod InfoForResourceTypeIMaterial2 {
            }
            pub mod InfoForResourceTypeIParticleSnapshot {
            }
            pub mod InfoForResourceTypeIParticleSystemDefinition {
            }
            pub mod InfoForResourceTypeIPulseGraphDef {
            }
            pub mod InfoForResourceTypeISmartProp {
            }
            pub mod InfoForResourceTypeIVectorGraphic {
            }
            pub mod InfoForResourceTypeManifestTestResource_t {
            }
            pub mod InfoForResourceTypeProceduralTestResource_t {
            }
            pub mod InfoForResourceTypeWorld_t {
            }
            pub mod ManifestTestResource_t {
                pub const m_name: usize = 0x0; // CUtlString
                pub const m_child: usize = 0x8; // CStrongHandle<InfoForResourceTypeManifestTestResource_t>
            }
            pub mod NoiseOscillatorDef_t {
                pub const m_flPhase: usize = 0x0; // float32
                pub const m_flFrequency: usize = 0x4; // float32
                pub const m_flAmplitude: usize = 0x8; // float32
            }
            pub mod NoiseStreamDef_t {
                pub const m_nType: usize = 0x0; // NoiseStreamType_t
                pub const m_nModifier: usize = 0x4; // NoiseStreamModifier_t
                pub const m_nTurbulence: usize = 0x8; // NoiseStreamTurbulence_t
                pub const m_flOutputMin: usize = 0xC; // float32
                pub const m_flOutputMax: usize = 0x10; // float32
                pub const m_flScale: usize = 0x14; // float32
                pub const m_vOffsetRate: usize = 0x18; // Vector
                pub const m_flOffset: usize = 0x24; // float32
                pub const m_nOctaves: usize = 0x28; // int32
                pub const m_flTurbulenceScale: usize = 0x2C; // float32
                pub const m_flTurbulenceMix: usize = 0x30; // float32
                pub const m_Oscillators: usize = 0x38; // CUtlVector<NoiseOscillatorDef_t>
            }
            pub mod PackedAABB_t {
                pub const m_nPackedMin: usize = 0x0; // uint32
                pub const m_nPackedMax: usize = 0x4; // uint32
            }
            pub mod VariableInfo_t {
                pub const m_name: usize = 0x0; // CUtlString
                pub const m_nameToken: usize = 0x8; // CUtlStringToken
                pub const m_nIndex: usize = 0xC; // FuseVariableIndex_t
                pub const m_nNumComponents: usize = 0xE; // uint8
                pub const m_eVarType: usize = 0xF; // FuseVariableType_t
                pub const m_eAccess: usize = 0x10; // FuseVariableAccess_t
            }
        }
    }
}
