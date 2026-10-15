// build 14191

pub const cs2_dumper = struct {
    pub const schemas = struct {
        pub const libresourcesystem_so = struct {
            pub const FuseVariableAccess_t = enum(u8) {
                WRITABLE = 0x0,
                READ_ONLY = 0x1
            };
            pub const FuseVariableType_t = enum(u8) {
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
            pub const NoiseStreamModifier_t = enum(u32) {
                NOISE_STREAM_MODIFIER_NONE = 0x0,
                NOISE_STREAM_MODIFIER_LINES = 0x1,
                NOISE_STREAM_MODIFIER_CLUMPS = 0x2,
                NOISE_STREAM_MODIFIER_RINGS = 0x3
            };
            pub const NoiseStreamTurbulence_t = enum(u32) {
                NOISE_STREAM_TURB_NONE = 0x0,
                NOISE_STREAM_TURB_HIGHLIGHT = 0x1,
                NOISE_STREAM_TURB_FEEDBACK = 0x2,
                NOISE_STREAM_TURB_LOOPY = 0x3,
                NOISE_STREAM_TURB_CONTRAST = 0x4,
                NOISE_STREAM_TURB_ALTERNATE = 0x5
            };
            pub const NoiseStreamType_t = enum(u32) {
                NOISE_STREAM_TYPE_PERLIN = 0x0,
                NOISE_STREAM_TYPE_SIMPLEX = 0x1,
                NOISE_STREAM_TYPE_WORLEY = 0x2,
                NOISE_STREAM_TYPE_CURL = 0x3,
                NOISE_STREAM_TYPE_NONE = 0x4
            };
            pub const AABBWS_t = struct {
                pub const m_vMinBounds: usize = 0x0; // VectorWS
                pub const m_vMaxBounds: usize = 0xC; // VectorWS
            };
            pub const AABB_t = struct {
                pub const m_vMinBounds: usize = 0x0; // Vector
                pub const m_vMaxBounds: usize = 0xC; // Vector
            };
            pub const CFuseProgram = struct {
                pub const m_programBuffer: usize = 0x0; // CUtlVector<uint8>
                pub const m_variablesRead: usize = 0x18; // CUtlVector<FuseVariableIndex_t>
                pub const m_variablesWritten: usize = 0x30; // CUtlVector<FuseVariableIndex_t>
                pub const m_nMaxTempVarsUsed: usize = 0x48; // int32
            };
            pub const CFuseSymbolTable = struct {
                pub const m_constants: usize = 0x0; // CUtlVector<ConstantInfo_t>
                pub const m_variables: usize = 0x18; // CUtlVector<VariableInfo_t>
                pub const m_functions: usize = 0x30; // CUtlVector<FunctionInfo_t>
                pub const m_constantMap: usize = 0x48; // CUtlHashtable<CUtlStringToken,int32>
                pub const m_variableMap: usize = 0x68; // CUtlHashtable<CUtlStringToken,int32>
                pub const m_functionMap: usize = 0x88; // CUtlHashtable<CUtlStringToken,int32>
            };
            pub const ConstantInfo_t = struct {
                pub const m_name: usize = 0x0; // CUtlString
                pub const m_nameToken: usize = 0x8; // CUtlStringToken
                pub const m_flValue: usize = 0xC; // float32
            };
            pub const FourQuaternions = struct {
                pub const x: usize = 0x0; // fltx4
                pub const y: usize = 0x10; // fltx4
                pub const z: usize = 0x20; // fltx4
                pub const w: usize = 0x30; // fltx4
            };
            pub const FunctionInfo_t = struct {
                pub const m_name: usize = 0x8; // CUtlString
                pub const m_nameToken: usize = 0x10; // CUtlStringToken
                pub const m_nParamCount: usize = 0x14; // int32
                pub const m_nIndex: usize = 0x18; // FuseFunctionIndex_t
                pub const m_bIsPure: usize = 0x1A; // bool
            };
            pub const FuseFunctionIndex_t = struct {
                pub const m_Value: usize = 0x0; // uint16
            };
            pub const FuseVariableIndex_t = struct {
                pub const m_Value: usize = 0x0; // uint16
            };
            pub const InfoForResourceTypeCAnimData = struct {
            };
            pub const InfoForResourceTypeCAnimationGroup = struct {
            };
            pub const InfoForResourceTypeCCSGOEconItem = struct {
            };
            pub const InfoForResourceTypeCChoreoSceneResource = struct {
            };
            pub const InfoForResourceTypeCCompositeMaterialKit = struct {
            };
            pub const InfoForResourceTypeCDOTANovelsList = struct {
            };
            pub const InfoForResourceTypeCDOTAPatchNotesList = struct {
            };
            pub const InfoForResourceTypeCDotaItemDefinitionResource = struct {
            };
            pub const InfoForResourceTypeCEntityLump = struct {
            };
            pub const InfoForResourceTypeCGcExportableExternalData = struct {
            };
            pub const InfoForResourceTypeCJavaScriptResource = struct {
            };
            pub const InfoForResourceTypeCModel = struct {
            };
            pub const InfoForResourceTypeCMorphSetData = struct {
            };
            pub const InfoForResourceTypeCNmClip = struct {
            };
            pub const InfoForResourceTypeCNmGraphDefinition = struct {
            };
            pub const InfoForResourceTypeCNmSkeleton = struct {
            };
            pub const InfoForResourceTypeCPanoramaDynamicImages = struct {
            };
            pub const InfoForResourceTypeCPanoramaLayout = struct {
            };
            pub const InfoForResourceTypeCPanoramaStyle = struct {
            };
            pub const InfoForResourceTypeCPhysAggregateData = struct {
            };
            pub const InfoForResourceTypeCPostProcessingResource = struct {
            };
            pub const InfoForResourceTypeCRenderMesh = struct {
            };
            pub const InfoForResourceTypeCResponseRulesList = struct {
            };
            pub const InfoForResourceTypeCSequenceGroupData = struct {
            };
            pub const InfoForResourceTypeCSurfaceGraph = struct {
            };
            pub const InfoForResourceTypeCTestResourceData = struct {
            };
            pub const InfoForResourceTypeCTextureBase = struct {
            };
            pub const InfoForResourceTypeCTypeScriptResource = struct {
            };
            pub const InfoForResourceTypeCVDSPResource = struct {
            };
            pub const InfoForResourceTypeCVDataItemDefs = struct {
            };
            pub const InfoForResourceTypeCVDataResource = struct {
            };
            pub const InfoForResourceTypeCVMixListResource = struct {
            };
            pub const InfoForResourceTypeCVPhysXSurfacePropertiesList = struct {
            };
            pub const InfoForResourceTypeCVSoundEventScriptList = struct {
            };
            pub const InfoForResourceTypeCVSoundStackScriptList = struct {
            };
            pub const InfoForResourceTypeCVoiceContainerBase = struct {
            };
            pub const InfoForResourceTypeCVoxelVisibility = struct {
            };
            pub const InfoForResourceTypeCWorldNode = struct {
            };
            pub const InfoForResourceTypeIAnimGraphModelBinding = struct {
            };
            pub const InfoForResourceTypeIMaterial2 = struct {
            };
            pub const InfoForResourceTypeIParticleSnapshot = struct {
            };
            pub const InfoForResourceTypeIParticleSystemDefinition = struct {
            };
            pub const InfoForResourceTypeIPulseGraphDef = struct {
            };
            pub const InfoForResourceTypeISmartProp = struct {
            };
            pub const InfoForResourceTypeIVectorGraphic = struct {
            };
            pub const InfoForResourceTypeManifestTestResource_t = struct {
            };
            pub const InfoForResourceTypeProceduralTestResource_t = struct {
            };
            pub const InfoForResourceTypeWorld_t = struct {
            };
            pub const ManifestTestResource_t = struct {
                pub const m_name: usize = 0x0; // CUtlString
                pub const m_child: usize = 0x8; // CStrongHandle<InfoForResourceTypeManifestTestResource_t>
            };
            pub const NoiseOscillatorDef_t = struct {
                pub const m_flPhase: usize = 0x0; // float32
                pub const m_flFrequency: usize = 0x4; // float32
                pub const m_flAmplitude: usize = 0x8; // float32
            };
            pub const NoiseStreamDef_t = struct {
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
            };
            pub const PackedAABB_t = struct {
                pub const m_nPackedMin: usize = 0x0; // uint32
                pub const m_nPackedMax: usize = 0x4; // uint32
            };
            pub const VariableInfo_t = struct {
                pub const m_name: usize = 0x0; // CUtlString
                pub const m_nameToken: usize = 0x8; // CUtlStringToken
                pub const m_nIndex: usize = 0xC; // FuseVariableIndex_t
                pub const m_nNumComponents: usize = 0xE; // uint8
                pub const m_eVarType: usize = 0xF; // FuseVariableType_t
                pub const m_eAccess: usize = 0x10; // FuseVariableAccess_t
            };
        };
    };
};
